// this is a stupid hack to allow us to access private members of clap::helpers::Plugin
// I should probably just fork that class instead...
#define private protected
#include <clap/helpers/host-proxy.hh>
#include <clap/helpers/host-proxy.hxx>
#include <clap/helpers/plugin.hxx>

#include <juce_gui_basics/juce_gui_basics.h>

#include "file_player_plugin.h"
#include "plugin_editor.h"

template struct clap::helpers::Plugin<file_player::plugin::misLevel, file_player::plugin::checkLevel>;
template struct clap::helpers::HostProxy<file_player::plugin::misLevel, file_player::plugin::checkLevel>;

namespace file_player::plugin
{
struct Test_Editor : juce::Component
{
    Test_Editor()
    {
        setSize (400, 400);
    }

    void paint (juce::Graphics& g) override
    {
        g.fillAll (juce::Colours::red);
    }
};

File_Player_Plugin::File_Player_Plugin (const clap_host* host)
    : Plugin (&descriptor, host)
{
    _plugin.get_extension = &get_extension;

    editor.create_editor = [] { return std::make_unique<Test_Editor>(); };
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
} // namespace file_player::plugin
