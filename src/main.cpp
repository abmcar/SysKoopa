#include <cassert>
#include <cstdio>
#include <iostream>
#include <memory>
#include <sstream>
#include <string>

#include "ast.h"
#include "riscv_codegen.h"
#include "util.h"

using namespace std;

extern FILE *yyin;
extern int yyparse(unique_ptr<BaseAST> &ast);
extern int yydebug;

int main(int argc, const char *argv[]) {
  // compiler 模式 输入文件 -o 输出文件
  assert(argc == 5 or argc == 6);
  auto mode = argv[1];
  auto input = argv[2];
  auto output = argv[4];
  if (argc == 6 and string(argv[5]) == "-debug") {
    yydebug = 1;
  }

  yyin = fopen(input, "r");
  assert(yyin);

  unique_ptr<BaseAST> ast;
  auto ret = yyparse(ast);
  assert(!ret);


  stringstream oss;
  oss << *ast;
  string irs = oss.str();
  cout << irs << endl;
  if (string(mode) == "-koopa") {
    write_file(output, irs);
  } else if (string(mode) == "-riscv") {
    CodeGen *codegen = new CodeGen(irs);
    string riscv_str = codegen->gererate();
    write_file(output, riscv_str);
    cout << riscv_str << endl;
  } else {
    cerr << "Error arguments" << endl;
    return 1;
  }
  cout << "success compile!" << endl;
  return 0;
}
