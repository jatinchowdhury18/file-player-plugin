#include <clap/helpers/host-proxy.hh>
#include <clap/helpers/host-proxy.hxx>

#include <juce_gui_basics/juce_gui_basics.h>

#include "file_player_editor.h"
#include "file_player_plugin.h"

template struct clap::helpers::Plugin<file_player::plugin::misLevel, file_player::plugin::checkLevel>;
template struct clap::helpers::HostProxy<file_player::plugin::misLevel, file_player::plugin::checkLevel>;

namespace file_player::plugin
{
extern "C" void buffer_swap_audio_thread (clap_plugin_t* plugin, jai::Player_State* player_state)
{
    auto& file_player_plugin = *static_cast<File_Player_Plugin*> (plugin->plugin_data);

    if (file_player_plugin.buffer_life_queue.peek() != nullptr)
    {
        std::unique_ptr<juce::AudioBuffer<float>> buffer_swap_ptr;
        if (file_player_plugin.buffer_life_queue.try_dequeue (buffer_swap_ptr))
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

            file_player_plugin.buffer_death_queue.try_enqueue (std::move (buffer_swap_ptr));
            file_player_plugin.request_host_callback();
        }
    }
}

File_Player_Plugin::File_Player_Plugin (const clap_host* host)
    : Plugin (&descriptor, host)
{
    // const auto offset_test = offsetof (File_Player_Plugin, player_state);
    jassert (offsetof (File_Player_Plugin, player_state) == 456);
    _plugin.get_extension = &get_extension;
    _plugin.process = reinterpret_cast<clap_process_status (*) (const clap_plugin*, const clap_process_t*)> (&plugin_process);

    player_state.audio_context = jai;
    player_state.buffer_swap_proc = &buffer_swap_audio_thread;
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

    if (strcmp (id, CLAP_EXT_GUI) == 0)
        return &editor::Editor_Provider::gui_extension;

    if (strcmp (id, CLAP_EXT_AUDIO_PORTS) == 0)
        return &audio_ports_ext;

    return Plugin::clapExtension (plugin, id);
}

void File_Player_Plugin::onMainThread() noexcept
{
    std::unique_ptr<juce::AudioBuffer<float>> buffer_killer {};
    while (buffer_death_queue.try_dequeue (buffer_killer))
        buffer_killer.reset (nullptr);
}
} // namespace file_player::plugin
