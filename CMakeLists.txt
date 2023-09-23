cmake_minimum_required(VERSION 3.15)
set(CMAKE_OSX_DEPLOYMENT_TARGET "10.15" CACHE STRING "Minimum OS X deployment target")
if(WIN32)
    set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
    set(CMAKE_SYSTEM_VERSION 7.1 CACHE STRING INTERNAL FORCE) # Windows SDK for Windows 7 and up
    set(CMAKE_DEBUG_POSTFIX "")
    message(STATUS "CMAKE_DEBUG_POSTFIX: ${CMAKE_DEBUG_POSTFIX}")
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

CPMAddPackage(
    NAME readerwriterqueue
    GITHUB_REPOSITORY cameron314/readerwriterqueue
    GIT_TAG v1.0.6
)
add_library(FilePlayerPlugin STATIC)
target_link_libraries(FilePlayerPlugin PUBLIC clap clap-helpers juce_libs readerwriterqueue)
add_subdirectory(src)

set_source_files_properties(src/plugin_entry.cpp PROPERTIES COMPILE_FLAGS "-I${clap-wrapper_SOURCE_DIR}/include")

add_library(FilePlayerPlugin_CLAP MODULE src/plugin_entry.cpp)
target_link_libraries(FilePlayerPlugin_CLAP PUBLIC FilePlayerPlugin)
target_library_make_clap(TARGET FilePlayerPlugin_CLAP
    CLAP_NAME "File Player Plugin"
    CLAP_BUNDLE_ID com.chowdsp.file-player
    CLAP_BUNDLE_VERSION ${PROJECT_VERSION}
)

add_library(FilePlayerPlugin_VST3 MODULE src/plugin_entry.cpp)
target_link_libraries(FilePlayerPlugin_VST3 PUBLIC FilePlayerPlugin)
target_add_vst3_wrapper(TARGET FilePlayerPlugin_VST3 OUTPUT_NAME "File Player Plugin")
#if (WIN32)
#    # Question - do we want this default in the helpers
#    set_target_properties(${VST3_TARGET}
#        PROPERTIES
#        LIBRARY_OUTPUT_DIRECTORY VST3
#    )
#endif()

if (APPLE)
    add_library(FilePlayerPlugin_AU MODULE src/plugin_entry.cpp)
    target_link_libraries(FilePlayerPlugin_AU PUBLIC FilePlayerPlugin)
    target_add_auv2_wrapper(
        TARGET FilePlayerPlugin_AU
        OUTPUT_NAME "File Player Plugin"
        BUNDLE_IDENTIFIER "com.chowdsp.file-player"
        BUNDLE_VERSION ${PROJECT_VERSION}
        CLAP_TARGET_FOR_CONFIG FilePlayerPlugin_CLAP
    )
endif()

add_executable(FilePlayerPlugin_Standalone)
target_sources(FilePlayerPlugin_Standalone PRIVATE src/plugin_entry.cpp)
target_link_libraries(FilePlayerPlugin_Standalone PRIVATE FilePlayerPlugin)
target_add_standalone_wrapper(TARGET FilePlayerPlugin_Standalone
    OUTPUT_NAME "File Player Plugin"
    STATICALLY_LINKED_CLAP_ENTRY True
    PLUGIN_ID "com.chowdsp.file-player")
