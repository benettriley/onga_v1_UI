#pragma once

#include "Paint.h"

#include <juce_audio_processors/juce_audio_processors.h>

/*
    Dials. Every dial face is a flat filled ink disc with a thin pointer. Nothing else on
    the face: no knurling, no rings, no highlight.

      - HeroDial: one per plug-in, the macro control. Red pointer, red value arc on a
        dotted track, minor/major scale ticks and 0 / 5 / 10 numerals.
      - SmallDial: every other continuous control. White pointer, 7 plain ticks.

    Both take their angles from -135 deg to +135 deg (0 = straight up).
*/
namespace onga::ui
{
namespace detail
{
    /** Point on a circle, angle in degrees clockwise from 12 o'clock. */
    inline juce::Point<float> polar (juce::Point<float> c, float r, float deg)
    {
        const float a = juce::degreesToRadians (deg);
        return { c.x + r * std::sin (a), c.y - r * std::cos (a) };
    }

    inline juce::Path arc (juce::Point<float> c, float r, float fromDeg, float toDeg)
    {
        juce::Path p;
        p.addCentredArc (c.x, c.y, r, r, 0.0f, juce::degreesToRadians (fromDeg), juce::degreesToRadians (toDeg), true);
        return p;
    }

    /** Paints the hero dial into a square area (design size 184 x 184). */
    inline void paintHero (juce::Graphics& g, juce::Rectangle<float> area, float pos, juce::Colour accent, juce::Colour faceFill)
    {
        const float s = area.getWidth() / metrics::heroDial;   // everything below is in 184-unit space
        const auto c = area.getTopLeft() + juce::Point<float> (92.0f, 92.0f) * s;
        const float value = -135.0f + 270.0f * juce::jlimit (0.0f, 1.0f, pos);

        // Dotted track (1 on / 3 off) and the red value arc on top of it.
        {
            juce::Path track, dotted;
            track = arc (c, 57.0f * s, -135.0f, 135.0f);
            const float dashes[] = { 1.0f * s, 3.0f * s };
            juce::PathStrokeType (1.0f * s).createDashedStroke (dotted, track, dashes, 2);
            g.setColour (colours::ink);
            g.fillPath (dotted);
        }
        if (pos > 0.0f)
        {
            g.setColour (accent);
            g.strokePath (arc (c, 57.0f * s, -135.0f, value), juce::PathStrokeType (4.0f * s));
        }

        // Scale: 21 ticks, majors at 0 / 5 / 10.
        g.setColour (colours::ink);
        for (int i = 0; i <= 20; ++i)
        {
            const float d = -135.0f + 13.5f * (float) i;
            if (i % 10 == 0)
                g.drawLine ({ polar (c, 62.0f * s, d), polar (c, 74.0f * s, d) }, 3.0f * s);
            else
                g.drawLine ({ polar (c, 64.0f * s, d), polar (c, (i % 2 == 0 ? 71.0f : 68.0f) * s, d) }, 1.5f * s);
        }

        g.setFont (Fonts::get().bold (12.0f * s));
        for (int i = 0; i <= 2; ++i)
        {
            const float d = -135.0f + 135.0f * (float) i;
            const auto p = polar (c, 82.0f * s, d);
            g.drawText (juce::String (i * 5), juce::Rectangle<float> (28.0f * s, 16.0f * s).withCentre (p.translated (0.0f, 0.0f)),
                        juce::Justification::centred, false);
        }

        // Face: flat disc with a 3 px ink outline, then the pointer.
        const auto disc = juce::Rectangle<float> (100.0f * s, 100.0f * s).withCentre (c);
        g.setColour (faceFill);
        g.fillEllipse (disc);
        g.setColour (colours::ink);
        g.drawEllipse (disc, 3.0f * s);
        g.setColour (accent);
        g.drawLine ({ c, polar (c, 46.0f * s, value) }, 3.0f * s);
    }

    /** Paints a small dial into a square area (design size 44 x 44, drawn in 64-unit space). */
    inline void paintSmall (juce::Graphics& g, juce::Rectangle<float> area, float pos, int ticks, juce::Colour faceFill,
                            int hintTick = -1)
    {
        const float s = area.getWidth() / 64.0f;
        const auto c = area.getCentre();
        for (int i = 0; i < ticks; ++i)
        {
            const float d = -135.0f + 270.0f * (float) i / (float) juce::jmax (1, ticks - 1);
            const bool hint = i == hintTick;
            g.setColour (hint ? colours::red : colours::ink);
            g.drawLine ({ polar (c, 26.0f * s, d), polar (c, (hint ? 32.0f : 30.0f) * s, d) }, (hint ? 3.0f : 2.0f) * s);
        }
        const auto disc = juce::Rectangle<float> (42.0f * s, 42.0f * s).withCentre (c);
        g.setColour (faceFill);
        g.fillEllipse (disc);
        g.setColour (colours::ink);
        g.drawEllipse (disc, 2.0f * s);
        g.setColour (colours::paper);
        g.drawLine ({ c, polar (c, 19.0f * s, -135.0f + 270.0f * juce::jlimit (0.0f, 1.0f, pos)) }, 2.0f * s);
    }
}

//==============================================================================
/** Shared behaviour: a rotary slider bound to one parameter, label above, readout below. */
class DialBase : public juce::Component
{
public:
    DialBase (juce::AudioProcessorValueTreeState& state, const juce::String& paramID, const juce::String& labelText, int dragPixels)
        : param (*state.getParameter (paramID)), label (labelText)
    {
        slider.setSliderStyle (juce::Slider::RotaryHorizontalVerticalDrag);
        slider.setTextBoxStyle (juce::Slider::NoTextBox, false, 0, 0);
        slider.setRotaryParameters (juce::degreesToRadians (-135.0f), juce::degreesToRadians (135.0f), true);
        slider.setMouseDragSensitivity (dragPixels);
        slider.setWantsKeyboardFocus (true);
        slider.setTitle (labelText);
        slider.setLookAndFeel (&lnf);
        slider.onValueChange = [this]
        {
            // Detents: snap while dragging when close to a marked value.
            if (! detents.empty() && slider.isMouseButtonDown())
            {
                const double span = slider.getMaximum() - slider.getMinimum();
                for (auto d : detents)
                {
                    const double dist = std::abs (slider.getValue() - d);
                    if (dist < snapRange * span && dist > 1.0e-9)
                    {
                        slider.setValue (d, juce::sendNotificationSync);
                        break;
                    }
                }
            }
            repaint();
        };
        slider.onDragStart = [this] { if (onUserChange) onUserChange(); };
        addAndMakeVisible (slider);
        attachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment> (state, paramID, slider);
        slider.setDoubleClickReturnValue (true, param.convertFrom0to1 (param.getDefaultValue()));
    }

    ~DialBase() override { slider.setLookAndFeel (nullptr); }

    /** The dial face colour (ink by default). */
    void setFaceColour (juce::Colour c) { face = c; repaint(); }
    void setAccent (juce::Colour c) { accent = c; repaint(); }

    /** Values (in parameter units) the dial snaps to while dragging, within snapRange of the full range. */
    void setDetents (std::vector<double> values, double range = 0.03) { detents = std::move (values); snapRange = range; }

    /** Show the readout text in capitals (for word values like NORMAL). */
    void setUppercaseValue (bool b) { upperValue = b; repaint(); }

    /** Called when the user starts dragging. */
    std::function<void()> onUserChange;

    juce::Slider& getSlider() noexcept { return slider; }

protected:
    float position() { return (float) slider.valueToProportionOfLength (slider.getValue()); }
    juce::String valueText() const
    {
        const auto t = param.getCurrentValueAsText();
        return upperValue ? t.toUpperCase() : t;
    }

    /** The slider draws nothing: the component paints the dial so it can sit in a
        design-space layout. The slider just owns mouse, keyboard and host automation. */
    struct Invisible final : juce::LookAndFeel_V4
    {
        void drawRotarySlider (juce::Graphics&, int, int, int, int, float, float, float, juce::Slider&) override {}
    };

    juce::RangedAudioParameter& param;
    juce::String label;
    Invisible lnf;
    juce::Slider slider;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> attachment;
    juce::Colour face = colours::ink, accent = colours::red;
    std::vector<double> detents;
    double snapRange = 0.03;
    bool upperValue = false;
};

//==============================================================================
/** The one hero dial. Lay it out at least 184 wide x 246 tall (label + dial + readout). */
class HeroDial final : public DialBase
{
public:
    HeroDial (juce::AudioProcessorValueTreeState& state, const juce::String& paramID, const juce::String& labelText)
        : DialBase (state, paramID, labelText, 320) {}

    static constexpr float kLabelH = 17.0f, kGap = 10.0f, kReadH = 25.0f;
    static constexpr float kHeight = kLabelH + kGap + metrics::heroDial + kGap + kReadH;

    void paint (juce::Graphics& g) override
    {
        const auto d = dialArea();
        g.setColour (colours::ink);
        drawSpaced (g, label, Fonts::get().bold (type::heroLabel), type::heroTracking,
                    { 0.0f, d.getY() - kGap - kLabelH, (float) getWidth(), kLabelH }, juce::Justification::centred);

        detail::paintHero (g, d, position(), accent, face);

        const auto text = valueText();
        const float w = readoutWidth (text, type::heroReadout, 8.0f, 64.0f);
        const auto box = juce::Rectangle<float> (w, kReadH).withCentre ({ d.getCentreX(), d.getBottom() + kGap + kReadH * 0.5f });
        drawReadout (g, box, text, type::heroReadout);

        if (slider.hasKeyboardFocus (false))
            drawFocusRing (g, box);
    }

    void resized() override { slider.setBounds (dialArea().reduced (20.0f).toNearestInt()); }

private:
    juce::Rectangle<float> dialArea() const
    {
        const float top = ((float) getHeight() - kHeight) * 0.5f + kLabelH + kGap;
        return juce::Rectangle<float> (metrics::heroDial, metrics::heroDial).withCentre ({ (float) getWidth() * 0.5f, top + metrics::heroDial * 0.5f });
    }
};

//==============================================================================
/*
    Small dial with label above and readout below. Natural size 80 x 86 at the default
    44 px dial. Options: a different dial size, tick count, words either side of the dial
    (e.g. CLOSE / FAR, bottom-aligned in 9 px caption type) and a highlighted "hint" tick.
*/
class SmallDial final : public DialBase
{
public:
    SmallDial (juce::AudioProcessorValueTreeState& state, const juce::String& paramID, const juce::String& labelText, int numTicks = 7)
        : DialBase (state, paramID, labelText, 220), ticks (numTicks) {}

    static constexpr float kLabelH = 15.0f, kGap = 4.0f, kReadH = 19.0f;
    static constexpr float kHeight = kLabelH + kGap + metrics::smallDial + kGap + kReadH;

    /** Height for a given dial size (kHeight is for the default 44 px). */
    static constexpr float heightFor (float dialSize) { return kLabelH + kGap + dialSize + kGap + kReadH; }

    void setDialSize (float px) { dial = px; resized(); repaint(); }
    void setEndLabels (const juce::String& left, const juce::String& right) { leftWord = left; rightWord = right; repaint(); }
    void setHintTick (int index) { if (hint != index) { hint = index; repaint(); } }
    void setShowLabel (bool b) { showLabel = b; resized(); repaint(); }

    void paint (juce::Graphics& g) override
    {
        const auto d = dialArea();
        if (showLabel)
            drawLabel (g, label, { 0.0f, d.getY() - kGap - kLabelH, (float) getWidth(), kLabelH }, juce::Justification::centred);
        detail::paintSmall (g, d, position(), ticks, face, hint);

        if (leftWord.isNotEmpty() || rightWord.isNotEmpty())
        {
            g.setColour (colours::subInk);
            g.setFont (Fonts::get().regular (type::caption));
            const auto row = juce::Rectangle<float> (0.0f, d.getBottom() - 13.0f, (float) getWidth(), 13.0f);
            g.drawText (leftWord, row.withRight (d.getX() - 4.0f), juce::Justification::centredRight, false);
            g.drawText (rightWord, row.withLeft (d.getRight() + 4.0f), juce::Justification::centredLeft, false);
        }

        const auto text = valueText();
        const float w = readoutWidth (text, type::readout, 6.0f, dial > metrics::smallDial ? 72.0f : 56.0f);
        const auto box = juce::Rectangle<float> (w, kReadH).withCentre ({ d.getCentreX(), d.getBottom() + kGap + kReadH * 0.5f });
        drawReadout (g, box, text);

        if (slider.hasKeyboardFocus (false))
            drawFocusRing (g, box);
    }

    void resized() override { slider.setBounds (dialArea().toNearestInt()); }

private:
    juce::Rectangle<float> dialArea() const
    {
        const float labelRoom = showLabel ? kLabelH + kGap : 0.0f;
        const float total = labelRoom + dial + kGap + kReadH;
        const float top = ((float) getHeight() - total) * 0.5f + labelRoom;
        return juce::Rectangle<float> (dial, dial).withCentre ({ (float) getWidth() * 0.5f, top + dial * 0.5f });
    }

    int ticks, hint = -1;
    float dial = metrics::smallDial;
    bool showLabel = true;
    juce::String leftWord, rightWord;
};
} // namespace onga::ui
