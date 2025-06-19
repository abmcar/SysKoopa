#include "util.h"
#include "ast.h"

void write_file(std::string file_name, std::string file_content) {
  std::ofstream os;                  // 创建一个文件输出流对象
  os.open(file_name, std::ios::out); // 将对象与文件关联
  os << file_content;                // 将输入的内容放入txt文件中
  os.close();
  return;
}

std::string get_koopa_exp_reg(ExpAST *exp) {
  if (exp->is_number()) {
    return std::to_string(exp->get_number());
  } else {
    return "%" + std::to_string(exp->get_reg());
  }
}

std::string get_koopa_logical_exp_reg(ExpAST *exp) {
  if (exp->is_number()) {
    if (exp->get_number() != 0) {
      return "1";
    } else {
      return "0";
    }
  } else {
    return "%" + std::to_string(exp->get_reg());
  }
}

void pushup_exp_reg(ExpAST *exp, ExpAST *parent) {
  if (exp->is_number()) {
    parent->number = exp->get_number();
    parent->kind = ExpAST::Kind::NUMBER;
  } else {
    parent->set_reg(exp->get_reg());
  }
}

void modify_sp(int offset, std::stringstream &oss) {
  if (offset > 2047 || offset < -2048) {
    oss << "  li t0, " << offset << "\n";
    oss << "  addi sp, sp, t0\n";
  } else {
    oss << "  addi sp, sp, " << offset << "\n";
  }
}