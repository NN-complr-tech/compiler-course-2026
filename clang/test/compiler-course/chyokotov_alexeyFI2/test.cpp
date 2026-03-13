// RUN: split-file %s %t
// RUN: %clang_cc1 -load %llvmshlibdir/chyokotov_alexey_FI2_ClangAST%pluginext -plugin chyokotov_a_analyzer_plugin -fsyntax-only -verify %t/with_warnings.cpp

//--- with_warnings.cpp
extern "C" {
    void* malloc(unsigned long size);
    void* fopen(const char* filename, const char* mode);
    void free(void* ptr);
    int fclose(void* stream);
}

int* global_leak = (int*)malloc(1000); // expected-warning {{memory leak: 'global_leak'}}

void test_var_decl() {
    int* p1 = (int*)malloc(100); // expected-warning {{memory leak: 'p1'}}
    int* p2 = new int(42);        // expected-warning {{memory leak: 'p2'}}
    void* f1 = fopen("test.txt", "r"); // expected-warning {{memory leak: 'f1'}}
}

void test_binary_operator() {
    int* p;
    p = (int*)malloc(100); // expected-warning {{memory leak: 'p'}}
    
    int* q;
    q = new int(42);       // expected-warning {{memory leak: 'q'}}
}

int* test_return_1() {
    int* p = (int*)malloc(100);
    return p; // expected-warning {{memory leak: 'p'}}
}

int* test_return_2() {
    int* p = new int(42);
    return p; // expected-warning {{memory leak: 'p'}}
}

void* test_return_3() {
    void* f = fopen("test.txt", "r");
    return f; // expected-warning {{memory leak: 'f'}}
}

void test_free_ok() {
    int* p = (int*)malloc(100);
    free(p);
}

void test_fclose_ok() {
    void* f = fopen("test.txt", "r");
    fclose(f);
}

void test_delete_ok() {
    int* p = new int(42);
    delete p;
}

void test_delete_array_ok() {
    int* p = new int[10];
    delete[] p;
}

void test_branch_leak(int x) {
    int* p = (int*)malloc(100);
    if (x > 0) {
        return; // expected-warning {{memory leak: 'p'}}
    }
    free(p);
}

void test_nested_scope() {
    {
        int* p = (int*)malloc(100); // expected-warning {{memory leak: 'p'}}
    }
}

void test_multiple_vars() {
    int* p1 = (int*)malloc(100);
    int* p2 = (int*)malloc(200); // expected-warning {{memory leak: 'p2'}}
    int* p3 = new int(42);
    
    free(p1);
    delete p3;
}

void test_static() {
    static int* static_leak = (int*)malloc(500); // expected-warning {{memory leak: 'static_leak'}}
}

void test_new_array() {
    int* arr = new int[50]; // expected-warning {{memory leak: 'arr'}}
}

void free_ptr(int* p) {
    free(p);
}

void test_free_in_other_func() {
    int* p = (int*)malloc(100);
    free_ptr(p);
}