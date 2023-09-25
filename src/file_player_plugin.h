#pragma once

#include <clap/clap.h>
#include "clap_plugin.h"

#include <juce_audio_basics/juce_audio_basics.h>
#include <readerwriterqueue.h>

#include "jai/JaiContext.h"
#include "jai/file_player_jai_lib.h"

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

    void request_host_callback() { _host.requestCallback(); }
    void onMainThread() noexcept override;

    clap::helpers::HostProxy<misLevel, checkLevel>& get_host() { return _host; }

    jai::Player_State player_state {};
    editor::Editor editor;
    moodycamel::ReaderWriterQueue<std::unique_ptr<juce::AudioBuffer<float>>, 4> buffer_life_queue;
    moodycamel::ReaderWriterQueue<std::unique_ptr<juce::AudioBuffer<float>>, 4> buffer_death_queue;

    JaiContextWrapper jai;
};
} // namespace file_player::plugin
