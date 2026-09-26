#pragma once

#include "Paint.h"

#include <juce_audio_processors/juce_audio_processors.h>

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
    SelectorRow (juce::AudioProcessorValueTreeState& state, const juce::String& paramID, const juce::String& labelText,
                 juce::StringArray buttonNames, juce::StringArray captionsBelow = {})
        : param (*state.getParameter (paramID)), label (labelText), names (std::move (buttonNames)), captions (std::move (captionsBelow)),
          attachment (param, [this] (float v) { selected = juce::roundToInt (v); repaint(); }, state.undoManager)
    {
        setWantsKeyboardFocus (true);
        setTitle (labelText);
        attachment.sendInitialUpdate();
    }

    static constexpr float kLabelH = 15.0f, kGap = 4.0f, kCaptionH = 13.0f;

    /** Height this row wants, given whether it has a label and captions. */
    static float heightFor (bool hasLabel, bool hasCaptions)
    {
        return (hasLabel ? kLabelH + kGap : 0.0f) + metrics::railH + kGap + metrics::selectorH + metrics::shadow
             + (hasCaptions ? kGap + kCaptionH : 0.0f);
    }

    void setButtonTextSize (float px) { buttonPx = px; repaint(); }

    void paint (juce::Graphics& g) override
    {
        auto r = getLocalBounds().toFloat();
        if (label.isNotEmpty())
        {
            drawLabel (g, label, r.removeFromTop (kLabelH));
            r.removeFromTop (kGap);
        }
        r.removeFromRight (metrics::shadow);   // room for the shadow

        const auto rail = r.removeFromTop (metrics::railH);
        r.removeFromTop (kGap);
        bar = r.removeFromTop (metrics::selectorH);
        r.removeFromTop (metrics::shadow);

        const int n = names.size();
        const float segW = bar.getWidth() / (float) n;
        const auto centreX = [&] (int i) { return bar.getX() + segW * ((float) i + 0.5f); };

        // Tick rail.
        g.setColour (colours::ink);
        for (float x = centreX (0); x < centreX (n - 1); x += 2.0f)
            g.fillRect (juce::Rectangle<float> (std::round (x), rail.getY() + 1.0f, 1.0f, 1.0f));
        for (int i = 0; i < n; ++i)
        {
            const bool sel = i == selected;
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
            if (i == selected)
            {
                g.setColour (colours::ink);
                g.fillRect (seg);
                g.setColour (colours::paper);
                g.drawRect (seg.reduced (2.0f), 1.0f);   // the inset white line
            }
            g.setColour (i == selected ? colours::paper : colours::ink);
            g.setFont (font);
            g.drawText (names[i], seg.reduced (2.0f, 0.0f), juce::Justification::centred, false);

            if (i > 0)
            {
                g.setColour (colours::ink);
                g.fillRect (juce::Rectangle<float> (seg.getX(), bar.getY(), metrics::hairline, bar.getHeight()));
            }
        }
        g.setColour (hasKeyboardFocus (false) ? colours::blue : colours::ink);
        g.drawRect (bar, hasKeyboardFocus (false) ? 2.0f : metrics::hairline);

        // Captions.
        if (! captions.isEmpty())
        {
            r.removeFromTop (kGap);
            const auto caps = r.removeFromTop (kCaptionH);
            g.setColour (colours::subInk);
            for (int i = 0; i < n && i < captions.size(); ++i)
            {
                g.setFont (i == selected ? Fonts::get().bold (type::caption) : Fonts::get().regular (type::caption));
                g.drawText (captions[i], juce::Rectangle<float> (bar.getX() + segW * (float) i, caps.getY(), segW, caps.getHeight()),
                            juce::Justification::centred, false);
            }
        }
    }

    void mouseDown (const juce::MouseEvent& e) override
    {
        if (bar.contains (e.position))
            select (juce::jlimit (0, names.size() - 1, (int) ((e.position.x - bar.getX()) / bar.getWidth() * (float) names.size())));
    }

    bool keyPressed (const juce::KeyPress& k) override
    {
        if (k == juce::KeyPress::leftKey)  { select (juce::jmax (0, selected - 1)); return true; }
        if (k == juce::KeyPress::rightKey) { select (juce::jmin (names.size() - 1, selected + 1)); return true; }
        return false;
    }

    void focusGained (FocusChangeType) override { repaint(); }
    void focusLost (FocusChangeType) override { repaint(); }

private:
    void select (int i)
    {
        if (i != selected)
            attachment.setValueAsCompleteGesture ((float) i);
    }

    juce::RangedAudioParameter& param;
    juce::String label;
    juce::StringArray names, captions;
    juce::ParameterAttachment attachment;
    juce::Rectangle<float> bar;
    float buttonPx = type::label;
    int selected = 0;
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
