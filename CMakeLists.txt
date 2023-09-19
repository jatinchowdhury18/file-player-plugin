cmake_minimum_required(VERSION 3.15)
set(CMAKE_OSX_DEPLOYMENT_TARGET "10.12" CACHE STRING "Minimum OS X deployment target")
if(WIN32)
    set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
    set(CMAKE_SYSTEM_VERSION 7.1 CACHE STRING INTERNAL FORCE) # Windows SDK for Windows 7 and up
endif()
project(FilePlayerPlugin VERSION 1.0.0 LANGUAGES C CXX)

if (APPLE)
    enable_language(OBJC)
    enable_language(OBJCXX)
    set(CMAKE_OBJC_VISIBILITY_PRESET hidden)
    set(CMAKE_OBJCXX_VISIBILITY_PRESET hidden)
endif()

set(BUILD_SHARED_LIBS OFF CACHE BOOL "Never want shared if not specified")
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_VISIBILITY_PRESET hidden)
set(CMAKE_VISIBILITY_INLINES_HIDDEN ON)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${CMAKE_SOURCE_DIR}/cmake/)

include(CPM)
include(ClapLibs)
include(JuceLib)

add_library(FilePlayerPlugin STATIC)
target_link_libraries(FilePlayerPlugin PUBLIC clap clap-helpers juce_libs)
add_subdirectory(src)

add_library(FilePlayerPlugin_CLAP MODULE
    src/plugin_entry.cpp
)
target_link_libraries(FilePlayerPlugin_CLAP PUBLIC FilePlayerPlugin)
target_library_make_clap(TARGET FilePlayerPlugin_CLAP
    CLAP_NAME "File Player Plugin"
    CLAP_BUNDLE_ID com.chowdsp.file-player-plugin
    CLAP_BUNDLE_VERSION ${PROJECT_VERSION}
)
