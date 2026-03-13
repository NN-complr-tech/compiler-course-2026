class MyClass {
public:
    static int class_static;
};

static int sg1;
namespace N { static int n_sg1; }
namespace { static int anon_sg1; }
int MyClass::class_static = 0;
const int ci = 5;
constexpr int cei = 10;
volatile int vi = 0;
extern int dup;
int dup;
extern int rep;
int rep;
int multi1, multi2, multi3;
