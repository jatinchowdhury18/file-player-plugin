#program_export
plugin_process :: (plugin: *clap_plugin_t, process: *clap_process_t) -> clap_process_status #c_call {
    player_state := from_plugin(plugin);
    push_context player_state.audio_context {
        player_state.buffer_swap_proc (plugin, player_state);

        // @TODO: right now we're not splitting blocks on param automation or modulation events
        process_input_events(plugin, player_state, process.in_events);

        play_audio (player_state, process.audio_outputs[0].channel_count, process.frames_count, process.audio_outputs[0].data32);

        player_state.pull_param_changes_proc(plugin, process.out_events);

        return.CLAP_PROCESS_CONTINUE;
    }
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

play_audio :: (using state: *Player_State, num_channels: u32, num_samples: u32, buffer_data: **float) {
    // clear any lingering data
    for ch : 0..num_channels-1 { memset(buffer_data[ch], 0, num_samples * size_of(float)); }

    if (playing_buffer.juce_buffer == null)
        return;

    // @TODO: faster way to get the params!
    gain_param_value := params.gain_param.value;

    samples_to_copy := min(num_samples, playing_buffer.num_samples - sample_ptr);
    if (samples_to_copy > 0) {
        for ch : 0..num_channels-1 {
            // copy data
            channel_data := playing_buffer.channel_ptrs[ch % playing_buffer.num_channels];
            memcpy(buffer_data[ch], channel_data + sample_ptr, samples_to_copy * size_of(float));

            // apply gain
            for n : 0..num_samples-1 { buffer_data[ch][n] *= cast(float32) gain_param_value; }
        }
        sample_ptr += samples_to_copy;
    }
}

process_input_events :: (plugin: *clap_plugin_t, using state: *Player_State, events: *clap_input_events_t) {
    get_param :: (cookie: *void, param_id: clap_id, param_table: *Hash_Table.Table) -> *Params.Param {
        return ifx cookie != null {
            cast(*Params.Param) cookie;
        } else {
            Hash_Table.table_find_pointer(param_table, param_id).*;
        }
    }

    events_count := events.size(events);
    if events_count == 0 { return; }

    for event_index : 0..events_count-1 {
        event := events.get(events, event_index);

        if event.type == CLAP_EVENT_PARAM_VALUE {
            param_event := cast(*clap_event_param_value_t) event;

            param := get_param(param_event.cookie, param_event.param_id, *params.param_table);
            if param == null { continue; } // Host misbehaving!

            param.value = param_event.value;

            push_param_change_proc(plugin, param_event.param_id, param_event.value);
        } else if event.type == CLAP_EVENT_PARAM_MOD {
            param_event := cast(*clap_event_param_mod_t) event;
	        
            param := get_param(param_event.cookie, param_event.param_id, *params.param_table);
            if param == null { continue; } // Host misbehaving!

            param.modulation = param_event.amount;
        }
    }
}
