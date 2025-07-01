// riscv_codegen.cpp
#include <cstddef>
#include <iostream>
#include <string>

#include "koopa.h"
#include "riscv_codegen.h"
#include "util.h"

CodeGen::CodeGen(const std::string &koopa_ir) {
  push_addr_manager();
  push_stack_offset_manager();
  koopa_program_t program;
  assert(koopa_parse_from_string(koopa_ir.c_str(), &program) ==
         KOOPA_EC_SUCCESS);
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
  oss << "  .data\n";
  // 访问所有全局变量
  Visit(program.values);
  oss << "\n\n";
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
  if (func->bbs.len == 0) {
    return;
  }
  oss << "  .text\n";
  oss << "  .globl " << get_label(func->name) << "\n";
  oss << get_label(func->name) << ":\n";
  push_stack_offset_manager();
  push_addr_manager();
  AllocateStack(func);
  modify_sp(-get_stack_offset_manager().final_stack_size, oss);
  if (get_stack_offset_manager().r != 0) {
    oss << "  li t6, " << get_stack_offset_manager().final_stack_size - 4
        << "\n";
    oss << "  add t6, sp, t6\n";
    oss << "  sw ra, 0(t6)\n";
  }
  Visit(func->bbs);
  pop_stack_offset_manager();
  pop_addr_manager();
  oss << "\n";
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
  case KOOPA_RVT_CALL:
    // 访问 call 指令
    Visit(kind.data.call);
    break;
  case KOOPA_RVT_GLOBAL_ALLOC:
    // 访问 global_alloc 指令
    Visit(kind.data.global_alloc, value->name);
    break;
  case KOOPA_RVT_GET_ELEM_PTR:
    // 访问 get_elem_ptr 指令
    Visit(kind.data.get_elem_ptr);
    break;
  default:
    // 其他类型暂时遇不到
    std::cerr << "Unknown instruction: " << kind.tag << std::endl;
    assert(false);
  }
}

void CodeGen::Visit(const koopa_raw_return_t &ret) {
  if (ret.value == nullptr) {
    if (get_stack_offset_manager().r != 0) {
      oss << "  li t6, " << get_stack_offset_manager().final_stack_size - 4
          << "\n";
      oss << "  add t6, sp, t6\n";
      oss << "  lw ra, 0(t6)\n";
    }
    modify_sp(get_stack_offset_manager().final_stack_size, oss);
    oss << "  ret\n";
    return;
  }
  if (ret.value->kind.tag == KOOPA_RVT_INTEGER) {
    oss << "  li a0, " << get_value(ret.value) << "\n";
  } else if (ret.value->kind.tag == KOOPA_RVT_GLOBAL_ALLOC) {
    oss << "  la a0, " << get_label(ret.value->name) << "\n";
    oss << "  lw a0, 0(a0)\n";
  } else {
    int offset = get_stack_offset_manager().getOffset(ret.value);
    oss << "  li t6, " << offset << "\n";
    oss << "  add t6, sp, t6\n";
    oss << "  lw a0, 0(t6)\n";
  }
  if (get_stack_offset_manager().r != 0) {
    oss << "  li t6, " << get_stack_offset_manager().final_stack_size - 4
        << "\n";
    oss << "  add t6, sp, t6\n";
    oss << "  lw ra, 0(t6)\n";
  }
  modify_sp(get_stack_offset_manager().final_stack_size, oss);
  oss << "  ret\n";
}

// return int32_t value of koopa_raw_value_t
int32_t CodeGen::get_value(const koopa_raw_value_t val) {
  return val->kind.data.integer.value;
}

void CodeGen::Visit(const koopa_raw_integer_t &i32) { oss << i32.value; }

void CodeGen::Visit(const koopa_raw_binary_t &binary) {
  auto &addr_manager = get_addr_manager();
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
  oss << "  li t6, " << get_stack_offset_manager().getOffset(binary) << "\n";
  oss << "  add t6, sp, t6\n";
  oss << "  sw " << res_addr << ", 0(t6)\n";
  addr_manager.freeReg(res_addr);
  addr_manager.freeReg(lhs_addr);
  addr_manager.freeReg(rhs_addr);
  addr_manager.freeId(binary);
  addr_manager.freeId(binary.lhs);
  addr_manager.freeId(binary.rhs);
}

void CodeGen::Visit(const koopa_raw_load_t &load) {
  auto &stack_offset_manager = get_stack_offset_manager();
  auto &addr_manager = get_addr_manager();
  std::string res_addr = addr_manager.getAddr(load);
  if (load.src->kind.tag == KOOPA_RVT_GLOBAL_ALLOC) {
    oss << "  lw " << res_addr << ", " << get_label(load.src->name) << "\n";
  } else if (load.src->kind.tag == KOOPA_RVT_GET_ELEM_PTR) {
    int src_offset =
        stack_offset_manager.getOffset(load.src->kind.data.get_elem_ptr);
    oss << "  li t6, " << src_offset << "\n";
    oss << "  add t6, sp, t6\n";
    oss << "  lw " << res_addr << ", 0(t6)\n";
    oss << "  lw " << res_addr << ", 0(" << res_addr << ")\n";
  } else {
    int src_offset = stack_offset_manager.getOffset(load.src->name);
    oss << "  li t6, " << src_offset << "\n";
    oss << "  add t6, sp, t6\n";
    oss << "  lw " << res_addr << ", 0(t6)\n";
  }
  oss << "  li t5, " << stack_offset_manager.getOffset(load) << "\n";
  oss << "  add t5, sp, t5\n";
  oss << "  sw " << res_addr << ", 0(t5)\n";
  addr_manager.freeReg(res_addr);
  addr_manager.freeId(load);
}

void CodeGen::Visit(const koopa_raw_store_t &store) {
  if (store.value->kind.tag == KOOPA_RVT_AGGREGATE) {
    auto dest_offset = get_stack_offset_manager().getOffset(store.dest->name);
    auto agg = store.value->kind.data.aggregate;
    for (int i = 0; i < agg.elems.len; ++i) {
      auto elem = reinterpret_cast<koopa_raw_value_t>(agg.elems.buffer[i]);
      if (elem->kind.tag == KOOPA_RVT_INTEGER) {
        oss << "  li t6, " << dest_offset + i * 4 << "\n";
        oss << "  add t6, sp, t6\n";
        oss << "  li t5, " << get_value(elem) << "\n";
        oss << "  sw t5, 0(t6)\n";
      } else if (elem->kind.tag == KOOPA_RVT_BINARY) {
        auto bin_addr = get_addr_manager().getAddr(elem);
        oss << "  li t6, " << dest_offset + i * 4 << "\n";
        oss << "  add t6, sp, t6\n";
        oss << "  sw " << bin_addr << ", 0(t6)\n";
        get_addr_manager().freeReg(bin_addr);
        get_addr_manager().freeId(elem);
      } else {
        assert(false);
      }
    }
    return;
  }

  std::string val_addr = get_addr_manager().getAddr(store.value);
  cmd_li(store.value, val_addr);

  std::string dest_name = "";
  std::string dest_str = "";
  if (store.dest->name != nullptr) {
    dest_name = store.dest->name;
  }
  if (store.dest->kind.tag == KOOPA_RVT_GLOBAL_ALLOC) {
    std::string tmp_reg = get_addr_manager().getAddr(store);
    oss << "  la " << tmp_reg << ", " << get_label(store.dest->name) << "\n";
    dest_str = "0(" + tmp_reg + ")";
    get_addr_manager().freeReg(tmp_reg);
    get_addr_manager().freeId(store.dest);
  } else if (store.dest->kind.tag == KOOPA_RVT_GET_ELEM_PTR) {
    int dest_offset = get_stack_offset_manager().getOffset(
        store.dest->kind.data.get_elem_ptr);
    oss << "  li t6, " << dest_offset << "\n";
    oss << "  add t6, sp, t6\n";
    oss << "  lw t6, 0(t6)\n"; 
    dest_str = "0(t6)";
  } else {
    int dest_offset = get_stack_offset_manager().getOffset(dest_name);
    oss << "  li t6, " << dest_offset << "\n";
    oss << "  add t6, sp, t6\n";
    dest_str = "0(t6)";
  }

  oss << "  sw " << val_addr << ", " << dest_str << "\n";

  get_addr_manager().freeReg(val_addr);
  get_addr_manager().freeId(store.value);
}

void CodeGen::Visit(const koopa_raw_branch_t &branch) {
  auto &addr_manager = get_addr_manager();
  std::string cond_addr = addr_manager.getAddr(branch.cond);
  cmd_li(branch.cond, cond_addr);
  oss << "  bnez " << cond_addr << ", " << get_label(branch.true_bb->name)
      << "\n";
  oss << "  j " << get_label(branch.false_bb->name) << "\n";
  addr_manager.freeReg(cond_addr);
  addr_manager.freeId(branch.cond);
}

void CodeGen::Visit(const koopa_raw_jump_t &jump) {
  oss << "  j " << get_label(jump.target->name) << "\n";
}

void CodeGen::Visit(const koopa_raw_call_t &call) {
  for (int i = 0; i < call.args.len; ++i) {
    auto ptr = call.args.buffer[i];
    auto arg = reinterpret_cast<koopa_raw_value_t>(ptr);
    if (i <= 7) {
      if (arg->kind.tag == KOOPA_RVT_INTEGER) {
        oss << "  li " << "a" + std::to_string(i) << ", " << get_value(arg)
            << "\n";
      } else {
        int offset = get_stack_offset_manager().getOffset(arg);
        oss << "  li t6, " << offset << "\n";
        oss << "  add t6, sp, t6\n";
        oss << "  lw " << "a" + std::to_string(i) << ", 0(t6)\n";
        std::string arg_addr = get_addr_manager().getAddr(arg);
      }
    } else {
      std::string arg_addr = get_addr_manager().getAddr(arg);
      if (arg->kind.tag == KOOPA_RVT_INTEGER) {
        cmd_li(arg, arg_addr);
        oss << "  li t6, " << (i - 8) * 4 << "\n";
        oss << "  add t6, sp, t6\n";
        oss << "  sw " << arg_addr << ", 0(t6)\n";
      } else {
        oss << "  li t6, " << (i - 8) * 4 << "\n";
        oss << "  add t6, sp, t6\n";
        oss << "  sw " << arg_addr << ", 0(t6)\n";
      }
      get_addr_manager().freeReg(arg_addr);
      get_addr_manager().freeId(arg);
    }
  }
  oss << "  call " << get_label(call.callee->name) << "\n";
  if (call.callee->ty->data.function.ret->tag != KOOPA_RTT_UNIT) {
    oss << "  li t6, " << get_stack_offset_manager().getOffset(call) << "\n";
    oss << "  add t6, sp, t6\n";
    oss << "  sw a0, 0(t6)\n";
  }
}

void CodeGen::Visit(const koopa_raw_global_alloc_t &global_alloc,
                    std::string var_name) {
  oss << "  .global " << get_label(var_name) << "\n";
  oss << get_label(var_name) << ":\n";
  if (global_alloc.init->kind.tag == KOOPA_RVT_INTEGER) {
    oss << "  .word " << get_value(global_alloc.init) << "\n";
  } else if (global_alloc.init->kind.tag == KOOPA_RVT_ZERO_INIT) {
    if (global_alloc.init->ty->tag == KOOPA_RTT_ARRAY) {
      oss << "  .zero "
          << global_alloc.init->ty->data.array.len * 4
          << "\n";
    } else {
      oss << "  .zero 4\n";
    }
  } else if (global_alloc.init->kind.tag == KOOPA_RVT_AGGREGATE) {
    auto agg = global_alloc.init->kind.data.aggregate;
    auto len = global_alloc.init->ty->data.array.len;
    for (int i = 0; i < len; ++i) {
      if (i < agg.elems.len) {
        auto elem = reinterpret_cast<koopa_raw_value_t>(agg.elems.buffer[i]);
        if (elem->kind.tag == KOOPA_RVT_INTEGER) {
          oss << "  .word " << get_value(elem) << "\n";
        } else if (elem->kind.tag == KOOPA_RVT_ZERO_INIT) {
          oss << "  .zero 4\n";
        } else {
          assert(false);
        }
      } else {
        oss << "  .zero 4\n";
      }
    }
  } else {
    assert(false);
  }
}

void CodeGen::Visit(const koopa_raw_get_elem_ptr_t &get_elem_ptr) {
  if (get_elem_ptr.src->kind.tag == KOOPA_RVT_GLOBAL_ALLOC) {
    auto &addr_manager = get_addr_manager();
    std::string res_addr = addr_manager.getAddr(get_elem_ptr);
    oss << "  la " << res_addr << ", " << get_label(get_elem_ptr.src->name)
        << "\n";
    auto idx_addr = addr_manager.getAddr(get_elem_ptr.index);
    cmd_li(get_elem_ptr.index, idx_addr);
    oss << "  li t6, " << 4 << "\n";
    oss << "  mul t6, " << idx_addr << ", t6\n";
    oss << "  add " << res_addr << ", " << res_addr << ", t6\n";
    oss << "  li t6, " << get_stack_offset_manager().getOffset(get_elem_ptr)
        << "\n";
    oss << "  add t6, sp, t6\n";
    oss << "  sw " << res_addr << ", 0(t6)\n";
    addr_manager.freeReg(res_addr);
    addr_manager.freeReg(idx_addr);
    addr_manager.freeId(get_elem_ptr);
    addr_manager.freeId(get_elem_ptr.index);
    return;
  }
  auto &stack_offset_manager = get_stack_offset_manager();
  auto &addr_manager = get_addr_manager();
  std::string res_addr = addr_manager.getAddr(get_elem_ptr);
  int array_offset = stack_offset_manager.getOffset(get_elem_ptr.src->name);
  oss << "  li t6, " << array_offset << "\n";
  oss << "  add " << res_addr << ", sp, t6\n";

  std::string idx_addr = addr_manager.getAddr(get_elem_ptr.index);
  cmd_li(get_elem_ptr.index, idx_addr);

  auto elem = get_elem_ptr.src->ty->data.pointer.base;
  auto elem_ty_tag = elem->tag;
  int elem_size = 4;
  while (elem_ty_tag == KOOPA_RTT_POINTER) {
    auto array = elem->data.pointer.base->data.array;
    elem_size = array.len * elem_size;
    elem = array.base;
    elem_ty_tag = elem->tag;
  }
  oss << "  li t6, " << elem_size << "\n";
  oss << "  mul t6, " << idx_addr << ", t6\n";
  oss << "  add " << res_addr << ", " << res_addr << ", t6\n";

  oss << "  li t6, " << stack_offset_manager.getOffset(get_elem_ptr) << "\n";
  oss << "  add t6, sp, t6\n";
  oss << "  sw " << res_addr << ", 0(t6)\n";
  addr_manager.freeReg(idx_addr);
  addr_manager.freeId(get_elem_ptr.index);
  addr_manager.freeReg(res_addr);
  addr_manager.freeId(get_elem_ptr);
}

void CodeGen::cmd_li(const koopa_raw_value_t &value, std::string &res_addr) {
  auto &stack_offset_manager = get_stack_offset_manager();
  if (res_addr == "x0") {
    return;
  }
  if (value->kind.tag == KOOPA_RVT_INTEGER) {
    oss << "  li " << res_addr << ", " << get_value(value) << "\n";
  } else if (value->kind.tag == KOOPA_RVT_BINARY) {
    oss << "  li t6, "
        << stack_offset_manager.getOffset(value->kind.data.binary) << "\n";
    oss << "  add t6, sp, t6\n";
    oss << "  lw " << res_addr << ", 0(t6)\n";
  } else if (value->kind.tag == KOOPA_RVT_LOAD) {
    oss << "  li t6, " << stack_offset_manager.getOffset(value->kind.data.load)
        << "\n";
    oss << "  add t6, sp, t6\n";
    oss << "  lw " << res_addr << ", 0(t6)\n";
  } else if (value->kind.tag == KOOPA_RVT_FUNC_ARG_REF) {
    auto arg_ref = value->kind.data.func_arg_ref;
    if (arg_ref.index <= 7) {
      get_addr_manager().freeReg(res_addr);
      res_addr = "a" + std::to_string(arg_ref.index);
      return;
    }
    oss << "  li t6, "
        << get_stack_offset_manager().final_stack_size + (arg_ref.index - 8) * 4
        << "\n";
    oss << "  add t6, sp, t6\n";
    oss << "  lw " << res_addr << ", 0(t6)\n";
  } else if (value->kind.tag == KOOPA_RVT_CALL) {
    oss << "  li t6, " << stack_offset_manager.getOffset(value->kind.data.call)
        << "\n";
    oss << "  add t6, sp, t6\n";
    oss << "  lw " << res_addr << ", 0(t6)\n";
  } else if (value->kind.tag == KOOPA_RVT_GLOBAL_ALLOC) {
    oss << "  la " << res_addr << ", " << get_label(value->name) << "\n";
    oss << "  lw " << res_addr << ", 0(" << res_addr << ")\n";
  } else if (value->kind.tag == KOOPA_RVT_AGGREGATE) {
  } else {
    assert(false);
  }
}

// 分配栈空间：为 alloc 和有返回值的指令分配 4 字节，并记录偏移
void CodeGen::AllocateStack(const koopa_raw_function_t &func) {
  auto &stack_offset_manager = get_stack_offset_manager();
  stack_offset_manager.clear();
  int call_num = 0;
  int max_param_num = 0;
  int total_stack_size = 0;
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
      if (inst->kind.tag == KOOPA_RVT_CALL) {
        call_num++;
        if ((int)inst->kind.data.call.args.len > max_param_num) {
          max_param_num = (int)inst->kind.data.call.args.len;
        }
      }
    }
  }
  // 计算总栈空间并 16 字节对齐
  stack_offset_manager.r = std::max(call_num, 0) * 4;
  stack_offset_manager.a = std::max(max_param_num - 8, 0) * 4;

  total_stack_size = stack_offset_manager.current_stack_offset +
                     stack_offset_manager.r + stack_offset_manager.a;
  stack_offset_manager.final_stack_size = ((total_stack_size + 15) / 16) * 16;
}

void CodeGen::push_addr_manager() { addr_managers.push_back(AddrManager()); }

void CodeGen::push_stack_offset_manager() {
  stack_offset_managers.push_back(StackOffsetManager());
}

void CodeGen::pop_addr_manager() { addr_managers.pop_back(); }

void CodeGen::pop_stack_offset_manager() { stack_offset_managers.pop_back(); }

AddrManager &CodeGen::get_addr_manager() {
  assert(!addr_managers.empty());
  return addr_managers.back();
}

StackOffsetManager &CodeGen::get_stack_offset_manager() {
  assert(!stack_offset_managers.empty());
  return stack_offset_managers.back();
}