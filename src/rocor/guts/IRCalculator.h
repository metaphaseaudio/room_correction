//
// Created by Matt on 4/18/2020.
//
#pragma once

#include <JuceHeader.h>
#include <meta/util/simd_ops.h>
#include "CapturePosition.h"


namespace rocor
{
    class IRCalculator
        : public juce::Thread
        , public juce::ChangeBroadcaster
    {
    public:
        IRCalculator(const rocor::CaptureMap<juce::AudioBuffer<float>>& captureBank, const juce::AudioBuffer<float>& reference, int chans, int samps);
        void run() override;

        juce::AudioBuffer<float> getCalculatedIR() { return m_Impulse; };

        void rocor::IRCalculator::saveImpulse(juce::AudioFormatWriter* writer);
        void rocor::IRCalculator::loadImpulse(juce::AudioFormatReader* reader);

        void saveIndividualImpulses(juce::AudioFormat* fmt, const juce::File& dir, int sampleRate);
        void loadIndividualImpulses(juce::AudioFormat* fmt, const juce::File& dir);

        static juce::AudioBuffer<float> calc_impulse(juce::AudioBuffer<float>& x, juce::AudioBuffer<float>& y);

        double progress;
        const rocor::CaptureMap<juce::AudioBuffer<float>>& getCalculatedIRMap() const { return m_Calculated; }

    private:
        const rocor::CaptureMap<juce::AudioBuffer<float>>& r_CaptureBank;
        rocor::CaptureMap<juce::AudioBuffer<float>> m_Calculated;
        const juce::AudioBuffer<float>& r_Reference;
        juce::AudioBuffer<float> m_Impulse;
    };
}