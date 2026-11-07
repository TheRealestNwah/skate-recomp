cmake_minimum_required(VERSION 3.25)
project(skate_research_tools LANGUAGES CXX)

find_package(rexglue 0.10.1 EXACT CONFIG REQUIRED)
add_executable(inspect_xex inspect_xex.cpp)
target_compile_features(inspect_xex PRIVATE cxx_std_23)
target_link_libraries(inspect_xex PRIVATE rex::runtime)
rexglue_apply_target_settings(inspect_xex)
add_custom_command(TARGET inspect_xex POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        $<TARGET_RUNTIME_DLLS:inspect_xex> $<TARGET_FILE_DIR:inspect_xex>
    COMMAND_EXPAND_LISTS
    VERBATIM
)
