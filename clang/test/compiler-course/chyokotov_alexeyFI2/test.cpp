// RUN: %clang_cc1 -load %llvmshlibdir/chyokotov_alexey_FI2_ClangAST%pluginext -add-plugin chyokotov_a_analyzer_plugin -fsyntax-only -verify %s

int main() {
    return 0;
}