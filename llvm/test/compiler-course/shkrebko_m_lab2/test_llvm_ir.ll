; RUN: opt -load-pass-plugin %llvmshlibdir/shkrebko_m_lab2_LLVM_IR%pluginext\
; RUN: -passes=invert-relational-icmp -S %s | FileCheck %s

; CHECK-LABEL: @slt_i64
; CHECK-NEXT: %cmp = icmp slt i64 %a, %b
; CHECK-NEXT: ret i1 %cmp
define i1 @slt_i64(i64 %a, i64 %b) {
%cmp = icmp slt i64 %a, %b
ret i1 %cmp
}

; CHECK-LABEL: @sgt_i64
; CHECK-NEXT: %cmp.rev = icmp sle i64 %a, %b
; CHECK-NEXT: %cmp.not = xor i1 %cmp.rev, true
; CHECK-NEXT: ret i1 %cmp.not
define i1 @sgt_i64(i64 %a, i64 %b) {
%cmp = icmp sgt i64 %a, %b
ret i1 %cmp
}

; CHECK-LABEL: @ule_i8
; CHECK-NEXT: %cmp = icmp ule i8 %x, %y
; CHECK-NEXT: ret i1 %cmp
define i1 @ule_i8(i8 %x, i8 %y) {
%cmp = icmp ule i8 %x, %y
ret i1 %cmp
}

; CHECK-LABEL: @uge_i8
; CHECK-NEXT: %cmp.rev = icmp ult i8 %x, %y
; CHECK-NEXT: %cmp.not = xor i1 %cmp.rev, true
; CHECK-NEXT: ret i1 %cmp.not
define i1 @uge_i8(i8 %x, i8 %y) {
%cmp = icmp uge i8 %x, %y
ret i1 %cmp
}

; CHECK-LABEL: @eq_i32
; CHECK-NEXT: %cmp = icmp eq i32 %p, %q
; CHECK-NEXT: ret i1 %cmp
define i1 @eq_i32(i32 %p, i32 %q) {
%cmp = icmp eq i32 %p, %q
ret i1 %cmp
}

; CHECK-LABEL: @ne_i32
; CHECK-NEXT: %cmp = icmp ne i32 %p, %q
; CHECK-NEXT: ret i1 %cmp
define i1 @ne_i32(i32 %p, i32 %q) {
%cmp = icmp ne i32 %p, %q
ret i1 %cmp
}