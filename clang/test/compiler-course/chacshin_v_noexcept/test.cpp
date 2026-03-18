// RUN: %clang_cc1 -fcxx-exceptions -fexceptions -load %llvmshlibdir/libChacshinNoexcept_Chacshin_Vladimir_FIIT3_ClangAST%pluginext -add-plugin chacshin_noexcept_plugin -ast-dump %s | FileCheck %s

class Base {
public:
    virtual ~Base() {}
};

class Derived : public Base {};

struct CtorThrow {
    CtorThrow() { throw 1; }
};

struct CtorNoThrow {
    CtorNoThrow() {}
};

void empty() {}

void simpleThrow() {
    throw 42;
}

void withNew() {
    int* p = new int;
    delete p;
}

void castToRef() {
    Derived d;
    Base& b = d;
    Derived& d2 = dynamic_cast<Derived&>(b);
}

void castToPtr() {
    Derived d;
    Base* b = &d;
    Derived* d2 = dynamic_cast<Derived*>(b);
}

void callNoThrow() {
    empty();
}

void callThrow() {
    simpleThrow();
}

int fact(int n) {
    if (n <= 1) return 1;
    return n * fact(n - 1);
}

int factThrow(int n) {
    if (n <= 1) throw 1;
    return n * factThrow(n - 1);
}

void lambdaThrow() {
    auto l = []() { throw 1; };
    l();
}

void lambdaNoThrow() {
    auto l = []() {};
    l();
}

void createCtorThrow() {
    CtorThrow obj;
}

void createCtorNoThrow() {
    CtorNoThrow obj;
}

void callCtorThrow() {
    CtorThrow obj;
}

void callCtorNoThrow() {
    CtorNoThrow obj;
}

void foo(int) {}

void pointerCall(void (*f)(int)) {
    f(42);
}

// CHECK: FunctionDecl {{.*}} empty 'void () noexcept'
// CHECK: FunctionDecl {{.*}} simpleThrow 'void ()'
// CHECK: FunctionDecl {{.*}} withNew 'void ()'
// CHECK: FunctionDecl {{.*}} castToRef 'void ()'
// CHECK: FunctionDecl {{.*}} castToPtr 'void () noexcept'
// CHECK: FunctionDecl {{.*}} callNoThrow 'void () noexcept'
// CHECK: FunctionDecl {{.*}} callThrow 'void ()'
// CHECK: FunctionDecl {{.*}} fact 'int (int) noexcept'
// CHECK: FunctionDecl {{.*}} factThrow 'int (int)'
// CHECK: FunctionDecl {{.*}} lambdaThrow 'void ()'
// CHECK: FunctionDecl {{.*}} lambdaNoThrow 'void () noexcept'
// CHECK: FunctionDecl {{.*}} createCtorThrow 'void ()'
// CHECK: FunctionDecl {{.*}} createCtorNoThrow 'void () noexcept'
// CHECK: FunctionDecl {{.*}} callCtorThrow 'void ()'
// CHECK: FunctionDecl {{.*}} callCtorNoThrow 'void () noexcept'
// CHECK: FunctionDecl {{.*}} pointerCall 'void (void (*)(int))'