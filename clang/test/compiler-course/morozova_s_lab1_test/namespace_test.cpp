// RUN: %llvm_tools_dir/my_tool %s | FileCheck %s

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
