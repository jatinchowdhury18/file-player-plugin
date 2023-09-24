target_sources(FilePlayerPlugin PRIVATE
    file_player_plugin.cpp

    plugin_editor.cpp
    file_player_editor.cpp
)

if(APPLE)
    target_sources(FilePlayerPlugin PRIVATE plugin_editor.mm)
endif()

include(FindJaiCompiler)
if (NOT(${JAI_COMPILER} STREQUAL "JAI_COMPILER-NOTFOUND"))
    message(STATUS "Configuring Jai compilation!")
    add_subdirectory(jai)
endif()
