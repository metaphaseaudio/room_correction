//
// Created by Matt on 4/18/2020.
//

#include <cassert>
#include "IRCalculator.h"


template <typename T>
static T calc_norm_factor(const T* x, size_t n)
{
    using Mode = meta::simd<T>::ModeType<sizeof(*x)>::Mode;
    const int nSIMDOps = n / Mode::numParallel;

    const auto xLoad = (meta::simd<T>::isAligned(x)) ? Mode::load_aligned : Mode::load_unaligned;

    Mode::ParallelType x_pow = Mode::load_one(0);

    for (int i = 0; i < nSIMDOps; i++)
    {
        const Mode::ParallelType xp = xLoad(x);
        x_pow = Mode::hadd(x_pow, Mode::mul(xp, xp));
        x += (16 / sizeof(*x));
    }

    T tmp[4];
    const auto tmpStore = (meta::simd<T>::isAligned(tmp)) ? Mode::store_aligned : Mode::store_unaligned;

    tmpStore(tmp, x_pow);
    T x_sum = tmp[0] + tmp[1] + tmp[2] + tmp[3];

    n &= (Mode::numParallel - 1);

    if (n == 0) { return x_sum; }
    for (int i = 0; i < n; ++i) { x_sum += x[i] * x[i]; }

    return x_sum;
}


template <typename T>
static juce::AudioBuffer<T> calculate_impulse(juce::AudioBuffer<T>& cap, const juce::AudioBuffer<T>& ref, size_t impulse_samps)
{
    const auto total_length = cap.getNumSamples() + ref.getNumSamples(); // impulse_samps
    auto tmp = juce::AudioBuffer<float>(1, total_length);
    auto out = juce::AudioBuffer<float>(cap.getNumChannels(), total_length);
    out.clear();

    // Prep reference stream
    juce::AudioBuffer<float> ref_cp(ref);
    ref_cp.reverse(0, ref_cp.getNumSamples());

    // Prep convolution
    juce::dsp::ProcessSpec spec = { 1, static_cast<juce::uint32>(std::max(cap.getNumSamples(), ref.getNumSamples()) * 2), 1 };
    juce::dsp::Convolution conv;
    conv.prepare(spec);
    conv.copyAndLoadImpulseResponseFromBuffer(ref_cp, 1, false, false, false, ref.getNumSamples());

    // Calc normalization denominator
    float denom = std::sqrt(calc_norm_factor(ref.getArrayOfReadPointers()[0], ref.getNumSamples()));

    for (auto chan = out.getNumChannels(); --chan >= 0;)
    {
        // Clear temp
        tmp.clear();
        auto tmpptr = tmp.getArrayOfWritePointers()[0];
        auto outptr = out.getArrayOfWritePointers()[chan];

        // Convolve
        juce::dsp::AudioBlock<float> in_block(cap.getArrayOfWritePointers() + chan, 1, 0, cap.getNumSamples());
        juce::dsp::AudioBlock<float> out_block(tmp);
        juce::dsp::ProcessContextNonReplacing<float> context(in_block, out_block);
        conv.process(context);

        // Normalize
        meta::simd<float>::div(tmpptr, denom, tmp.getNumSamples());

        // Move to output, trimmed
        auto offset = ref.getNumSamples() * chan;
        out.copyFrom(chan, 0, tmp, 0, offset, total_length);
    }

    return out;
}

///////////////////////////////////////////////////////////////////////////////
rocor::IRCalculator::IRCalculator
(const rocor::CaptureMap<juce::AudioBuffer<float>>& captureBank, const juce::AudioBuffer<float>& reference, int chans, int samps)
    : juce::Thread("IRcalc")
    , r_CaptureBank(captureBank)
    , r_Reference(reference)
    , m_Impulse(chans, samps)
    , progress(0)
{ m_Impulse.clear(); }

void rocor::IRCalculator::run()
{
    m_Impulse.clear();
    progress = 0;

    const auto capture_count = float(r_CaptureBank.size() * m_Impulse.getNumChannels());

    if (capture_count == 0)
    {
        sendChangeMessage();
        return;
    }

    float done = 0;

    for (auto capture : r_CaptureBank)
    {
        auto pos = capture.first;
        m_Calculated[pos] = calculate_impulse(capture.second, r_Reference, m_Impulse.getNumSamples());

        for (auto chan = m_Impulse.getNumChannels(); --chan >= 0;)
        {
            m_Impulse.addFrom(chan, 0, m_Calculated.at(pos), chan, 0, m_Impulse.getNumSamples());
            progress = ++done / capture_count;
        }
    }

    m_Impulse.applyGain(1.0f / capture_count);
    sendChangeMessage();
}

void rocor::IRCalculator::saveImpulse(juce::AudioFormatWriter* writer)
{
    writer->writeFromAudioSampleBuffer(m_Impulse, 0, m_Impulse.getNumSamples());
}

void rocor::IRCalculator::loadImpulse(juce::AudioFormatReader* reader)
{
    m_Impulse.setSize(reader->numChannels, reader->lengthInSamples);
    reader->read(m_Impulse.getArrayOfWritePointers(), reader->numChannels, 0, reader->lengthInSamples);
}

void rocor::IRCalculator::saveIndividualImpulses(juce::AudioFormat* fmt, const juce::File& dir, int sampleRate)
{
    for (int i = rocor::capture_position_names.size(); --i >= 0;)
    {
        const auto name = rocor::capture_position_names[i];
        const auto pos = rocor::ordered_capture_positions[i];

        if (!m_Calculated.count(pos)) { continue;}

        const auto outfile = dir.getChildFile(name).withFileExtension(fmt->getFileExtensions()[0]);
        outfile.create();

        auto outstream = outfile.createOutputStream();
        const auto& buffer = m_Calculated.at(pos);  // r_CaptureBank.at(pos);
        std::unique_ptr<juce::AudioFormatWriter> writer(fmt->createWriterFor(outstream.release(), sampleRate, buffer.getNumChannels(), 32, NULL, 0));
        writer->writeFromAudioSampleBuffer(buffer, 0, m_Impulse.getNumSamples());
    }
}

void rocor::IRCalculator::loadIndividualImpulses(juce::AudioFormat* fmt, const juce::File& dir)
{
    for (auto i = rocor::capture_position_names.size(); --i >= 0;)
    {
        const auto name = rocor::capture_position_names[i];
        const auto pos = rocor::ordered_capture_positions[i];

        if (m_Calculated.at(pos).getNumSamples() != m_Impulse.getNumSamples())
        {
            continue;
        };

        auto in = dir.getChildFile(name).withFileExtension(fmt->getFileExtensions()[0]).createInputStream();
        std::unique_ptr<juce::AudioFormatReader> reader(fmt->createReaderFor(in.get(), false));
        m_Calculated[pos] = juce::AudioBuffer<float>(reader->numChannels, reader->lengthInSamples);
        reader->read(m_Calculated.at(pos).getArrayOfWritePointers(), reader->numChannels, 0, reader->lengthInSamples);
    }
}
