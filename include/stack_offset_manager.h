#pragma once

#include <cassert>
#include <string>
#include <unordered_map>

#include "koopa.h"

class StackOffsetManager {
public:
  void setOffset(const koopa_raw_value_t &value);
  void setOffset(int idx, int offset);
  int getOffset(const koopa_raw_binary_t &binary);
  int getOffset(const koopa_raw_call_t &call);
  int getOffset(const koopa_raw_func_arg_ref_t &func_arg_ref);
  int getOffset(const koopa_raw_load_t &load);
  int getOffset(const std::string &alloc_name);
  int getOffset(const koopa_raw_value_t &value);
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
  std::unordered_map<int, int> func_arg_idx_id_map;
  std::unordered_map<const koopa_raw_call_t *, int> call_id_map;
  std::unordered_map<int, int> id_to_offset_map;

  int id_counter = 0;
};
