cmake_minimum_required(VERSION 3.13)
project(ReplacePowi)

find_program(LLVM_CONFIG_EXECUTABLE llvm-config)
if(LLVM_CONFIG_EXECUTABLE)
    execute_process(COMMAND ${LLVM_CONFIG_EXECUTABLE} --cmakedir
                    OUTPUT_VARIABLE LLVM_CMAKE_DIR
                    OUTPUT_STRIP_TRAILING_WHITESPACE)
    set(LLVM_DIR ${LLVM_CMAKE_DIR})
endif()

if(NOT LLVM_DIR)
    set(LLVM_DIR "/usr/lib/llvm-18/lib/cmake/llvm" CACHE PATH "Path to LLVM CMake files")
endif()

find_package(LLVM REQUIRED CONFIG)

message(STATUS "Found LLVM ${LLVM_PACKAGE_VERSION}")
message(STATUS "Using LLVMConfig.cmake in: ${LLVM_DIR}")

add_definitions(${LLVM_DEFINITIONS})
include_directories(${LLVM_INCLUDE_DIRS})
link_directories(${LLVM_LIBRARY_DIRS})

add_library(ReplacePowi MODULE ReplacePowi.cpp)

target_link_libraries(ReplacePowi ${LLVM_LIBRARIES})

set_target_properties(ReplacePowi PROPERTIES
    CXX_STANDARD 17
    CXX_STANDARD_REQUIRED ON
    PREFIX ""
)
