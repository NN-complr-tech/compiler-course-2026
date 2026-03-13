#include "clang/AST/ASTConsumer.h"
#include "clang/AST/ASTContext.h"
#include "clang/AST/RecursiveASTVisitor.h"
#include "clang/Frontend/CompilerInstance.h"
#include "clang/Frontend/FrontendActions.h"
#include "clang/Tooling/CommonOptionsParser.h"
#include "clang/Tooling/Tooling.h"
#include "llvm/Support/CommandLine.h"
#include "llvm/Support/raw_ostream.h"

using namespace clang;
using namespace clang::tooling;
using namespace llvm;

class MyVisitor : public RecursiveASTVisitor<MyVisitor> {
public:
  int globalCount = 0;
  int localCount = 0;
  int staticLocalCount = 0;
  int paramCount = 0;

  bool VisitVarDecl(VarDecl *VD) {
    if (VD != VD->getCanonicalDecl() &&
        !(VD->isThisDeclarationADefinition() && VD->hasGlobalStorage() &&
          !VD->isStaticLocal())) {
      return true;
    }

    if (!VD->isThisDeclarationADefinition())
      return true;

    if (VD->hasGlobalStorage() && !VD->isStaticLocal())
      globalCount++;
    else if (VD->isLocalVarDecl() && !VD->isStaticLocal())
      localCount++;
    else if (VD->isStaticLocal())
      staticLocalCount++;

    return true;
  }

  bool VisitParmVarDecl(ParmVarDecl *PD) {
    if (auto *FD = dyn_cast<FunctionDecl>(PD->getDeclContext()))
      if (!FD->isTemplated() || isa<CXXConstructorDecl>(FD))
        paramCount++;
    return true;
  }
};

class MyASTConsumer : public ASTConsumer {
public:
  void HandleTranslationUnit(ASTContext &Context) override {
    MyVisitor Visitor;
    Visitor.TraverseDecl(Context.getTranslationUnitDecl());

    outs() << "========== СТАТИСТИКА ПЕРЕМЕННЫХ ==========\n";
    outs() << "Глобальных переменных: " << Visitor.globalCount << "\n";
    outs() << "Локальных переменных: " << Visitor.localCount << "\n";
    outs() << "Статических локальных: " << Visitor.staticLocalCount << "\n";
    outs() << "Параметров функций: " << Visitor.paramCount << "\n";
    outs() << "===========================================\n";
  }
};

class MyAction : public ASTFrontendAction {
public:
  std::unique_ptr<ASTConsumer> CreateASTConsumer(CompilerInstance &CI,
                                                 StringRef File) override {
    return std::make_unique<MyASTConsumer>();
  }
};

static cl::OptionCategory MyToolCategory("my-tool options");

int main(int argc, const char **argv) {
  auto ExpectedParser = CommonOptionsParser::create(argc, argv, MyToolCategory);
  if (!ExpectedParser) {
    llvm::errs() << ExpectedParser.takeError();
    return 1;
  }
  CommonOptionsParser &OptionsParser = ExpectedParser.get();
  ClangTool Tool(OptionsParser.getCompilations(),
                 OptionsParser.getSourcePathList());
  return Tool.run(newFrontendActionFactory<MyAction>().get());
}
