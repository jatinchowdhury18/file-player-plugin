#pragma once

#include <clap/clap.h>

#include <juce_gui_basics/juce_gui_basics.h>

#include <utility>

#include "../jai/file_player_jai_lib.h"

namespace file_player::plugin
{
struct Param_Action;
} // namespace file_player::plugin

namespace file_player::editor
{
struct Param_Slider : juce::Slider,
                      juce::Slider::Listener
{
    // finding the params by param_index is bad... let's find another way eventually
    explicit Param_Slider (size_t param_index,
                           const clap_plugin_t* plugin,
                           std::function<void(plugin::Param_Action&&)> action_sender);
    ~Param_Slider() override;

    void update_value (double new_value, bool send_to_host = true);

    void sliderValueChanged (Slider* slider) override;
    void sliderDragStarted (Slider*) override;
    void sliderDragEnded (Slider*) override;

    clap_param_info param_info {};
    jai::Param* param = nullptr;
    std::function<void(plugin::Param_Action&&)> param_action_sender;
    bool skip_sending_to_host = false;
};
} // namespace file_player::editor
