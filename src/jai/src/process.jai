#program_export
plugin_process :: (plugin: *clap_plugin_t, process: *clap_process_t) -> clap_process_status #c_call {
    player_state := from_plugin(plugin);

    player_state.buffer_swap_proc(plugin, player_state);

    play_audio(player_state.audio_context, player_state, process.audio_outputs[0].channel_count, process.frames_count, process.audio_outputs[0].data32);

    return .CLAP_PROCESS_CONTINUE;
}

#program_export
swap_buffers :: (state: *Player_State,
                  swap_juce_buffer: *void,
                  new_num_channels: s32,
                  new_num_samples: s32,
                  new_channel_ptrs: **float) -> *void #c_call {
    using state.playing_buffer;

    // swap buffers
    dead_buffer := juce_buffer;
    juce_buffer = swap_juce_buffer;

    // fill out data
    num_channels = cast(u32) new_num_channels;
    num_samples = cast(u32) new_num_samples;
    for ch : 0..channel_ptrs.count-1 {
            if ch >= new_num_channels { channel_ptrs[ch] = null; }
            else { channel_ptrs[ch] = new_channel_ptrs[ch]; }
        }

    state.sample_ptr = 0;

    return dead_buffer;
}

#program_export
play_audio :: (ctx: *Context, using state: *Player_State, num_channels: u32, num_samples: u32, buffer_data: **float) #c_call {
    push_context ctx {
        // clear any lingering data
        for ch : 0..num_channels-1 { memset(buffer_data[ch], 0, num_samples * size_of(float)); }

        if (playing_buffer.juce_buffer == null)
            return;

    samples_to_copy := min(num_samples, playing_buffer.num_samples - sample_ptr);
        if (samples_to_copy > 0) {
            for ch : 0..num_channels-1 {
                channel_data := playing_buffer.channel_ptrs[ch % playing_buffer.num_channels];
                    memcpy(buffer_data[ch], channel_data + sample_ptr, samples_to_copy * size_of(float));
                }
            sample_ptr += samples_to_copy;
        }
    }
}
