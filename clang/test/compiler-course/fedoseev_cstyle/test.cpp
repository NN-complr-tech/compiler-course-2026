// RUN: %clang_cc1 -load %llvmshlibdir/CstyleCastPlugin_Fedoseev_Sergey_FIIT0_ClangAST%pluginext -plugin cstyle_cast_plugin -fsyntax-only %s 2>&1 | FileCheck %s

// Арифметическое приведение
void arithmetic() {
    int i = 10;
    double d = (double)i; // CHECK: double d = static_cast<double>(i);
}

// const_cast
void const_qualifier() {
    const int ci = 5;
    int* ptr = (int*)&ci; // CHECK: int* ptr = const_cast<int*>(&ci);
}

// reinterpret_cast (указатели на разные типы)
void pointer_cast() {
    int* p = nullptr;
    char* c = (char*)p; // CHECK: char* c = reinterpret_cast<char*>(p);
}

// dynamic_cast (полиморфные классы)
struct Base { virtual ~Base() = default; };
struct Derived : Base {};

void polymorphic_cast() {
    Base* b = new Derived;
    Derived* d = (Derived*)b; // CHECK: Derived* d = dynamic_cast<Derived*>(b);
}

// static_cast (неполиморфные указатели)
struct NonPolyBase {};
struct NonPolyDerived : NonPolyBase {};

void non_polymorphic_cast() {
    NonPolyBase* b = new NonPolyDerived;
    NonPolyDerived* d = (NonPolyDerived*)b; // CHECK: NonPolyDerived* d = reinterpret_cast<NonPolyDerived*>(b);
}

// Приведение к ссылке (static_cast)
void reference_cast() {
    int i = 42;
    double& d = (double&)i; // CHECK: double& d = static_cast<double&>(i);
}