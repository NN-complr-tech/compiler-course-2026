#include "mlir/Dialect/Func/IR/FuncOps.h"
#include "mlir/IR/PatternMatch.h"
#include "mlir/Pass/Pass.h"
#include "mlir/Tools/Plugins/PassPlugin.h"
#include "llvm/ADT/StringMap.h"
#include "llvm/Support/raw_ostream.h"

using namespace mlir;

namespace {
struct VolkovCallCounterPass : public PassWrapper<VolkovCallCounterPass, OperationPass<ModuleOp>> {
  StringRef getArgument() const final { return "volkov-call-counter"; }
  StringRef getDescription() const final { 
    return "Calculates the number of calls for each function and attaches it as an attribute."; 
  }

  void runOnOperation() override {
    ModuleOp module = getOperation();
    
    llvm::StringMap<int32_t> callFrequency;

    module.walk([&](func::CallOp callInst) {
      StringRef targetName = callInst.getCallee();
      callFrequency[targetName]++;
    });

    module.walk([&](func::FuncOp function) {
      int32_t callsCount = callFrequency.lookup(function.getName());
      
      auto attrType = IntegerType::get(function.getContext(), 32);
      auto countAttr = IntegerAttr::get(attrType, callsCount);
      
      function->setAttr("call_count", countAttr);
    });

    size_t totalOperations = 0;
    module.walk([&](Operation *op) {
      totalOperations++;
    });
    
    llvm::outs() << "Total amount of operations: " << totalOperations << '\n';
  }
};
} // namespace

MLIR_DECLARE_EXPLICIT_TYPE_ID(VolkovCallCounterPass)
MLIR_DEFINE_EXPLICIT_TYPE_ID(VolkovCallCounterPass)

mlir::PassPluginLibraryInfo getVolkovCallCounterPluginInfo() {
  return {
    MLIR_PLUGIN_API_VERSION, 
    "VolkovCallCounterPass", 
    "v1.0",[](PassRegistry *registry) {
      registry->insertPass([]() { return std::make_unique<VolkovCallCounterPass>(); });
    }
  };
}

extern "C" LLVM_ATTRIBUTE_WEAK mlir::PassPluginLibraryInfo mlirGetPassPluginInfo() {
  return getVolkovCallCounterPluginInfo();
}