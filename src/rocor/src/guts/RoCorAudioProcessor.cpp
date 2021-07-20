#include <meta/audio/SingletonSampleRate.h>
#include <meta/util/math.h>
#include <juce_audio_utils/juce_audio_utils.h>
#include <meta/dsp/PinkRandom.h>

#include <rocor/guts/RoCorAudioProcessor.h>
#include <rocor/gooey/RoCorAudioProcessorEditor.h>

using namespace juce;
//==============================================================================
RoCorAudioProcessor::RoCorAudioProcessor()
	: AudioProcessor(BusesProperties().withInput("Input", AudioChannelSet::stereo(), true).withOutput("Output", AudioChannelSet::stereo(), true))
	, m_InputView(1)
	, m_Impulse(2, 22050)
	, m_IsPlayingBack(false)
	, m_IsCapturing(false)
	, m_IsProcessing(false)
	, m_Sweep(48000, 10, 22000, 5 * 48000)
	, m_IRCalc(m_CaptureBank, m_Reference, 16, 2, 48000)
{}

RoCorAudioProcessor::~RoCorAudioProcessor() {}

//==============================================================================
const String RoCorAudioProcessor::getName() const { return JucePlugin_Name; }

bool RoCorAudioProcessor::acceptsMidi() const { return false; }
bool RoCorAudioProcessor::producesMidi() const { return false; }
bool RoCorAudioProcessor::isMidiEffect() const { return false; }

double RoCorAudioProcessor::getTailLengthSeconds() const { return 0.0; }

int RoCorAudioProcessor::getNumPrograms() { return 1; }
int RoCorAudioProcessor::getCurrentProgram() { return 0; }
void RoCorAudioProcessor::setCurrentProgram(int index) {}
const String RoCorAudioProcessor::getProgramName(int index) { return ""; }
void RoCorAudioProcessor::changeProgramName(int index, const String &newName) {}

//==============================================================================
void RoCorAudioProcessor::prepareToPlay(double sampleRate, int samplesPerBlock)
{
    m_IsCapturing = false;
    generateReference();
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
    m_InputView.pushBuffer(buffer.getArrayOfReadPointers(), 1, buffer.getNumSamples());

	if (m_IsCapturing)
    {
	    const auto nSamps = std::min<size_t>(m_Capture.getNumSamples() - m_CaptureIndex, buffer.getNumSamples());
        m_Capture.copyFrom(m_CaptureChan, m_CaptureIndex, buffer, 0, 0, nSamps);
        m_CaptureIndex += buffer.getNumSamples();

        if (m_CaptureIndex >= m_Capture.getNumSamples())
        {
            m_IsCapturing = false;
            const auto nextChan = m_CaptureChan + 1;
            if (nextChan < m_Impulse.getNumChannels()) { resetCapture(nextChan); }
            else
            {
                juce::AudioBuffer<float> copy = m_Capture;
                m_CaptureBank.emplace(std::make_pair(m_CapturePosition, copy));
                m_Capture.clear();
                m_IsProcessing = true;
                sendChangeMessage();
            }
        }
    }

    if (m_IsPlayingBack)
    {
        buffer.clear();
        const auto nSamps = std::min<size_t>(m_Reference.getNumSamples() - m_ReferenceIndex, buffer.getNumSamples());
        buffer.copyFrom(m_CaptureChan, 0, m_Reference, 0, m_ReferenceIndex, nSamps);
        m_ReferenceIndex += buffer.getNumSamples();
        m_IsCapturing = true;
        if (m_ReferenceIndex >= m_Reference.getNumSamples()) { m_IsPlayingBack = false; }
    }

    if (m_IsProcessing)
    {
        ScopedNoDenormals noDenormals;

        // TODO: This is a dumb hack. Obviously.
        buffer.clear();
    }

}

//==============================================================================
bool RoCorAudioProcessor::hasEditor() const { return true; }
AudioProcessorEditor *RoCorAudioProcessor::createEditor() { return new RoCorAudioProcessorEditor(*this); }

//==============================================================================
void RoCorAudioProcessor::getStateInformation(MemoryBlock &destData) {}

void RoCorAudioProcessor::setStateInformation(const void *data, int sizeInBytes) {}

void RoCorAudioProcessor::resetCapture(int chan)
{
    m_IsCapturing = false;
    m_CaptureIndex = 0;
    m_ReferenceIndex = 0;
    m_CaptureChan = chan;
    m_IsPlayingBack = true;
}

void RoCorAudioProcessor::startCapture(rocor::CapturePosition position)
{
    m_IsCapturing = false;
    m_IsProcessing = false;
    m_Impulse.clear();
    m_Capture = juce::AudioBuffer<float>(m_Impulse.getNumChannels(), m_Reference.getNumSamples());
    m_CapturePosition = position;
    resetCapture(0);
}

void RoCorAudioProcessor::changeListenerCallback(juce::ChangeBroadcaster* source)
    { m_Impulse = m_IRCalc.getCalculatedIR(); }


void RoCorAudioProcessor::generateReference()
{
    const auto golaySamps = meta::Golay<ROCOR_GOLAY_N>::size * 2;
    const auto pauseSamps = ROCOR_PAUSE_SECS * getSampleRate();
    const auto noiseSamps = ROCOR_NOISE_SECS * getSampleRate();
    const auto sweepSamps = ROCOR_SWEEP_SECS * getSampleRate();
    const auto nyquist = getSampleRate() / 2;
    m_Reference.setSize(1, golaySamps + pauseSamps + golaySamps + pauseSamps);// + noiseSamps + pauseSamps + sweepSamps + pauseSamps);
    m_Reference.clear();

    size_t samp_i = 0;

    // Setup generators
    constexpr meta::complementary_sequence<meta::Golay<ROCOR_GOLAY_N>::size> golay_sequence = meta::Golay<ROCOR_GOLAY_N>::value;
    constexpr auto golay_a = std::get<0>(golay_sequence);
    constexpr auto golay_b = std::get<1>(golay_sequence);
    auto noise = meta::PinkRandom();

    m_Sweep.setEnd(nyquist);
    m_Sweep.setTicks(sweepSamps);
    m_Sweep.reset();

    for (const auto& samp : golay_a)    { m_Reference.setSample(0, samp_i, samp);           samp_i++; } samp_i += pauseSamps;
    for (const auto& samp : golay_b)    { m_Reference.setSample(0, samp_i, samp);           samp_i++; } // samp_i += pauseSamps;
//    for (const auto& samp : golay_a)    { m_Reference.setSample(0, samp_i, samp);           samp_i++; }
//    for (const auto& samp : golay_b)    { m_Reference.setSample(0, samp_i, -samp);          samp_i++; } samp_i += pauseSamps;
//    for (int i = noiseSamps; --i >= 0;) { m_Reference.setSample(0, samp_i, noise.tick());   samp_i++; } samp_i += pauseSamps;
//    for (int i = noiseSamps; --i >= 0;) { m_Reference.setSample(0, samp_i, m_Sweep.tick()); samp_i++; }
}

void RoCorAudioProcessor::stopCapture()
{
    m_IsCapturing = false;
    m_IsPlayingBack = false;
}

//==============================================================================
// This creates new instances of the plugin..
AudioProcessor *JUCE_CALLTYPE createPluginFilter() { return new RoCorAudioProcessor(); }

