// RUN: mlir-opt -load-pass-plugin=%mlir_lib_dir/morozova_s_lab4_MLIR%shlibext --pass-pipeline="builtin.module(call-counter)" %s | FileCheck %s

// CHECK-LABEL: func.func @simple_function
// CHECK-SAME: {call_count = 3 : i64}
func.func @simple_function() {
  return
}

// CHECK-LABEL: func.func @function_called_once
// CHECK-SAME: {call_count = 1 : i64}
func.func @function_called_once() {
  return
}

// CHECK-LABEL: func.func @function_never_called
// CHECK-SAME: {call_count = 0 : i64}
func.func @function_never_called() {
  return
}

// CHECK-LABEL: func.func @recursive_function
// CHECK-SAME: {call_count = 1 : i64}
func.func @recursive_function(%arg: i32) -> i32 {
  %c0 = arith.constant 0 : i32
  %c1 = arith.constant 1 : i32
  %cmp = arith.cmpi eq, %arg, %c0 : i32
  scf.if %cmp {
    func.return %c0 : i32
  } else {
    %dec = arith.subi %arg, %c1 : i32
    %res = func.call @recursive_function(%dec) : (i32) -> i32
    func.return %res : i32
  }
  func.return %c0 : i32
}

// CHECK-LABEL: func.func @called_in_loop
// CHECK-SAME: {call_count = 10 : i64}
func.func @called_in_loop() {
  return
}

// CHECK-LABEL: func.func @called_in_conditional
// CHECK-SAME: {call_count = 5 : i64}
func.func @called_in_conditional() {
  return
}

// CHECK-LABEL: func.func @called_from_nested_scopes
// CHECK-SAME: {call_count = 3 : i64}
func.func @called_from_nested_scopes() {
  return
}

// CHECK-LABEL: module @nested_module {
// CHECK-LABEL: func.func @inner_function
// CHECK-SAME: {call_count = 2 : i64}
// CHECK-LABEL: func.func @outer_function
// CHECK-SAME: {call_count = 0 : i64}
module @nested_module {
  func.func @inner_function() {
    return
  }
  
  func.func @outer_function() {
    func.call @inner_function() : () -> ()
    func.call @inner_function() : () -> ()
    return
  }
}

// CHECK-LABEL: func.func @with_arguments
// CHECK-SAME: {call_count = 2 : i64}
func.func @with_arguments(%a: i32, %b: f32) -> i32 {
  %c0 = arith.constant 0 : i32
  return %c0 : i32
}

// CHECK-LABEL: func.func @main
// CHECK-SAME: {call_count = 0 : i64}
func.func @main() {
  %c0 = arith.constant 0 : i32
  %c1 = arith.constant 1 : i32
  %c5 = arith.constant 5 : i32
  %c10 = arith.constant 10 : i32
  %cf1 = arith.constant 1.0 : f32
  
  func.call @simple_function() : () -> ()
  func.call @simple_function() : () -> ()
  func.call @simple_function() : () -> ()
  
  func.call @function_called_once() : () -> ()
  
  scf.for %i = %c0 to %c10 step %c1 {
    func.call @called_in_loop() : () -> ()
  }
  
  scf.for %i = %c0 to %c5 step %c1 {
    %cond = arith.cmpi eq, %i, %c1 : i32
    scf.if %cond {
      func.call @called_in_conditional() : () -> ()
    } else {
      func.call @called_in_conditional() : () -> ()
    }
  }
  
  scf.for %i = %c0 to %c5 step %c1 {
    scf.for %j = %c0 to %c2 step %c1 {
      func.call @called_from_nested_scopes() : () -> ()
    }
    func.call @called_from_nested_scopes() : () -> ()
  }
  
  %res = func.call @recursive_function(%c5) : (i32) -> i32
  
  func.call @with_arguments(%c1, %cf1) : (i32, f32) -> i32
  func.call @with_arguments(%c5, %cf1) : (i32, f32) -> i32
  
  return
}
