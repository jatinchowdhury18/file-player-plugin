make_name :: (name_set: string) -> [CLAP_NAME_SIZE] u8 {
    name: [CLAP_NAME_SIZE] u8;
    assert(name_set.count < name.count);
    memset(name.data, 0, name.count);
    memcpy(name.data, name_set.data, min(name.count - 1, name_set.count));
    return name;
}

Params :: struct {
    gain_param_info : clap_param_info_t : .{
        id = 0,
        flags = .CLAP_PARAM_IS_AUTOMATABLE,
        cookie = null,
        name = #run make_name("Gain"),
        min_value = 0.0,
        max_value = 1.0,
        default_value = 0.5,
    };
    pitch_param_info : clap_param_info_t : .{
        id = 1,
        flags = .CLAP_PARAM_IS_AUTOMATABLE,
        cookie = null,
        name = #run make_name("Pitch"),
        min_value = 0.5,
        max_value = 2.0,
        default_value = 1.0,
    };

    count :: 2;
    param_info : [count] clap_param_info_t : .[gain_param_info, pitch_param_info];
};

#program_export
params_count :: (plugin: *clap_plugin_t) -> u32 #c_call {
    return Params.count;
}

#program_export
param_get_info :: (plugin: *clap_plugin_t, param_index: u32, param_info: *clap_param_info_t) -> bool #c_call {
    using player_state := from_plugin(plugin);

    if (param_index >= params.count) return false;

    memcpy(param_info, *params.param_info[param_index], size_of(clap_param_info_t));
    return true;
}

#program_export
param_get_value :: (plugin: *clap_plugin_t, param_id: clap_id, out_value: *float64) -> bool #c_call {
    out_value.* = 0.5;
    return true;
}

#program_export
param_value_to_text :: (plugin: *clap_plugin_t,
                        param_id: clap_id,
                        value: float64,
                        out_buffer: *char,
                        out_buffer_capacity: u32) -> bool #c_call {
    return true;
}

#program_export
param_text_to_value :: (plugin: *clap_plugin_t,
                        param_id: clap_id,
                        param_value_text: *char,
                        out_value: *float64) -> bool #c_call {
    out_value.* = 0.5;
    return true;
}

#program_export
params_flush :: (plugin: *clap_plugin_t, in: *clap_input_events_t, out: *clap_output_events_t) -> void #c_call {
    // @TODO
    return;
}

#import "clap-jai";
