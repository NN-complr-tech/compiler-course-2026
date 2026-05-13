#include "mlir/Dialect/Func/IR/FuncOps.h"
#include "mlir/IR/BuiltinOps.h"
#include "mlir/Interfaces/CallInterfaces.h"
#include "mlir/Pass/Pass.h"
#include "mlir/Pass/PassManager.h"
#include "llvm/ADT/StringMap.h"
#include "llvm/Support/Compiler.h"

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

    module.walk([&](CallOpInterface callOp) {
      auto callee = callOp.getCallableForCallee().dyn_cast<SymbolRefAttr>();
      if (callee) {
        StringRef funcName = callee.getRootReference().getValue();
        callCounts[funcName]++;
      }
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

namespace mlir {
namespace compiler_course {

std::unique_ptr<Pass> createCallCounterPass() {
  return std::make_unique<CallCounterPass>();
}

} // namespace compiler_course
} // namespace mlir

#define MLIR_PLUGIN_API_VERSION 1

extern "C" LLVM_ATTRIBUTE_WEAK mlir::PassPluginLibraryInfo
mlirGetPassPluginInfo() {
  struct PassPluginLibraryInfo info;
  info.apiVersion = MLIR_PLUGIN_API_VERSION;
  info.pluginName = "call_counter";
  info.pluginVersion = "0.1";
  info.registerPassRegistryCallback = [](mlir::PassRegistry &registry) {
    registry.addPass(mlir::compiler_course::createCallCounterPass());
  };
  return info;
}
