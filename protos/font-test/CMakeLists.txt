
set(font_src
    ${CMAKE_CURRENT_SOURCE_DIR}/main.cc
    ${path_stb_src}/stb_error.cc
    ${path_stb_src}/stb_gl.cc
    ${path_stb_src}/stb_gl_shader.cc
    ${path_stb_src}/stb_buffer.cc
    ${path_stb_src}/stb_gl_object.cc
    ${path_stb_src}/stb_model.cc
    ${path_stb_src}/stb_font.cc
    ${path_stb_src}/stb_text_hud.cc
    ${log_boost_src}
    ${path_stb_src}/stb_model.cc
    )

add_executable(font
    ${font_src}
    )

target_link_libraries(font
    ${lib_common}
    ${lib_sdl}
    ${lib_freetype}
    ${lib_boost_log}
    ${lib_boost_thread}
    ${lib_boost_system}
    ${lib_boost_filesys}
    ${lib_boost_datetime}
    ${lib_boost_chrono}
    lib_gl
    )

stb_set_compile_flags(${font_src})
