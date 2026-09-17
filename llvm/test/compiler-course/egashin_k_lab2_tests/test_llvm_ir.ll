; RUN: opt -load-pass-plugin %llvmshlibdir/egashin_k_lab2_LLVM_IR%pluginext \
; RUN:   -passes=loop-hooks -verify-each -S %s | FileCheck %s
; RUN: opt -load-pass-plugin %llvmshlibdir/egashin_k_lab2_LLVM_IR%pluginext \
; RUN:   -passes=loop-hooks -verify-each %s -o %t.bc
; RUN: lli %t.bc

@events = internal global i64 0

define void @loop_start() {
  %old = load i64, ptr @events
  %shift = mul i64 %old, 10
  %next = add i64 %shift, 1
  store i64 %next, ptr @events
  ret void
}

define void @loop_end() {
  %old = load i64, ptr @events
  %shift = mul i64 %old, 10
  %next = add i64 %shift, 2
  store i64 %next, ptr @events
  ret void
}

; CHECK-LABEL: define i32 @plain(
; CHECK-NEXT: entry:
; CHECK-NEXT: ret i32 %n
define i32 @plain(i32 %n) {
entry:
  ret i32 %n
}

; CHECK-LABEL: define i32 @count(
; CHECK: call void @loop_start()
; CHECK: header:
; CHECK-NOT: call void @loop_start()
; CHECK: call void @loop_end()
; CHECK: ret i32
define i32 @count(i32 %n) memory(none) nounwind nosync nofree willreturn {
entry:
  br label %header
header:
  %i = phi i32 [ 0, %entry ], [ %next, %body ]
  %test = icmp slt i32 %i, %n
  br i1 %test, label %body, label %exit
body:
  %next = add i32 %i, 1
  br label %header
exit:
  ret i32 %i
}

; CHECK-LABEL: define i32 @nested(
; CHECK: call void @loop_start()
; CHECK: outer:
; CHECK: call void @loop_start()
; CHECK: inner:
; CHECK: call void @loop_end()
; CHECK: call void @loop_end()
; CHECK: ret i32
define i32 @nested(i1 %stop) {
entry:
  br label %outer
outer:
  %i = phi i32 [ 0, %entry ], [ %inext, %latch ]
  br label %inner
inner:
  %j = phi i32 [ 0, %outer ], [ %jnext, %inner.latch ]
  br i1 %stop, label %exit, label %inner.latch
inner.latch:
  %jnext = add i32 %j, 1
  %jtest = icmp slt i32 %jnext, 3
  br i1 %jtest, label %inner, label %latch
latch:
  %inext = add i32 %i, 1
  %itest = icmp slt i32 %inext, 2
  br i1 %itest, label %outer, label %exit
exit:
  %result = phi i32 [ %j, %inner ], [ %inext, %latch ]
  ret i32 %result
}

; CHECK-LABEL: define i32 @shared_exit(
; CHECK: call void @loop_start()
; CHECK: loop:
; CHECK: call void @loop_end()
; CHECK: ret i32
define i32 @shared_exit(i1 %skip, i1 %stop) {
entry:
  br i1 %skip, label %exit, label %loop
loop:
  %i = phi i32 [ 0, %entry ], [ %next, %body ]
  br i1 %stop, label %exit, label %body
body:
  %next = add i32 %i, 1
  %test = icmp slt i32 %next, 4
  br i1 %test, label %loop, label %exit
exit:
  %result = phi i32 [ -1, %entry ], [ %i, %loop ], [ %next, %body ]
  ret i32 %result
}

; CHECK-LABEL: define i32 @multiple_entries(
; CHECK: call void @loop_start()
; CHECK: call void @loop_start()
; CHECK: loop:
; CHECK: call void @loop_end()
; CHECK: ret i32
define i32 @multiple_entries(i1 %which) {
entry:
  br i1 %which, label %left, label %right
left:
  br label %loop
right:
  br label %loop
loop:
  %i = phi i32 [ 0, %left ], [ 2, %right ], [ %next, %loop ]
  %next = add i32 %i, 1
  %test = icmp slt i32 %next, 4
  br i1 %test, label %loop, label %exit
exit:
  ret i32 %next
}

; CHECK-LABEL: define i32 @switch_exit(
; CHECK: call void @loop_start()
; CHECK: loop:
; CHECK: switch
; CHECK: call void @loop_end()
; CHECK: ret i32
define i32 @switch_exit() {
entry:
  br label %loop
loop:
  %i = phi i32 [ 0, %entry ], [ %next, %body ]
  switch i32 %i, label %body [ i32 3, label %exit
                              i32 4, label %exit ]
body:
  %next = add i32 %i, 1
  br label %loop
exit:
  %result = phi i32 [ %i, %loop ], [ %i, %loop ]
  ret i32 %result
}

declare void @abort()

define void @check(i32 %actual, i32 %expected, i64 %expected.events) {
  %events = load i64, ptr @events
  store i64 0, ptr @events
  %value.ok = icmp eq i32 %actual, %expected
  %events.ok = icmp eq i64 %events, %expected.events
  %ok = and i1 %value.ok, %events.ok
  br i1 %ok, label %pass, label %fail
pass:
  ret void
fail:
  call void @abort()
  unreachable
}

define i32 @main() {
  %zero = call i32 @count(i32 0)
  call void @check(i32 %zero, i32 0, i64 12)
  %many = call i32 @count(i32 5)
  call void @check(i32 %many, i32 5, i64 12)
  %nested = call i32 @nested(i1 false)
  call void @check(i32 %nested, i32 2, i64 112122)
  %break = call i32 @nested(i1 true)
  call void @check(i32 %break, i32 0, i64 1122)
  %skip = call i32 @shared_exit(i1 true, i1 false)
  call void @check(i32 %skip, i32 -1, i64 0)
  %early = call i32 @shared_exit(i1 false, i1 true)
  call void @check(i32 %early, i32 0, i64 12)
  %normal = call i32 @shared_exit(i1 false, i1 false)
  call void @check(i32 %normal, i32 4, i64 12)
  %left = call i32 @multiple_entries(i1 true)
  call void @check(i32 %left, i32 4, i64 12)
  %right = call i32 @multiple_entries(i1 false)
  call void @check(i32 %right, i32 4, i64 12)
  %switch = call i32 @switch_exit()
  call void @check(i32 %switch, i32 3, i64 12)
  ret i32 0
}
