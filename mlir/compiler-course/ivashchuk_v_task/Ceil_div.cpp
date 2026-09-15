#include "mlir/Dialect/Arith/IR/Arith.h"
#include "mlir/IR/BuiltinOps.h"
#include "mlir/Pass/Pass.h"
#include "mlir/Tools/Plugins/PassPlugin.h"

using namespace mlir;

namespace {

class CeilDivPass : public PassWrapper<CeilDivPass, OperationPass<ModuleOp>> {
public:
  StringRef getArgument() const final { return "ceil_div"; }
  StringRef getDescription() const final {
    return "Change arith.ceildivui and arith.ceildivsi for (a + b - 1) / b";
  }

  void getDependentDialects(DialectRegistry &registry) const override {
    registry.insert<arith::ArithDialect>();
  }

  void runOnOperation() override {
    getOperation().walk([&](Operation *op) {
      if (auto ui = dyn_cast<arith::CeilDivUIOp>(op)) {
        changeCeilDiv(ui, /*isSigned=*/false);
      } else if (auto si = dyn_cast<arith::CeilDivSIOp>(op)) {
        changeCeilDiv(si, /*isSigned=*/true);
      }
    });
  }

private:
  template <typename CeilDivOpTy>
  void changeCeilDiv(CeilDivOpTy op, bool isSigned) {
    OpBuilder builder(op);
    Location loc = op.getLoc();
    Value a = op.getLhs();
    Value b = op.getRhs();
    Value one = builder.create<arith::ConstantOp>(
        loc, builder.getIntegerAttr(op.getType(), 1));
    Value bMinusOne = builder.create<arith::SubIOp>(loc, b, one);
    Value aPlusBMinusOne = builder.create<arith::AddIOp>(loc, a, bMinusOne);
    Value result =
        isSigned
            ? builder.create<arith::DivSIOp>(loc, aPlusBMinusOne, b).getResult()
            : builder.create<arith::DivUIOp>(loc, aPlusBMinusOne, b)
                  .getResult();
    op.replaceAllUsesWith(result);
    op.erase();
  }
};
} // namespace

MLIR_DECLARE_EXPLICIT_TYPE_ID(CeilDivPass)
MLIR_DEFINE_EXPLICIT_TYPE_ID(CeilDivPass)
mlir::PassPluginLibraryInfo getCeilDivPassPluginInfo() {
  return {MLIR_PLUGIN_API_VERSION, "CeilDivPass", "1.0",
          []() { mlir::PassRegistration<CeilDivPass>(); }};
}
extern "C" LLVM_ATTRIBUTE_WEAK mlir::PassPluginLibraryInfo
mlirGetPassPluginInfo() {
  return getCeilDivPassPluginInfo();
}