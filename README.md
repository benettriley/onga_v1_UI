# onga-ui

The shared panel kit for **Onga Tools** plug-ins (JUCE 8, C++17, header-only plus one
font blob). The design rules are in [ONGA_UI.md](ONGA_UI.md).

| Header | What's in it |
|---|---|
| `Tokens.h` | colours, surfaces, sizes, type scale |
| `Paint.h` | Space Mono fonts, checker dither, dithered shadows, plates, labels, readouts |
| `Chrome.h` | faceplate, pinstriped title bar, footer |
| `Dials.h` | `HeroDial`, `SmallDial` (bound to APVTS parameters) |
| `Selector.h` | `SelectorRow` (tick rail, shadow, inset selection), `StepReadout` |
| `Screen.h` | dark screen background, title, header, legend |
| `PixelLogo.h` | the animated pixel logo tile and a 5×7 pixel font |
| `Panel.h` | `OngaEditor`: 820 × 580 design size, scales as a unit |

Add it to a plug-in as a submodule (details and CI setup in ONGA_UI.md §9):

```sh
git submodule add https://github.com/benettriley/onga_v1_UI onga-ui
```

```cmake
add_subdirectory(onga-ui)
target_link_libraries(MyPlugin PRIVATE onga_ui)
```

```cpp
#include <onga_ui/OngaUI.h>
```

Fonts: Space Mono Regular/Bold, SIL Open Font License 1.1 (`fonts/SpaceMono-OFL.txt`).
