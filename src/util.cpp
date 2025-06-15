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
