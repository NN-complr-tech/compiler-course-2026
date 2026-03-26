; RUN: opt -load-pass-plugin %llvmshlibdir/kutergin_valentin_3823b1fi3_LLVM_IR%pluginext -passes=strength-reduction -S %s | FileCheck %s

; CHECK-LABEL: define dso_local noundef i32 @_Z4testi(i32 noundef %x)
define dso_local noundef i32 @_Z4testi(i32 noundef %x) {
entry:
    %x.addr = alloca i32 
    store i32 %x, ptr %x.addr
    %0 = load i32, ptr %x.addr

    ; CHECK: %shl_opt = shl i32 %0, 3
    %mul = mul nsw i32 %0, 8

    ; CHECK: %ashr_opt = ashr i32 %1, 2
    %1 = load i32, ptr %x.addr
    %div = sdiv i32 %1, 4

    ; CHECK: %ashr_opt1 = ashr i32 %2, 4
    %2 = load i32, ptr %x.addr
    %div1 = sdiv i32 %2, 16

    ; CHECK: ret i32
    %3 = load i32, ptr %x.addr
    ret i32 %3
}