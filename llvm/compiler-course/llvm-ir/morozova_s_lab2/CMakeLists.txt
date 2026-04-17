get_filename_component(FOLDER_ID ${CMAKE_CURRENT_SOURCE_DIR} NAME)
set(PLUGIN_ID "${FOLDER_ID}_LLVM_IR")

if (WIN32)
    add_library(${PLUGIN_ID} STATIC ReplacePowi.cpp)
    target_compile_definitions(${PLUGIN_ID} PRIVATE LLVM_EXPORTS)
    set(LLVM_TEST_DEPENDS ${PLUGIN_ID} ${LLVM_TEST_DEPENDS} PARENT_SCOPE)
else()
    add_llvm_pass_plugin(${PLUGIN_ID} ReplacePowi.cpp
        DEPENDS
        intrinsics_gen
        BUILDTREE_ONLY
    )
    set(LLVM_TEST_DEPENDS ${PLUGIN_ID} ${LLVM_TEST_DEPENDS} PARENT_SCOPE)
endif()

