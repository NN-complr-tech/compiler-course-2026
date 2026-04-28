#include "mlir/Dialect/Affine/IR/AffineOps.h"
#include "mlir/IR/Builders.h"
#include "mlir/IR/BuiltinOps.h"
#include "mlir/Pass/Pass.h"
#include "mlir/Tools/Plugins/PassPlugin.h"

#include <cstdint>
#include <optional>

using namespace mlir;

namespace {

class ExamplePass : public PassWrapper<ExamplePass, OperationPass<ModuleOp>> {
public:
  StringRef getArgument() const final { return "example_MLIR"; }

  StringRef getDescription() const final {
    return "Annotate affine.for loops with trip_count attribute";
  }

  void runOnOperation() override {
    ModuleOp module = getOperation();
    Builder builder(module.getContext());

    module.walk([&](affine::AffineForOp forOp) {
      std::optional<int64_t> tripCount = getTripCount(forOp);
      if (!tripCount.has_value())
        return;

      forOp->setAttr("trip_count", builder.getI64IntegerAttr(*tripCount));
    });
  }

private:
  std::optional<int64_t> getTripCount(affine::AffineForOp forOp) const {
    std::optional<int64_t> lowerBound = forOp.getConstantLowerBound();
    std::optional<int64_t> upperBound = forOp.getConstantUpperBound();

    if (!lowerBound.has_value() || !upperBound.has_value())
      return std::nullopt;

    int64_t step = static_cast<int64_t>(forOp.getStep());
    if (step <= 0)
      return std::nullopt;

    int64_t distance = *upperBound - *lowerBound;
    if (distance <= 0)
      return 0;

    return (distance + step - 1) / step;
  }
};

} // namespace

MLIR_DECLARE_EXPLICIT_TYPE_ID(ExamplePass)
MLIR_DEFINE_EXPLICIT_TYPE_ID(ExamplePass)

mlir::PassPluginLibraryInfo getExamplePassPluginInfo() {
  return {MLIR_PLUGIN_API_VERSION, "ExamplePass", "1.0",
          []() { mlir::PassRegistration<ExamplePass>(); }};
}

extern "C" LLVM_ATTRIBUTE_WEAK mlir::PassPluginLibraryInfo
mlirGetPassPluginInfo() {
  return getExamplePassPluginInfo();
}