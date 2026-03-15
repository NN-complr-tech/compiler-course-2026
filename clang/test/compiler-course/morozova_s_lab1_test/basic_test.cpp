// RUN: %clang_cc1 -load %llvmshlibdir/VarStatPlugin.so -plugin var-stat -fsyntax-only %s 2>&1 | FileCheck %s

// CHECK: ========== СТАТИСТИКА ПЕРЕМЕННЫХ ==========
// CHECK: Глобальных переменных: 1
// CHECK: Локальных переменных: 1
// CHECK: Статических локальных: 1
// CHECK: Параметров функций: 1
// CHECK: ===========================================

int global = 10;

void func(int param) {
    static int staticLocal = 5;
    int local = 42;
}
