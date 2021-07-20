//
// Created by Matt on 4/18/2020.
//
#pragma once

#include <meta/dsp/MultiChanConvolve.h>
#include <meta/util/container_helpers/comparisons.h>
#include <meta/generators/complementary_sequence.h>
#include <rocor/guts/IRCalculator.h>


TEST(IRCalculatorTest, golay)
{
    // Setup the IR
    juce::AudioBuffer<float> test_ir(3, 5);
    test_ir.clear();
    test_ir.setSample(0, 0, 1.0f);
    test_ir.setSample(1, 3, 0.5f);
    auto pre_conv = meta::dsp::MultiChanConvolve(std::move(test_ir), 512);

    // Setup the test signal
    int golay_n = 2;
    int gap_samps = 5;
    auto golay_pair = meta::generate_golay_dynamic<float>(golay_n);
    juce::AudioBuffer<float> base_signal(test_ir.getNumChannels(), (gap_samps * 3) + std::pow(2, golay_n + 1));

    base_signal.clear();
    base_signal.copyFrom(0, gap_samps, golay_pair.first.data(), golay_pair.first.size());
    base_signal.copyFrom(0, gap_samps + golay_pair.first.size() + gap_samps, golay_pair.second.data(), golay_pair.second.size());
    base_signal.copyFrom(1, 0, base_signal, 0, 0, base_signal.getNumSamples());
    base_signal.copyFrom(2, 0, base_signal, 0, 0, base_signal.getNumSamples());

    auto test_signal = pre_conv.convolve(base_signal);

    // set up the calculator and run
    auto ir_calc = rocor::GolayIRCalculator(golay_n);
    auto ir = ir_calc.calculate(test_signal, 1, 5);

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

