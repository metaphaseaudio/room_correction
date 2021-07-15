#pragma once

#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_audio_utils/juce_audio_utils.h>
#include <meta/gooey/RadioGrid.h>
#include <meta/gooey/WaveformComponent.h>
#include <meta/gooey/MaskingComponent.h>

#include "../guts/RoCorAudioProcessor.h"
#include "ThumbnailViewListBoxModel.h"
#include "placement_map/PlacmentMapComponent.h"


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

    explicit RoCorAudioProcessorEditor(RoCorAudioProcessor&);
    ~RoCorAudioProcessorEditor() override;

    void paint (juce::Graphics&) override;
    void resized() override;

    void changeListenerCallback(juce::ChangeBroadcaster* source) override;
    void buttonClicked(juce::Button*) override;
    void comboBoxChanged(juce::ComboBox* changed) override;

private:
    RoCorAudioProcessor& processor;
	juce::AudioFormatManager m_FormatMgr;
	meta::WaveformComponent m_ImpulseView;

	juce::TabbedComponent m_Tabs;
	PlacmentMapComponent m_PlacementView;
    rocor::ThumbnailViewListBoxModel m_CapturesModel;
    rocor::ThumbnailViewListBoxModel m_ImpulsesModel;
    juce::ListBox m_Captures;
    juce::ListBox m_Impulses;

    juce::TextButton m_NewTest;
    juce::TextButton m_StartTest;
    juce::TextButton m_StartCalc;
    juce::TextButton m_Save;
    juce::TextButton m_SaveAll;
    juce::TextButton m_Load;
    juce::TextButton m_LoadAll;

    juce::Label m_LabelChanCount;
    juce::ComboBox m_ChanCount;

    // Protoype component-blocking progress indicator
    juce::ProgressBar m_Progress;
    meta::MaskingComponent m_SemiModalMask;
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (RoCorAudioProcessorEditor);
};
