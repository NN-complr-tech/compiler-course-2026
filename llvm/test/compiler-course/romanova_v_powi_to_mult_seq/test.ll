; RUN: opt -load-pass-plugin %llvmshlibdir/romanova_v_powi_to_mult_seq_transform_LLVM_IR%pluginext\
; RUN: -passes=powi-to-mult-seq -S < %s | FileCheck %s

; CHECK-LABEL: define dso_local noundef double @_Z5test0d
; CHECK-NOT: call double @llvm.powi
; CHECK: ret double 1.000000e+00
define dso_local noundef double @_Z5test0d(double noundef %x) #0 {
entry:
  %x.addr = alloca double, align 8
  store double %x, ptr %x.addr, align 8
  %0 = load double, ptr %x.addr, align 8
  %1 = call double @llvm.powi.f64.i32(double %0, i32 0)
  ret double %1
}

declare double @llvm.powi.f64.i32(double, i32) #1

; CHECK-LABEL: define dso_local noundef double @_Z5test1d
; CHECK-NOT: call double @llvm.powi
; CHECK: ret double %0
define dso_local noundef double @_Z5test1d(double noundef %x) #0 {
entry:
  %x.addr = alloca double, align 8
  store double %x, ptr %x.addr, align 8
  %0 = load double, ptr %x.addr, align 8
  %1 = call double @llvm.powi.f64.i32(double %0, i32 1)
  ret double %1
}

; CHECK-LABEL: define dso_local noundef double @_Z5test2d
; CHECK-NOT: call double @llvm.powi
; CHECK: %1 = fmul double %0, %0
; CHECK: ret double %1
define dso_local noundef double @_Z5test2d(double noundef %x) #0 {
entry:
  %x.addr = alloca double, align 8
  store double %x, ptr %x.addr, align 8
  %0 = load double, ptr %x.addr, align 8
  %1 = call double @llvm.powi.f64.i32(double %0, i32 2)
  ret double %1
}

; CHECK-LABEL: define dso_local noundef double @_Z5test3d
; CHECK-NOT: call double @llvm.powi
; CHECK: %1 = fmul double %0, %0
; CHECK: %2 = fmul double %1, %0
; CHECK: ret double %2
define dso_local noundef double @_Z5test3d(double noundef %x) #0 {
entry:
  %x.addr = alloca double, align 8
  store double %x, ptr %x.addr, align 8
  %0 = load double, ptr %x.addr, align 8
  %1 = call double @llvm.powi.f64.i32(double %0, i32 3)
  ret double %1
}

; CHECK-LABEL: define dso_local noundef double @_Z5test4d
; CHECK-NOT: call double @llvm.powi
; CHECK: %1 = fmul double %0, %0
; CHECK: %2 = fmul double %1, %1
; CHECK: ret double %2
define dso_local noundef double @_Z5test4d(double noundef %x) #0 {
entry:
  %x.addr = alloca double, align 8
  store double %x, ptr %x.addr, align 8
  %0 = load double, ptr %x.addr, align 8
  %1 = call double @llvm.powi.f64.i32(double %0, i32 4)
  ret double %1
}

; CHECK-LABEL: define dso_local noundef double @_Z7testextd
; CHECK: %1 = call double @llvm.powi.f64.i32(double %0, i32 5)
; CHECK: ret double %1
define dso_local noundef double @_Z7testextd(double noundef %x) #0 {
entry:
  %x.addr = alloca double, align 8
  store double %x, ptr %x.addr, align 8
  %0 = load double, ptr %x.addr, align 8
  %1 = call double @llvm.powi.f64.i32(double %0, i32 5)
  ret double %1
}

; CHECK-LABEL: define dso_local noundef double @_Z8testdnegd
; CHECK: %1 = call double @llvm.powi.f64.i32(double %0, i32 -1)
; CHECK: ret double %1
define dso_local noundef double @_Z8testdnegd(double noundef %x) #0 {
entry:
  %x.addr = alloca double, align 8
  store double %x, ptr %x.addr, align 8
  %0 = load double, ptr %x.addr, align 8
  %1 = call double @llvm.powi.f64.i32(double %0, i32 -1)
  ret double %1
}
