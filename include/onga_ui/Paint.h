#pragma once

#include "Tokens.h"

#include <juce_gui_basics/juce_gui_basics.h>
#include <OngaUiFonts.h>

/*
    Painting primitives: fonts, dithered surfaces, dithered shadows, spaced text and
    readouts. Everything is flat. No gradients, no blur, no bevels.
*/
namespace onga::ui
{
class Fonts
{
public:
    static Fonts& get()
    {
        static Fonts instance;
        return instance;
    }

    /** Space Mono Bold: labels, titles, button text. `px` is the CSS font size. */
    juce::Font bold (float px) const { return juce::Font (juce::FontOptions (boldFace).withPointHeight (px)); }

    /** Space Mono Regular: readouts, captions, screen text. */
    juce::Font regular (float px) const { return juce::Font (juce::FontOptions (regularFace).withPointHeight (px)); }

private:
    Fonts()
        : regularFace (juce::Typeface::createSystemTypefaceFor (OngaUiFonts::SpaceMonoRegular_ttf, OngaUiFonts::SpaceMonoRegular_ttfSize)),
          boldFace (juce::Typeface::createSystemTypefaceFor (OngaUiFonts::SpaceMonoBold_ttf, OngaUiFonts::SpaceMonoBold_ttfSize))
    {
    }

    juce::Typeface::Ptr regularFace, boldFace;
};

//==============================================================================
/** A 2 px checker tile (the suite's dither). Cached per colour pair. */
inline juce::FillType checkerFill (juce::Colour base, juce::Colour dot)
{
    struct Key { juce::uint32 a, b; bool operator== (const Key& o) const { return a == o.a && b == o.b; } };
    static std::vector<std::pair<Key, juce::Image>> cache;

    const Key key { base.getARGB(), dot.getARGB() };
    for (auto& [k, img] : cache)
        if (k == key)
            return juce::FillType (img, {});

    // 8 x 8 tile of 2 x 2 checker cells, 1 logical px each.
    juce::Image img (juce::Image::ARGB, 8, 8, true);
    for (int y = 0; y < 8; ++y)
        for (int x = 0; x < 8; ++x)
            img.setPixelAt (x, y, ((x + y) & 1) == 0 ? dot : base);
    cache.push_back ({ key, img });
    return juce::FillType (img, {});
}

/** Fills a rectangle with a surface: base tone plus the checker. */
inline void fillSurface (juce::Graphics& g, juce::Rectangle<float> r, const Surface& s)
{
    juce::Graphics::ScopedSaveState save (g);
    g.setImageResamplingQuality (juce::Graphics::lowResamplingQuality);
    if (s.dot == s.base)
        g.setColour (s.base);
    else
        g.setFillType (checkerFill (s.base, s.dot));
    g.fillRect (r);
}

/** Fills any path with a 50 % checker of one colour over transparent. */
inline void fillChecker (juce::Graphics& g, const juce::Path& p, juce::Colour c)
{
    juce::Graphics::ScopedSaveState save (g);
    g.setImageResamplingQuality (juce::Graphics::lowResamplingQuality);
    g.setFillType (checkerFill (juce::Colours::transparentBlack, c));
    g.fillPath (p);
}

inline void fillChecker (juce::Graphics& g, juce::Rectangle<float> r, juce::Colour c)
{
    juce::Path p;
    p.addRectangle (r);
    fillChecker (g, p, c);
}

/** The Onga drop shadow: a 50 % ink checker, offset 3 px down-right. Draw it first. */
inline void drawShadow (juce::Graphics& g, juce::Rectangle<float> r)
{
    fillChecker (g, r.translated (metrics::shadow, metrics::shadow), colours::ink);
}

/** A plate: surface fill with a 1 px ink outline drawn inside the bounds. */
inline void drawPlate (juce::Graphics& g, juce::Rectangle<float> r, const Surface& s = surfaces::plate)
{
    fillSurface (g, r, s);
    g.setColour (colours::ink);
    g.drawRect (r, metrics::hairline);
}

//==============================================================================
/** Text with CSS-style letter-spacing (tracking px added after each glyph). */
inline void drawSpaced (juce::Graphics& g, const juce::String& text, const juce::Font& font, float tracking,
                        juce::Rectangle<float> area, juce::Justification just)
{
    juce::GlyphArrangement ga;
    ga.addLineOfText (font, text, 0.0f, 0.0f);
    for (int i = 0; i < ga.getNumGlyphs(); ++i)
        ga.moveRangeOfGlyphs (i, 1, tracking * (float) i, 0.0f);

    // Lay out on the font's line box (not the ink box) so baselines line up across labels.
    const auto ink = ga.getBoundingBox (0, -1, false);
    const auto box = juce::Rectangle<float> (juce::jmax (ink.getWidth(), 1.0f), font.getHeight());
    const auto placed = just.appliedToRectangle (box, area);
    ga.moveRangeOfGlyphs (0, -1, placed.getX() - ink.getX(), placed.getY() + font.getAscent());
    ga.draw (g);
}

inline float spacedWidth (const juce::String& text, const juce::Font& font, float tracking)
{
    return juce::GlyphArrangement::getStringWidth (font, text) + tracking * (float) juce::jmax (0, text.length() - 1);
}

/** Bold caps control label (10 px, 1 px tracking). */
inline void drawLabel (juce::Graphics& g, const juce::String& text, juce::Rectangle<float> area,
                       juce::Justification just = juce::Justification::centredLeft, juce::Colour c = colours::ink)
{
    g.setColour (c);
    drawSpaced (g, text, Fonts::get().bold (type::label), type::labelTracking, area, just);
}

/** The standard readout: ink box, paper text, Space Mono regular. Returns the box used. */
inline juce::Rectangle<float> drawReadout (juce::Graphics& g, juce::Rectangle<float> r, const juce::String& text,
                                           float px = type::readout)
{
    g.setColour (colours::ink);
    g.fillRect (r);
    g.setColour (colours::paper);
    g.setFont (Fonts::get().regular (px));
    g.drawText (text, r, juce::Justification::centred, false);
    return r;
}

/** Width a readout needs: text plus CSS padding, never under minW. */
inline float readoutWidth (const juce::String& text, float px, float padX, float minW)
{
    return juce::jmax (minW, juce::GlyphArrangement::getStringWidth (Fonts::get().regular (px), text) + 2.0f * padX);
}

/** Blue focus ring for keyboard focus. */
inline void drawFocusRing (juce::Graphics& g, juce::Rectangle<float> r)
{
    g.setColour (colours::blue);
    g.drawRect (r.expanded (2.0f), 2.0f);
}
} // namespace onga::ui
