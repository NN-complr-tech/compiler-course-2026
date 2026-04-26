#include "X86.h"
#include "X86InstrInfo.h"
#include "X86Subtarget.h"
#include "llvm/CodeGen/MachineFunctionPass.h"
#include "llvm/CodeGen/MachineInstrBuilder.h"
#include <vector>

using namespace llvm;

namespace {

class IncDecToAddSubPass : public MachineFunctionPass {
public:
  static char ID;
  IncDecToAddSubPass() : MachineFunctionPass(ID) {}
  bool runOnMachineFunction(MachineFunction &MF) override;

private:
  bool isIncDec(unsigned Opc, int &Delta, unsigned &AddOpc,
                unsigned &SubOpc) const {
    switch (Opc) {
    case X86::INC8r:
      Delta = 1;
      AddOpc = X86::ADD8ri;
      SubOpc = X86::SUB8ri;
      return true;
    case X86::DEC8r:
      Delta = -1;
      AddOpc = X86::ADD8ri;
      SubOpc = X86::SUB8ri;
      return true;
    case X86::INC16r:
      Delta = 1;
      AddOpc = X86::ADD16ri;
      SubOpc = X86::SUB16ri;
      return true;
    case X86::DEC16r:
      Delta = -1;
      AddOpc = X86::ADD16ri;
      SubOpc = X86::SUB16ri;
      return true;
    case X86::INC32r:
      Delta = 1;
      AddOpc = X86::ADD32ri;
      SubOpc = X86::SUB32ri;
      return true;
    case X86::DEC32r:
      Delta = -1;
      AddOpc = X86::ADD32ri;
      SubOpc = X86::SUB32ri;
      return true;
    case X86::INC64r:
      Delta = 1;
      AddOpc = X86::ADD64ri32;
      SubOpc = X86::SUB64ri32;
      return true;
    case X86::DEC64r:
      Delta = -1;
      AddOpc = X86::ADD64ri32;
      SubOpc = X86::SUB64ri32;
      return true;
    default:
      return false;
    }
  }
};

char IncDecToAddSubPass::ID = 0;

bool IncDecToAddSubPass::runOnMachineFunction(MachineFunction &MF) {
  const TargetInstrInfo *TII = MF.getSubtarget().getInstrInfo();
  bool Changed = false;

  for (auto &MBB : MF) {
    for (auto It = MBB.begin(); It != MBB.end();) {
      int BaseDelta = 0;
      unsigned TargetAdd = 0, TargetSub = 0;

      if (!isIncDec(It->getOpcode(), BaseDelta, TargetAdd, TargetSub)) {
        ++It;
        continue;
      }

      Register TargetReg = It->getOperand(0).getReg();
      int TotalDelta = BaseDelta;

      std::vector<MachineInstr *> InstsToRemove;
      InstsToRemove.push_back(&*It);

      auto NextIt = std::next(It);

      while (NextIt != MBB.end()) {
        int NextDelta = 0;
        unsigned NextAdd = 0, NextSub = 0;

        if (!isIncDec(NextIt->getOpcode(), NextDelta, NextAdd, NextSub))
          break;

        if (NextIt->getOperand(0).getReg() != TargetReg)
          break;

        if (TargetAdd != NextAdd)
          break;

        TotalDelta += NextDelta;
        InstsToRemove.push_back(&*NextIt);
        ++NextIt;
      }

      if (TotalDelta != 0) {
        unsigned NewOpc = (TotalDelta > 0) ? TargetAdd : TargetSub;
        int AbsDelta = std::abs(TotalDelta);

        BuildMI(MBB, It, It->getDebugLoc(), TII->get(NewOpc), TargetReg)
            .addReg(TargetReg)
            .addImm(AbsDelta);
      }

      for (MachineInstr *MI : InstsToRemove) {
        MI->eraseFromParent();
      }

      Changed = true;
      It = NextIt;
    }
  }

  return Changed;
}

} // namespace

static RegisterPass<IncDecToAddSubPass>
    X("inc-dec-to-add-sub", "Replace INC/DEC sequences with single ADD/SUB",
      false, false);