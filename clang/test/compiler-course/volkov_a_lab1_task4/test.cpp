// RUN: %clang_cc1 -load %llvmshlibdir/volkov_a_lab1_task4_ClangAST%pluginext -plugin volkov_a_var_statistic -fsyntax-only %s 2>&1 | FileCheck %s

// CHECK: Total count : 28
// CHECK-NEXT: Global variables : 8
// CHECK-NEXT: Static variables : 7
// CHECK-NEXT: Local variables  : 6
// CHECK-NEXT: Function params  : 7

int alpha = 1;         // global: 1
extern double beta;    // global: 2
static char gamma;     // static: 1

namespace ModuleA {
    extern double beta; // global: 3 (другой scope, считается новой)
    static int delta;   // static: 2
    int epsilon;        // global: 4
}

namespace ModuleA {
    extern int epsilon; // игнорируется (повторное объявление в том же namespace)
    extern double beta; // игнорируется (повторное объявление)
}

namespace {
    int zeta = 0;       // global: 5 (anon namespace без static - расценивается как global)
    static float eta;   // static: 3
}

namespace {
    extern int zeta;    // игнорируется
}

extern double beta;     // игнорируется (повторное объявление глобальной ::beta)
extern int alpha;       // игнорируется (повторное объявление)

int global_x;           // global: 6
int global_y;           // global: 7
int global_z;           // global: 8

struct DataPoint {
    DataPoint(int x, int y) {} // param: 1, param: 2
    int data_x; // FieldDecl (игнорируется)
    int data_y; // FieldDecl (игнорируется)
};

template<typename T>
T algorithm(T input1, T input2) { // param: 3, param: 4
    static T state;          // Static: 4
    T intermediate = input1; // local: 1
    return intermediate;
}

long compute_value(long val) { // param: 5
    int tmp1 = val * 2;        // local: 2
    int tmp2 = tmp1 + 1;       // local: 3
    return tmp2;
}

int main(int argc, char** argv) {     // param: 6, param: 7
    static DataPoint dp_static(0, 0); // static: 5
    DataPoint dp_local(1, 1);         // local: 4
    
    static constexpr int const_var = 100; // static: 6
    constexpr float const_flt = 3.14f;    // local: 5
    
    long res = compute_value(10L);        // local: 6
    
    static int final_stat = 42;           // static: 7
    
    return 0;
}