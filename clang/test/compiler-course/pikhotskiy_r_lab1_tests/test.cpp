// RUN: %clang_cc1 -load %llvmshlibdir/pikhotskiy_r_lab_1_ClangAST%pluginext -plugin var_counter_plugin -fsyntax-only %s 2>&1 | FileCheck %s

// CHECK: Global variables: 8
// CHECK-NEXT: Local variables: 7
// CHECK-NEXT: Static variables: 7
// CHECK-NEXT: Function parameters: 5
// CHECK-NEXT: Total: 27

int global1 = 10;                 // global +1
int global2;                       // global +1
extern int global3;                // global +1

static int static_global1 = 5;     // static +1
static int static_global2;          // static +1

namespace {
    int anon_global1;               // global +1
    static int anon_static1;         // static +1
}

namespace MyNS {
    int ns_global;                   // global +1
    static int ns_static;            // static +1
}

int func1(int a, int b) {           // params +2
    int local1 = 0;                  // local +1
    static int static_local1 = 0;     // static +1
    return local1;
}

double func2(double x) {             // params +1
    int local2 = 42;                  // local +1
    {
        int block_local = 100;         // local +1
        static int block_static = 200;  // static +1
    }
    return x;
}

class Test {
    int field;                        // не считаем
    static int static_field;           // static +1
public:
    Test(int p) {                      // params +1
        int local3 = 1;                  // local +1
    }
    void method(int p1, float p2) {    // params +2
        int local4 = 2;                   // local +1
        static int method_static = 0;      // static +1
    }
};

int Test::static_field = 0;            // static +1

int main(int argc, char* argv[]) {     // params +2
    int local5 = 3;                      // local +1
    static int main_static = 4;           // static +1
    return 0;
}