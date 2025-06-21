#pragma once
#include <map>
#include <string>
#include <vector>
#include <cassert>

class BaseAST;
class ExpAST;

class SymbolTable {
public:
  enum class DefType { CONST, VAR_IDENT, VAR_EXP };
  static SymbolTable &getInstance();

  std::map<std::string, int> val_map;
  std::map<std::string, std::string> type_map;
  std::map<std::string, DefType> def_type_map;
  std::map<std::string, ExpAST *> exp_val_map;

  std::map<std::string, std::string> lval_ident_map;
  std::map<std::string, bool> is_ident_alloc_map;
  std::map<std::string, int> ident_var_map;

  std::string get_var_ident(const std::string ident);
  bool is_var_defined(const std::string &ident);
};

class SymbolTableManger {
public:
  static SymbolTableManger &getInstance();

  ExpAST *get_exp(const std::string &ident);
  int get_val(const std::string &ident);
  SymbolTable::DefType get_def_type(const std::string &ident);
  std::string get_ident(const std::string &ident);
  void alloc_ident(const std::string &ident);
  std::string get_lval_ident(const std::string &ident);
  void push_symbol_table();
  void alloc_stmt_table(BaseAST *stmt);
  void use_stmt_table(BaseAST *stmt);
  void pop_symbol_table();
  bool is_var_defined(const std::string &ident);
  SymbolTable &get_stmt_table(BaseAST *stmt);
  SymbolTable &get_back_table();

private:
  SymbolTableManger() = default;
  SymbolTable *find_table(const std::string &ident);

  std::vector<SymbolTable> symbol_table_stack;
  std::map<BaseAST *, SymbolTable> stmt_table_map;
  std::map<std::string, int> ident_count_map;
};
