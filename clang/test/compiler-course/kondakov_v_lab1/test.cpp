// RUN: %clang_cc1 -load %llvmshlibdir/kondakov_v_lab1_ClangAST%pluginext -plugin kondakov_v_lab1_plugin -fsyntax-only %s 2>&1 | FileCheck %s

struct Box {
  int Value = 0;

  void mutate() { ++Value; }
  int read() const { return Value; }
};

void mutate_ref(int &Value) { Value = 10; }
void mutate_ptr(int *Value) { *Value = 10; }
void rebind_ptr(int *&Value) { Value = nullptr; }

// CHECK-LABEL: void local_pointer_readonly() {
// CHECK: const int *const Ptr = &Value;
void local_pointer_readonly() {
  int Value = 1;
  int *Ptr = &Value;
  int Copy = *Ptr;
  (void)Copy;
}

// CHECK-LABEL: void local_pointer_writes_data() {
// CHECK: int *Ptr = &Value;
void local_pointer_writes_data() {
  int Value = 1;
  int *Ptr = &Value;
  *Ptr = 2;
}

// CHECK-LABEL: void local_pointer_reassigned() {
// CHECK: int *Ptr = &Value;
void local_pointer_reassigned() {
  int Value = 1;
  int Other = 2;
  int *Ptr = &Value;
  Ptr = &Other;
  (void)Ptr;
}

// CHECK-LABEL: void local_reference_readonly() {
// CHECK: const int &Ref = Value;
void local_reference_readonly() {
  int Value = 1;
  int &Ref = Value;
  int Copy = Ref;
  (void)Copy;
}

// CHECK-LABEL: void local_reference_mutated() {
// CHECK: int &Ref = Value;
void local_reference_mutated() {
  int Value = 1;
  int &Ref = Value;
  Ref = 2;
}

// CHECK-LABEL: void parameter_readonly(const int *const Ptr, const int &Ref) {
void parameter_readonly(int *Ptr, int &Ref) {
  int Sum = *Ptr + Ref;
  (void)Sum;
}

// CHECK-LABEL: void parameter_mutates_pointee(int *Ptr) {
void parameter_mutates_pointee(int *Ptr) { mutate_ptr(Ptr); }

// CHECK-LABEL: void parameter_rebinds_pointer(int *Ptr) {
void parameter_rebinds_pointer(int *Ptr) { rebind_ptr(Ptr); }

// CHECK-LABEL: void parameter_mutates_reference(int &Ref) {
void parameter_mutates_reference(int &Ref) { mutate_ref(Ref); }

// CHECK-LABEL: void method_calls() {
// CHECK: const Box &ReadOnly = BoxValue;
// CHECK: Box *Mutable = &BoxValue;
void method_calls() {
  Box BoxValue;

  Box &ReadOnly = BoxValue;
  ReadOnly.read();

  Box *Mutable = &BoxValue;
  Mutable->mutate();
}
