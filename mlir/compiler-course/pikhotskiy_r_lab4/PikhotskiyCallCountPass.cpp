#include "mlir/Dialect/Func/IR/FuncOps.h"
#include "mlir/IR/BuiltinOps.h"
#include "mlir/Pass/Pass.h"
#include "mlir/Tools/Plugins/PassPlugin.h"
#include "llvm/ADT/StringMap.h"

using namespace mlir;

namespace {
class PikhotskiyCallCountPass
    : public PassWrapper<PikhotskiyCallCountPass, OperationPass<ModuleOp>> {
public:
  StringRef getArgument() const final { return "pikhotskiy-call-count"; }
  StringRef getDescription() const final {
    return "Counts incoming calls from other func.func operations and stores "
           "the result in a call_count attribute";
  }

  void runOnOperation() override {
    ModuleOp module = getOperation();
    Builder builder(&getContext());

    llvm::StringMap<int64_t> incomingCalls;
    for (func::FuncOp function : module.getOps<func::FuncOp>())
      incomingCalls[function.getSymName()] = 0;

    for (func::FuncOp caller : module.getOps<func::FuncOp>()) {
      StringRef callerName = caller.getSymName();
      caller.walk([&](func::CallOp callOp) {
        StringRef calleeName = callOp.getCallee();
        if (calleeName == callerName)
          return;
        auto it = incomingCalls.find(calleeName);
        if (it != incomingCalls.end())
          ++it->second;
      });
    }

    for (func::FuncOp function : module.getOps<func::FuncOp>()) {
      int64_t count = incomingCalls.lookup(function.getSymName());
      function->setAttr("call_count", builder.getI64IntegerAttr(count));
    }
  }
};
} // namespace

MLIR_DECLARE_EXPLICIT_TYPE_ID(PikhotskiyCallCountPass)
MLIR_DEFINE_EXPLICIT_TYPE_ID(PikhotskiyCallCountPass)

mlir::PassPluginLibraryInfo getPikhotskiyCallCountPassPluginInfo() {
  return {MLIR_PLUGIN_API_VERSION, "PikhotskiyCallCountPass", "1.0",
          []() { mlir::PassRegistration<PikhotskiyCallCountPass>(); }};
}

extern "C" LLVM_ATTRIBUTE_WEAK mlir::PassPluginLibraryInfo
mlirGetPassPluginInfo() {
  return getPikhotskiyCallCountPassPluginInfo();
}
