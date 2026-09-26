#pragma once

#include "Paint.h"

#include <functional>
#include <map>

/*
    The logo tile: the plug-in's name in chunky pixel letters on the paper/ink checker,
    inside a 3 px ink frame, with one slow looping animation that hints at what the
    plug-in does.

    A logo is a frame generator. For phase p in [0, 1) it returns four layers, all in the
    tile's 198 x 94 art space:

        outline  ink blocks behind the letters (usually each cell grown by 1-2 px)
        cream    paper-filled cells
        dither   cells filled with the paper/ink checker
        accent   cells in the plug-in's accent colour

    plus optional `under` layers drawn before the outline (echoes, trails, shadows) and
    `over` layers drawn last (sparkles). Give a layer the colour PixelLogo::useAccent() to
    mean "the accent" in `over`, or "50 % ink checker" in `under`.

    Frames are generated once, then stepped (not tweened) so motion stays pixel-crisp.
    Default loop: 40 frames over 4.8 s.
*/
namespace onga::ui
{
struct LogoFrame
{
    juce::Path outline, cream, dither, accent;
    std::vector<std::pair<juce::Path, juce::Colour>> under;   // optional back layers, drawn first, in order
    std::vector<std::pair<juce::Path, juce::Colour>> over;    // optional front layers (sparkles etc.)
};

/** Adds one square pixel cell to a path. */
inline void addCell (juce::Path& p, float x, float y, float size) { p.addRectangle (x, y, size, size); }

//==============================================================================
/** 5 x 7 pixel font for logo words. Rows top to bottom, '1' = filled. */
inline const std::array<const char*, 7>* glyph5x7 (juce::juce_wchar c)
{
    static const std::map<juce::juce_wchar, std::array<const char*, 7>> font {
        { 'A', { "01110", "10001", "10001", "11111", "10001", "10001", "10001" } },
        { 'B', { "11110", "10001", "10001", "11110", "10001", "10001", "11110" } },
        { 'C', { "01111", "10000", "10000", "10000", "10000", "10000", "01111" } },
        { 'D', { "11110", "10001", "10001", "10001", "10001", "10001", "11110" } },
        { 'E', { "11111", "10000", "10000", "11110", "10000", "10000", "11111" } },
        { 'F', { "11111", "10000", "10000", "11110", "10000", "10000", "10000" } },
        { 'G', { "01111", "10000", "10000", "10111", "10001", "10001", "01111" } },
        { 'H', { "10001", "10001", "10001", "11111", "10001", "10001", "10001" } },
        { 'I', { "11111", "00100", "00100", "00100", "00100", "00100", "11111" } },
        { 'K', { "10001", "10010", "10100", "11000", "10100", "10010", "10001" } },
        { 'L', { "10000", "10000", "10000", "10000", "10000", "10000", "11111" } },
        { 'M', { "10001", "11011", "10101", "10101", "10001", "10001", "10001" } },
        { 'N', { "10001", "11001", "10101", "10011", "10001", "10001", "10001" } },
        { 'O', { "01110", "10001", "10001", "10001", "10001", "10001", "01110" } },
        { 'P', { "11110", "10001", "10001", "11110", "10000", "10000", "10000" } },
        { 'Q', { "01110", "10001", "10001", "10001", "10101", "10010", "01101" } },
        { 'R', { "11110", "10001", "10001", "11110", "10100", "10010", "10001" } },
        { 'S', { "01111", "10000", "10000", "01110", "00001", "00001", "11110" } },
        { 'T', { "11111", "00100", "00100", "00100", "00100", "00100", "00100" } },
        { 'U', { "10001", "10001", "10001", "10001", "10001", "10001", "01110" } },
        { 'V', { "10001", "10001", "10001", "10001", "10001", "01010", "00100" } },
        { 'W', { "10001", "10001", "10001", "10101", "10101", "11011", "10001" } },
        { 'X', { "10001", "10001", "01010", "00100", "01010", "10001", "10001" } },
        { 'Y', { "10001", "10001", "01010", "00100", "00100", "00100", "00100" } },
        { 'Z', { "11111", "00001", "00010", "00100", "01000", "10000", "11111" } },
    };
    auto it = font.find (c);
    return it == font.end() ? nullptr : &it->second;
}

/** Calls fn(x, y, column) for each filled cell of `text` set in the 5 x 7 font. */
inline void forEachGlyphCell (const juce::String& text, float x0, float y0, float cell,
                              const std::function<void (float x, float y, int col)>& fn)
{
    int col = 0;
    for (auto ch : text)
    {
        if (auto* g = glyph5x7 (ch))
            for (int r = 0; r < 7; ++r)
                for (int cx = 0; cx < 5; ++cx)
                    if ((*g)[(size_t) r][cx] == '1')
                        fn (x0 + (float) (col + cx) * cell, y0 + (float) r * cell, col + cx);
        col += 6;
    }
}

//==============================================================================
class PixelLogo final : public juce::Component, private juce::Timer
{
public:
    using Generator = std::function<LogoFrame (float phase)>;

    static constexpr float kArtW = 198.0f, kArtH = 94.0f;

    PixelLogo (const juce::String& accessibleName, Generator gen, int numFrames = 40, int loopMs = 4800)
    {
        setTitle (accessibleName);
        frames.reserve ((size_t) numFrames);
        for (int i = 0; i < numFrames; ++i)
            frames.push_back (gen ((float) i / (float) numFrames));
        interval = juce::jmax (16, loopMs / juce::jmax (1, numFrames));
        startTimer (interval);
        setOpaque (true);
    }

    void setAccent (juce::Colour c) { accent = c; repaint(); }

    /** Freeze on frame 0 (e.g. for reduced motion or snapshots). */
    void setAnimating (bool on)
    {
        if (on) { startTimer (interval); return; }
        stopTimer();
        current = 0;
        repaint();
    }

    void paint (juce::Graphics& g) override
    {
        const auto all = getLocalBounds().toFloat();
        const float sx = all.getWidth() / metrics::logoW, sy = all.getHeight() / metrics::logoH;
        const auto inner = all.reduced (metrics::logoFrame * sx, metrics::logoFrame * sy);

        {
            juce::Graphics::ScopedSaveState save (g);
            g.reduceClipRegion (inner.toNearestInt());
            // Art space (198 x 94) sits 1 px inside the tile's top-left corner.
            g.addTransform (juce::AffineTransform::scale (sx, sy).translated (all.getX() + sx, all.getY() + sy));

            fillSurface (g, { -1.0f, -1.0f, kArtW + 2.0f, kArtH + 2.0f }, { colours::paper, colours::ink });

            const auto& f = frames[(size_t) current];
            for (auto& [p, c] : f.under)
            {
                if (c.isTransparent()) fillChecker (g, p, colours::ink);
                else { g.setColour (c); g.fillPath (p); }
            }
            g.setColour (colours::ink);
            g.fillPath (f.outline);
            g.setColour (colours::paper);
            g.fillPath (f.cream);
            {
                juce::Graphics::ScopedSaveState s2 (g);
                g.setImageResamplingQuality (juce::Graphics::lowResamplingQuality);
                g.setFillType (checkerFill (colours::paper, colours::ink));
                g.fillPath (f.dither);
            }
            g.setColour (accent);
            g.fillPath (f.accent);
            for (auto& [p, c] : f.over)
            {
                g.setColour (c.isTransparent() ? accent : c);
                g.fillPath (p);
            }
        }

        g.setColour (colours::ink);
        g.drawRect (all, metrics::logoFrame * sx);
    }

    /** Colour to use in LogoFrame::under/over to mean "the accent" (or checker for under). */
    static juce::Colour useAccent() { return juce::Colours::transparentBlack; }

private:
    void timerCallback() override
    {
        current = (current + 1) % (int) frames.size();
        repaint();
    }

    std::vector<LogoFrame> frames;
    int current = 0, interval = 120;
    juce::Colour accent = colours::red;
};
} // namespace onga::ui
