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
  std::string getAddr(const koopa_raw_load_t &load);
  std::string getAddr(const koopa_raw_store_t &store);
  void freeReg(const std::string &reg);
  void freeId(const koopa_raw_value_t &value);
  void freeId(const koopa_raw_binary_t &binary);
  void freeId(const koopa_raw_load_t &load);
  void freeId(const koopa_raw_store_t &store);

private:
  std::string getReg();
  int getNextId();
  std::string idToAddr(int id);
  template <typename T>
  std::string getAddrImpl(const T &obj,
                          std::unordered_map<const T *, int> &map);

  // 对象到id的映射
  std::unordered_map<const koopa_raw_binary_t *, int> raw_bin_id_map;
  std::unordered_map<const koopa_raw_value_t *, int> raw_val_id_map;
  std::unordered_map<const koopa_raw_load_t *, int> raw_load_id_map;
  std::unordered_map<const koopa_raw_store_t *, int> raw_store_id_map;


  std::unordered_map<int, std::string> id_addr_map;
  std::unordered_map<std::string, int> addr_id_map;

  // 可用的寄存器集合
  std::set<std::string> regs = {"t0", "t1", "t2", "t3", "t4", "t5", "t6"};

  // id计数器
  int id_counter = 0;
};

class StackOffsetManager {
public:
  void setOffset(const koopa_raw_value_t &value);
  int getOffset(const koopa_raw_load_t &load);
  int getOffset(const koopa_raw_binary_t &binary);
  int getOffset(const std::string &alloc_name);
  void clear();
  int current_stack_offset = 0;
  int r = 0;
  int a = 0;
  int final_stack_size = 0;

private:
  int id_to_offset(int id);
  int getNextId();
  std::unordered_map<const koopa_raw_load_t *, int> load_id_map;
  std::unordered_map<const koopa_raw_binary_t *, int> binary_id_map;
  std::unordered_map<std::string, int> alloc_name_id_map;
  std::unordered_map<int, int> id_to_offset_map;

  
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
  void AllocateStack(const koopa_raw_function_t &func);
  void Visit(const koopa_raw_program_t &);
  void Visit(const koopa_raw_slice_t &);
  void Visit(const koopa_raw_function_t &);
  void Visit(const koopa_raw_basic_block_t &);
  void Visit(const koopa_raw_value_t &);
  void Visit(const koopa_raw_return_t &);
  void Visit(const koopa_raw_integer_t &);
  void Visit(const koopa_raw_binary_t &);
  void Visit(const koopa_raw_load_t &);
  void Visit(const koopa_raw_store_t &);
  void Visit(const koopa_raw_branch_t &);
  void Visit(const koopa_raw_jump_t &);
  void cmd_li(const koopa_raw_value_t &value, const std::string &res_addr);
  int32_t get_value(const koopa_raw_value_t);
  void print_num(int num);
  AddrManager addr_manager;
  StackOffsetManager stack_offset_manager = StackOffsetManager();
};
