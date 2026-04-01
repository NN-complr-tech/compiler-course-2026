; RUN: opt -load-pass-plugin %llvmshlibdir/romanov_a_fmuladd_decompose_LLVM_IR%pluginext \
; RUN: -passes=romanov_a_fmuladd_decompose -S %s | FileCheck %s

define float @test_no_fmuladd(float %a, float %b) {
; CHECK-LABEL: @test_no_fmuladd
; CHECK-NEXT: %r = fmul float %a, %b
; CHECK-NEXT: ret float %r
  %r = fmul float %a, %b
  ret float %r
}

define half @test_f16(half %a, half %b, half %c) {
; CHECK-LABEL: @test_f16
; CHECK-NEXT: %decomp.mul = fmul half %a, %b
; CHECK-NEXT: %decomp.add = fadd half %decomp.mul, %c
; CHECK-NEXT: ret half %decomp.add
  %res = call half @llvm.fmuladd.f16(half %a, half %b, half %c)
  ret half %res
}

define float @test_f32(float %a, float %b, float %c) {
; CHECK-LABEL: @test_f32
; CHECK-NEXT: %decomp.mul = fmul float %a, %b
; CHECK-NEXT: %decomp.add = fadd float %decomp.mul, %c
; CHECK-NEXT: ret float %decomp.add
  %res = call float @llvm.fmuladd.f32(float %a, float %b, float %c)
  ret float %res
}

define double @test_f64(double %a, double %b, double %c) {
; CHECK-LABEL: @test_f64
; CHECK-NEXT: %decomp.mul = fmul double %a, %b
; CHECK-NEXT: %decomp.add = fadd double %decomp.mul, %c
; CHECK-NEXT: ret double %decomp.add
  %res = call double @llvm.fmuladd.f64(double %a, double %b, double %c)
  ret double %res
}

define fp128 @test_f128(fp128 %a, fp128 %b, fp128 %c) {
; CHECK-LABEL: @test_f128
; CHECK-NEXT: %decomp.mul = fmul fp128 %a, %b
; CHECK-NEXT: %decomp.add = fadd fp128 %decomp.mul, %c
; CHECK-NEXT: ret fp128 %decomp.add
  %res = call fp128 @llvm.fmuladd.f128(fp128 %a, fp128 %b, fp128 %c)
  ret fp128 %res
}

define float @test_multiple_calls(float %a, float %b, float %c) {
; CHECK-LABEL: @test_multiple_calls
; CHECK-NEXT: %decomp.mul = fmul float %a, %b
; CHECK-NEXT: %decomp.add = fadd float %decomp.mul, %c
; CHECK-NEXT: %decomp.mul1 = fmul float %decomp.add, %a
; CHECK-NEXT: %decomp.add2 = fadd float %decomp.mul1, %b
; CHECK-NEXT: ret float %decomp.add2
  %r1 = call float @llvm.fmuladd.f32(float %a, float %b, float %c)
  %r2 = call float @llvm.fmuladd.f32(float %r1, float %a, float %b)
  ret float %r2
}

declare half  @llvm.fmuladd.f16(half, half, half)
declare float @llvm.fmuladd.f32(float, float, float)
declare double @llvm.fmuladd.f64(double, double, double)
declare fp128 @llvm.fmuladd.f128(fp128, fp128, fp128)