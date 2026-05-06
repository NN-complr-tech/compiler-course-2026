#include "mlir/Dialect/Affine/IR/AffineOps.h"
#include "mlir/IR/Builders.h"
#include "mlir/IR/BuiltinOps.h"
#include "mlir/Pass/Pass.h"
#include "mlir/Tools/Plugins/PassPlugin.h"
#include "llvm/Support/raw_ostream.h"
#include <cstdint>
#include <optional>

using namespace mlir;

namespace {

class ShkrebkoTripCountPass
    : public PassWrapper<ShkrebkoTripCountPass, OperationPass<ModuleOp>> {
public:
  StringRef getArgument() const final { return "shkrebko-trip-count"; }

  StringRef getDescription() const final {
    return "Annotate affine.for loops with constant 'trip_count' attribute";
  }

  void runOnOperation() override {
    ModuleOp module = getOperation();
    Builder builder(module.getContext());

    module.walk([&](affine::AffineForOp forOp) {
      if (forOp->hasAttr("trip_count"))
        return;

      std::optional<int64_t> tripCount = computeTripCount(forOp);
      if (tripCount.has_value()) {
        forOp->setAttr("trip_count",
                       builder.getI64IntegerAttr(tripCount.value()));
      }
    });
  }

private:
  std::optional<int64_t> computeTripCount(affine::AffineForOp forOp) const {
    if (!forOp.hasConstantLowerBound() || !forOp.hasConstantUpperBound())
      return std::nullopt;

    int64_t lb = forOp.getConstantLowerBound();
    int64_t ub = forOp.getConstantUpperBound();
    int64_t step = forOp.getStepAsInt();

    if (step <= 0)
      return std::nullopt;

    if (lb >= ub)
      return 0;

    int64_t distance = ub - lb;
    int64_t iterations = (distance + step - 1) / step;
    return iterations;
  }
};

} // namespace

MLIR_DECLARE_EXPLICIT_TYPE_ID(ShkrebkoTripCountPass)
MLIR_DEFINE_EXPLICIT_TYPE_ID(ShkrebkoTripCountPass)

mlir::PassPluginLibraryInfo getShkrebkoTripCountPassPluginInfo() {
  return {MLIR_PLUGIN_API_VERSION, "ShkrebkoTripCountPass", "1.0",
          []() { mlir::PassRegistration<ShkrebkoTripCountPass>(); }};
}

extern "C" LLVM_ATTRIBUTE_WEAK mlir::PassPluginLibraryInfo
mlirGetPassPluginInfo() {
  return getShkrebkoTripCountPassPluginInfo();
}