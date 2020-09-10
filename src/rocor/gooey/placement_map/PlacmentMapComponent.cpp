//
// Created by Matt on 5/3/2020.
//

#include "PlacmentMapComponent.h"

PlacmentMapComponent::PlacmentMapComponent()
{
    for (auto& capPos : rocor::ordered_capture_positions)
    {
        auto& icon = std::get<0>(m_Icons.emplace(capPos, std::make_unique<PlacementIcon>()))->second;
        icon->addChangeListener(this);
        addAndMakeVisible(icon.get());
    }

    m_Icons.at(rocor::FRONT_L)->selected = true;
}


void PlacmentMapComponent::resized()
{
    auto iconShape = juce::Rectangle<float>(30, 30);
    auto localBounds = getLocalBounds().reduced(30);

    iconShape.setCentre(localBounds.getTopLeft().toFloat());     m_Icons.at(rocor::FRONT_L)->setBounds(iconShape.toNearestInt());
    iconShape.setCentre(localBounds.getTopRight().toFloat());    m_Icons.at(rocor::FRONT_R)->setBounds(iconShape.toNearestInt());
    iconShape.setCentre(localBounds.getBottomLeft().toFloat());  m_Icons.at(rocor::BACK_L)->setBounds(iconShape.toNearestInt());
    iconShape.setCentre(localBounds.getBottomRight().toFloat()); m_Icons.at(rocor::BACK_R)->setBounds(iconShape.toNearestInt());

    const auto vertOffset = juce::Point<float>(0, std::min<float>(localBounds.getHeight() / 5, 50));
    const auto horizOffset = juce::Point<float>(std::min<float>(localBounds.getWidth() / 5, 50), 0);

    iconShape.setCentre(localBounds.getCentre().toFloat() + vertOffset);  m_Icons.at(rocor::LISTEN_BACK)->setBounds(iconShape.toNearestInt());
    iconShape.setCentre(localBounds.getCentre().toFloat() - vertOffset);  m_Icons.at(rocor::LISTEN_FRONT)->setBounds(iconShape.toNearestInt());
    iconShape.setCentre(localBounds.getCentre().toFloat() + horizOffset); m_Icons.at(rocor::LISTEN_R)->setBounds(iconShape.toNearestInt());
    iconShape.setCentre(localBounds.getCentre().toFloat() - horizOffset); m_Icons.at(rocor::LISTEN_L)->setBounds(iconShape.toNearestInt());
}

void PlacmentMapComponent::paint(juce::Graphics& g)
{
    g.setColour(juce::Colours::lightgrey.brighter()); g.fillRect(getLocalBounds());
    g.setColour(juce::Colours::grey);                 g.drawRect(getLocalBounds(), 3);
}

rocor::CapturePosition PlacmentMapComponent::getSelected() const
{
    for (const auto pos : rocor::ordered_capture_positions)
    {
        if (m_Icons.at(pos)->selected) { return pos; }
    }
}

void PlacmentMapComponent::setSelected(rocor::CapturePosition position)
{
    for (const auto pos : rocor::ordered_capture_positions)
    {
        const auto& icon = m_Icons.at(pos);
        icon->selected = false;
    }

    m_Icons.at(position)->selected = true;
}

void PlacmentMapComponent::selectNext()
{
    bool select_next = false;
    for (const auto pos : rocor::ordered_capture_positions)
    {
        auto& icon = m_Icons.at(pos);
        if (icon->selected)
        {
            icon->selected = false;
            select_next = true;
        }
        else if (select_next)
        {
            icon->selected = true;
            repaint();
            return;
        }
    }
    m_Icons.at(rocor::ordered_capture_positions[0])->selected = true;
    repaint();
}

void PlacmentMapComponent::changeListenerCallback(juce::ChangeBroadcaster* changed)
{
    for (auto& icon : m_Icons) { icon.second->selected = false; }
    dynamic_cast<PlacementIcon*>(changed)->selected = true;
    sendChangeMessage();
    repaint();
}

void PlacmentMapComponent::setPositionComplete(rocor::CapturePosition position, bool done)
{
    m_Icons.at(position)->complete = done;
    repaint();
}

bool PlacmentMapComponent::getPositionComplete(rocor::CapturePosition position)
{
    return m_Icons.at(position)->complete;
}
