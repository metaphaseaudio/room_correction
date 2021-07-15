//
// Created by Matt on 4/18/2020.
//
#pragma once

#include <meta/dsp/MultiChanConvolve.h>
#include <meta/util/container_helpers/comparisons.h>
#include <meta/generators/complementary_sequence.h>
#include <rocor/guts/IRCalculator.h>


class GolayIRCalculator
{
public:
    GolayIRCalculator(int golay_n)
    {
        auto golay_pair = meta::generate_golay_dynamic<float>(golay_n);
        juce::AudioBuffer<float> a, b;
        auto a_data = golay_pair.first.data();
        auto b_data = golay_pair.second.data();
        a.setDataToReferTo(&a_data, 1, golay_pair.first.size());
        b.setDataToReferTo(&b_data, 1, golay_pair.second.size());
        m_GolayA = std::make_unique<meta::dsp::MultiChanConvolve>(std::move(a), 512);
        m_GolayB = std::make_unique<meta::dsp::MultiChanConvolve>(std::move(b), 512);
    }

    juce::AudioBuffer<float> calculate(const juce::AudioBuffer<float>& x, int pre_roll, int len)
    {
        // simple way of getting around the off-by-one incurred by argmax
        pre_roll++;
        juce::AudioBuffer<float> x_cpy(x);
        x_cpy.reverse(0, x_cpy.getNumSamples());
        auto a = m_GolayA->convolve(x_cpy);
        auto b = m_GolayB->convolve(x_cpy);

        float* max_a = meta::argmax(a.getArrayOfWritePointers()[0], a.getArrayOfWritePointers()[0] + a.getNumSamples()) - len;
        float* max_b = meta::argmax(b.getArrayOfWritePointers()[0], b.getArrayOfWritePointers()[0] + b.getNumSamples()) - len;

        juce::AudioBuffer<float> rv(1, len + pre_roll);
        rv.copyFrom(0, 0, max_a, len + pre_roll, 1.0f / m_GolayA->get_ir_length());
        rv.addFrom(0, 0, max_b, len + pre_roll, 1.0f / m_GolayB->get_ir_length());
        rv.applyGain(0.5);
        rv.reverse(0, rv.getNumSamples());
        return rv;
    }

private:
    std::unique_ptr<meta::dsp::MultiChanConvolve> m_GolayA, m_GolayB;
};


TEST(IRCalculatorTest, golay)
{
    // Setup the IR
    juce::AudioBuffer<float> test_ir(1, 5);
    test_ir.clear();
    test_ir.setSample(0, 1, 1.0f);
    test_ir.setSample(0, 2, 0.5f);
    auto pre_conv = meta::dsp::MultiChanConvolve(std::move(test_ir), 512);

    // Setup the test signal
    int golay_n = 4;
    int gap_samps = 100;
    auto golay_pair = meta::generate_golay_dynamic<float>(golay_n);
    juce::AudioBuffer<float> base_signal(1, (gap_samps * 3) + std::pow(2, golay_n + 1));

    base_signal.clear();
    base_signal.copyFrom(0, gap_samps,
                         golay_pair.first.data(), golay_pair.first.size());
    base_signal.copyFrom(0, gap_samps + golay_pair.first.size() + gap_samps,
                         golay_pair.second.data(), golay_pair.second.size());

    auto test_signal = pre_conv.convolve(base_signal);

    // set up the calculator
    auto ir_calc = GolayIRCalculator(golay_n);
    auto ir = ir_calc.calculate(test_signal,0, 5);
    ASSERT_EQ(*test_ir.getReadPointer(0), *ir.getReadPointer(0));
}

TEST(IRCalculatorTest, mono_static)
{
    std::array<float, 3> refarr = { 0, 1, 0.5 };
    std::array<float, 7> caparr = { 0, 0, 1, 0, 0, 0, 0 };
    float* refp = refarr.data();
    float* capp = caparr.data();
    juce::AudioBuffer<float> ref(&refp , 1, refarr.size());
    juce::AudioBuffer<float> cap(&capp, 1, caparr.size());



    auto out = rocor::IRCalculator::calc_impulse(cap, ref);
}

