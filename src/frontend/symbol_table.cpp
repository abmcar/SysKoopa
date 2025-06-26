#include "symbol_table.h"
#include "ast.h"

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

SymbolTable *SymbolTableManger::find_table(const std::string &ident) {
  for (auto table = symbol_table_stack.rbegin();
       table != symbol_table_stack.rend(); ++table) {
    if ((*table)->is_ident_alloc_map[ident]) {
      return *table;
    }
  }
  return nullptr;
}

int SymbolTableManger::get_val(const std::string &ident) {
  auto table = find_table(ident);
  assert(table);
  return table->val_map[ident];
}

SymbolTable::DefType SymbolTableManger::get_def_type(const std::string &ident) {
  auto table = find_table(ident);
  assert(table);
  return table->def_type_map[ident];
}

std::string SymbolTableManger::get_ident(const std::string &ident) {
  auto table = find_table(ident);
  assert(table);
  return table->lval_ident_map[ident];
}

void SymbolTableManger::alloc_ident(const std::string &ident) {
  get_back_table().is_ident_alloc_map[ident] = true;
}

std::string SymbolTableManger::get_lval_ident(const std::string &ident) {
  return ident + std::to_string(ident_count_map[ident]++);
}

void SymbolTableManger::push_symbol_table() {
  symbol_table_stack.push_back(new SymbolTable());
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
  for (auto table = symbol_table_stack.rbegin();
       table != symbol_table_stack.rend(); ++table) {
    if ((*table)->is_var_defined(ident)) {
      return true;
    }
  }
  return false;
}

void SymbolTableManger::alloc_func_fparams(
    const std::string &ident, std::vector<FuncFParamAST> func_fparams) {
  get_back_table().func_fparams_map[ident] = std::move(func_fparams);
}

std::vector<FuncFParamAST>
SymbolTableManger::get_func_fparams(const std::string &ident) {
  auto table = find_table(ident);
  assert(table);
  return table->func_fparams_map[ident];
}

bool SymbolTableManger::is_func_has_fparams(const std::string &ident) {
  auto table = find_table(ident);
  assert(table);
  return table->is_func_has_fparams_map[ident];
}

bool SymbolTableManger::is_global_table() {
  return symbol_table_stack.size() == 1;
}

void SymbolTableManger::alloc_func_has_fparams(const std::string &ident,
                                               bool is_func_has_fparams) {
  get_back_table().is_func_has_fparams_map[ident] = is_func_has_fparams;
}

SymbolTable &SymbolTableManger::get_stmt_table(BaseAST *stmt) {
  return *stmt_table_map[stmt];
}

SymbolTable &SymbolTableManger::get_back_table() {
  assert(!symbol_table_stack.empty());
  return *symbol_table_stack.back();
}
