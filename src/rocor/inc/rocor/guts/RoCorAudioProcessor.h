/*
 ==============================================================================

   This file was auto-generated!

   It contains the basic framework code for a JUCE plugin processor.

 ==============================================================================
*/

#pragma once

#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_audio_utils/juce_audio_utils.h>
#include <meta/generators/LinearRamp.h>
#include <meta/dsp/BandlimitedWavetable.h>
#include <meta/generators/complementary_sequence.h>
#include "IRCalculator.h"
#include "CapturePosition.h"

#include <meta/generators/SineSweep.h>
//==============================================================================
/**
*/

#define ROCOR_OSC_TABLE_SIZE 48000
#define ROCOR_PAUSE_SECS 1.0f
#define ROCOR_SWEEP_SECS 5
#define ROCOR_NOISE_SECS 5
#define ROCOR_GOLAY_N 15


class RoCorAudioProcessor
    : public juce::AudioProcessor
    , public juce::ChangeBroadcaster
    , public juce::ChangeListener
{
public:
    typedef enum {
        NONE,
        GOLAY_A,
        GOLAY_B,
        PINK,
        SWEEP
    } playbackType;

    //==============================================================================
        RoCorAudioProcessor();
    ~RoCorAudioProcessor();

    //==============================================================================
    void prepareToPlay(double sampleRate, int samplesPerBlock) override;

    void releaseResources() override;

#ifndef JucePlugin_PreferredChannelConfigurations
    bool isBusesLayoutSupported (const BusesLayout& layouts) const override;
#endif

    void processBlock(juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    //==============================================================================
    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override;

    //==============================================================================
    const juce::String getName() const override;

    bool acceptsMidi() const override;

    bool producesMidi() const override;

    bool isMidiEffect() const override;

    double getTailLengthSeconds() const override;

    //==============================================================================
    int getNumPrograms() override;

    int getCurrentProgram() override;

    void setCurrentProgram(int index) override;

    const juce::String getProgramName(int index) override;

    void changeProgramName(int index, const juce::String& newName) override;

    //==============================================================================
    void getStateInformation(juce::MemoryBlock& destData) override;

    void setStateInformation(const void* data, int sizeInBytes) override;

    void changeListenerCallback(juce::ChangeBroadcaster* source) override;

    void startCapture(rocor::CapturePosition position);
    rocor::CapturePosition getCapturePosition() const
    {
        return m_CapturePosition;
    };

    void stopCapture();

    void startImpulseCalc() { m_IRCalc.startThread(); };
    bool isCapturing() const { return m_IsCapturing; }
    rocor::IRCalculator& getIRCalc() { return m_IRCalc; }
    void setImpulseLength(size_t x) { m_Impulse.setSize(m_Impulse.getNumChannels(), x, true, true); }
    void setImpulseChans(int x)     { m_Impulse.setSize(x, m_Impulse.getNumSamples(),  true, true); }


    const juce::AudioBuffer<float>& getImpulse() const { return m_Impulse; };


    juce::AudioVisualiserComponent m_InputView;
    const rocor::CaptureMap<juce::AudioBuffer<float>>& getCaptures() const { return m_CaptureBank; }

private:
    rocor::IRCalculator m_IRCalc;

    void resetCapture(int chan);
    bool m_IsCapturing = false;
    bool m_IsPlayingBack = false;
    bool m_IsProcessing = false;
    int m_CaptureChan = 0;

    rocor::CapturePosition m_CapturePosition = rocor::CapturePosition::FRONT_L;
    size_t m_CaptureIndex;

    size_t m_ReferenceIndex;
    juce::AudioBuffer<float> m_Capture;

    juce::AudioBuffer<float> m_Reference;
    rocor::CaptureMap<juce::AudioBuffer<float>> m_CaptureBank;
    juce::AudioBuffer<float> m_Impulse;
    meta::SineSweep<float, ROCOR_OSC_TABLE_SIZE> m_Sweep;

    void generateReference();
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (RoCorAudioProcessor);

};
