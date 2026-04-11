; RUN: opt -load-pass-plugin %llvmshlibdir/lobanov_d_lab2_LLVM_IR%pluginext -passes=mul-div-to-shift -S %s | FileCheck %s

define i16 @mul_i16_32(i16 %x) {
; CHECK-LABEL: @mul_i16_32
; CHECK-NEXT:    [[RES:%.*]] = shl i16 %x, 5
; CHECK-NEXT:    ret i16 [[RES]]
  %res = mul i16 %x, 32
  ret i16 %res
}

define i64 @udiv_i64_128(i64 %x) {
; CHECK-LABEL: @udiv_i64_128
; CHECK-NEXT:    [[RES:%.*]] = lshr i64 %x, 7
; CHECK-NEXT:    ret i64 [[RES]]
  %res = udiv i64 %x, 128
  ret i64 %res
}

define i32 @mul_by_one(i32 %x) {
; CHECK-LABEL: @mul_by_one
; CHECK-NEXT:    [[RES:%.*]] = shl i32 %x, 0
; CHECK-NEXT:    ret i32 [[RES]]
  %res = mul i32 %x, 1
  ret i32 %res
}

define i32 @udiv_by_one(i32 %x) {
; CHECK-LABEL: @udiv_by_one
; CHECK-NEXT:    [[RES:%.*]] = lshr i32 %x, 0
; CHECK-NEXT:    ret i32 [[RES]]
  %res = udiv i32 %x, 1
  ret i32 %res
}

define i32 @const_left_2(i32 %x) {
; CHECK-LABEL: @const_left_2
; CHECK-NEXT:    [[RES:%.*]] = shl i32 %x, 1
; CHECK-NEXT:    ret i32 [[RES]]
  %res = mul i32 2, %x
  ret i32 %res
}

define i32 @multi_ops(i32 %a, i32 %b, i32 %c) {
; CHECK-LABEL: @multi_ops
; CHECK-DAG:     [[A_SHIFT:%.*]] = shl i32 %a, 3
; CHECK-DAG:     [[B_SHIFT:%.*]] = lshr i32 %b, 4
; CHECK-DAG:     [[C_SHIFT:%.*]] = ashr i32 %c, 2
; CHECK-NEXT:    [[SUM1:%.*]] = add i32 [[A_SHIFT]], [[B_SHIFT]]
; CHECK-NEXT:    [[SUM2:%.*]] = add i32 [[SUM1]], [[C_SHIFT]]
; CHECK-NEXT:    ret i32 [[SUM2]]
  %mul_a = mul i32 %a, 8
  %udiv_b = udiv i32 %b, 16
  %sdiv_c = sdiv i32 %c, 4
  %sum = add i32 %mul_a, %udiv_b
  %total = add i32 %sum, %sdiv_c
  ret i32 %total
}

define i32 @mul_negative_power(i32 %x) {
; CHECK-LABEL: @mul_negative_power
; CHECK-NEXT:    [[RES:%.*]] = mul i32 %x, -16
; CHECK-NEXT:    ret i32 [[RES]]
  %res = mul i32 %x, -16
  ret i32 %res
}

define i32 @mul_not_pow2_6(i32 %x) {
; CHECK-LABEL: @mul_not_pow2_6
; CHECK-NEXT:    [[RES:%.*]] = mul i32 %x, 6
; CHECK-NEXT:    ret i32 [[RES]]
  %res = mul i32 %x, 6
  ret i32 %res
}

define i32 @mixed_replace(i32 %a, i32 %b) {
; CHECK-LABEL: @mixed_replace
; CHECK:         [[MUL1:%.*]] = shl i32 %a, 1
; CHECK-NEXT:    [[MUL2:%.*]] = mul i32 %b, 7
; CHECK-NEXT:    [[DIV:%.*]] = ashr i32 %b, 3
; CHECK-NEXT:    [[ADD1:%.*]] = add i32 [[MUL1]], [[MUL2]]
; CHECK-NEXT:    [[RES:%.*]] = add i32 [[ADD1]], [[DIV]]
; CHECK-NEXT:    ret i32 [[RES]]
  %mul1 = mul i32 %a, 2
  %mul2 = mul i32 %b, 7
  %div = sdiv i32 %b, 8
  %add = add i32 %mul1, %mul2
  %result = add i32 %add, %div
  ret i32 %result
}