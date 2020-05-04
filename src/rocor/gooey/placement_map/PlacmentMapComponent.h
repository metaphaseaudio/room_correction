//
// Created by Matt on 5/3/2020.
//

#pragma once
#include <JuceHeader.h>
#include "PlacementIcon.h"


class PlacmentMapComponent
    : public juce::Component
{
public:
    PlacmentMapComponent();
    void setActivePlacement(int i);


    void paint (juce::Graphics& g) override;
    void resized() override;

private:
    std::array<PlacementIcon, 8> m_Icons;
};


