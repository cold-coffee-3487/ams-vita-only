cmake_minimum_required(VERSION 3.15)
project(ams-vita CXX)

add_library(ams_vita INTERFACE)
target_compile_features(ams_vita INTERFACE cxx_std_20)
target_include_directories(ams_vita INTERFACE
    ${CMAKE_CURRENT_LIST_DIR}/include
)

if(CMAKE_SOURCE_DIR STREQUAL PROJECT_SOURCE_DIR)
    set(AMS_VITA_BUILD_TESTS_DEFAULT ON)
else()
    set(AMS_VITA_BUILD_TESTS_DEFAULT OFF)
endif()

option(AMS_VITA_BUILD_TESTS "Build AMS VITA tests" ${AMS_VITA_BUILD_TESTS_DEFAULT})

if(AMS_VITA_BUILD_TESTS)
    enable_testing()
    add_subdirectory(tests)
endif()
