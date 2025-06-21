#include "symbol_table.h"
#include "ast.h"

SymbolTable &SymbolTable::getInstance() {
  static SymbolTable instance;
  return instance;
}

std::string SymbolTable::get_var_ident(const std::string ident) {
  ident_var_map[ident]++;
  std::string var_ident = "%" + ident + std::to_string(ident_var_map[ident]);
  return var_ident;
}

bool SymbolTable::is_var_defined(const std::string &ident) {
  if (def_type_map.find(ident) == def_type_map.end()) {
    return false;
  }
  if (def_type_map[ident] == DefType::VAR_EXP ||
      def_type_map[ident] == DefType::VAR_IDENT) {
    return true;
  } else {
    return false;
  }
}

SymbolTableManger &SymbolTableManger::getInstance() {
  static SymbolTableManger instance;
  return instance;
}

ExpAST *SymbolTableManger::get_exp(const std::string &ident) {
  for (auto table = symbol_table_stack.rbegin(); table != symbol_table_stack.rend(); table++) {
    if (table->is_ident_alloc_map[ident]) {
      return table->exp_val_map[ident];
    }
  }
  assert(false);
}

int SymbolTableManger::get_val(const std::string &ident) {
  for (auto table = symbol_table_stack.rbegin(); table != symbol_table_stack.rend(); table++) {
    if (table->is_ident_alloc_map[ident]) {
      return table->val_map[ident];
    }
  }
  assert(false);
}

SymbolTable::DefType SymbolTableManger::get_def_type(const std::string &ident) {
  for (auto table = symbol_table_stack.rbegin(); table != symbol_table_stack.rend(); table++) {
    if (table->is_ident_alloc_map[ident]) {
      return table->def_type_map[ident];
    }
  }
  assert(false);
}

std::string SymbolTableManger::get_ident(const std::string &ident) {
  for (auto table = symbol_table_stack.rbegin(); table != symbol_table_stack.rend(); table++) {
    if (table->is_ident_alloc_map[ident]) {
      return table->lval_ident_map[ident];
    }
  }
  assert(false);
}

void SymbolTableManger::alloc_ident(const std::string &ident) {
  get_back_table().is_ident_alloc_map[ident] = true;
}

std::string SymbolTableManger::get_lval_ident(const std::string &ident) {
  return ident + std::to_string(ident_count_map[ident]++);
}

void SymbolTableManger::push_symbol_table() {
  symbol_table_stack.push_back(SymbolTable());
}

void SymbolTableManger::alloc_stmt_table(BaseAST *stmt) {
  stmt_table_map[stmt] = symbol_table_stack.back();
}

void SymbolTableManger::use_stmt_table(BaseAST *stmt) {
  symbol_table_stack.push_back(stmt_table_map[stmt]);
}

void SymbolTableManger::pop_symbol_table() {
  symbol_table_stack.pop_back();
}

bool SymbolTableManger::is_var_defined(const std::string &ident) {
  for (auto symbol_table = symbol_table_stack.rbegin();
       symbol_table != symbol_table_stack.rend(); symbol_table++) {
    if (symbol_table->is_var_defined(ident)) {
      return true;
    }
  }
  return false;
}

SymbolTable &SymbolTableManger::get_stmt_table(BaseAST *stmt) {
  return stmt_table_map[stmt];
}

SymbolTable &SymbolTableManger::get_back_table() {
  return symbol_table_stack.back();
}
