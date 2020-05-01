#pragma once

#include <JuceHeader.h>
#include <meta/gooey/WaveformComponent.h>
#include "../guts/RoCorAudioProcessor.h"
#include <meta/gooey/RadioGrid.h>

//==============================================================================
/**
*/
class RoCorAudioProcessorEditor
    : public juce::AudioProcessorEditor	
    , juce::ChangeListener
    , juce::Button::Listener
    , juce::ComboBox::Listener
{
public:
    enum ChanCounts {
        MONO=1,
        STEREO=2,
        QUAD=4,
        FIVE=5,
        OCTO=8
    };

    RoCorAudioProcessorEditor (RoCorAudioProcessor&);
    ~RoCorAudioProcessorEditor();

    //==============================================================================
    void paint (juce::Graphics&) override;
    void resized() override;

    void changeListenerCallback(juce::ChangeBroadcaster* source) override;
    void buttonClicked(juce::Button*);
    void comboBoxChanged(juce::ComboBox* changed);

private:
    RoCorAudioProcessor& processor;
	juce::AudioFormatManager m_FormatMgr;
	meta::WaveformComponent m_ImpulseView;
	meta::RadioGrid<juce::TextButton, 1, 2> m_ImpulseSelect;
	juce::TextButton mStartTest;
    juce::TextButton mStartCalc;

    juce::Label m_LabelChanCount;
    juce::ComboBox m_ChanCount;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (RoCorAudioProcessorEditor);
};
