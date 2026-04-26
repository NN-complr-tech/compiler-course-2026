#include "X86.h"
#include "X86InstrInfo.h"
#include "X86Subtarget.h"
#include "llvm/CodeGen/MachineFunctionPass.h"
#include "llvm/CodeGen/MachineInstrBuilder.h"

using namespace llvm;

namespace {
class CheckNullPtrPass : public MachineFunctionPass {
public:
  static char ID;
  CheckNullPtrPass() : MachineFunctionPass(ID) {}
  bool runOnMachineFunction(MachineFunction &MF) override;
};

char CheckNullPtrPass::ID = 0;

bool CheckNullPtrPass::runOnMachineFunction(MachineFunction &func) {
  bool Modified = false;
  const TargetInstrInfo *TII = func.getSubtarget().getInstrInfo();

  const X86Subtarget &STI = func.getSubtarget<X86Subtarget>();
  if (!STI.is64Bit())
    return false;

  for (auto &MBB : func) {

    for (auto MI = MBB.begin(); MI != MBB.end(); ++MI) {
      if (!MI->mayLoad() && !MI->mayStore())
        continue;
      auto &Desc = MI->getDesc();
      int MemOpIdx = X86II::getMemoryOperandNo(Desc.TSFlags);
      if (MemOpIdx < 0)
        continue;

      MemOpIdx += X86II::getOperandBias(Desc);
      auto &BaseOp = MI->getOperand(MemOpIdx + X86::AddrBaseReg);

      if (!BaseOp.isReg() || !BaseOp.getReg().isValid())
        continue;
      Register BaseReg = BaseOp.getReg();
      if (BaseReg == X86::RSP || BaseReg == X86::RBP)
        continue;

      DebugLoc DL = MI->getDebugLoc();
      BuildMI(MBB, MI, DL, TII->get(X86::MOV64rr), X86::RDI).addReg(BaseReg);
      BuildMI(MBB, MI, DL, TII->get(X86::CALL64pcrel32))
          .addExternalSymbol("checkPointer")
          .addReg(X86::RDI, RegState::Implicit)
          .addReg(X86::RSP, RegState::Implicit);
      Modified = true;
    }
  }
  return Modified;
}
} // namespace

static RegisterPass<CheckNullPtrPass>
    X("check-null-ptr-x86", "Checking for a null pointer before dereference",
      false, false);