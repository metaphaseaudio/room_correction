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
        a.reverse(0, a.getNumSamples());
        b.reverse(0, b.getNumSamples());
        m_GolayA = std::make_unique<meta::dsp::MultiChanConvolve>(std::move(a), 512);
        m_GolayB = std::make_unique<meta::dsp::MultiChanConvolve>(std::move(b), 512);
    }

    juce::AudioBuffer<float> calculate(juce::AudioBuffer<float>& x, int pre_roll, int len)
    {
        // allocate memory for temporary, in-place calculations
        juce::AudioBuffer<float> convolved_result(x.getNumChannels(), x.getNumSamples() + m_GolayA->get_ir_length());

        // allocate memory for storing the maxes
        juce::AudioBuffer<float> tmp;
        std::vector<int> bursts_i(x.getNumChannels());
        for (auto c = x.getNumChannels(); --c >= 0;)
        {
            auto chan_ptr = x.getArrayOfWritePointers()[c];
            tmp.setDataToReferTo(&chan_ptr, 1, x.getNumSamples());

            auto a = m_GolayA->convolve(tmp);
            auto b = m_GolayB->convolve(tmp);

            // Locate the peak of the impulse, work backwards (we're reversed)
            auto a_ptr = a.getArrayOfWritePointers()[c];
            auto b_ptr = b.getArrayOfWritePointers()[c];

            // find the start of each impulse
            float* a_max = meta::argmax(a_ptr, a_ptr + a.getNumSamples());
            auto a_max_i = a_max - a_ptr + 1;
            float* b_max = meta::argmax(b_ptr, b_ptr + b.getNumSamples());
            auto b_max_i = b_max - b_ptr + 1;

            jassert(a_max_i != b_max_i); // This should really be impossible

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

            bursts_i[c] = impulse_i;
        }

        auto first_burst_sample = *meta::argmax(bursts_i.begin(), bursts_i.end());
        auto last_burst_sample = *meta::argmin(bursts_i.begin(), bursts_i.end());
        auto propagation_delay = last_burst_sample - first_burst_sample;

        auto total_length = len + pre_roll + propagation_delay;
        auto copy_start = first_burst_sample - (len + propagation_delay);

        // allocate memory for impulse + pre-roll + any inter-channel
        // propagation delay
        juce::AudioBuffer<float> rv(x.getNumChannels(), total_length);

        for (auto c = x.getNumChannels(); --c >= 0;) { rv.copyFrom(c, 0, convolved_result, c, copy_start, total_length); }
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
    test_ir.setSample(0, 0, 1.0f);
//    test_ir.setSample(0, 1, 0.5f);
    auto pre_conv = meta::dsp::MultiChanConvolve(std::move(test_ir), 512);

    // Setup the test signal
    int golay_n = 2;
    int gap_samps = 5;
    auto golay_pair = meta::generate_golay_dynamic<float>(golay_n);
    juce::AudioBuffer<float> base_signal(1, (gap_samps * 3) + std::pow(2, golay_n + 1));

    base_signal.clear();
    base_signal.copyFrom(0, gap_samps,
                         golay_pair.first.data(), golay_pair.first.size());
    base_signal.copyFrom(0, gap_samps + golay_pair.first.size() + gap_samps,
                         golay_pair.second.data(), golay_pair.second.size());

    auto test_signal = pre_conv.convolve(base_signal);

    // set up the calculator and run
    auto ir_calc = GolayIRCalculator(golay_n);
    auto ir = ir_calc.calculate(test_signal,0, 5);

    ASSERT_NEAR(*test_ir.getReadPointer(0), *ir.getReadPointer(0), 0.01);
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

