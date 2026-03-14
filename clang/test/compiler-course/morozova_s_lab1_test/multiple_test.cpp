// RUN: %p/../../../../bin/my_tool %s | FileCheck %s

// CHECK: ========== СТАТИСТИКА ПЕРЕМЕННЫХ ==========
// CHECK: Глобальных переменных: 4
// CHECK: Локальных переменных: 3
// CHECK: Статических локальных: 2
// CHECK: Параметров функций: 2
// CHECK: ===========================================

int a, b;
static int c;
const int d = 5;

void foo(int x, int y) {
    int l1, l2, l3;
    static int s1, s2;
}
