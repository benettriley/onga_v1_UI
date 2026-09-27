#pragma once

#include "Paint.h"

#include <juce_audio_processors/juce_audio_processors.h>

#include <functional>
#include <vector>

/*
    Selector row: the suite's stepped switch, bound to a choice parameter.

        FOCUS                                 <- label (optional)
          .......................|            <- tick rail: dotted line, selected tick taller
        +-------+-------+-------+#######+     <- buttons on a dithered 3 px drop shadow
        | THUMP | BODY  | SNAP  |[FULL] |        selected = ink with an inset paper line
        +-------+-------+-------+#######+
          80 Hz  250 Hz  2 kHz    ALL         <- captions (optional), selected one bold
*/
namespace onga::ui
{
class SelectorRow final : public juce::Component
{
public:
    /** Bound to a choice parameter. */
    SelectorRow (juce::AudioProcessorValueTreeState& state, const juce::String& paramID, const juce::String& labelText,
                 juce::StringArray buttonNames, juce::StringArray captionsBelow = {})
        : label (labelText), names (std::move (buttonNames)), captions (std::move (captionsBelow))
    {
        attachment = std::make_unique<juce::ParameterAttachment> (*state.getParameter (paramID),
                                                                  [this] (float v) { selected = juce::roundToInt (v); repaint(); },
                                                                  state.undoManager);
        setWantsKeyboardFocus (true);
        setTitle (labelText);
        attachment->sendInitialUpdate();
    }

    /** Not bound to a parameter: a view switch (e.g. which page or screen mode is shown).
        Clicks call onChange; set the state with setSelected(). */
    SelectorRow (const juce::String& labelText, juce::StringArray buttonNames, juce::StringArray captionsBelow = {})
        : label (labelText), names (std::move (buttonNames)), captions (std::move (captionsBelow))
    {
        setWantsKeyboardFocus (true);
        setTitle (labelText);
    }

    /** Called with the new index when the user picks a button (view-switch rows only). */
    std::function<void (int)> onChange;

    void setSelected (int index) { if (index != selected) { selected = index; repaint(); } }

    /** Called after any pick the user makes, in either mode (e.g. to hand control back
        from an automatic mode). */
    std::function<void()> onUserChange;

    /** Replace the button names (e.g. a preset row whose bank changes). */
    void setNames (juce::StringArray newNames) { names = std::move (newNames); repaint(); }
    int getSelected() const noexcept { return selected; }

    static constexpr float kLabelH = 15.0f, kGap = 4.0f, kCaptionH = 13.0f;

    /** Height this row wants, given whether it has a label and captions. */
    static float heightFor (bool hasLabel, bool hasCaptions)
    {
        return (hasLabel ? kLabelH + kGap : 0.0f) + metrics::railH + kGap + metrics::selectorH + metrics::shadow
             + (hasCaptions ? kGap + kCaptionH : 0.0f);
    }

    void setButtonTextSize (float px) { buttonPx = px; repaint(); }

    /** Hide the tick rail (e.g. a big top-row selector that stands on its own). */
    void setShowRail (bool b) { showRail = b; repaint(); }

    /** Buttons stretch to fill the component's height instead of the standard 26 px. */
    void setFillHeight (bool b) { fillHeight = b; repaint(); }

    /** A second, smaller line inside each button (e.g. "TUBE + TRANSFORMER"). */
    void setSubLabels (juce::StringArray subs) { subLabels = std::move (subs); repaint(); }

    /** Grey the whole row out with the plate dither and ignore clicks (e.g. a control
        that doesn't apply to the current mode). The parameter keeps its value. */
    /** Draw a small line icon (in a 22 x 12 box, stroked 1.5 px) instead of each name.
        Names stay as the accessible titles. */
    void setIcons (std::vector<juce::Path> newIcons) { icons = std::move (newIcons); repaint(); }

    void setGreyedOut (bool g) { if (g != greyed) { greyed = g; setWantsKeyboardFocus (! g); repaint(); } }

    void paint (juce::Graphics& g) override
    {
        auto r = getLocalBounds().toFloat();
        if (label.isNotEmpty())
        {
            drawLabel (g, label, r.removeFromTop (kLabelH));
            r.removeFromTop (kGap);
        }
        r.removeFromRight (metrics::shadow);   // room for the shadow

        juce::Rectangle<float> rail;
        if (showRail)
        {
            rail = r.removeFromTop (metrics::railH);
            r.removeFromTop (kGap);
        }
        const float captionRoom = captions.isEmpty() ? 0.0f : kGap + kCaptionH;
        bar = r.removeFromTop (fillHeight ? r.getHeight() - metrics::shadow - captionRoom : metrics::selectorH);
        r.removeFromTop (metrics::shadow);
        const int shown = greyed ? -1 : selected;

        const int n = names.size();
        if (n == 0)
            return;
        const float segW = bar.getWidth() / (float) n;
        const auto centreX = [&] (int i) { return bar.getX() + segW * ((float) i + 0.5f); };

        // Tick rail.
        g.setColour (colours::ink);
        if (showRail)
            for (float x = centreX (0); x < centreX (n - 1); x += 2.0f)
                g.fillRect (juce::Rectangle<float> (std::round (x), rail.getY() + 1.0f, 1.0f, 1.0f));
        for (int i = 0; showRail && i < n; ++i)
        {
            const bool sel = i == shown;
            const float w = sel ? 3.0f : 1.0f, h = sel ? 7.0f : 5.0f;
            g.fillRect (juce::Rectangle<float> (std::round (centreX (i) - w * 0.5f), rail.getBottom() - h, w, h));
        }

        // Shadow, buttons, outline.
        drawShadow (g, bar);
        g.setColour (colours::paper);
        g.fillRect (bar);

        const auto font = Fonts::get().bold (buttonPx);
        for (int i = 0; i < n; ++i)
        {
            auto seg = juce::Rectangle<float> (bar.getX() + segW * (float) i, bar.getY(), segW, bar.getHeight());
            const bool sel = i == shown;
            if (sel)
            {
                g.setColour (colours::ink);
                g.fillRect (seg);
                g.setColour (colours::paper);
                g.drawRect (seg.reduced (2.0f), 1.0f);   // the inset white line
            }
            g.setColour (sel ? colours::paper : colours::ink);
            g.setFont (font);
            if (i < subLabels.size() && subLabels[i].isNotEmpty())
            {
                const float subPx = type::caption, lineH = buttonPx * 1.25f, total = lineH + 4.0f + subPx * 1.3f;
                const float top = seg.getCentreY() - total * 0.5f;
                g.drawText (names[i], juce::Rectangle<float> (seg.getX(), top, seg.getWidth(), lineH), juce::Justification::centred, false);
                g.setColour (sel ? surfaces::plate.base : colours::subInk);
                g.setFont (Fonts::get().regular (subPx));
                g.drawText (subLabels[i], juce::Rectangle<float> (seg.getX(), top + lineH + 4.0f, seg.getWidth(), subPx * 1.3f),
                            juce::Justification::centred, false);
            }
            else if (i < (int) icons.size() && ! icons[(size_t) i].isEmpty())
            {
                const auto box = juce::Rectangle<float> (22.0f, 12.0f).withCentre (seg.getCentre());
                auto icon = icons[(size_t) i];
                icon.applyTransform (juce::AffineTransform::translation (box.getX(), box.getY()));
                g.strokePath (icon, juce::PathStrokeType (1.5f));
            }
            else
                g.drawText (names[i], seg.reduced (2.0f, 0.0f), juce::Justification::centred, false);

            if (i > 0)
            {
                g.setColour (colours::ink);
                g.fillRect (juce::Rectangle<float> (seg.getX(), bar.getY(), metrics::hairline, bar.getHeight()));
            }
        }
        g.setColour (hasKeyboardFocus (false) ? colours::blue : colours::ink);
        g.drawRect (bar, hasKeyboardFocus (false) ? 2.0f : metrics::hairline);

        if (greyed)
            fillChecker (g, bar.reduced (metrics::hairline), surfaces::plate.base);

        // Captions.
        if (! captions.isEmpty())
        {
            r.removeFromTop (kGap);
            const auto caps = r.removeFromTop (kCaptionH);
            g.setColour (colours::subInk);
            for (int i = 0; i < n && i < captions.size(); ++i)
            {
                g.setFont (i == shown ? Fonts::get().bold (type::caption) : Fonts::get().regular (type::caption));
                g.drawText (captions[i], juce::Rectangle<float> (bar.getX() + segW * (float) i, caps.getY(), segW, caps.getHeight()),
                            juce::Justification::centred, false);
            }
        }
    }

    void mouseDown (const juce::MouseEvent& e) override
    {
        if (! greyed && bar.contains (e.position))
            select (juce::jlimit (0, names.size() - 1, (int) ((e.position.x - bar.getX()) / bar.getWidth() * (float) names.size())));
    }

    bool keyPressed (const juce::KeyPress& k) override
    {
        if (greyed) return false;
        if (k == juce::KeyPress::leftKey)  { select (juce::jmax (0, selected - 1)); return true; }
        if (k == juce::KeyPress::rightKey) { select (juce::jmin (names.size() - 1, selected + 1)); return true; }
        return false;
    }

    void focusGained (FocusChangeType) override { repaint(); }
    void focusLost (FocusChangeType) override { repaint(); }

private:
    void select (int i)
    {
        if (i == selected || i < 0 || i >= names.size())
            return;
        if (attachment != nullptr)
            attachment->setValueAsCompleteGesture ((float) i);
        else
        {
            setSelected (i);
            if (onChange)
                onChange (i);
        }
        if (onUserChange)
            onUserChange();
    }

    juce::String label;
    juce::StringArray names, captions, subLabels;
    std::vector<juce::Path> icons;
    std::unique_ptr<juce::ParameterAttachment> attachment;
    juce::Rectangle<float> bar;
    float buttonPx = type::label;
    int selected = 0;
    bool showRail = true, fillHeight = false, greyed = false;
};

//==============================================================================
/** A readout that steps through a choice parameter on click / Enter / Space. */
class StepReadout final : public juce::Component
{
public:
    StepReadout (juce::AudioProcessorValueTreeState& state, const juce::String& id, const juce::String& title)
        : param (*state.getParameter (id)),
          attachment (param, [this] (float) { repaint(); }, state.undoManager)
    {
        attachment.sendInitialUpdate();
        setMouseCursor (juce::MouseCursor::PointingHandCursor);
        setTitle (title);
        setWantsKeyboardFocus (true);
    }

    void paint (juce::Graphics& g) override
    {
        drawReadout (g, getLocalBounds().toFloat(), param.getCurrentValueAsText(), 11.0f);
        if (hasKeyboardFocus (false))
            drawFocusRing (g, getLocalBounds().toFloat().reduced (2.0f));
    }

    void mouseDown (const juce::MouseEvent&) override { step(); }
    bool keyPressed (const juce::KeyPress& k) override
    {
        if (k == juce::KeyPress::returnKey || k == juce::KeyPress::spaceKey) { step(); return true; }
        return false;
    }
    void focusGained (FocusChangeType) override { repaint(); }
    void focusLost (FocusChangeType) override { repaint(); }

private:
    void step()
    {
        const int n = param.getNumSteps();
        const int current = juce::roundToInt (param.convertFrom0to1 (param.getValue()));
        attachment.setValueAsCompleteGesture ((float) ((current + 1) % n));
    }

    juce::RangedAudioParameter& param;
    juce::ParameterAttachment attachment;
};
} // namespace onga::ui
