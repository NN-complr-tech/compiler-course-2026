define dso_local noundef double @_Z12test_pow_negd(double noundef %a) #0 {
entry:
  %a.addr = alloca double, align 8
  store double %a, ptr %a.addr, align 8
  %0 = load double, ptr %a.addr, align 8
  %1 = call double @llvm.powi.f64.i32(double %0, i32 -1)
  ret double %1
}

define dso_local noundef double @_Z9test_pow0d(double noundef %a) #0 {
entry:
  %a.addr = alloca double, align 8
  store double %a, ptr %a.addr, align 8
  %0 = load double, ptr %a.addr, align 8
  %1 = call double @llvm.powi.f64.i32(double %0, i32 0)
  ret double %1
}

define dso_local noundef double @_Z9test_pow1d(double noundef %a) #0 {
entry:
  %a.addr = alloca double, align 8
  store double %a, ptr %a.addr, align 8
  %0 = load double, ptr %a.addr, align 8
  %1 = call double @llvm.powi.f64.i32(double %0, i32 1)
  ret double %1
}

define dso_local noundef double @_Z9test_pow2d(double noundef %a) #0 {
entry:
  %a.addr = alloca double, align 8
  store double %a, ptr %a.addr, align 8
  %0 = load double, ptr %a.addr, align 8
  %1 = call double @llvm.powi.f64.i32(double %0, i32 2)
  ret double %1
}

define dso_local noundef double @_Z9test_pow3d(double noundef %a) #0 {
entry:
  %a.addr = alloca double, align 8
  store double %a, ptr %a.addr, align 8
  %0 = load double, ptr %a.addr, align 8
  %1 = call double @llvm.powi.f64.i32(double %0, i32 3)
  ret double %1
}

define dso_local noundef double @_Z9test_pow4d(double noundef %a) #0 {
entry:
  %a.addr = alloca double, align 8
  store double %a, ptr %a.addr, align 8
  %0 = load double, ptr %a.addr, align 8
  %1 = call double @llvm.powi.f64.i32(double %0, i32 4)
  ret double %1
}

define dso_local noundef double @_Z9test_pow5d(double noundef %a) #0 {
entry:
  %a.addr = alloca double, align 8
  store double %a, ptr %a.addr, align 8
  %0 = load double, ptr %a.addr, align 8
  %1 = call double @llvm.powi.f64.i32(double %0, i32 5)
  ret double %1
}

define dso_local noundef double @_Z17test_pow_multipowd(double noundef %a) #0 {
entry:
  %a.addr = alloca double, align 8
  %tmp = alloca double, align 8
  store double %a, ptr %a.addr, align 8
  %0 = load double, ptr %a.addr, align 8
  %1 = call double @llvm.powi.f64.i32(double %0, i32 2)
  store double %1, ptr %tmp, align 8
  %2 = load double, ptr %tmp, align 8
  %3 = call double @llvm.powi.f64.i32(double %2, i32 3)
  ret double %3
}

define dso_local noundef double @_Z19test_pow_ifmultipowd(double noundef %a) #0 {
entry:
  %a.addr = alloca double, align 8
  %tmp = alloca double, align 8
  store double %a, ptr %a.addr, align 8
  %0 = load double, ptr %a.addr, align 8
  %1 = call double @llvm.powi.f64.i32(double %0, i32 2)
  store double %1, ptr %tmp, align 8
  %2 = load double, ptr %tmp, align 8
  %3 = call double @llvm.powi.f64.i32(double %2, i32 2)
  store double %3, ptr %tmp, align 8
  %4 = load double, ptr %tmp, align 8
  %5 = call double @llvm.powi.f64.i32(double %4, i32 2)
  ret double %5
}

define dso_local noundef double @_Z21test_pow_loopmultipowd(double noundef %a) #0 {
entry:
  %a.addr = alloca double, align 8
  %tmp = alloca double, align 8
  store double %a, ptr %a.addr, align 8
  %0 = load double, ptr %a.addr, align 8
  %1 = call double @llvm.powi.f64.i32(double %0, i32 2)
  store double %1, ptr %tmp, align 8
  br label %while.body

while.body:                                       
  %2 = load double, ptr %tmp, align 8
  %3 = call double @llvm.powi.f64.i32(double %2, i32 2)
  store double %3, ptr %tmp, align 8
  br label %while.end

while.end:                                        
  %4 = load double, ptr %tmp, align 8
  %5 = call double @llvm.powi.f64.i32(double %4, i32 2)
  ret double %5
}

define dso_local noundef double @_Z17test_pow_variabledi(double noundef %a, i32 noundef %n) #0 {
entry:
  %a.addr = alloca double, align 8
  %n.addr = alloca i32, align 4
  store double %a, ptr %a.addr, align 8
  store i32 %n, ptr %n.addr, align 4
  %0 = load double, ptr %a.addr, align 8
  %1 = load i32, ptr %n.addr, align 4
  %2 = call double @llvm.powi.f64.i32(double %0, i32 %1)
  ret double %2
}
