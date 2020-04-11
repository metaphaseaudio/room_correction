/*
  ==============================================================================

    This file was auto-generated!

    It contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include <meta/generators/LinearRamp.h>
#include <meta/dsp/WavetableOscillator.h>
#include <meta/dsp/BandlimitedWavetable.h>

//==============================================================================
/**
*/

#define ROCOR_OSC_TABLE_SIZE 48000
#define ROCOR_SWEEP_SECS 5
#define ROCOR_NOISE_SECS 5
#define ROCOR_GOLAY_SECS 0


class RoCorAudioProcessor
    : public juce::AudioProcessor
{
public:
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

    void startImpulseCapture();
    void stopImpulseCapture();
	const bool isCapturing() const { return m_IsCapturing; }

    const juce::AudioBuffer<float>* calculateImpulse(
            juce::AudioBuffer<float>* capture, juce::AudioBuffer<float>* ref) const;


	juce::AudioVisualiserComponent m_InputView;

private:
    //==============================================================================
    bool m_IsCapturing = false;
    bool m_IsProcessing = false;
    
	std::array<float, ROCOR_OSC_TABLE_SIZE> m_SineTable;
	meta::WavetableOscilator<float, ROCOR_OSC_TABLE_SIZE> m_SineOsc;
	meta::LinearRamp m_Ramp;

	size_t m_CaptureIndex;
	std::unique_ptr<juce::AudioBuffer<float>> p_Capture;
	std::unique_ptr<juce::AudioBuffer<float>> p_Reference;

    std::vector<std::unique_ptr<juce::AudioBuffer<float>>> m_Impulses;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (RoCorAudioProcessor);
};
