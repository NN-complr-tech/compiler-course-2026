; RUN: opt -load-pass-plugin %llvmshlibdir/gusev_d_lab2_LLVM_IR%pluginext \
; RUN:   -passes=gusev-d-lab2 -S %s | FileCheck %s

; CHECK-NOT: srem
; CHECK-NOT: urem
; CHECK-NOT: frem

; Integer signed remainder.
define i32 @test_srem(i32 %a, i32 %b) {
; CHECK-LABEL: @test_srem(
; CHECK: entry:
; CHECK-NEXT: [[DIV:%.*]] = sdiv i32 %a, %b
; CHECK-NEXT: [[MUL:%.*]] = mul i32 [[DIV]], %b
; CHECK-NEXT: [[SUB:%.*]] = sub i32 %a, [[MUL]]
; CHECK-NEXT: ret i32 [[SUB]]
entry:
  %r = srem i32 %a, %b
  ret i32 %r
}

; Integer unsigned remainder.
define i32 @test_urem(i32 %a, i32 %b) {
; CHECK-LABEL: @test_urem(
; CHECK: entry:
; CHECK-NEXT: [[DIV:%.*]] = udiv i32 %a, %b
; CHECK-NEXT: [[MUL:%.*]] = mul i32 [[DIV]], %b
; CHECK-NEXT: [[SUB:%.*]] = sub i32 %a, [[MUL]]
; CHECK-NEXT: ret i32 [[SUB]]
entry:
  %r = urem i32 %a, %b
  ret i32 %r
}