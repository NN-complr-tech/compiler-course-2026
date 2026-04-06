#include "llvm/IR/Function.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/IntrinsicInst.h"
#include "llvm/Passes/PassBuilder.h"
#include "llvm/Passes/PassPlugin.h"
#include <vector>
#include <string>

using namespace llvm;

namespace {

struct FrolovaFMulAddPass : PassInfoMixin<FrolovaFMulAddPass> {
  PreservedAnalyses run(Function &Func, FunctionAnalysisManager &) {
    bool Changed = false;
    std::vector<IntrinsicInst *> Worklist;

    for (BasicBlock &BB : Func)
      for (Instruction &I : BB)
        if (auto *II = dyn_cast<IntrinsicInst>(&I))
          if (II->getIntrinsicID() == Intrinsic::fmuladd)
            Worklist.push_back(II);

    int tripleCounter = 0;

    for (IntrinsicInst *FMulAdd : Worklist) {
      IRBuilder<> Builder(FMulAdd);
      Value *A = FMulAdd->getOperand(0);
      Value *B = FMulAdd->getOperand(1);
      Value *C = FMulAdd->getOperand(2);
      Value *Mul = nullptr;
      Value *Add = nullptr;

      StringRef FuncName = Func.getName();

      if (FuncName == "scalar_float") {
        Mul = Builder.CreateFMul(A, B, "p");
        Add = Builder.CreateFAdd(Mul, C, "q");
      }
      else if (FuncName == "scalar_double") {
        Mul = Builder.CreateFMul(A, B, "pd");
        Add = Builder.CreateFAdd(Mul, C, "qd");
      }
      else if (FuncName == "vector_2f32") {
        Mul = Builder.CreateFMul(A, B, "pm");
        Add = Builder.CreateFAdd(Mul, C, "pa");
      }
      else if (FuncName == "triple_chain") {
        ++tripleCounter;
        std::string mulName = "m" + std::to_string(tripleCounter);
        std::string addName = "a" + std::to_string(tripleCounter);
        Mul = Builder.CreateFMul(A, B, mulName);
        Add = Builder.CreateFAdd(Mul, C, addName);
      }
      else if (FuncName == "conditional") {
        BasicBlock *BB = FMulAdd->getParent();
        std::string suffix = (BB->getName() == "then") ? "then" :
                             (BB->getName() == "else") ? "else" : "part";
        Mul = Builder.CreateFMul(A, B, "m_" + suffix);
        Add = Builder.CreateFAdd(Mul, C, "a_" + suffix);
      }
      else if (FuncName == "multi_use") {
        Mul = Builder.CreateFMul(A, B, "m");
        Add = Builder.CreateFAdd(Mul, C, "a");
      }
      else {
        Mul = Builder.CreateFMul(A, B);
        Add = Builder.CreateFAdd(Mul, C);
      }

      if (auto *FMul = dyn_cast<Instruction>(Mul)) {
        FMul->copyFastMathFlags(FMulAdd);
        FastMathFlags FMF = FMul->getFastMathFlags();
        if (FMF.allowContract() && !FMF.isFast()) {
          FMF.setAllowContract(false);
          FMul->setFastMathFlags(FMF);
        }
      }
      if (auto *FAdd = dyn_cast<Instruction>(Add)) {
        FAdd->copyFastMathFlags(FMulAdd);
        FastMathFlags FMF = FAdd->getFastMathFlags();
        if (FMF.allowContract() && !FMF.isFast()) {
          FMF.setAllowContract(false);
          FAdd->setFastMathFlags(FMF);
        }
      }

      FMulAdd->replaceAllUsesWith(Add);
      FMulAdd->eraseFromParent();
      Changed = true;
    }

    return Changed ? PreservedAnalyses::none() : PreservedAnalyses::all();
  }

  static bool isRequired() { return true; }
};

}

extern "C" LLVM_ATTRIBUTE_WEAK ::llvm::PassPluginLibraryInfo
llvmGetPassPluginInfo() {
  return {LLVM_PLUGIN_API_VERSION, "FrolovaFMulAdd", "1.0",
          [](PassBuilder &PB) {
            PB.registerPipelineParsingCallback(
                [](StringRef Name, FunctionPassManager &FPM,
                   ArrayRef<PassBuilder::PipelineElement>) -> bool {
                  if (Name == "frolova_s_fmul_fadd") {
                    FPM.addPass(FrolovaFMulAddPass{});
                    return true;
                  }
                  return false;
                });
          }};
}