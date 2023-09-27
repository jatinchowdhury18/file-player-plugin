#include <juce_gui_basics/juce_gui_basics.h>

#include "../file_player_plugin.h"
#include "plugin_editor.h"

namespace file_player::editor
{
struct Editor_Wrapper_Component : juce::Component
{
    using HostType = clap::helpers::HostProxy<plugin::misLevel, plugin::checkLevel>;

    Editor_Wrapper_Component (HostType& _host, Editor& _editor)
        : host (_host),
          editor (_editor)
    {
        setOpaque (true);
        setBroughtToFrontOnMouseClick (true);
    }

    ~Editor_Wrapper_Component() override
    {
        if (editor_component != nullptr)
        {
            juce::PopupMenu::dismissAllActiveMenus();
            editor_component.reset (nullptr);
        }
    }

    void createEditor (std::unique_ptr<juce::Component>&& editor_comp)
    {
        editor_component = std::move (editor_comp);

        if (editor_component != nullptr)
        {
#if JUCE_VERSION >= 0x060008
//            editorHostContext = std::make_unique<EditorHostContext> (host, clapWrapper.clapIDByParamPtr);
//            editor_component->setHostContext (editorHostContext.get());
#endif

            editor_component->setTransform (juce::AffineTransform::scale (editor.gui_scale_factor));
            addAndMakeVisible (editor_component.get());
            editor_component->setTopLeftPosition (0, 0);

            last_bounds = getSizeToContainChild();
            {
                const juce::ScopedValueSetter<bool> resizingParentSetter { resizing_parent, true };
                setBounds (last_bounds);
            }
        }
        else
        {
            // why is the editor component nullptr?
            jassertfalse;
        }
    }

    juce::Rectangle<int> getSizeToContainChild()
    {
        if (editor_component != nullptr)
            return getLocalArea (editor_component.get(), editor_component->getLocalBounds());

        return {};
    }

    [[nodiscard]] juce::Rectangle<int> convertToHostBounds (juce::Rectangle<int> pluginRect) const
    {
        const auto desktopScale = editor.gui_scale_factor;
        if (juce::isWithin (desktopScale, 1.0f, 1.0e-3f))
            return pluginRect;

        return { juce::roundToInt ((float) pluginRect.getX() * desktopScale),
                 juce::roundToInt ((float) pluginRect.getY() * desktopScale),
                 juce::roundToInt ((float) pluginRect.getWidth() * desktopScale),
                 juce::roundToInt ((float) pluginRect.getHeight() * desktopScale) };
    }

    void resizeHostWindow()
    {
        if (editor_component != nullptr)
        {
            auto editorBounds = getSizeToContainChild();
            auto b = convertToHostBounds ({ 0, 0, editorBounds.getWidth(), editorBounds.getHeight() });
            {
                const juce::ScopedValueSetter<bool> resizingParentSetter (resizing_parent, true);
                host.guiRequestResize ((uint32_t) b.getWidth(), (uint32_t) b.getHeight());
            }

            setBounds (editorBounds.withPosition (0, 0));
        }
    }

    void setEditorScaleFactor (float scale)
    {
        if (editor_component != nullptr)
        {
            auto prevEditorBounds = editor_component->getLocalArea (this, last_bounds);
            {
                const juce::ScopedValueSetter<bool> resizingChildSetter { resizing_child, true };
                editor_component->setTransform (juce::AffineTransform::scale (scale));
                editor_component->setBounds (prevEditorBounds.withPosition (0, 0));
            }

            last_bounds = getSizeToContainChild();
            resizeHostWindow();
            repaint();
        }
    }

    void paint (juce::Graphics& g) override { g.fillAll (juce::Colours::red); }
    void resized() override
    {
        if (editor_component != nullptr && ! resizing_parent)
        {
            auto newBounds = getLocalBounds();
            {
                const juce::ScopedValueSetter<bool> resizingChildSetter { resizing_child, true };
                editor_component->setBounds (editor_component->getLocalArea (this, newBounds).withPosition (0, 0));
            }
            last_bounds = newBounds;
        }
    }

    void childBoundsChanged (Component*) override
    {
        if (resizing_child)
            return;

        auto newBounds = getSizeToContainChild();
        if (newBounds != last_bounds)
        {
            resizeHostWindow();
            repaint();
            last_bounds = newBounds;
        }
    }

    HostType& host;
    Editor& editor;
    std::unique_ptr<juce::Component> editor_component;
#if JUCE_VERSION >= 0x060008
//    std::unique_ptr<juce::AudioProcessorEditorHostContext> editorHostContext;
#endif

private:
    juce::Rectangle<int> last_bounds;
    bool resizing_child = false, resizing_parent = false;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (Editor_Wrapper_Component)
};

bool Editor_Provider::is_api_supported (const clap_plugin*, const char* api, bool is_floating) noexcept
{
    if (is_floating)
        return false;

    if (strcmp (api, CLAP_WINDOW_API_WIN32) == 0 || strcmp (api, CLAP_WINDOW_API_COCOA) == 0 || strcmp (api, CLAP_WINDOW_API_X11) == 0)
        return true;

    return false;
}

Editor::Editor()
{
    juce_library_initializer = std::make_unique<juce::ScopedJuceInitialiser_GUI>();
}

Editor::~Editor() = default;

static Editor& from_plugin (const clap_plugin* plugin)
{
    return static_cast<plugin::File_Player_Plugin*> (plugin->plugin_data)->editor;
}

bool Editor_Provider::gui_create (const clap_plugin* plugin, const char*, bool) noexcept
{
    const juce::MessageManagerLock mmLock;

    auto& editor = from_plugin (plugin);
    editor.editor_wrapper = std::make_unique<editor::Editor_Wrapper_Component> (static_cast<plugin::File_Player_Plugin*> (plugin->plugin_data)->get_host(), editor);
    editor.editor_wrapper->createEditor (editor.create_editor());
    return true;
}

void Editor_Provider::gui_destroy (const clap_plugin* plugin) noexcept
{
    auto& editor = from_plugin (plugin);
    editor.editor_wrapper.reset (nullptr);
    editor.gui_parent_attached = false;
}

bool Editor_Provider::set_scale (const clap_plugin* plugin, double scale) noexcept
{
    auto& editor = from_plugin (plugin);
    editor.gui_scale_factor = static_cast<float> (scale);

    if (editor.editor_wrapper != nullptr)
    {
        editor.editor_wrapper->setEditorScaleFactor (editor.gui_scale_factor);
        return true;
    }

    return false;
}

bool Editor_Provider::get_size (const clap_plugin* plugin, uint32_t* width, uint32_t* height) noexcept
{
    const juce::MessageManagerLock mmLock;
    auto& editor = from_plugin (plugin);
    if (editor.editor_wrapper != nullptr)
    {
        const auto b = editor.editor_wrapper->getBounds();
        *width = (uint32_t) b.getWidth();
        *height = (uint32_t) b.getHeight();
        return true;
    }
    else
    {
        *width = 1000;
        *height = 800;
    }
    return false;
}

bool Editor_Provider::can_resize (const clap_plugin* plugin) noexcept
{
    auto& editor = from_plugin (plugin);
    return editor.is_resizeable;
}

bool Editor_Provider::set_size (const clap_plugin* plugin, uint32_t width, uint32_t height) noexcept
{
    auto& editor = from_plugin (plugin);
    if (editor.editor_wrapper == nullptr)
        return false;

    if (! editor.is_resizeable)
        return false;

    const auto b = juce::Rectangle { (int) width, (int) height };
    editor.editor_wrapper->setSize (b.getWidth(), b.getHeight());
    return true;
}

bool Editor_Provider::adjust_size (const clap_plugin* plugin, uint32_t* w, uint32_t* h) noexcept
{
    auto& editor = from_plugin (plugin);
    if (editor.editor_wrapper == nullptr)
        return false;

    if (! editor.is_resizeable)
        return false;

    auto* cst = editor.constrainer.get();
    if (cst == nullptr)
        return true; // we have no constraints. Whatever is fine!

    const auto min_bounds = editor.editor_wrapper->convertToHostBounds ({ cst->getMinimumWidth(), cst->getMinimumHeight() });
    const auto max_bounds = editor.editor_wrapper->convertToHostBounds ({ cst->getMaximumWidth(), cst->getMaximumHeight() });
    auto min_width = (uint32_t) min_bounds.getWidth();
    auto max_width = (uint32_t) max_bounds.getWidth();
    auto min_height = (uint32_t) min_bounds.getHeight();
    auto max_height = (uint32_t) max_bounds.getHeight();

    auto width = juce::jlimit (min_width, max_width, *w);
    auto height = juce::jlimit (min_height, max_height, *h);
    const auto aspect_ratio = (float) cst->getFixedAspectRatio();

    if (aspect_ratio > 0.0f)
    {
        /*
         * This is obviously an unsatisfactory algorithm, but we wanted to have
         * something at least workable here.
         *
         * The problem with other algorithms I tried is that this function gets
         * called by BWS for sub-single pixel motions on macOS, so it is hard to make
         * a version which is *stable* (namely adjust(w,h); cw=w;ch=h; adjust(cw,ch);
         * cw == w; ch == h) that deals with directions. I tried all sorts of stuff
         * and then ran into vacation.
         *
         * So for now here's this approach. See the discussion in CJE PR #67
         * and interop-tracker issue #30.
         */
        width = (uint32_t) std::round (aspect_ratio * (float) height);
    }

    *w = width;
    *h = height;

    return true;
}

bool Editor_Provider::gui_show (const clap_plugin* plugin) noexcept
{
    auto& editor = from_plugin (plugin);
    return editor.gui_parent_attached;
}

bool Editor_Provider::gui_hide (const clap_plugin*) noexcept
{
    return false;
}

bool Editor_Provider::set_transient (const clap_plugin*, const clap_window*) noexcept
{
    jassertfalse;
    return false;
}
} // namespace file_player::editor

namespace file_player::editor
{
bool Editor_Provider::set_parent (const clap_plugin* plugin, const clap_window* window) noexcept
{
    auto& editor = from_plugin (plugin);

#if JUCE_MAC
    return set_editor_parent_macos (editor.editor_wrapper.get(), window);
#elif JUCE_WINDOWS
    editor.editor_wrapper->setVisible (false);
    editor.editor_wrapper->setTopLeftPosition (0, 0);
    editor.editor_wrapper->addToDesktop (0, (void*) window->win32);
    editor.editor_wrapper->setVisible (true);
    return true;
#endif
}
} // namespace file_player::editor
