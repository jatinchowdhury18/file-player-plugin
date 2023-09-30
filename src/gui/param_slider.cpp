#include "param_slider.h"
#include "../file_player_plugin.h"

namespace file_player::editor
{
Param_Slider::Param_Slider (size_t param_index,
                            const clap_plugin_t* plugin,
                            std::function<void (plugin::Param_Action&&)> action_sender,
                            Param_Slider_Skew skew)
    : param_action_sender (std::move (action_sender))
{
    auto* params_ext = reinterpret_cast<const clap_plugin_params*> (plugin->get_extension (plugin, CLAP_EXT_PARAMS));

    params_ext->get_info (plugin, param_index, &param_info);

    juce::NormalisableRange slider_range { param_info.min_value, param_info.max_value };
    slider_range.interval = 0.001;
    if (skew == Param_Slider_Skew::Log)
        slider_range.setSkewForCentre (std::sqrt (slider_range.start * slider_range.end));
    setNormalisableRange (slider_range);

    setDoubleClickReturnValue (true, param_info.default_value);

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
