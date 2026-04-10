; RUN: split-file %s %t
; RUN: env FMADPLUGIN_PATH=%libdir/fmadplugin.so \
; RUN:   opt -load-pass-plugin=$FMADPLUGIN_PATH -passes=decompose-fmuladd -S %t/input.ll | FileCheck %t/expected.ll
; REQUIRES: plugin

; --- input.ll
define float @test_f32(float %a, float %b, float %c) {
  %res = call float @llvm.fmuladd.f32(float %a, float %b, float %c)
  ret float %res
}

define double @test_f64(double %a, double %b, double %c) {
  %res = call double @llvm.fmuladd.f64(double %a, double %b, double %c)
  ret double %res
}

define half @test_f16(half %a, half %b, half %c) {
  %res = call half @llvm.fmuladd.f16(half %a, half %b, half %c)
  ret half %res
}

define <4 x float> @test_vec4(<4 x float> %a, <4 x float> %b, <4 x float> %c) {
  %res = call <4 x float> @llvm.fmuladd.v4f32(<4 x float> %a, <4 x float> %b, <4 x float> %c)
  ret <4 x float> %res
}

define <2 x double> @test_vec2f64(<2 x double> %a, <2 x double> %b, <2 x double> %c) {
  %res = call <2 x double> @llvm.fmuladd.v2f64(<2 x double> %a, <2 x double> %b, <2 x double> %c)
  ret <2 x double> %res
}

define float @test_fast(float %a, float %b, float %c) {
  %res = call fast float @llvm.fmuladd.f32(float %a, float %b, float %c)
  ret float %res
}

define float @test_nnan_ninf(float %a, float %b, float %c) {
  %res = call nnan ninf float @llvm.fmuladd.f32(float %a, float %b, float %c)
  ret float %res
}

define float @test_contract(float %a, float %b, float %c) {
  %res = call contract float @llvm.fmuladd.f32(float %a, float %b, float %c)
  ret float %res
}

define float @test_multi_use(float %a, float %b, float %c) {
  %fma = call float @llvm.fmuladd.f32(float %a, float %b, float %c)
  %add1 = fadd float %fma, 1.0
  %add2 = fadd float %fma, 2.0
  %r = fadd float %add1, %add2
  ret float %r
}

declare float @llvm.fmuladd.f32(float, float, float)
declare double @llvm.fmuladd.f64(double, double, double)
declare half @llvm.fmuladd.f16(half, half, half)
declare <4 x float> @llvm.fmuladd.v4f32(<4 x float>, <4 x float>, <4 x float>)
declare <2 x double> @llvm.fmuladd.v2f64(<2 x double>, <2 x double>, <2 x double>)

; --- expected.ll
define float @test_f32(float %a, float %b, float %c) {
  %fmul = fmul float %a, %b
  %fadd = fadd float %fmul, %c
  ret float %fadd
}

define double @test_f64(double %a, double %b, double %c) {
  %fmul = fmul double %a, %b
  %fadd = fadd double %fmul, %c
  ret double %fadd
}

define half @test_f16(half %a, half %b, half %c) {
  %fmul = fmul half %a, %b
  %fadd = fadd half %fmul, %c
  ret half %fadd
}

define <4 x float> @test_vec4(<4 x float> %a, <4 x float> %b, <4 x float> %c) {
  %fmul = fmul <4 x float> %a, %b
  %fadd = fadd <4 x float> %fmul, %c
  ret <4 x float> %fadd
}

define <2 x double> @test_vec2f64(<2 x double> %a, <2 x double> %b, <2 x double> %c) {
  %fmul = fmul <2 x double> %a, %b
  %fadd = fadd <2 x double> %fmul, %c
  ret <2 x double> %fadd
}

define float @test_fast(float %a, float %b, float %c) {
  %fmul = fmul fast float %a, %b
  %fadd = fadd fast float %fmul, %c
  ret float %fadd
}

define float @test_nnan_ninf(float %a, float %b, float %c) {
  %fmul = fmul nnan ninf float %a, %b
  %fadd = fadd nnan ninf float %fmul, %c
  ret float %fadd
}

define float @test_contract(float %a, float %b, float %c) {
  %fmul = fmul contract float %a, %b
  %fadd = fadd contract float %fmul, %c
  ret float %fadd
}

define float @test_multi_use(float %a, float %b, float %c) {
  %fmul = fmul float %a, %b
  %fadd = fadd float %fmul, %c
  %add1 = fadd float %fadd, 1.0
  %add2 = fadd float %fadd, 2.0
  %r = fadd float %add1, %add2
  ret float %r
}