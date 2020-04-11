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
    , mupStartTest("Start Test")
    , processor(p)
	, m_ImpulseView(1, m_FormatMgr, m_ThumbCache)
{
    // Make sure that before the constructor has finished, you've set the
    // editor's size to whatever you need it to be.
    setSize(500, 600);
    addAndMakeVisible(&mupStartTest);
	addAndMakeVisible(&processor.m_InputView);
	mupStartTest.addListener(this);
}

RoCorAudioProcessorEditor::~RoCorAudioProcessorEditor()
{
}

//==============================================================================
void RoCorAudioProcessorEditor::paint (Graphics& g)
{
    g.fillAll(getLookAndFeel().findColour(ResizableWindow::backgroundColourId));
	const auto bezel = getLocalBounds().reduced(5);
	auto control_space = bezel.reduced(5);
	auto bottom_buttons = control_space.removeFromBottom(50);
	control_space.removeFromBottom(5);
	g.setColour(juce::Colours::silver);
	g.fillRect(bezel);
	mupStartTest.setBounds(bottom_buttons.removeFromLeft(100));
	processor.m_InputView.setBounds(control_space.removeFromBottom(100));
	control_space.removeFromBottom(25);
}

void RoCorAudioProcessorEditor::resized()
{
}

void RoCorAudioProcessorEditor::changeListenerCallback(juce::ChangeBroadcaster* source)
{
}

void RoCorAudioProcessorEditor::buttonClicked(juce::Button* b)
{
    if (b == &mupStartTest){
		processor.startImpulseCapture();
    }
}
