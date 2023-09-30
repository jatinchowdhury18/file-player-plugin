#pragma once

#include <juce_audio_formats/juce_audio_formats.h>
#include <juce_gui_basics/juce_gui_basics.h>

#include "param_slider.h"

namespace file_player::plugin
{
struct File_Player_Plugin;
struct Param_Action;
} // namespace file_player::plugin

namespace file_player::editor
{
struct File_Player_Editor : juce::Component
{
    explicit File_Player_Editor (plugin::File_Player_Plugin& plugin);

    void paint (juce::Graphics& g) override;
    void resized() override;

    void handle_param_action (const plugin::Param_Action&);

    plugin::File_Player_Plugin& plugin;
    const std::function<void(plugin::Param_Action&&)> action_forwarder;

    juce::TextButton select_file_button { "Select File" };
    std::shared_ptr<juce::FileChooser> file_chooser;
    juce::AudioFormatManager audio_format_manager;

    Param_Slider gain_slider;
    Param_Slider repitch_slider;
};
} // namespace file_player::editor
