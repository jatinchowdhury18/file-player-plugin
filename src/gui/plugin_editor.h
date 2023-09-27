#pragma once

#include <clap/clap.h>

namespace juce
{
class Component;
class ScopedJuceInitialiser_GUI;
class ComponentBoundsConstrainer;
}

namespace file_player::editor
{
struct Editor_Wrapper_Component;

struct Editor
{
    Editor();
    ~Editor();

    std::unique_ptr<juce::ScopedJuceInitialiser_GUI> juce_library_initializer;

    bool is_resizeable = false;
    bool gui_parent_attached = false;
    float gui_scale_factor = 1.0f;
    std::unique_ptr<Editor_Wrapper_Component> editor_wrapper;
    std::function<std::unique_ptr<juce::Component>()> create_editor = nullptr;
    std::unique_ptr<juce::ComponentBoundsConstrainer> constrainer {};

    juce::Component* editor_comp = nullptr;
};

struct Editor_Provider
{
    static bool is_api_supported (const clap_plugin* plugin, const char* api, bool is_floating) noexcept;
    static bool get_preferred_api (const clap_plugin_t*, const char**, bool*) noexcept { return false; }

    static bool gui_create (const clap_plugin* plugin, const char* api, bool is_floating) noexcept;
    static void gui_destroy (const clap_plugin* plugin) noexcept;

    static bool set_scale (const clap_plugin* plugin, double scale) noexcept;
    static bool get_size (const clap_plugin* plugin, uint32_t* width, uint32_t* height) noexcept;
    static bool set_size (const clap_plugin* plugin, uint32_t width, uint32_t height) noexcept;
    static bool can_resize (const clap_plugin* plugin) noexcept;
    static bool get_resize_hints (const clap_plugin_t*, clap_gui_resize_hints_t*) noexcept { return false; }
    static bool adjust_size (const clap_plugin* plugin, uint32_t* width, uint32_t* height) noexcept;

    static bool gui_show (const clap_plugin* plugin) noexcept;
    static bool gui_hide (const clap_plugin* plugin) noexcept;

    static bool set_parent (const clap_plugin* plugin, const clap_window* window) noexcept;
    static bool set_transient (const clap_plugin* plugin, const clap_window* window) noexcept;

    static void suggest_title (const clap_plugin*, const char*) noexcept {}

    static constexpr clap_plugin_gui gui_extension {
        .is_api_supported = is_api_supported,
        .get_preferred_api = get_preferred_api,
        .create = gui_create,
        .destroy = gui_destroy,
        .set_scale = set_scale,
        .get_size = get_size,
        .can_resize = can_resize,
        .get_resize_hints = get_resize_hints,
        .adjust_size = adjust_size,
        .set_size = set_size,
        .set_parent = set_parent,
        .set_transient = set_transient,
        .suggest_title = suggest_title,
        .show = gui_show,
        .hide = gui_hide,
    };

    // static use only!
    Editor_Provider() = delete;
};

#if JUCE_MAC
// defined in plugin_editor.mm
bool set_editor_parent_macos (juce::Component* editor_comp, const clap_window* window) noexcept;
#endif
} // namespace file_player::editor
