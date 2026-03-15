// RUN: %clang_cc1 -load %llvmshlibdir/VarStatPlugin%shlibext -plugin var-stat -fsyntax-only %s 2>&1 | FileCheck %s

// CHECK: ========== СТАТИСТИКА ПЕРЕМЕННЫХ ==========
// CHECK: Глобальных переменных: 4
// CHECK: Локальных переменных: 0
// CHECK: Статических локальных: 0
// CHECK: Параметров функций: 0
// CHECK: ===========================================

namespace N {
    int x;
    static int y;
}

namespace {
    int z;
    static int w;
}
