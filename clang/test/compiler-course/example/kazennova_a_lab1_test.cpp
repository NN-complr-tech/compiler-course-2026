// RUN: %clang_cc1 -load %llvmshlibdir/kazennova_a_lab1_ClangAST%pluginext -plugin kazennova_a_lab1_plugin -fsyntax-only %s 2>&1 | FileCheck %s

// CHECK-LABEL: test_simple
void test_simple() {
    int x = 10;
    float y = (float)x;
    // CHECK: float y = static_cast<float>(x);
}

// CHECK-LABEL: test_expression
void test_expression() {
    int a = 5, b = 3;
    double d = (double)(a + b);
    // CHECK: double d = static_cast<double>(a + b);
}

// CHECK-LABEL: test_pointer
void test_pointer() {
    int* ptr = 0;
    void* v = (void*)ptr;
    // CHECK: void* v = static_cast<void*>(ptr);
}

// CHECK-LABEL: test_multiple
void test_multiple() {
    int x = 10, y = 20;
    float f = (float)x + (float)y;
    // CHECK: float f = static_cast<float>(x) + static_cast<float>(y);
}

// CHECK-LABEL: test_complex
void test_complex() {
    int x = 1, y = 2;
    float f = (float)(x + y) * (float)(x - y);
    // CHECK: float f = static_cast<float>(x + y) * static_cast<float>(x - y);
}

// CHECK-LABEL: test_no_change
void test_no_change() {
    int x = 10;
    float y = static_cast<float>(x);  // уже C++ cast
    // CHECK: float y = static_cast<float>(x);
    
    double d = 3.14;
    int i = (int)d;  // этот должен измениться
    // CHECK: int i = static_cast<int>(d);
}

// CHECK-LABEL: test_void_cast
void test_void_cast() {
    int x = 10;
    (void)x;  // C-style cast to void
    // CHECK: (void)x;
}

// CHECK-LABEL: test_array_cast
void test_array_cast() {
    int arr[10];
    int* ptr = (int*)arr;  // array to pointer
    // CHECK: int* ptr = static_cast<int*>(arr);
}