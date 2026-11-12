cmake_minimum_required(VERSION 3.25)
project(skate_fast_codegen LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin")

find_package(rexglue 0.10.1 EXACT CONFIG REQUIRED)
find_package(Git REQUIRED)
include(FetchContent)

FetchContent_Declare(rexglue_source
    GIT_REPOSITORY https://github.com/rexglue/rexglue-sdk.git
    GIT_TAG bd833a2ab69d305f99ddf57418fc38ffec8af93d
    GIT_SUBMODULES thirdparty/cli11 thirdparty/inja
    PATCH_COMMAND "${GIT_EXECUTABLE}" apply "${CMAKE_CURRENT_SOURCE_DIR}/rexglue-gap-fill.patch"
    SOURCE_SUBDIR cmake
)
FetchContent_MakeAvailable(rexglue_source)
set(REXGLUE_ROOT "${rexglue_source_SOURCE_DIR}")

get_target_property(runtime_dll rex::runtime IMPORTED_LOCATION_RELEASE)
configure_file("${runtime_dll}" "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/rexruntime.dll" COPYONLY)

add_library(rexruntime ALIAS rex::runtime)
add_library(tomlplusplus ALIAS rex::tomlplusplus)
add_library(disasm ALIAS rex::disasm)
add_library(xxHash::xxhash ALIAS rex::xxhash)
add_library(simde ALIAS rex::simde)

# The two SDK subdirectories normally receive these helpers from the SDK root.
function(rex_target_pch target)
    target_precompile_headers(${target} PRIVATE ${ARGN})
endfunction()
function(rex_config_postfix target)
endfunction()
function(rexglue_add_version_resource target)
endfunction()

add_subdirectory("${REXGLUE_ROOT}/src/codegen" codegen)
add_subdirectory("${REXGLUE_ROOT}/src/rexglue" cli)
rexglue_apply_target_settings(rexcodegen)
rexglue_apply_target_settings(rexglue)
