cmake_minimum_required(VERSION 3.13)
project(rychkova_lab2)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Ищем LLVM
find_package(LLVM REQUIRED CONFIG)

# Добавляем пути к заголовкам LLVM
include_directories(${LLVM_INCLUDE_DIRS})
add_definitions(${LLVM_DEFINITIONS})

set(TARGET_NAME "rychkova_lab2_LLVM_IR")
set(SOURCES pass.cpp)

add_library(${TARGET_NAME} SHARED ${SOURCES})

# Линкуем с библиотеками LLVM
target_link_libraries(${TARGET_NAME}
    LLVM
    LLVMCore
    LLVMSupport
)

if(UNIX)
    set_target_properties(${TARGET_NAME} PROPERTIES 
        PREFIX "lib" 
        SUFFIX ".so"
    )
endif()

message(STATUS "Building ${TARGET_NAME} with LLVM ${LLVM_PACKAGE_VERSION}")