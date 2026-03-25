; RUN: opt -load-pass-plugin=%llvmshlibdir/ovsyannikov_n_lab2_LLVM_IR%pluginext -passes=ovsyannikov-lse -S %s | FileCheck %s

define i32 @test_load_after_store(ptr %p) {
; CHECK-LABEL: @test_load_after_store(
; CHECK-NEXT: store i32 42, ptr %p
; CHECK-NEXT: ret i32 42
  store i32 42, ptr %p
  %val = load i32, ptr %p
  ret i32 %val
}

define i32 @test_load_after_load(ptr %p) {
; CHECK-LABEL: @test_load_after_load(
; CHECK-NEXT: %v1 = load i32, ptr %p
; CHECK-NEXT: %res = add i32 %v1, %v1
  %v1 = load i32, ptr %p
  %v2 = load i32, ptr %p
  %res = add i32 %v1, %v2
  ret i32 %res
}

define void @test_dead_store(ptr %p, i32 %v1, i32 %v2) {
; CHECK-LABEL: @test_dead_store(
; CHECK-NOT: store i32 %v1, ptr %p
; CHECK: store i32 %v2, ptr %p
  store i32 %v1, ptr %p
  store i32 %v2, ptr %p
  ret void
}

declare void @unknown_call()
define i32 @test_clobber(ptr %p) {
; CHECK-LABEL: @test_clobber(
; CHECK: store i32 10, ptr %p
; CHECK: call void @unknown_call()
; CHECK: %v = load i32, ptr %p
  store i32 10, ptr %p
  call void @unknown_call()
  %v = load i32, ptr %p
  ret i32 %v
}
