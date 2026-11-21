find_package(Git REQUIRED)
include(FetchContent)

set(pacing_patch "${CMAKE_CURRENT_SOURCE_DIR}/rexglue-frame-pacing.patch")
set_property(DIRECTORY APPEND PROPERTY CMAKE_CONFIGURE_DEPENDS "${pacing_patch}")
set(rexglue_revision bd833a2ab69d305f99ddf57418fc38ffec8af93d)
file(SHA256 "${pacing_patch}" pacing_patch_hash)
string(SHA256 pacing_source_hash "${rexglue_revision}:${pacing_patch_hash}")
string(SUBSTRING "${pacing_source_hash}" 0 12 pacing_source_id)

# Start from the pinned source whenever the local patch changes.
set(FETCHCONTENT_UPDATES_DISCONNECTED_REXGLUE_FRAME_PACING ON)
FetchContent_Declare(rexglue_frame_pacing
    GIT_REPOSITORY https://github.com/rexglue/rexglue-sdk.git
    GIT_TAG ${rexglue_revision}
    GIT_SUBMODULES ""
    SOURCE_DIR "${CMAKE_CURRENT_BINARY_DIR}/rexglue-${pacing_source_id}"
    SOURCE_SUBDIR cmake
)
FetchContent_MakeAvailable(rexglue_frame_pacing)
set(REXGLUE_ROOT "${rexglue_frame_pacing_SOURCE_DIR}")

execute_process(
    COMMAND "${GIT_EXECUTABLE}" apply --reverse --check "${pacing_patch}"
    WORKING_DIRECTORY "${REXGLUE_ROOT}"
    RESULT_VARIABLE patch_applied
    OUTPUT_QUIET ERROR_QUIET
)
if(NOT patch_applied EQUAL 0)
    execute_process(
        COMMAND "${GIT_EXECUTABLE}" apply "${pacing_patch}"
        WORKING_DIRECTORY "${REXGLUE_ROOT}"
        COMMAND_ERROR_IS_FATAL ANY
    )
endif()

set(REXGLUE_USE_D3D12 ON)
set(REXGLUE_USE_VULKAN OFF)
add_library(rexruntime ALIAS rex::runtime)
add_library(xxHash::xxhash ALIAS rex::xxhash)

function(rex_target_pch target)
    target_precompile_headers(${target} PRIVATE ${ARGN})
endfunction()
function(rexglue_add_version_resource target)
endfunction()

add_subdirectory("${REXGLUE_ROOT}/src/graphics" graphics)
target_include_directories(rexgpu-xenos PRIVATE "${REXGLUE_ROOT}/include")
target_compile_features(rexgpu-xenos PRIVATE cxx_std_23)
target_link_libraries(rexgpu-xenos PRIVATE synchronization)
rexglue_apply_target_settings(rexgpu-xenos)
