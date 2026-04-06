; RUN: opt -load-pass-plugin %llvmshlibdir/dolov_v_lab2_LLVM_IR%pluginext -passes=load-store-elim -S %s | FileCheck %s

define i64 @simple_redundant_load(ptr %p) {
; CHECK-LABEL: @simple_redundant_load(
; CHECK-NEXT: store i64 555, ptr %p
; CHECK-NEXT: ret i64 555
  store i64 555, ptr %p
  %val = load i64, ptr %p
  ret i64 %val
}

define void @simple_dead_store(ptr %p, i64 %v) {
; CHECK-LABEL: @simple_dead_store(
; CHECK-NOT: store i64 10, ptr %p
; CHECK: store i64 %v, ptr %p
  store i64 10, ptr %p
  store i64 %v, ptr %p
  ret void
}

define i64 @volatile_test(ptr %p) {
; CHECK-LABEL: @volatile_test(
; CHECK: store volatile i64 1, ptr %p
; CHECK: %res = load volatile i64, ptr %p
  store volatile i64 1, ptr %p
  %res = load volatile i64, ptr %p
  ret i64 %res
}

define i64 @atomic_test(ptr %p) {
; CHECK-LABEL: @atomic_test(
; CHECK: store atomic i64 9, ptr %p monotonic, align 8
; CHECK: %res = load atomic i64, ptr %p monotonic, align 8
  store atomic i64 9, ptr %p monotonic, align 8
  %res = load atomic i64, ptr %p monotonic, align 8
  ret i64 %res
}

declare void @side_effect_func()
define i64 @clobber_test(ptr %p) {
; CHECK-LABEL: @clobber_test(
; CHECK: store i64 77, ptr %p
; CHECK: call void @side_effect_func()
; CHECK: %v = load i64, ptr %p
  store i64 77, ptr %p
  call void @side_effect_func()
  %v = load i64, ptr %p
  ret i64 %v
}