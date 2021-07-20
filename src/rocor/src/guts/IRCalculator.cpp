//
// Created by Matt on 4/18/2020.
//

#include <cassert>
#include <meta/util/simd_ops.h>
#include <meta/generators/complementary_sequence.h>
#include <meta/dsp/FindInSignal.h>
#include <rocor/guts/IRCalculator.h>


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


static juce::AudioBuffer<float> calculate_impulse_from_golay(juce::AudioBuffer<float>& capture, size_t golay_n = 16)
{
    auto golay_pair = meta::generate_golay_dynamic<float>(golay_n);
    juce::AudioBuffer<float> a, b;
    auto a_data = golay_pair.first.data();
    auto b_data = golay_pair.second.data();

    a.setDataToReferTo(&a_data, 1, golay_pair.first.size());
    b.setDataToReferTo(&b_data, 1, golay_pair.second.size());
    meta::dsp::MultiChanConvolve golay_a(std::move(a), 512);
    meta::dsp::MultiChanConvolve golay_b(std::move(b), 512);

    auto a_range = meta::dsp::find_in_signal(golay_a, capture);
    auto b_range = meta::dsp::find_in_signal(golay_b, capture);

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
//    ref_cp.reverse(0, ref_cp.getNumSamples());

    // Prep convolution
//    juce::dsp::ProcessSpec spec = { 1, static_cast<juce::uint32>(std::max(cap.getNumSamples(), ref.getNumSamples()) * 2), 1 };
    juce::dsp::ProcessSpec spec = { 1, static_cast<juce::uint32>(total_length), 1 };
    juce::dsp::Convolution conv;
    conv.prepare(spec);
    conv.loadImpulseResponse(std::move(ref_cp), 1,
                             juce::dsp::Convolution::Stereo::no,
                             juce::dsp::Convolution::Trim::no,
                             juce::dsp::Convolution::Normalise::no);

    // Calc normalization denominator
    float denom = std::sqrt(calc_norm_factor(ref.getArrayOfReadPointers()[0], ref.getNumSamples()));

    for (auto chan = out.getNumChannels(); --chan >= 0;)
    {
        // Clear temp
        tmp.clear();

        // Convolve
        juce::dsp::AudioBlock<float> in_block(cap.getArrayOfWritePointers() + chan, 1, 0, cap.getNumSamples());
        juce::dsp::AudioBlock<float> out_block(tmp);
        juce::dsp::ProcessContextNonReplacing<float> context(in_block, out_block);
        conv.process(context);

        // Normalize
        meta::simd<float>::div(tmp.getArrayOfWritePointers()[0], denom, tmp.getNumSamples());

        // Move to output, trimmed
        auto offset = ref.getNumSamples() * chan;
        out.copyFrom(chan, 0, tmp, 0, offset, total_length);
    }

    return out;
}

///////////////////////////////////////////////////////////////////////////////
rocor::IRCalculator::IRCalculator(
        const rocor::CaptureMap<juce::AudioBuffer<float>>& captureBank, const juce::AudioBuffer<float>& ref, int golay_n, int chans, int samps)
    : juce::Thread("IRcalc")
    , r_CaptureBank(captureBank)
    , m_Ref(ref)
    , m_Impulse(chans, samps)
    , progress(0)
    , m_GolayCalc(golay_n)
{
    // Ready the impulse for calcualtion
    m_Impulse.clear();
}

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

    juce::AudioBuffer<float> tmp(r_CaptureBank.size() + 1, m_Ref.getNumSamples() + m_Impulse.getNumSamples());
    tmp.clear();
    tmp.copyFrom(0, 0, m_Ref, 0, 0, m_Ref.getNumSamples()); // do this to get all the propagation delay
    auto chan_i = 1;
    for (auto capture : r_CaptureBank)
    {
        tmp.copyFrom(chan_i, 0, capture.second, 0, 0, capture.second.getNumSamples());
        chan_i++;
    }

    auto calculated = m_GolayCalc.calculate(tmp, 30, m_Impulse.getNumSamples());
//
//    chan_i = 1;
//    for (auto capture : r_CaptureBank)
//    {
//        auto pos = capture.first;
//        m_Calculated[pos] =
//
//        for (auto chan = m_Impulse.getNumChannels(); --chan >= 0;)
//        {
//            m_Impulse.addFrom(chan, 0, m_Calculated.at(pos), chan, 0, m_Impulse.getNumSamples());
//            progress = ++done / capture_count;
//        }
//    }

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

juce::AudioBuffer<float> rocor::IRCalculator::calc_impulse(juce::AudioBuffer<float>& x, juce::AudioBuffer<float>& y)
{
    return calculate_impulse<float>(x, y, 1024);
}


rocor::GolayIRCalculator::GolayIRCalculator(int golay_n)
{
    auto golay_pair = meta::generate_golay_dynamic<float>(golay_n);
    juce::AudioBuffer<float> a, b;
    auto a_data = golay_pair.first.data();
    auto b_data = golay_pair.second.data();
    a.setDataToReferTo(&a_data, 1, golay_pair.first.size());
    b.setDataToReferTo(&b_data, 1, golay_pair.second.size());
    a.reverse(0, a.getNumSamples());
    b.reverse(0, b.getNumSamples());
    m_GolayA = std::make_unique<meta::dsp::MultiChanConvolve>(std::move(a), 512);
    m_GolayB = std::make_unique<meta::dsp::MultiChanConvolve>(std::move(b), 512);
}

juce::AudioBuffer<float> rocor::GolayIRCalculator::calculate(juce::AudioBuffer<float>& x, int pre_roll, int len)
{
    // allocate memory for temporary, in-place calculations
    juce::AudioBuffer<float> convolved_result(x.getNumChannels(), x.getNumSamples() + m_GolayA->get_ir_length());

    // allocate memory for storing the maxes
    juce::AudioBuffer<float> tmp;
    std::vector<int> bursts_i;
    for (auto c = x.getNumChannels(); --c >= 0;)
    {
        auto chan_ptr = x.getArrayOfWritePointers()[c];
        tmp.setDataToReferTo(&chan_ptr, 1, x.getNumSamples());

        auto a = m_GolayA->convolve(tmp);
        auto b = m_GolayB->convolve(tmp);

        // Locate the peak of the impulse, work backwards (we're reversed).
        // also, these are mono. don't try to grab the channel.
        auto a_ptr = a.getArrayOfWritePointers()[0];
        auto b_ptr = b.getArrayOfWritePointers()[0];

        // find the start of each impulse
        float* a_max = meta::argmax(a_ptr, a_ptr + a.getNumSamples());
        auto a_max_i = a_max - a_ptr;
        float* b_max = meta::argmax(b_ptr, b_ptr + b.getNumSamples());
        auto b_max_i = b_max - b_ptr;

        // These are all cases where the burst is not found.
        if (a_max_i == b_max_i || a_max_i + len > x.getNumSamples() || b_max_i + len > x.getNumSamples())
        {
            jassertfalse;
            // Make sure the channel is cleared since we don't bother clearing otherwise.
            convolved_result.clear(c, 0, convolved_result.getNumSamples());
            continue;
        }

        auto impulse_i = std::max(a_max_i, b_max_i);

        // figure out what's first (max, again, because they who are last
        // shall be first)
        std::pair<float*, float*> first_burst = a_max_i >= b_max_i ? std::make_pair(a_ptr, a_max) : std::make_pair(b_ptr, b_max);
        std::pair<float*, float*> second_burst = a_max_i < b_max_i ? std::make_pair(a_ptr, a_max) : std::make_pair(b_ptr, b_max);

        auto burst_distance = std::abs(a_max_i - b_max_i);
        auto total_available_pre_roll = tmp.getNumSamples() - impulse_i;

        jassert(total_available_pre_roll > pre_roll);

        // Do all the math to manage the averaging/normalization here so
        // that the second pass is just logic around cutting/moving.
        convolved_result.copyFrom(c, 0, first_burst.first, convolved_result.getNumSamples(), 0.5f / m_GolayA->get_ir_length());
        convolved_result.addFrom(c, burst_distance, second_burst.first, tmp.getNumSamples() - burst_distance, 0.5f / m_GolayA->get_ir_length());

        // Store this for later so we can measure the earliest/latest bursts
        bursts_i.push_back(impulse_i);
    }

    // Because of the possibility of silence, we have to assume everything is silent
    auto copy_start = 0;
    auto total_length = len + pre_roll;

    // calculate memory allocation for impulse + pre-roll + any inter-channel
    // propagation delay
    if (!bursts_i.empty())
    {
        auto last_burst_sample = *meta::argmax(bursts_i.begin(), bursts_i.end());
        auto first_burst_sample = *meta::argmin(bursts_i.begin(), bursts_i.end());
        auto propagation_delay = last_burst_sample - first_burst_sample;
        total_length += propagation_delay;
        copy_start = first_burst_sample - pre_roll;
    }

    juce::AudioBuffer<float> rv(x.getNumChannels(), total_length);
    // We do need to take the time to clear here due to the possibility of
    // propagation delay.
    rv.clear();

    for (auto c = x.getNumChannels(); --c >= 0;) { rv.copyFrom(c, 0, convolved_result, c, copy_start, total_length); }
    return rv;
}

