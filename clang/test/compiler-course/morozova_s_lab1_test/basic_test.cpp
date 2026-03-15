// RUN: %llvm_tools_dir/my_tool %s | FileCheck %s

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
