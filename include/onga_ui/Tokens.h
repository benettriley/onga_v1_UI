#pragma once

#include <juce_graphics/juce_graphics.h>

/*
    Onga Tools design tokens. Every plug-in panel pulls its colours and sizes from here,
    so the suite stays one family. See ONGA_UI.md for the rules behind them.

    Surfaces are warm greys, each a flat base tone with a 2 px checker of a close tone on
    top ("dither"). Depth order, darkest to lightest:

        face (faceplate)  ->  plate (dial plates)  ->  bar (title bar / footer)  ->  paper (buttons)
*/
namespace onga::ui
{
struct Surface
{
    juce::Colour base, dot;   // dot == base means "solid"
};

namespace colours
{
    // Ink and paper
    inline const juce::Colour ink      { 0xff141413 };   // outlines, text, dial faces, selected buttons
    inline const juce::Colour paper    { 0xfffaf9f5 };   // buttons, knob paper, readout text
    inline const juce::Colour cream    { 0xfff4f2ec };   // text on screens
    inline const juce::Colour subInk   { 0xff2e2d29 };   // captions under selectors, units

    // Screen palette
    inline const juce::Colour signal   { 0xffe9e6df };   // main data on a screen (bars, set curves)
    inline const juce::Colour ghost    { 0xff8c8980 };   // secondary data, header text on screens
    inline const juce::Colour rule     { 0xff5a5852 };   // floor / zero lines on screens
    inline const juce::Colour dim      { 0xff3a3934 };   // inactive data (gated, background spectrum)

    // Accents. Red is the one hero accent; amber and blue are data colours on screens.
    inline const juce::Colour red      { 0xffd9432b };   // hero pointer + value arc, "shaved" / dynamic data
    inline const juce::Colour amber    { 0xffe3a33b };
    inline const juce::Colour blue     { 0xff5b82e6 };   // also the keyboard-focus ring
}

namespace surfaces
{
    inline const Surface face   { juce::Colour (0xffc2bdb1), juce::Colour (0xffb1ab9e) };
    inline const Surface plate  { juce::Colour (0xffd8d4ca), juce::Colour (0xffbdb7aa) };
    inline const Surface bar    { juce::Colour (0xfff4f2ec), juce::Colour (0xffdcd8ce) };
    inline const Surface paper  { colours::paper, colours::paper };
    inline const Surface screen { juce::Colour (0xff121211), juce::Colour (0xff20201d) };
}

namespace metrics
{
    // Panel
    inline constexpr int   panelW = 820, panelH = 580;   // design size; the editor scales it as a unit
    inline constexpr float titleH = 26.0f, footerH = 22.0f;
    inline constexpr float pad = 14.0f, gap = 12.0f;

    // Lines
    inline constexpr float hairline = 1.0f;     // panel, plate and button outlines
    inline constexpr float logoFrame = 3.0f;    // the logo tile frame
    inline constexpr float shadow = 3.0f;       // dithered drop shadow offset

    // Logo tile
    inline constexpr float logoW = 200.0f, logoH = 96.0f;

    // Controls
    inline constexpr float selectorH = 26.0f;   // selector row button height
    inline constexpr float railH = 7.0f;        // tick rail above a selector row
    inline constexpr float smallDial = 44.0f;   // small dial drawn size
    inline constexpr float heroDial = 184.0f;   // hero dial drawn size (incl. scale + numerals)
}

namespace type
{
    // Space Mono, sizes in CSS px (= JUCE point height).
    inline constexpr float label = 10.0f;       // bold caps, 1 px tracking
    inline constexpr float labelTracking = 1.0f;
    inline constexpr float heroLabel = 12.0f;   // bold caps, 2 px tracking
    inline constexpr float heroTracking = 2.0f;
    inline constexpr float readout = 10.0f;     // regular, on ink
    inline constexpr float heroReadout = 13.0f;
    inline constexpr float caption = 9.0f;      // regular, subInk
    inline constexpr float screenTitle = 14.0f; // bold, cream
    inline constexpr float screenText = 10.0f;  // legend + header text on screens
    inline constexpr float title = 12.0f;       // title bar name, bold, 2 px tracking
    inline constexpr float footer = 10.0f;
}
} // namespace onga::ui
