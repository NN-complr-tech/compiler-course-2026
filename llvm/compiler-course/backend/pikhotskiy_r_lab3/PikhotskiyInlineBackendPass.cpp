#include "X86.h"
#include "llvm/ADT/DenseMap.h"
#include "llvm/CodeGen/MachineFunctionPass.h"
#include "llvm/CodeGen/MachineInstr.h"
#include "llvm/CodeGen/MachineModuleInfo.h"
#include "llvm/CodeGen/MachineRegisterInfo.h"
#include "llvm/IR/Function.h"
#include "llvm/Pass.h"

using namespace llvm;

namespace {

constexpr unsigned MaxInlineInstructions = 15;
constexpr unsigned MaxRecursionDepth = 3;
constexpr unsigned MaxInliningIterations = 64;

const Function *getDirectCallee(const MachineInstr &MI) {
  for (const MachineOperand &MO : MI.operands()) {
    if (!MO.isGlobal())
      continue;
    return dyn_cast<Function>(MO.getGlobal());
  }
  return nullptr;
}

unsigned countInlineableInstructions(const MachineFunction &MF) {
  if (MF.empty())
    return 0;

  unsigned Count = 0;
  const MachineBasicBlock &MBB = MF.front();
  for (const MachineInstr &MI : MBB) {
    if (MI.isDebugInstr() || MI.isCFIInstruction())
      continue;
    if (MI.isTerminator())
      continue;
    ++Count;
  }
  return Count;
}

bool isInlineableCallee(const MachineFunction &MF) {
  if (MF.empty())
    return false;
  if (std::next(MF.begin()) != MF.end())
    return false;
  return countInlineableInstructions(MF) <= MaxInlineInstructions;
}

MachineInstr *cloneWithRegMap(MachineFunction &CallerMF,
                              const MachineInstr &SourceMI,
                              const MachineRegisterInfo &CalleeMRI,
                              DenseMap<Register, Register> &VRegMap) {
  MachineInstr *NewMI = CallerMF.CloneMachineInstr(&SourceMI);
  MachineRegisterInfo &CallerMRI = CallerMF.getRegInfo();

  for (MachineOperand &MO : NewMI->operands()) {
    if (!MO.isReg())
      continue;

    Register Reg = MO.getReg();
    if (!Reg.isVirtual())
      continue;

    Register &Mapped = VRegMap[Reg];
    if (!Mapped) {
      const TargetRegisterClass *RC = CalleeMRI.getRegClass(Reg);
      Mapped = CallerMRI.createVirtualRegister(RC);
    }
    MO.setReg(Mapped);
  }

  return NewMI;
}

bool inlineCall(MachineFunction &CallerMF, MachineInstr &CallMI,
                const MachineFunction &CalleeMF) {
  MachineBasicBlock *CallerMBB = CallMI.getParent();
  if (!CallerMBB)
    return false;

  DenseMap<Register, Register> VRegMap;
  const MachineRegisterInfo &CalleeMRI = CalleeMF.getRegInfo();

  MachineBasicBlock::iterator InsertIt = CallMI.getIterator();
  const MachineBasicBlock &CalleeEntry = CalleeMF.front();
  for (const MachineInstr &MI : CalleeEntry) {
    if (MI.isDebugInstr() || MI.isCFIInstruction())
      continue;
    if (MI.isTerminator())
      continue;

    MachineInstr *Cloned = cloneWithRegMap(CallerMF, MI, CalleeMRI, VRegMap);
    CallerMBB->insert(InsertIt, Cloned);
  }

  CallMI.eraseFromParent();
  return true;
}

class PikhotskiyInlineBackendPass : public MachineFunctionPass {
public:
  static char ID;

  PikhotskiyInlineBackendPass() : MachineFunctionPass(ID) {}

  bool runOnMachineFunction(MachineFunction &MF) override {
    MachineModuleInfo &MMI =
        getAnalysis<MachineModuleInfoWrapperPass>().getMMI();

    bool Changed = false;
    unsigned SelfRecInlineDepth = 0;

    for (unsigned Iter = 0; Iter < MaxInliningIterations; ++Iter) {
      bool IterChanged = false;
      bool InlinedSelfRecursiveCall = false;

      for (MachineBasicBlock &MBB : MF) {
        for (auto It = MBB.begin(); It != MBB.end();) {
          MachineInstr &MI = *It++;
          if (!MI.isCall())
            continue;

          const Function *CalleeF = getDirectCallee(MI);
          if (!CalleeF)
            continue;

          MachineFunction *CalleeMF = MMI.getMachineFunction(*CalleeF);
          if (!CalleeMF || !isInlineableCallee(*CalleeMF))
            continue;

          bool IsSelfRecursive = CalleeF == &MF.getFunction();
          if (IsSelfRecursive && SelfRecInlineDepth >= MaxRecursionDepth)
            continue;

          if (!inlineCall(MF, MI, *CalleeMF))
            continue;

          Changed = true;
          IterChanged = true;
          if (IsSelfRecursive)
            InlinedSelfRecursiveCall = true;
        }
      }

      if (InlinedSelfRecursiveCall)
        ++SelfRecInlineDepth;

      if (!IterChanged)
        break;
    }

    return Changed;
  }

  void getAnalysisUsage(AnalysisUsage &AU) const override {
    AU.addRequired<MachineModuleInfoWrapperPass>();
    MachineFunctionPass::getAnalysisUsage(AU);
  }
};

char PikhotskiyInlineBackendPass::ID = 0;

} // namespace

static RegisterPass<PikhotskiyInlineBackendPass>
    X("pikhotskiy-inline-backend",
      "Inline small direct calls in backend (limit=15, recursion depth=3)",
      false, false);
