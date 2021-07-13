//
// Created by Matt on 4/18/2020.
//
#pragma once
#include <JuceHeader.h>
#include <rocor/guts/IRCalculator.h>



TEST(IRCalculatorTest, mono)
{
    std::array<float, 3> refarr = { 0, 1, 0.5 };
    std::array<float, 7> caparr = { 0, 0, 1, 0, 0, 0, 0 };
    float* refp = refarr.data();
    float* capp = caparr.data();
    juce::AudioBuffer<float> ref(&refp , 1, refarr.size());
    juce::AudioBuffer<float> cap(&capp, 1, caparr.size());

    auto out = rocor::IRCalculator::calc_impulse(cap, ref);
}

