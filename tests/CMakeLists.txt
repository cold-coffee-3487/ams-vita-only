cmake_minimum_required(VERSION 3.15)
project(ams-vita-tests CXX)

if(NOT TARGET ams_vita)
    add_library(ams_vita INTERFACE)
    target_compile_features(ams_vita INTERFACE cxx_std_20)
    target_include_directories(ams_vita INTERFACE ${CMAKE_CURRENT_LIST_DIR}/../include)
endif()

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_library(ams_vita_strict_warnings INTERFACE)
target_compile_options(ams_vita_strict_warnings INTERFACE
    -Wall
    -Wextra
    -Wpedantic
    -Werror
    -Wconversion
    -Wsign-conversion
    -Wsign-compare
    -Wcast-align
    -Wcast-qual
    -Wdouble-promotion
    -Wformat=2
    -Wimplicit-fallthrough
    -Wnull-dereference
    -Wold-style-cast
    -Wpacked
    -Wredundant-decls
    -Wshadow
    -Wundef
    -Wunused
    -Wwrite-strings
    -Wnon-virtual-dtor
    -Woverloaded-virtual
    $<$<CXX_COMPILER_ID:GNU>:-Wduplicated-branches>
    $<$<CXX_COMPILER_ID:GNU>:-Wduplicated-cond>
    $<$<CXX_COMPILER_ID:GNU>:-Wlogical-op>
    $<$<CXX_COMPILER_ID:GNU>:-Wuseless-cast>
    $<$<CXX_COMPILER_ID:Clang>:-Wcomma>
    $<$<CXX_COMPILER_ID:Clang>:-Wconditional-uninitialized>
    $<$<CXX_COMPILER_ID:Clang>:-Wextra-semi>
    $<$<CXX_COMPILER_ID:Clang>:-Wnewline-eof>
    $<$<CXX_COMPILER_ID:Clang>:-Wunreachable-code>
)

option(AMS_VITA_ENABLE_SANITIZERS "Build tests with AddressSanitizer and UndefinedBehaviorSanitizer" OFF)
if(AMS_VITA_ENABLE_SANITIZERS)
    target_compile_options(ams_vita_strict_warnings INTERFACE -fsanitize=address,undefined -fno-omit-frame-pointer)
    target_link_options(ams_vita_strict_warnings INTERFACE -fsanitize=address,undefined)
endif()

enable_testing()

function(add_ams_vita_test target source)
    add_executable(${target} ${source})
    target_link_libraries(${target} PRIVATE ams_vita ams_vita_strict_warnings)
    add_test(NAME ${target} COMMAND ${target})
endfunction()

add_ams_vita_test(test_primitives test_primitives.cpp)
add_ams_vita_test(test_data_packet test_data_packet.cpp)
add_ams_vita_test(test_control_packet test_control_packet.cpp)
add_ams_vita_test(test_generated_roundtrip test_generated_roundtrip.cpp)
add_ams_vita_test(test_ack_packet test_ack_packet.cpp)
add_ams_vita_test(test_context_packet test_context_packet.cpp)
add_ams_vita_test(test_indicator_forbidden_ranges test_indicator_forbidden_ranges.cpp)
add_ams_vita_test(test_payload_consumption test_payload_consumption.cpp)
add_ams_vita_test(test_fixed_data_packet test_fixed_data_packet.cpp)
