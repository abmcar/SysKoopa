#include "util.h"

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
