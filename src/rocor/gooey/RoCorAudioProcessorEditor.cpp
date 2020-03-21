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
    , mupStartTest(new juce::TextButton("Start Test"))
    , processor(p)
{
    // Make sure that before the constructor has finished, you've set the
    // editor's size to whatever you need it to be.
    setSize(500, 600);
    addAndMakeVisible(mupStartTest.get());
    mupStartTest->addListener(this);
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
	g.setColour(juce::Colours::silver);
	g.fillRect(bezel);
	mupStartTest->setBounds(bottom_buttons.removeFromLeft(100));
}

void RoCorAudioProcessorEditor::resized()
{
}

void RoCorAudioProcessorEditor::changeListenerCallback(juce::ChangeBroadcaster* source)
{
}

void RoCorAudioProcessorEditor::buttonClicked(juce::Button* b)
{
    if (b == mupStartTest.get()){
		processor.startImpulseCapture();
    }
}
