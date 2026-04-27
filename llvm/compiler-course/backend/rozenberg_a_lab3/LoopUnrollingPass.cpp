#include "X86.h"
#include "X86InstrInfo.h"
#include "X86Subtarget.h"
#include "llvm/CodeGen/MachineFunctionPass.h"
#include "llvm/CodeGen/MachineInstrBuilder.h"
#include "llvm/CodeGen/MachineLoopInfo.h"
#include "llvm/CodeGen/MachineBasicBlock.h"
#include "llvm/CodeGen/MachineFunction.h"
#include "llvm/CodeGen/MachineRegisterInfo.h"
#include "llvm/Support/raw_ostream.h"

using namespace llvm;

// max iteration unroll count
static constexpr unsigned MaxUnrollCount = 5;

namespace {

class LoopUnrollingPass : public MachineFunctionPass {
public:
  static char ID;
  LoopUnrollingPass() : MachineFunctionPass(ID) {}

  void getAnalysisUsage(AnalysisUsage &AU) const override {
    AU.addRequired<MachineLoopInfoWrapperPass>();
    MachineFunctionPass::getAnalysisUsage(AU);
  }
  
  bool runOnMachineFunction(MachineFunction &MF) override {
    MachineLoopInfo &MLI = getAnalysis<MachineLoopInfoWrapperPass>().getLI();

    outs() << "[LoopUnrolling] " << MF.getName() << "\n";

    // collect all loops in post-order before changing CFG
    // (after eraseFromParent MLI is invalid)
    SmallVector<MachineLoop *, 16> Worklist;
    for (MachineLoop *Top : MLI)
      collectPostOrder(Top, Worklist);

    outs() << "  found loops: " << Worklist.size() << "\n";

    bool Changed = false;
    for (MachineLoop *L : Worklist)
      Changed |= unrollLoop(L, MF);

    return Changed;
  }

private:
  Register findInductionVar(MachineBasicBlock *Header) {
    for (MachineInstr &MI : *Header) {
      unsigned Opc = MI.getOpcode();
      if (Opc == X86::CMP32ri   || Opc == X86::CMP32ri8 ||
          Opc == X86::CMP64ri32 || Opc == X86::CMP64ri8)
        return MI.getOperand(0).getReg();
    }
    return Register();
  }

  int64_t getTripCount(MachineBasicBlock *Header) {
    int64_t Imm     = -1;
    int64_t JccCond = -1;
  
    for (MachineInstr &MI : *Header) {
      unsigned Opc = MI.getOpcode();
  
      if (Opc == X86::CMP32ri   || Opc == X86::CMP32ri8 ||
          Opc == X86::CMP64ri32 || Opc == X86::CMP64ri8) {
        if (MI.getNumOperands() >= 2 && MI.getOperand(1).isImm())
          Imm = MI.getOperand(1).getImm();
      }
  
      // JCC_1: operand[0]=target_bb, operand[1]=cond, operand[2]=eflags
      if (Opc == X86::JCC_1) {
        if (MI.getNumOperands() >= 2 && MI.getOperand(1).isImm())
          JccCond = MI.getOperand(1).getImm();
      }
    }

    if (Imm < 0 || JccCond < 0)
      return -1;
  
    // X86::CondCode:
    //   15 = COND_G  (JG,  signed >):  exit when i >  Imm → tripCount = Imm + 1
    //   13 = COND_GE (JGE, signed >=): exit when i >= Imm → tripCount = Imm
    switch (JccCond) {
    case 15: return Imm + 1; // JG
    case 13: return Imm;     // JGE
    default:
      outs() << "  [skip] unsupported JCC code: " << JccCond << "\n";
      return -1;
    }
  }

  SmallVector<MachineBasicBlock *, 16> collectLoopBlocks(MachineBasicBlock *Header, MachineBasicBlock *Exit) {
    SmallVector<MachineBasicBlock *, 16> Blocks;
    SmallPtrSet<MachineBasicBlock *, 16> Visited;
    SmallVector<MachineBasicBlock *, 16> Worklist = {Header};
  
    while (!Worklist.empty()) {
      MachineBasicBlock *MBB = Worklist.pop_back_val();
      if (!Visited.insert(MBB).second)
        continue;
      if (MBB == Exit)
        continue;
      Blocks.push_back(MBB);
      for (MachineBasicBlock *Succ : MBB->successors())
        Worklist.push_back(Succ);
    }
    return Blocks;
  }

  // returns true if instruction is loop condition (CMP) or INC induction var
  bool isLoopOverhead(const MachineInstr &MI, Register IndVar) {
    unsigned Opc = MI.getOpcode();
  
    if (Opc == X86::CMP32ri   || Opc == X86::CMP32ri8 ||
        Opc == X86::CMP64ri32 || Opc == X86::CMP64ri8)
      return true;
  
    if ((Opc == X86::INC32r || Opc == X86::INC64r) &&
        MI.getNumOperands() > 0 && MI.getOperand(0).isReg() &&
        MI.getOperand(0).getReg() == IndVar)
      return true;
  
    return false;
  }

  bool unrollLoop(MachineLoop *L, MachineFunction &MF) {
    MachineBasicBlock *Preheader = L->getLoopPreheader();
    MachineBasicBlock *Header    = L->getHeader();
    MachineBasicBlock *Latch     = L->getLoopLatch();
    MachineBasicBlock *Exit      = findLoopExitBlock(L);
  
    if (!Preheader) {
      outs() << "  [skip] no preheader\n";
      return false;
    }

    if (!Header) {
      outs() << "  [skip] no header\n";
      return false;
    }

    if (!Latch) {
      outs() << "  [skip] no latch\n";
      return false;
    }

    if (!Exit) {
      outs() << "  [skip] no exit\n";
      return false;
    }
  
    int64_t TripCount = getTripCount(Header);
    if (TripCount < 1) {
      outs() << "  [skip] invalid tripCount\n";
      return false;
    }
    if (TripCount > MaxUnrollCount) {
      outs() << "  [skip] tripCount=" << TripCount
            << " > MaxUnrollCount=" << MaxUnrollCount << "\n";
      return false;
    }
  
    Register IndVar = findInductionVar(Header);
    if (!IndVar.isValid()) {
      outs() << "  [skip] induction var invalid\n";
      return false;
    }
  
    outs() << "  [unroll] tripCount=" << TripCount
          << " IndVar=" << IndVar << "\n";
  
    const TargetInstrInfo *TII = MF.getSubtarget().getInstrInfo();
    MachineRegisterInfo  &MRI  = MF.getRegInfo();
  
    // collect body blocks before changing CFG
    SmallVector<MachineBasicBlock *, 16> BodyBlocks =
        collectLoopBlocks(Header, Exit);
  
    // create new block for unrolled body
    MachineBasicBlock *UnrollMBB = MF.CreateMachineBasicBlock();
    MF.insert(std::next(Preheader->getIterator()), UnrollMBB);
  
    // insert TripCount body copies
    for (int64_t Iter = 0; Iter < TripCount; ++Iter) {
      // iteration const in separate vreg
      Register IterReg = MRI.createVirtualRegister(MRI.getRegClass(IndVar));
      BuildMI(*UnrollMBB, UnrollMBB->end(), DebugLoc(),
              TII->get(X86::MOV32ri), IterReg)
          .addImm(Iter);
  
      for (MachineBasicBlock *MBB : BodyBlocks) {
        for (MachineInstr &MI : *MBB) {
          // skip terminators, debug and loop overhead-instructions
          if (MI.isTerminator() || MI.isDebugInstr())
            continue;
          if (isLoopOverhead(MI, IndVar))
            continue;
  
          MachineInstr *NewMI = MF.CloneMachineInstr(&MI);
          // change IndVar to IterReg in all operands
          for (MachineOperand &MO : NewMI->operands())
            if (MO.isReg() && MO.getReg() == IndVar)
              MO.setReg(IterReg);
          UnrollMBB->push_back(NewMI);
        }
      }
    }
 
    // building CFG
    TII->removeBranch(*Preheader);
    Preheader->removeSuccessor(Header);
    Preheader->addSuccessor(UnrollMBB);
    BuildMI(*Preheader, Preheader->end(), DebugLoc(), TII->get(X86::JMP_1))
        .addMBB(UnrollMBB);
  
    UnrollMBB->addSuccessor(Exit);
    Exit->replacePhiUsesWith(Latch, UnrollMBB);
    BuildMI(*UnrollMBB, UnrollMBB->end(), DebugLoc(), TII->get(X86::JMP_1))
        .addMBB(Exit);
  
    // delete all loop blocks
    for (MachineBasicBlock *MBB : BodyBlocks) {
      while (!MBB->succ_empty())
        MBB->removeSuccessor(MBB->succ_begin());
      while (!MBB->pred_empty())
        (*MBB->pred_begin())->removeSuccessor(MBB);
      MBB->eraseFromParent();
    }
  
    return true;
  }

  void collectPostOrder(MachineLoop *L, SmallVectorImpl<MachineLoop *> &Result) {
    for (MachineLoop *Sub : L->getSubLoops())
      collectPostOrder(Sub, Result);
    Result.push_back(L);
  }

  MachineBasicBlock* findLoopExitBlock(MachineLoop *L) {
    if (MachineBasicBlock *Exit = L->getExitBlock())
      return Exit;
    
    // Fallback: searching for Latch successors not included in loop body
    MachineBasicBlock *Latch = L->getLoopLatch();
    MachineBasicBlock *Header = L->getHeader();

    if (Header) {
      for (MachineBasicBlock *Succ : Header->successors()) {
        if (!L->contains(Succ))
          return Succ;
      }
    }
    
    if (Latch && Latch != Header) {
      for (MachineBasicBlock *Succ : Latch->successors()) {
        if (!L->contains(Succ))
          return Succ;
      }
    }
    return nullptr;
  }
};

char LoopUnrollingPass::ID = 0;


} // namespace

static RegisterPass<LoopUnrollingPass> X("loop-unrolling-x86", "Loop unrolling pass", false,
                                   false);
