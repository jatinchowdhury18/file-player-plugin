#include <clap/helpers/host-proxy.hh>
#include <clap/helpers/host-proxy.hxx>

#include <juce_gui_basics/juce_gui_basics.h>

#include "file_player_editor.h"
#include "file_player_plugin.h"

template struct clap::helpers::Plugin<file_player::plugin::misLevel, file_player::plugin::checkLevel>;
template struct clap::helpers::HostProxy<file_player::plugin::misLevel, file_player::plugin::checkLevel>;

namespace file_player::plugin
{
File_Player_Plugin::File_Player_Plugin (const clap_host* host)
    : Plugin (&descriptor, host)
{
    _plugin.get_extension = &get_extension;

    editor.create_editor = [this]
    { return std::make_unique<editor::File_Player_Editor> (*this); };
    editor.is_resizeable = true;

    editor.constrainer = std::make_unique<juce::ComponentBoundsConstrainer>();
    editor.constrainer->setSizeLimits (125, 125, 500, 500);
    editor.constrainer->setFixedAspectRatio (1.0);
}

const void* File_Player_Plugin::get_extension (const clap_plugin* plugin, const char* id) noexcept
{
    if (strcmp (id, CLAP_EXT_GUI) == 0)
        return &editor::Editor_Provider::gui_extension;

    return Plugin::clapExtension (plugin, id);
}

bool File_Player_Plugin::audioPortsInfo (uint32_t index, bool isInput, clap_audio_port_info* info) const noexcept
{
    if (index > 0)
        return false;

    info->channel_count = 2;
    info->id = 1;
    info->in_place_pair = 1;
    info->flags = CLAP_AUDIO_PORT_IS_MAIN;
    strcpy (info->name, "main");
    return true;
}

void File_Player_Plugin::onMainThread() noexcept
{
    std::unique_ptr<juce::AudioBuffer<float>> buffer_killer {};
    while (buffer_death_queue.try_dequeue (buffer_killer))
        buffer_killer.reset (nullptr);
}

clap_process_status File_Player_Plugin::process (const clap_process* process) noexcept
{
    if (buffer_life_queue.peek() != nullptr)
    {
        std::unique_ptr<juce::AudioBuffer<float>> buffer_swap_ptr;
        if (buffer_life_queue.try_dequeue (buffer_swap_ptr))
        {
            const auto num_channels = buffer_swap_ptr->getNumChannels();
            const auto num_samples = buffer_swap_ptr->getNumSamples();
            const auto buffer_data = buffer_swap_ptr->getArrayOfWritePointers();
            buffer_swap_ptr.reset (reinterpret_cast<juce::AudioBuffer<float>*> (
                swap_buffers (&player_state,
                              buffer_swap_ptr.release(),
                              num_channels,
                              num_samples,
                              const_cast<float**> (buffer_data))));

            buffer_death_queue.try_enqueue (std::move (buffer_swap_ptr));
            _host.requestCallback();
        }
    }

    play_audio (jai, &player_state, process->audio_outputs[0].channel_count, process->frames_count, process->audio_outputs[0].data32);

    return CLAP_PROCESS_CONTINUE;
}
} // namespace file_player::plugin
