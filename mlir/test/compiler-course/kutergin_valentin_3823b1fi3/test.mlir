// RUN: mlir-opt %s --load-pass-plugin=%mlir_lib_dir/kutergin_valentin_3823b1fi3_MLIR.so -p "builtin.module(lower-memref-copy)" | FileCheck %s

// CHECK-LABEL: func.func @test_unrolling
func.func @test_unrolling(%arg0: memref<10x20xf32>, %arg1: memref<10x20xf32>) {
    // CHECK: scf.for %[[I:.*]] = %c0 to %c10 step %c1
    // CHECK:   scf.for %[[J:.*]] = %c0 to %c20 step %c1

    // CHECK:     %[[VAL:.*]] = memref.load %arg0[%[[I]], %[[J]]]
    // CHECK:     memref.store %[[VAL]], %arg1[%[[I]], %[[J]]]

    // CHECK-NOT: memref.copy

    memref.copy %arg0, %arg1 : memref<10x20xf32> to memref<10x20xf32>
    return
}