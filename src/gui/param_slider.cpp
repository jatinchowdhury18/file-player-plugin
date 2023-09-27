#include "param_slider.h"
#include "../file_player_plugin.h"

namespace file_player::editor
{
Param_Slider::Param_Slider (size_t param_index,
                            const clap_plugin_t* plugin,
                            std::function<void (plugin::Param_Action&&)> action_sender)
    : param_action_sender (std::move (action_sender))
{
    auto* params_ext = reinterpret_cast<const clap_plugin_params*> (plugin->get_extension (plugin, CLAP_EXT_PARAMS));

    params_ext->get_info (plugin, param_index, &param_info);

    setRange ({ param_info.min_value, param_info.max_value }, 0.01);
    param = reinterpret_cast<jai::Param*> (param_info.cookie);
    setValue (param->value, juce::dontSendNotification);

    addListener (this);
}

Param_Slider::~Param_Slider()
{
    removeListener (this);
}

void Param_Slider::update_value (double new_value, bool send_to_host)
{
    if (send_to_host)
        skip_sending_to_host = true;

    setValue (new_value, juce::sendNotificationSync);

    skip_sending_to_host = false;
}

void Param_Slider::sliderValueChanged (Slider* slider)
{
    if (skip_sending_to_host)
        return;

    param->value = getValue();
    param_action_sender ({
        .param_id = param_info.id,
        .new_value = param->value,
    });
}

void Param_Slider::sliderDragStarted (Slider*)
{
    param_action_sender ({
        .param_id = param_info.id,
        .is_begin_gesture = true,
    });
}

void Param_Slider::sliderDragEnded (Slider*)
{
    param_action_sender ({
        .param_id = param_info.id,
        .is_end_gesture = true,
    });
}
} // namespace file_player::editor
