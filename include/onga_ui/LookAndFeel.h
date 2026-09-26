#pragma once

#include "Paint.h"

/*
    OngaLookAndFeel: the suite style for JUCE's stock widgets (TextButton, ToggleButton,
    ComboBox, PopupMenu, AlertWindow, TextEditor, ProgressBar, tooltips).

    Use it on anything that isn't an onga-ui component: dropdowns, action buttons,
    checkboxes, menus, dialogs.

      - Buttons and dropdowns: paper face, 1 px ink outline, dithered 3 px drop shadow.
        Pressed / toggled-on: ink with an inset paper line (like a selected selector cell).
      - Checkboxes: paper square, ink X.
      - Menus and dialogs: paper, 1 px ink outline, dithered shadow; highlight is ink.
      - Space Mono throughout.

    Scale: components inside an OngaEditor panel are already scaled by the panel
    transform, so leave the scale at 1 for them. Things drawn at window size (popup
    menus, alert windows, full-window overlays) should use a second instance with
    setScale (editorWidth / 820).

    Button properties (set via getProperties()):
      "onga.flat" = true   no drop shadow (e.g. small footer buttons)
*/
namespace onga::ui
{
class OngaLookAndFeel : public juce::LookAndFeel_V4
{
public:
    OngaLookAndFeel()
    {
        using namespace juce;
        setColour (ResizableWindow::backgroundColourId, surfaces::face.base);
        setColour (Label::textColourId, colours::ink);
        setColour (TextButton::buttonColourId, colours::paper);
        setColour (TextButton::textColourOffId, colours::ink);
        setColour (TextButton::textColourOnId, colours::paper);
        setColour (ComboBox::textColourId, colours::ink);
        setColour (ComboBox::backgroundColourId, colours::paper);
        setColour (ComboBox::outlineColourId, colours::ink);
        setColour (PopupMenu::backgroundColourId, colours::paper);
        setColour (PopupMenu::textColourId, colours::ink);
        setColour (PopupMenu::highlightedBackgroundColourId, colours::ink);
        setColour (PopupMenu::highlightedTextColourId, colours::paper);
        setColour (AlertWindow::backgroundColourId, colours::paper);
        setColour (AlertWindow::textColourId, colours::ink);
        setColour (AlertWindow::outlineColourId, colours::ink);
        setColour (TextEditor::backgroundColourId, colours::paper);
        setColour (TextEditor::textColourId, colours::ink);
        setColour (TextEditor::highlightColourId, colours::ink);
        setColour (TextEditor::highlightedTextColourId, colours::paper);
        setColour (TextEditor::outlineColourId, colours::ink);
        setColour (TextEditor::focusedOutlineColourId, colours::blue);
        setColour (CaretComponent::caretColourId, colours::ink);
        setColour (ToggleButton::textColourId, colours::ink);
        setColour (TooltipWindow::backgroundColourId, colours::paper);
        setColour (TooltipWindow::textColourId, colours::ink);
        setColour (TooltipWindow::outlineColourId, colours::ink);
    }

    /** 1.0 at the panel's design size. */
    void setScale (float s) noexcept { scale = std::max (0.25f, s); }
    float getScale() const noexcept { return scale; }

    /** Whole-pixel line thickness at the current scale (never below 1). */
    float line() const noexcept { return std::max (1.0f, std::round (scale)); }
    /** Scaled whole-pixel distance. */
    int px (float designPixels) const noexcept { return std::max (1, juce::roundToInt (designPixels * scale)); }

    /** Space Mono at a design-size height. */
    juce::Font font (float designPx, bool bold = true) const
    {
        return bold ? Fonts::get().bold (designPx * scale) : Fonts::get().regular (designPx * scale);
    }

    //==============================================================================
    // Primitives for custom-drawn overlays and dialogs.

    /** The dithered 3 px drop shadow at the current scale. */
    void drawShadowAt (juce::Graphics& g, juce::Rectangle<int> r) const
    {
        const float o = std::round (metrics::shadow * scale);
        fillChecker (g, r.toFloat().translated (o, o), colours::ink);
    }

    /** Paper box, 1 px ink outline, dithered shadow. */
    void drawFrame (juce::Graphics& g, juce::Rectangle<int> r, bool withShadow = true) const
    {
        if (withShadow)
            drawShadowAt (g, r);
        g.setColour (colours::paper);
        g.fillRect (r);
        g.setColour (colours::ink);
        g.drawRect (r, (int) line());
    }

    /** A plate with its label inside the top-left corner. */
    void drawPanel (juce::Graphics& g, juce::Rectangle<int> r, const juce::String& title) const
    {
        fillSurface (g, r.toFloat(), surfaces::plate);
        g.setColour (colours::ink);
        g.drawRect (r, (int) line());
        if (title.isNotEmpty())
        {
            g.setColour (colours::ink);
            drawSpaced (g, title, font (type::label), type::labelTracking * scale,
                        r.reduced (px (12), px (8)).toFloat().withHeight ((float) px (15)), juce::Justification::centredLeft);
        }
    }

    /** Pinstripes: 1 px lines every 3 px. */
    void drawPinstripes (juce::Graphics& g, juce::Rectangle<int> r) const
    {
        const int t = (int) line();
        g.setColour (colours::ink);
        for (int y = r.getY(); y < r.getBottom(); y += t * 3)
            g.fillRect (r.getX(), y, r.getWidth(), t);
    }

    /** 50 % checker of one colour: the suite's grey-out and dimming. */
    void drawDither (juce::Graphics& g, juce::Rectangle<int> r, juce::Colour c) const
    {
        fillChecker (g, r.toFloat(), c);
    }

    //==============================================================================
    // Stock widget overrides.

    void drawButtonBackground (juce::Graphics& g, juce::Button& b, const juce::Colour&, bool over, bool down) override
    {
        const bool flat = b.getProperties()["onga.flat"];
        const int t = (int) line(), sh = flat ? 0 : (int) std::round (metrics::shadow * scale);
        auto r = b.getLocalBounds().withTrimmedRight (sh).withTrimmedBottom (sh);
        const bool on = b.getToggleState() || down;

        if (! flat)
            drawShadowAt (g, r);
        g.setColour (on ? colours::ink : colours::paper);
        g.fillRect (r);
        if (on && ! flat)
        {
            g.setColour (colours::paper);
            g.drawRect (r.reduced (t * 2), t);   // inset line, like a selected selector cell
        }
        g.setColour (b.hasKeyboardFocus (false) ? colours::blue : colours::ink);
        g.drawRect (r, over && ! on ? t * 2 : t);

        if (! b.isEnabled())
            drawDither (g, r.reduced (t), surfaces::plate.base);
    }

    void drawButtonText (juce::Graphics& g, juce::TextButton& b, bool, bool down) override
    {
        const bool flat = b.getProperties()["onga.flat"];
        const int sh = flat ? 0 : (int) std::round (metrics::shadow * scale);
        auto r = b.getLocalBounds().withTrimmedRight (sh).withTrimmedBottom (sh);
        const bool on = b.getToggleState() || down;
        g.setColour (on ? colours::paper : colours::ink);
        g.setFont (getTextButtonFont (b, b.getHeight()));
        g.drawText (b.getButtonText(), r.reduced (px (4), 0), juce::Justification::centred, false);
    }

    void drawToggleButton (juce::Graphics& g, juce::ToggleButton& b, bool over, bool) override
    {
        const int t = (int) line();
        const int box = px (14);
        juce::Rectangle<int> r (0, (b.getHeight() - box) / 2, box, box);
        g.setColour (colours::paper);
        g.fillRect (r);
        g.setColour (b.hasKeyboardFocus (false) ? colours::blue : colours::ink);
        g.drawRect (r, over ? t * 2 : t);
        if (b.getToggleState())
        {
            g.setColour (colours::ink);
            const auto f = r.reduced (t * 3).toFloat();
            g.drawLine ({ f.getTopLeft(), f.getBottomRight() }, (float) t * 1.5f);
            g.drawLine ({ f.getTopRight(), f.getBottomLeft() }, (float) t * 1.5f);
        }
        g.setColour (colours::ink);
        g.setFont (font (type::label));
        g.drawText (b.getButtonText(), b.getLocalBounds().withTrimmedLeft (box + px (6)), juce::Justification::centredLeft, false);
        if (! b.isEnabled())
            drawDither (g, b.getLocalBounds(), surfaces::plate.base);
    }

    void drawComboBox (juce::Graphics& g, int w, int h, bool, int, int, int, int, juce::ComboBox& cb) override
    {
        const int t = (int) line(), sh = (int) std::round (metrics::shadow * scale);
        juce::Rectangle<int> r (0, 0, w - sh, h - sh);
        drawShadowAt (g, r);
        g.setColour (colours::paper);
        g.fillRect (r);
        g.setColour (cb.hasKeyboardFocus (true) ? colours::blue : colours::ink);
        g.drawRect (r, t);

        // Flat down-triangle, 8 x 5 at design size.
        g.setColour (colours::ink);
        const auto c = juce::Point<float> ((float) r.getRight() - (float) px (12), (float) r.getCentreY());
        juce::Path tri;
        tri.addTriangle (c.x - 4.0f * scale, c.y - 2.5f * scale, c.x + 4.0f * scale, c.y - 2.5f * scale, c.x, c.y + 2.5f * scale);
        g.fillPath (tri);
        if (! cb.isEnabled())
            drawDither (g, r.reduced (t), surfaces::plate.base);
    }

    void positionComboBoxText (juce::ComboBox& cb, juce::Label& l) override
    {
        const int sh = (int) std::round (metrics::shadow * scale);
        l.setBounds (px (4), 0, cb.getWidth() - px (24) - sh, cb.getHeight() - sh);
        l.setFont (getComboBoxFont (cb));
        l.setMinimumHorizontalScale (1.0f);
    }

    juce::Font getComboBoxFont (juce::ComboBox&) override { return font (11.0f, false); }
    juce::Font getTextButtonFont (juce::TextButton&, int) override { return font (type::label); }
    juce::Font getLabelFont (juce::Label&) override { return font (11.0f, false); }
    juce::Font getPopupMenuFont() override { return font (11.0f, false); }
    juce::Font getAlertWindowTitleFont() override { return font (12.0f); }
    juce::Font getAlertWindowMessageFont() override { return font (11.0f, false); }
    juce::Font getAlertWindowFont() override { return font (11.0f, false); }

    void drawPopupMenuBackground (juce::Graphics& g, int w, int h) override
    {
        g.fillAll (colours::paper);
        g.setColour (colours::ink);
        g.drawRect (0, 0, w, h, (int) line());
    }

    int getPopupMenuBorderSize() override { return px (2); }

    void getIdealPopupMenuItemSize (const juce::String& text, bool isSeparator, int, int& w, int& h) override
    {
        h = isSeparator ? px (7) : px (20);
        w = juce::GlyphArrangement::getStringWidthInt (getPopupMenuFont(), text) + px (40);
    }

    void drawPopupMenuSectionHeader (juce::Graphics& g, const juce::Rectangle<int>& area, const juce::String& name) override
    {
        g.setColour (colours::subInk);
        drawSpaced (g, name.toUpperCase(), font (type::caption), type::labelTracking * scale,
                    area.withTrimmedLeft (px (10)).toFloat(), juce::Justification::centredLeft);
    }

    void drawPopupMenuItem (juce::Graphics& g, const juce::Rectangle<int>& area, bool isSeparator, bool isActive,
                            bool isHighlighted, bool isTicked, bool hasSubMenu, const juce::String& text,
                            const juce::String&, const juce::Drawable*, const juce::Colour*) override
    {
        const int t = (int) line();
        if (isSeparator)
        {
            g.setColour (colours::ink);
            for (int x = area.getX() + px (6); x < area.getRight() - px (6); x += t * 2)
                g.fillRect (x, area.getCentreY(), t, t);
            return;
        }
        const bool hl = isHighlighted && isActive;
        if (hl)
        {
            g.setColour (colours::ink);
            g.fillRect (area);
        }
        g.setColour (hl ? colours::paper : colours::ink);
        g.setFont (getPopupMenuFont());
        if (isTicked)
        {
            const auto box = juce::Rectangle<int> (px (6), px (6)).withCentre ({ area.getX() + px (10), area.getCentreY() });
            g.fillRect (box);
        }
        g.drawText (text, area.withTrimmedLeft (px (20)), juce::Justification::centredLeft, true);
        if (hasSubMenu)
        {
            const auto c = juce::Point<float> ((float) (area.getRight() - px (10)), (float) area.getCentreY());
            juce::Path tri;
            tri.addTriangle (c.x - 2.5f * scale, c.y - 4.0f * scale, c.x - 2.5f * scale, c.y + 4.0f * scale, c.x + 2.5f * scale, c.y);
            g.fillPath (tri);
        }
        if (! isActive)
            drawDither (g, area, colours::paper);
    }

    void drawProgressBar (juce::Graphics& g, juce::ProgressBar&, int w, int h, double progress, const juce::String& text) override
    {
        const int t = (int) line();
        juce::Rectangle<int> r (0, 0, w, h);
        g.setColour (colours::paper);
        g.fillRect (r);
        const auto inner = r.reduced (t * 2);
        fillChecker (g, inner.toFloat(), colours::ink);   // the empty track is dithered
        g.setColour (colours::ink);
        g.fillRect (inner.withWidth (juce::roundToInt ((double) inner.getWidth() * juce::jlimit (0.0, 1.0, progress))));
        g.drawRect (r, t);
        if (text.isNotEmpty())
        {
            g.setColour (colours::paper);
            g.setFont (font (type::label));
            g.drawText (text, r, juce::Justification::centred, false);
        }
    }

    void drawAlertBox (juce::Graphics& g, juce::AlertWindow& a, const juce::Rectangle<int>& textArea, juce::TextLayout& layout) override
    {
        const int t = (int) line();
        auto r = a.getLocalBounds();
        g.fillAll (colours::paper);
        g.setColour (colours::ink);
        g.drawRect (r, t);
        layout.draw (g, textArea.toFloat());
    }

    void fillTextEditorBackground (juce::Graphics& g, int w, int h, juce::TextEditor&) override
    {
        g.setColour (colours::paper);
        g.fillRect (0, 0, w, h);
    }

    void drawTextEditorOutline (juce::Graphics& g, int w, int h, juce::TextEditor& e) override
    {
        g.setColour (e.hasKeyboardFocus (true) ? colours::blue : colours::ink);
        g.drawRect (0, 0, w, h, (int) line());
    }

    juce::Rectangle<int> getTooltipBounds (const juce::String& tip, juce::Point<int> pos, juce::Rectangle<int> parent) override
    {
        const auto f = font (type::caption + 1.0f, false);
        const int w = juce::jmin (px (280), juce::GlyphArrangement::getStringWidthInt (f, tip) + px (16));
        const int lines = juce::jmax (1, (juce::GlyphArrangement::getStringWidthInt (f, tip) + px (16)) / juce::jmax (1, w) + 1);
        const int h = px (8) + lines * (int) f.getHeight();
        return juce::Rectangle<int> (pos.x > parent.getCentreX() ? pos.x - (w + 12) : pos.x + 24,
                                     pos.y > parent.getCentreY() ? pos.y - (h + 6) : pos.y + 6, w, h)
            .constrainedWithin (parent);
    }

    void drawTooltip (juce::Graphics& g, const juce::String& text, int w, int h) override
    {
        g.fillAll (colours::paper);
        g.setColour (colours::ink);
        g.drawRect (0, 0, w, h, (int) line());
        g.setFont (font (type::caption + 1.0f, false));
        g.drawFittedText (text, juce::Rectangle<int> (w, h).reduced (px (8), px (4)), juce::Justification::centredLeft, 6);
    }

private:
    float scale = 1.0f;
};
} // namespace onga::ui
