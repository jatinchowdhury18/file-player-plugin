//#include "plugin_editor.cpp"

#include <Cocoa/Cocoa.h>

#include <juce_gui_basics/juce_gui_basics.h>

#include "plugin_editor.h"

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
            [((NSView*) comp->getWindowHandle()) setFrameOrigin:NSZeroPoint];

        comp->setVisible (true);
        comp->toFront (false);

        [[parentView window] setAcceptsMouseMovedEvents:YES];
        return parentView;
    }
}

bool set_editor_parent_macos (juce::Component* editor_comp, const clap_window* window) noexcept
{
    auto* ns_view = window->cocoa;
    const auto desktop_flags = 0; // no layer-backed view
    auto host_window = attachComponentToWindowRefVST (editor_comp, desktop_flags, ns_view);
    juce::ignoreUnused (host_window);
    return true;
}
} // namespace file_player::editor
