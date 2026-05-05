// RUN: mlir-opt -load-pass-plugin=%mlir_lib_dir/lifanov_k_mlir_MLIR%shlibext \
// RUN:   --pass-pipeline="builtin.module(lifanov_k_mlir_MLIR)" %s | FileCheck %s

// CHECK-DAG: func.func private @trace_condition_then_begin()
// CHECK-DAG: func.func private @trace_condition_then_end()
// CHECK-DAG: func.func private @trace_condition_else_begin()
// CHECK-DAG: func.func private @trace_condition_else_end()

// CHECK-LABEL: func.func @check_scf_if
func.func @check_scf_if(%flag: i1) {
  // CHECK: scf.if
  scf.if %flag {
    // CHECK-NEXT: func.call @trace_condition_then_begin()
    %v1 = arith.constant 10 : i32
    // CHECK: func.call @trace_condition_then_end()
    // CHECK-NEXT: } else {
  } else {
    // CHECK-NEXT: func.call @trace_condition_else_begin()
    %v2 = arith.constant 20 : i32
    // CHECK: func.call @trace_condition_else_end()
    // CHECK-NEXT: }
  }
  return
}

// CHECK-LABEL: func.func @check_affine_if
func.func @check_affine_if(%idx: index) {
  // CHECK: affine.if
  affine.if affine_set<(d0) : (d0 - 5 >= 0)>(%idx) {
    // CHECK-NEXT: func.call @trace_condition_then_begin()
    %a = arith.constant 1 : i32
    // CHECK: func.call @trace_condition_then_end()
    // CHECK-NEXT: } else {
  } else {
    // CHECK-NEXT: func.call @trace_condition_else_begin()
    %b = arith.constant 0 : i32
    // CHECK: func.call @trace_condition_else_end()
    // CHECK-NEXT: }
  }
  return
}
