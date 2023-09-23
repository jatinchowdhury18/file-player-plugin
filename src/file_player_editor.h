#pragma once

#include <juce_gui_basics/juce_gui_basics.h>
#include <juce_audio_formats/juce_audio_formats.h>

namespace file_player::plugin
{
struct File_Player_Plugin;
}

namespace file_player::editor
{
struct File_Player_Editor : juce::Component
{
    explicit File_Player_Editor (plugin::File_Player_Plugin& plugin);

    void paint (juce::Graphics& g) override;
    void resized() override;

    plugin::File_Player_Plugin& plugin;

    juce::TextButton select_file_button { "Select File" };
    std::shared_ptr<juce::FileChooser> file_chooser;
    juce::AudioFormatManager audio_format_manager;
};
}
