// RUN: %build_dir/bin/my_tool %s | FileCheck %s

// CHECK: ========== СТАТИСТИКА ПЕРЕМЕННЫХ ==========
// CHECK: Глобальных переменных: 1
// CHECK: Локальных переменных: 0
// CHECK: Статических локальных: 0
// CHECK: Параметров функций: 2
// CHECK: ===========================================

class MyClass {
public:
    MyClass(int val) : value(val) {}
    int value;
    static int count;
};

int MyClass::count = 0;

struct MyStruct {
    MyStruct(int num = 0) : data(num) {}
    int data;
};
