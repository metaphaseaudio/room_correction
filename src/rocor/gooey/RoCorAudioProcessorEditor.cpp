/*
  ==============================================================================

    This file was auto-generated!

    It contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#include "../guts/RoCorAudioProcessor.h"
#include "RoCorAudioProcessorEditor.h"

using namespace juce;

//==============================================================================
RoCorAudioProcessorEditor::RoCorAudioProcessorEditor(RoCorAudioProcessor& p)
    : AudioProcessorEditor(&p)
    , mNewTest("New IR")
    , mStartTest("Start Test")
    , mStartCalc("Run IR Calc")
    , processor(p)
    , m_ImpulseView(m_FormatMgr, 1, 1)
    , m_LabelChanCount("Chan Count Label", "Channel Arrangement")
    , m_ChanCount("Chan Count")
{
    setSize(400, 650);
    processor.addChangeListener(this);
    addAndMakeVisible(&processor.m_InputView);
    addAndMakeVisible(&mStartCalc);
    addAndMakeVisible(m_PlacementView);

    addAndMakeVisible(&mStartTest);
    mStartTest.addListener(this);

    addAndMakeVisible(m_ImpulseView);
	m_ImpulseView.setClip(processor.getImpulse(), processor.getImpulse().getNumSamples());


	addAndMakeVisible(m_LabelChanCount);
    addAndMakeVisible(m_ChanCount);
	m_ChanCount.addItem("Mono", ChanCounts::MONO);
    m_ChanCount.addItem("Stereo", ChanCounts::STEREO);
    m_ChanCount.addItem("Quadraphonic", ChanCounts::QUAD);
    m_ChanCount.addItem("5/5.1", ChanCounts::FIVE);
    m_ChanCount.addItem("Octophonic", ChanCounts::OCTO);
    m_ChanCount.addListener(this);
    m_ChanCount.setSelectedId(processor.getImpulse().getNumChannels(), juce::NotificationType::dontSendNotification);
}

RoCorAudioProcessorEditor::~RoCorAudioProcessorEditor() { processor.removeChangeListener(this); }

//==============================================================================
void RoCorAudioProcessorEditor::paint (Graphics& g)
{
    g.fillAll(getLookAndFeel().findColour(ResizableWindow::backgroundColourId));
}

void RoCorAudioProcessorEditor::resized()
{
    const auto bezel = getLocalBounds().reduced(5);
    auto control_space = bezel.reduced(5);
    auto top_controls = control_space.removeFromTop(44);
    auto bottom_buttons = control_space.removeFromBottom(50);

    m_LabelChanCount.setBounds(top_controls.removeFromTop(22));
    m_ChanCount.setBounds(top_controls);
    control_space.removeFromTop(5);

    control_space.removeFromBottom(5);
    mStartTest.setBounds(bottom_buttons.removeFromLeft(100));
    mStartCalc.setBounds(bottom_buttons.removeFromRight(100));

    processor.m_InputView.setBounds(control_space.removeFromBottom(100));
    m_ImpulseView.setBounds(control_space.removeFromTop(100));
    control_space.removeFromTop(5);
    control_space.removeFromBottom(5);
    m_PlacementView.setBounds(control_space);

}

void RoCorAudioProcessorEditor::changeListenerCallback(juce::ChangeBroadcaster* source)
{
    if (source == &processor) { m_ImpulseView.setClip(processor.getImpulse(), processor.getSampleRate()); }
}

void RoCorAudioProcessorEditor::buttonClicked(juce::Button* b)
{
    if (b == &mStartTest) { processor.startImpulseCapture(); }
    if (b == &mStartCalc) { processor.startImpulseCalc(); }
}

void RoCorAudioProcessorEditor::comboBoxChanged(juce::ComboBox* changed)
{
    if (changed == & m_ChanCount) { processor.setImpulseChans(changed->getSelectedId()); }
}