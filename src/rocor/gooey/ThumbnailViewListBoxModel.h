//
// Created by Matt on 8/2/2020.
//

#pragma once
#include <JuceHeader.h>
#include <meta/gooey/WaveformComponent.h>
#include "../guts/CapturePosition.h"


namespace rocor
{
    class PositionThumbnail
        : public juce::Component
    {
    public:
        PositionThumbnail(const std::string& name, size_t sampsPerPixel);
        void paint(juce::Graphics &g) override;
        void resized() override;
        void show(const juce::AudioBuffer<float>& buffer, int sampleRate);
        void clear() { m_Thumb.clear(); }

    private:
        juce::Label m_Label;
        meta::WaveformComponent m_Thumb;

        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(PositionThumbnail);
    };

    class ThumbnailViewListBoxModel
        : public juce::ListBoxModel
    {
    public:
        explicit ThumbnailViewListBoxModel(const rocor::CaptureMap<juce::AudioBuffer<float>>& map, int sampleRate=48000);
        void setSampleRate(int sampleRate) { m_SampleRate = sampleRate; }
        int getNumRows() override
        {
            auto rv = rocor::ordered_capture_positions.size();
            return rv;
        }
        void paintListBoxItem(int row, juce::Graphics& g, int width, int height, bool isSelected) override;
        juce::Component* refreshComponentForRow(int row, bool isSelected, juce::Component* toUpdate) override;
        void refreshPosition(rocor::CapturePosition pos);

    private:
        const rocor::CaptureMap<juce::AudioBuffer<float>>& r_CaptureMap;
        rocor::CaptureMap<PositionThumbnail*> m_Thumbnails;
        juce::AudioBuffer<float> m_Blank;
        int m_SampleRate;
    };
}
