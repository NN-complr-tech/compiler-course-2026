// RUN: split-file %s %t
// RUN: %clang_cc1 -load %llvmshlibdir/example_ClangAST%pluginext -plugin example_plugin -fsyntax-only -verify %t/leak_cases.cpp
// RUN: %clang_cc1 -load %llvmshlibdir/example_ClangAST%pluginext -plugin example_plugin -fsyntax-only -verify %t/clean_cases.cpp


//============================
// leak_cases.cpp
//============================
//--- leak_cases.cpp

extern "C" {
    void* malloc(unsigned long size);
    void* fopen(const char* name, const char* mode);
}

int* global_data = (int*)malloc(64);

int* create_buffer(int n) {
    int* buf = (int*)malloc(n);
    return buf; // expected-warning {{variable 'buf' holds resource (malloc) that may leak on return}}
}

int* allocate_array(int n) {
    int* arr = new int[n];
    return arr; // expected-warning {{variable 'arr' holds resource (new) that may leak on return}}
}

void* open_file(const char* path) {
    void* file = fopen(path,"r");
    return file; // expected-warning {{variable 'file' holds resource (fopen) that may leak on return}}
}

void early_return_case(int n,int cond) {
    int* data = new int[n];

    if (cond == 0)
        return; // expected-warning {{variable 'data' holds resource (new) that may leak on return}}

    delete[] data;
}

void nested_scope_leak(int n) {
    {
        int* temp = (int*)malloc(n);
    }
    return; // expected-warning {{variable 'temp' holds resource (malloc) that may leak on return}}
}

void malloc_without_free(int n) {
    int* ptr = (int*)malloc(n); // expected-warning {{resource for variable 'ptr' (malloc) not released}}
}

void new_without_delete(int n) {
    int* numbers = new int[n]; // expected-warning {{resource for variable 'numbers' (new) not released}}
}



//============================
// clean_cases.cpp
//============================
//--- clean_cases.cpp
// expected-no-diagnostics

extern "C" {
    void* malloc(unsigned long size);
    void free(void* p);
    void* fopen(const char* name, const char* mode);
    int fclose(void* f);
}

void correct_malloc(int n) {
    int* data = (int*)malloc(n);
    free(data);
}

void correct_new(int n) {
    int* arr = new int[n];
    delete[] arr;
}

void correct_file(const char* name) {
    void* f = fopen(name,"r");
    fclose(f);
}

void nested_clean_case(int n) {
    {
        int* p = (int*)malloc(n);
        free(p);
    }
}

bool branch_clean(int n,int value) {

    int* arr = new int[n];

    if (value > n) {
        delete[] arr;
        return true;
    }

    delete[] arr;
    return false;
}