// RUN: %mlir-opt -load-pass-plugin=%mlir_lib_dir//rychkova_d_lab4_MLIR.so --pass-pipeline="builtin.module(rychkova-copy-to-loop)" %s | FileCheck %s

func.func @test_1d_static(%A: memref<5xf32>, %B: memref<5xf32>) {
  memref.copy %A, %B : memref<5xf32> to memref<5xf32>
  return
}

// CHECK-LABEL: func.func @test_1d_static
// CHECK: scf.for {{.*}} = %{{.*}} to 5 step 1
// CHECK: memref.load
// CHECK: memref.store

func.func @test_2d_static(%A: memref<3x4xi32>, %B: memref<3x4xi32>) {
  memref.copy %A, %B : memref<3x4xi32> to memref<3x4xi32>
  return
}

// CHECK-LABEL: func.func @test_2d_static
// CHECK: scf.for {{.*}} = %{{.*}} to 3 step 1
// CHECK: scf.for {{.*}} = %{{.*}} to 4 step 1
// CHECK: memref.load
// CHECK: memref.store

func.func @test_3d_static(%A: memref<2x3x2xf64>, %B: memref<2x3x2xf64>) {
  memref.copy %A, %B : memref<2x3x2xf64> to memref<2x3x2xf64>
  return
}

// CHECK-LABEL: func.func @test_3d_static
// CHECK: scf.for {{.*}} = %{{.*}} to 2 step 1
// CHECK: scf.for {{.*}} = %{{.*}} to 3 step 1
// CHECK: scf.for {{.*}} = %{{.*}} to 2 step 1
// CHECK: memref.load
// CHECK: memref.store

func.func @test_1d_dynamic(%A: memref<?xf32>, %B: memref<?xf32>) {
  memref.copy %A, %B : memref<?xf32> to memref<?xf32>
  return
}

// CHECK-LABEL: func.func @test_1d_dynamic
// CHECK: %[[DIM:.*]] = memref.dim %{{.*}}, %{{.*}} : memref<?xf32>
// CHECK: scf.for {{.*}} = %{{.*}} to %[[DIM]] step 1
// CHECK: memref.load
// CHECK: memref.store

func.func @test_2d_dynamic(%A: memref<?x?xi32>, %B: memref<?x?xi32>) {
  memref.copy %A, %B : memref<?x?xi32> to memref<?x?xi32>
  return
}

// CHECK-LABEL: func.func @test_2d_dynamic
// CHECK: %[[DIM0:.*]] = memref.dim %{{.*}}, %{{.*}} : memref<?x?xi32>
// CHECK: %[[DIM1:.*]] = memref.dim %{{.*}}, %{{.*}} : memref<?x?xi32>
// CHECK: scf.for {{.*}} = %{{.*}} to %[[DIM0]] step 1
// CHECK: scf.for {{.*}} = %{{.*}} to %[[DIM1]] step 1
// CHECK: memref.load
// CHECK: memref.store

func.func @test_multiple_copies(%A: memref<4xi32>, %B: memref<4xi32>, %C: memref<4xi32>) {
  memref.copy %A, %B : memref<4xi32> to memref<4xi32>
  memref.copy %B, %C : memref<4xi32> to memref<4xi32>
  return
}

// CHECK-LABEL: func.func @test_multiple_copies
// CHECK: scf.for {{.*}} = %{{.*}} to 4 step 1
// CHECK: memref.load
// CHECK: memref.store
// CHECK: scf.for {{.*}} = %{{.*}} to 4 step 1
// CHECK: memref.load
// CHECK: memref.store

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
