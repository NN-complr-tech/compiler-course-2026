#include "X86.h"
#include "X86InstrInfo.h"
#include "X86Subtarget.h"
#include "llvm/CodeGen/MachineFunctionPass.h"
#include "llvm/CodeGen/MachineInstrBuilder.h"

using namespace llvm;

namespace {
class ExamplePass : public MachineFunctionPass {
public:
  static char ID;
  ExamplePass() : MachineFunctionPass(ID) {}

  bool runOnMachineFunction(MachineFunction &MF) override;

private:
  bool processBasicBlock(MachineBasicBlock &MBB);
  bool replaceIncDecSeq(MachineBasicBlock::iterator &MI,
                        MachineBasicBlock &MBB);
  const X86InstrInfo *TII = nullptr;
};

char ExamplePass::ID = 0;

bool ExamplePass::runOnMachineFunction(MachineFunction &func) {
  TII = func.getSubtarget<X86Subtarget>().getInstrInfo();
  bool changed = false;
  for (MachineBasicBlock &MBB : func) {
    changed |= processBasicBlock(MBB);
  }

  return changed;
}

bool ExamplePass::replaceIncDecSeq(MachineBasicBlock::iterator &MI,
                                   MachineBasicBlock &MBB) {
  MachineInstr &FirstInstr = *MI;
  unsigned Opcode = FirstInstr.getOpcode();

  bool IsINC, IsDEC;

  // bool IsINC = (Opcode == X86::INC64r || Opcode == X86::INC32r ||
  //               Opcode == X86::INC16r || Opcode == X86::INC8r ||
  //               Opcode == X86::INC64m || Opcode == X86::INC32m ||
  //               Opcode == X86::INC16m || Opcode == X86::INC8m);

  // bool IsDEC = (Opcode == X86::DEC64r || Opcode == X86::DEC32r ||
  //               Opcode == X86::DEC16r || Opcode == X86::DEC8r ||
  //               Opcode == X86::DEC64m || Opcode == X86::DEC32m ||
  //               Opcode == X86::DEC16m || Opcode == X86::DEC8m);

  // if (!(IsINC || IsDEC)) return false;

  int64_t Delta = 0;
  MachineOperand *TargetOp = nullptr;
  bool IsMemOp = false;

  TargetOp = &FirstInstr.getOperand(0);
  IsMemOp = FirstInstr.mayLoadOrStore();

  std::vector<MachineBasicBlock::iterator> toRemove;
  MachineBasicBlock::iterator NextMI = MI;

  while (NextMI != MBB.end()) {
    MachineInstr &CurrInstr = *NextMI;
    unsigned CurrOpcode = CurrInstr.getOpcode();

    IsINC = (CurrOpcode == X86::INC64r || CurrOpcode == X86::INC32r ||
             CurrOpcode == X86::INC16r || CurrOpcode == X86::INC8r ||
             CurrOpcode == X86::INC64m || CurrOpcode == X86::INC32m ||
             CurrOpcode == X86::INC16m || CurrOpcode == X86::INC8m);

    IsDEC = (CurrOpcode == X86::DEC64r || CurrOpcode == X86::DEC32r ||
             CurrOpcode == X86::DEC16r || CurrOpcode == X86::DEC8r ||
             CurrOpcode == X86::DEC64m || CurrOpcode == X86::DEC32m ||
             CurrOpcode == X86::DEC16m || CurrOpcode == X86::DEC8m);

    if (!(IsINC || IsDEC))
      break;

    MachineOperand *CurrOp = nullptr;

    CurrOp = &CurrInstr.getOperand(0);

    if (!CurrOp->isIdenticalTo(*TargetOp))
      break;

    if (IsINC)
      Delta++;
    else if (IsDEC)
      Delta--;

    toRemove.push_back(NextMI);
    ++NextMI;
  }

  if (Delta == 0) {
    for (auto &It : toRemove)
      It->eraseFromParent();
    MI = NextMI;
    return true;
  }

  const DebugLoc &DL = FirstInstr.getDebugLoc();
  unsigned AddOpcode, SubOpcode;

  if (IsMemOp) {
    switch (Opcode) {
    case X86::INC8m:
    case X86::DEC8m:
      AddOpcode = X86::ADD8mi;
      SubOpcode = X86::SUB8mi;
      break;
    case X86::INC16m:
    case X86::DEC16m:
      AddOpcode = X86::ADD16mi;
      SubOpcode = X86::SUB16mi;
      break;
    case X86::INC32m:
    case X86::DEC32m:
      AddOpcode = X86::ADD32mi;
      SubOpcode = X86::SUB32mi;
      break;
    case X86::INC64m:
    case X86::DEC64m:
      AddOpcode = X86::ADD64mi32;
      SubOpcode = X86::SUB64mi32;
      break;
    default:
      return false;
    }
  } else {
    switch (Opcode) {
    case X86::INC8r:
    case X86::DEC8r:
      AddOpcode = X86::ADD8ri;
      SubOpcode = X86::SUB8ri;
      break;
    case X86::INC16r:
    case X86::DEC16r:
      AddOpcode = X86::ADD16ri;
      SubOpcode = X86::SUB16ri;
      break;
    case X86::INC32r:
    case X86::DEC32r:
      AddOpcode = X86::ADD32ri;
      SubOpcode = X86::SUB32ri;
      break;
    case X86::INC64r:
    case X86::DEC64r:
      AddOpcode = X86::ADD64ri32;
      SubOpcode = X86::SUB64ri32;
      break;
    default:
      return false;
    }
  }

  unsigned NewOpcode;
  if (Delta > 0)
    NewOpcode = AddOpcode;
  else {
    NewOpcode = SubOpcode;
    Delta = -Delta;
  }

  if (!toRemove.empty() && toRemove[0] == MI)
    toRemove.erase(toRemove.begin());

  for (auto &It : toRemove)
    It->eraseFromParent();

  BuildMI(MBB, MI, DL, TII->get(NewOpcode)).add(*TargetOp).addImm(Delta);

  MI->eraseFromParent();
  MI = NextMI;

  return true;
}

bool ExamplePass::processBasicBlock(MachineBasicBlock &MBB) {
  bool Changed = false;

  for (auto MI = MBB.begin(); MI != MBB.end();) {
    unsigned Opcode = MI->getOpcode();
    bool IsINC = (Opcode == X86::INC64r || Opcode == X86::INC32r ||
                  Opcode == X86::INC16r || Opcode == X86::INC8r ||
                  Opcode == X86::INC64m || Opcode == X86::INC32m ||
                  Opcode == X86::INC16m || Opcode == X86::INC8m);

    bool IsDEC = (Opcode == X86::DEC64r || Opcode == X86::DEC32r ||
                  Opcode == X86::DEC16r || Opcode == X86::DEC8r ||
                  Opcode == X86::DEC64m || Opcode == X86::DEC32m ||
                  Opcode == X86::DEC16m || Opcode == X86::DEC8m);

    if (IsINC || IsDEC) {
      if (replaceIncDecSeq(MI, MBB)) {
        Changed = true;
        continue;
      }
    }
    ++MI;
  }

  return Changed;
}

} // namespace

static RegisterPass<ExamplePass> X("incdec-to-addsub-x86", "description pass",
                                   false, false);
