#include <meta/audio/SingletonSampleRate.h>
#include <meta/util/math.h>

#include "RoCorAudioProcessor.h"
#include "../gooey/RoCorAudioProcessorEditor.h"

using namespace juce;
//==============================================================================
RoCorAudioProcessor::RoCorAudioProcessor()
	: m_SineTable(meta::BandlimitedWavetable<float, ROCOR_OSC_TABLE_SIZE>::makeSin())
	, m_SineOsc(m_SineTable)
	, m_Ramp(10, 20000, 1)
{}
RoCorAudioProcessor::~RoCorAudioProcessor() {}

//==============================================================================
const String RoCorAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool RoCorAudioProcessor::acceptsMidi() const
{
#if JucePlugin_WantsMidiInput
    return true;
#else
    return false;
#endif
}

bool RoCorAudioProcessor::producesMidi() const
{
#if JucePlugin_ProducesMidiOutput
    return true;
#else
    return false;
#endif
}

bool RoCorAudioProcessor::isMidiEffect() const
{
#if JucePlugin_IsMidiEffect
    return true;
#else
    return false;
#endif
}

double RoCorAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int RoCorAudioProcessor::getNumPrograms() { return 1; }

int RoCorAudioProcessor::getCurrentProgram() { return 0; }

void RoCorAudioProcessor::setCurrentProgram(int index) {}

const String RoCorAudioProcessor::getProgramName(int index) { return {}; }

void RoCorAudioProcessor::changeProgramName(int index, const String &newName) {}

//==============================================================================
void RoCorAudioProcessor::prepareToPlay(double sampleRate, int samplesPerBlock)
{
	m_Ramp.setEnd(sampleRate / 2);
	m_Ramp.setTicks(5 * sampleRate);
	m_Ramp.reset();
}

void RoCorAudioProcessor::releaseResources() {}

#ifndef JucePlugin_PreferredChannelConfigurations
bool RoCorAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
#if JucePlugin_IsMidiEffect
    ignoreUnused (layouts);
    return true;
#else
    // This is the place where you check if the layout is supported.
    // In this template code we only support mono or stereo.
    if (layouts.getMainOutputChannelSet() != AudioChannelSet::mono()
     && layouts.getMainOutputChannelSet() != AudioChannelSet::stereo())
        return false;

    // This checks if the input layout matches the output layout
#if ! JucePlugin_IsSynth
    if (layouts.getMainOutputChannelSet() != layouts.getMainInputChannelSet())
        return false;
#endif

    return true;
#endif
}
#endif

void RoCorAudioProcessor::processBlock(AudioBuffer<float> &buffer, MidiBuffer &midiMessages)
{
    if (m_IsCapturing) 
	{		
		for (auto samp = 0; samp < buffer.getNumSamples(); samp++)
		{
			if (m_Ramp.getProgress() < 1)
			{
				m_SineOsc.setFrequency(m_Ramp.tick());
				auto value = m_SineOsc.tick();
				for (auto chan = 0; chan < buffer.getNumChannels(); chan++) { buffer.setSample(chan, samp, value); }
			}
			else { m_IsCapturing = false;  }
		}
    }

    else if (m_IsProcessing) {
        ScopedNoDenormals noDenormals;
    }
}

//==============================================================================
bool RoCorAudioProcessor::hasEditor() const
{
    return true; // (change this to false if you choose to not supply an editor)
}

AudioProcessorEditor *RoCorAudioProcessor::createEditor()
    { return new RoCorAudioProcessorEditor(*this); }

//==============================================================================
void RoCorAudioProcessor::getStateInformation(MemoryBlock &destData) {}

void RoCorAudioProcessor::setStateInformation(const void *data, int sizeInBytes) {}

void RoCorAudioProcessor::startImpulseCapture()
{
    m_IsCapturing = true;
    p_Capture.reset(new juce::AudioBuffer<float>(1, 0));
	m_Ramp.reset();
}

void RoCorAudioProcessor::stopImpulseCapture()
{
    m_IsCapturing = false;

}

const juce::AudioBuffer<float>*
RoCorAudioProcessor::calculateImpulse
(juce::AudioBuffer<float>* capture, juce::AudioBuffer<float>* ref) const
{
    return nullptr;
}

//==============================================================================
// This creates new instances of the plugin..
AudioProcessor *JUCE_CALLTYPE createPluginFilter()
    { return new RoCorAudioProcessor(); }
