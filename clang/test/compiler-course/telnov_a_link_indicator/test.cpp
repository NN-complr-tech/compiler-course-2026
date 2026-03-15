// RUN: %clang_cc1 -std=c++17 -load %llvmshlibdir/telnov_a_link_indicator_ClangAST%pluginext -plugin example_plugin -fsyntax-only %s 2>&1 | FileCheck %s

void takes_const_ref(const int&);
void takes_mut_ref(int&);
void takes_const_ptr(const int*);
void takes_mut_ptr(int*);

void ref_read_only(int& value) {
  int x = value;
  (void)x;
}
// CHECK: void ref_read_only(const int& value)

void ref_modified(int& value) {
  value = 10;
}
// CHECK: void ref_modified(int& value)

void ptr_read_only(int* ptr) {
  int x = *ptr;
  (void)x;
}
// CHECK: void ptr_read_only(const int* const ptr)

void ptr_pointee_modified(int* ptr) {
  *ptr = 42;
}
// CHECK: void ptr_pointee_modified(int* ptr)

void ptr_reassigned(int* ptr) {
  int local = 0;
  ptr = &local;
}
// CHECK: void ptr_reassigned(const int* ptr)

void already_const_ref(const int& value) {
  int x = value;
  (void)x;
}
// CHECK: void already_const_ref(const int& value)

void already_const_ptr(const int* const ptr) {
  int x = *ptr;
  (void)x;
}
// CHECK: void already_const_ptr(const int* const ptr)

void local_ref_candidate() {
  int value = 0;
  int& ref = value;
  int x = ref;
  (void)x;
}
// CHECK: const int& ref = value;

void local_ptr_candidate() {
  int value = 0;
  int* ptr = &value;
  int x = *ptr;
  (void)x;
}
// CHECK: const int* const ptr = &value;

void pass_ref_to_const(int& value) {
  takes_const_ref(value);
}
// CHECK: void pass_ref_to_const(const int& value)

void pass_ref_to_mut(int& value) {
  takes_mut_ref(value);
}
// CHECK: void pass_ref_to_mut(int& value)

void pass_ptr_to_const(int* ptr) {
  takes_const_ptr(ptr);
}
// CHECK: void pass_ptr_to_const(const int* const ptr)

void pass_ptr_to_mut(int* ptr) {
  takes_mut_ptr(ptr);
}
// CHECK: void pass_ptr_to_mut(int* ptr)