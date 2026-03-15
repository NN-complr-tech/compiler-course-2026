#include "clang/AST/ASTConsumer.h"
#include "clang/AST/Expr.h"
#include "clang/AST/ExprCXX.h"
#include "clang/AST/RecursiveASTVisitor.h"
#include "clang/Frontend/CompilerInstance.h"
#include "clang/Frontend/FrontendPluginRegistry.h"
#include "clang/Rewrite/Core/Rewriter.h"
#include "llvm/ADT/DenseMap.h"
#include "llvm/ADT/DenseSet.h"
#include "llvm/Support/raw_ostream.h"

namespace {

struct MutationState {
  bool SelfMutated = false;
  bool PointeeMutated = false;
};

class ConstQualifierVisitor
    : public clang::RecursiveASTVisitor<ConstQualifierVisitor> {
public:
  ConstQualifierVisitor(clang::ASTContext &Context, clang::Rewriter &Rewriter)
      : Context(Context), Rewriter(Rewriter) {}

  bool VisitVarDecl(clang::VarDecl *Decl) {
    if (!isSupportedDecl(Decl)) {
      return true;
    }

    if (canAddConst(Decl->getType())) {
      Candidates.insert(Decl);
    }
    return true;
  }

  bool VisitBinaryOperator(clang::BinaryOperator *Op) {
    if (Op->isAssignmentOp()) {
      markMutation(Op->getLHS());
    }
    return true;
  }

  bool VisitUnaryOperator(clang::UnaryOperator *Op) {
    if (Op->isIncrementDecrementOp()) {
      markMutation(Op->getSubExpr());
    }
    return true;
  }

  bool VisitCallExpr(clang::CallExpr *Call) {
    const auto *Callee = Call->getDirectCallee();
    if (!Callee) {
      return true;
    }

    const unsigned ArgsCount = Call->getNumArgs();
    const unsigned ParamsCount = Callee->getNumParams();
    const unsigned Count = ArgsCount < ParamsCount ? ArgsCount : ParamsCount;

    for (unsigned I = 0; I < Count; ++I) {
      analyzeArgument(Call->getArg(I), Callee->getParamDecl(I)->getType());
    }
    return true;
  }

  bool VisitCXXMemberCallExpr(clang::CXXMemberCallExpr *Call) {
    const auto *Method = Call->getMethodDecl();
    if (!Method || Method->isConst()) {
      return true;
    }

    clang::Expr *Object = ignoreWrappers(Call->getImplicitObjectArgument());
    if (Object && Object->getType()->isPointerType()) {
      markPointeeMutation(Object);
    } else {
      markMutation(Object);
    }
    return true;
  }

  void applyRewrites() {
    for (const auto *Decl : Candidates) {
      const MutationState State = States.lookup(Decl);
      std::string Replacement = buildReplacementType(Decl, State);
      if (Replacement.empty()) {
        continue;
      }

      clang::TypeSourceInfo *TypeInfo = Decl->getTypeSourceInfo();
      if (!TypeInfo) {
        continue;
      }

      const clang::SourceRange Range = TypeInfo->getTypeLoc().getSourceRange();
      if (Range.isInvalid()) {
        continue;
      }

      Rewriter.ReplaceText(Range, Replacement);
    }
  }

private:
  clang::ASTContext &Context;
  clang::Rewriter &Rewriter;
  llvm::DenseSet<const clang::VarDecl *> Candidates;
  llvm::DenseMap<const clang::VarDecl *, MutationState> States;

  bool isSupportedDecl(const clang::VarDecl *Decl) const {
    if (!Decl) {
      return false;
    }

    const auto *Function =
        llvm::dyn_cast<clang::FunctionDecl>(Decl->getDeclContext());
    if (!Function || !Function->hasBody()) {
      return false;
    }

    if (!llvm::isa<clang::ParmVarDecl>(Decl) && !Decl->isLocalVarDecl()) {
      return false;
    }

    const clang::SourceManager &SM = Context.getSourceManager();
    if (!SM.isWrittenInMainFile(SM.getSpellingLoc(Decl->getLocation()))) {
      return false;
    }

    // The task is specifically about `T&` (lvalue references) and `T*`.
    return Decl->getType()->isLValueReferenceType() ||
           Decl->getType()->isPointerType();
  }

  static bool canAddConst(clang::QualType Type) {
    if (Type->isLValueReferenceType()) {
      if (Type.getNonReferenceType()->isFunctionType()) {
        return false;
      }
      return !Type.getNonReferenceType().isConstQualified();
    }

    if (Type->isPointerType()) {
      if (Type->getPointeeType()->isFunctionType()) {
        return false;
      }
      return !Type.isLocalConstQualified() ||
             !Type->getPointeeType().isConstQualified();
    }

    return false;
  }

  MutationState &getState(const clang::VarDecl *Decl) { return States[Decl]; }

  void markState(const clang::VarDecl *Decl, bool MarkSelf, bool MarkPointee) {
    if (!Decl || !Candidates.count(Decl)) {
      return;
    }

    MutationState &State = getState(Decl);
    if (Decl->getType()->isReferenceType()) {
      State.PointeeMutated |= MarkSelf || MarkPointee;
      return;
    }

    State.SelfMutated |= MarkSelf;
    State.PointeeMutated |= MarkPointee;
  }

  void markDirectMutation(const clang::VarDecl *Decl) {
    markState(Decl, true, false);
  }

  void markPointeeMutation(clang::Expr *Expr) {
    Expr = ignoreWrappers(Expr);
    if (!Expr) {
      return;
    }

    if (auto *Ref = llvm::dyn_cast<clang::DeclRefExpr>(Expr)) {
      if (const auto *Decl = llvm::dyn_cast<clang::VarDecl>(Ref->getDecl())) {
        markState(Decl, false, true);
      }
      return;
    }

    if (auto *Op = llvm::dyn_cast<clang::UnaryOperator>(Expr)) {
      if (Op->getOpcode() == clang::UO_Deref) {
        markPointeeMutation(Op->getSubExpr());
      } else if (Op->getOpcode() == clang::UO_AddrOf) {
        markAddressEscaped(Op->getSubExpr());
      } else {
        markPointeeMutation(Op->getSubExpr());
      }
      return;
    }

    if (auto *Member = llvm::dyn_cast<clang::MemberExpr>(Expr)) {
      if (Member->isArrow()) {
        markPointeeMutation(Member->getBase());
      } else {
        markMutation(Member->getBase());
      }
      return;
    }

    if (auto *Subscript = llvm::dyn_cast<clang::ArraySubscriptExpr>(Expr)) {
      markPointeeMutation(Subscript->getBase());
      return;
    }
  }

  void markAddressEscaped(clang::Expr *Expr) {
    Expr = ignoreWrappers(Expr);
    if (!Expr) {
      return;
    }

    if (auto *Ref = llvm::dyn_cast<clang::DeclRefExpr>(Expr)) {
      if (const auto *Decl = llvm::dyn_cast<clang::VarDecl>(Ref->getDecl())) {
        markState(Decl, true, true);
      }
      return;
    }

    if (auto *Op = llvm::dyn_cast<clang::UnaryOperator>(Expr)) {
      if (Op->getOpcode() == clang::UO_Deref) {
        markPointeeMutation(Op->getSubExpr());
      } else {
        markAddressEscaped(Op->getSubExpr());
      }
      return;
    }

    if (auto *Member = llvm::dyn_cast<clang::MemberExpr>(Expr)) {
      if (Member->isArrow()) {
        markPointeeMutation(Member->getBase());
      } else {
        markMutation(Member->getBase());
      }
      return;
    }

    if (auto *Subscript = llvm::dyn_cast<clang::ArraySubscriptExpr>(Expr)) {
      markPointeeMutation(Subscript->getBase());
    }
  }

  void markMutation(clang::Expr *Expr) {
    Expr = ignoreWrappers(Expr);
    if (!Expr) {
      return;
    }

    if (auto *Ref = llvm::dyn_cast<clang::DeclRefExpr>(Expr)) {
      if (const auto *Decl = llvm::dyn_cast<clang::VarDecl>(Ref->getDecl())) {
        markDirectMutation(Decl);
      }
      return;
    }

    if (auto *Op = llvm::dyn_cast<clang::UnaryOperator>(Expr)) {
      if (Op->getOpcode() == clang::UO_Deref) {
        markPointeeMutation(Op->getSubExpr());
      } else if (Op->getOpcode() == clang::UO_AddrOf) {
        markAddressEscaped(Op->getSubExpr());
      } else {
        markMutation(Op->getSubExpr());
      }
      return;
    }

    if (auto *Member = llvm::dyn_cast<clang::MemberExpr>(Expr)) {
      if (Member->isArrow()) {
        markPointeeMutation(Member->getBase());
      } else {
        markMutation(Member->getBase());
      }
      return;
    }

    if (auto *Subscript = llvm::dyn_cast<clang::ArraySubscriptExpr>(Expr)) {
      markPointeeMutation(Subscript->getBase());
      return;
    }
  }

  void analyzeArgument(clang::Expr *Arg, clang::QualType ParamType) {
    Arg = ignoreWrappers(Arg);
    if (!Arg) {
      return;
    }

    if (ParamType->isReferenceType()) {
      if (ParamType.getNonReferenceType().isConstQualified()) {
        return;
      }

      if (ParamType.getNonReferenceType()->isPointerType()) {
        markPointerArgument(Arg, true);
      } else {
        markMutation(Arg);
      }
      return;
    }

    if (ParamType->isPointerType() &&
        !ParamType->getPointeeType().isConstQualified()) {
      markPointerArgument(Arg, false);
    }
  }

  void markPointerArgument(clang::Expr *Arg, bool CanRebindPointer) {
    Arg = ignoreWrappers(Arg);
    if (!Arg) {
      return;
    }

    if (auto *Ref = llvm::dyn_cast<clang::DeclRefExpr>(Arg)) {
      if (const auto *Decl = llvm::dyn_cast<clang::VarDecl>(Ref->getDecl())) {
        markState(Decl, CanRebindPointer, true);
      }
      return;
    }

    if (auto *Op = llvm::dyn_cast<clang::UnaryOperator>(Arg)) {
      if (Op->getOpcode() == clang::UO_AddrOf) {
        markAddressEscaped(Op->getSubExpr());
      } else if (Op->getOpcode() == clang::UO_Deref) {
        markPointeeMutation(Op->getSubExpr());
      }
      return;
    }

    if (auto *Subscript = llvm::dyn_cast<clang::ArraySubscriptExpr>(Arg)) {
      markPointeeMutation(Subscript->getBase());
    }
  }

  clang::Expr *ignoreWrappers(clang::Expr *Expr) const {
    if (!Expr) {
      return nullptr;
    }
    return Expr->IgnoreParenImpCasts();
  }

  std::string buildReplacementType(const clang::VarDecl *Decl,
                                   const MutationState &State) const {
    clang::QualType Type = Decl->getType();
    clang::PrintingPolicy Policy(Context.getLangOpts());

    if (Type->isLValueReferenceType()) {
      if (Type.getNonReferenceType().isConstQualified() ||
          Type.getNonReferenceType()->isFunctionType() ||
          State.PointeeMutated) {
        return {};
      }

      const clang::QualType ReferencedType =
          Context.getConstType(Type.getNonReferenceType());
      const clang::QualType NewType =
          Context.getLValueReferenceType(ReferencedType);
      return NewType.getAsString(Policy);
    }

    if (!Type->isPointerType()) {
      return {};
    }

    if (Type->getPointeeType()->isFunctionType()) {
      return {};
    }

    if (State.SelfMutated || State.PointeeMutated) {
      return {};
    }

    clang::QualType NewType =
        Context.getPointerType(Context.getConstType(Type->getPointeeType()));
    clang::Qualifiers Quals = NewType.getLocalQualifiers();
    Quals.addConst();
    NewType = Context.getQualifiedType(NewType.getUnqualifiedType(), Quals);

    return NewType.getAsString(Policy);
  }
};

class ConstQualifierConsumer : public clang::ASTConsumer {
public:
  explicit ConstQualifierConsumer(clang::CompilerInstance &CI)
      : Rewriter(CI.getSourceManager(), CI.getLangOpts()) {}

  void HandleTranslationUnit(clang::ASTContext &Context) override {
    ConstQualifierVisitor Visitor(Context, Rewriter);
    Visitor.TraverseDecl(Context.getTranslationUnitDecl());
    Visitor.applyRewrites();

    const clang::FileID MainFile = Context.getSourceManager().getMainFileID();
    if (const auto *Buffer = Rewriter.getRewriteBufferFor(MainFile)) {
      llvm::outs() << std::string(Buffer->begin(), Buffer->end());
      return;
    }

    bool Invalid = false;
    llvm::StringRef Source =
        Context.getSourceManager().getBufferData(MainFile, &Invalid);
    if (!Invalid) {
      llvm::outs() << Source;
    }
  }

private:
  clang::Rewriter Rewriter;
};

class ConstQualifierAction : public clang::PluginASTAction {
public:
  std::unique_ptr<clang::ASTConsumer>
  CreateASTConsumer(clang::CompilerInstance &CI, llvm::StringRef) override {
    return std::make_unique<ConstQualifierConsumer>(CI);
  }

  bool ParseArgs(const clang::CompilerInstance &,
                 const std::vector<std::string> &) override {
    return true;
  }
};

} // namespace

static clang::FrontendPluginRegistry::Add<ConstQualifierAction>
    X("kondakov_v_lab1_plugin",
      "Adds const to references and pointers that are not modified");
