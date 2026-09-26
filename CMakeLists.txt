# onga-ui: the shared Onga Tools panel kit (JUCE 8, header-only + one font blob).
#
# Use from a plug-in's CMakeLists.txt, after JUCE is available:
#
#     add_subdirectory(onga-ui)                       # or a git submodule path
#     target_link_libraries(MyPlugin PRIVATE onga_ui)
#
# Then `#include <onga_ui/OngaUI.h>`.

juce_add_binary_data(onga_ui_fonts
    HEADER_NAME OngaUiFonts.h
    NAMESPACE   OngaUiFonts
    SOURCES
        ${CMAKE_CURRENT_LIST_DIR}/fonts/SpaceMono-Regular.ttf
        ${CMAKE_CURRENT_LIST_DIR}/fonts/SpaceMono-Bold.ttf)

add_library(onga_ui INTERFACE)
target_include_directories(onga_ui INTERFACE ${CMAKE_CURRENT_LIST_DIR}/include)
target_link_libraries(onga_ui INTERFACE onga_ui_fonts juce::juce_gui_basics juce::juce_audio_processors)
