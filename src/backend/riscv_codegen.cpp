// riscv_codegen.cpp
#include "riscv_codegen.h"
#include "koopa.h"
#include "util.h"

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
std::string AddrManager::getAddrImpl(
    const T &obj, std::unordered_map<const T *, int> &map) {
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
  regs.insert(reg);
}

std::string AddrManager::getAddr(const koopa_raw_binary_t &binary) {
  return getAddrImpl(binary, raw_bin_id_map);
}

std::string AddrManager::getAddr(const koopa_raw_store_t &store) {
  return getAddrImpl(store, raw_store_id_map);
}

std::string AddrManager::getAddr(const koopa_raw_load_t &load) {
  return getAddrImpl(load, raw_load_id_map);
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

void AddrManager::freeId(const koopa_raw_value_t &value) {
  raw_val_id_map.erase(&value);
}

void AddrManager::freeId(const koopa_raw_store_t &store) {
  raw_store_id_map.erase(&store);
}

void AddrManager::freeId(const koopa_raw_load_t &load) {
  raw_load_id_map.erase(&load);
}

void AddrManager::freeId(const koopa_raw_binary_t &binary) {
  raw_bin_id_map.erase(&binary);
}

int StackOffsetManager::getNextId() { return ++id_counter; }

int StackOffsetManager::id_to_offset(int id) {
  if (id_to_offset_map.find(id) == id_to_offset_map.end()) {
    assert(false);
  }
  return id_to_offset_map[id];
}

void StackOffsetManager::setOffset(const koopa_raw_value_t &value) {
  auto kind = value->kind.tag;
  switch (kind) {
  case KOOPA_RVT_LOAD:
    load_id_map[&value->kind.data.load] = getNextId();
    id_to_offset_map[load_id_map[&value->kind.data.load]] =
        current_stack_offset;
    current_stack_offset += 4;
    break;
  case KOOPA_RVT_BINARY:
    binary_id_map[&value->kind.data.binary] = getNextId();
    id_to_offset_map[binary_id_map[&value->kind.data.binary]] =
        current_stack_offset;
    current_stack_offset += 4;
    break;
  case KOOPA_RVT_ALLOC:
    alloc_name_id_map[value->name] = getNextId();
    id_to_offset_map[alloc_name_id_map[value->name]] = current_stack_offset;
    current_stack_offset += 4;
    break;
  default:
    assert(false);
  }
}

int StackOffsetManager::getOffset(const koopa_raw_load_t &load) {
  int id;
  if (load_id_map.find(&load) == load_id_map.end()) {
    assert(false);
  } else {
    id = load_id_map[&load];
  }
  return id_to_offset(id);
}

int StackOffsetManager::getOffset(const koopa_raw_binary_t &binary) {
  int id;
  if (binary_id_map.find(&binary) == binary_id_map.end()) {
    assert(false);
  } else {
    id = binary_id_map[&binary];
  }
  return id_to_offset(id);
}

int StackOffsetManager::getOffset(const std::string &alloc_name) {
  int id;
  if (alloc_name_id_map.find(alloc_name) == alloc_name_id_map.end()) {
    assert(false);
  } else {
    id = alloc_name_id_map[alloc_name];
  }
  return id_to_offset(id);
}

void StackOffsetManager::clear() {
  load_id_map.clear();
  binary_id_map.clear();
  alloc_name_id_map.clear();
  id_to_offset_map.clear();
  current_stack_offset = 0;
  id_counter = 0;
}

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
  return oss.str();
}

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
  std::string func_name = func->name;
  func_name.erase(0, 1);
  oss << "  .globl " << func_name << "\n";
  oss << func_name << ":\n";
  AllocateStack(func);
  modify_sp(-total_stack_size, oss);
  // 访问所有基本块
  Visit(func->bbs);
}

// 访问基本块
void CodeGen::Visit(const koopa_raw_basic_block_t &bb) {
  // 执行一些其他的必要操作
  // ...
  // 访问所有指令
  std::string label = get_label(bb->name);
  if (label != "entry") {
    oss << label << ":\n";
  }
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
  case KOOPA_RVT_BINARY:
    // 访问 binary 指令
    Visit(kind.data.binary);
    break;
  case KOOPA_RVT_LOAD:
    // 访问 load 指令
    Visit(kind.data.load);
    break;
  case KOOPA_RVT_STORE:
    // 访问 store 指令
    Visit(kind.data.store);
    break;
  case KOOPA_RVT_ALLOC:
    // 访问 alloc 指令
    break;
  case KOOPA_RVT_BRANCH:
    // 访问 branch 指令
    Visit(kind.data.branch);
    break;
  case KOOPA_RVT_JUMP:
    // 访问 jump 指令
    Visit(kind.data.jump);
    break;
  default:
    // 其他类型暂时遇不到
    std::cerr << "Unknown instruction: " << kind.tag << std::endl;
    assert(false);
  }
}

void CodeGen::Visit(const koopa_raw_return_t &ret) {
  if (ret.value->kind.tag == KOOPA_RVT_INTEGER) {
    oss << "  li a0, " << get_value(ret.value) << "\n";
  } else {
    oss << "  mv a0, " << addr_manager.getAddr(ret.value) << "\n";
  }
  modify_sp(total_stack_size, oss);
  oss << "  ret\n";
}

int32_t CodeGen::get_value(const koopa_raw_value_t val) {
  return val->kind.data.integer.value;
}

void CodeGen::Visit(const koopa_raw_integer_t &i32) { oss << i32.value; }

void CodeGen::Visit(const koopa_raw_binary_t &binary) {
  std::string lhs_addr = addr_manager.getAddr(binary.lhs);
  std::string rhs_addr = addr_manager.getAddr(binary.rhs);
  cmd_li(binary.lhs, lhs_addr);
  cmd_li(binary.rhs, rhs_addr);
  std::string res_addr = addr_manager.getAddr(binary);

  switch (binary.op) {
  case KOOPA_RBO_SUB:
    oss << "  sub " << res_addr << ", " << lhs_addr << ", " << rhs_addr << "\n";
    break;
  case KOOPA_RBO_NOT_EQ:
    oss << "  xor " << res_addr << ", " << lhs_addr << ", " << rhs_addr << "\n";
    oss << "  snez " << res_addr << ", " << res_addr << "\n";
    break;
  case KOOPA_RBO_EQ:
    oss << "  xor " << res_addr << ", " << lhs_addr << ", " << rhs_addr << "\n";
    oss << "  seqz " << res_addr << ", " << res_addr << "\n";
    break;
  case KOOPA_RBO_ADD:
    oss << "  add " << res_addr << ", " << lhs_addr << ", " << rhs_addr << "\n";
    break;
  case KOOPA_RBO_MUL:
    oss << "  mul " << res_addr << ", " << lhs_addr << ", " << rhs_addr << "\n";
    break;
  case KOOPA_RBO_DIV:
    oss << "  div " << res_addr << ", " << lhs_addr << ", " << rhs_addr << "\n";
    break;
  case KOOPA_RBO_MOD:
    oss << "  rem " << res_addr << ", " << lhs_addr << ", " << rhs_addr << "\n";
    break;
  case KOOPA_RBO_AND:
    oss << "  and " << res_addr << ", " << lhs_addr << ", " << rhs_addr << "\n";
    break;
  case KOOPA_RBO_OR:
    oss << "  or " << res_addr << ", " << lhs_addr << ", " << rhs_addr << "\n";
    break;
  case KOOPA_RBO_GT:
    oss << "  sgt " << res_addr << ", " << lhs_addr << ", " << rhs_addr << "\n";
    break;
  case KOOPA_RBO_LT:
    oss << "  slt " << res_addr << ", " << lhs_addr << ", " << rhs_addr << "\n";
    break;
  case KOOPA_RBO_GE:
    oss << "  slt " << res_addr << ", " << lhs_addr << ", " << rhs_addr << "\n";
    oss << "  xori " << res_addr << ", " << res_addr << ", 1\n";
    break;
  case KOOPA_RBO_LE:
    oss << "  sgt " << res_addr << ", " << lhs_addr << ", " << rhs_addr << "\n";
    oss << "  xori " << res_addr << ", " << res_addr << ", 1\n";
    break;
  default:
    std::cerr << "Unknown binary operation: " << binary.op << std::endl;
    assert(false);
  }
  oss << "  sw " << res_addr << ", " << stack_offset_manager.getOffset(binary) << "(sp)\n";
  // addr_manager.freeReg(res_addr);
  addr_manager.freeReg(lhs_addr);
  addr_manager.freeReg(rhs_addr);
  // addr_manager.freeId(binary);
  addr_manager.freeId(binary.lhs);
  addr_manager.freeId(binary.rhs);
}

void CodeGen::Visit(const koopa_raw_load_t &load) {
  std::string res_addr = addr_manager.getAddr(load);
  int src_offset = stack_offset_manager.getOffset(load.src->name);
  oss << "  lw " << res_addr << ", " << src_offset << "(sp)\n";
  int offset = stack_offset_manager.getOffset(load);
  oss << "  sw " << res_addr << ", " << offset << "(sp)\n";
  addr_manager.freeReg(res_addr);
  addr_manager.freeId(load);
}

void CodeGen::Visit(const koopa_raw_store_t &store) {
  std::string dest_name = store.dest->name;
  int dest_offset = stack_offset_manager.getOffset(dest_name);
  std::string val_addr = addr_manager.getAddr(store.value);
  if (store.value->kind.tag == KOOPA_RVT_BINARY) {
    cmd_li(store.value, val_addr);
  } else if (store.value->kind.tag == KOOPA_RVT_INTEGER) {
    cmd_li(store.value, val_addr);
  } else {
    assert(false);
  }
  oss << "  sw " << val_addr << ", " << dest_offset << "(sp)\n";
  addr_manager.freeReg(val_addr);
  addr_manager.freeId(store.value);
}

void CodeGen::Visit(const koopa_raw_branch_t &branch) {
  std::string cond_addr = addr_manager.getAddr(branch.cond);
  cmd_li(branch.cond, cond_addr);
  oss << "  bnez " << cond_addr << ", " << get_label(branch.true_bb->name) << "\n";
  oss << "  j " << get_label(branch.false_bb->name) << "\n";
  addr_manager.freeReg(cond_addr);
}

void CodeGen::Visit(const koopa_raw_jump_t &jump) {
  oss << "  j " << get_label(jump.target->name) << "\n";
  // oss << "jumpTest\n";
}

void CodeGen::cmd_li(const koopa_raw_value_t &value,
                     const std::string &res_addr) {
  if (res_addr == "x0") {
    return;
  }
  if (value->kind.tag == KOOPA_RVT_INTEGER) {
    oss << "  li " << res_addr << ", " << get_value(value) << "\n";
  } else if (value->kind.tag == KOOPA_RVT_BINARY) {
    oss << "  lw " << res_addr << ", " << stack_offset_manager.getOffset(value->kind.data.binary)  << "(sp)\n";
  } else if (value->kind.tag == KOOPA_RVT_LOAD) {
    oss << "  lw " << res_addr << ", " << stack_offset_manager.getOffset(value->kind.data.load)  << "(sp)\n";
  } else {  
    assert(false);
  }
}

// 分配栈空间：为 alloc 和有返回值的指令分配 4 字节，并记录偏移
void CodeGen::AllocateStack(const koopa_raw_function_t &func) {
  stack_offset_manager.clear();
  // 遍历所有基本块
  for (size_t i = 0; i < func->bbs.len; ++i) {
    auto bb = reinterpret_cast<koopa_raw_basic_block_t>(func->bbs.buffer[i]);
    // 遍历基本块内所有指令
    for (size_t j = 0; j < bb->insts.len; ++j) {
      auto inst = reinterpret_cast<koopa_raw_value_t>(bb->insts.buffer[j]);
      // alloc 或有返回值的指令
      if (inst->kind.tag == KOOPA_RVT_ALLOC ||
          (inst->ty->tag != KOOPA_RTT_UNIT)) {
        stack_offset_manager.setOffset(inst);
      }
    }
  }
  // 计算总栈空间并 16 字节对齐
  total_stack_size =
      ((stack_offset_manager.current_stack_offset + 15) / 16) * 16;
}