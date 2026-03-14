// RUN: %clang_cc1 -load %llvmshlibdir/CStyleCastReplacerPlugin_Frolova_Sofya_FIIT3_ClangAST%pluginext -add-plugin cstyle_cast_replacer %s 2>&1 | FileCheck %s

void test_casts() {
    double d = 10.5;

    // CHECK: int i = static_cast<int>(d);
    int i = (int)d;

    // CHECK: int* p = reinterpret_cast<int*>(0x12345);
    int* p = (int*)0x12345;

    const int ci = 5;
    // CHECK: int i2 = static_cast<int>(ci);
    int i2 = (int)ci;
}

void test_const_casts() {
    const int* pci = nullptr;
    // CHECK: int* pi = const_cast<int*>(pci);
    int* pi = (int*)pci;                 

    int* pi2 = nullptr;
    // CHECK: const int* pci2 = const_cast<const int*>(pi2);
    const int* pci2 = (const int*)pi2;   
    const int& rci = 5;
    // CHECK: int& ri = const_cast<int&>(rci);
    int& ri = (int&)rci;                 

    int x = 42;
    int& ri2 = x;
    // CHECK: const int& rci2 = const_cast<const int&>(ri2);
    const int& rci2 = (const int&)ri2;    
}

void test_volatile_casts() {
    volatile int vi = 10;
    // CHECK: int i = static_cast<int>(vi);
    int i = (int)vi;                     

    volatile int* pvi = nullptr;
    // CHECK: int* pi = const_cast<int*>(pvi);   
    int* pi = (int*)pvi;

    const volatile int* pcvi = nullptr;
    // CHECK: int* pi2 = const_cast<int*>(pcvi); 
    int* pi2 = (int*)pcvi;
}

void test_reinterpret_casts() {
    int i = 42;
    // CHECK: char* pc = reinterpret_cast<char*>(&i);
    char* pc = (char*)&i;

    // CHECK: unsigned long addr = reinterpret_cast<unsigned long>(pc);
    unsigned long addr = (unsigned long)pc;

    // CHECK: int* pi = reinterpret_cast<int*>(addr);
    int* pi = (int*)addr;

    // CHECK: char& rc = reinterpret_cast<char&>(i);
    char& rc = (char&)i;

    // CHECK: int& ir = reinterpret_cast<int&>(rc);
    int& ir = (int&)rc;
}
