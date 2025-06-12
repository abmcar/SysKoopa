// riscv_codegen.cpp
#include "riscv_codegen.h"

CodeGen::CodeGen(const std::string &koopa_ir) {
  const char *str = koopa_ir.c_str();
  koopa_program_t program;
  assert(koopa_parse_from_string(str, &program) == KOOPA_EC_SUCCESS);
  builder = koopa_new_raw_program_builder();
  raw = koopa_build_raw_program(builder, program);
  koopa_delete_program(program);
}

CodeGen::~CodeGen() { koopa_delete_raw_program_builder(builder); }

std::string CodeGen::gererate() {
  Visit(raw);
  return oss_.str();
}

// 后面是 Visit 的类成员定义（略）...

// 访问 raw program
void CodeGen::Visit(const koopa_raw_program_t &program) {
  // 执行一些其他的必要操作
  oss << "  .text\n";

  // 访问所有全局变量
  Visit(program.values);
  // 访问所有函数
  Visit(program.funcs);
}

// 访问 raw slice
void CodeGen::Visit(const koopa_raw_slice_t &slice) {
  for (size_t i = 0; i < slice.len; ++i) {
    auto ptr = slice.buffer[i];
    // 根据 slice 的 kind 决定将 ptr 视作何种元素
    switch (slice.kind) {
    case KOOPA_RSIK_FUNCTION:
      // 访问函数
      Visit(reinterpret_cast<koopa_raw_function_t>(ptr));
      break;
    case KOOPA_RSIK_BASIC_BLOCK:
      // 访问基本块
      Visit(reinterpret_cast<koopa_raw_basic_block_t>(ptr));
      break;
    case KOOPA_RSIK_VALUE:
      // 访问指令
      Visit(reinterpret_cast<koopa_raw_value_t>(ptr));
      break;
    default:
      // 我们暂时不会遇到其他内容, 于是不对其做任何处理
      assert(false);
    }
  }
}

// 访问函数
void CodeGen::Visit(const koopa_raw_function_t &func) {
  // 执行一些其他的必要操作
  oss << "  .globl " << func->name << "\n";
  oss << func->name << ":\n";
  // 访问所有基本块
  Visit(func->bbs);
}

// 访问基本块
void CodeGen::Visit(const koopa_raw_basic_block_t &bb) {
  // 执行一些其他的必要操作
  // ...
  // 访问所有指令
  Visit(bb->insts);
}

// 访问指令
void CodeGen::Visit(const koopa_raw_value_t &value) {
  // 根据指令类型判断后续需要如何访问
  const auto &kind = value->kind;
  switch (kind.tag) {
  case KOOPA_RVT_RETURN:
    // 访问 return 指令
    Visit(kind.data.ret);
    break;
  case KOOPA_RVT_INTEGER:
    // 访问 integer 指令
    Visit(kind.data.integer);
    break;
  default:
    // 其他类型暂时遇不到
    assert(false);
  }
}

void CodeGen::Visit(const koopa_raw_return_t &ret) {
  oss << "  li a0, " << get_value(ret.value) << "\n";
  oss << "  ret\n";
}

int32_t CodeGen::get_value(const koopa_raw_value_t val) {
  return val->kind.data.integer.value;
}

void CodeGen::Visit(const koopa_raw_integer_t &i32) {}
