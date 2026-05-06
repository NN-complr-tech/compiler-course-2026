#include "llvm/ADT/DenseMap.h"
#include "llvm/ADT/SCCIterator.h"
#include "llvm/ADT/SmallVector.h"
#include "llvm/Analysis/CallGraph.h"
#include "llvm/IR/Constants.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/IntrinsicInst.h"
#include "llvm/IR/Metadata.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/PassManager.h"
#include "llvm/Passes/PassBuilder.h"
#include "llvm/Passes/PassPlugin.h"
#include "llvm/Transforms/Utils/Cloning.h"

using namespace llvm;

namespace {

constexpr unsigned MaxInlineInstructions = 15;
constexpr unsigned MaxRecursionDepth = 3;
constexpr char RecursionDepthMetadata[] = "course.inline.depth";

class FunctionInliningPass : public PassInfoMixin<FunctionInliningPass> {
  using RecursiveGroupMap = DenseMap<Function *, unsigned>;

public:
  PreservedAnalyses run(Module &M, ModuleAnalysisManager &) {
    RecursiveGroupMap RecursiveGroups = collectRecursiveGroups(M);
    bool Changed = false;

    bool LocalChange = false;
    do {
      LocalChange = false;

      SmallVector<CallBase *, 32> WorkList;
      collectCallSites(M, WorkList);

      for (CallBase *Call : WorkList) {
        if (Call == nullptr || Call->getParent() == nullptr)
          continue;

        if (!canInline(*Call, RecursiveGroups))
          continue;

        Function *Callee = Call->getCalledFunction();
        unsigned Depth = getRecursionDepth(*Call);

        InlineFunctionInfo IFI;
        InlineResult Result = InlineFunction(*Call, IFI);
        if (!Result.isSuccess())
          continue;

        markRecursiveCallSites(IFI.InlinedCallSites, RecursiveGroups, Callee,
                               Depth + 1);
        Changed = true;
        LocalChange = true;
      }
    } while (LocalChange);

    return Changed ? PreservedAnalyses::none() : PreservedAnalyses::all();
  }

  static bool isRequired() { return true; }

private:
  static void collectCallSites(Module &M, SmallVectorImpl<CallBase *> &Calls) {
    for (Function &F : M) {
      if (F.isDeclaration())
        continue;

      for (BasicBlock &BB : F)
        for (Instruction &I : BB)
          if (auto *Call = dyn_cast<CallBase>(&I))
            Calls.push_back(Call);
    }
  }

  static RecursiveGroupMap collectRecursiveGroups(Module &M) {
    RecursiveGroupMap Groups;
    CallGraph CG(M);
    unsigned NextGroupId = 0;

    for (scc_iterator<CallGraph *> It = scc_begin(&CG); !It.isAtEnd(); ++It) {
      const std::vector<CallGraphNode *> &SCC = *It;
      SmallVector<Function *, 4> Functions;

      for (CallGraphNode *Node : SCC)
        if (Function *F = Node->getFunction())
          Functions.push_back(F);

      if (Functions.empty())
        continue;

      bool IsRecursive = Functions.size() > 1;
      if (!IsRecursive)
        IsRecursive = hasSelfCall(*Functions.front());

      if (!IsRecursive)
        continue;

      for (Function *F : Functions)
        Groups[F] = NextGroupId;

      ++NextGroupId;
    }

    return Groups;
  }

  static bool hasSelfCall(Function &F) {
    for (BasicBlock &BB : F)
      for (Instruction &I : BB)
        if (auto *Call = dyn_cast<CallBase>(&I))
          if (Call->getCalledFunction() == &F)
            return true;

    return false;
  }

  static bool canInline(CallBase &Call,
                        const RecursiveGroupMap &RecursiveGroups) {
    Function *Callee = Call.getCalledFunction();
    if (Callee == nullptr || Callee->isDeclaration())
      return false;

    if (isa<IntrinsicInst>(&Call) || Call.isInlineAsm())
      return false;

    if (Callee->isVarArg())
      return false;

    if (Call.hasFnAttr(Attribute::NoInline) ||
        Callee->hasFnAttribute(Attribute::NoInline))
      return false;

    if (countInlineInstructions(*Callee) > MaxInlineInstructions)
      return false;

    auto GroupIt = RecursiveGroups.find(Callee);
    if (GroupIt == RecursiveGroups.end())
      return true;

    return getRecursionDepth(Call) < MaxRecursionDepth;
  }

  static unsigned countInlineInstructions(Function &F) {
    unsigned Count = 0;

    for (BasicBlock &BB : F) {
      for (Instruction &I : BB) {
        if (isa<DbgInfoIntrinsic>(&I))
          continue;

        ++Count;
      }
    }

    return Count;
  }

  static unsigned getRecursionDepth(CallBase &Call) {
    MDNode *Node = Call.getMetadata(RecursionDepthMetadata);
    if (Node == nullptr || Node->getNumOperands() != 1)
      return 0;

    auto *Depth = mdconst::dyn_extract<ConstantInt>(Node->getOperand(0));
    if (Depth == nullptr)
      return 0;

    return Depth->getZExtValue();
  }

  static void setRecursionDepth(CallBase &Call, unsigned Depth) {
    LLVMContext &Context = Call.getContext();
    Metadata *Value = ConstantAsMetadata::get(
        ConstantInt::get(Type::getInt32Ty(Context), Depth));
    Call.setMetadata(RecursionDepthMetadata, MDNode::get(Context, Value));
  }

  static void markRecursiveCallSites(ArrayRef<CallBase *> CallSites,
                                     const RecursiveGroupMap &RecursiveGroups,
                                     Function *InlinedFunction,
                                     unsigned Depth) {
    auto GroupIt = RecursiveGroups.find(InlinedFunction);
    if (GroupIt == RecursiveGroups.end())
      return;

    unsigned GroupId = GroupIt->second;
    for (CallBase *Call : CallSites) {
      if (Call == nullptr)
        continue;

      Function *Callee = Call->getCalledFunction();
      auto CalleeGroupIt = RecursiveGroups.find(Callee);
      if (CalleeGroupIt == RecursiveGroups.end())
        continue;

      if (CalleeGroupIt->second == GroupId)
        setRecursionDepth(*Call, Depth);
    }
  }
};

} // namespace

extern "C" LLVM_ATTRIBUTE_WEAK PassPluginLibraryInfo llvmGetPassPluginInfo() {
  return {LLVM_PLUGIN_API_VERSION, "FunctionInliningPass", "0.1",
          [](PassBuilder &PB) {
            PB.registerPipelineParsingCallback(
                [](StringRef Name, ModulePassManager &MPM,
                   ArrayRef<PassBuilder::PipelineElement>) {
                  if (Name == "function-inlining") {
                    MPM.addPass(FunctionInliningPass());
                    return true;
                  }
                  return false;
                });
          }};
}