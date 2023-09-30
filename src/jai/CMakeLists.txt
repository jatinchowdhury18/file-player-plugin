if(WIN32)
    set(JAI_LIBRARY_FILE "file_player_jai_lib.lib")
else()
    set(JAI_LIBRARY_FILE "file_player_jai_lib.a")
endif()

set(JAI_SOURCES
    "build.jai"
    "src/audio_ports.jai"
    "src/params.jai"
    "src/player.jai"
    "src/plugin.jai"
    "src/process.jai"
    "src/interpolation.jai"
)
add_custom_command(OUTPUT ${CMAKE_CURRENT_SOURCE_DIR}/${JAI_LIBRARY_FILE}
        COMMAND ${JAI_COMPILER} build.jai
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
        DEPENDS ${JAI_SOURCES}
)
add_custom_target(jai_library_build DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/${JAI_LIBRARY_FILE})

add_library(file_player_jai_lib STATIC IMPORTED GLOBAL)
add_dependencies(file_player_jai_lib jai_library_build)

set_target_properties(file_player_jai_lib
    PROPERTIES
    IMPORTED_LOCATION ${CMAKE_CURRENT_SOURCE_DIR}/${JAI_LIBRARY_FILE}
)

target_link_libraries(FilePlayerPlugin PRIVATE file_player_jai_lib)
target_sources(FilePlayerPlugin PRIVATE JaiContext.cpp)
