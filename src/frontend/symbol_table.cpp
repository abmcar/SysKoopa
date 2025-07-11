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
      def_type_map[ident] == DefType::VAR_IDENT ||
      def_type_map[ident] == DefType::CONST_ARRAY ||
      def_type_map[ident] == DefType::VAR_ARRAY) {
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
  return ident + "_" + std::to_string(ident_count_map[ident]++);
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

std::vector<int> SymbolTableManger::get_const_array_val(const std::string &ident) {
  auto table = find_table(ident);
  assert(table);
  return table->const_array_val_map[ident];
}

std::vector<int> SymbolTableManger::get_array_dims(const std::string &ident) {
  auto table = find_table(ident);
  assert(table);
  return table->array_dims_map[ident];
}

bool SymbolTableManger::has_array_dims(const std::string &ident) {
  auto table = find_table(ident);
  if (!table)
    return false;
  return table->array_dims_map.find(ident) != table->array_dims_map.end();
}

void SymbolTableManger::set_array_dims(const std::string &ident,
                                       const std::vector<int> &dims) {
  get_back_table().array_dims_map[ident] = dims;
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

bool SymbolTableManger::is_func_param(const std::string &ident) {
  auto table = find_table(ident);
  if (!table)
    return false;
  return table->is_func_param_map[ident];
}

void SymbolTableManger::set_func_param(const std::string &ident) {
  get_back_table().is_func_param_map[ident] = true;
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
