// RUN: mlir-opt -load-pass-plugin=%mlir_lib_dir/papulina_y_lab4_MLIR%shlibext --pass-pipeline="builtin.module(replace-memref-copy)" %s | FileCheck %s

// CHECK-LABEL: func.func @test_1d
func.func @test_1d(%arg0: memref<10xf32>, %arg1: memref<10xf32>) {
  // CHECK-NOT: memref.copy
  // CHECK: %[[C0:.*]] = arith.constant 0 : index
  // CHECK: %[[C10:.*]] = arith.constant 10 : index
  // CHECK: scf.for %[[I:.*]] = %[[C0]] to %[[C10]]
  // CHECK:   %[[V:.*]] = memref.load %arg0[%[[I]]]
  // CHECK:   memref.store %[[V]], %arg1[%[[I]]]
  memref.copy %arg0, %arg1 : memref<10xf32> to memref<10xf32>
  return
}

// CHECK-LABEL: func.func @test_2d
func.func @test_2d(%arg0: memref<4x8xf32>, %arg1: memref<4x8xf32>) {
  // CHECK: scf.for %[[I:.*]] = {{.*}} to %c4
  // CHECK:   scf.for %[[J:.*]] = {{.*}} to %c8
  // CHECK:     memref.load %arg0[%[[I]], %[[J]]]
  memref.copy %arg0, %arg1 : memref<4x8xf32> to memref<4x8xf32>
  return
}

// CHECK-LABEL: func.func @test_several_copies
func.func @test_several_copies(%arg0: memref<10xf32>, %arg1: memref<10xf32>, %arg2: memref<10xf32>) {
  // CHECK: scf.for %[[I1:.*]] = {{.*}}
  // CHECK:   memref.store {{.*}}, %arg1[%[[I1]]] 
  // CHECK: scf.for %[[I2:.*]] = {{.*}}
  // CHECK:   memref.store {{.*}}, %arg2[%[[I2]]]
  memref.copy %arg0, %arg1 : memref<10xf32> to memref<10xf32>
  memref.copy %arg1, %arg2 : memref<10xf32> to memref<10xf32>
  return
}

// CHECK-LABEL: func.func @test_3d_copy
func.func @test_3d_copy(%arg0: memref<2x3x4xi32>, %arg1: memref<2x3x4xi32>) {
  // CHECK-NOT: memref.copy
  // CHECK: scf.for %[[I:.*]] = {{.*}} to %c2
  // CHECK:   scf.for %[[J:.*]] = {{.*}} to %c3
  // CHECK:     scf.for %[[K:.*]] = {{.*}} to %c4
  // CHECK:       %[[VAL:.*]] = memref.load %arg0[%[[I]], %[[J]], %[[K]]] : memref<2x3x4xi32>
  // CHECK:       memref.store %[[VAL]], %arg1[%[[I]], %[[J]], %[[K]]] : memref<2x3x4xi32>
  memref.copy %arg0, %arg1 : memref<2x3x4xi32> to memref<2x3x4xi32>
  return
}