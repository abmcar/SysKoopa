#pragma once
#include <ostream>
#ifndef AST_H
#define AST_H

#include <iostream>
#include <memory>
#include <string>
#include <vector>

#include "util.h"

enum class UnaryOpKind { Plus, Minus, Not };
enum class AddOpKind { Plus, Minus };
enum class MulOpKind { Mul, Div, Mod };
enum class LogicalOpKind {
  Or,
  And,
  Equal,
  NotEqual,
  Greater,
  Less,
  GreaterEqual,
  LessEqual
};


class BaseAST;
class ExpAST;
class DefAST;

#include "symbol_table.h"

class IRGenerator {
public:
  static IRGenerator &getInstance() {
    static IRGenerator instance;
    return instance;
  }

  int getNextReg() { return reg_counter++; }
  int getNextIfCount() { return if_counter++; }
  void reset() { reg_counter = 0; }
  std::map<std::streampos, bool> is_return_map;

private:
  IRGenerator() : reg_counter(0), if_counter(0) {}
  int reg_counter;
  int if_counter;
};

class BaseAST {
public:
  virtual ~BaseAST() = default;
  virtual void Dump() const = 0;
  virtual void print(std::ostream &os) {};
  friend std::ostream &operator<<(std::ostream &os, BaseAST &ast);
};

class CompUnitAST : public BaseAST {
public:
  std::unique_ptr<BaseAST> func_def;
  void Dump() const override;
  void print(std::ostream &os) override;
};

class FuncDefAST : public BaseAST {
public:
  std::unique_ptr<BaseAST> func_type;
  std::string ident;
  std::unique_ptr<BaseAST> block;
  void Dump() const override;
  void print(std::ostream &os) override;
};

class FuncTypeAST : public BaseAST {
public:
  std::string func_type;
  void Dump() const override;
  void print(std::ostream &os) override;
};

class DeclAST : public BaseAST {
public:
  enum Kind { CONST_DECL, VAR_DECL };
  Kind kind;
  std::unique_ptr<BaseAST> const_decl;
  std::unique_ptr<BaseAST> var_decl;
  void Dump() const override;
  void print(std::ostream &os) override;
};

class ConstDeclAST : public BaseAST {
public:
  std::string b_type;
  std::vector<std::unique_ptr<DefAST>> *const_def_list;
  void Dump() const override;
  void print(std::ostream &os) override;
};

class BTypeAST : public BaseAST {
public:
  std::string b_type;
  void Dump() const override;
  void print(std::ostream &os) override;
};

class DefAST : public BaseAST {
public:
  enum Kind { CONST_DEF, VAR_DEF, VAR_IDENT };
  Kind kind;
  std::string ident;
  void Dump() const override {};
  void print(std::ostream &os) override {};
};

class VarDeclAST : public BaseAST {
public:
  std::string b_type;
  std::vector<std::unique_ptr<DefAST>> *var_def_list;
  void Dump() const override;
  void print(std::ostream &os) override;
};

class VarDefAST : public DefAST {
public:
  std::unique_ptr<ExpAST> init_val;
  void Dump() const override;
  void print(std::ostream &os) override;
};

class ConstDefAST : public DefAST {
public:
  int const_init_val;
  void Dump() const override;
  void print(std::ostream &os) override;
};

class BlockAST : public BaseAST {
public:
  std::vector<std::unique_ptr<BaseAST>> *block_item_list;
  void Dump() const override;
  void print(std::ostream &os) override;
};

class BlockItemAST : public BaseAST {
public:
  enum Kind { DECL, STMT };
  Kind kind;
  std::unique_ptr<BaseAST> decl;
  std::unique_ptr<BaseAST> stmt;
  void Dump() const override;
  void print(std::ostream &os) override;
};

class ExpAST;

class StmtAST : public BaseAST {
public:
  enum Kind { RETURN_STMT, ASSIGN_STMT, BLOCK_STMT, EXP_STMT, EMPTY_STMT, IF_STMT, IF_ELSE_STMT };
  Kind kind;
  std::unique_ptr<ExpAST> exp;
  std::string l_val;
  std::unique_ptr<ExpAST> r_exp;
  std::unique_ptr<BaseAST> block;
  std::unique_ptr<ExpAST> if_exp;
  std::unique_ptr<BaseAST> if_stmt;
  std::unique_ptr<BaseAST> else_stmt;
  void Dump() const override;
  void print(std::ostream &os) override;
};

class ExpAST : public BaseAST {
public:
  enum Kind {
    EXP,
    NUMBER,
    PRIMARY_EXP,
    UNARY_OP_EXP,
    REL_EXP,
    ADD_EXP,
    EQ_EXP,
    L_AND_EXP,
    L_OR_EXP,
    MUL_EXP,
    UNARY_EXP,
    L_VAL
  };
  Kind kind;
  int number;
  std::unique_ptr<ExpAST> l_or_exp;
  void Dump() const override;
  void print(std::ostream &os) override;
  int get_reg() const { return reg; }
  bool is_number() const { return kind == Kind::NUMBER; }
  int get_number() const { return number; }
  void set_reg(int reg) { this->reg = reg; }
  virtual int calc_number();

protected:
  int reg = -1;
};

class PrimaryExpAST : public ExpAST {
public:
  std::unique_ptr<ExpAST> exp;
  std::string l_val;
  void Dump() const override;
  void print(std::ostream &os) override;
  int calc_number() override;
};

class UnaryExpAST : public ExpAST {
public:
  std::unique_ptr<ExpAST> primary_exp, unary_exp;
  UnaryOpKind unary_op;
  void Dump() const override;
  void print(std::ostream &os) override;
  int calc_number() override;
};

class AddExpAST : public ExpAST {
public:
  std::unique_ptr<ExpAST> add_exp, mul_exp;
  AddOpKind add_op;
  void Dump() const override;
  void print(std::ostream &os) override;
  int calc_number() override;
};

class MulExpAST : public ExpAST {
public:
  std::unique_ptr<ExpAST> mul_exp, unary_exp;
  MulOpKind mul_op;
  void Dump() const override;
  void print(std::ostream &os) override;
  int calc_number() override;
};

class LOrExpAST : public ExpAST {
public:
  std::unique_ptr<ExpAST> l_or_exp, l_and_exp;
  LogicalOpKind logical_op;
  void Dump() const override;
  void print(std::ostream &os) override;
  int calc_number() override;
};

class LAndExpAST : public ExpAST {
public:
  std::unique_ptr<ExpAST> l_and_exp, eq_exp;
  LogicalOpKind logical_op;
  void Dump() const override;
  void print(std::ostream &os) override;
  int calc_number() override;
};

class EqExpAST : public ExpAST {
public:
  std::unique_ptr<ExpAST> eq_exp, rel_exp;
  LogicalOpKind logical_op;
  void Dump() const override;
  void print(std::ostream &os) override;
  int calc_number() override;
};

class RelExpAST : public ExpAST {
public:
  std::unique_ptr<ExpAST> rel_exp, add_exp;
  LogicalOpKind logical_op;
  void Dump() const override;
  void print(std::ostream &os) override;
  int calc_number() override;
};

#endif // AST_H