cmake_minimum_required(VERSION 3.13)
project(FMADecomposePlugin)

find_package(LLVM REQUIRED CONFIG)

add_definitions(${LLVM_DEFINITIONS})
include_directories(${LLVM_INCLUDE_DIRS})
link_directories(${LLVM_LIBRARY_DIRS})

add_library(fmadplugin SHARED fmadplugin.cpp)

set_target_properties(fmadplugin PROPERTIES POSITION_INDEPENDENT_CODE ON)

target_link_libraries(fmadplugin PRIVATE ${LLVM_LIBS})

set_target_properties(fmadplugin PROPERTIES
    LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib
)