CPMAddPackage(
    NAME clap
    GITHUB_REPOSITORY free-audio/clap
    GIT_TAG 1.1.8
)

CPMAddPackage(
    NAME clap-helpers
    GITHUB_REPOSITORY free-audio/clap-helpers
    GIT_TAG ab8d2585872e603f41a41116f019cf2893b9b396
)

set(CLAP_WRAPPER_DOWNLOAD_DEPENDENCIES TRUE CACHE BOOL "Get em")
set(CLAP_WRAPPER_DONT_ADD_TARGETS TRUE CACHE BOOL "I'll targetize")
set(CLAP_WRAPPER_BUILD_AUV2 TRUE CACHE BOOL "It's only logical")
CPMAddPackage(
    NAME clap-wrapper
    GITHUB_REPOSITORY free-audio/clap-wrapper
    GIT_TAG a8d4892187387df8bcc64bc81bbe3a20ed005dad
)
if (MSVC)
    target_compile_options(clap-wrapper-shared-detail INTERFACE /Zc:char8_t-)
else()
    target_compile_options(clap-wrapper-shared-detail INTERFACE -fno-char8_t)
endif()

function(target_library_make_clap)
    set(oneValueArgs
        TARGET
        CLAP_NAME
        CLAP_BUNDLE_ID
        CLAP_BUNDLE_VERSION
    )
    cmake_parse_arguments(TCLP "" "${oneValueArgs}" "" ${ARGN} )

    if (NOT DEFINED TCLP_TARGET)
        message(FATAL_ERROR "You must define TARGET in target_library_is_clap")
    endif()

    if (NOT DEFINED TCLP_CLAP_NAME)
        message(STATUS "Using target name as clap name in target_libarry_is_clap")
        set(TCLP_CLAP_NAME TCLP_TARGET)
    endif()

    if (NOT DEFINED TCLP_CLAP_BUNDLE_ID)
        message(STATUS "Using sst default macos bundle prefix in target_libary_is_clap")
        set(TCLP_CLAP_BUNDLE_ID "org.surge-synth-team.${TCLP_TARGET}")
    endif()

    if (NOT DEFINED TCLP_CLAP_BUNDLE_VERSION)
        message(STATUS "No bundle version in target_library_is_clap; using 0.1")
        set(TCLP_CLAP_BUNDLE_VERSION "0.1")
    endif()

    if(APPLE)
        set_target_properties(${TCLP_TARGET} PROPERTIES
            BUNDLE True
            BUNDLE_EXTENSION clap
            LIBRARY_OUTPUT_NAME ${TCLP_CLAP_NAME}
            MACOSX_BUNDLE_GUI_IDENTIFIER ${TCLP_CLAP_BUNDLE_ID}
            MACOSX_BUNDLE_BUNDLE_NAME ${TCLP_CLAP_NAME}
            MACOSX_BUNDLE_BUNDLE_VERSION "${TCLP_CLAP_BUNDLE_VERSION}"
            MACOSX_BUNDLE_SHORT_VERSION_STRING "${TCLP_CLAP_MACOS_BUNDLE_VERSION}"
            MACOSX_BUNDLE_INFO_PLIST ${CMAKE_CURRENT_SOURCE_DIR}/cmake/generic-clap.plist.in
        )

        if (NOT ${CMAKE_GENERATOR} STREQUAL "Xcode")
            add_custom_command(TARGET ${TCLP_TARGET} POST_BUILD
                COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_CURRENT_SOURCE_DIR}/cmake/PkgInfo "$<TARGET_FILE_DIR:${TCLP_TARGET}>/../"
            )
        endif()
        set_target_properties(${TCLP_TARGET} PROPERTIES
            MACOSX_BUNDLE TRUE
            XCODE_ATTRIBUTE_GENERATE_PKGINFO_FILE "YES"
        )
    elseif(UNIX)
        set_target_properties(${TCLP_TARGET} PROPERTIES OUTPUT_NAME ${TCLP_CLAP_NAME} SUFFIX ".clap" PREFIX "")
    else()
        set_target_properties(${TCLP_TARGET}
            PROPERTIES
            OUTPUT_NAME ${TCLP_CLAP_NAME}
            SUFFIX ".clap" PREFIX ""
        )
    endif()
endfunction(target_library_make_clap)
