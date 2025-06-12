// riscv_codegen.h
#pragma once

#include <cassert>
#include <sstream>

#include "koopa.h"

class CodeGen {
public:
  std::string gererate();
  CodeGen(const std::string &koopa_ir);
  ~CodeGen();

private:
  std::stringstream oss;
  koopa_raw_program_builder_t builder;
  koopa_raw_program_t raw;
  void Visit(const koopa_raw_program_t &);
  void Visit(const koopa_raw_slice_t &);
  void Visit(const koopa_raw_function_t &);
  void Visit(const koopa_raw_basic_block_t &);
  void Visit(const koopa_raw_value_t &);
  void Visit(const koopa_raw_return_t &);
  void Visit(const koopa_raw_integer_t &);
  int32_t get_value(const koopa_raw_value_t);

  std::stringstream oss_;
};