get_filename_component(PASS_DIR ${CMAKE_CURRENT_SOURCE_DIR} NAME)
set(PLUGIN_NAME "${PASS_DIR}_MLIR")

file(GLOB_RECURSE PASS_SRC
    *.cpp
    *.h
    *.hpp
)

add_llvm_pass_plugin(${PLUGIN_NAME}
    ${PASS_SRC}
    DEPENDS
        intrinsics_gen
        MLIRBuiltinLocationAttributesIncGen
    BUILDTREE_ONLY
)

target_link_libraries(${PLUGIN_NAME}
    PRIVATE
        MLIRFuncDialect
        MLIRSCFDialect
        MLIRAffineDialect
        MLIRPass
        MLIRTransforms
)

set(MLIR_TEST_DEPENDS
    ${PLUGIN_NAME}
    ${MLIR_TEST_DEPENDS}
    PARENT_SCOPE
)
