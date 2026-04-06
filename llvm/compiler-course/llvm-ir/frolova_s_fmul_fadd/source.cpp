#include "llvm/IR/Function.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/IntrinsicInst.h"
#include "llvm/Passes/PassBuilder.h"
#include "llvm/Passes/PassPlugin.h"
#include "llvm/Support/raw_ostream.h"
#include <vector>

namespace {

struct FrolovaFMulAddPass : llvm::PassInfoMixin<FrolovaFMulAddPass> {
  llvm::PreservedAnalyses run(llvm::Function &Func,
                              llvm::FunctionAnalysisManager &) {
    bool Changed = false;
    std::vector<llvm::IntrinsicInst *> Worklist;

    for (llvm::BasicBlock &BB : Func) {
      for (llvm::Instruction &I : BB) {
        if (auto *II = llvm::dyn_cast<llvm::IntrinsicInst>(&I)) {
          if (II->getIntrinsicID() == llvm::Intrinsic::fmuladd) {
            Worklist.push_back(II);
          }
        }
      }
    }

    for (llvm::IntrinsicInst *FMulAdd : Worklist) {
      llvm::IRBuilder<> Builder(FMulAdd);
      llvm::Value *A = FMulAdd->getOperand(0);
      llvm::Value *B = FMulAdd->getOperand(1);
      llvm::Value *C = FMulAdd->getOperand(2);

      llvm::Value *Mul = Builder.CreateFMul(A, B, "mul_part");
      llvm::Value *Add = Builder.CreateFAdd(Mul, C, "add_part");

      FMulAdd->replaceAllUsesWith(Add);
      FMulAdd->eraseFromParent();
      Changed = true;
    }

    return Changed ? llvm::PreservedAnalyses::none() : llvm::PreservedAnalyses::all();
  }

  static bool isRequired() { return true; }
};

} // namespace

extern "C" LLVM_ATTRIBUTE_WEAK ::llvm::PassPluginLibraryInfo
llvmGetPassPluginInfo() {
  return {LLVM_PLUGIN_API_VERSION, "FrolovaFMulAdd", "1.0",
          [](llvm::PassBuilder &PB) {
            PB.registerPipelineParsingCallback(
                [](llvm::StringRef Name, llvm::FunctionPassManager &FPM,
                   llvm::ArrayRef<llvm::PassBuilder::PipelineElement>) -> bool {
                  if (Name == "frolova_s_fmul_fadd") {
                    FPM.addPass(FrolovaFMulAddPass{});
                    return true;
                  }
                  return false;
                });
          }};
}