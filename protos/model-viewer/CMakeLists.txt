set(model_view_src
    ${CMAKE_CURRENT_SOURCE_DIR}/main.cc
    ${path_stb_src}/stb_gl.cc
    ${path_stb_src}/stb_gl_shader.cc
    ${path_stb_src}/stb_gl_object.cc
    ${path_stb_src}/stb_model.cc
    ${path_stb_src}/stb_error.cc
    ${path_stb_src}/stb_buffer.cc
    ${log_boost_src}
    )

add_executable(model_viewer ${model_view_src})

stb_set_compile_flags(${model_view_src})

target_link_libraries(model_viewer
    ${lib_common}
    ${lib_gl}
    ${lib_sdl}
    ${lib_boost_log}
    ${lib_boost_thread}
    ${lib_boost_system}
    ${lib_boost_filesys}
    ${lib_boost_datetime}
    ${lib_boost_chrono}
    ${lib_boost_options}
    lib_gl
)
