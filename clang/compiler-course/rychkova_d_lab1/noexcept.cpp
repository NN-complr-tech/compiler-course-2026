#include "clang/AST/ASTConsumer.h"
#include "clang/AST/ASTContext.h"
#include "clang/Frontend/CompilerInstance.h"
#include "clang/Frontend/FrontendPluginRegistry.h"
#include "llvm/Support/raw_ostream.h"

namespace {

class DiagnosticConsumer : public clang::ASTConsumer {
public:
  void HandleTranslationUnit(clang::ASTContext &ctx) override {
    llvm::errs() << "PLUGIN IS WORKING! AST DUMP:\n";
    ctx.getTranslationUnitDecl()->dump(llvm::errs());
    llvm::errs() << "PLUGIN FINISHED!\n";
  }
};

class DiagnosticPlugin : public clang::PluginASTAction {
public:
  std::unique_ptr<clang::ASTConsumer>
  CreateASTConsumer(clang::CompilerInstance &ci, llvm::StringRef) override {
    llvm::errs() << "CreateASTConsumer called\n";
    return std::make_unique<DiagnosticConsumer>();
  }

  bool ParseArgs(const clang::CompilerInstance &,
                 const std::vector<std::string> &) override {
    llvm::errs() << "ParseArgs called\n";
    return true;
  }

  ActionType getActionType() override { 
    llvm::errs() << "getActionType called\n";
    return AddBeforeMainAction; 
  }
};

} // namespace

static clang::FrontendPluginRegistry::Add<DiagnosticPlugin>
    X("noexcept_analyzer", "Diagnostic plugin");
