// RUN: mlir-opt -load-pass-plugin=%mlir_lib_dir/volkov_a_lab_4_MLIR%shlibext --pass-pipeline="builtin.module(volkov-call-counter)" %s | FileCheck %s

module {
  // CHECK: func.func @check_odd(%arg0: i32) -> i1 attributes {call_count = 0 : i32}
  // функция без вызвов
  func.func @check_odd(%arg0: i32) -> i1 {
    %c1 = arith.constant 1 : i32
    %c0 = arith.constant 0 : i32
    %rem = arith.remsi %arg0, %c1 : i32
    %res = arith.cmpi ne, %rem, %c0 : i32
    func.return %res : i1
  }

  // CHECK: func.func @add_two(%arg0: i32) -> i32 attributes {call_count = 2 : i32}
  // утилита, вызывающася 2 раза
  func.func @add_two(%arg0: i32) -> i32 {
    %c2 = arith.constant 2 : i32
    %sum = arith.addi %arg0, %c2 : i32
    func.return %sum : i32
  }

  // CHECK: func.func @entry_point() attributes {call_count = 0 : i32}
  // точка входа
  func.func @entry_point() {
    %c10 = arith.constant 10 : i32
    %0 = func.call @add_two(%c10) : (i32) -> i32
    %c20 = arith.constant 20 : i32
    %1 = func.call @add_two(%c20) : (i32) -> i32
    func.return
  }

  // CHECK: func.func @func_a() attributes {call_count = 3 : i32}
  func.func @func_a() {
    func.return
  }

  // CHECK: func.func @func_b() attributes {call_count = 2 : i32}
  func.func @func_b() {
    func.call @func_a() : () -> ()
    func.call @func_a() : () -> ()
    func.return
  }

  // CHECK: func.func @func_c() attributes {call_count = 1 : i32}
  func.func @func_c() {
    func.call @func_a() : () -> ()
    func.call @func_b() : () -> ()
    func.return
  }

  // CHECK: func.func @func_d() attributes {call_count = 0 : i32}
  func.func @func_d() {
    func.call @func_b() : () -> ()
    func.call @func_c() : () -> ()
    func.return
  }

  // CHECK: func.func @recursive_func() attributes {call_count = 1 : i32}
  func.func @recursive_func() {
    func.call @recursive_func() : () -> ()
    return
  }
}