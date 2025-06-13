#pragma once
#ifndef AST_H
#define AST_H

#include <iostream>
#include <memory>
#include <string>

enum class UnaryOpKind { Plus, Minus, Not };

class BaseAST;
class CompUnitAST;
class FuncDefAST;
class FuncTypeAST;
class BlockAST;
class StmtAST;
class ExpAST;
class UnaryExpAST;
class PrimaryExpAST;

class IRGenerator {
public:
  static IRGenerator &getInstance() {
    static IRGenerator instance;
    return instance;
  }

  int getNextReg() { return reg_counter++; }
  void reset() { reg_counter = 0; }

private:
  IRGenerator() : reg_counter(0) {}
  int reg_counter;
};

class BaseAST {
public:
  virtual ~BaseAST() = default;
  virtual void Dump() const = 0;
  virtual void print(std::ostream &os) {};
  virtual int get_reg() const { return -1; }
  virtual bool is_number() const { return false; }
  virtual int get_number() const { return -1; }
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

class BlockAST : public BaseAST {
public:
  std::unique_ptr<BaseAST> stmt;
  void Dump() const override;
  void print(std::ostream &os) override;
};

class StmtAST : public BaseAST {
public:
  std::unique_ptr<BaseAST> exp;
  void Dump() const override;
  void print(std::ostream &os) override;
};

class ExpAST : public BaseAST {
public:
  enum Kind { EXP, NUMBER };
  Kind kind;
  int number;
  std::unique_ptr<BaseAST> unary_exp;
  void Dump() const override;
  void print(std::ostream &os) override;
  int get_reg() const override { return reg; }
  bool is_number() const override { return kind == Kind::NUMBER; }
  int get_number() const override { return number; }

protected:
  int reg = -1;
};

class PrimaryExpAST : public BaseAST {
public:
  enum Kind { EXP, NUMBER };
  Kind kind;
  std::unique_ptr<BaseAST> exp;
  int number;
  void Dump() const override;
  void print(std::ostream &os) override;
  int get_reg() const override { return reg; }
  bool is_number() const override { return kind == Kind::NUMBER; }
  int get_number() const override { return number; }

protected:
  int reg = -1;
};

class UnaryExpAST : public BaseAST {
public:
  enum Kind { PRIMARY_EXP, UNARY_OP_EXP, NUMBER };
  Kind kind;
  std::unique_ptr<BaseAST> primary_exp, unary_exp;
  int number;
  UnaryOpKind unary_op;
  void Dump() const override;
  void print(std::ostream &os) override;
  int get_reg() const override { return reg; }
  bool is_number() const override { return kind == Kind::NUMBER; }
  int get_number() const override { return number; }

protected:
  int reg = -1;
};

#endif // AST_H