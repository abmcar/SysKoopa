#pragma once

#include <sstream>
#include <vector>

#include "addr_manager.h"
#include "stack_offset_manager.h"
#include "koopa.h"

class CodeGen {
public:
  CodeGen(const std::string &koopa_ir);
  ~CodeGen();
  std::string gererate();

private:
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
  void Visit(const koopa_raw_call_t &);
  void Visit(const koopa_raw_global_alloc_t &, std::string var_name);
  void Visit(const koopa_raw_get_elem_ptr_t &);
  void cmd_li(const koopa_raw_value_t &value, std::string &res_addr);
  int32_t get_value(const koopa_raw_value_t);
  void init_global_var(const koopa_raw_value_t &value);
  void print_num(int num);
  void push_addr_manager();
  void push_stack_offset_manager();
  void pop_addr_manager();
  void pop_stack_offset_manager();
  AddrManager &get_addr_manager();
  StackOffsetManager &get_stack_offset_manager();
  int store_aggregate(const koopa_raw_value_t &value, int dest_offset);
  void alloc_aggregate(const koopa_raw_value_t &value);
  int get_elem_size(const koopa_raw_type_t &type);

  std::stringstream oss;
  koopa_raw_program_builder_t builder;
  koopa_raw_program_t raw;
  std::vector<AddrManager> addr_managers;
  std::vector<StackOffsetManager> stack_offset_managers;
};
