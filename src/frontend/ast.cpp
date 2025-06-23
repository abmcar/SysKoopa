#include "ast.h"
#include "symbol_table.h"
#include "util.h"
#include <string>

std::ostream &operator<<(std::ostream &os, BaseAST &ast) {
  ast.print(os);
  return os;
};

void CompUnitAST::print(std::ostream &os) {
  for (auto &item : *func_def_list) {
    item->print(os);
    os << "\n";
  }
}

void FuncDefAST::print(std::ostream &os) {
  os << "fun @" << this->ident << "(";
  if (func_fparam_list != nullptr) {
    auto func_fparams =
        SymbolTableManger::getInstance().get_func_fparams(this->ident);
    int n = func_fparams.size();
    for (int i = 0; i < n; i++) {
      os << "@" << func_fparams[i].ident << ": ";
      if (func_fparams[i].b_type == "int") {
        os << "i32";
      } else {
        assert(false);
      }
      if (i != n - 1) {
        os << ", ";
      }
    }
  }
  os << ")";
  if (func_type == "int") {
    os << ": i32";
  }
  os << " {\n";
  os << "%"
     << "entry"
     << ":\n";

  SymbolTableManger::getInstance().use_stmt_table(this->block.get());
  if (func_fparam_list != nullptr) {
    auto func_fparams =
        SymbolTableManger::getInstance().get_func_fparams(this->ident);
    int n = func_fparams.size();
    for (int i = 0; i < n; i++) {
      SymbolTableManger::getInstance().alloc_ident(func_fparams[i].ident);
      SymbolTableManger::getInstance().get_back_table().def_type_map[func_fparams[i].ident] = SymbolTable::DefType::VAR_IDENT;
      SymbolTableManger::getInstance().get_back_table().lval_ident_map[func_fparams[i].ident] = this->ident + "_" + func_fparams[i].ident;
      os << "  @" << this->ident << "_" << func_fparams[i].ident << " = alloc i32\n";
      os << "  store @" << func_fparams[i].ident << ", @" << this->ident << "_" << func_fparams[i].ident << "\n";
    }
  }
  block->print(os);
  SymbolTableManger::getInstance().pop_symbol_table();
  if (!IRManager::getInstance().is_return_map[os.tellp()]) {
    os << "  ret\n";
  }
  os << "}\n";
}

void DeclAST::print(std::ostream &os) {
  if (kind == DeclAST::Kind::VAR_DECL) {
    var_decl->print(os);
  } else if (kind == DeclAST::Kind::CONST_DECL) {
    const_decl->print(os);
  }
}

void ConstDeclAST::print(std::ostream &os) {
  for (auto &item : *const_def_list) {
    item->print(os);
  }
}

void FuncFParamAST::print(std::ostream &os) {}

void BTypeAST::print(std::ostream &os) {}

void VarDeclAST::print(std::ostream &os) {
  for (auto &item : *var_def_list) {
    item->print(os);
  }
}

void VarDefAST::print(std::ostream &os) {
  std::string ident = SymbolTableManger::getInstance()
                          .get_back_table()
                          .lval_ident_map[this->ident];
  os << "  @" + ident << " = alloc i32\n";
  if (kind == DefAST::Kind::VAR_DEF) {
    init_val->print(os);
    os << "  store " << get_koopa_exp_reg(init_val.get()) << ", @" + ident
       << "\n";
  }
  SymbolTableManger::getInstance().alloc_ident(this->ident);
}

void ConstDefAST::print(std::ostream &os) {
  SymbolTableManger::getInstance().alloc_ident(this->ident);
}

void BlockAST::print(std::ostream &os) {
  if (block_item_list == nullptr) {
    return;
  }
  for (auto &item : *block_item_list) {
    item->print(os);
  }
}

void BlockItemAST::print(std::ostream &os) {
  if (kind == BlockItemAST::Kind::DECL) {
    decl->print(os);
  } else if (kind == BlockItemAST::Kind::STMT) {
    stmt->print(os);
  }
}

void StmtAST::print(std::ostream &os) {
  if (IRManager::getInstance().is_in_while &&
      IRManager::getInstance().is_return_map[os.tellp()]) {
    return;
  }
  if (kind == StmtAST::Kind::RETURN_STMT) {
    if (IRManager::getInstance().is_return_map[os.tellp()]) {
      return;
    }
    if (exp != nullptr) {
      exp->print(os);
      os << "  ret " << get_koopa_exp_reg(exp.get()) << "\n";
    } else {
      os << "  ret\n";
    }
    IRManager::getInstance().is_return_map[os.tellp()] = true;
  } else if (kind == StmtAST::Kind::ASSIGN_STMT) {
    r_exp->print(os);
    std::string ident = SymbolTableManger::getInstance().get_ident(l_val);
    os << "  store " << get_koopa_exp_reg(r_exp.get()) << ", @" + ident << "\n";
  } else if (kind == StmtAST::Kind::BLOCK_STMT) {
    block->print(os);
  } else if (kind == StmtAST::Kind::EXP_STMT) {
    exp->print(os);
  } else if (kind == StmtAST::Kind::EMPTY_STMT) {
  } else if (kind == StmtAST::Kind::IF_STMT) {
    IRManager::getInstance().is_in_if = true;
    if_exp->print(os);
    IRManager::getInstance().is_in_if = false;
    int if_count = IRManager::getInstance().getNextIfCount();
    os << "  br " << get_koopa_exp_reg(if_exp.get()) << ", %then_" << if_count
       << ", %end_" << if_count << "\n";
    os << "%then_" << if_count << ":\n";
    if_stmt->print(os);
    if (!IRManager::getInstance().is_return_map[os.tellp()]) {
      os << "  jump %end_" << if_count << "\n";
    }
    os << "%end_" << if_count << ":\n";
  } else if (kind == StmtAST::Kind::IF_ELSE_STMT) {
    IRManager::getInstance().is_in_if = true;
    IRManager::getInstance().is_in_if_else = true;
    if_exp->print(os);
    IRManager::getInstance().is_in_if = false;
    IRManager::getInstance().is_in_if_else = false;
    int return_count = 0;
    int if_count = IRManager::getInstance().getNextIfCount();
    os << "  br " << get_koopa_exp_reg(if_exp.get()) << ", %then_" << if_count
       << ", %else_" << if_count << "\n";
    os << "%then_" << if_count << ":\n";
    if_stmt->print(os);
    if (!IRManager::getInstance().is_return_map[os.tellp()]) {
      os << "  jump %end_" << if_count << "\n";
      return_count++;
    }
    os << "%else_" << if_count << ":\n";
    else_stmt->print(os);
    if (!IRManager::getInstance().is_return_map[os.tellp()]) {
      os << "  jump %end_" << if_count << "\n";
      return_count++;
    }
    if (return_count != 0) {
      os << "%end_" << if_count << ":\n";
    }
  } else if (kind == StmtAST::Kind::WHILE_STMT) {
    IRManager::getInstance().enter_while();
    int while_count = IRManager::getInstance().getNextWhileCount();
    os << "  jump %entry_while_" << while_count << "\n";
    os << "%entry_while_" << while_count << ":\n";
    while_exp->print(os);
    os << "  br " << get_koopa_exp_reg(while_exp.get()) << ", %while_body_"
       << while_count << ", %while_end_" << while_count << "\n";
    os << "%while_body_" << while_count << ":\n";
    while_stmt->print(os);
    if (!IRManager::getInstance().is_return_map[os.tellp()]) {
      os << "  jump %entry_while_" << while_count << "\n";
    }
    os << "%while_end_" << while_count << ":\n";
    IRManager::getInstance().exit_while();
  } else if (kind == StmtAST::Kind::BREAK_STMT) {
    os << "  jump %while_end_" << IRManager::getInstance().getNowWhileCount()
       << "\n";
    IRManager::getInstance().is_return_map[os.tellp()] = true;
  } else if (kind == StmtAST::Kind::CONTINUE_STMT) {
    os << "  jump %entry_while_" << IRManager::getInstance().getNowWhileCount()
       << "\n";
    IRManager::getInstance().is_return_map[os.tellp()] = true;
  }
}

void ExpAST::print(std::ostream &os) {
  // unary_exp->print(os);
  // if (unary_exp->is_number()) {
  //   number = unary_exp->get_number();
  //   kind = ExpAST::Kind::NUMBER;
  // } else {
  //   reg = unary_exp->get_reg();
  // }
  // add_exp->print(os);
  // if (add_exp->is_number()) {
  //   number = add_exp->get_number();
  //   kind = ExpAST::Kind::NUMBER;
  // } else {
  //   reg = add_exp->get_reg();
  // }
  l_or_exp->print(os);
  pushup_exp_reg(l_or_exp.get(), this);
}

void PrimaryExpAST::print(std::ostream &os) {
  if (kind == Kind::EXP) {
    exp->print(os);
    reg = exp->get_reg();
  } else if (kind == Kind::L_VAL) {
    if (SymbolTableManger::getInstance().get_def_type(l_val) ==
        SymbolTable::DefType::CONST) {
      number = SymbolTableManger::getInstance().get_val(l_val);
      kind = ExpAST::Kind::NUMBER;
    } else {
      reg = IRManager::getInstance().getNextReg();
      std::string ident = SymbolTableManger::getInstance().get_ident(l_val);
      os << "  %" << reg << " = load @" + ident << "\n";
    }
  }
}

void UnaryExpAST::print(std::ostream &os) {
  if (kind == ExpAST::Kind::PRIMARY_EXP) {
    primary_exp->print(os);
    pushup_exp_reg(primary_exp.get(), this);
  } else if (kind == ExpAST::Kind::UNARY_OP_EXP) {
    unary_exp->print(os);
    switch (unary_op) {
    case UnaryOpKind::Plus:
      pushup_exp_reg(unary_exp.get(), this);
      break;
    case UnaryOpKind::Minus:
      reg = IRManager::getInstance().getNextReg();
      os << "  %" << reg << " = sub 0, " << get_koopa_exp_reg(unary_exp.get())
         << "\n";
      break;
    case UnaryOpKind::Not:
      reg = IRManager::getInstance().getNextReg();
      os << "  %" << reg << " = eq " << get_koopa_exp_reg(unary_exp.get())
         << ", 0\n";
      break;
    }
  } else if (kind == ExpAST::Kind::FUNC_CALL_WITHOUT_PARAMS) {
    auto DType = SymbolTableManger::getInstance().get_def_type(ident);
    if (DType == SymbolTable::DefType::FUNC_VOID) {
      os << "  call @" << ident << "()\n";
    } else if (DType == SymbolTable::DefType::FUNC_INT) {
      reg = IRManager::getInstance().getNextReg();
      os << "  %" << reg << " = call @" << ident << "()\n";
    }
  } else if (kind == ExpAST::Kind::FUNC_CALL_WITH_PARAMS) {
    auto func_fparams =
        SymbolTableManger::getInstance().get_func_fparams(ident);
    if (func_fparams.size() != func_rparam_list->size()) {
      std::cout
          << "error: function call with params has wrong number of parameters"
          << std::endl;
      exit(1);
    }
    std::vector<std::string> reg_list;
    for (auto &item : *func_rparam_list) {
      item->print(os);
      reg_list.push_back(get_koopa_exp_reg(item.get()));
    }
    reg = IRManager::getInstance().getNextReg();
    os << "  %" << reg << " = call @" << ident << "(";
    for (int i = 0; i < reg_list.size(); i++) {
      os << reg_list[i];
      if (i != reg_list.size() - 1) {
        os << ", ";
      }
    }
    os << ")\n";
  }
}

void AddExpAST::print(std::ostream &os) {
  if (kind == ExpAST::Kind::MUL_EXP) {
    mul_exp->print(os);
    pushup_exp_reg(mul_exp.get(), this);
  } else {
    mul_exp->print(os);
    add_exp->print(os);
    reg = IRManager::getInstance().getNextReg();
    const char *op = add_op == AddOpKind::Plus ? "add" : "sub";
    os << "  %" << reg << " = " << op << " " << get_koopa_exp_reg(add_exp.get())
       << ", " << get_koopa_exp_reg(mul_exp.get()) << "\n";
  }
}

void MulExpAST::print(std::ostream &os) {
  if (kind == ExpAST::Kind::UNARY_EXP) {
    unary_exp->print(os);
    pushup_exp_reg(unary_exp.get(), this);
  } else {
    mul_exp->print(os);
    unary_exp->print(os);
    reg = IRManager::getInstance().getNextReg();
    const char *op = "mul";
    switch (mul_op) {
    case MulOpKind::Mul:
      op = "mul";
      break;
    case MulOpKind::Div:
      op = "div";
      break;
    case MulOpKind::Mod:
      op = "mod";
      break;
    }
    os << "  %" << reg << " = " << op << " " << get_koopa_exp_reg(mul_exp.get())
       << ", " << get_koopa_exp_reg(unary_exp.get()) << "\n";
  }
}

void LOrExpAST::print(std::ostream &os) {
  if (kind == ExpAST::Kind::L_AND_EXP) {
    l_and_exp->print(os);
    pushup_exp_reg(l_and_exp.get(), this);
  } else {
    l_or_exp->print(os);
    if (IRManager::getInstance().is_in_if) {
      int tmp_reg = IRManager::getInstance().getNextReg();
      std::string next_pos =
          "%end_" + std::to_string(IRManager::getInstance().getNowIfCount());
      if (IRManager::getInstance().is_in_if_else) {
        next_pos =
            "%else_" + std::to_string(IRManager::getInstance().getNowIfCount());
      }
      os << "  br " << get_koopa_exp_reg(l_or_exp.get()) << ", %continue_"
         << tmp_reg << ", " << next_pos << "\n";
      os << "%continue_" << tmp_reg << ":\n";
    }
    l_and_exp->print(os);
    reg = IRManager::getInstance().getNextReg();
    os << "  %" << reg << " = or " << get_koopa_logical_exp_reg(l_or_exp.get())
       << ", " << get_koopa_logical_exp_reg(l_and_exp.get()) << "\n";
    reg = IRManager::getInstance().getNextReg();
    os << "  %" << reg << " = gt %" << reg - 1 << ", 0\n";
  }
}

void LAndExpAST::print(std::ostream &os) {
  if (kind == ExpAST::Kind::EQ_EXP) {
    eq_exp->print(os);
    pushup_exp_reg(eq_exp.get(), this);
  } else {
    l_and_exp->print(os);
    if (IRManager::getInstance().is_in_if) {
      int tmp_reg = IRManager::getInstance().getNextReg();
      std::string next_pos =
          "%end_" + std::to_string(IRManager::getInstance().getNowIfCount());
      if (IRManager::getInstance().is_in_if_else) {
        next_pos =
            "%else_" + std::to_string(IRManager::getInstance().getNowIfCount());
      }
      os << "  %" << tmp_reg << " = ne " << get_koopa_exp_reg(l_and_exp.get())
         << ", 0\n";
      os << "  br %" << tmp_reg << ", %continue_" << tmp_reg << ", " << next_pos
         << "\n";
      os << "%continue_" << tmp_reg << ":\n";
    }
    eq_exp->print(os);
    reg = IRManager::getInstance().getNextReg();
    os << "  %" << reg << " = and "
       << get_koopa_logical_exp_reg(l_and_exp.get()) << ", "
       << get_koopa_logical_exp_reg(eq_exp.get()) << "\n";
    reg = IRManager::getInstance().getNextReg();
    os << "  %" << reg << " = gt %" << reg - 1 << ", 0\n";
  }
}

void EqExpAST::print(std::ostream &os) {
  if (kind == ExpAST::Kind::REL_EXP) {
    rel_exp->print(os);
    pushup_exp_reg(rel_exp.get(), this);
  } else {
    eq_exp->print(os);
    rel_exp->print(os);
    reg = IRManager::getInstance().getNextReg();
    const char *op = logical_op == LogicalOpKind::Equal ? "eq" : "ne";
    os << "  %" << reg << " = " << op << " " << get_koopa_exp_reg(eq_exp.get())
       << ", " << get_koopa_exp_reg(rel_exp.get()) << "\n";
  }
}

void RelExpAST::print(std::ostream &os) {
  if (kind == ExpAST::Kind::ADD_EXP) {
    add_exp->print(os);
    pushup_exp_reg(add_exp.get(), this);
  } else {
    rel_exp->print(os);
    add_exp->print(os);
    reg = IRManager::getInstance().getNextReg();
    const char *op = nullptr;
    switch (logical_op) {
    case LogicalOpKind::Greater:
      op = "gt";
      break;
    case LogicalOpKind::Less:
      op = "lt";
      break;
    case LogicalOpKind::GreaterEqual:
      op = "ge";
      break;
    case LogicalOpKind::LessEqual:
      op = "le";
      break;
    default:
      break;
    }
    if (op)
      os << "  %" << reg << " = " << op << " "
         << get_koopa_exp_reg(rel_exp.get()) << ", "
         << get_koopa_exp_reg(add_exp.get()) << "\n";
  }
}

namespace {
template <typename Node, typename Fn>
int calc_number_impl(Node &node, Fn compute) {
  if (node.kind != ExpAST::Kind::NUMBER) {
    node.number = compute();
    node.kind = ExpAST::Kind::NUMBER;
  }
  return node.number;
}
} // namespace

int ExpAST::calc_number() {
  return calc_number_impl(*this, [&]() {
    if (kind == ExpAST::Kind::L_OR_EXP) {
      return l_or_exp->calc_number();
    }
    assert(false);
    return 0;
  });
}

int PrimaryExpAST::calc_number() {
  return calc_number_impl(*this, [&]() {
    if (kind == Kind::EXP) {
      return exp->calc_number();
    }
    if (kind == Kind::L_VAL) {
      return SymbolTableManger::getInstance().get_back_table().val_map[l_val];
    }
    assert(false);
    return 0;
  });
}

int UnaryExpAST::calc_number() {
  return calc_number_impl(*this, [&]() {
    if (kind == ExpAST::Kind::PRIMARY_EXP) {
      return primary_exp->calc_number();
    }
    if (unary_op == UnaryOpKind::Minus) {
      return -unary_exp->calc_number();
    }
    if (unary_op == UnaryOpKind::Plus) {
      return unary_exp->calc_number();
    }
    assert(false);
    return 0;
  });
}

int MulExpAST::calc_number() {
  return calc_number_impl(*this, [&]() {
    if (kind == ExpAST::Kind::UNARY_EXP) {
      return unary_exp->calc_number();
    }
    if (mul_op == MulOpKind::Mul) {
      return mul_exp->calc_number() * unary_exp->calc_number();
    }
    if (mul_op == MulOpKind::Div) {
      return mul_exp->calc_number() / unary_exp->calc_number();
    }
    if (mul_op == MulOpKind::Mod) {
      return mul_exp->calc_number() % unary_exp->calc_number();
    }
    assert(false);
    return 0;
  });
}

int AddExpAST::calc_number() {
  return calc_number_impl(*this, [&]() {
    if (kind == ExpAST::Kind::MUL_EXP) {
      return mul_exp->calc_number();
    }
    if (add_op == AddOpKind::Plus) {
      return add_exp->calc_number() + mul_exp->calc_number();
    }
    if (add_op == AddOpKind::Minus) {
      return add_exp->calc_number() - mul_exp->calc_number();
    }
    assert(false);
    return 0;
  });
}

int RelExpAST::calc_number() {
  return calc_number_impl(*this, [&]() {
    if (kind == ExpAST::Kind::ADD_EXP) {
      return add_exp->calc_number();
    }
    if (logical_op == LogicalOpKind::Greater) {
      return rel_exp->calc_number() > add_exp->calc_number() ? 1 : 0;
    }
    if (logical_op == LogicalOpKind::Less) {
      return rel_exp->calc_number() < add_exp->calc_number() ? 1 : 0;
    }
    if (logical_op == LogicalOpKind::GreaterEqual) {
      return rel_exp->calc_number() >= add_exp->calc_number() ? 1 : 0;
    }
    if (logical_op == LogicalOpKind::LessEqual) {
      return rel_exp->calc_number() <= add_exp->calc_number() ? 1 : 0;
    }
    assert(false);
    return 0;
  });
}

int EqExpAST::calc_number() {
  return calc_number_impl(*this, [&]() {
    if (kind == ExpAST::Kind::REL_EXP) {
      return rel_exp->calc_number();
    }
    if (logical_op == LogicalOpKind::Equal) {
      return rel_exp->calc_number() == eq_exp->calc_number() ? 1 : 0;
    }
    if (logical_op == LogicalOpKind::NotEqual) {
      return rel_exp->calc_number() != eq_exp->calc_number() ? 1 : 0;
    }
    assert(false);
    return 0;
  });
}

int LAndExpAST::calc_number() {
  return calc_number_impl(*this, [&]() {
    if (kind == ExpAST::Kind::EQ_EXP) {
      return eq_exp->calc_number();
    }
    if (kind == ExpAST::Kind::L_AND_EXP) {
      return l_and_exp->calc_number() && eq_exp->calc_number() ? 1 : 0;
    }
    assert(false);
    return 0;
  });
}

int LOrExpAST::calc_number() {
  return calc_number_impl(*this, [&]() {
    if (kind == ExpAST::Kind::L_AND_EXP) {
      return l_and_exp->calc_number();
    }
    if (kind == ExpAST::Kind::L_OR_EXP) {
      return l_or_exp->calc_number() || l_and_exp->calc_number() ? 1 : 0;
    }
    assert(false);
    return 0;
  });
}