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
            std::swap (buffer_swap_ptr, playing_buffer);
            sample_ptr = 0;

            buffer_death_queue.try_enqueue (std::move (buffer_swap_ptr));
            _host.requestCallback();
        }
    }

    const auto num_channels_out = (int) process->audio_outputs[0].channel_count;
    const auto num_samples = (int) process->frames_count;
    auto buffer_data = process->audio_outputs[0].data32;

    for (int ch = 0; ch < num_channels_out; ++ch)
        std::fill (buffer_data[ch], buffer_data[ch] + num_samples, 0.0f);

    if (playing_buffer != nullptr)
    {
        const auto samples_to_copy = std::min (num_samples, playing_buffer->getNumSamples() - sample_ptr);

        if (samples_to_copy > 0)
        {
            for (int ch = 0; ch < num_channels_out; ++ch)
            {
                const auto* channel_data = playing_buffer->getReadPointer (ch % playing_buffer->getNumChannels());
                std::copy (channel_data + sample_ptr, channel_data + sample_ptr + samples_to_copy, buffer_data[ch]);
            }
            sample_ptr += samples_to_copy;
        }
    }

    return CLAP_PROCESS_CONTINUE;
}
} // namespace file_player::plugin
