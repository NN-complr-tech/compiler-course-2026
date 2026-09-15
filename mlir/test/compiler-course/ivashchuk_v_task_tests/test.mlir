// RUN: mlir-opt -load-pass-plugin=%mlir_lib_dir/ivashchuk_v_task_MLIR%shlibext \
// RUN:   --pass-pipeline="builtin.module(ceil_div)" %s | FileCheck %s


// 1. ceildivui меняем на (a + b - 1) / b

// CHECK-LABEL: func.func @ceildivui_simple
func.func @ceildivui_simple(%a: i32, %b: i32) -> i32 {
  // CHECK-NEXT: %[[ONE:.*]] = arith.constant 1 : i32
  // CHECK-NEXT: %[[B1:.*]] = arith.subi %arg1, %[[ONE]] : i32
  // CHECK-NEXT: %[[SUM:.*]] = arith.addi %arg0, %[[B1]] : i32
  // CHECK-NEXT: %[[RES:.*]] = arith.divui %[[SUM]], %arg1 : i32
  // CHECK-NEXT: return %[[RES]] : i32
  %r = arith.ceildivui %a, %b : i32
  return %r : i32
}

// 2. ceildivsi замена на (a + b - 1) / b

// CHECK-LABEL: func.func @ceildivsi_simple
func.func @ceildivsi_simple(%a: i32, %b: i32) -> i32 {
  // CHECK-NEXT: %[[ONE:.*]] = arith.constant 1 : i32
  // CHECK-NEXT: %[[B1:.*]] = arith.subi %arg1, %[[ONE]] : i32
  // CHECK-NEXT: %[[SUM:.*]] = arith.addi %arg0, %[[B1]] : i32
  // CHECK-NEXT: %[[RES:.*]] = arith.divsi %[[SUM]], %arg1 : i32
  // CHECK-NEXT: return %[[RES]] : i32
  %r = arith.ceildivsi %a, %b : i32
  return %r : i32
}

// 3. Оба оператора сразу в одной ф-ии

// CHECK-LABEL: func.func @both_ops
func.func @both_ops(%a: i32, %b: i32) -> i32 {
  // CHECK: arith.divui
  // CHECK: arith.divsi
  %u = arith.ceildivui %a, %b : i32
  %s = arith.ceildivsi %u, %b : i32
  return %s : i32
}

// 4. тип данных i64

// CHECK-LABEL: func.func @ceildivui_i64
func.func @ceildivui_i64(%a: i64, %b: i64) -> i64 {
  // CHECK-NEXT: %[[ONE:.*]] = arith.constant 1 : i64
  // CHECK-NEXT: %[[B1:.*]] = arith.subi %arg1, %[[ONE]] : i64
  // CHECK-NEXT: %[[SUM:.*]] = arith.addi %arg0, %[[B1]] : i64
  // CHECK-NEXT: %[[RES:.*]] = arith.divui %[[SUM]], %arg1 : i64
  // CHECK-NEXT: return %[[RES]] : i64
  %r = arith.ceildivui %a, %b : i64
  return %r : i64
}

// 5. Нет ceildiv ничего не должно меняться

// CHECK-LABEL: func.func @no_ceildiv
func.func @no_ceildiv(%a: i32, %b: i32) -> i32 {
  // CHECK-NEXT: %[[RES:.*]] = arith.divsi %arg0, %arg1 : i32
  // CHECK-NEXT: return %[[RES]] : i32
  %r = arith.divsi %a, %b : i32
  return %r : i32
}