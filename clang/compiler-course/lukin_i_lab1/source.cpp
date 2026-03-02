#include "clang/AST/ASTConsumer.h"
#include "clang/AST/RecursiveASTVisitor.h"
#include "clang/Frontend/CompilerInstance.h"
#include "clang/Frontend/FrontendPluginRegistry.h"
#include "llvm/Support/raw_ostream.h"

namespace {

struct Statistic//здесь будет храниться статистика по переменным (их количество)
{
  int global_obj;
  int static_vars;
  int local_vars;
  int params;

  Statistic() : global_obj(0), static_vars(0), local_vars(0), params(0) {}
};

class StatisticVisitor final : public clang::RecursiveASTVisitor<StatisticVisitor> {
public:
  explicit StatisticVisitor(clang::ASTContext *context) : m_context(context), stat(Statistic()) {}

  //вызывается, когда доходим до пар-в ф-ии
  bool VisitParmVarDecl(clang::ParmVarDecl *D) {
    stat.params++;
    return true;
  }

  //вызывается, когда доходим до обьявления переменных
  bool VisitVarDecl(clang::VarDecl *D){
    //т.к. пар-ры ф-ии также явл-ся переменными, проверяем, не они ли это. Иначе посчитаем дважды
    if(clang::isa<clang::ParmVarDecl>(D)){
      return true;
    }

    //теперь по порядку проверяем
    //1.Если static глобавльно или в функции, если static в классе
    //2.Если обьявление на уровне файла или namespace
    //3.Если локальная переменная
    if(D->getStorageClass() == clang::SC_Static || D->isStaticDataMember()){
      stat.static_vars++;
    }
    else if(D->isFileVarDecl()){
      stat.global_obj++;
    }
    else if(D->isLocalVarDecl()){
      stat.local_vars++;
    }

    return true;
  }

  Statistic get_statistic()
  {
    return stat;
  }

private:
  clang::ASTContext *m_context;
  Statistic stat;
};

class StatisticConsumer final : public clang::ASTConsumer {
public:
  explicit StatisticConsumer(clang::ASTContext *context) : m_visitor(context) {}

  void HandleTranslationUnit(clang::ASTContext &context) override {
    m_visitor.TraverseDecl(context.getTranslationUnitDecl());

    Statistic stat = m_visitor.get_statistic();
    llvm::outs() << "\nStatistics \n";
    llvm::outs() << "Global objects: " << stat.global_obj << "\n";
    llvm::outs() << "Local variables: " << stat.local_vars << "\n";
    llvm::outs() << "Static variables: " << stat.static_vars << "\n";
    llvm::outs() << "Params: " << stat.params << "\n";
  }

private:
  StatisticVisitor m_visitor;
};

class StatisticAction final : public clang::PluginASTAction {
public:
  std::unique_ptr<clang::ASTConsumer>
  CreateASTConsumer(clang::CompilerInstance &ci, llvm::StringRef) override {
    return std::make_unique<StatisticConsumer>(&ci.getASTContext());
  }

  bool ParseArgs(const clang::CompilerInstance &ci,
                 const std::vector<std::string> &args) override {
    return true;
  }
};
} // namespace

static clang::FrontendPluginRegistry::Add<StatisticAction>
    X("VariableStatisticsPlugin", "Plugin to get variables statistics");
