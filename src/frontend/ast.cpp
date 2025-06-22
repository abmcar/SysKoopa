#include "ast.h"

void CompUnitAST::Dump() const {
  std::cout << "CompUnitAST { ";
  func_def->Dump();
  std::cout << " }\n";
}

void FuncDefAST::Dump() const {
  std::cout << "FuncDefAST { ";
  func_type->Dump();
  std::cout << ", " << ident << ", ";
  block->Dump();
  std::cout << " }";
}

void FuncTypeAST::Dump() const {
  std::cout << "FuncTypeAst { " << func_type << " }";
}

void DeclAST::Dump() const {
  std::cout << "DeclAST { ";
  if (kind == DeclAST::Kind::CONST_DECL) {
    const_decl->Dump();
  } else if (kind == DeclAST::Kind::VAR_DECL) {
    var_decl->Dump();
  }
  std::cout << " }";
}

void ConstDeclAST::Dump() const {
  std::cout << "ConstDeclAST { ";
  for (auto &item : *const_def_list) {
    item->Dump();
  }
  std::cout << " }";
}

void BTypeAST::Dump() const { std::cout << "BTypeAST { " << b_type << " }"; }

void VarDeclAST::Dump() const {
  std::cout << "VarDeclAST { ";
  for (auto &item : *var_def_list) {
    item->Dump();
  }
  std::cout << " }";
}

void VarDefAST::Dump() const {
  std::cout << "VarDefAST { " << ident;
  if (kind == DefAST::Kind::VAR_DEF) {
    std::cout << " = ";
    init_val->Dump();
  }
  std::cout << " }";
}

void ConstDefAST::Dump() const {
  std::cout << "ConstDefAST { " << ident << " = " << const_init_val << " }";
}

void BlockAST::Dump() const {
  std::cout << "BlockAST { ";
  if (block_item_list == nullptr) {
    std::cout << "EMPTY_BLOCK";
    return;
  }
  for (auto &item : *block_item_list) {
    item->Dump();
  }
  std::cout << " }";
}

void BlockItemAST::Dump() const {
  std::cout << "BlockItemAST { ";
  if (kind == BlockItemAST::Kind::DECL) {
    decl->Dump();
  } else if (kind == BlockItemAST::Kind::STMT) {
    stmt->Dump();
  }
  std::cout << " }";
}

void StmtAST::Dump() const {
  std::cout << "StmtAST { ";
  if (kind == StmtAST::Kind::RETURN_STMT) {
    std::cout << "RETURN_STMT, ";
    exp->Dump();
  } else if (kind == StmtAST::Kind::ASSIGN_STMT) {
    r_exp->Dump();
  } else if (kind == StmtAST::Kind::BLOCK_STMT) {
    std::cout << "BLOCK_STMT, ";
    block->Dump();
  } else if (kind == StmtAST::Kind::EXP_STMT) {
    std::cout << "EXP_STMT, ";
    exp->Dump();
  } else if (kind == StmtAST::Kind::EMPTY_STMT) {
    std::cout << "EMPTY_STMT";
  } else if (kind == StmtAST::Kind::IF_STMT) {
    std::cout << "IF_STMT, ";
    if_exp->Dump();
    if_stmt->Dump();
  } else if (kind == StmtAST::Kind::IF_ELSE_STMT) {
    std::cout << "IF_ELSE_STMT, ";
    if_exp->Dump();
    if_stmt->Dump();
    else_stmt->Dump();
  } else if (kind == StmtAST::Kind::WHILE_STMT) {
    std::cout << "WHILE_STMT, ";
    while_exp->Dump();
    while_stmt->Dump();
  }
  std::cout << " }";
}

void ExpAST::Dump() const {
  std::cout << "ExpAST { ";
  l_or_exp->Dump();
  std::cout << " }";
}

void PrimaryExpAST::Dump() const {
  std::cout << "PrimaryExpAST { ";
  if (kind == ExpAST::Kind::EXP) {
    std::cout << "EXP, ";
    exp->Dump();
  } else if (kind == ExpAST::Kind::NUMBER) {
    std::cout << "NUMBER, " << number;
  } else if (kind == ExpAST::Kind::L_VAL) {
    std::cout << "L_VAL, " << l_val;
  }
  std::cout << " }";
}
void UnaryExpAST::Dump() const {
  std::cout << "UnaryExpAST { ";
  if (kind == UnaryExpAST::Kind::PRIMARY_EXP) {
    primary_exp->Dump();
  } else {
    std::cout << "UNARY_OP_EXP, ";
    if (unary_op == UnaryOpKind::Plus) {
      std::cout << "+, ";
    } else if (unary_op == UnaryOpKind::Minus) {
      std::cout << "-, ";
    } else if (unary_op == UnaryOpKind::Not) {
      std::cout << "!, ";
    }
    unary_exp->Dump();
  }
  std::cout << " }";
}

void AddExpAST::Dump() const {
  std::cout << "AddExpAST { ";
  if (add_exp == nullptr) {
    mul_exp->Dump();
  } else {
    add_exp->Dump();
    if (add_op == AddOpKind::Plus) {
      std::cout << "+, ";
    } else if (add_op == AddOpKind::Minus) {
      std::cout << "-, ";
    }
    mul_exp->Dump();
  }
  std::cout << " }";
}

void MulExpAST::Dump() const {
  std::cout << "MulExpAST { ";
  if (mul_exp == nullptr) {
    unary_exp->Dump();
  } else {
    mul_exp->Dump();
    if (mul_op == MulOpKind::Mul) {
      std::cout << "*, ";
    } else if (mul_op == MulOpKind::Div) {
      std::cout << "/, ";
    } else if (mul_op == MulOpKind::Mod) {
      std::cout << "%, ";
    }
    unary_exp->Dump();
  }
  std::cout << " }";
}

void LOrExpAST::Dump() const {
  std::cout << "LOrExpAST { ";
  if (l_or_exp == nullptr) {
    l_and_exp->Dump();
  } else {
    l_or_exp->Dump();
    std::cout << "||, ";
    l_and_exp->Dump();
  }
  std::cout << " }";
}

void LAndExpAST::Dump() const {
  std::cout << "LAndExpAST { ";
  if (l_and_exp == nullptr) {
    eq_exp->Dump();
  } else {
    l_and_exp->Dump();
    std::cout << "&&, ";
    eq_exp->Dump();
  }
  std::cout << " }";
}

void EqExpAST::Dump() const {
  std::cout << "EqExpAST { ";
  if (eq_exp == nullptr) {
    rel_exp->Dump();
  } else {
    eq_exp->Dump();
    if (logical_op == LogicalOpKind::Equal) {
      std::cout << "==, ";
    } else if (logical_op == LogicalOpKind::NotEqual) {
      std::cout << "!=, ";
    }
    rel_exp->Dump();
  }
  std::cout << " }";
}

void RelExpAST::Dump() const {
  std::cout << "RelExpAST { ";
  if (rel_exp == nullptr) {
    add_exp->Dump();
  } else {
    rel_exp->Dump();
    if (logical_op == LogicalOpKind::Greater) {
      std::cout << ">, ";
    } else if (logical_op == LogicalOpKind::Less) {
      std::cout << "<, ";
    } else if (logical_op == LogicalOpKind::GreaterEqual) {
      std::cout << ">=, ";
    } else if (logical_op == LogicalOpKind::LessEqual) {
      std::cout << "<=, ";
    }
    add_exp->Dump();
  }
  std::cout << " }";
}

std::ostream &operator<<(std::ostream &os, BaseAST &ast) {
  ast.print(os);
  return os;
};

void CompUnitAST::print(std::ostream &os) { func_def->print(os); }

void FuncDefAST::print(std::ostream &os) {
  os << "fun @" << this->ident << "("
     << ")"
     << ": ";
  func_type->print(os);
  os << " {\n";
  os << "%"
     << "entry"
     << ":\n";
  block->print(os);
  os << "}";
}

void FuncTypeAST::print(std::ostream &os) {
  if (func_type == "int") {
    os << "i32";
  }
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
  SymbolTableManger::getInstance().use_stmt_table(this);
  for (auto &item : *block_item_list) {
    item->print(os);
  }
  SymbolTableManger::getInstance().pop_symbol_table();
}

void BlockItemAST::print(std::ostream &os) {
  if (kind == BlockItemAST::Kind::DECL) {
    decl->print(os);
  } else if (kind == BlockItemAST::Kind::STMT) {
    stmt->print(os);
  }
}

void StmtAST::print(std::ostream &os) {
  if (kind == StmtAST::Kind::RETURN_STMT) {
    if (IRManager::getInstance().is_return_map[os.tellp()]) {
      return;
    }
    exp->print(os);
    if (exp != nullptr) {
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

    if (add_op == AddOpKind::Plus) {
      os << "  %" << reg << " = add " << get_koopa_exp_reg(add_exp.get())
         << ", " << get_koopa_exp_reg(mul_exp.get()) << "\n";
    } else if (add_op == AddOpKind::Minus) {
      os << "  %" << reg << " = sub " << get_koopa_exp_reg(add_exp.get())
         << ", " << get_koopa_exp_reg(mul_exp.get()) << "\n";
    }
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
    if (mul_op == MulOpKind::Mul) {
      os << "  %" << reg << " = mul " << get_koopa_exp_reg(mul_exp.get())
         << ", " << get_koopa_exp_reg(unary_exp.get()) << "\n";
    } else if (mul_op == MulOpKind::Div) {
      os << "  %" << reg << " = div " << get_koopa_exp_reg(mul_exp.get())
         << ", " << get_koopa_exp_reg(unary_exp.get()) << "\n";
    } else if (mul_op == MulOpKind::Mod) {
      os << "  %" << reg << " = mod " << get_koopa_exp_reg(mul_exp.get())
         << ", " << get_koopa_exp_reg(unary_exp.get()) << "\n";
    }
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
    if (logical_op == LogicalOpKind::Equal) {
      os << "  %" << reg << " = eq " << get_koopa_exp_reg(eq_exp.get()) << ", "
         << get_koopa_exp_reg(rel_exp.get()) << "\n";
    } else if (logical_op == LogicalOpKind::NotEqual) {
      os << "  %" << reg << " = ne " << get_koopa_exp_reg(eq_exp.get()) << ", "
         << get_koopa_exp_reg(rel_exp.get()) << "\n";
    }
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
    if (logical_op == LogicalOpKind::Greater) {
      os << "  %" << reg << " = gt " << get_koopa_exp_reg(rel_exp.get()) << ", "
         << get_koopa_exp_reg(add_exp.get()) << "\n";
    } else if (logical_op == LogicalOpKind::Less) {
      os << "  %" << reg << " = lt " << get_koopa_exp_reg(rel_exp.get()) << ", "
         << get_koopa_exp_reg(add_exp.get()) << "\n";
    } else if (logical_op == LogicalOpKind::GreaterEqual) {
      os << "  %" << reg << " = ge " << get_koopa_exp_reg(rel_exp.get()) << ", "
         << get_koopa_exp_reg(add_exp.get()) << "\n";
    } else if (logical_op == LogicalOpKind::LessEqual) {
      os << "  %" << reg << " = le " << get_koopa_exp_reg(rel_exp.get()) << ", "
         << get_koopa_exp_reg(add_exp.get()) << "\n";
    }
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