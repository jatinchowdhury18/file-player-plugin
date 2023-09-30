#include <clap/helpers/host-proxy.hh>
#include <clap/helpers/host-proxy.hxx>

#include <juce_gui_basics/juce_gui_basics.h>

#include "file_player_plugin.h"
#include "gui/file_player_editor.h"

template struct clap::helpers::Plugin<file_player::plugin::misLevel, file_player::plugin::checkLevel>;
template struct clap::helpers::HostProxy<file_player::plugin::misLevel, file_player::plugin::checkLevel>;

namespace file_player::plugin
{
void buffer_swap_audio_thread (clap_plugin_t* plugin, jai::Player_State* player_state)
{
    auto* file_player_plugin = static_cast<File_Player_Plugin*> (plugin->plugin_data);

    if (file_player_plugin->buffer_life_queue.peek() != nullptr)
    {
        std::unique_ptr<juce::AudioBuffer<float>> buffer_swap_ptr;
        if (file_player_plugin->buffer_life_queue.try_dequeue (buffer_swap_ptr))
        {
            const auto num_channels = buffer_swap_ptr->getNumChannels();
            const auto num_samples = buffer_swap_ptr->getNumSamples();
            const auto buffer_data = buffer_swap_ptr->getArrayOfWritePointers();
            buffer_swap_ptr.reset (reinterpret_cast<juce::AudioBuffer<float>*> (
                swap_buffers (player_state,
                              buffer_swap_ptr.release(),
                              num_channels,
                              num_samples,
                              const_cast<float**> (buffer_data))));

            file_player_plugin->buffer_death_queue.try_enqueue (std::move (buffer_swap_ptr));
            file_player_plugin->request_host_callback();
        }
    }
}

void param_change_audio_to_gui (clap_plugin_t* plugin, clap_id param_id, double new_value)
{
    auto& file_player_plugin = *static_cast<File_Player_Plugin*> (plugin->plugin_data);
    file_player_plugin.params_audio_to_gui_queue.enqueue ({
        .param_id = param_id,
        .new_value = new_value,
    });
    file_player_plugin.request_host_callback();
}

void param_changes_gui_to_audio (clap_plugin_t* plugin, clap_output_events* out_events)
{
    auto& file_player_plugin = *static_cast<File_Player_Plugin*> (plugin->plugin_data);

    Param_Action action;
    while (file_player_plugin.params_gui_to_audio_queue.try_dequeue (action))
    {
        if (action.is_begin_gesture || action.is_end_gesture)
        {
            clap_event_param_gesture event {
                .header {
                    .size = sizeof (clap_event_param_gesture),
                    .time = 0,
                    .space_id = CLAP_CORE_EVENT_SPACE_ID,
                    .type = static_cast<uint16_t> (action.is_begin_gesture ? CLAP_EVENT_PARAM_GESTURE_BEGIN : CLAP_EVENT_PARAM_GESTURE_END),
                    .flags = {},
                },
                .param_id = action.param_id,
            };
            out_events->try_push (out_events, reinterpret_cast<const clap_event_header*> (&event));
        }
        else
        {
            clap_event_param_value event {
                .header {
                    .size = sizeof (clap_event_param_value),
                    .time = 0,
                    .space_id = CLAP_CORE_EVENT_SPACE_ID,
                    .type = static_cast<uint16_t> (CLAP_EVENT_PARAM_VALUE),
                    .flags = {},
                },
                .param_id = action.param_id,
                .note_id = -1,
                .port_index = -1,
                .channel = -1,
                .key = -1,
                .value = action.new_value,
            };
            out_events->try_push (out_events, reinterpret_cast<const clap_event_header*> (&event));

        }
    }
}

File_Player_Plugin::File_Player_Plugin (const clap_host* host)
    : Plugin (&descriptor, host)
{
    //    const auto offset_test = offsetof (File_Player_Plugin, player_state);
    //    jassert (offsetof (File_Player_Plugin, player_state) == 456);

    _plugin.get_extension = &get_extension;
    _plugin.process = reinterpret_cast<clap_process_status (*) (const clap_plugin*, const clap_process_t*)> (&plugin_process);

    init_player_state (main_context, &player_state);
    player_state.main_context = main_context;
    player_state.audio_context = audio_context;
    player_state.buffer_swap_proc = reinterpret_cast<void*> (&buffer_swap_audio_thread);
    player_state.push_param_change_proc = reinterpret_cast<void*> (&param_change_audio_to_gui);
    player_state.pull_param_changes_proc = reinterpret_cast<void*> (&param_changes_gui_to_audio);
    jassert (from_plugin (&_plugin) == &player_state);

    editor.create_editor = [this]
    { return std::make_unique<editor::File_Player_Editor> (*this); };
    editor.is_resizeable = true;

    editor.constrainer = std::make_unique<juce::ComponentBoundsConstrainer>();
    editor.constrainer->setSizeLimits (125, 125, 500, 500);
    editor.constrainer->setFixedAspectRatio (1.0);
}

const void* File_Player_Plugin::get_extension (const clap_plugin* plugin, const char* id) noexcept
{
    static const clap_plugin_audio_ports audio_ports_ext {
        .count = reinterpret_cast<uint32_t (*) (const clap_plugin_t*, bool)> (audio_ports_count),
        .get = reinterpret_cast<bool (*) (const clap_plugin_t*, uint32_t, bool, clap_audio_port_info_t*)> (audio_ports_get),
    };

    static const clap_plugin_params params_ext {
        .count = reinterpret_cast<uint32_t (*) (const clap_plugin_t*)> (params_count),
        .get_info = reinterpret_cast<bool (*) (const clap_plugin_t*, uint32_t, clap_param_info_t*)> (param_get_info),
        .get_value = reinterpret_cast<bool (*) (const clap_plugin_t*, clap_id, double*)> (param_get_value),
        .value_to_text = reinterpret_cast<bool (*) (const clap_plugin_t*, clap_id, double, char*, uint32_t)> (param_value_to_text),
        .text_to_value = reinterpret_cast<bool (*) (const clap_plugin_t*, clap_id, const char*, double*)> (param_text_to_value),
        .flush = reinterpret_cast<void (*) (const clap_plugin_t*, const clap_input_events_t*, const clap_output_events_t*)> (params_flush),
    };

    if (strcmp (id, CLAP_EXT_GUI) == 0)
        return &editor::Editor_Provider::gui_extension;

    if (strcmp (id, CLAP_EXT_AUDIO_PORTS) == 0)
        return &audio_ports_ext;

    if (strcmp (id, CLAP_EXT_PARAMS) == 0)
        return &params_ext;

    return Plugin::clapExtension (plugin, id);
}

void File_Player_Plugin::onMainThread() noexcept
{
    std::unique_ptr<juce::AudioBuffer<float>> buffer_killer {};
    while (buffer_death_queue.try_dequeue (buffer_killer))
        buffer_killer.reset (nullptr);

    Param_Action param_action;
    while (params_audio_to_gui_queue.try_dequeue (param_action))
    {
        if (auto* editor_comp = reinterpret_cast<editor::File_Player_Editor*> (editor.editor_comp))
            editor_comp->handle_param_action (param_action);
    }
}
} // namespace file_player::plugin
