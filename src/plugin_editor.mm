//#include <juce_audio_plugin_client/detail/juce_IncludeSystemHeaders.h>
//#include <juce_audio_plugin_client/detail/juce_PluginUtilities.h>
//#include <juce_audio_plugin_client/detail/juce_VSTWindowUtilities.h>

#include <Cocoa/Cocoa.h>

#include "plugin_editor.cpp"

namespace file_player::editor
{
static void* attachComponentToWindowRefVST (juce::Component* comp,
                                            int desktopFlags,
                                            void* parentWindowOrView)
{
    JUCE_AUTORELEASEPOOL
    {
        NSView* parentView = [(NSView*) parentWindowOrView retain];

        const auto defaultFlags = juce::ComponentPeer::windowIgnoresKeyPresses;
        comp->addToDesktop (desktopFlags | defaultFlags, parentView);

        // (this workaround is because Wavelab provides a zero-size parent view..)
        if (juce::approximatelyEqual ([parentView frame].size.height, 0.0))
            [((NSView*) comp->getWindowHandle()) setFrameOrigin: NSZeroPoint];

        comp->setVisible (true);
        comp->toFront (false);

        [[parentView window] setAcceptsMouseMovedEvents: YES];
        return parentView;
    }
}

bool Editor_Provider::set_parent (const clap_plugin* plugin, const clap_window* window) noexcept
{
    auto& editor = from_plugin (plugin);

#if JUCE_MAC
    auto* ns_view = window->cocoa;
    const auto desktop_flags = 0; // no layer-backed view
    auto host_window = attachComponentToWindowRefVST (editor.editor_wrapper.get(), desktop_flags, ns_view);
    juce::ignoreUnused (host_window);
    return true;
#endif
}
}
