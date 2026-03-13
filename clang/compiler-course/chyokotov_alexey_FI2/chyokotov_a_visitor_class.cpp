#include "clang/AST/ASTConsumer.h"
#include "clang/AST/RecursiveASTVisitor.h"
#include "clang/Frontend/CompilerInstance.h"
#include "clang/Frontend/FrontendPluginRegistry.h"
#include "llvm/Support/raw_ostream.h"

#include <string>
#include <map>

namespace {
class ChyokotovAVisitor final : public clang::RecursiveASTVisitor<ChyokotovAVisitor> {
public:
  explicit ChyokotovAVisitor(clang::ASTContext *context) : m_context(context) {}

  bool VisitBinaryOperator(clang::BinaryOperator *binop) {
    if(!binop || !binop->isAssignmentOp()){
      return true;
    }
    clang::Expr* rhs = binop->getRHS()->IgnoreImplicit();
    if(isAllocation(rhs)){
      clang::VarDecl *var = getVarDecl(binop->getLHS()->IgnoreImplicit());
      if(var){
        vars[var] = 1;
      }
    }
    return true;
  }

  bool VisitCallExpr(clang::CallExpr *call) {
    clang::FunctionDecl *func = call->getDirectCallee();
    if (!func) {
      return true;
    }

    llvm::StringRef funcName = func->getName();
    if (funcName == "free" || funcName == "fclose") {
      if (call->getNumArgs() > 0) {
        clang::Expr *arg = call->getArg(0)->IgnoreParenCasts();
        clang::VarDecl *var = getVarDecl(arg);
        if (var) {
          vars[var] = 2;
        }
      }
    }
    return true;
  }

  bool VisitVarDecl(clang::VarDecl *var) {
    clang::Expr *exp = var->getInit();
    if(exp && isAllocation(exp)){
      vars[var] = 1;
    }
    return true;
  }

  bool VisitReturnStmt(clang::ReturnStmt *ret) {
    if (!ret) return true;
    
    clang::Expr *retValue = ret->getRetValue();
    if (!retValue) return true;
    
    retValue = retValue->IgnoreParenCasts();
    
    if (auto *declRef = clang::dyn_cast<clang::DeclRefExpr>(retValue)) {
        if (auto *var = clang::dyn_cast<clang::VarDecl>(declRef->getDecl())) {
            if (vars.count(var) && vars[var] == 1) {
                vars[var] = 1;
            }
        }
    }
    return true;
  }

  bool VisitCXXDeleteExpr(clang::CXXDeleteExpr *exp) {
    clang::Expr *arg = exp->getArgument()->IgnoreParenCasts();
    clang::VarDecl *var = getVarDecl(arg);
    if (var) {
      vars[var] = 2;
    }
    return true;
  }

  void outputs(){
    clang::DiagnosticsEngine &DE = m_context->getDiagnostics();
    for(auto &i : vars){
      if(i.second == 1){
        unsigned diagID = DE.getCustomDiagID(
                clang::DiagnosticsEngine::Warning,
                "memory leak: '%0'"
            );
            
        DE.Report(i.first->getLocation(), diagID) << i.first->getNameAsString();
      }
    }
  }

private:
  bool isAllocation(clang::Expr* exp){
    clang::Expr *castexp = exp->IgnoreParenCasts();

    if (clang::CallExpr *call = clang::dyn_cast<clang::CallExpr>(castexp)){
      if(clang::FunctionDecl *func = call->getDirectCallee()){
        llvm::StringRef funcName = func->getName();
        return (funcName == "malloc" || funcName == "fopen");
      }
    }
    return clang::isa<clang::CXXNewExpr>(castexp);
  }

  clang::VarDecl *getVarDecl(clang::Expr *exp) {
    if (!exp) {
      return nullptr;
    }
    clang::Expr *castexp = exp->IgnoreParenCasts();
    if (clang::DeclRefExpr *ref = clang::dyn_cast<clang::DeclRefExpr>(castexp)) {
      return clang::dyn_cast<clang::VarDecl>(ref->getDecl());
    }
    return nullptr;
  }

  clang::ASTContext *m_context;
  std::map<clang::VarDecl*, int> vars;
};

class ExampleConsumer final : public clang::ASTConsumer {
public:
  explicit ExampleConsumer(clang::ASTContext *context) : m_visitor(context) {}

  void HandleTranslationUnit(clang::ASTContext &context) override {
    m_visitor.TraverseDecl(context.getTranslationUnitDecl());
    m_visitor.outputs();
  }

private:
  ChyokotovAVisitor m_visitor;
};

class ExampleAction final : public clang::PluginASTAction {
public:
  std::unique_ptr<clang::ASTConsumer>
  CreateASTConsumer(clang::CompilerInstance &ci, llvm::StringRef) override {
    return std::make_unique<ExampleConsumer>(&ci.getASTContext());
  }

  bool ParseArgs(const clang::CompilerInstance &ci,
                 const std::vector<std::string> &args) override {
    return true;
  }


  ActionType getActionType() override {
    return AddAfterMainAction;
  }


};
} // namespace

static clang::FrontendPluginRegistry::Add<ExampleAction>
    X("chyokotov_a_analyzer_plugin", "Description_plugin");

/*
clang++ -cc1 -load "/home/acheck/labworks/compiler-course-2026/build/lib/chyokotov_alexey_FI2_ClangAST.so" -plugin chyokotov_a_analyzer_plugin "/home/acheck/labworks/compiler-course-2026/clang/test/compiler-course/chyokotov_alexeyFI2/test.cpp"
 rebuild: ninja -C build/

 // RUN: split-file %s %t
// RUN: %clang_cc1 -load %llvmshlibdir/chyokotov_alexey_FI2_ClangAST%pluginext -plugin chyokotov_a_analyzer_plugin -fsyntax-only -verify %t/with_warnings.cpp

//--- with_warnings.cpp
*/