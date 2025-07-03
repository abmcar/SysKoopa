#include "ast.h"
#include "symbol_table.h"
#include "util.h"
#include <string>

std::ostream &operator<<(std::ostream &os, BaseAST &ast) {
  ast.print(os);
  return os;
};

void decl_lib_symbols() {
  SymbolTableManger::getInstance().alloc_ident("getint");
  SymbolTableManger::getInstance().alloc_func_has_fparams("getint", false);
  SymbolTableManger::getInstance().get_back_table().def_type_map["getint"] =
      SymbolTable::DefType::FUNC_INT;

  SymbolTableManger::getInstance().alloc_ident("getch");
  SymbolTableManger::getInstance().alloc_func_has_fparams("getch", false);
  SymbolTableManger::getInstance().get_back_table().def_type_map["getch"] =
      SymbolTable::DefType::FUNC_INT;

  SymbolTableManger::getInstance().alloc_ident("getarray");
  SymbolTableManger::getInstance().alloc_func_has_fparams("getarray", true);
  std::vector<FuncFParamAST> func_fparams = {FuncFParamAST("*i32", "n")};
  SymbolTableManger::getInstance().alloc_func_fparams("getarray", func_fparams);
  SymbolTableManger::getInstance().get_back_table().def_type_map["getarray"] =
      SymbolTable::DefType::FUNC_INT;

  SymbolTableManger::getInstance().alloc_ident("putint");
  SymbolTableManger::getInstance().alloc_func_has_fparams("putint", true);
  func_fparams = {FuncFParamAST("i32", "i")};
  SymbolTableManger::getInstance().alloc_func_fparams("putint", func_fparams);
  SymbolTableManger::getInstance().get_back_table().def_type_map["putint"] =
      SymbolTable::DefType::FUNC_VOID;

  SymbolTableManger::getInstance().alloc_ident("putch");
  SymbolTableManger::getInstance().alloc_func_has_fparams("putch", true);
  func_fparams = {FuncFParamAST("i32", "i")};
  SymbolTableManger::getInstance().alloc_func_fparams("putch", func_fparams);
  SymbolTableManger::getInstance().get_back_table().def_type_map["putch"] =
      SymbolTable::DefType::FUNC_VOID;

  SymbolTableManger::getInstance().alloc_ident("putarray");
  SymbolTableManger::getInstance().alloc_func_has_fparams("putarray", true);
  func_fparams = {FuncFParamAST("i32", "i"), FuncFParamAST("*i32", "a")};
  SymbolTableManger::getInstance().alloc_func_fparams("putarray", func_fparams);
  SymbolTableManger::getInstance().get_back_table().def_type_map["putarray"] =
      SymbolTable::DefType::FUNC_VOID;

  SymbolTableManger::getInstance().alloc_ident("starttime");
  SymbolTableManger::getInstance().alloc_func_has_fparams("starttime", false);
  SymbolTableManger::getInstance().get_back_table().def_type_map["starttime"] =
      SymbolTable::DefType::FUNC_VOID;

  SymbolTableManger::getInstance().alloc_ident("stoptime");
  SymbolTableManger::getInstance().alloc_func_has_fparams("stoptime", false);
  SymbolTableManger::getInstance().get_back_table().def_type_map["stoptime"] =
      SymbolTable::DefType::FUNC_VOID;
}

void decl_lib_functions(std::ostream &os) {
  os << "decl @getint(): i32\n";
  os << "decl @getch(): i32\n";
  os << "decl @getarray(*i32): i32\n";
  os << "decl @putint(i32)\n";
  os << "decl @putch(i32)\n";
  os << "decl @putarray(i32, *i32)\n";
  os << "decl @starttime()\n";
  os << "decl @stoptime()\n";
  os << "\n\n";
}

void CompUnitAST::print(std::ostream &os) {
  decl_lib_functions(os);
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
      SymbolTableManger::getInstance()
          .get_back_table()
          .def_type_map[func_fparams[i].ident] =
          SymbolTable::DefType::VAR_IDENT;
      SymbolTableManger::getInstance()
          .get_back_table()
          .lval_ident_map[func_fparams[i].ident] =
          this->ident + "_" + func_fparams[i].ident;
      os << "  @" << this->ident << "_" << func_fparams[i].ident
         << " = alloc i32\n";
      os << "  store @" << func_fparams[i].ident << ", @" << this->ident << "_"
         << func_fparams[i].ident << "\n";
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

void InitValAST::print(std::ostream &os) {
  if (kind == Kind::EXP && exp) {
    exp->print(os);
  } else {
    for (auto &item : *list) {
      item->print(os);
    }
  }
}

void ConstInitValAST::print(std::ostream &os) {
  if (kind == Kind::EXP && exp) {
    exp->print(os);
  } else {
    for (auto &item : *list) {
      item->print(os);
    }
  }
}

// 计算数组总大小的辅助函数
int calc_array_total_size(std::vector<std::unique_ptr<ExpAST>> *array_dims, int start_dim = 0) {
  int total = 1;
  for (int i = start_dim; i < array_dims->size(); i++) {
    total *= array_dims->at(i)->calc_number();
  }
  return total;
}

// 生成多维数组类型字符串的辅助函数
std::string generate_array_type(std::vector<std::unique_ptr<ExpAST>> *array_dims) {
  if (array_dims->empty()) {
    return "i32";
  }
  
  std::string type = "i32";
  for (int i = array_dims->size() - 1; i >= 0; i--) {
    int dim_size = array_dims->at(i)->calc_number();
    type = "[" + type + ", " + std::to_string(dim_size) + "]";
  }
  return type;
}

// 计算各维度大小的辅助函数
std::vector<int> calc_dims_size(std::vector<std::unique_ptr<ExpAST>> *array_dims) {
  std::vector<int> dims;
  for (auto &dim : *array_dims) {
    dims.push_back(dim->calc_number());
  }
  return dims;
}

// 输出嵌套数组结构的辅助函数
void output_nested_array(std::ostream &os, const std::vector<int> &data, 
                         const std::vector<int> &dims, int current_dim, int &index) {
  if (current_dim == dims.size() - 1) {
    // 最后一维，直接输出数据
    os << "{";
    for (int i = 0; i < dims[current_dim]; i++) {
      os << data[index++];
      if (i != dims[current_dim] - 1) {
        os << ", ";
      }
    }
    os << "}";
  } else {
    // 中间维度，递归输出
    os << "{";
    for (int i = 0; i < dims[current_dim]; i++) {
      output_nested_array(os, data, dims, current_dim + 1, index);
      if (i != dims[current_dim] - 1) {
        os << ", ";
      }
    }
    os << "}";
  }
}

// 填充初始化列表，转换为已经填好0的形式
void flatten_init_list(InitValAST *init_val, std::vector<int> &result, 
                       const std::vector<int> &dims, int &pos, int current_dim = 0) {
  if (init_val->kind == InitValAST::Kind::EXP) {
    // 遇到整数，从最后一维开始填充数据
    if (pos < result.size()) {
      result[pos++] = init_val->exp->calc_number();
    }
  } else if (init_val->kind == InitValAST::Kind::LIST) {
    // 遇到初始化列表时
    if (current_dim >= dims.size()) {
      return; // 维度越界
    }
    
    
    
    // 对齐到边界：检查当前对齐到了哪一个边界
    int aligned_dim = current_dim;
    for (int dim = 0; dim <= current_dim; dim++) {
      int dim_boundary = 1;
      for (int i = dim; i < dims.size(); i++) {
        dim_boundary *= dims[i];
      }
      if (pos % dim_boundary == 0) {
        aligned_dim = dim;
        break;
      }
    }
    
    // 将当前初始化列表视作这个边界所对应的最长维度的数组的初始化列表
    // 递归处理列表中的每个元素
    if (init_val->list) {
      for (auto &item : *init_val->list) {
        flatten_init_list(item.get(), result, dims, pos, aligned_dim + 1);
      }
    }
  }
}

void init_array_val(std::ostream &os, InitValAST *init_val,
                    std::vector<std::unique_ptr<ExpAST>> *array_dims, int dim) {
  if (dim == 0) {
    // 首次调用，将初始化列表转换为已经填好0的平坦数组
    std::vector<int> dims = calc_dims_size(array_dims);
    int total_size = calc_array_total_size(array_dims);
    std::vector<int> flattened(total_size, 0); // 初始化为0
    
    int pos = 0;
    if (init_val) {
      flatten_init_list(init_val, flattened, dims, pos, 0);
    }
    
    // 递归输出多维数组结构
    int index = 0;
    output_nested_array(os, flattened, dims, 0, index);
  }
}

// 填充常量初始化列表，转换为已经填好0的形式
void flatten_const_init_list(ConstInitValAST *const_init_val, std::vector<int> &result, 
                             const std::vector<int> &dims, int &pos, int current_dim = 0) {
  if (const_init_val->kind == ConstInitValAST::Kind::EXP) {
    // 遇到整数，从最后一维开始填充数据
    if (pos < result.size()) {
      result[pos++] = const_init_val->exp->calc_number();
    }
  } else if (const_init_val->kind == ConstInitValAST::Kind::LIST) {
    // 遇到初始化列表时
    if (current_dim >= dims.size()) {
      return; // 维度越界
    }
    
    // 对齐到边界：检查当前对齐到了哪一个边界
    int aligned_dim = current_dim;
    for (int dim = 0; dim <= current_dim; dim++) {
      int dim_boundary = 1;
      for (int i = dim; i < dims.size(); i++) {
        dim_boundary *= dims[i];
      }
      if (pos % dim_boundary == 0) {
        aligned_dim = dim;
        break;
      }
    }
    
    // 将当前初始化列表视作这个边界所对应的最长维度的数组的初始化列表
    // 递归处理列表中的每个元素
    if (const_init_val->list) {
      for (auto &item : *const_init_val->list) {
        flatten_const_init_list(item.get(), result, dims, pos, aligned_dim + 1);
      }
    }
  }
}

// 计算常量数组总大小的辅助函数
int calc_const_array_total_size(const std::vector<int> &array_dims, int start_dim = 0) {
  int total = 1;
  for (int i = start_dim; i < array_dims.size(); i++) {
    total *= array_dims[i];
  }
  return total;
}

// 生成常量多维数组类型字符串的辅助函数
std::string generate_const_array_type(const std::vector<int> &array_dims) {
  if (array_dims.empty()) {
    return "i32";
  }
  
  std::string type = "i32";
  for (int i = array_dims.size() - 1; i >= 0; i--) {
    int dim_size = array_dims[i];
    type = "[" + type + ", " + std::to_string(dim_size) + "]";
  }
  return type;
}

void init_const_array_val(std::ostream &os, ConstInitValAST *const_init_val,
                          const std::vector<int> &array_dims, int dim) {
  if (dim == 0) {
    // 首次调用，将初始化列表转换为已经填好0的平坦数组
    int total_size = calc_const_array_total_size(array_dims);
    std::vector<int> flattened(total_size, 0); // 初始化为0
    
    int pos = 0;
    if (const_init_val) {
      flatten_const_init_list(const_init_val, flattened, array_dims, pos, 0);
    }
    
    // 递归输出多维数组结构
    int index = 0;
    output_nested_array(os, flattened, array_dims, 0, index);
  }
}

void VarDefAST::print(std::ostream &os) {
  std::string ident = SymbolTableManger::getInstance()
                          .get_back_table()
                          .lval_ident_map[this->ident];
  bool is_global = SymbolTableManger::getInstance().is_global_table();

  if (is_global) {
    if (kind == DefAST::Kind::VAR_IDENT) {
      os << "global @" + ident << " = alloc i32, zeroinit\n";
    } else if (kind == DefAST::Kind::VAR_DEF) {
      if (init_val)
        init_val->print(os);
      os << "global @" + ident << " = alloc i32, "
         << get_koopa_exp_reg(init_val ? init_val->exp.get() : nullptr) << "\n";
    } else if (kind == DefAST::Kind::VAR_ARRAY_IDENT) {
      std::string array_type = generate_array_type(array_dims);
      os << "global @" + ident << " = alloc " << array_type << ", zeroinit\n";
    } else if (kind == DefAST::Kind::VAR_ARRAY_DEF) {
      std::string array_type = generate_array_type(array_dims);
      os << "global @" + ident << " = alloc " << array_type << ", ";
      init_array_val(os, init_val.get(), array_dims, 0);
      os << "\n";
    } else {
      assert(false);
    }
    SymbolTableManger::getInstance().alloc_ident(this->ident);
    return;
  } else {
    if (kind == DefAST::Kind::VAR_IDENT || kind == DefAST::Kind::VAR_DEF) {
      os << "  @" + ident << " = alloc i32\n";
    } else if ((kind == DefAST::Kind::VAR_ARRAY_IDENT ||
                kind == DefAST::Kind::VAR_ARRAY_DEF)) {
      std::string array_type = generate_array_type(array_dims);
      os << "  @" + ident << " = alloc " << array_type << "\n";
      if (kind == DefAST::Kind::VAR_ARRAY_DEF && init_val) {
        os << "  store ";
        init_array_val(os, init_val.get(), array_dims, 0);
        os << ", @" + ident << "\n";
      }
    } else {
      assert(false);
    }
  }

  if (kind == DefAST::Kind::VAR_DEF && init_val) {
    init_val->print(os);
    os << "  store " << get_koopa_exp_reg(init_val->exp.get()) << ", @" + ident
       << "\n";
  }

  SymbolTableManger::getInstance().alloc_ident(this->ident);
}

void ConstDefAST::print(std::ostream &os) {
  SymbolTableManger::getInstance().alloc_ident(this->ident);
  auto ident = SymbolTableManger::getInstance().get_ident(this->ident);
  bool is_global = SymbolTableManger::getInstance().is_global_table();

  if (SymbolTableManger::getInstance().get_def_type(this->ident) ==
      SymbolTable::DefType::CONST_ARRAY && !array_dims.empty()) {
    // 支持多维常量数组
    std::string array_type = generate_const_array_type(array_dims);
    
    if (is_global) {
      os << "global @" << ident << " = alloc " << array_type << ", ";
      if (const_init_val_ast) {
        init_const_array_val(os, const_init_val_ast.get(), array_dims, 0);
      } else {
        os << "zeroinit";
      }
      os << "\n";
    } else {
      os << "  @" << ident << " = alloc " << array_type << "\n";
      if (const_init_val_ast) {
        os << "  store ";
        init_const_array_val(os, const_init_val_ast.get(), array_dims, 0);
        os << ", @" << ident << "\n";
      }
    }
  } else if (SymbolTableManger::getInstance().get_def_type(this->ident) ==
             SymbolTable::DefType::CONST) {
    // 普通常量处理保持不变
  }
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

void LValAST::print(std::ostream &os) {
  if (kind == Kind::IDENT) {
    os << ident;
  } else if (kind == Kind::ARRAY_ACCESS) {
    if (array_index_list->size() == 1) {
      auto reg = IRManager::getInstance().getNextReg();
      array_index_list->at(0)->print(os);
      std::string idx = get_koopa_exp_reg(array_index_list->at(0).get());
      array_index_list->at(0)->calc_number();
      os << "  %" << reg << " = getelemptr @" + ident << ", " << idx << "\n";
      os << "  @" << ident << " = %" << reg << "\n";
    } else {
      os << "  # TODO multi-dimensional array access\n";
    }
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
    std::string ident =
        SymbolTableManger::getInstance().get_ident(l_val->ident);
    if (SymbolTableManger::getInstance().get_def_type(l_val->ident) ==
            SymbolTable::DefType::VAR_ARRAY &&
        l_val->array_index_list->size() == 1) {
      auto reg = IRManager::getInstance().getNextReg();
      l_val->array_index_list->at(0)->print(os);
      std::string idx = get_koopa_exp_reg(l_val->array_index_list->at(0).get());
      os << "  %" << reg << " = getelemptr @" + ident << ", " << idx << "\n";
      os << "  store " << get_koopa_exp_reg(r_exp.get()) << ", %" << reg
         << "\n";
    } else if (SymbolTableManger::getInstance().get_def_type(l_val->ident) ==
               SymbolTable::DefType::VAR_ARRAY) {
      os << "  # TODO multi-dimensional array assignment\n";
    } else {
      os << "  store " << get_koopa_exp_reg(r_exp.get()) << ", @" + ident
         << "\n";
    }
  } else if (kind == StmtAST::Kind::BLOCK_STMT) {
    SymbolTableManger::getInstance().use_stmt_table(block.get());
    block->print(os);
    SymbolTableManger::getInstance().pop_symbol_table();
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
    auto l_val = this->l_val.get();
    if (l_val->kind == LValAST::Kind::ARRAY_ACCESS) {
      if (l_val->array_index_list->size() == 1) {
        l_val->array_index_list->at(0)->print(os);
        std::string idx =
            get_koopa_exp_reg(l_val->array_index_list->at(0).get());
        int ptr = IRManager::getInstance().getNextReg();
        reg = IRManager::getInstance().getNextReg();
        std::string ident =
            SymbolTableManger::getInstance().get_ident(l_val->ident);
        os << "  %" << ptr << " = getelemptr @" + ident << ", " << idx << "\n";
        os << "  %" << reg << " = load %" << ptr << "\n";
      } else {
        os << "  # TODO multi-dimensional array load\n";
      }
    } else {
      if (SymbolTableManger::getInstance().get_def_type(l_val->ident) ==
          SymbolTable::DefType::CONST) {
        number = SymbolTableManger::getInstance().get_val(l_val->ident);
        kind = ExpAST::Kind::NUMBER;
      } else {
        reg = IRManager::getInstance().getNextReg();
        std::string ident =
            SymbolTableManger::getInstance().get_ident(l_val->ident);
        os << "  %" << reg << " = load @" + ident << "\n";
      }
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
    auto DType = SymbolTableManger::getInstance().get_def_type(ident);
    if (DType == SymbolTable::DefType::FUNC_VOID) {
      os << "  call @" << ident << "(";
    } else if (DType == SymbolTable::DefType::FUNC_INT) {
      reg = IRManager::getInstance().getNextReg();
      os << "  %" << reg << " = call @" << ident << "(";
    }
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

    reg = IRManager::getInstance().getNextReg();

    std::string tmp_var_name = "Or" + std::to_string(reg) + "_tmp_var";
    std::string lhs_false = "%lhs_false" + std::to_string(reg);
    std::string rhs_false = "%rhs_false" + std::to_string(reg);
    std::string end = "%end_or_" + std::to_string(reg);

    os << "  @" + tmp_var_name << " = alloc i32\n";
    os << "  store 1, @" + tmp_var_name << "\n";
    os << "  br " << get_koopa_exp_reg(l_or_exp.get()) << ", " << end << ", "
       << lhs_false << "\n";

    os << lhs_false << ":\n";
    l_and_exp->print(os);
    os << "  br " << get_koopa_exp_reg(l_and_exp.get()) << ", " << end << ", "
       << rhs_false << "\n";

    os << rhs_false << ":\n";
    os << "  store 0, @" + tmp_var_name << "\n";
    os << "  jump " << end << "\n";

    os << end << ":\n";
    os << "  %" << reg << " = load @" + tmp_var_name << "\n";
  }
}

void LAndExpAST::print(std::ostream &os) {
  if (kind == ExpAST::Kind::EQ_EXP) {
    eq_exp->print(os);
    pushup_exp_reg(eq_exp.get(), this);
  } else {
    l_and_exp->print(os);

    reg = IRManager::getInstance().getNextReg();

    std::string tmp_var_name = "And" + std::to_string(reg) + "_tmp_var";
    std::string lhs_true = "%lhs_true" + std::to_string(reg);
    std::string rhs_false = "%rhs_false" + std::to_string(reg);
    std::string end = "%end_and_" + std::to_string(reg);

    os << "  @" + tmp_var_name << " = alloc i32\n";
    os << "  store 1, @" + tmp_var_name << "\n";
    os << "  br " << get_koopa_exp_reg(l_and_exp.get()) << ", " << lhs_true
       << ", " << rhs_false << "\n";

    os << lhs_true << ":\n";
    eq_exp->print(os);
    os << "  br " << get_koopa_exp_reg(eq_exp.get()) << ", " << end << ", "
       << rhs_false << "\n";

    os << rhs_false << ":\n";
    os << "  store 0, @" + tmp_var_name << "\n";
    os << "  jump " << end << "\n";

    os << end << ":\n";
    os << "  %" << reg << " = load @" + tmp_var_name << "\n";
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
      return SymbolTableManger::getInstance()
          .get_back_table()
          .val_map[l_val->ident];
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