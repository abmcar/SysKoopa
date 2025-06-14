// riscv_codegen.h
#pragma once

#include <cassert>
#include <iostream>
#include <set>
#include <sstream>
#include <unordered_map>

#include "koopa.h"

class AddrManager {
public:
  std::string getAddr(const koopa_raw_binary_t &binary);
  std::string getAddr(const koopa_raw_value_t &value);
  void freeReg(const std::string &reg);

private:
  std::string getReg();
  std::unordered_map<const koopa_raw_binary_t *, std::string> raw_bin_addr_map;
  std::unordered_map<const koopa_raw_value_t *, std::string> raw_val_addr_map;
  std::set<std::string> used_regs = {"t0", "t1", "t2", "t3", "t4", "t5", "t6"};
};

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
  void Visit(const koopa_raw_binary_t &);
  void cmd_li(const koopa_raw_value_t &value, const std::string &res_addr);
  int32_t get_value(const koopa_raw_value_t);
  AddrManager addr_manager;
};
