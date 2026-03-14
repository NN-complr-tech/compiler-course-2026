class MyClass {
public:
  static int class_static;
};
int MyClass::class_static = 0;

static int sg1;
namespace N {
static int n_sg1;
}
namespace {
static int anon_sg1;
}
