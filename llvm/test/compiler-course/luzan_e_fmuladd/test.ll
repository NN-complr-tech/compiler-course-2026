; RUN: opt -load-pass-plugin %llvmshlibdir/luzan_e_fmuladd_LLVM_IR%pluginext \
; RUN: -passes=fmuladddec -S %s | FileCheck %s

define float @test_basic(float %a, float %b, float %c) {
; CHECK-LABEL: @test_basic
; CHECK-NEXT: %fmul = fmul float %a, %b
; CHECK-NEXT: %fadd = fadd float %fmul, %c
; CHECK-NEXT: ret float %fadd

  %res = call float @llvm.fmuladd.f32(float %a, float %b, float %c)
  ret float %res
}

define float @test_two_intr(float %a, float %b, float %c) {
; CHECK-LABEL: @test_two_intr
; CHECK-NEXT: %fmul = fmul float %a, %b
; CHECK-NEXT: %fadd = fadd float %fmul, %c
; CHECK-NEXT: %fmul1 = fmul float %fadd, %b
; CHECK-NEXT: %fadd2 = fadd float %fmul1, %c

  %x = call float @llvm.fmuladd.f32(float %a, float %b, float %c)
  %y = call float @llvm.fmuladd.f32(float %x, float %b, float %c)
  ret float %y
}

define float @test_multiple_intr(float %a, float %b, float %c) {
; CHECK-LABEL: @test_multiple_intr
; CHECK-NEXT: %fmul = fmul float %a, %b
; CHECK-NEXT: %fadd = fadd float %fmul, %c
; CHECK-NEXT: %fmul1 = fmul float %fadd, %b
; CHECK-NEXT: %fadd2 = fadd float %fmul1, %c
; CHECK-NEXT: %fmul3 = fmul float %fadd, %fadd2
; CHECK-NEXT: %fadd4 = fadd float %fmul3, %c

  %x = call float @llvm.fmuladd.f32(float %a, float %b, float %c)
  %y = call float @llvm.fmuladd.f32(float %x, float %b, float %c)
  %z = call float @llvm.fmuladd.f32(float %x, float %y, float %c)
  ret float %y
}

define float @test_no_change(float %a, float %b) {
; CHECK-LABEL: @test_no_change
; CHECK-NEXT: fmul float %a, %b

  %x = fmul float %a, %b
  ret float %x
}

define double @test_double(double %a, double %b, double %c) {
; CHECK-LABEL: @test_double
; CHECK-NEXT: fmul double %a, %b
; CHECK-NEXT: fadd double

  %res = call double @llvm.fmuladd.f64(double %a, double %b, double %c)
  ret double %res
}

define float @test_fast(float %a, float %b, float %c) {
; CHECK-LABEL: @test_fast
; CHECK-NEXT: fmul fast float %a, %b
; CHECK-NEXT: fadd fast float

  %res = call fast float @llvm.fmuladd.f64(float %a, float %b, float %c)
  ret float %res
}