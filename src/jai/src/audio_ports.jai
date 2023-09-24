#program_export
audio_ports_count :: (plugin: *clap_plugin_t, is_input: bool) -> u32 #c_call {
    return 1;
}

#program_export
audio_ports_get :: (plugin: *clap_plugin_t, index: u32, is_input: bool, info: *clap_audio_port_info_t) -> bool #c_call {
    if index > 0 { return false; }

    info.channel_count = 2;
    info.id = 1;
    info.in_place_pair = 1;
    info.flags = CLAP_AUDIO_PORT_IS_MAIN;
    memcpy(info.name.data, "main\0".data, 5);
    return true;
}

#import "clap-jai";
