#include "util.h"

// 函数声明略
// ...

void write_file(std::string file_name, std::string file_content) {
  std::ofstream os;                  // 创建一个文件输出流对象
  os.open(file_name, std::ios::out); // 将对象与文件关联
  os << file_content;                // 将输入的内容放入txt文件中
  os.close();
  return;
}

std::string convert_ir_riscv(std::string irs) {
  const char *str = irs.c_str();
  koopa_program_t program;
  koopa_error_code_t ret = koopa_parse_from_string(str, &program);
  assert(ret == KOOPA_EC_SUCCESS);
  koopa_raw_program_builder_t builder = koopa_new_raw_program_builder();
  koopa_raw_program_t raw = koopa_build_raw_program(builder, program);
  koopa_delete_program(program);

  std::stringstream oss;
  Visit(raw, oss);

  koopa_delete_raw_program_builder(builder);
  return oss.str();
}

// 访问 raw program
void Visit(const koopa_raw_program_t &program, std::stringstream &oss) {
  // 执行一些其他的必要操作
  oss << "  .text\n";

  // 访问所有全局变量
  Visit(program.values, oss);
  // 访问所有函数
  Visit(program.funcs, oss);
}

// 访问 raw slice
void Visit(const koopa_raw_slice_t &slice, std::stringstream &oss) {
  for (size_t i = 0; i < slice.len; ++i) {
    auto ptr = slice.buffer[i];
    // 根据 slice 的 kind 决定将 ptr 视作何种元素
    switch (slice.kind) {
    case KOOPA_RSIK_FUNCTION:
      // 访问函数
      Visit(reinterpret_cast<koopa_raw_function_t>(ptr), oss);
      break;
    case KOOPA_RSIK_BASIC_BLOCK:
      // 访问基本块
      Visit(reinterpret_cast<koopa_raw_basic_block_t>(ptr), oss);
      break;
    case KOOPA_RSIK_VALUE:
      // 访问指令
      Visit(reinterpret_cast<koopa_raw_value_t>(ptr), oss);
      break;
    default:
      // 我们暂时不会遇到其他内容, 于是不对其做任何处理
      assert(false);
    }
  }
}

// 访问函数
void Visit(const koopa_raw_function_t &func, std::stringstream &oss) {
  // 执行一些其他的必要操作
  oss << "  .globl " << func->name << "\n";
  oss << func->name << ":\n";
  // 访问所有基本块
  Visit(func->bbs, oss);
}

// 访问基本块
void Visit(const koopa_raw_basic_block_t &bb, std::stringstream &oss) {
  // 执行一些其他的必要操作
  // ...
  // 访问所有指令
  Visit(bb->insts, oss);
}

// 访问指令
void Visit(const koopa_raw_value_t &value, std::stringstream &oss) {
  // 根据指令类型判断后续需要如何访问
  const auto &kind = value->kind;
  switch (kind.tag) {
  case KOOPA_RVT_RETURN:
    // 访问 return 指令
    Visit(kind.data.ret, oss);
    break;
  case KOOPA_RVT_INTEGER:
    // 访问 integer 指令
    Visit(kind.data.integer, oss);
    break;
  default:
    // 其他类型暂时遇不到
    assert(false);
  }
}

void Visit(const koopa_raw_return_t &ret, std::stringstream &oss) {
  oss << "  li a0, " << get_value(ret.value) << "\n";
  oss << "  ret\n";
}

int32_t get_value(const koopa_raw_value_t val) {
  return val->kind.data.integer.value;
}

void Visit(const koopa_raw_integer_t &i32, std::stringstream &oss) {}
// 访问对应类型指令的函数定义略
// 视需求自行实现
// ...
