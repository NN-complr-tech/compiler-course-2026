// RUN: mlir-opt -load-pass-plugin=%mlir_lib_dir//CopyToLoopPass_Tvoya_Familiya_FIIT1_MLIR%shlibext \
// RUN: --pass-pipeline="builtin.module(tvoya-copy-to-loop)" %s | FileCheck %s

// CHECK-LABEL: func.func @copy_1d
// CHECK: scf.for
// CHECK: memref.load
// CHECK: memref.store
func.func @copy_1d(%A: memref<4xi32>, %B: memref<4xi32>) {
  memref.copy %A, %B : memref<4xi32> to memref<4xi32>
  return
}

// CHECK-LABEL: func.func @copy_2d
// CHECK: scf.for
// CHECK: scf.for
// CHECK: memref.load
// CHECK: memref.store
func.func @copy_2d(%A: memref<2x3xi32>, %B: memref<2x3xi32>) {
  memref.copy %A, %B : memref<2x3xi32> to memref<2x3xi32>
  return
}