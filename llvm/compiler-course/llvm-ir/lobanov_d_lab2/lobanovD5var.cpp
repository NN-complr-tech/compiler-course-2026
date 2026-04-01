#include "llvm/IR/Function.h"
#include "llvm/IR/Instructions.h"
#include "llvm/Passes/PassBuilder.h"
#include "llvm/Passes/PassPlugin.h"
#include "llvm/Support/raw_ostream.h"

using namespace llvm;

namespace {

struct MulDivToShiftPass : PassInfoMixin<MulDivToShiftPass> {
  PreservedAnalyses run(Function &F, FunctionAnalysisManager &) {
    std::vector<std::pair<Instruction *, BinaryOperator *>> replacements;

    for (BasicBlock &BB : F) {
      for (Instruction &I : BB) {

        auto *BinOp = dyn_cast<BinaryOperator>(&I);
        if (!BinOp)
          continue;

        unsigned Opcode = BinOp->getOpcode();
        if (Opcode != Instruction::Mul && Opcode != Instruction::UDiv &&
            Opcode != Instruction::SDiv)
          continue;

        ConstantInt *ConstOp = nullptr;
        Value *OtherOp = nullptr;

        if (Opcode == Instruction::Mul) {
          if ((ConstOp = dyn_cast<ConstantInt>(BinOp->getOperand(0))) &&
              !isa<ConstantInt>(BinOp->getOperand(1))) {
            OtherOp = BinOp->getOperand(1);
          } else if ((ConstOp = dyn_cast<ConstantInt>(BinOp->getOperand(1))) &&
                     !isa<ConstantInt>(BinOp->getOperand(0))) {
            OtherOp = BinOp->getOperand(0);
          }
        } else {
          ConstOp = dyn_cast<ConstantInt>(BinOp->getOperand(1));
          if (ConstOp)
            OtherOp = BinOp->getOperand(0);
        }

        if (ConstOp && OtherOp) {
          APInt Val = ConstOp->getValue();
          if (Val.isPowerOf2() && Val.isNonNegative()) {
            unsigned ShiftAmt = Val.logBase2();
            Type *IntTy = OtherOp->getType();

            Constant *ShiftConst =
                ConstantInt::get(OtherOp->getType(), ShiftAmt);

            Instruction::BinaryOps ShiftOp;
            if (Opcode == Instruction::Mul)
              ShiftOp = Instruction::Shl;
            else if (Opcode == Instruction::UDiv)
              ShiftOp = Instruction::LShr;
            else // SDiv
              ShiftOp = Instruction::AShr;

            BinaryOperator *NewInst = BinaryOperator::Create(
                ShiftOp, OtherOp, ShiftConst, BinOp->getName(), &I);
            replacements.emplace_back(BinOp, NewInst);
          }
        }
      }
    }

    for (auto &Pair : replacements) {
      Instruction *Old = Pair.first;
      BinaryOperator *New = Pair.second;
      Old->replaceAllUsesWith(New);
      Old->eraseFromParent();
    }

    return replacements.empty() ? PreservedAnalyses::all()
                                : PreservedAnalyses::none();
  }

  static bool isRequired() { return true; }
};

} // namespace

extern "C" LLVM_ATTRIBUTE_WEAK ::llvm::PassPluginLibraryInfo
llvmGetPassPluginInfo() {
  return {LLVM_PLUGIN_API_VERSION, "MulDivToShiftPass", "0.1",
          [](PassBuilder &PB) {
            PB.registerPipelineParsingCallback(
                [](StringRef name, FunctionPassManager &FPM,
                   ArrayRef<PassBuilder::PipelineElement>) -> bool {
                  if (name == "mul-div-to-shift") {
                    FPM.addPass(MulDivToShiftPass{});
                    return true;
                  }
                  return false;
                });
          }};
}