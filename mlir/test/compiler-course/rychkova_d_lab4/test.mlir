// RUN: mlir-opt -load-pass-plugin=%mlir_lib_dir//rychkova_d_lab4_MLIR%shlibext --pass-pipeline="builtin.module(rychkova-copy-to-loop)" %s | FileCheck %s

func.func @test_1d_static(%A: memref<5xf32>, %B: memref<5xf32>) {
  memref.copy %A, %B : memref<5xf32> to memref<5xf32>
  return
}

// CHECK-LABEL: func.func @test_1d_static
// CHECK: %c0 = arith.constant 0 : index
// CHECK: %c1 = arith.constant 1 : index
// CHECK: %c5 = arith.constant 5 : index
// CHECK: scf.for {{.*}} = %c0 to %c5 step %c1
// CHECK: memref.load
// CHECK: memref.store
// CHECK-NOT: memref.copy

func.func @test_2d_static(%A: memref<3x4xi32>, %B: memref<3x4xi32>) {
  memref.copy %A, %B : memref<3x4xi32> to memref<3x4xi32>
  return
}

// CHECK-LABEL: func.func @test_2d_static
// CHECK: %c0 = arith.constant 0 : index
// CHECK: %c1 = arith.constant 1 : index
// CHECK: %c3 = arith.constant 3 : index
// CHECK: %c4 = arith.constant 4 : index
// CHECK: scf.for {{.*}} = %c0 to %c3 step %c1
// CHECK: scf.for {{.*}} = %c0 to %c4 step %c1
// CHECK: memref.load
// CHECK: memref.store
// CHECK-NOT: memref.copy

func.func @test_3d_static(%A: memref<2x3x2xf64>, %B: memref<2x3x2xf64>) {
  memref.copy %A, %B : memref<2x3x2xf64> to memref<2x3x2xf64>
  return
}

// CHECK-LABEL: func.func @test_3d_static
// CHECK: %c0 = arith.constant 0 : index
// CHECK: %c1 = arith.constant 1 : index
// CHECK: %c2 = arith.constant 2 : index
// CHECK: %c3 = arith.constant 3 : index
// CHECK: scf.for {{.*}} = %c0 to %c2 step %c1
// CHECK: scf.for {{.*}} = %c0 to %c3 step %c1
// CHECK: scf.for {{.*}} = %c0 to %c2 step %c1
// CHECK: memref.load
// CHECK: memref.store
// CHECK-NOT: memref.copy

func.func @test_1d_dynamic(%A: memref<?xf32>, %B: memref<?xf32>) {
  memref.copy %A, %B : memref<?xf32> to memref<?xf32>
  return
}

// CHECK-LABEL: func.func @test_1d_dynamic
// CHECK: %c0 = arith.constant 0 : index
// CHECK: %c1 = arith.constant 1 : index
// CHECK: %dim = memref.dim %{{.*}}, %{{.*}} : memref<?xf32>
// CHECK: scf.for {{.*}} = %c0 to %dim step %c1
// CHECK: memref.load
// CHECK: memref.store
// CHECK-NOT: memref.copy

func.func @test_2d_dynamic(%A: memref<?x?xi32>, %B: memref<?x?xi32>) {
  memref.copy %A, %B : memref<?x?xi32> to memref<?x?xi32>
  return
}

// CHECK-LABEL: func.func @test_2d_dynamic
// CHECK: %c0 = arith.constant 0 : index
// CHECK: %c1 = arith.constant 1 : index
// CHECK: %dim = memref.dim %{{.*}}, %{{.*}} : memref<?x?xi32>
// CHECK: %dim_2 = memref.dim %{{.*}}, %{{.*}} : memref<?x?xi32>
// CHECK: scf.for {{.*}} = %c0 to %dim step %c1
// CHECK: scf.for {{.*}} = %c0 to %dim_2 step %c1
// CHECK: memref.load
// CHECK: memref.store
// CHECK-NOT: memref.copy

func.func @test_multiple_copies(%A: memref<4xi32>, %B: memref<4xi32>, %C: memref<4xi32>) {
  memref.copy %A, %B : memref<4xi32> to memref<4xi32>
  memref.copy %B, %C : memref<4xi32> to memref<4xi32>
  return
}

// CHECK-LABEL: func.func @test_multiple_copies
// CHECK: %c0 = arith.constant 0 : index
// CHECK: %c1 = arith.constant 1 : index
// CHECK: %c4 = arith.constant 4 : index
// CHECK: scf.for {{.*}} = %c0 to %c4 step %c1
// CHECK: memref.load
// CHECK: memref.store
// CHECK: scf.for {{.*}} = %c0 to %c4 step %c1
// CHECK: memref.load
// CHECK: memref.store
// CHECK-NOT: memref.copy

func.func @test_without_copy(%A: memref<4xi32>) {
  %c0 = arith.constant 0 : index
  %val = memref.load %A[%c0] : memref<4xi32>
  memref.store %val, %A[%c0] : memref<4xi32>
  return
}

// CHECK-LABEL: func.func @test_without_copy
// CHECK-NOT: scf.for
// CHECK: memref.load
// CHECK: memref.store
