//
// Created by mzapp on 4/15/18.
//

#include "RoCorComponent.h"

RoCorComponent::RoCorComponent(juce::AudioDeviceManager& devicemgr)
	: mProcPlayer()
{
	devicemgr.addAudioCallback(&mProcPlayer);
	mProcPlayer.setProcessor(&processor);
    m_Laf.reset(new RoCorLAF());
    editor.reset(new RoCorAudioProcessorEditor(processor));
    setLookAndFeel(m_Laf.get());
    addAndMakeVisible(editor.get());
    setBounds(0, 0, 500, 600);
}

void RoCorComponent::resized()
    { editor->setBounds(getLocalBounds()); }

RoCorComponent::~RoCorComponent()
    { setLookAndFeel(nullptr); }
