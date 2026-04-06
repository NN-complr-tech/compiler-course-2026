; RUN: opt -load-pass-plugin %llvmshlibdir/spichek_d_lab_2_LLVM_IR%pluginext\
; RUN: -passes=lse -S %s | FileCheck %s

; Тест 1: Store-to-Load Forwarding
; CHECK-LABEL: @test_store_load
; CHECK: %2 = alloca i32, align 4
; CHECK-NEXT: store i32 %0, ptr %2, align 4
; CHECK-NOT: load
; CHECK-NEXT: ret i32 %0
define dso_local i32 @test_store_load(i32 %0) {
  %2 = alloca i32, align 4
  store i32 %0, ptr %2, align 4
  %3 = load i32, ptr %2, align 4
  ret i32 %3
}

; Тест 2: Load-to-Load Forwarding
; CHECK-LABEL: @test_load_load
; CHECK: %[[VAL:[0-9]+]] = load i32, ptr %0, align 4
; CHECK-NOT: load
; CHECK-NEXT: %[[RES:[0-9]+]] = add i32 %[[VAL]], %[[VAL]]
; CHECK-NEXT: ret i32 %[[RES]]
define dso_local i32 @test_load_load(ptr %0) {
  %2 = load i32, ptr %0, align 4
  %3 = load i32, ptr %0, align 4
  %4 = add i32 %2, %3
  ret i32 %4
}

; Тест 3: Dead Store Elimination
; CHECK-LABEL: @test_dead_store
; CHECK: %3 = alloca i32, align 4
; CHECK-NOT: store i32 %0
; CHECK-NEXT: store i32 %1, ptr %3, align 4
; CHECK-NEXT: ret void
define dso_local void @test_dead_store(i32 %0, i32 %1) {
  %3 = alloca i32, align 4
  store i32 %0, ptr %3, align 4
  store i32 %1, ptr %3, align 4
  ret void
}

declare void @unknown_function()

; Тест 4: Memory Barrier (инвалидация)
; CHECK-LABEL: @test_barrier
; CHECK: %2 = alloca i32, align 4
; CHECK-NEXT: store i32 %0, ptr %2, align 4
; CHECK-NEXT: call void @unknown_function()
; CHECK-NEXT: %3 = load i32, ptr %2, align 4
; CHECK-NEXT: ret i32 %3
define dso_local i32 @test_barrier(i32 %0) {
  %2 = alloca i32, align 4
  store i32 %0, ptr %2, align 4
  call void @unknown_function()
  %3 = load i32, ptr %2, align 4
  ret i32 %3
}