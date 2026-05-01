// RUN: mlir-opt -load-pass-plugin=%mlir_lib_dir/konstantinov_s_lab4_MLIR%shlibext \
// RUN:   --pass-pipeline="builtin.module(func.func(konstantinov_s_lab4_MLIR))" %s | FileCheck %s

// CHECK-LABEL: func.func @just_function
// CHECK-SAME: max_block_depth = 0 : i64
func.func @just_function(%x: i32, %y: i32) -> i32 {
  %result = arith.muli %x, %y : i32
  return %result : i32
}
