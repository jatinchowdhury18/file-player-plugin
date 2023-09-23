CPMAddPackage(
    NAME juce
    GITHUB_REPOSITORY juce-framework/juce
    GIT_TAG 3d39e70393d629c39b9ae38aa17ed1b461b24a11
    OPTIONS "JUCE_MODULES_ONLY ON"
)

add_library(juce_libs STATIC)

target_link_libraries(juce_libs PUBLIC
    juce::juce_gui_basics
    juce::juce_graphics
    juce::juce_core
    juce::juce_events
    juce::juce_data_structures
    juce::juce_audio_formats
)

target_compile_definitions(juce_libs PUBLIC
    NDEBUG=$<IF:$<CONFIG:Release>,1,0>
    JUCE_GLOBAL_MODULE_SETTINGS_INCLUDED=1
    JUCE_STANDALONE_APPLICATION=0
    JUCE_USE_CURL=0
    JUCE_WEB_BROWSER=0
    JUCE_USE_FLAC=1
    JUCE_USE_OGGVORBIS=0
    JUCE_USE_WINDOWS_MEDIA_FORMAT=0
    JUCE_USE_MP3AUDIOFORMAT=1
)

if(WIN32)
    target_compile_options(juce_libs PRIVATE /bigobj)
endif()
