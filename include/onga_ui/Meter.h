#pragma once

#include "Paint.h"

/*
    LevelMeter: a thin vertical segmented meter on the dark screen checker.
    Lit segments are signal cream, the peak-hold segment stays lit, and the top segments
    turn red when the level is within 1 dB of full scale. Feed it linear peaks from a
    timer with setPeak(); it handles fall-off and hold.
*/
namespace onga::ui
{
class LevelMeter final : public juce::Component
{
public:
    explicit LevelMeter (int numSegments = 20, float floorDb = -60.0f) : segments (numSegments), floor (floorDb) {}

    void setPeak (float linear)
    {
        const float db = juce::Decibels::gainToDecibels (linear, floor);
        const float norm = juce::jlimit (0.0f, 1.0f, (db - floor) / -floor);
        level = std::max (norm, level * 0.85f);   // fast attack, gentle fall
        if (norm >= hold)
        {
            hold = norm;
            holdFrames = 45;
        }
        else if (--holdFrames < 0)
            hold = std::max (0.0f, hold - 0.02f);
        repaint();
    }

    void paint (juce::Graphics& g) override
    {
        auto r = getLocalBounds().toFloat();
        fillSurface (g, r, surfaces::screen);
        g.setColour (colours::ink);
        g.drawRect (r, metrics::hairline);

        const auto inner = r.reduced (3.0f);
        const float gap = 2.0f, segH = (inner.getHeight() - gap * (float) (segments - 1)) / (float) segments;
        const int lit = juce::roundToInt (level * (float) segments);
        const int held = juce::jlimit (0, segments - 1, juce::roundToInt (hold * (float) segments) - 1);
        for (int i = 0; i < segments; ++i)
        {
            const auto seg = juce::Rectangle<float> (inner.getX(), inner.getBottom() - (float) (i + 1) * segH - (float) i * gap,
                                                     inner.getWidth(), segH);
            const bool on = i < lit || (hold > 0.02f && i == held);
            const bool hot = i >= segments - 2;
            g.setColour (on ? (hot ? colours::red : colours::signal) : colours::dim);
            g.fillRect (seg);
        }
    }

private:
    int segments;
    float floor, level = 0.0f, hold = 0.0f;
    int holdFrames = 0;
};
} // namespace onga::ui
