#include "mlir/Dialect/LLVMIR/LLVMDialect.h"
#include "mlir/IR/PatternMatch.h"
#include "mlir/Pass/Pass.h"
#include "mlir/Tools/Plugins/PassPlugin.h"
#include "llvm/Support/raw_ostream.h"

#include "mlir/Dialect/Func/IR/FuncOps.h"
#include "mlir/Dialect/SCF/IR/SCF.h"
#include "mlir/Dialect/Affine/IR/AffineOps.h"


using namespace mlir;

namespace {
class LuzanEMaxDepthPass : public PassWrapper<LuzanEMaxDepthPass, OperationPass<func::FuncOp>> { // bcs operates on funcs
public:
  StringRef getArgument() const final { return "luzanemaxdepth"; }
  StringRef getDescription() const final { return "Description pass"; }

  void runOnOperation() override {
    ModuleOp moduleOp = getOperation();
    OpBuilder builder(moduleOp);

    auto countOp = 0;
    moduleOp.walk([&](Operation *op) { ++countOp; });

    llvm::outs() << "Count operations: " << countOp << '\n';
  }
};
} // namespace

MLIR_DECLARE_EXPLICIT_TYPE_ID(LuzanEMaxDepthPass)
MLIR_DEFINE_EXPLICIT_TYPE_ID(LuzanEMaxDepthPass)

mlir::PassPluginLibraryInfo getFunctionCallCounterPassPluginInfo() {
  return {MLIR_PLUGIN_API_VERSION, "luzanemaxdepth", "1.0",
          []() { mlir::PassRegistration<LuzanEMaxDepthPass>(); }};
}

extern "C" LLVM_ATTRIBUTE_WEAK mlir::PassPluginLibraryInfo
mlirGetPassPluginInfo() {
  return getFunctionCallCounterPassPluginInfo();
}
