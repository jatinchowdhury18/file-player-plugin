make_name :: (name_set: string) -> [CLAP_NAME_SIZE] u8 {
    name: [CLAP_NAME_SIZE] u8;
    assert(name_set.count < name.count);
    memset(name.data, 0, name.count);
    memcpy(name.data, name_set.data, name_set.count);
    return name;
}

Params_Metadata :: struct {
    gain_param : clap_param_info_t : .{
        id = 0,
        flags = .CLAP_PARAM_IS_AUTOMATABLE,
        cookie = null,
        name = #run make_name("Gain"),
        min_value = 0.0,
        max_value = 1.0,
        default_value = 0.5,
    };
    pitch_param : clap_param_info_t : .{
        id = 1,
        flags = .CLAP_PARAM_IS_AUTOMATABLE,
        cookie = null,
        name = #run make_name("Pitch"),
        min_value = 0.5,
        max_value = 2.0,
        default_value = 1.0,
    };
};

// @TODO: some kind of "walk metadata" macro/function for nested metadata structs

define_params :: ($metadata_type: Type) -> string {
    sb : String_Builder;
    init_string_builder(*sb);
    for member: type_info(metadata_type).members {
        if member.type != type_info(clap_param_info_t) continue;

        print_to_builder(*sb, "%1: Param = .{value = #run metadata.%1.default_value, id = #run metadata.%1.id};\n", member.name);
    }
    return builder_to_string(*sb);
}

Params :: struct {
    metadata : Params_Metadata : .{};

    params_count :: #run type_info(Params_Metadata).members.count; // @TODO this won't work for nested parameters

    Param :: struct {
        value: float64;
        modulation: float64 = 0.0;
        id: clap_id = CLAP_INVALID_ID;
    };

    #insert #run define_params(Params_Metadata);

    param_table: Hash_Table.Table(clap_id, *Param);
};

#program_export
params_count :: (plugin: *clap_plugin_t) -> u32 #c_call {
    return Params.params_count;
}

#program_export
param_get_info :: (plugin: *clap_plugin_t, param_index: u32, param_info: *clap_param_info_t) -> bool #c_call {
    using player_state := from_plugin(plugin);
    push_context main_context {
        if (param_index >= params.params_count) return false;

        // initialise the table if needed...
        if params.param_table.allocated == 0 {
            // So the hash table can't be 100% full, and to be performant, should be something like 70%
            // full. That said, if we know the parameter layout at compile-time, maybe we can do some
            // sort of perfect hashing? I think that would be both the most performant and the most
            // memory-efficient. 
            Hash_Table.init(*params.param_table, params.params_count * 3 / 2);
        }

        // find the param info and copy into place
        plugin_param_info: *clap_param_info_t;
        param_info_count := 0;
        for member: type_info(Params_Metadata).members {
            if member.type != type_info(clap_param_info_t) continue;
            if param_info_count == param_index {
                plugin_param_info = cast(*clap_param_info_t) (type_info(Params_Metadata).constant_storage.data + member.offset_into_constant_storage);
                break;
            }
            param_info_count += 1;
        }

        memcpy(param_info, plugin_param_info, size_of(clap_param_info_t));

        // look for the actual param struct in Params
        param_ptr : *Params.Param = null;
        for member: type_info(Params).members {
            if member.type != type_info(Params.Param) continue;

            candidate_ptr := cast(*Params.Param) (cast(*u8) (*params) + member.offset_in_bytes);
            assert(candidate_ptr.id != CLAP_INVALID_ID);

            if candidate_ptr.id == param_info.id {
                param_ptr = candidate_ptr;
                break;
            }
        }
        assert(param_ptr != null, tprint("Unable to find parameter with ID: %", param_info.id));

        param_info.cookie = param_ptr;

        param_table_ptr := Hash_Table.table_find_pointer(*params.param_table, param_info.id);
        if param_table_ptr == null {
            param_table_ptr = Hash_Table.table_add(*params.param_table,
                                                   param_info.id,
                                                   param_ptr);
        }
        
        return true;
    }
}

#program_export
param_get_value :: (plugin: *clap_plugin_t, param_id: clap_id, out_value: *float64) -> bool #c_call {
    using player_state := from_plugin(plugin);
    push_context main_context {
            param := Hash_Table.table_find_pointer(*params.param_table, param_id);
        if param == null { return false; }

        out_value.* = param.*.value;

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
        auto_release_temp();
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
