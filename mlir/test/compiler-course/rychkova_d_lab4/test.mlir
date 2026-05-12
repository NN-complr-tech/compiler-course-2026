// RUN: ... -load-pass-plugin=%mlir_lib_dir//rychkova_d_lab4_MLIR.so \
// RUN: --pass-pipeline="builtin.module(rychkova-copy-to-loop)" %s | FileCheck %s

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