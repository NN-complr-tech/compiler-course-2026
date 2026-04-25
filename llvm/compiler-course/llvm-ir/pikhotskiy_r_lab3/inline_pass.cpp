#include "llvm/ADT/SmallVector.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/Instructions.h"
#include "llvm/Passes/PassBuilder.h"
#include "llvm/Passes/PassPlugin.h"
#include "llvm/Transforms/Utils/Cloning.h"

namespace {

constexpr unsigned MaxInlineInstructions = 15;
constexpr unsigned MaxRecursionDepth = 3;
constexpr unsigned MaxInliningIterations = 64;

unsigned getInstructionCount(const llvm::Function &F) {
  unsigned Count = 0;
  for (const llvm::BasicBlock &BB : F)
    Count += static_cast<unsigned>(BB.size());
  return Count;
}

bool canInlineCallee(const llvm::Function *Callee) {
  if (!Callee)
    return false;
  if (Callee->isDeclaration() || Callee->isIntrinsic())
    return false;
  if (Callee->isVarArg())
    return false;
  return getInstructionCount(*Callee) <= MaxInlineInstructions;
}

struct PikhotskiyInliningPass
    : llvm::PassInfoMixin<PikhotskiyInliningPass> {
  llvm::PreservedAnalyses run(llvm::Function &F,
                              llvm::FunctionAnalysisManager &) {
    bool Changed = false;
    unsigned RecursiveInlineDepth = 0;

    for (unsigned Iteration = 0; Iteration < MaxInliningIterations;
         ++Iteration) {
      llvm::SmallVector<llvm::CallBase *, 16> Calls;
      for (llvm::BasicBlock &BB : F) {
        for (llvm::Instruction &I : BB) {
          if (auto *CB = llvm::dyn_cast<llvm::CallBase>(&I))
            Calls.push_back(CB);
        }
      }

      bool IterationChanged = false;
      bool InlinedRecursiveCall = false;

      for (llvm::CallBase *CB : Calls) {
        if (!CB->getParent())
          continue;

        llvm::Function *Callee = CB->getCalledFunction();
        if (!canInlineCallee(Callee))
          continue;

        bool IsRecursive = Callee == &F;
        if (IsRecursive && RecursiveInlineDepth >= MaxRecursionDepth)
          continue;

        llvm::InlineFunctionInfo IFI;
        llvm::InlineResult Result = llvm::InlineFunction(*CB, IFI);
        if (!Result.isSuccess())
          continue;

        Changed = true;
        IterationChanged = true;
        if (IsRecursive)
          InlinedRecursiveCall = true;
      }

      if (InlinedRecursiveCall)
        ++RecursiveInlineDepth;

      if (!IterationChanged)
        break;
    }

    return Changed ? llvm::PreservedAnalyses::none()
                   : llvm::PreservedAnalyses::all();
  }

  static bool isRequired() { return true; }
};

} // namespace

extern "C" LLVM_ATTRIBUTE_WEAK ::llvm::PassPluginLibraryInfo
llvmGetPassPluginInfo() {
  return {LLVM_PLUGIN_API_VERSION, "PikhotskiyInliningPass", "0.1",
          [](llvm::PassBuilder &PB) {
            PB.registerPipelineParsingCallback(
                [](llvm::StringRef Name, llvm::FunctionPassManager &FPM,
                   llvm::ArrayRef<llvm::PassBuilder::PipelineElement>) -> bool {
                  if (Name == "pikhotskiy-inline-pass") {
                    FPM.addPass(PikhotskiyInliningPass{});
                    return true;
                  }
                  return false;
                });
          }};
}
