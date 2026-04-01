; RUN: opt -load-pass-plugin %llvmshlibdir/papulina_yuliya_fi3_LLVM_IR%pluginext\
; RUN: -passes=mul-del-optimization -S %s | FileCheck %s

define i32 @_Z4mul1i(i32 noundef %a) {
; CHECK-LABEL: @_Z4mul1i
; CHECK-NEXT:  %Shl = shl i32 %a, 2
; CHECK-NEXT:  ret i32 %Shl
  %mul = mul nsw i32 %a, 4
  ret i32 %mul
}

define i32 @_Z4mul3i(i32 noundef %a) {
; CHECK-LABEL: @_Z4mul3i
; CHECK-NEXT:  %mul = mul nsw i32 %a, 7
; CHECK-NEXT:  ret i32 %mul
  %mul = mul nsw i32 %a, 7
  ret i32 %mul
}

define i32 @_Z4mul4i(i32 noundef %a) {
; CHECK-LABEL: @_Z4mul4i
; CHECK-NEXT:  %mul = mul nsw i32 %a, -8
; CHECK-NEXT:  ret i32 %mul
  %mul = mul nsw i32 %a, -8
  ret i32 %mul
}

define i32 @_Z4div1i(i32 noundef %a) {
; CHECK-LABEL: @_Z4div1i
; CHECK-NEXT:  %1 = icmp slt i32 %a, 0
; CHECK-NEXT:  %2 = select i1 %1, i32 7, i32 0
; CHECK-NEXT:  %3 = add i32 %a, %2
; CHECK-NEXT:  %AShr = ashr i32 %3, 3
; CHECK-NEXT:  ret i32 %AShr
  %div = sdiv i32 %a, 8
  ret i32 %div
}

define i32 @_Z4div2i(i32 noundef %a) {
; CHECK-LABEL: @_Z4div2i
; CHECK-NEXT:  %div = sdiv i32 %a, -8
; CHECK-NEXT:  ret i32 %div
  %div = sdiv i32 %a, -8
  ret i32 %div
}

define i32 @_Z4div3i(i32 noundef %a) {
; CHECK-LABEL: @_Z4div3i
; CHECK-NEXT:  %1 = icmp slt i32 %a, 0
; CHECK-NEXT:  %2 = select i1 %1, i32 3, i32 0
; CHECK-NEXT:  %3 = add i32 %a, %2
; CHECK-NEXT:  %AShr = ashr i32 %3, 2
; CHECK-NEXT:  ret i32 %AShr
  %div = sdiv i32 %a, 4
  ret i32 %div
}

define i32 @_Z4div4i(i32 noundef %a) {
; CHECK-LABEL: @_Z4div4i
; CHECK-NEXT:  %div = sdiv i32 %a, 3
; CHECK-NEXT:  ret i32 %div
  %div = sdiv i32 %a, 3
  ret i32 %div
}

define i32 @_Z4div5j(i32 noundef %a) {
; CHECK-LABEL:  @_Z4div5j
; CHECK-NEXT: %Shr = lshr i32 %a, 3
; CHECK-NEXT: ret i32 %Shr
  %div1 = udiv i32 %a, 8
  ret i32 %div1
}
