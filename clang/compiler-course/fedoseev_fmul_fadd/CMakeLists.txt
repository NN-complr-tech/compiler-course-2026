cmake_minimum_required(VERSION 3.13)
project(FMADecomposePlugin)

option(ENABLE_ADDRESS_SANITIZER "Enable Address Sanitizer" OFF)

find_package(LLVM REQUIRED CONFIG)

add_definitions(${LLVM_DEFINITIONS})
include_directories(${LLVM_INCLUDE_DIRS})
link_directories(${LLVM_LIBRARY_DIRS})

add_library(fmadplugin SHARED fmadplugin.cpp)

target_compile_options(fmadplugin PRIVATE -fPIC)
target_link_libraries(fmadplugin PRIVATE ${LLVM_LIBS})

if(ENABLE_ADDRESS_SANITIZER)
    target_compile_options(fmadplugin PRIVATE -fsanitize=address -g)
    target_link_options(fmadplugin PRIVATE -fsanitize=address)
endif()

set_target_properties(fmadplugin PROPERTIES
    LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib
)