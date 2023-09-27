#program_export
get_jai_info :: (ctx: *Context) -> string #c_call {
    return "File Player Plugin Jai library";
}

Audio_Buffer :: struct {
    juce_buffer: *void = null;
    num_channels: u32 = 0;
    num_samples: u32 = 0;
    channel_ptrs: [2]*float = .[null, null];
}

Player_State :: struct {
    main_context: *Context = null;
    audio_context: *Context = null;

    buffer_swap_proc: (plugin: *clap_plugin_t, state: *Player_State) -> void #no_context = null;
    playing_buffer: Audio_Buffer = .{};

    push_param_change_proc: (plugin: *clap_plugin_t, param_id: clap_id, new_value: float64) -> void #no_context = null;
    pull_param_changes_proc: (plugin: *clap_plugin_t, queue: *clap_output_events_t) -> void #no_context = null;

    sample_ptr: u32 = 0;

    params: Params = .{};
}

#program_export
from_plugin :: (plugin: *clap_plugin_t) -> *Player_State #c_call {
    #if OS == .WINDOWS {
        return cast (*Player_State) (plugin.plugin_data + 456);
    } else #if OS == .MACOS {
        return cast (*Player_State) (plugin.plugin_data + 432);
    }
    return null;
}

#import "Basic";
#import "clap-jai";
