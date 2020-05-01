//
// Created by Matt on 4/18/2020.
//
#pragma once

#include <JuceHeader.h>
#include <meta/util/simd_ops.h>


namespace rocor
{
    class IRCalculator
        : public juce::Thread
        , public juce::ChangeBroadcaster
    {
    public:
        IRCalculator(const std::vector<juce::AudioBuffer<float>>& captureBank, const juce::AudioBuffer<float>& reference, int chans, int samps);
        void run();
        juce::AudioBuffer<float> getCalculatedIR() { return m_Impulse; };

    private:
        const std::vector<juce::AudioBuffer<float>>& r_CaptureBank;
        const juce::AudioBuffer<float>& r_Reference;
        juce::AudioBuffer<float> m_Impulse;
    };
}