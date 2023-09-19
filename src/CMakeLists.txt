target_sources(FilePlayerPlugin PRIVATE
    file_player_plugin.cpp
)

if(APPLE)
    target_sources(FilePlayerPlugin PRIVATE plugin_editor.mm)
else()
    target_sources(FilePlayerPlugin PRIVATE plugin_editor.cpp)
endif()
