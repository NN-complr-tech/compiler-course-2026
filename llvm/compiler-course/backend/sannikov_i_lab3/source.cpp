#include "X86.h"
#include "X86InstrInfo.h"
#include "X86Subtarget.h"
#include "llvm/CodeGen/MachineBasicBlock.h"
#include "llvm/CodeGen/MachineFunction.h"
#include "llvm/CodeGen/MachineFunctionPass.h"
#include "llvm/CodeGen/MachineInstrBuilder.h"

using namespace llvm;

namespace {

class NullCheckPass : public MachineFunctionPass {
public:
  static char ID;
  NullCheckPass() : MachineFunctionPass(ID) {}
  bool runOnMachineFunction(MachineFunction &MF) override;
};

char NullCheckPass::ID = 0;

bool NullCheckPass::runOnMachineFunction(MachineFunction &MF) {
  const X86InstrInfo *TII = MF.getSubtarget<X86Subtarget>().getInstrInfo();
  bool chng = false;

  for (MachineBasicBlock &MBB : MF) {
    for (auto It = MBB.begin(); It != MBB.end(); ++It) {
      MachineInstr &MI = *It;

      if (!MI.mayLoad() && !MI.mayStore())
        continue;
      Register Bsreg;
      const MCInstrDesc &Desc = MI.getDesc();
      int MemOp = X86II::getMemoryOperandNo(Desc.TSFlags);
      if (MemOp != -1) {
        MemOp += X86II::getOperandBias(Desc);
        const MachineOperand &Base = MI.getOperand(MemOp + X86::AddrBaseReg);
        if (Base.isReg() && Base.getReg().isValid()) {
          Register reg = Base.getReg();
          if (reg != X86::RSP && reg != X86::RBP)
            Bsreg = reg;
        }
      }

      if (!Bsreg)
        continue;

      DebugLoc DL = MI.getDebugLoc();
      BuildMI(MBB, It, DL, TII->get(X86::TEST64rr)).addReg(Bsreg).addReg(Bsreg);
      BuildMI(MBB, It, DL, TII->get(X86::JCC_1)).addImm(2).addImm(X86::COND_NE);
      BuildMI(MBB, It, DL, TII->get(X86::TRAP));

      chng = true;
    }
  }

  return chng;
}

} // namespace

static RegisterPass<NullCheckPass>
    RegPass("null-ptr-safety",
            "Insert inline null checks before pointer dereference", false,
            false);