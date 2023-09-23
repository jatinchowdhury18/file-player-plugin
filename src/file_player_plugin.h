#pragma once

#include <clap/clap.h>
#include "clap_plugin.h"

#include <juce_audio_basics/juce_audio_basics.h>
#include <readerwriterqueue.h>

#include "plugin_editor.h"

namespace file_player::plugin
{
static constexpr clap::helpers::MisbehaviourHandler misLevel =
    clap::helpers::MisbehaviourHandler::Ignore;
static constexpr clap::helpers::CheckingLevel checkLevel = clap::helpers::CheckingLevel::Maximal;

using Plugin = clap::helpers::Plugin<misLevel, checkLevel>;

struct File_Player_Plugin : public Plugin
{
    static constexpr char const* features[] = { CLAP_PLUGIN_FEATURE_AUDIO_EFFECT, CLAP_PLUGIN_FEATURE_DELAY, nullptr };
    static constexpr clap_plugin_descriptor descriptor = { CLAP_VERSION_INIT,
                                                           "com.chowdsp.file-player", // @TODO: assert this matches with CMake (or copy from cmake)
                                                           "File Player Plugin",
                                                           "Chowdhury DSP",
                                                           "https://google.com",
                                                           "",
                                                           "",
                                                           "1.0.0",
                                                           "WIP!!!",
                                                           features };

    explicit File_Player_Plugin (const clap_host* host);

    static const void* get_extension (const clap_plugin* plugin, const char* id) noexcept;

    bool implementsAudioPorts() const noexcept override { return true; }
    uint32_t audioPortsCount (bool isInput) const noexcept override { return 1; }
    bool audioPortsInfo (uint32_t index, bool isInput, clap_audio_port_info* info) const noexcept override;

    void onMainThread() noexcept override;

    clap_process_status process (const clap_process* process) noexcept override;

    clap::helpers::HostProxy<misLevel, checkLevel>& get_host() { return _host; }

    editor::Editor editor;
    moodycamel::ReaderWriterQueue<std::unique_ptr<juce::AudioBuffer<float>>, 4> buffer_life_queue;
    moodycamel::ReaderWriterQueue<std::unique_ptr<juce::AudioBuffer<float>>, 4> buffer_death_queue;
    std::unique_ptr<juce::AudioBuffer<float>> playing_buffer;
    int sample_ptr = 0;
};
} // namespace file_player::plugin
