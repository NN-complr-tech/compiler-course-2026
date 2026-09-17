#include "llvm/ADT/SmallPtrSet.h"
#include "llvm/ADT/SmallVector.h"
#include "llvm/Analysis/LoopInfo.h"
#include "llvm/IR/Dominators.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/Module.h"
#include "llvm/Passes/PassBuilder.h"
#include "llvm/Passes/PassPlugin.h"
#include "llvm/Support/ErrorHandling.h"
#include "llvm/Transforms/Utils/BasicBlockUtils.h"

using namespace llvm;

struct LoopHooksPass : PassInfoMixin<LoopHooksPass> {
  PreservedAnalyses run(Function &F, FunctionAnalysisManager &) {
    if (F.isDeclaration() || F.getName() == "loop_start" ||
        F.getName() == "loop_end")
      return PreservedAnalyses::all();

    DominatorTree DT(F);
    LoopInfo Loops(DT);
    struct Edge {
      Instruction *Terminator;
      unsigned Successor;
      unsigned Starts;
      unsigned Ends;
    };
    SmallVector<Edge> Edges;
    for (BasicBlock &BB : F) {
      Instruction *Terminator = BB.getTerminator();
      SmallPtrSet<BasicBlock *, 4> Seen;
      for (unsigned I = 0; I < Terminator->getNumSuccessors(); ++I) {
        BasicBlock *Next = Terminator->getSuccessor(I);
        if (!Seen.insert(Next).second)
          continue;
        unsigned Starts = 0, Ends = 0;
        for (Loop *L = Loops.getLoopFor(Next); L && !L->contains(&BB);
             L = L->getParentLoop())
          ++Starts;
        for (Loop *L = Loops.getLoopFor(&BB); L && !L->contains(Next);
             L = L->getParentLoop())
          ++Ends;
        if (Starts || Ends) {
          if (isa<IndirectBrInst>(Terminator) || !Next->canSplitPredecessors())
            report_fatal_error("cannot instrument this loop edge");
          Edges.push_back({Terminator, I, Starts, Ends});
        }
      }
    }
    if (Edges.empty())
      return PreservedAnalyses::all();

    Type *Void = Type::getVoidTy(F.getContext());
    FunctionCallee Start =
        F.getParent()->getOrInsertFunction("loop_start", Void);
    FunctionCallee End = F.getParent()->getOrInsertFunction("loop_end", Void);
    for (const Edge &E : Edges) {
      BasicBlock *Block =
          SplitBlockPredecessors(E.Terminator->getSuccessor(E.Successor),
                                 {E.Terminator->getParent()}, ".loop");
      IRBuilder<> Builder(Block->getTerminator());
      for (unsigned I = 0; I < E.Ends; ++I)
        Builder.CreateCall(End);
      for (unsigned I = 0; I < E.Starts; ++I)
        Builder.CreateCall(Start);
    }
    for (Attribute::AttrKind Kind :
         {Attribute::Memory, Attribute::NoFree, Attribute::NoSync,
          Attribute::NoUnwind, Attribute::WillReturn, Attribute::Speculatable,
          Attribute::NoRecurse, Attribute::MustProgress})
      F.removeFnAttr(Kind);
    return PreservedAnalyses::none();
  }

  static bool isRequired() { return true; }
};

extern "C" LLVM_ATTRIBUTE_WEAK PassPluginLibraryInfo llvmGetPassPluginInfo() {
  return {LLVM_PLUGIN_API_VERSION, "LoopHooksPass", "0.1", [](PassBuilder &PB) {
            PB.registerPipelineParsingCallback(
                [](StringRef Name, FunctionPassManager &FPM,
                   ArrayRef<PassBuilder::PipelineElement>) {
                  if (Name != "loop-hooks")
                    return false;
                  FPM.addPass(LoopHooksPass());
                  return true;
                });
          }};
}
