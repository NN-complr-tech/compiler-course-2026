#include "clang/AST/ASTConsumer.h"
#include "clang/AST/ASTContext.h"
#include "clang/AST/Decl.h"
#include "clang/AST/Expr.h"
#include "clang/AST/RecursiveASTVisitor.h"
#include "clang/Frontend/CompilerInstance.h"
#include "clang/Frontend/FrontendPluginRegistry.h"
#include "llvm/Support/raw_ostream.h"
#include <queue>
#include <unordered_set>

namespace {

using FunctionSet = std::unordered_set<const clang::FunctionDecl *>;

class ThrowAnalyzer {
public:
  explicit ThrowAnalyzer(clang::ASTContext &ctx) : m_context(ctx) {}

  bool canThrow(const clang::Stmt *stmt, const FunctionSet &noexceptFns) {
    if (!stmt)
      return false;

    std::queue<const clang::Stmt *> worklist;
    worklist.push(stmt);

    while (!worklist.empty()) {
      const auto *current = worklist.front();
      worklist.pop();

      if (llvm::isa<clang::CXXThrowExpr>(current)) {
        return true;
      }

      if (llvm::isa<clang::CXXTryStmt>(current)) {
        return true;
      }

      if (const auto *call = llvm::dyn_cast<clang::CallExpr>(current)) {
        if (const auto *callee = call->getDirectCallee()) {
          if (!isCalleeSafe(callee, noexceptFns)) {
            return true;
          }
        }
      }

      if (const auto *construct =
              llvm::dyn_cast<clang::CXXConstructExpr>(current)) {
        if (const auto *ctor = construct->getConstructor()) {
          if (!isCalleeSafe(ctor, noexceptFns)) {
            return true;
          }
        }
      }

      if (llvm::isa<clang::CXXNewExpr>(current)) {
        return true;
      }

      for (const auto *child : current->children()) {
        if (child)
          worklist.push(child);
      }
    }

    return false;
  }

private:
  bool isCalleeSafe(const clang::FunctionDecl *fn,
                    const FunctionSet &noexceptFns) const {
    if (!fn)
      return true;

    if (const auto *fpt = fn->getType()->getAs<clang::FunctionProtoType>()) {
      if (fpt->isNothrow()) {
        return true;
      }
    }

    return noexceptFns.find(fn) != noexceptFns.end();
  }

  clang::ASTContext &m_context;
};

class FunctionCollector : public clang::RecursiveASTVisitor<FunctionCollector> {
public:
  explicit FunctionCollector(std::vector<clang::FunctionDecl *> &functions)
      : m_functions(functions) {}

  bool VisitFunctionDecl(clang::FunctionDecl *func) {
    if (func && func->hasBody() && !func->isImplicit() && !func->isDeleted()) {
      m_functions.push_back(func);
    }
    return true;
  }

private:
  std::vector<clang::FunctionDecl *> &m_functions;
};

class NoexceptAdder : public clang::ASTConsumer {
public:
  explicit NoexceptAdder(clang::ASTContext &ctx)
      : m_context(ctx), m_analyzer(ctx), m_collector(m_functions) {
    llvm::errs() << "NoexceptAdder constructor called\n";
  }

  void HandleTranslationUnit(clang::ASTContext &ctx) override {
    llvm::errs() << "=== HandleTranslationUnit START ===\n";
    
    m_collector.TraverseDecl(ctx.getTranslationUnitDecl());
    llvm::errs() << "Collected " << m_functions.size() << " functions\n";

    FunctionSet noexceptFunctions;
    for (const auto *func : m_functions) {
      if (hasNoexceptSpec(func)) {
        noexceptFunctions.insert(func);
        llvm::errs() << "Already noexcept: " << func->getNameAsString() << "\n";
      }
    }

    bool changed = true;
    int iteration = 0;
    while (changed) {
      changed = false;
      iteration++;
      llvm::errs() << "Iteration " << iteration << "\n";
      
      for (auto *func : m_functions) {
        if (noexceptFunctions.count(func)) {
          continue;
        }

        if (!func->getBody()) {
          continue;
        }

        if (!m_analyzer.canThrow(func->getBody(), noexceptFunctions)) {
          addNoexceptSpecifier(func);
          noexceptFunctions.insert(func);
          changed = true;
          llvm::errs() << "Added noexcept to: " << func->getNameAsString() << "\n";
        }
      }
    }

    llvm::errs() << "=== Dumping AST ===\n";
    ctx.getTranslationUnitDecl()->dump(llvm::errs());
    llvm::errs() << "=== HandleTranslationUnit END ===\n";
    llvm::errs().flush();
  }

private:
  bool hasNoexceptSpec(const clang::FunctionDecl *func) const {
    if (const auto *fpt = func->getType()->getAs<clang::FunctionProtoType>()) {
      return fpt->isNothrow();
    }
    return false;
  }

  void addNoexceptSpecifier(clang::FunctionDecl *func) {
    const auto *fpt = func->getType()->getAs<clang::FunctionProtoType>();
    if (!fpt)
      return;

    clang::FunctionProtoType::ExtProtoInfo epi = fpt->getExtProtoInfo();
    epi.ExceptionSpec.Type = clang::EST_BasicNoexcept;

    clang::QualType newType = m_context.getFunctionType(
        fpt->getReturnType(), fpt->getParamTypes(), epi);

    func->setType(newType);

    if (func->getDescribedFunctionTemplate()) {
    }
  }

  clang::ASTContext &m_context;
  ThrowAnalyzer m_analyzer;
  std::vector<clang::FunctionDecl *> m_functions;
  FunctionCollector m_collector;
};

class NoexceptPluginAction : public clang::PluginASTAction {
public:
  std::unique_ptr<clang::ASTConsumer>
  CreateASTConsumer(clang::CompilerInstance &ci, llvm::StringRef) override {
    llvm::errs() << "=== PluginAction: CreateASTConsumer ===\n";
    return std::make_unique<NoexceptAdder>(ci.getASTContext());
  }

  bool ParseArgs(const clang::CompilerInstance &,
                 const std::vector<std::string> &) override {
    llvm::errs() << "=== PluginAction: ParseArgs ===\n";
    return true;
  }

  ActionType getActionType() override { 
    llvm::errs() << "=== PluginAction: getActionType ===\n";
    return ReplaceAction;
  }
};

} // namespace

static clang::FrontendPluginRegistry::Add<NoexceptPluginAction>
    X("noexcept_analyzer",
      "Add noexcept specifier to functions that don't throw");
