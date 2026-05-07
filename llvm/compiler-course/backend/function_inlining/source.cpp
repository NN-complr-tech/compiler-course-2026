#include "X86.h"
#include "llvm/ADT/DenseMap.h"
#include "llvm/ADT/SCCIterator.h"
#include "llvm/ADT/SmallVector.h"
#include "llvm/Analysis/CallGraph.h"
#include "llvm/CodeGen/MachineFunction.h"
#include "llvm/CodeGen/MachineModuleInfo.h"
#include "llvm/CodeGen/MachineRegisterInfo.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/Module.h"
#include "llvm/Pass.h"

using namespace llvm;

namespace {

constexpr unsigned MaxInlineInstructions = 15;
constexpr unsigned MaxRecursionDepth = 3;

class FunctionInliningPass : public ModulePass {
  using RecursiveGroupMap = DenseMap<const Function *, unsigned>;

public:
  static char ID;

  FunctionInliningPass() : ModulePass(ID) {}

  void getAnalysisUsage(AnalysisUsage &AU) const override {
    AU.addRequired<MachineModuleInfoWrapperPass>();
    ModulePass::getAnalysisUsage(AU);
  }

  bool runOnModule(Module &M) override {
    RecursiveDepths.clear();

    const RecursiveGroupMap RecursiveGroups = collectRecursiveGroups(M);
    MachineModuleInfo &MMI = getAnalysis<MachineModuleInfoWrapperPass>().getMMI();

    bool Changed = false;
    bool LocalChange = false;
    do {
      LocalChange = false;

      for (Function &F : M) {
        if (F.isDeclaration())
          continue;

        if (RecursiveGroups.find(&F) != RecursiveGroups.end())
          continue;

        MachineFunction *MF = MMI.getMachineFunction(F);
        if (MF == nullptr)
          continue;

        for (MachineBasicBlock &MBB : *MF) {
          for (auto It = MBB.begin(); It != MBB.end();) {
            MachineInstr &MI = *It++;
            Function *Callee = getCalledFunction(MI);
            if (Callee == nullptr || Callee->isDeclaration())
              continue;

            MachineFunction *CalleeMF = MMI.getMachineFunction(*Callee);
            if (CalleeMF == nullptr)
              continue;

            if (!canInline(MI, *CalleeMF, RecursiveGroups))
              continue;

            inlineCall(*MF, MBB, MI, *CalleeMF, RecursiveGroups);
            Changed = true;
            LocalChange = true;
          }
        }
      }
    } while (LocalChange);

    return Changed;
  }

private:
  DenseMap<const MachineInstr *, unsigned> RecursiveDepths;

  static RecursiveGroupMap collectRecursiveGroups(Module &M) {
    RecursiveGroupMap Groups;
    CallGraph CG(M);
    unsigned NextGroupId = 0;

    for (scc_iterator<CallGraph *> It = scc_begin(&CG); !It.isAtEnd(); ++It) {
      const std::vector<CallGraphNode *> &SCC = *It;
      SmallVector<const Function *, 4> Functions;

      for (CallGraphNode *Node : SCC) {
        if (Function *F = Node->getFunction())
          Functions.push_back(F);
      }

      if (Functions.empty())
        continue;

      bool IsRecursive = Functions.size() > 1;
      if (!IsRecursive)
        IsRecursive = hasSelfCall(*Functions.front());

      if (!IsRecursive)
        continue;

      for (const Function *F : Functions)
        Groups[F] = NextGroupId;

      ++NextGroupId;
    }

    return Groups;
  }

  static bool hasSelfCall(const Function &F) {
    for (const BasicBlock &BB : F) {
      for (const Instruction &I : BB) {
        const auto *Call = dyn_cast<CallBase>(&I);
        if (Call != nullptr && Call->getCalledFunction() == &F)
          return true;
      }
    }

    return false;
  }

  static Function *getCalledFunction(MachineInstr &MI) {
    if (!MI.isCall())
      return nullptr;

    for (MachineOperand &Operand : MI.operands()) {
      if (!Operand.isGlobal())
        continue;

      return dyn_cast<Function>(Operand.getGlobal());
    }

    return nullptr;
  }

  static bool hasSingleBlock(const MachineFunction &MF) {
    auto It = MF.begin();
    if (It == MF.end())
      return false;

    ++It;
    return It == MF.end();
  }

  static unsigned countInstructions(const MachineFunction &MF) {
    unsigned Count = 0;

    for (const MachineBasicBlock &MBB : MF) {
      for (const MachineInstr &MI : MBB) {
        if (MI.isDebugInstr() || MI.isMetaInstruction())
          continue;

        ++Count;
      }
    }

    return Count;
  }

  bool canInline(MachineInstr &Call, const MachineFunction &CalleeMF,
                 const RecursiveGroupMap &RecursiveGroups) const {
    if (!hasSingleBlock(CalleeMF))
      return false;

    if (countInstructions(CalleeMF) > MaxInlineInstructions)
      return false;

    const Function *Callee = &CalleeMF.getFunction();
    auto GroupIt = RecursiveGroups.find(Callee);
    if (GroupIt == RecursiveGroups.end())
      return true;

    return RecursiveDepths.lookup(&Call) < MaxRecursionDepth;
  }

  static bool sameRecursiveGroup(const Function *Left, const Function *Right,
                                 const RecursiveGroupMap &RecursiveGroups) {
    if (Left == nullptr || Right == nullptr)
      return false;

    auto LeftIt = RecursiveGroups.find(Left);
    auto RightIt = RecursiveGroups.find(Right);
    if (LeftIt == RecursiveGroups.end() || RightIt == RecursiveGroups.end())
      return false;

    return LeftIt->second == RightIt->second;
  }

  static void remapVirtualRegisters(
      MachineInstr &MI, MachineRegisterInfo &CallerMRI,
      const MachineRegisterInfo &CalleeMRI,
      DenseMap<Register, Register> &RegisterMap) {
    for (MachineOperand &Operand : MI.operands()) {
      if (!Operand.isReg())
        continue;

      Register Reg = Operand.getReg();
      if (!Reg.isVirtual())
        continue;

      auto Mapping = RegisterMap.find(Reg);
      if (Mapping == RegisterMap.end()) {
        const TargetRegisterClass *RegClass = CalleeMRI.getRegClass(Reg);
        Register NewReg = CallerMRI.createVirtualRegister(RegClass);
        Mapping = RegisterMap.insert({Reg, NewReg}).first;
      }

      Operand.setReg(Mapping->second);
    }
  }

  void inlineCall(MachineFunction &CallerMF, MachineBasicBlock &CallerMBB,
                  MachineInstr &Call, const MachineFunction &CalleeMF,
                  const RecursiveGroupMap &RecursiveGroups) {
    const unsigned CurrentDepth = RecursiveDepths.lookup(&Call);
    const Function *Callee = &CalleeMF.getFunction();

    MachineRegisterInfo &CallerMRI = CallerMF.getRegInfo();
    const MachineRegisterInfo &CalleeMRI = CalleeMF.getRegInfo();
    DenseMap<Register, Register> RegisterMap;

    for (const MachineInstr &Original : CalleeMF.front()) {
      if (Original.isDebugInstr() || Original.isMetaInstruction() ||
          Original.isReturn()) {
        continue;
      }

      MachineInstr *Cloned = CallerMF.CloneMachineInstr(&Original);
      remapVirtualRegisters(*Cloned, CallerMRI, CalleeMRI, RegisterMap);
      CallerMBB.insert(Call, Cloned);

      Function *NestedCallee = getCalledFunction(*Cloned);
      if (sameRecursiveGroup(Callee, NestedCallee, RecursiveGroups))
        RecursiveDepths[Cloned] = CurrentDepth + 1;
    }

    RecursiveDepths.erase(&Call);
    Call.eraseFromParent();
  }
};

char FunctionInliningPass::ID = 0;

} // namespace

static RegisterPass<FunctionInliningPass>
    X("function-inlining-backend", "Function inlining backend pass", false,
      false);