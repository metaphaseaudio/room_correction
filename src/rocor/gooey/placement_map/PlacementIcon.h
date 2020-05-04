//
// Created by Matt on 5/3/2020.
//

#pragma once

#include <JuceHeader.h>

struct PlacementIcon
    : public juce::Component
{
    void paint (juce::Graphics& g) override
    {
        const auto inner_colour = m_IsSelected ? juce::Colours::yellow : juce::Colours::aquamarine;
        const auto outer_colour = m_IsSelected ? juce::Colours::orange : juce::Colours::grey;
        const auto localBounds = getLocalBounds().reduced(2);
        g.setColour(inner_colour); g.fillEllipse(localBounds.toFloat());
        g.setColour(outer_colour); g.drawEllipse(localBounds.toFloat(), 1);
    }

    bool m_IsSelected = false;
};