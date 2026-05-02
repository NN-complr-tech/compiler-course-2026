#include "X86.h"
#include "X86InstrInfo.h"
#include "X86Subtarget.h"
#include "llvm/CodeGen/MachineFunctionPass.h"
#include "llvm/CodeGen/MachineInstrBuilder.h"

using namespace llvm;

namespace {

struct OpcodeMapping {
  unsigned IncOpc;
  unsigned DecOpc;
  unsigned AddOpc;
  unsigned SubOpc;
};

static const OpcodeMapping OpcTable[] = {
    {X86::INC8r, X86::DEC8r, X86::ADD8ri, X86::SUB8ri},
    {X86::INC16r, X86::DEC16r, X86::ADD16ri, X86::SUB16ri},
    {X86::INC32r, X86::DEC32r, X86::ADD32ri, X86::SUB32ri},
    {X86::INC64r, X86::DEC64r, X86::ADD64ri32, X86::SUB64ri32},
};

static const OpcodeMapping *findMapping(unsigned Opc) {
  for (const OpcodeMapping &M : OpcTable)
    if (M.IncOpc == Opc || M.DecOpc == Opc)
      return &M;
  return nullptr;
}

static bool isIncOpcode(unsigned Opc) {
  for (const OpcodeMapping &M : OpcTable)
    if (M.IncOpc == Opc)
      return true;
  return false;
}

class IncDecToAddSubPass : public MachineFunctionPass {
public:
  static char ID;
  IncDecToAddSubPass() : MachineFunctionPass(ID) {}
  bool runOnMachineFunction(MachineFunction &MF) override;

private:
  bool processBlock(MachineBasicBlock &MBB, const X86InstrInfo *TII);
};

char IncDecToAddSubPass::ID = 0;

bool IncDecToAddSubPass::processBlock(MachineBasicBlock &MBB,
                                      const X86InstrInfo *TII) {
  bool Changed = false;

  MachineBasicBlock::iterator Pos = MBB.begin();
  while (Pos != MBB.end()) {
    MachineInstr &Lead = *Pos;
    const OpcodeMapping *Map = findMapping(Lead.getOpcode());

    if (!Map) {
      ++Pos;
      continue;
    }

    unsigned RunOpc = Lead.getOpcode();
    Register RunReg = Lead.getOperand(0).getReg();

    SmallVector<MachineInstr *, 8> Run;
    MachineBasicBlock::iterator Scan = Pos;
    while (Scan != MBB.end() && Scan->getOpcode() == RunOpc &&
           Scan->getOperand(0).getReg() == RunReg) {
      Run.push_back(&*Scan);
      ++Scan;
    }

    unsigned NewOpc = isIncOpcode(RunOpc) ? Map->AddOpc : Map->SubOpc;
    Register SrcReg = Lead.getOperand(1).getReg();
    bool SrcKill = Lead.getOperand(1).isKill();
    DebugLoc DL = Lead.getDebugLoc();

    BuildMI(MBB, Lead, DL, TII->get(NewOpc), RunReg)
        .addReg(SrcReg, SrcKill ? RegState::Kill : 0)
        .addImm(static_cast<int64_t>(Run.size()));

    for (MachineInstr *MI : Run)
      MI->eraseFromParent();

    Changed = true;
    Pos = Scan;
  }

  return Changed;
}

bool IncDecToAddSubPass::runOnMachineFunction(MachineFunction &MF) {
  const X86Subtarget &STI = MF.getSubtarget<X86Subtarget>();
  const X86InstrInfo *TII = STI.getInstrInfo();

  bool Changed = false;
  for (MachineBasicBlock &MBB : MF)
    Changed |= processBlock(MBB, TII);

  return Changed;
}

} // namespace

static RegisterPass<IncDecToAddSubPass>
    X("inc-dec-to-add-sub-x86", "Replace INC/DEC with ADD/SUB", false, false);