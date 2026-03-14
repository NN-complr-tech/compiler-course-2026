// RUN: %build_dir/bin/my_tool %s | FileCheck %s

// CHECK: ========== СТАТИСТИКА ПЕРЕМЕННЫХ ==========
// CHECK: Глобальных переменных: 23
// CHECK: Локальных переменных: 6
// CHECK: Статических локальных: 5
// CHECK: Параметров функций: 9
// CHECK: ===========================================

int g1; // +1 глобальная  (1)
int g2 = 5; // +1 глобальная (2)
int g3, g4, g5; // +3 глобальные (5)
static int sg1; // +1 глобальная (6)
extern int eg1;
const int cg1 = 10; // +1 глобальная (7)
constexpr int ceg1 = 20; // +1 глобальная (8)

namespace N {
    int n_g1; // +1 глобальная (9)
    static int n_sg1; // +1 глобальная (10)
}

namespace {
    int anon_g1; // +1 глобальная (11)
    static int anon_sg1; // +1 глобальная (12)
}

extern int duplicate;
int duplicate; // +1 глобальная (13)

void func1(int p1) { // +1 параметр (1)
}

void func2(int p1, int p2) { // +2 параметра (3)
}

void func3(int p1, int p2, int p3, int p4) { //+4 параметра (7)
}

void test_locals() {
    int l1; // +1 локальная (1)
    int l2 = 42; // +1 локальная (2)
    int l3, l4, l5; // +3 локальные (5)

    static int sl1; // +1 статическая локальная (1)
    static int sl2 = 100; // +1 статическая локальная (2)
}

void test_more_static() {
    static int sl3; // +1 статическая локальная (3)
    static int sl4; // +1 статическая локальная (4)
}

template<typename T>
T template_func(T a, T b) {
    static int tls; // +1 статическая локальная (5)
    int tl = 42; // +1 локальная (6)
    return a + b;
}

class MyClass {
public:
    MyClass(int v) : value(v) {} // +1 параметр (8)
    int value;
    static int class_static;
};

int MyClass::class_static = 0; // +1 глобальная (14)

struct Struct {
    Struct(int num = 0) : a(num), k(0.0) {} // +1 параметр (9)
    int a;
    double k;
};

extern int repeat;
int repeat; // +1 глобальная (15)

const int ci = 5; // +1 глобальная (16)
constexpr int cei = 10; // +1 глобальная (17)
volatile int vi = 0; // +1 глобальная (18)

int multi1, multi2, multi3; // +3 глобальные (21)

Struct s1;        // +1 глобальная (22)
Struct s2(42);    // +1 глобальная (23)

void empty() {}
