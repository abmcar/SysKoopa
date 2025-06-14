// riscv_codegen.h
#pragma once

#include <cassert>
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
  int getNextId();
  std::string idToAddr(int id);

  // 对象到id的映射
  std::unordered_map<const koopa_raw_binary_t *, int> raw_bin_id_map;
  std::unordered_map<const koopa_raw_value_t *, int> raw_val_id_map;

  // id到地址的映射
  std::unordered_map<int, std::string> id_addr_map;

  // 可用的寄存器集合
  std::set<std::string> regs = {"t0", "t1", "t2", "t3", "t4", "t5", "t6"};

  // id计数器
  int id_counter = 0;
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
