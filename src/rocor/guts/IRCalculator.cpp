//
// Created by Matt on 4/18/2020.
//

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
    const auto tmpStore = (meta::simd<T>::isAligned(tmp)) ? Mode::store_aligned :
                          Mode::store_unaligned;

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

    auto tmp = juce::AudioBuffer<float>(1, cap.getNumSamples() + ref.getNumSamples());
    auto out = juce::AudioBuffer<float>(1, impulse_samps);
    std::memset(tmp.getArrayOfWritePointers()[0], 0 , sizeof(T*) * tmp.getNumSamples());
    std::memset(out.getArrayOfWritePointers()[0], 0 , sizeof(T*) * out.getNumSamples());

    // Prep reference stream
    juce::AudioBuffer<float> ref_cp(ref);
    ref_cp.reverse(0, ref_cp.getNumSamples());

    // Prep convolution
    juce::dsp::ProcessSpec spec = { 1, std::max(cap.getNumSamples(), ref.getNumSamples()) * 2, 1 };
    juce::dsp::Convolution conv;
    conv.prepare(spec);
    conv.copyAndLoadImpulseResponseFromBuffer(ref_cp, 1, false, false, false, ref.getNumSamples());

    // Convolve
    juce::dsp::AudioBlock<float> in_block(cap);
    juce::dsp::AudioBlock<float> out_block(tmp);
    juce::dsp::ProcessContextNonReplacing<float> context(in_block, out_block);
    conv.process(context);

    // Normalize
    float denom = std::sqrt(calc_norm_factor(ref.getArrayOfReadPointers()[0], ref.getNumSamples()));
    meta::simd<float>::div(tmp.getArrayOfWritePointers()[0], denom, tmp.getNumSamples());

    // Move to output
    auto start_index = ref.getNumSamples() - 1;
    std::memcpy(out.getArrayOfWritePointers()[0]
            , tmp.getArrayOfReadPointers()[0] + start_index
            , sizeof(out.getArrayOfWritePointers()[0]) * impulse_samps);

    return out;
}

///////////////////////////////////////////////////////////////////////////////
rocor::IRCalculator::IRCalculator
(const std::vector<juce::AudioBuffer<float>>& captureBank, const juce::AudioBuffer<float>& reference, int chans, int samps)
    : juce::Thread("IRcalc")
    , r_CaptureBank(captureBank)
    , r_Reference(reference)
    , m_Impulse(chans, samps)
{
    m_Impulse.clear();
}

void rocor::IRCalculator::run()
{
    for (auto& capture : r_CaptureBank)
    {
        juce::AudioBuffer<float> tmp = capture;
        tmp.clear();
        m_Impulse = calculate_impulse(tmp, r_Reference, m_Impulse.getNumSamples());
    }
    sendChangeMessage();
}

