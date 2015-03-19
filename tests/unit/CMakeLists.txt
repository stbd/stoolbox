#------------------------ Model tests ------------------------#
set(unit_test_model_src
    ${CMAKE_CURRENT_SOURCE_DIR}/model_tests.cc
    ${path_stb_src}/stb_model.cc
    ${path_stb_src}/stb_error.cc
    )

add_executable(unit_test_model ${unit_test_model_src})

target_link_libraries(unit_test_model
    ${lib_boost_unit_test}
    )

stb_set_compile_flags(${unit_test_model_src})

#------------------------ Error state tests ------------------------#
set(unit_test_error_src
    ${CMAKE_CURRENT_SOURCE_DIR}/error_state_tests.cc
    ${path_stb_src}/stb_error.cc
    )

add_executable(unit_test_error ${unit_test_error_src})

target_link_libraries(unit_test_error
    ${lib_boost_unit_test}
    )

stb_set_compile_flags(${unit_test_error_src})

#------------------------ Buffer tests ------------------------#
set(unit_test_buffer_src
    ${CMAKE_CURRENT_SOURCE_DIR}/buffer_tests.cc
    ${path_stb_src}/stb_buffer.cc
    )

add_executable(unit_test_buffer ${unit_test_buffer_src})

target_link_libraries(unit_test_buffer
    ${lib_boost_unit_test}
    )

stb_set_compile_flags(${unit_test_buffer_src})
