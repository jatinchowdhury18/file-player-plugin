#pragma once

#include "clap_plugin.h"
#include <clap/clap.h>

#include <juce_audio_basics/juce_audio_basics.h>
#include <readerwriterqueue.h>

#include "jai/JaiContext.h"
#include "jai/file_player_jai_lib.h"

#include "gui/plugin_editor.h"

namespace file_player::plugin
{
static constexpr clap::helpers::MisbehaviourHandler misLevel =
    clap::helpers::MisbehaviourHandler::Ignore;
static constexpr clap::helpers::CheckingLevel checkLevel = clap::helpers::CheckingLevel::Maximal;

using Plugin = clap::helpers::Plugin<misLevel, checkLevel>;

struct Param_Action
{
    clap_id param_id {};
    //    void* param_cookie = nullptr;
    double new_value = 0.0;
    bool is_begin_gesture = false;
    bool is_end_gesture = false;
};

struct BufferPtr
{
    std::unique_ptr<juce::AudioBuffer<float>> buffer;
    double sample_rate = 0.0;
};

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
    moodycamel::ReaderWriterQueue<BufferPtr, 4> buffer_life_queue;
    moodycamel::ReaderWriterQueue<BufferPtr, 4> buffer_death_queue;
    moodycamel::ReaderWriterQueue<Param_Action, 32> params_gui_to_audio_queue;
    moodycamel::ReaderWriterQueue<Param_Action, 32> params_audio_to_gui_queue;

    JaiContextWrapper audio_context;
    JaiContextWrapper main_context;
};
} // namespace file_player::plugin
