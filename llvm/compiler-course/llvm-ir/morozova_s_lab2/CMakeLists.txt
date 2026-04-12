cmake_minimum_required(VERSION 3.13)
project(ReplacePowi)

find_package(LLVM REQUIRED CONFIG)

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
