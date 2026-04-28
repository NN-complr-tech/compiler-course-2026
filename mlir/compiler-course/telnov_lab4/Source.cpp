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
    return "Attach trip_count attribute to affine.for loops with known bounds";
  }

  void runOnOperation() override {
    Builder builder(getOperation().getContext());

    getOperation().walk([&](affine::AffineForOp loop) {
      loop->removeAttr("trip_count");

      std::optional<int64_t> count = calculateTripCount(loop);
      if (!count.has_value())
        return;

      loop->setAttr("trip_count", builder.getI64IntegerAttr(*count));
    });
  }

private:
  std::optional<int64_t> calculateTripCount(affine::AffineForOp loop) const {
    std::optional<int64_t> lower = loop.getConstantLowerBound();
    std::optional<int64_t> upper = loop.getConstantUpperBound();

    if (!lower.has_value() || !upper.has_value())
      return std::nullopt;

    int64_t step = static_cast<int64_t>(loop.getStep());
    if (step <= 0)
      return std::nullopt;

    int64_t range = *upper - *lower;
    if (range <= 0)
      return 0;

    return (range + step - 1) / step;
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