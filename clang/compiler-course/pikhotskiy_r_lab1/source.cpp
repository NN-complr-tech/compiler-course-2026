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

    if (var->isFileVarDecl()) {
      if (var->getStorageClass() == clang::SC_Static)
        statics++;
      else
        globals++;
    } else if (var->isLocalVarDecl()) {
      if (var->isStaticLocal())
        statics++;
      else
        locals++;
    }

    return true;
  }

  bool VisitParmVarDecl(clang::ParmVarDecl *parm) {
    params++;
    return true;
  }

  void printResults() {
    llvm::outs() << "Global variables: " << globals << "\n";
    llvm::outs() << "Local variables: " << locals << "\n";
    llvm::outs() << "Static variables: " << statics << "\n";
    llvm::outs() << "Function parameters: " << params << "\n";
    llvm::outs() << "Total: " << globals + locals + statics + params << "\n";
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
};
} // namespace

static clang::FrontendPluginRegistry::Add<VarCounterAction>
    X("var_counter_plugin", "counts different types of variables");