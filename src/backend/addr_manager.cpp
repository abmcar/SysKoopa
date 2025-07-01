#include "addr_manager.h"

int AddrManager::getNextId() { return id_counter++; }

std::string AddrManager::idToAddr(int id) {
  if (id_addr_map.find(id) == id_addr_map.end()) {
    id_addr_map[id] = getReg();
  }
  return id_addr_map[id];
}

std::string AddrManager::getReg() {
  auto reg = regs.begin();
  std::string reg_str = *reg;
  regs.erase(reg);
  return reg_str;
}

template <typename T>
std::string AddrManager::getAddrImpl(const T &obj,
                                     std::unordered_map<const T *, int> &map) {
  int id;
  if (map.find(&obj) == map.end()) {
    id = getNextId();
    map[&obj] = id;
  } else {
    id = map[&obj];
  }
  return idToAddr(id);
}

void AddrManager::freeReg(const std::string &reg) {
  if (reg == "x0") {
    return;
  }
  if (reg[0] == 'a') {
    return;
  }
  regs.insert(reg);
}

std::string AddrManager::getAddr(const koopa_raw_binary_t &binary) {
  return getAddrImpl(binary, raw_bin_id_map);
}

std::string AddrManager::getAddr(const koopa_raw_load_t &load) {
  return getAddrImpl(load, raw_load_id_map);
}

std::string AddrManager::getAddr(const koopa_raw_store_t &store) {
  return getAddrImpl(store, raw_store_id_map);
}

std::string AddrManager::getAddr(const koopa_raw_value_t &value) {
  if (value->kind.tag == KOOPA_RVT_INTEGER &&
      value->kind.data.integer.value == 0) {
    return "x0";
  }
  if (value->kind.tag == KOOPA_RVT_BINARY) {
    return getAddr(value->kind.data.binary);
  }

  return getAddrImpl(value, raw_val_id_map);
}

std::string AddrManager::getAddr(const koopa_raw_get_elem_ptr_t &get_elem_ptr) {
  return getAddrImpl(get_elem_ptr, raw_get_elem_ptr_id_map);
}

void AddrManager::freeId(const koopa_raw_binary_t &binary) {
  raw_bin_id_map.erase(&binary);
}

void AddrManager::freeId(const koopa_raw_load_t &load) {
  raw_load_id_map.erase(&load);
}

void AddrManager::freeId(const koopa_raw_store_t &store) {
  raw_store_id_map.erase(&store);
}

void AddrManager::freeId(const koopa_raw_value_t &value) {
  raw_val_id_map.erase(&value);
}

void AddrManager::freeId(const koopa_raw_get_elem_ptr_t &get_elem_ptr) {
  raw_get_elem_ptr_id_map.erase(&get_elem_ptr);
}

// Explicit template instantiation if needed
template std::string AddrManager::getAddrImpl(const koopa_raw_binary_t &,
                                              std::unordered_map<const koopa_raw_binary_t *, int> &);
template std::string AddrManager::getAddrImpl(const koopa_raw_load_t &,
                                              std::unordered_map<const koopa_raw_load_t *, int> &);
template std::string AddrManager::getAddrImpl(const koopa_raw_store_t &,
                                              std::unordered_map<const koopa_raw_store_t *, int> &);
template std::string AddrManager::getAddrImpl(const koopa_raw_value_t &,
                                              std::unordered_map<const koopa_raw_value_t *, int> &);
template std::string AddrManager::getAddrImpl(const koopa_raw_get_elem_ptr_t &,
                                              std::unordered_map<const koopa_raw_get_elem_ptr_t *, int> &);