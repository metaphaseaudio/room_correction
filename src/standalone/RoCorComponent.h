//
// Created by mzapp on 4/15/18.
//
#pragma once

#include <JuceHeader.h>
#include <rocor/gooey/RoCorLAF.h>
#include <rocor/guts/RoCorAudioProcessor.h>
#include <rocor/gooey/RoCorAudioProcessorEditor.h>

class RoCorComponent
    : public juce::Component
{
public:
    explicit RoCorComponent(juce::AudioDeviceManager& devicemgr);
    ~RoCorComponent();
    void resized() override;

private:
    RoCorAudioProcessor processor;
	juce::AudioProcessorPlayer mProcPlayer;
    std::unique_ptr<RoCorLAF> m_Laf;
    std::unique_ptr<RoCorAudioProcessorEditor> editor;
};


juce::ApplicationCommandManager& getCommandManager();
juce::ApplicationProperties& getAppProperties();
bool isOnTouchDevice();
