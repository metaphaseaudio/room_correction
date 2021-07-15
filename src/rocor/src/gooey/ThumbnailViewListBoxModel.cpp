//
// Created by Matt on 8/2/2020.
//

#include <rocor/gooey/ThumbnailViewListBoxModel.h>

rocor::ThumbnailViewListBoxModel::ThumbnailViewListBoxModel(const rocor::CaptureMap<juce::AudioBuffer<float>>& map, int sampleRate)
    : r_CaptureMap(map)
    , m_SampleRate(sampleRate)
    , m_Blank(1, 1)
{ m_Blank.clear(); }

void rocor::ThumbnailViewListBoxModel::refreshPosition(rocor::CapturePosition pos)
{
    auto& thumbnail = m_Thumbnails.at(pos);
    thumbnail->clear();

    if (r_CaptureMap.count(pos))
    {
        const auto& buff = r_CaptureMap.at(pos);

    }
}

void rocor::ThumbnailViewListBoxModel::paintListBoxItem(int row, juce::Graphics& g, int width, int height, bool isSelected)
    { g.fillAll(isSelected ? juce::Colours::yellow : juce::Colours::darkgrey); }

juce::Component* rocor::ThumbnailViewListBoxModel::refreshComponentForRow(int row, bool isSelected, juce::Component* toUpdate)
{
    if (row >= rocor::ordered_capture_positions.size()) { return toUpdate; }

    auto pos = rocor::ordered_capture_positions[row];

    if (toUpdate == nullptr)
    {
        auto name = juce::String(rocor::capture_position_names[row]);
        name = name.replace("_", "").toLowerCase() + ":";
        toUpdate = new PositionThumbnail(name.toStdString(), 1);
    }

    if (r_CaptureMap.count(pos)) {
        dynamic_cast<PositionThumbnail*>(toUpdate)->show(r_CaptureMap.at(pos), m_SampleRate);
    }
    else                         { dynamic_cast<PositionThumbnail*>(toUpdate)->show(m_Blank, m_SampleRate); }

    return toUpdate;
}


/////////////////////////////////////////////////////////////////////////////////////////
rocor::PositionThumbnail::PositionThumbnail(const std::string& name, size_t sampsPerPixel)
    : m_Label(name)
    , m_Thumb(1, 1)
{
    addAndMakeVisible(m_Label);
    addAndMakeVisible(m_Thumb);
}

void rocor::PositionThumbnail::resized()
{
    constexpr auto labelHeight = 20;
    constexpr auto thumbnailHeight = 50;
    auto localBounds = getLocalBounds().reduced(2);
    m_Label.setBounds(localBounds.removeFromTop(labelHeight));
    m_Thumb.setBounds(localBounds.removeFromTop(thumbnailHeight));
}


void rocor::PositionThumbnail::show(const juce::AudioBuffer<float>& buff, int sampleRate)
{
    m_Thumb.setClip(buff, sampleRate);
    repaint();
}

void rocor::PositionThumbnail::paint(juce::Graphics& g)
{
    g.setColour(juce::Colours::red);
}
