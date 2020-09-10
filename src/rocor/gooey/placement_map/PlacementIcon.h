//
// Created by Matt on 5/3/2020.
//

#pragma once

#include <JuceHeader.h>

class PlacementIcon
    : public juce::Component
    , public juce::ChangeBroadcaster
{
public:
    PlacementIcon()
        : selected(false)
        , complete(false)
    {}

    void paint (juce::Graphics& g) override
    {
        const auto inner_colour = getInnerColour();
        const auto outer_colour = getOuterColour();
        const auto localBounds = getLocalBounds().reduced(2);
        g.setColour(inner_colour); g.fillEllipse(localBounds.toFloat());
        g.setColour(outer_colour); g.drawEllipse(localBounds.toFloat(), 1);
        if (complete) { /* TODO: Draw checkmark */ }
    }

    void mouseDown(const juce::MouseEvent& event) override { sendChangeMessage(); }

    std::atomic_bool selected;
    std::atomic_bool complete;

private:

    juce::Colour getInnerColour()
    {
        if (selected) { return juce::Colours::yellow; }
        if (complete) { return juce::Colours::aquamarine; }
        return juce::Colours::lightyellow;
    }

    juce::Colour getOuterColour()
    {
        if (selected) { return juce::Colours::white; }
        if (complete) { return juce::Colours::green; }
        return juce::Colours::orange;
    }
};
