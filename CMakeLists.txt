
project(SToolbox)
cmake_minimum_required(VERSION 2.8)

set(path_stb_root ${CMAKE_CURRENT_SOURCE_DIR})
set(path_stb_src ${path_stb_root}/src)
set(path_boost ${CMAKE_CURRENT_SOURCE_DIR}/libs/boost)
set(path_glm ${CMAKE_CURRENT_SOURCE_DIR}/libs/glm/)
set(path_freetype ${CMAKE_CURRENT_SOURCE_DIR}/libs/freetype)
set(path_gl ${CMAKE_CURRENT_SOURCE_DIR}/libs/gl)
set(path_sdl ${CMAKE_CURRENT_SOURCE_DIR}/libs/sdl2)
set(path_freetype ${CMAKE_CURRENT_SOURCE_DIR}/libs/freetype)

if (${CMAKE_SYSTEM_NAME} MATCHES "Linux")
    #-----------------------------------LINUX-----------------------------------#
    include_directories(SYSTEM
        ${path_boost}/include
    )

    # Compilation flags
    # SToolbox macro for platform
    set(flags_c "-DSTB_LINUX")

    set(flags_c "-Wall -Wextra -pedantic")
    set(flags_cc "-Wall -Wextra -pedantic -std=c++11")

    if (${CMAKE_BUILD_TYPE} MATCHES "Debug")
        set(flags_c "${flags_c} -g")
        set(flags_cc "${flags_cc} -g")
    endif()

    set(lib_sdl ${path_sdl}/lib/libSDL2.a)
    set(lib_common
        GL
        m
        dl
        rt
        pthread
        )

    set(lib_boost_log "${path_boost}/lib/libboost_log.a")
    set(lib_boost_thread "${path_boost}/lib/libboost_thread.a")
    set(lib_boost_system "${path_boost}/lib/libboost_system.a")
    set(lib_boost_chrono "${path_boost}/lib/libboost_chrono.a")
    set(lib_boost_options "${path_boost}/lib/libboost_program_options.a")
    set(lib_boost_unit_test "${path_boost}/lib/libboost_unit_test_framework.a")

    #-----------------------------------LINUX-----------------------------------#
endif()

if (${CMAKE_SYSTEM_NAME} MATCHES "Windows")
    #-----------------------------------Windows-----------------------------------#
    # Compilation flags
    # SToolbox macro for platform
    set(flags_c "-DSTB_WINDOWS")

    # Warning level
    set(flags_c "${flags_c} -W4")

    # Warning from GLM for the usage of nameless structs
    set(flags_c "${flags_c} /wd4201")

    # Disable Windows warnings for unsafe std warnings
    set(flags_c "${flags_c} -D_CRT_SECURE_NO_WARNINGS")

    #Boost
    set(flags_c "${flags_c} /wd4714")
    set(flags_c "${flags_c} /wd4512")
    set(flags_c "${flags_c} /wd4714")
    set(flags_c "${flags_c} /wd4512")
    set(flags_cc "${flags_c}")

    set(boost_version 1_57)
    include_directories(SYSTEM
        "${path_boost}/include/boost-${boost_version}/"
    )
    set(boost_vc_version "vc120")
    set(boost_prefix "${path_boost}/lib/libboost")
    if (${CMAKE_BUILD_TYPE} MATCHES "Debug")
        set(boost_postfix "${boost_vc_version}-mt-gd-${boost_version}.lib")
    else()
        set(boost_postfix "${boost_vc_version}-mt-${boost_version}.lib")
    endif()

    set(lib_boost_log "${boost_prefix}_log-${boost_postfix}")
    set(lib_boost_thread "${boost_prefix}_thread-${boost_postfix}")
    set(lib_boost_system "${boost_prefix}_system-${boost_postfix}")
    set(lib_boost_chrono "${boost_prefix}_chrono-${boost_postfix}")
    set(lib_boost_options "${boost_prefix}_program_options-${boost_postfix}")
    set(lib_boost_filesys "${boost_prefix}_filesystem-${boost_postfix}")
    set(lib_boost_datetime "${boost_prefix}_date_time-${boost_postfix}")
    set(lib_boost_unit_test "${boost_prefix}_unit_test_framework-${boost_postfix}")

    set(freetype_version 550)
    set(lib_freetype ${path_freetype}/lib/freetype${freetype_version}.lib)

    set(lib_common
        "OpenGL32"
        "winmm"
        "version"
        "Imm32"
    )
    set(lib_sdl
        "${path_sdl}/lib/SDL2.lib"
        "${path_sdl}/lib/SDL2main.lib"
    )
    #-----------------------------------Windows-----------------------------------#
endif()

#-----------------------------------Common-----------------------------------#

set(path_gl_src "${path_gl}/gl_stb.c")
set(log_boost_inc "${path_stb_src}/stb_log_boost.cc")
set(log_boost_src "${path_stb_src}/stb_log_boost.cc")

include_directories(SYSTEM
    "${path_glm}/include"
    "${path_freetype}/include"
    "${path_gl}"
    "${path_sdl}/include"
    "${path_freetype}/include"
    )

include_directories(
    "${CMAKE_CURRENT_SOURCE_DIR}/inc"
    )

#-----------------------------------Common-----------------------------------#

#-----------------------------------Functions-----------------------------------#

function(stb_set_compile_flags)
    foreach(file ${ARGN})
        if (${file} MATCHES ".cc")
            set_source_files_properties(${file}
                PROPERTIES COMPILE_FLAGS ${flags_cc}
                )
        elseif(${file} MATCHES ".c")
            set_source_files_properties(${file}
                PROPERTIES COMPILE_FLAGS ${flags_c}
                )
        endif()
    endforeach()
endfunction()

#-----------------------------------Functions-----------------------------------#

add_subdirectory(protos)
add_subdirectory(tests)
