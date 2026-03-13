#include "clang/AST/ASTConsumer.h"
#include "clang/AST/RecursiveASTVisitor.h"
#include "clang/AST/ExprCXX.h"
#include "clang/Frontend/CompilerInstance.h"
#include "clang/Frontend/FrontendPluginRegistry.h"
#include "clang/Basic/SourceManager.h"
#include "llvm/Support/raw_ostream.h"

using namespace clang;

namespace {

struct ResourceState {
  std::string name;
  std::string type;
  SourceLocation loc;
  bool freed = false;
};

class ExampleVisitor :
    public RecursiveASTVisitor<ExampleVisitor> {

public:
  explicit ExampleVisitor(ASTContext *context)
      : context(context) {}

  bool TraverseFunctionDecl(FunctionDecl *func) {

    if (!func->hasBody())
      return true;

    resources.clear();

    RecursiveASTVisitor::TraverseFunctionDecl(func);

    checkUnreleased();

    return true;
  }

  bool VisitDeclStmt(DeclStmt *stmt) {

    for (auto *d : stmt->decls()) {

      if (auto *var = dyn_cast<VarDecl>(d)) {

        if (!var->hasInit())
          continue;

        Expr *init = var->getInit();

        if (isa<CXXNewExpr>(init)) {

          resources.push_back({
              var->getNameAsString(),
              "new",
              var->getLocation(),
              false
          });
        }

        if (auto *call = dyn_cast<CallExpr>(init)) {

          if (auto *callee = call->getDirectCallee()) {

            std::string name = callee->getNameAsString();

            if (name == "malloc") {

              resources.push_back({
                  var->getNameAsString(),
                  "malloc",
                  var->getLocation(),
                  false
              });
            }

            if (name == "fopen") {

              resources.push_back({
                  var->getNameAsString(),
                  "fopen",
                  var->getLocation(),
                  false
              });
            }
          }
        }
      }
    }

    return true;
  }

  bool VisitCallExpr(CallExpr *call) {

    if (auto *callee = call->getDirectCallee()) {

      std::string name = callee->getNameAsString();

      if (name == "free" || name == "fclose") {

        if (call->getNumArgs() == 1) {

          if (auto *ref =
              dyn_cast<DeclRefExpr>(call->getArg(0))) {

            std::string var =
                ref->getDecl()->getNameAsString();

            markFreed(var);
          }
        }
      }
    }

    return true;
  }

  bool VisitCXXDeleteExpr(CXXDeleteExpr *expr) {

    Expr *arg = expr->getArgument();

    if (auto *ref = dyn_cast<DeclRefExpr>(arg)) {

      std::string var =
          ref->getDecl()->getNameAsString();

      markFreed(var);
    }

    return true;
  }

  bool VisitReturnStmt(ReturnStmt *) {

    for (auto &r : resources) {

      if (!r.freed) {

        unsigned line =
            context->getSourceManager()
                .getSpellingLineNumber(r.loc);

        llvm::errs()
            << "warning: variable '"
            << r.name
            << "' holds resource ("
            << r.type
            << ") that may leak on return (line "
            << line << ")\n";
      }
    }

    return true;
  }

private:

  ASTContext *context;

  std::vector<ResourceState> resources;

  void markFreed(const std::string &name) {

    for (auto &r : resources) {

      if (r.name == name)
        r.freed = true;
    }
  }

  void checkUnreleased() {

    auto &SM = context->getSourceManager();

    for (auto &r : resources) {

      if (!r.freed) {

        unsigned line =
            SM.getSpellingLineNumber(r.loc);

        llvm::errs()
            << "warning: resource for variable '"
            << r.name
            << "' (" << r.type
            << ") not released (line "
            << line << ")\n";
      }
    }
  }
};

class ExampleConsumer : public ASTConsumer {

public:

  explicit ExampleConsumer(ASTContext *context)
      : visitor(context) {}

  void HandleTranslationUnit(ASTContext &context) override {

    visitor.TraverseDecl(context.getTranslationUnitDecl());

  }

private:

  ExampleVisitor visitor;

};

class ExampleAction : public PluginASTAction {

public:

  std::unique_ptr<ASTConsumer>
  CreateASTConsumer(CompilerInstance &ci,
                    llvm::StringRef) override {

    return std::make_unique<ExampleConsumer>(
        &ci.getASTContext());
  }

  bool ParseArgs(const CompilerInstance &,
                 const std::vector<std::string> &) override {

    return true;
  }
};

}

static FrontendPluginRegistry::Add<ExampleAction>
X("example_plugin", "advanced resource analyzer");