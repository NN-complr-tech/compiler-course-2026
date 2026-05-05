#include "mlir/Dialect/Affine/IR/AffineOps.h"
#include "mlir/Dialect/Func/IR/FuncOps.h"
#include "mlir/Dialect/SCF/IR/SCF.h"
#include "mlir/IR/Builders.h"
#include "mlir/IR/BuiltinOps.h"
#include "mlir/Pass/Pass.h"
#include "mlir/Tools/Plugins/PassPlugin.h"

using namespace mlir;

namespace {

class InsertTraceBranchesPass
    : public PassWrapper<InsertTraceBranchesPass, OperationPass<ModuleOp>> {
public:
  StringRef getArgument() const final { return "lifanov_k_mlir_MLIR"; }

  StringRef getDescription() const final {
    return "Adds trace calls to then/els regions of condiional operations";
  }

  void runOnOperation() override {
    ModuleOp module = getOperation();
    MLIRContext *context = module.getContext();
    OpBuilder builder(context);

    auto declareTraceFunction = [&](StringRef funcName) {
      if (module.lookupSymbol<func::FuncOp>(funcName))
        return;

      builder.setInsertionPointToStart(module.getBody());
      auto type = builder.getFunctionType({}, {});
      auto fn =
          builder.create<func::FuncOp>(builder.getUnknownLoc(), funcName, type);
      fn.setPrivate();
    };

    declareTraceFunction("trace_condition_then_begin");
    declareTraceFunction("trace_condition_then_end");
    declareTraceFunction("trace_condition_else_begin");
    declareTraceFunction("trace_condition_else_end");

    auto instrumentBlock = [&](Block &regionBlock, StringRef beginTrace,
                               StringRef endTrace) {
      if (regionBlock.empty())
        return;

      builder.setInsertionPointToStart(&regionBlock);
      builder.create<func::CallOp>(builder.getUnknownLoc(), beginTrace,
                                   TypeRange{});

      Operation *lastOp = regionBlock.getTerminator();
      builder.setInsertionPoint(lastOp);
      builder.create<func::CallOp>(builder.getUnknownLoc(), endTrace,
                                   TypeRange{});
    };

    module.walk([&](Operation *operation) {
      if (auto ifOp = dyn_cast<scf::IfOp>(operation)) {
        instrumentBlock(*ifOp.thenBlock(), "trace_condition_then_begin",
                        "trace_condition_then_end");

        if (ifOp.elseBlock()) {
          instrumentBlock(*ifOp.elseBlock(), "trace_condition_else_begin",
                          "trace_condition_else_end");
        }
        return;
      }

      if (auto affineIf = dyn_cast<affine::AffineIfOp>(operation)) {
        instrumentBlock(*affineIf.getThenBlock(), "trace_condition_then_begin",
                        "trace_condition_then_end");

        if (affineIf.hasElse()) {
          instrumentBlock(*affineIf.getElseBlock(),
                          "trace_condition_else_begin",
                          "trace_condition_else_end");
        }
      }
    });
  }
};

} // namespace

MLIR_DECLARE_EXPLICIT_TYPE_ID(InsertTraceBranchesPass)
MLIR_DEFINE_EXPLICIT_TYPE_ID(InsertTraceBranchesPass)

mlir::PassPluginLibraryInfo getInsertTraceBranchesPassInfo() {
  return {MLIR_PLUGIN_API_VERSION, "InsertTraceBranchesPass", "1.0",
          []() { PassRegistration<InsertTraceBranchesPass>(); }};
}

extern "C" LLVM_ATTRIBUTE_WEAK mlir::PassPluginLibraryInfo
mlirGetPassPluginInfo() {
  return getInsertTraceBranchesPassInfo();
}
