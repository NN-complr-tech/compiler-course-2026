#include "llvm/IR/Function.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/Instructions.h"
#include "llvm/Passes/PassBuilder.h"
#include "llvm/Passes/PassPlugin.h"
#include "llvm/Support/raw_ostream.h"

namespace {
struct MulDelOptimization : llvm::PassInfoMixin<MulDelOptimization> {
  llvm::PreservedAnalyses run(llvm::Function &func,
                              llvm::FunctionAnalysisManager &) {
    for (llvm::BasicBlock &bb : func) {
      for (llvm::Instruction &instr : llvm::make_early_inc_range(bb)) {
        if (llvm::BinaryOperator *binOp =
                llvm::dyn_cast<llvm::BinaryOperator>(&instr)) {
          if (binOp->getOpcode() == llvm::Instruction::Mul) {
            binOp->dump();
            llvm::Value *lhs = binOp->getOperand(0);
            llvm::Value *rhs = binOp->getOperand(1);
            llvm::IRBuilder<> builder(binOp);
            llvm::Value *shiftOp = builder.CreateShl(lhs, rhs);
            binOp->replaceAllUsesWith(shiftOp);
            binOp->eraseFromParent();
          }
        }
      }
    }
    return llvm::PreservedAnalyses::all();
  }

  static bool isRequired() { return true; }
};
} // namespace

extern "C" LLVM_ATTRIBUTE_WEAK ::llvm::PassPluginLibraryInfo
llvmGetPassPluginInfo() {
  return {LLVM_PLUGIN_API_VERSION, "MulDelOptimization", "0.1",
          [](llvm::PassBuilder &PB) {
            PB.registerPipelineParsingCallback(
                [](llvm::StringRef name, llvm::FunctionPassManager &FPM,
                   llvm::ArrayRef<llvm::PassBuilder::PipelineElement>) -> bool {
                  if (name == "mul-del-optimization") {
                    FPM.addPass(MulDelOptimization{});
                    return true;
                  }
                  return false;
                });
          }};
}
