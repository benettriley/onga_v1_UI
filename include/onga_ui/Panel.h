#pragma once

#include "Tokens.h"

#include <juce_audio_processors/juce_audio_processors.h>

/*
    OngaEditor: every Onga panel is laid out once at 820 x 580 design pixels and scaled
    as a unit, so line weights, dither and pixel type keep their proportions. Subclass
    nothing: construct it with your panel component.

        OngaEditor (processor, std::make_unique<MyPanel> (processor))
*/
namespace onga::ui
{
class OngaEditor : public juce::AudioProcessorEditor
{
public:
    OngaEditor (juce::AudioProcessor& p, std::unique_ptr<juce::Component> panelToOwn,
                int designW = metrics::panelW, int designH = metrics::panelH)
        : juce::AudioProcessorEditor (p), panel (std::move (panelToOwn)), w (designW), h (designH)
    {
        addAndMakeVisible (*panel);
        panel->setBounds (0, 0, w, h);
        constrainer.setFixedAspectRatio ((double) w / (double) h);
        constrainer.setSizeLimits (w * 3 / 4, h * 3 / 4, w * 2, h * 2);
        setConstrainer (&constrainer);
        setResizable (true, true);
        setSize (w, h);
    }

    void paint (juce::Graphics&) override {}
    void resized() override { panel->setTransform (juce::AffineTransform::scale ((float) getWidth() / (float) w)); }

    juce::Component& getPanel() noexcept { return *panel; }

private:
    std::unique_ptr<juce::Component> panel;
    int w, h;
    juce::ComponentBoundsConstrainer constrainer;
};
} // namespace onga::ui
