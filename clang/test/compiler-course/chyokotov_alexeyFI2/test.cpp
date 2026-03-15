// RUN: %clang_cc1 -load /home/acheck/labworks/compiler-course-2026/build/lib/chyokotov_alexey_FI2_ClangAST.so -add-plugin chyokotov_a_analyzer_plugin -fsyntax-only -verify %s
extern "C" {
  void* malloc(unsigned long size);
  void free(void* ptr);
  void* fopen(const char* filename, const char* mode);
  int fclose(void* stream);
}

void test_malloc_no_return() {
  int* p = (int*)malloc(100); // expected-warning {{memory leak: 'p'}}
}

void test_new_no_return() {
  int* p = new int(42); // expected-warning {{memory leak: 'p'}}
}

void test_fopen_no_return() {
  void* f = fopen("test.txt", "r"); // expected-warning {{memory leak: 'f'}}
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