//
// Created by Matt on 5/3/2020.
//
#pragma once
#include <juce_gui_basics/juce_gui_basics.h>
#include "PlacementIcon.h"
#include "../../guts/CapturePosition.h"


class PlacmentMapComponent
    : public juce::Component
    , public juce::ChangeBroadcaster
    , juce::ChangeListener
{
public:
    PlacmentMapComponent();

    void paint(juce::Graphics& g) override;

    void resized() override;

    void setPositionComplete(rocor::CapturePosition position, bool done);
    bool getPositionComplete(rocor::CapturePosition position);

    rocor::CapturePosition getSelected() const;
    void setSelected(rocor::CapturePosition);
    void selectNext();

    void changeListenerCallback(juce::ChangeBroadcaster* changed);

private:
    rocor::CaptureMap<std::unique_ptr<PlacementIcon>> m_Icons;
};
