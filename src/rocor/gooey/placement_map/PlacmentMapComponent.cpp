//
// Created by Matt on 5/3/2020.
//

#include "PlacmentMapComponent.h"

PlacmentMapComponent::PlacmentMapComponent()
{
    for (auto& icon : m_Icons) { addAndMakeVisible(icon); }
    m_Icons[0].m_IsSelected = true;
}

void PlacmentMapComponent::setActivePlacement(int x)
{
    for (int i = m_Icons.size(); --i >= 0;) { m_Icons[i].m_IsSelected = x == i; }
}

void PlacmentMapComponent::resized()
{
    auto iconShape = juce::Rectangle<float>(30, 30);
    auto localBounds = getLocalBounds().reduced(30);

    iconShape.setCentre(localBounds.getTopLeft().toFloat());     m_Icons.at(0).setBounds(iconShape.toNearestInt());
    iconShape.setCentre(localBounds.getTopRight().toFloat());    m_Icons.at(1).setBounds(iconShape.toNearestInt());
    iconShape.setCentre(localBounds.getBottomLeft().toFloat());  m_Icons.at(2).setBounds(iconShape.toNearestInt());
    iconShape.setCentre(localBounds.getBottomRight().toFloat()); m_Icons.at(3).setBounds(iconShape.toNearestInt());


    const auto vertOffset = juce::Point<float>(0, std::min<float>(localBounds.getHeight() / 5, 50));
    const auto horizOffset = juce::Point<float>(std::min<float>(localBounds.getWidth() / 5, 50), 0);

    iconShape.setCentre(localBounds.getCentre().toFloat() + vertOffset);    m_Icons.at(4).setBounds(iconShape.toNearestInt());
    iconShape.setCentre(localBounds.getCentre().toFloat() - vertOffset);    m_Icons.at(5).setBounds(iconShape.toNearestInt());
    iconShape.setCentre(localBounds.getCentre().toFloat() + horizOffset);  m_Icons.at(6).setBounds(iconShape.toNearestInt());
    iconShape.setCentre(localBounds.getCentre().toFloat() - horizOffset); m_Icons.at(7).setBounds(iconShape.toNearestInt());
}

void PlacmentMapComponent::paint(juce::Graphics& g)
{
    g.setColour(juce::Colours::lightgrey.brighter()); g.fillRect(getLocalBounds());
    g.setColour(juce::Colours::grey);                 g.drawRect(getLocalBounds(), 3);
}
