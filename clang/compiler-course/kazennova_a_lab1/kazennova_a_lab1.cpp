#include "clang/AST/ASTConsumer.h"
#include "clang/AST/RecursiveASTVisitor.h"
#include "clang/Frontend/CompilerInstance.h"
#include "clang/Frontend/FrontendPluginRegistry.h"
#include "clang/Rewrite/Core/Rewriter.h"
#include "llvm/Support/raw_ostream.h"

using namespace clang;

namespace {
class CastVisitor final : public RecursiveASTVisitor<CastVisitor> {
public:
  explicit CastVisitor(ASTContext *context, Rewriter &rewriter) 
      : m_context(context), m_rewriter(rewriter) {}

  bool VisitCStyleCastExpr(CStyleCastExpr *cast) {
    QualType targetType = cast->getTypeAsWritten();
    
    Expr *subExpr = cast->getSubExpr();
    
    std::string subExprStr = getExprAsString(subExpr);
    
    std::string replacement = "static_cast<" + targetType.getAsString() + ">(" + subExprStr + ")";
    
    m_rewriter.ReplaceText(cast->getSourceRange(), replacement);
    
    return true;
  }

private:
  ASTContext *m_context;
  Rewriter &m_rewriter;

  std::string getExprAsString(Expr *expr) {
    SourceManager &sm = m_context->getSourceManager();
    SourceRange range = expr->getSourceRange();
    
    if (range.isInvalid()) return "<expr>";
    
    const char *begin = sm.getCharacterData(range.getBegin());
    const char *end = sm.getCharacterData(range.getEnd());
    
    return std::string(begin, end - begin + 1);
  }
};

class CastConsumer final : public ASTConsumer {
public:
  explicit CastConsumer(ASTContext *context, Rewriter &rewriter) 
      : m_visitor(context, rewriter) {}

  void HandleTranslationUnit(ASTContext &context) override {
    m_visitor.TraverseDecl(context.getTranslationUnitDecl());
  }

private:
  CastVisitor m_visitor;
};

class CastAction final : public PluginASTAction {
public:
  std::unique_ptr<ASTConsumer> CreateASTConsumer(CompilerInstance &ci, 
                                                  StringRef) override {
    m_rewriter.setSourceMgr(ci.getSourceManager(), ci.getLangOpts());
    return std::make_unique<CastConsumer>(&ci.getASTContext(), m_rewriter);
  }

  void EndSourceFileAction() override {
    m_rewriter.getEditBuffer(m_rewriter.getSourceMgr().getMainFileID())
              .write(llvm::outs());
  }

  bool ParseArgs(const CompilerInstance &ci,
                 const std::vector<std::string> &args) override {
    return true;
  }

private:
  Rewriter m_rewriter;
};

} // namespace

static FrontendPluginRegistry::Add<CastAction>
    X("kazennova_a_lab1_plugin", "Replace C-style casts with static_cast");