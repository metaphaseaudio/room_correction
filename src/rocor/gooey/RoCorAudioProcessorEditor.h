#pragma once

#include <JuceHeader.h>
#include "../guts/RoCorAudioProcessor.h"

//==============================================================================
/**
*/
class RoCorAudioProcessorEditor
    : public juce::AudioProcessorEditor	
    , juce::ChangeListener
    , juce::Button::Listener
{
public:
    RoCorAudioProcessorEditor (RoCorAudioProcessor&);
    ~RoCorAudioProcessorEditor();

    //==============================================================================
    void paint (juce::Graphics&) override;
    void resized() override;

    void changeListenerCallback(juce::ChangeBroadcaster* source) override;
    void buttonClicked(juce::Button*);

private:
    RoCorAudioProcessor& processor;
	juce::AudioFormatManager m_FormatMgr;
	juce::AudioThumbnailCache m_ThumbCache;
	juce::AudioThumbnail m_ImpulseView;
    juce::TextButton mupStartTest;
	
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (RoCorAudioProcessorEditor);
};
