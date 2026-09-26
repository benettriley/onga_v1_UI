#pragma once

#include "Paint.h"

/*
    The screen: the one dark display per plug-in and its focal point. Minimal labels, no
    scale numbers. A title top-left in bold cream, optional header text top-right in ghost
    grey, and a legend bottom-right whose every entry is visible on the screen and named
    after the control it belongs to.
*/
namespace onga::ui
{
/** Screen background (dark checker) with its 1 px ink outline. */
inline void paintScreen (juce::Graphics& g, juce::Rectangle<float> r)
{
    fillSurface (g, r, surfaces::screen);
    g.setColour (colours::ink);
    g.drawRect (r, metrics::hairline);
}

/** Screen title, top-left (x 12, baseline ~24 in the board). */
inline void drawScreenTitle (juce::Graphics& g, juce::Rectangle<float> screen, const juce::String& title)
{
    g.setColour (colours::cream);
    g.setFont (Fonts::get().bold (type::screenTitle));
    g.drawText (title, juce::Rectangle<float> (screen.getX() + 12.0f, screen.getY() + 8.0f, 240.0f, 20.0f),
                juce::Justification::centredLeft, false);
}

/** Header text, top-right, ghost grey (e.g. "TRACK > VOCALS > SILK AIR"). */
inline void drawScreenHeader (juce::Graphics& g, juce::Rectangle<float> screen, const juce::String& text)
{
    g.setColour (colours::ghost);
    g.setFont (Fonts::get().regular (type::screenText));
    g.drawText (text, juce::Rectangle<float> (screen.getRight() - 12.0f - 300.0f, screen.getY() + 8.0f, 300.0f, 18.0f),
                juce::Justification::centredRight, false);
}

struct LegendItem
{
    juce::String name;
    juce::Colour colour;
};

/** Legend, right-aligned so its last entry ends at `right`. Swatch 8 x 8, 4 px to the text, 12 px between entries. */
inline void drawLegend (juce::Graphics& g, float right, float centreY, const std::vector<LegendItem>& items)
{
    const auto font = Fonts::get().regular (type::screenText);
    float x = right;
    for (auto it = items.rbegin(); it != items.rend(); ++it)
    {
        const float tw = juce::GlyphArrangement::getStringWidth (font, it->name);
        x -= tw;
        g.setColour (colours::cream);
        g.setFont (font);
        g.drawText (it->name, juce::Rectangle<float> (x, centreY - 8.0f, tw + 2.0f, 16.0f), juce::Justification::centredLeft, false);
        x -= 4.0f + 8.0f;
        g.setColour (it->colour);
        g.fillRect (juce::Rectangle<float> (x, centreY - 4.0f, 8.0f, 8.0f));
        x -= 12.0f;
    }
}
} // namespace onga::ui
