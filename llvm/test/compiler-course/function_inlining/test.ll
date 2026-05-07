; RUN: opt -load-pass-plugin=%llvmshlibdir/function_inlining_LLVM_IR%pluginext -passes=function-inlining -S %s | FileCheck %s

define i32 @add_one(i32 %x) {
entry:
  %sum = add i32 %x, 1
  ret i32 %sum
}

define i32 @inline_simple(i32 %arg) {
; CHECK-LABEL: @inline_simple(
; CHECK-NOT: call i32 @add_one
; CHECK: add i32 %arg, 1
; CHECK: ret i32
entry:
  %result = call i32 @add_one(i32 %arg)
  ret i32 %result
}

define i32 @too_large(i32 %x) {
entry:
  %step1 = add i32 %x, 1
  %step2 = add i32 %step1, 1
  %step3 = add i32 %step2, 1
  %step4 = add i32 %step3, 1
  %step5 = add i32 %step4, 1
  %step6 = add i32 %step5, 1
  %step7 = add i32 %step6, 1
  %step8 = add i32 %step7, 1
  %step9 = add i32 %step8, 1
  %step10 = add i32 %step9, 1
  %step11 = add i32 %step10, 1
  %step12 = add i32 %step11, 1
  %step13 = add i32 %step12, 1
  %step14 = add i32 %step13, 1
  %step15 = add i32 %step14, 1
  %step16 = add i32 %step15, 1
  ret i32 %step16
}

define i32 @do_not_inline_large(i32 %arg) {
; CHECK-LABEL: @do_not_inline_large(
; CHECK: call i32 @too_large
entry:
  %result = call i32 @too_large(i32 %arg)
  ret i32 %result
}

define i32 @recursive_countdown(i32 %n) {
entry:
  %cond = icmp eq i32 %n, 0
  br i1 %cond, label %base, label %step

base:
  ret i32 0

step:
  %next = sub i32 %n, 1
  %value = call i32 @recursive_countdown(i32 %next)
  %result = add i32 %value, 1
  ret i32 %result
}

define i32 @inline_recursive(i32 %n) {
; CHECK-LABEL: @inline_recursive(
; CHECK: sub i32
; CHECK: sub i32
; CHECK: call i32 @recursive_countdown
entry:
  %result = call i32 @recursive_countdown(i32 %n)
  ret i32 %result
}