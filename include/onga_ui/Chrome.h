#pragma once

#include "Paint.h"

/*
    Panel chrome: the faceplate, the pinstriped title bar and the footer.
    There is no close box, no menu bar and no window furniture: the title bar is just the
    plug-in's name between two sets of stripes.
*/
namespace onga::ui
{
/** Faceplate fill plus the 1 px outer outline. */
inline void paintFaceplate (juce::Graphics& g, juce::Rectangle<float> all)
{
    fillSurface (g, all, surfaces::face);
    g.setColour (colours::ink);
    g.drawRect (all, metrics::hairline);
}

/** Title bar: bar surface, 1 px rule under it, name centred between pinstripes. */
inline void paintTitleBar (juce::Graphics& g, juce::Rectangle<float> r, const juce::String& name)
{
    fillSurface (g, r, surfaces::bar);
    g.setColour (colours::ink);
    g.fillRect (r.withTop (r.getBottom() - metrics::hairline));

    const auto font = Fonts::get().bold (type::title);
    const float nameW = spacedWidth (name, font, type::heroTracking) + 2.0f * 8.0f;    // text + 8 px padding each side
    const auto inner = r.withTrimmedBottom (metrics::hairline).reduced (8.0f, 0.0f);   // 8 px side padding
    const auto nameArea = juce::Rectangle<float> (nameW, inner.getHeight()).withCentre (inner.getCentre());

    // Stripes: 1 px lines every 3 px, 11 px tall, with an 8 px gap either side of the name.
    const float stripeTop = std::round (inner.getCentreY() - 5.5f);
    for (int i = 0; i < 4; ++i)
    {
        const float y = stripeTop + 3.0f * (float) i;
        g.fillRect (juce::Rectangle<float>::leftTopRightBottom (inner.getX(), y, nameArea.getX() - 8.0f, y + 1.0f));
        g.fillRect (juce::Rectangle<float>::leftTopRightBottom (nameArea.getRight() + 8.0f, y, inner.getRight(), y + 1.0f));
    }

    drawSpaced (g, name, font, type::heroTracking, nameArea, juce::Justification::centred);
}

/** Footer: bar surface, 1 px rule above, brand on the left, status text on the right. */
inline void paintFooter (juce::Graphics& g, juce::Rectangle<float> r, const juce::String& right,
                         const juce::String& left = "ONGA TOOLS")
{
    fillSurface (g, r, surfaces::bar);
    g.setColour (colours::ink);
    g.fillRect (r.withHeight (metrics::hairline));
    g.setFont (Fonts::get().regular (type::footer));
    const auto text = r.withTrimmedTop (metrics::hairline).reduced (10.0f, 0.0f);
    g.drawText (left, text, juce::Justification::centredLeft, false);
    g.drawText (right, text, juce::Justification::centredRight, false);
}

/** " · " with proper spacing, for footer strings. */
inline juce::String dot() { return juce::String (" ") + juce::String::charToString ((juce::juce_wchar) 0x00b7) + " "; }
} // namespace onga::ui
