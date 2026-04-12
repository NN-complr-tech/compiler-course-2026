#include "llvm/IR/Constants.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/Instructions.h"
#include "llvm/Support/raw_ostream.h"

using namespace llvm;

namespace {

class ReplacePowi {
public:
  bool runOnFunction(Function &F) {
    bool Changed = false;

    for (BasicBlock &BB : F) {
      for (Instruction &I : BB) {
        auto *Call = dyn_cast<CallInst>(&I);
        if (!Call)
          continue;

        Function *Callee = Call->getCalledFunction();
        if (!Callee)
          continue;

        StringRef Name = Callee->getName();
        if (!Name.contains("llvm.powi"))
          continue;

        if (Call->arg_size() < 2)
          continue;

        Value *PowerArg = Call->getArgOperand(1);
        ConstantInt *ConstPower = dyn_cast<ConstantInt>(PowerArg);
        if (!ConstPower)
          continue;

        int64_t Power = ConstPower->getSExtValue();
        if (Power < 0 || Power > 4)
          continue;

        Value *Base = Call->getArgOperand(0);
        IRBuilder<> Builder(Call);
        Value *NewResult = nullptr;

        if (Power == 0) {
          NewResult = ConstantFP::get(Base->getType(), 1.0);
        } else if (Power == 1) {
          NewResult = Base;
        } else if (Power == 2) {
          NewResult = Builder.CreateFMul(Base, Base);
        } else if (Power == 3) {
          Value *Sq = Builder.CreateFMul(Base, Base);
          NewResult = Builder.CreateFMul(Sq, Base);
        } else if (Power == 4) {
          Value *Sq = Builder.CreateFMul(Base, Base);
          NewResult = Builder.CreateFMul(Sq, Sq);
        }

        if (NewResult) {
          Call->replaceAllUsesWith(NewResult);
          Call->eraseFromParent();
          Changed = true;
        }
      }
    }

    return Changed;
  }
};

} // namespace

namespace llvm {
void initializeReplacePowiLegacyPass(PassRegistry &);
}

namespace {
struct ReplacePowiLegacy : public FunctionPass {
  static char ID;
  ReplacePowiLegacy() : FunctionPass(ID) {
    initializeReplacePowiLegacyPass(*PassRegistry::getPassRegistry());
  }

  bool runOnFunction(Function &F) override {
    ReplacePowi Pass;
    return Pass.runOnFunction(F);
  }

  StringRef getPassName() const override { return "ReplacePowi"; }
};

char ReplacePowiLegacy::ID = 0;
} // namespace

INITIALIZE_PASS(ReplacePowiLegacy, "replace-powi-legacy",
                "Replace powi with multiplications", false, false)

static RegisterPass<ReplacePowiLegacy> X("replace-powi",
                                         "Replace powi with multiplications");
