#pragma once

#include <cassert>
#include <set>
#include <string>
#include <unordered_map>

#include "koopa.h"

class AddrManager {
public:
  std::string getAddr(const koopa_raw_binary_t &binary);
  std::string getAddr(const koopa_raw_load_t &load);
  std::string getAddr(const koopa_raw_store_t &store);
  std::string getAddr(const koopa_raw_value_t &value);
  std::string getAddr(const koopa_raw_get_ptr_t &get_ptr);
  std::string getAddr(const koopa_raw_get_elem_ptr_t &get_elem_ptr);
  void freeReg(const std::string &reg);
  void freeId(const koopa_raw_binary_t &binary);
  void freeId(const koopa_raw_load_t &load);
  void freeId(const koopa_raw_store_t &store);
  void freeId(const koopa_raw_value_t &value);
  void freeId(const koopa_raw_get_elem_ptr_t &get_elem_ptr);
  void freeId(const koopa_raw_get_ptr_t &get_ptr);
private:
  std::string getReg();
  int getNextId();
  std::string idToAddr(int id);
  template <typename T>
  std::string getAddrImpl(const T &obj,
                          std::unordered_map<const T *, int> &map);

  std::unordered_map<const koopa_raw_binary_t *, int> raw_bin_id_map;
  std::unordered_map<const koopa_raw_value_t *, int> raw_val_id_map;
  std::unordered_map<const koopa_raw_load_t *, int> raw_load_id_map;
  std::unordered_map<const koopa_raw_store_t *, int> raw_store_id_map;
  std::unordered_map<const koopa_raw_get_elem_ptr_t *, int> raw_get_elem_ptr_id_map;
  std::unordered_map<const koopa_raw_get_ptr_t *, int> raw_get_ptr_id_map;

  std::unordered_map<int, std::string> id_addr_map;
  std::unordered_map<std::string, int> addr_id_map;

  std::set<std::string> regs = {"t0", "t1", "t2", "t3", "t4", "t5", "t6"};
  int id_counter = 0;
};
