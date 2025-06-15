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

void BlockAST::Dump() const {
  std::cout << "BlockAST { ";
  stmt->Dump();
  std::cout << " }";
}

void StmtAST::Dump() const {
  std::cout << "StmtAST { ";
  exp->Dump();
  std::cout << " }";
}

void ExpAST::Dump() const {
  std::cout << "ExpAST { ";
  add_exp->Dump();
  std::cout << " }";
}

void PrimaryExpAST::Dump() const {
  std::cout << "PrimaryExpAST { ";
  if (kind == PrimaryExpAST::Kind::EXP) {
    std::cout << "EXP, ";
    exp->Dump();
  } else if (kind == PrimaryExpAST::Kind::NUMBER) {
    std::cout << "NUMBER, " << number;
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
  if (add_op == AddOpKind::Mul) {
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
  if (mul_op == MulOpKind::Unary) {
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

std::ostream &operator<<(std::ostream &os, BaseAST &ast) {
  ast.print(os);
  return os;
};

void CompUnitAST::print(std::ostream &os) { func_def->print(os); }

void FuncDefAST::print(std::ostream &os) {
  os << "fun @" << this->ident << "(" << ")" << ": ";
  func_type->print(os);
  os << " {\n";
  block->print(os);
  os << "}";
}

void FuncTypeAST::print(std::ostream &os) {
  if (func_type == "int") {
    os << "i32";
  }
}

void BlockAST::print(std::ostream &os) {
  os << "%" << "entry" << ":\n";
  stmt->print(os);
}

void StmtAST::print(std::ostream &os) {
  exp->print(os);
  os << "  ret " << get_koopa_exp_reg(exp.get()) << "\n";
}

void ExpAST::print(std::ostream &os) {
  // unary_exp->print(os);
  // if (unary_exp->is_number()) {
  //   number = unary_exp->get_number();
  //   kind = ExpAST::Kind::NUMBER;
  // } else {
  //   reg = unary_exp->get_reg();
  // }
  add_exp->print(os);
  if (add_exp->is_number()) {
    number = add_exp->get_number();
    kind = ExpAST::Kind::NUMBER;
  } else {
    reg = add_exp->get_reg();
  }
}

void PrimaryExpAST::print(std::ostream &os) {
  if (kind == Kind::EXP) {
    exp->print(os);
    reg = exp->get_reg();
  } else if (kind == Kind::NUMBER) {
  }
}

void UnaryExpAST::print(std::ostream &os) {
  if (kind == UnaryExpAST::Kind::PRIMARY_EXP) {
    primary_exp->print(os);
    if (primary_exp->is_number()) {
      number = primary_exp->get_number();
      kind = UnaryExpAST::Kind::NUMBER;
    } else {
      reg = primary_exp->get_reg();
    }
  } else if (kind == UnaryExpAST::Kind::UNARY_OP_EXP) {
    unary_exp->print(os);
    int unary_exp_reg = unary_exp->get_reg();

    switch (unary_op) {
    case UnaryOpKind::Plus:
      if (unary_exp->is_number()) {
        number = unary_exp->get_number();
        kind = UnaryExpAST::Kind::NUMBER;
      } else {
        reg = unary_exp_reg;
      }
      break;
    case UnaryOpKind::Minus:
      reg = IRGenerator::getInstance().getNextReg();
      os << "  %" << reg << " = sub 0, " << get_koopa_exp_reg(unary_exp.get())
         << "\n";
      break;
    case UnaryOpKind::Not:
      reg = IRGenerator::getInstance().getNextReg();
      os << "  %" << reg << " = eq " << get_koopa_exp_reg(unary_exp.get())
         << ", 0\n";
      break;
    }
  }
}

void AddExpAST::print(std::ostream &os) {
  if (add_op == AddOpKind::Mul) {
    mul_exp->print(os);
    if (mul_exp->is_number()) {
      number = mul_exp->get_number();
      kind = ExpAST::Kind::NUMBER;
    } else {
      reg = mul_exp->get_reg();
    }
  } else {
    mul_exp->print(os);
    add_exp->print(os);
    reg = IRGenerator::getInstance().getNextReg();

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
  if (mul_op == MulOpKind::Unary) {
    unary_exp->print(os);
    if (unary_exp->is_number()) {
      number = unary_exp->get_number();
      kind = ExpAST::Kind::NUMBER;
    } else {
      reg = unary_exp->get_reg();
    }
  } else {
    mul_exp->print(os);
    unary_exp->print(os);
    reg = IRGenerator::getInstance().getNextReg();
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