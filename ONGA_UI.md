# ONGA UI — the Onga Tools panel spec

One look for every Onga Tools plug-in. This file is the source of truth; the C++ in
`include/onga_ui/` implements it. If the two disagree, fix the code.

Reference build: **ONGA BLOOM** ([onga_bloom](https://github.com/benettriley/onga_bloom)). Board designs for all five plug-ins live in the
*Onga Anchor Directions* design canvas (Suite v1 page).

---

## 1. Feel

- **System 7 flavour, legibility first.** Texture, not costume.
- **No** fake windows, menu bars, close boxes, OS chrome, knurling, bevels or faux 3D.
- **Warm greys with checker dither everywhere**, but never where it costs legibility:
  text, readouts, pointers and selected states stay solid.
- **One dark screen** per plug-in is the focal point.
- **Thin lines.** Small nudges, not big swings.

## 2. Panel grid

All panels are designed at **820 × 580** and scaled as a unit (`onga::ui::OngaEditor`,
aspect locked, 75 %–200 %). A plug-in with a lot of parameters may go taller at the same
width (Voxmaster is 820 × 700); keep the width so the suite lines up.

| Zone | Size |
|---|---|
| Outer outline | 1 px ink |
| Title bar | 26 px (incl. 1 px rule under it) |
| Footer | 22 px (incl. 1 px rule over it) |
| Body padding | 14 px |
| Gap between blocks | 12 px |
| Logo tile | 200 × 96, always top-left of the body |

Typical body: a 200 px left column (logo over a plate), the screen in the middle, and a
plate column or plate row for the remaining controls. The logo is always top-left.

## 3. Colour

### Surfaces — base tone + 2 px checker of a close tone

| Token | Base | Dot | Use |
|---|---|---|---|
| `surfaces::face` | `#C2BDB1` | `#B1AB9E` | faceplate (darkest) |
| `surfaces::plate` | `#D8D4CA` | `#BDB7AA` | dial plates / group boxes |
| `surfaces::bar` | `#F4F2EC` | `#DCD8CE` | title bar, footer |
| `surfaces::paper` | `#FAF9F5` | solid | buttons, selector cells |
| `surfaces::screen` | `#121211` | `#20201D` | the dark screen |

Order, darkest to lightest: **faceplate → plates → buttons**.

### Ink and data

| Token | Hex | Use |
|---|---|---|
| `ink` | `#141413` | outlines, text, dial faces, selected buttons, readout boxes |
| `paper` | `#FAF9F5` | readout text, pointers on small dials |
| `cream` | `#F4F2EC` | text on screens |
| `subInk` | `#2E2D29` | captions under selectors, units |
| `signal` | `#E9E6DF` | main data on a screen |
| `ghost` | `#8C8980` | secondary screen data, screen header text |
| `rule` | `#5A5852` | floor / zero lines on screens |
| `dim` | `#3A3934` | inactive screen data (gated, background) |

### Accents

| Token | Hex | Use |
|---|---|---|
| `red` | `#D9432B` | **the** accent: hero pointer + value arc, "shaved"/dynamic data, logo highlights |
| `amber` | `#E3A33B` | screen data only |
| `blue` | `#5B82E6` | screen data only, and the keyboard-focus ring |

Red only appears on the hero dial, on screen data and in the logo.

## 4. Type

Space Mono (SIL OFL, bundled). Sizes are CSS px (= JUCE point height).

| Role | Face | Size | Tracking |
|---|---|---|---|
| Control label | Bold caps | 10 | 1 px |
| Hero label | Bold caps | 12 | 2 px |
| Title bar name | Bold caps | 12 | 2 px |
| Button text | Bold caps | 10 (13 in big selector rows) | 0 |
| Readout | Regular, paper on ink | 10 (hero 13, status 11) | 0 |
| Caption | Regular (selected: bold), subInk | 9 | 0 |
| Screen title | Bold, cream | 14 | 0 |
| Screen text / legend | Regular, cream | 10 | 0 |
| Footer | Regular | 10 | 0 |

## 5. Components

### Title bar — `paintTitleBar (g, area, "ONGA NAME")`
Bar surface. Name centred, 8 px padding, with four 1 px pinstripes (every 3 px, 11 px
tall) filling each side. Nothing else. No close box.

### Footer — `paintFooter (g, area, rightText)`
Bar surface. `ONGA TOOLS` left; right side is `vX.Y.Z · 48 kHz · LATENCY n SMP` or the
plug-in's own one-line status.

### Plate — `drawPlate (g, rect)`
Plate surface, 1 px ink outline. Group label (10 px bold caps) top-left inside, 8–12 px
padding.

### Drop shadow — `drawShadow (g, rect)`
50 % ink checker offset 3 px down-right, drawn before the thing it sits under. Used on
selector rows and standalone buttons. **Never solid, never blurred.**

### Hero dial — `HeroDial`
One per plug-in: its macro control. 184 × 184 design px.
- Flat ink face r = 50 with a 3 px ink outline. Nothing on the face.
- Red pointer, 3 px, centre → r 46.
- Dotted track r 57 (1 on / 3 off), red value arc on it, 4 px.
- 21 scale ticks: minor r 64→68/71 at 1.5 px, major (0 / 5 / 10) r 62→74 at 3 px.
- Numerals 0 / 5 / 10 at r 82, 12 px bold.
- Label above (12 px bold, 2 px tracking), readout below (13 px, min 64 wide).

The hero also takes `setDialSize` (the whole dial scales, numerals included) when a
row is shorter than 246 px.

### Small dial — `SmallDial`
Every other continuous control. 44 × 44 design px by default.
- Flat ink face with white 2 px pointer. 7 plain ticks (2 px) around it.
- Label above (10 px bold), readout below (10 px, min 56 wide).
- Options: `setDialSize` (e.g. 72 for a featured dial, 36 in a tight row),
  `setEndLabels ("CLOSE", "FAR")` (9 px subInk either side), `setDetents`,
  `setHintTick` (a suggested position: that tick turns red), `setUppercaseValue`
  for word readouts, `setShowLabel (false)` when the label sits beside it.

### Selector row — `SelectorRow`
The stepped switch for every choice parameter.
- Optional label (10 px bold caps).
- **Tick rail** above: dotted line between the first and last button centres; a tick
  per button, selected tick 3 × 7, others 1 × 5.
- Buttons: 26 px tall, paper cells split by 1 px ink, 1 px outline, **3 px dithered
  shadow**.
- **Selected = ink cell with an inset 1 px paper line (2 px in).**
- Optional captions under each button (9 px subInk, selected bold).
- Keyboard: ← / →. Focus: 2 px blue outline.
- View switches (which page or screen mode is showing) use the same row without a
  parameter: `SelectorRow ("CHAIN", { ... })`, `onChange`, `setSelected`. Sub-labels can
  carry state, e.g. ON / OFF under each section in Voxmaster's CHAIN row.
- Options: `setShowRail (false)` and `setFillHeight (true)` for a big top-row
  selector, `setSubLabels` for a second 9 px line inside each button,
  `setGreyedOut (true)` when the control doesn't apply (plate dither over it, clicks
  ignored, the parameter keeps its value).

### Readout — `drawReadout (g, rect, text)`
Ink box, paper text, Space Mono regular, centred, 2 × 8 px padding. Clickable readouts
that step a choice use `StepReadout`.

### Level meter — `LevelMeter`
Thin vertical meter on the screen checker: 1 px ink outline, 20 cream segments on dim,
top two red, peak-hold segment stays lit. `setPeak (linear)` from a timer.

### Stock widgets — `OngaLookAndFeel`
Set it on the panel (scale 1) for dropdowns, action buttons and checkboxes, and keep a
second instance at window scale (`setScale (width / 820)`) for popup menus, alert
windows, tooltips and full-window overlays.
- Buttons and dropdowns: paper, 1 px ink, dithered 3 px shadow; pressed/on = ink with
  an inset paper line. `getProperties().set ("onga.flat", true)` drops the shadow
  (small footer buttons).
- Checkbox: paper square with an ink X. Menus: paper, ink highlight, dotted separators.
- Dialog overlays: dim the window with the ink checker; the dialog is a bar-surface box
  with a 1 px outline and dithered shadow, its title between pinstripes.

### Footer buttons
If a plug-in needs presets, settings or bypass on the panel, they go in the footer as
small flat buttons after `ONGA TOOLS`, never in the title bar.

### Screen — `paintScreen`, `drawScreenTitle`, `drawScreenHeader`, `drawLegend`
- Screen surface, 1 px ink outline.
- Title top-left (14 px bold cream). Optional header top-right (10 px ghost), e.g.
  `TRACK > VOCALS > SILK AIR`.
- Data drawn as **hard pixels**: bars, cells, stepped lines; ordered dither instead of
  soft edges. No gradients, no glow.
- **No scale numbers.** Minimal labels (a line tag like `0 VU TAPE`, band markers like
  `LOW / HIGH`).
- **Legend bottom-right**: 8 × 8 swatch, 4 px, name, 12 px between entries. **Every
  entry must be visible on the screen**, and names match the controls (e.g. `SPACE`,
  not `ROOM`).

### Logo tile — `PixelLogo`
- 200 × 96 tile, 3 px ink frame, paper/ink checker background.
- The plug-in name in chunky pixel letters (`forEachGlyphCell`, 5 × 7 font, or a custom
  glyph set like BLOOM's 9-row letters).
- Letters: ink outline blocks behind each cell; cells filled cream, dither or accent.
- **One** slow loop that hints at what the plug-in does. Frames are stepped, not
  tweened: default 40 frames / 4.8 s. Keep it hypnotic, not busy.

## 6. The suite's logos

| Plug-in | Word | Animation |
|---|---|---|
| ONGA BLOOM | `BLOOM` (9-row letters) | Columns ride a slow sine wave; cream / dither / red bands radiate out from the middle O like petals opening. |
| ONGA TRANSFORMER | `TRANS` / `FORMER` | A drive wave sweeps left→right: cells heat from small cream dots → dither → red with a heavier outline. |
| ONGA PANNA | `PANNA` | The word itself pans: glides left↔right across the tile, leaving a red motion trail on the side it just left. |
| ONGA magEQ | `MAGEQ` | Static cream word; pixel sparkles twinkle on and off all over the tile (red dot → four-point star → cream → gone). |
| ONGA VOXMASTER | `VOX` / `MASTER` | 70s stacked shadow: the word extruded down-left in red / amber / blue bands split by black lines; the bands flow outward like echoes. |

Each animation lives with its plug-in (a `LogoFrame (float phase)` function), not in the kit.

## 7. Rules checklist (review every panel against this)

- [ ] Logo tile top-left, 3 px frame, name in pixel letters, one slow loop.
- [ ] Title bar is only stripes + name. Footer is `ONGA TOOLS` (+ any flat footer buttons) + one status line.
- [ ] Surfaces: faceplate darkest, plates lighter, buttons lightest, all dithered except paper.
- [ ] Exactly one hero dial (red pointer + arc). All other dials are small, white pointer.
- [ ] Every choice parameter is a `SelectorRow` with tick rail, shadow and inset selection.
- [ ] One dark screen. No scale numbers. Legend entries all appear on screen and match control names.
- [ ] No solid shadows, gradients, bevels, knurling or window chrome.
- [ ] Red only on the hero dial, screen data and the logo.
- [ ] Labels 10 px bold caps; readouts ink boxes; captions 9 px subInk.
- [ ] Keyboard focus visible (blue).

## 8. Using the kit

```cmake
# after JUCE is available
add_subdirectory(onga-ui)                        # or the submodule path
target_link_libraries(MyPlugin PRIVATE onga_ui)
```

```cpp
#include <onga_ui/OngaUI.h>
using namespace onga::ui;

class MyEditor : public OngaEditor { ... };      // 820 x 580, scales as a unit

// in the panel component:
paintFaceplate (g, all);
paintTitleBar (g, titleArea, "ONGA NAME");
paintFooter (g, footerArea, "v1.0.0" + dot() + "48 kHz");
drawPlate (g, plateArea);

HeroDial   drive  { apvts, "drive", "DRIVE" };
SmallDial  output { apvts, "output", "OUTPUT" };
SelectorRow low   { apvts, "lowcut", "LOW CUT (HZ)", { "OFF", "80", "120" } };
PixelLogo  logo   { "ONGA NAME", myLogoFrame };
```

Complete panels: `Source/plugin/PluginEditor.cpp` in
[onga_transformer](https://github.com/benettriley/onga_transformer) (stock widgets,
menus, a dialog overlay) and `Source/PluginEditor.cpp` in [onga_bloom](https://github.com/benettriley/onga_bloom) for a complete panel.

## 9. Adding the kit to a plug-in

The kit lives in [benettriley/onga_v1_UI](https://github.com/benettriley/onga_v1_UI) and
each plug-in pulls it in as a git submodule at `onga-ui/`:

```sh
git submodule add https://github.com/benettriley/onga_v1_UI onga-ui
```

```cmake
add_subdirectory(onga-ui)                        # after JUCE is available
target_link_libraries(MyPlugin PRIVATE onga_ui)
```

Clone plug-ins with `git clone --recursive`, or run `git submodule update --init` in an
existing clone. In GitHub Actions, check out with submodules:

```yaml
- uses: actions/checkout@v4
  with:
    submodules: recursive
    token: ${{ secrets.ONGA_UI_TOKEN || github.token }}
```

This repo is private, so the default Actions token can't read it from another repo.
Either make this repo public, or add a fine-grained personal access token with
read-only *Contents* access to it as the `ONGA_UI_TOKEN` secret in each plug-in repo.

To pick up kit changes in a plug-in: `git submodule update --remote onga-ui`, rebuild,
check the panel, commit the new submodule pointer.
