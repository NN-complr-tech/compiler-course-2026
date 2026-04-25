; RUN: opt -load-pass-plugin %llvmshlibdir/pikhotskiy_r_lab3_LLVM_IR%pluginext \
; RUN:     -passes=pikhotskiy-inline-pass -S %s | FileCheck %s

define i32 @inc(i32 %x) {
; CHECK-LABEL: @inc(
; CHECK-NEXT:    [[ADD:%.*]] = add i32 %x, 1
; CHECK-NEXT:    ret i32 [[ADD]]
  %add = add i32 %x, 1
  ret i32 %add
}

define i32 @caller(i32 %v) {
; CHECK-LABEL: @caller(
; CHECK-NOT:    call i32 @inc(
; CHECK:        [[ADD:%.*]] = add i32 %v, 1
; CHECK-NEXT:   ret i32 [[ADD]]
  %r = call i32 @inc(i32 %v)
  ret i32 %r
}

define i32 @big(i32 %x) {
; CHECK-LABEL: @big(
; CHECK:        ret i32
  %a0 = add i32 %x, 1
  %a1 = add i32 %a0, 1
  %a2 = add i32 %a1, 1
  %a3 = add i32 %a2, 1
  %a4 = add i32 %a3, 1
  %a5 = add i32 %a4, 1
  %a6 = add i32 %a5, 1
  %a7 = add i32 %a6, 1
  %a8 = add i32 %a7, 1
  %a9 = add i32 %a8, 1
  %a10 = add i32 %a9, 1
  %a11 = add i32 %a10, 1
  %a12 = add i32 %a11, 1
  %a13 = add i32 %a12, 1
  %a14 = add i32 %a13, 1
  %a15 = add i32 %a14, 1
  ret i32 %a15
}

define i32 @caller_big(i32 %v) {
; CHECK-LABEL: @caller_big(
; CHECK:        [[R:%.*]] = call i32 @big(i32 %v)
; CHECK-NEXT:   ret i32 [[R]]
  %r = call i32 @big(i32 %v)
  ret i32 %r
}

define i32 @rec(i32 %n) {
; CHECK-LABEL: @rec(
; CHECK-COUNT-1: call i32 @rec(i32
entry:
  %is_zero = icmp eq i32 %n, 0
  br i1 %is_zero, label %base, label %step

base:
  ret i32 0

step:
  %dec = sub i32 %n, 1
  %r = call i32 @rec(i32 %dec)
  %add = add i32 %r, 1
  ret i32 %add
}
