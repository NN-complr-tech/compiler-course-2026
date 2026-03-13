// RUN: %clang_cc1 -load %llvmshlibdir/sannikov_i_c_style_cast_ClangAST%pluginext -plugin cast_rewriter -fsyntax-only %s 2>&1 | FileCheck %s

enum Color {
  Red = 1,
  Green = 2
};

// Тест 1: Приведение double - int должно замениться на static_cast
// CHECK: int test_1(double x) {
// CHECK-NEXT:   return static_cast<int>(x);
// CHECK-NEXT: }
int test_1(double x) {
  return (int)x;
}

// Тест 2: Приведение int - double должно замениться на static_cast
// CHECK: double test_2(int x) {
// CHECK-NEXT:   return static_cast<double>(x);
// CHECK-NEXT: }
double test_2(int x) {
  return (double)x;
}

// Тест 3: Приведение long - unsigned должно замениться на static_cast
// CHECK: unsigned test_3(long x) {
// CHECK-NEXT:   return static_cast<unsigned>(x);
// CHECK-NEXT: }
unsigned test_3(long x) {
  return (unsigned)x;
}

// Тест 4: Приведение enum - int должно замениться на static_cast
// CHECK: int test_enum_to_int(Color c) {
// CHECK-NEXT:   return static_cast<int>(c);
// CHECK-NEXT: }
int test_enum_to_int(Color c) {
  return (int)c;
}

// Тест 5: Приведение int - enum должно замениться на static_cast
// CHECK: Color test_int_to_enum(int x) {
// CHECK-NEXT:   return static_cast<Color>(x);
// CHECK-NEXT: }
Color test_int_to_enum(int x) {
  return (Color)x;
}

// Тест 6: Снятие const у указателя должно замениться на const_cast
// CHECK: int test_const_ptr(const int *ptr) {
// CHECK-NEXT:   int *p = const_cast<int*>(ptr);
// CHECK-NEXT:   return *p;
// CHECK-NEXT: }
int test_const_ptr(const int *ptr) {
  int *p = (int*)ptr;
  return *p;
}

// Тест 7: Снятие volatile у указателя должно замениться на const_cast
// CHECK: int test_volatile_ptr(volatile int *ptr) {
// CHECK-NEXT:   int *p = const_cast<int*>(ptr);
// CHECK-NEXT:   return *p;
// CHECK-NEXT: }
int test_volatile_ptr(volatile int *ptr) {
  int *p = (int*)ptr;
  return *p;
}

// Тест 8: Снятие const у ссылки должно замениться на const_cast
// CHECK: void test_const_ref(const int &src) {
// CHECK-NEXT:   int &ref = const_cast<int&>(src);
// CHECK-NEXT:   static_cast<void>(ref);
// CHECK-NEXT: }
void test_const_ref(const int &src) {
  int &ref = (int&)src;
  (void)ref;
}

// Тест 9: Приведение указателя к целочисленному типу должно стать reinterpret_cast
// CHECK: long test_ptr_to_int(int *ptr) {
// CHECK-NEXT:   return reinterpret_cast<long>(ptr);
// CHECK-NEXT: }
long test_ptr_to_int(int *ptr) {
  return (long)ptr;
}

// Тест 10: Приведение void* к целому типу должно стать reinterpret_cast
// CHECK: unsigned long test_void_ptr_to_int(void *ptr) {
// CHECK-NEXT:   return reinterpret_cast<unsigned long>(ptr);
// CHECK-NEXT: }
unsigned long test_void_ptr_to_int(void *ptr) {
  return (unsigned long)ptr;
}

// Тест 11: Приведение числа к указателю должно стать reinterpret_cast
// CHECK: int *test_int_to_ptr(long value) {
// CHECK-NEXT:   return reinterpret_cast<int*>(value);
// CHECK-NEXT: }
int *test_int_to_ptr(long value) {
  return (int*)value;
}

// Тест 12: Приведение одного указателя к другому типу указателя должно стать reinterpret_cast
// CHECK: double *test_ptr_to_ptr(int *ptr) {
// CHECK-NEXT:   return reinterpret_cast<double*>(ptr);
// CHECK-NEXT: }
double *test_ptr_to_ptr(int *ptr) {
  return (double*)ptr;
}

// Тест 13: Приведение void* → char* должно стать reinterpret_cast
// CHECK: char *test_void_to_char(void *ptr) {
// CHECK-NEXT:   return reinterpret_cast<char*>(ptr);
// CHECK-NEXT: }
char *test_void_to_char(void *ptr) {
  return (char*)ptr;
}

// Тест 14: Приведение int* → void* должно стать reinterpret_cast
// CHECK: void *test_other_ptr_to_void(int *ptr) {
// CHECK-NEXT:   return reinterpret_cast<void*>(ptr);
// CHECK-NEXT: }
void *test_other_ptr_to_void(int *ptr) {
  return (void*)ptr;
}

// Тест 15: Проверка функции с несколькими C-style cast в одном теле
// CHECK: int test_many(double d, const int *ptr, long raw, Color c) {
// CHECK-NEXT:   int a = static_cast<int>(d);
// CHECK-NEXT:   int *b = const_cast<int*>(ptr);
// CHECK-NEXT:   int *cptr = reinterpret_cast<int*>(raw);
// CHECK-NEXT:   long addr = reinterpret_cast<long>(cptr);
// CHECK-NEXT:   int e = static_cast<int>(c);
// CHECK-NEXT:   return a + *b + *cptr + static_cast<int>(addr) + e;
// CHECK-NEXT: }
int test_many(double d, const int *ptr, long raw, Color c) {
  int a = (int)d;
  int *b = (int*)ptr;
  int *cptr = (int*)raw;
  long addr = (long)cptr;
  int e = (int)c;
  return a + *b + *cptr + (int)addr + e;
}

// Тест 16: C-style cast внутри условия цикла for
// CHECK: int test_for_loop(double x) {
// CHECK-NEXT:   for (int i = 0; i < static_cast<int>(x); ++i) {
// CHECK-NEXT:   }
// CHECK-NEXT:   return 0;
// CHECK-NEXT: }
int test_for_loop(double x) {
  for (int i = 0; i < (int)x; ++i) {
  }
  return 0;
}

// Тест 17: Вложенные C-style cast должны быть заменены на вложенные static_cast
// CHECK: int test_nested(double x, double y) {
// CHECK-NEXT:   return static_cast<int>(static_cast<int>(x) + static_cast<int>(y));
// CHECK-NEXT: }
int test_nested(double x, double y) {
  return (int)((int)x + (int)y);
}

// Тест 18: C-style cast в операции присваивания
// CHECK: void test_assignment(double x) {
// CHECK-NEXT:   int value;
// CHECK-NEXT:   value = static_cast<int>(x);
// CHECK-NEXT: }
void test_assignment(double x) {
  int value;
  value = (int)x;
}

// Тест 19: C-style cast с лишними скобками
// CHECK: int test_parenthesized(double x) {
// CHECK-NEXT:   return static_cast<int>(x);
// CHECK-NEXT: }
int test_parenthesized(double x) {
  return ((int)(x));
}

// Тест 20: C-style cast для сложного выражения
// CHECK: int test_binary_expr(double x, double y) {
// CHECK-NEXT:   return static_cast<int>(x + y);
// CHECK-NEXT: }
int test_binary_expr(double x, double y) {
  return (int)(x + y);
}