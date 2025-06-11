#include "ast.h"

void CompUnitAST::Dump() const {
  std::cout << "CompUnitAST { ";
  func_def->Dump();
  std::cout << " }";
}

void FuncDefAST::Dump() const {
  std::cout << "FuncDefAST { ";
  func_type->Dump();
  std::cout << ", " << ident << ", ";
  block->Dump();
  std::cout << " }";
}

void FuncTypeAST::Dump() const {
  std::cout << "FuncTypeAst { " << func_type << " }";
}

void BlockAST::Dump() const {
  std::cout << "BlockAST { ";
  stmt->Dump();
  std::cout << " }";
}

void StmtAST::Dump() const { std::cout << "StmtAST { " << number << " }"; }

std::ostream &operator<<(std::ostream &os, const BaseAST &ast) {
  ast.print(os);
  return os;
};

void CompUnitAST::print(std::ostream &os) const { func_def->print(os); }

void FuncDefAST::print(std::ostream &os) const {
  os << "fun @" << this->ident << "(" << ")" << ": ";
  func_type->print(os);
  os << " {\n";
  block->print(os);
  os << "}";
}

void FuncTypeAST::print(std::ostream &os) const {
    if (func_type == "int") {
        os << "i32";
    }
}

void BlockAST::print(std::ostream &os) const {
    os << "%" << "entry" << ":\n";
    stmt->print(os);
}

void StmtAST::print(std::ostream &os) const {
    os << "ret " << number << "\n";
}