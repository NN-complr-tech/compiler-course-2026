#include "mlir/Dialect/Func/IR/FuncOps.h"
#include "mlir/IR/BuiltinOps.h"
#include "mlir/Pass/Pass.h"
#include "mlir/Pass/PassRegistry.h"
#include "llvm/ADT/StringMap.h"

using namespace mlir;

namespace {

class CallCounterPass
    : public PassWrapper<CallCounterPass, OperationPass<ModuleOp>> {
public:
  MLIR_DEFINE_EXPLICIT_INTERNAL_INLINE_TYPE_ID(CallCounterPass)

  StringRef getArgument() const final { return "call-counter"; }
  StringRef getDescription() const final {
    return "Counts how many times each function is called";
  }

  void runOnOperation() override {
    ModuleOp module = getOperation();
    llvm::StringMap<int64_t> callCounts;

    module.walk([&](func::CallOp callOp) {
      StringRef funcName = callOp.getCallee();
      callCounts[funcName]++;
    });

    module.walk([&](func::FuncOp func) {
      StringRef funcName = func.getName();
      int64_t count = callCounts[funcName];
      auto callCountAttr =
          IntegerAttr::get(IntegerType::get(func.getContext(), 64), count);
      func->setAttr("call_count", callCountAttr);
    });
  }
};

} // namespace

static PassRegistration<CallCounterPass> registration;
