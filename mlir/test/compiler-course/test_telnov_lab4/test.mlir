// RUN: mlir-opt -load-pass-plugin=%mlir_lib_dir/telnov_lab4_MLIR%shlibext
// --pass-pipeline="builtin.module(example_MLIR)" %s | FileCheck %s

// CHECK-LABEL: func.func @basic_constant_loop
func.func @basic_constant_loop() {
  // CHECK: affine.for %{{.*}} = 0 to 12 {
  // CHECK: } {trip_count = 12 : i64}
  affine.for %i = 0 to 12 {
  }
  return
}

// CHECK-LABEL: func.func @loop_with_step
func.func @loop_with_step() {
  // CHECK: affine.for %{{.*}} = 1 to 10 step 3 {
  // CHECK: } {trip_count = 3 : i64}
  affine.for %i = 1 to 10 step 3 {
  }
  return
}

// CHECK-LABEL: func.func @ceil_division_case
func.func @ceil_division_case() {
  // CHECK: affine.for %{{.*}} = 0 to 11 step 4 {
  // CHECK: } {trip_count = 3 : i64}
  affine.for %i = 0 to 11 step 4 {
  }
  return
}

// CHECK-LABEL: func.func @negative_start
func.func @negative_start() {
  // CHECK: affine.for %{{.*}} = -2 to 7 step 3 {
  // CHECK: } {trip_count = 3 : i64}
  affine.for %i = -2 to 7 step 3 {
  }
  return
}

// CHECK-LABEL: func.func @empty_loop
func.func @empty_loop() {
  // CHECK: affine.for %{{.*}} = 5 to 5 {
  // CHECK: } {trip_count = 0 : i64}
  affine.for %i = 5 to 5 {
  }
  return
}

// CHECK-LABEL: func.func @nested_case
func.func @nested_case() {
  // CHECK: affine.for %{{.*}} = 0 to 2 {
  // CHECK: affine.for %{{.*}} = 3 to 12 step 3 {
  // CHECK: } {trip_count = 3 : i64}
  // CHECK: } {trip_count = 2 : i64}
  affine.for %i = 0 to 2 {
    affine.for %j = 3 to 12 step 3 {
    }
  }
  return
}

// CHECK-LABEL: func.func @dynamic_upper_bound
func.func @dynamic_upper_bound(% n : index) {
  // CHECK: affine.for %{{.*}} = 0 to %{{.*}} {
  // CHECK-NOT: trip_count
  // CHECK: return
  affine.for %i = 0 to %n {
  }
  return
}

// CHECK-LABEL: func.func @old_attribute_removed_when_unknown
func.func @old_attribute_removed_when_unknown(% n : index) {
  // CHECK: affine.for %{{.*}} = 0 to %{{.*}} {
  // CHECK-NOT: trip_count
  // CHECK: return
  affine.for %i = 0 to %n {
  } {trip_count = 99 : i64}
  return
}