#import "Basic";
#import "Compiler";
#import "generate_cpp_header";

SRC_FILES :: string.[
    "src/player.jai",
    "src/audio_ports.jai",
];

build :: (building_arm64 := false, lib_postfix := "") {
    w := compiler_create_workspace();

    target_options := get_build_options(w);
    target_options.output_executable_name = tprint("%0%", "file_player_jai_lib", lib_postfix);
    target_options.output_type = .STATIC_LIBRARY; // specifies output to be a static library
    target_options.backend = .LLVM;
    if building_arm64 {
        target_options.cpu_target = .ARM64;
        target_options.llvm_options.target_system_triple = "arm64-apple-darwin21.6.0";
    }
    target_options.text_output_flags = 1;

    cli_args := target_options.compile_time_command_line;
    for arg, idx: cli_args {
        if arg == "-release" {
            set_optimization(*target_options, .OPTIMIZED);
            target_options.stack_trace = false;
        } else if arg == "-quiet" {
            target_options.text_output_flags = 0;
        }
    }

    set_build_options(target_options, w);

    compiler_begin_intercept(w);
    for file, _ : SRC_FILES {
        add_build_file(tprint("%/%", #filepath, file), w);
    }

    if ! building_arm64 {
        cpp_header_info : Header_Info;
        array_add(*cpp_header_info.types_to_skip,
            "clap_version_t",
            "clap_plugin_descriptor_t",
            "clap_event_header_t",
            "clap_event_transport_t",
            "clap_audio_buffer_t",
            "clap_input_events_t",
            "clap_output_events_t",
            "clap_process_t",
            "clap_plugin_t",
            "clap_process_status",
            "clap_transport_flags",
            "clap_audio_port_info_t");
        while true {
            message := compiler_wait_for_message();
            handle_message(*cpp_header_info, message);
            if message.kind == {
              case .COMPLETE;
                break;
            }
        }
        
        generate_header(*cpp_header_info, "file_player_jai_lib.h");
    }

    compiler_end_intercept(w);

    set_build_options_dc(.{do_output=false});  // No executable for this workspace.
}

#if OS == .MACOS {
    #run {
        print("Building universal binary on MacOS...\n");

        arm_lib := "file_player_jai_lib_arm64.a";
        x64_lib := "file_player_jai_lib_x64.a";

        File.file_delete(arm_lib);
        File.file_delete(x64_lib);

        build(false, "_x64");
        build(true, "_arm64");

        // For some reason we need to wait a second for the ARM build to finish?
        sleep_milliseconds(100);

        lipo_command := string.["lipo", "-create", "-output", "file_player_jai_lib.a", x64_lib, arm_lib];
        Process.run_command(..lipo_command);

        lipo_check_command := string.["lipo", "-info", "file_player_jai_lib.a"];
        Process.run_command(..lipo_check_command);
    }

    Process :: #import "Process";
    File :: #import "File";
} else {
    #run build();
}
