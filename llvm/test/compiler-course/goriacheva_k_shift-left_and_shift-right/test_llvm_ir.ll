; RUN: opt -load-pass-plugin %llvmshlibdir/goriacheva_k_shift-left_and_shift-right_LLVM_IR%pluginext \
; RUN: -passes=shift -S %s | FileCheck %s

; =========================================
; MUL -> SHL
; =========================================
define i32 @mul_pow2(i32 %x) {
; CHECK-LABEL: @mul_pow2
; CHECK: shl i32 %x, 3
  %1 = mul i32 %x, 8
  ret i32 %1
}

; =========================================
; UDIV -> LSHR
; =========================================
define i32 @udiv_pow2(i32 %x) {
; CHECK-LABEL: @udiv_pow2
; CHECK: lshr i32 %x, 2
  %1 = udiv i32 %x, 4
  ret i32 %1
}

; =========================================
; SDIV -> ASHR
; =========================================
define i32 @sdiv_pow2(i32 %x) {
; CHECK-LABEL: @sdiv_pow2
; CHECK: ashr i32 %x, 1
  %1 = sdiv i32 %x, 2
  ret i32 %1
}

; =========================================
; НЕ степень двойки (не должно меняться)
; =========================================
define i32 @mul_not_pow2(i32 %x) {
; CHECK-LABEL: @mul_not_pow2
; CHECK: mul i32 %x, 3
  %1 = mul i32 %x, 3
  ret i32 %1
}

; =========================================
; Константа слева (4 * x)
; =========================================
define i32 @mul_const_left(i32 %x) {
; CHECK-LABEL: @mul_const_left
; CHECK: shl i32 %x, 2
  %1 = mul i32 4, %x
  ret i32 %1
}

; =========================================
; Комбинированный тест (udiv + sdiv)
; =========================================
define i32 @both_div(i32 %x) {
; CHECK-LABEL: @both_div
; CHECK: lshr i32 %x, 1
; CHECK: ashr i32 %x, 1
  %1 = udiv i32 %x, 2
  %2 = sdiv i32 %x, 2
  %3 = add i32 %1, %2
  ret i32 %3
}


