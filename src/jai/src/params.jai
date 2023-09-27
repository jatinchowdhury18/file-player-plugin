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
    
    Param :: struct {
        value: float64;
        modulation: float64 = 0.0;
    };
    param_table: Hash_Table.Table(clap_id, Param);

    gain_param: *Param;
};

#program_export
params_count :: (plugin: *clap_plugin_t) -> u32 #c_call {
    return Params.count;
}

#program_export
param_get_info :: (plugin: *clap_plugin_t, param_index: u32, param_info: *clap_param_info_t) -> bool #c_call {
    using player_state := from_plugin(plugin);
    push_context main_context {
        if (param_index >= params.count) return false;

        if params.param_table.allocated == 0 {
            // So the hash table can't be 100% full, and to be performant, should be something like 70%
            // full. That said, if we know the parameter layout at compile-time, maybe we can do some
            // sort of perfect hashing? I think that would be both the most performant and the most
            // memory-efficient. 
            Hash_Table.init(*params.param_table, params.count * 3 / 2);
        }

        plugin_param_info := *params.param_info[param_index];
        memcpy(param_info, plugin_param_info, size_of(clap_param_info_t));

        param_ptr := Hash_Table.table_find_pointer(*params.param_table, param_info.id);
        if param_ptr == null {
            param_ptr = Hash_Table.table_add(*params.param_table,
                                              param_info.id,
                                              Params.Param.{ value = param_info.default_value, modulation = 0.0 });
        }
        param_info.cookie = param_ptr;

        // @TODO: obviously find a better way to do this!
        if param_info.id == params.gain_param_info.id { params.gain_param = param_ptr; }
        
        return true;
    }
}

#program_export
param_get_value :: (plugin: *clap_plugin_t, param_id: clap_id, out_value: *float64) -> bool #c_call {
    using player_state := from_plugin(plugin);
    push_context main_context {
        param := Hash_Table.table_find_pointer(*params.param_table, param_id);
        if param == null { return false; }

        out_value.* = param.value;

        return true;
    }
}

#program_export
param_value_to_text :: (plugin: *clap_plugin_t,
                        param_id: clap_id,
                        value: float64,
                        out_buffer: *char,
                        out_buffer_capacity: u32) -> bool #c_call {
    using player_state := from_plugin(plugin);
    push_context main_context {
        param_str := tprint("%", value);
        memcpy(out_buffer, param_str.data, min(cast(s64) out_buffer_capacity, param_str.count));
        return true;
    }
}

#program_export
param_text_to_value :: (plugin: *clap_plugin_t,
                        param_id: clap_id,
                        param_value_text: *char,
                        out_value: *float64) -> bool #c_call {
    using player_state := from_plugin(plugin);
    push_context main_context {
        param_str := to_string(param_value_text, c_style_strlen(param_value_text));
        value, success, _ := string_to_float(param_str);

        if ! success { return false; }

        out_value.* = value;
        return true;
    }{}
}

#program_export
params_flush :: (plugin: *clap_plugin_t, in: *clap_input_events_t, out: *clap_output_events_t) -> void #c_call {
    // @TODO
    return;
}

#import "clap-jai";
Hash_Table :: #import "Hash_Table";
