#include "clang/AST/ASTConsumer.h"
#include "clang/AST/Decl.h"
#include "clang/AST/RecursiveASTVisitor.h"
#include "clang/Frontend/CompilerInstance.h"
#include "clang/Frontend/FrontendPluginRegistry.h"
#include "llvm/Support/raw_ostream.h"

namespace {
class VarCounterVisitor : public clang::RecursiveASTVisitor<VarCounterVisitor> {
public:
  explicit VarCounterVisitor(clang::ASTContext *context)
      : globals(0), locals(0), statics(0), params(0) {}

  bool VisitVarDecl(clang::VarDecl *var) {
    if (var != var->getCanonicalDecl())
      return true;

    if (clang::isa<clang::ParmVarDecl>(var))
      return true;

    if (var->getStorageClass() == clang::SC_Static || 
        var->isStaticLocal() || 
        var->isStaticDataMember()) {
      statics++;
    }
    else if (var->isLocalVarDecl()) {
      locals++;
    }
    else if (var->isFileVarDecl()) {
      globals++;
    }

    return true;
  }

  bool VisitParmVarDecl(clang::ParmVarDecl *parm) {
    params++;
    return true;
  }

  void printResults() {
    llvm::errs() << "Global variables: " << globals << "\n";
    llvm::errs() << "Local variables: " << locals << "\n";
    llvm::errs() << "Static variables: " << statics << "\n";
    llvm::errs() << "Function parameters: " << params << "\n";
    llvm::errs() << "Total: " << globals + locals + statics + params << "\n";
  }

private:
  size_t globals;
  size_t locals;
  size_t statics;
  size_t params;
};

class VarCounterConsumer : public clang::ASTConsumer {
public:
  explicit VarCounterConsumer(clang::ASTContext *context) : visitor(context) {}

  void HandleTranslationUnit(clang::ASTContext &context) override {
    visitor.TraverseDecl(context.getTranslationUnitDecl());
    visitor.printResults();
  }

private:
  VarCounterVisitor visitor;
};

class VarCounterAction : public clang::PluginASTAction {
public:
  std::unique_ptr<clang::ASTConsumer>
  CreateASTConsumer(clang::CompilerInstance &ci, llvm::StringRef) override {
    return std::make_unique<VarCounterConsumer>(&ci.getASTContext());
  }

  bool ParseArgs(const clang::CompilerInstance &ci,
                 const std::vector<std::string> &args) override {
    return true;
  }

  PluginASTAction::ActionType getActionType() override {
    return AddAfterMainAction;
  }
};
} // namespace

static clang::FrontendPluginRegistry::Add<VarCounterAction>
    X("var_counter_plugin", "counts different types of variables");