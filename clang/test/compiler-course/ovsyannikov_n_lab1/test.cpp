// RUN: %clang_cc1 -load %llvmshlibdir/ovsyannikov_n_lab1_ClangAST%pluginext -plugin ovsyannikov_no_override -Wno-inconsistent-missing-override -fsyntax-only -verify %s

class Base {
public:
    virtual void foo() {}
    virtual void bar() {}
    virtual ~Base() {}
};

class Derived : public Base {
public:
    void foo() {} // expected-warning {{ovsyannikov-check: method 'foo' overrides a virtual function but lacks 'override' specifier}}
    
    void bar() override {} 
};
