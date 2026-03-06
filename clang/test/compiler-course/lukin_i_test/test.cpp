// RUN: %clang_cc1 -load %llvmshlibdir/VariableStatisticsPlugin_Lukin_Ivan_FIIT3_ClangAST%pluginext -plugin VariableStatisticsPlugin -fsyntax-only %s 2>&1 | FileCheck %s

int global1 = 0;//глобальная

static int static1 = 0;//статическая на уровне файла (все равно статик)

class Example{
    static int static2;//статическая для класса 
};

void foo1()
{
    int local1 = 0;//локальная переменная
    static int static3 = 0;//статик в функции
    double local2 = 0.0;//локальная переменная
}

double global2 = 0.0;

void foo2(int param1, int param2)//два параметра
{
    int local3 = 0;//локальная
}


// CHECK: Statistics
// CHECK-NEXT: Global objects: 2
// CHECK-NEXT: Local variables: 3
// CHECK-NEXT: Static variables: 3
// CHECK-NEXT: Params: 2