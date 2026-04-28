// RUN: mlir-opt -load-pass-plugin=%mlir_lib_dir/telnov_lab4_MLIR%shlibext
// --pass-pipeline="builtin.module(example_MLIR)" %s | FileCheck %s

// CHECK-LABEL: func.func @simple_loop
func.func @simple_loop() {
  // CHECK: affine.for %{{.*}} = 0 to 10 {trip_count = 10 : i64}
  affine.for %i = 0 to 10 {
  }
  return
}

// CHECK-LABEL: func.func @loop_with_step
func.func @loop_with_step() {
  // CHECK: affine.for %{{.*}} = 0 to 10 step 2 {trip_count = 5 : i64}
  affine.for %i = 0 to 10 step 2 {
  }
  return
}

// CHECK-LABEL: func.func @loop_with_non_divisible_step
func.func @loop_with_non_divisible_step() {
  // CHECK: affine.for %{{.*}} = 0 to 5 step 2 {trip_count = 3 : i64}
  affine.for %i = 0 to 5 step 2 {
  }
  return
}

// CHECK-LABEL: func.func @loop_with_non_zero_lower_bound
func.func @loop_with_non_zero_lower_bound() {
  // CHECK: affine.for %{{.*}} = 2 to 11 step 3 {trip_count = 3 : i64}
  affine.for %i = 2 to 11 step 3 {
  }
  return
}

// CHECK-LABEL: func.func @nested_loops
func.func @nested_loops() {
  // CHECK: affine.for %{{.*}} = 0 to 3 {trip_count = 3 : i64}
  affine.for %i = 0 to 3 {
    // CHECK: affine.for %{{.*}} = 1 to 6 step 2 {trip_count = 3 : i64}
    affine.for %j = 1 to 6 step 2 {
    }
  }
  return
}

// CHECK-LABEL: func.func @unknown_upper_bound
func.func @unknown_upper_bound(% n : index) {
  // CHECK: affine.for %{{.*}} = 0 to %{{.*}} {
  // CHECK-NOT: trip_count
  affine.for %i = 0 to %n {
  }
  return
}

// CHECK-LABEL: func.func @unknown_lower_bound
func.func @unknown_lower_bound(% n : index) {
  // CHECK: affine.for %{{.*}} = %{{.*}} to 10 {
  // CHECK-NOT: trip_count
  affine.for %i = %n to 10 {
  }
  return
}

// CHECK-LABEL: func.func @unknown_symbolic_bound
func.func @unknown_symbolic_bound(% n : index) {
  // CHECK: affine.for %{{.*}} = 0 to affine_map
  // CHECK-NOT: trip_count
  affine.for %i = 0 to affine_map<(d0) -> (d0 + 5)>(%n) {
  }
  return
}