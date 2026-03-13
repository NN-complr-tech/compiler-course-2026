namespace N {
    int x = 1;
    static int y = 2;
}

static int global_static = 3;
int global_normal = 4;

void func(int a, int b) {
    static int static_local = 5;
    int local = a + b;
    
    for (int i = 0; i < 10; i++) {
        int loop_var = i;
        local += loop_var;
    }
    
    {
        int block_var = 42;
        local += block_var;
    }
}

class Test {
public:
    Test(int val) : value(val) {}
    int value;
    static int count;
};

int Test::count = 0;

template<typename T>
T identity(T x) {
    return x;
}
