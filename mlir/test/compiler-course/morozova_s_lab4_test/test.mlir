// RUN: mlir-opt -load-pass-plugin=%mlir_lib_dir/morozova_s_lab4_MLIR%shlibext --pass-pipeline="builtin.module(call-counter)" %s | FileCheck %s

// CHECK-LABEL: func.func @func_called_twice
// CHECK-SAME: {call_count = 2 : i64}
func.func @func_called_twice() {
  return
}

// CHECK-LABEL: func.func @func_called_once
// CHECK-SAME: {call_count = 1 : i64}
func.func @func_called_once() {
  return
}

// CHECK-LABEL: func.func @func_not_called
// CHECK-SAME: {call_count = 0 : i64}
func.func @func_not_called() {
  return
}

// CHECK-LABEL: func.func @recursive
// CHECK-SAME: {call_count = 1 : i64}
func.func @recursive(%arg: i32) -> i32 {
  %c0 = arith.constant 0 : i32
  %cmp = arith.cmpi eq, %arg, %c0 : i32
  scf.if %cmp {
    func.return %c0 : i32
  } else {
    %c1 = arith.constant 1 : i32
    %dec = arith.subi %arg, %c1 : i32
    %res = func.call @recursive(%dec) : (i32) -> i32
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

// CHECK-LABEL: func.func @main
// CHECK-SAME: {call_count = 0 : i64}
func.func @main() {
  %c0 = arith.constant 0 : i32
  %c1 = arith.constant 1 : i32
  %c5 = arith.constant 5 : i32
  %c10 = arith.constant 10 : i32

  func.call @func_called_twice() : () -> ()
  func.call @func_called_twice() : () -> ()
  func.call @func_called_once() : () -> ()

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

  %res = func.call @recursive(%c5) : (i32) -> i32

  return
}
