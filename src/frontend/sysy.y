%debug

%code requires {
  #include <memory>
  #include <string>
  #include "ast.h"
  #include "symbol_table.h"
}

%{

#include <iostream>
#include <memory>
#include <string>
#include "ast.h"
#include "symbol_table.h"

// 声明 lexer 函数和错误处理函数
int yylex();
void yyerror(std::unique_ptr<std::string> &ast, const char *s);
void yyerror(std::unique_ptr<BaseAST> &ast, const char *s);

using namespace std;

%}

// 定义 parser 函数和错误处理函数的附加参数
// 我们需要返回一个字符串作为 AST, 所以我们把附加参数定义成字符串的智能指针
// 解析完成后, 我们要手动修改这个参数, 把它设置成解析得到的字符串
// %parse-param { std::unique_ptr<std::string> &ast }
%parse-param { std::unique_ptr<BaseAST> &ast }

// yylval 的定义, 我们把它定义成了一个联合体 (union)
// 因为 token 的值有的是字符串指针, 有的是整数
// 之前我们在 lexer 中用到的 str_val 和 int_val 就是在这里被定义的
// 至于为什么要用字符串指针而不直接用 string 或者 unique_ptr<string>?
// 请自行 STFW 在 union 里写一个带析构函数的类会出现什么情况
%union {
  std::string *str_val;
  int int_val;
  UnaryOpKind unary_op_kind;
  BaseAST *ast_val;
  ExpAST *exp_ast_val;
  DefAST *def_ast_val;
  std::vector<std::unique_ptr<BaseAST>> *ast_vec_val;
  std::vector<std::unique_ptr<DefAST>> *def_ast_vec_val;
}

// lexer 返回的所有 token 种类的声明
// 注意 IDENT 和 INT_CONST 会返回 token 的值, 分别对应 str_val 和 int_val
%token INT RETURN CONST IF ELSE WHILE BREAK CONTINUE
%token <str_val> IDENT
%token <int_val> INT_CONST
%token LOGICAL_OP_GREATER_EQUAL LOGICAL_OP_LESS_EQUAL LOGICAL_OP_EQUAL LOGICAL_OP_NOT_EQUAL LOGICAL_OP_OR LOGICAL_OP_AND LOGICAL_OP_GREATER LOGICAL_OP_LESS

%nonassoc LOWER_THAN_ELSE
%nonassoc ELSE

// 非终结符的类型定义
%type <ast_val> FuncDef FuncType Block BlockItem Stmt Decl ConstDecl VarDecl
%type <int_val> Number ConstInitVal
%type <str_val> BType LVal
%type <unary_op_kind> UnaryOp
%type <exp_ast_val> Exp PrimaryExp UnaryExp AddExp MulExp LOrExp LAndExp EqExp RelExp ConstExp InitVal
%type <ast_vec_val> BlockItemList
%type <def_ast_val> ConstDef VarDef
%type <def_ast_vec_val> ConstDefList VarDefList
%%

// 开始符, CompUnit ::= FuncDef, 大括号后声明了解析完成后 parser 要做的事情
// 之前我们定义了 FuncDef 会返回一个 str_val, 也就是字符串指针
// 而 parser 一旦解析完 CompUnit, 就说明所有的 token 都被解析了, 即解析结束了
// 此时我们应该把 FuncDef 返回的结果收集起来, 作为 AST 传给调用 parser 的函数
// $1 指代规则里第一个符号的返回值, 也就是 FuncDef 的返回值
// CompUnit ::= FuncDef
CompUnit
  : FuncDef {
    auto comp_unit = make_unique<CompUnitAST>();
    comp_unit->func_def = unique_ptr<BaseAST>($1);
    ast = move(comp_unit);
    // ast = unique_ptr<string>($1);
  }
  ;

// FuncDef ::= FuncType IDENT '(' ')' Block;
// 我们这里可以直接写 '(' 和 ')', 因为之前在 lexer 里已经处理了单个字符的情况
// 解析完成后, 把这些符号的结果收集起来, 然后拼成一个新的字符串, 作为结果返回
// $$ 表示非终结符的返回值, 我们可以通过给这个符号赋值的方法来返回结果
// 你可能会问, FuncType, IDENT 之类的结果已经是字符串指针了
// 为什么还要用 unique_ptr 接住它们, 然后再解引用, 把它们拼成另一个字符串指针呢
// 因为所有的字符串指针都是我们 new 出来的, new 出来的内存一定要 delete
// 否则会发生内存泄漏, 而 unique_ptr 这种智能指针可以自动帮我们 delete
// 虽然此处你看不出用 unique_ptr 和手动 delete 的区别, 但当我们定义了 AST 之后
// 这种写法会省下很多内存管理的负担
FuncDef
  : FuncType IDENT '(' ')' Block {
    auto ast = new FuncDefAST();
    ast->func_type = unique_ptr<BaseAST>($1);
    ast->ident= *unique_ptr<string>($2);
    ast->block = unique_ptr<BaseAST>($5);
    $$ = ast;
  }
  ;

// FuncType ::= "int";
FuncType
  : INT {
    auto ast = new FuncTypeAST();
    ast->func_type = "int";
    $$ = ast;
  }
  ;

// Decl ::= ConstDecl | VarDecl
Decl
  : ConstDecl {
    auto ast = new DeclAST();
    ast->kind = DeclAST::Kind::CONST_DECL;
    ast->const_decl = unique_ptr<BaseAST>($1);
    $$ = ast;
  }
  | VarDecl {
    auto ast = new DeclAST();
    ast->kind = DeclAST::Kind::VAR_DECL;
    ast->var_decl = unique_ptr<BaseAST>($1);
    $$ = ast;
  }
  ;

// ConstDecl ::= "const" Btype ConstDef {"," ConstDef} ";";
// ConstDecl ::= "const" Btype ConstDefList ";";
ConstDecl
  : CONST BType ConstDefList ';' {
    auto ast = new ConstDeclAST();
    ast->b_type = *unique_ptr<string>($2);
    ast->const_def_list = $3;
    for (auto &item : *$3) {
      SymbolTableManger::getInstance().get_back_table().type_map[item->ident] = ast->b_type;
    }
    $$ = ast;
  }
  ;

// VarDecl ::= Btype VarDef {"," VarDef} ";";
// VarDecl ::= Btype VarDefList ";";
VarDecl
  : BType VarDefList ';' {
    auto ast = new VarDeclAST();
    ast->b_type = *unique_ptr<string>($1);
    ast->var_def_list = $2;
    for (auto &item : *ast->var_def_list) {
      SymbolTableManger::getInstance().get_back_table().type_map[item->ident] = ast->b_type;
    }
    $$ = ast;
  }
  ;

// VarDefList ::= VarDef {"," VarDef};
VarDefList
  : VarDef {
    auto vec = new vector<unique_ptr<DefAST>>();
    vec->push_back(unique_ptr<DefAST>($1));
    $$ = vec;
  }
  | VarDef ',' VarDefList {
    auto vec = new vector<unique_ptr<DefAST>>();
    vec->push_back(unique_ptr<DefAST>($1));
    for (auto &item : *$3) {
      vec->push_back(move(item));
    }
    delete $3;
    $$ = vec;
  }
  ;

// VarDef ::= IDENT | IDENT "=" InitVal;
VarDef
  : IDENT {
    auto ast = new VarDefAST();
    ast->kind = DefAST::Kind::VAR_IDENT;
    ast->ident = *unique_ptr<string>($1);
    SymbolTableManger::getInstance().get_back_table().def_type_map[ast->ident] = SymbolTable::DefType::VAR_IDENT;
    SymbolTableManger::getInstance().get_back_table().lval_ident_map[ast->ident] = SymbolTableManger::getInstance().get_lval_ident(ast->ident);
    $$ = ast;
  }
  | IDENT '=' InitVal {
    auto ast = new VarDefAST();
    ast->kind = DefAST::Kind::VAR_DEF;
    ast->ident = *unique_ptr<string>($1);
    ast->init_val = unique_ptr<ExpAST>($3);
    SymbolTableManger::getInstance().get_back_table().def_type_map[ast->ident] = SymbolTable::DefType::VAR_EXP;
    SymbolTableManger::getInstance().get_back_table().exp_val_map[ast->ident] = ast->init_val.get();
    SymbolTableManger::getInstance().get_back_table().lval_ident_map[ast->ident] = SymbolTableManger::getInstance().get_lval_ident(ast->ident);
    $$ = ast;
  }
  ;

// InitVal ::= Exp;
InitVal
  : Exp {
    $$ = $1;
  }
  ;

// BType ::= "int";
BType
  : INT {
    $$ = new std::string("int");
  }
  ;

// ConstDefList ::= ConstDef {"," ConstDef};
ConstDefList
  : ConstDef {
    auto vec = new vector<unique_ptr<DefAST>>();
    vec->push_back(unique_ptr<DefAST>($1));
    $$ = vec;
  }
  | ConstDef ',' ConstDefList {
    auto vec = new vector<unique_ptr<DefAST>>();
    vec->push_back(unique_ptr<DefAST>($1));
    for (auto &item : *$3) {
      vec->push_back(move(item));
    }
    $$ = vec;
  }
  ;

// ConstDef ::= IDENT "=" ConstInitVal;
ConstDef
  : IDENT '=' ConstInitVal {
    auto ast = new ConstDefAST();
    ast->kind = DefAST::Kind::CONST_DEF;
    ast->ident = *unique_ptr<string>($1);
    ast->const_init_val = $3;
    SymbolTableManger::getInstance().get_back_table().val_map[ast->ident] = ast->const_init_val;
    SymbolTableManger::getInstance().get_back_table().def_type_map[ast->ident] = SymbolTable::DefType::CONST;
    SymbolTableManger::getInstance().get_back_table().lval_ident_map[ast->ident] = SymbolTableManger::getInstance().get_lval_ident(ast->ident);
    $$ = ast;
  }
  ;

// ConstInitVal ::= ConstExp;
ConstInitVal
  : ConstExp {
    $$ = $1->calc_number();
  }
  ;

// Block ::= "{" {BlockItem} "}";
Block
  : '{' {
    SymbolTableManger::getInstance().push_symbol_table();
  }
  BlockItemList '}' {
    auto ast = new BlockAST();
    ast->block_item_list = $3;
    $$ = ast;
    SymbolTableManger::getInstance().alloc_stmt_table(ast);
    SymbolTableManger::getInstance().pop_symbol_table();
  }
  | '{' '}' {
    auto ast = new BlockAST();
    ast->block_item_list = nullptr;
    $$ = ast;
  }
  ;

// BlockItemList ::= BlockItem | BlockItem BlockItemList;
BlockItemList
  : BlockItem {
    auto vec = new vector<unique_ptr<BaseAST>>();
    vec->push_back(unique_ptr<BaseAST>($1));
    $$ = vec;
  }
  | BlockItem BlockItemList {
    auto vec = new vector<unique_ptr<BaseAST>>();
    vec->push_back(unique_ptr<BaseAST>($1));
    for (auto &item : *$2) {
      vec->push_back(move(item));
    }
    $$ = vec;
  }
  ;

// BlockItem ::= Decl | Stmt;
BlockItem
  : Decl {
    auto ast = new BlockItemAST();
    ast->kind = BlockItemAST::Kind::DECL;
    ast->decl = unique_ptr<BaseAST>($1);
    $$ = ast;
  }
  | Stmt {
    auto ast = new BlockItemAST();
    ast->kind = BlockItemAST::Kind::STMT;
    ast->stmt = unique_ptr<BaseAST>($1);
    $$ = ast;
  }
  ;

/* Stmt ::= "return" [Exp] ";" | 
    LVal "=" Exp ";" | 
    Block |
    [Exp] ";" | 
    "if" "(" [Exp] ")" Stmt [ "else" Stmt ] |
    "while" "(" [Exp] ")" Stmt |
    "break" ";" |
    "continue" ";";
*/

Stmt
  : RETURN Exp ';' {
    auto ast = new StmtAST();
    ast->kind = StmtAST::Kind::RETURN_STMT;
    ast->exp = unique_ptr<ExpAST>($2);
    $$ = ast;
  }
  | RETURN ';' {
    auto ast = new StmtAST();
    ast->kind = StmtAST::Kind::RETURN_STMT;
    ast->exp = nullptr;
    $$ = ast;
  }
  | LVal '=' Exp ';' {
    auto ast = new StmtAST();
    ast->kind = StmtAST::Kind::ASSIGN_STMT;
    ast->l_val = *unique_ptr<string>($1);
    ast->r_exp = unique_ptr<ExpAST>($3);
    if (!SymbolTableManger::getInstance().is_var_defined(ast->l_val)) {
      assert(false);
    } else {
      SymbolTableManger::getInstance().get_back_table().def_type_map[ast->l_val] = SymbolTable::DefType::VAR_EXP;
      SymbolTableManger::getInstance().get_back_table().exp_val_map[ast->l_val] = ast->r_exp.get();
    }
    $$ = ast;
  }
  | Block {
    auto ast = new StmtAST();
    ast->kind = StmtAST::Kind::BLOCK_STMT;
    ast->block = unique_ptr<BaseAST>($1);
    $$ = ast;
  }
  | Exp ';' {
    auto ast = new StmtAST();
    ast->kind = StmtAST::Kind::EXP_STMT;
    ast->exp = unique_ptr<ExpAST>($1);
    $$ = ast;
  }
  | ';' {
    auto ast = new StmtAST();
    ast->kind = StmtAST::Kind::EMPTY_STMT;
    $$ = ast;
  }
  | IF '(' Exp ')' Stmt %prec LOWER_THAN_ELSE {
    auto ast = new StmtAST();
    ast->kind = StmtAST::Kind::IF_STMT;
    ast->if_exp = unique_ptr<ExpAST>($3);
    ast->if_stmt = unique_ptr<BaseAST>($5);
    $$ = ast;
  }
  | IF '(' Exp ')' Stmt ELSE Stmt {
    auto ast = new StmtAST();
    ast->kind = StmtAST::Kind::IF_ELSE_STMT;
    ast->if_exp = unique_ptr<ExpAST>($3);
    ast->if_stmt = unique_ptr<BaseAST>($5);
    ast->else_stmt = unique_ptr<BaseAST>($7);
    $$ = ast;
  }
  | WHILE '(' Exp ')' Stmt {
    auto ast = new StmtAST();
    ast->kind = StmtAST::Kind::WHILE_STMT;
    ast->while_exp = unique_ptr<ExpAST>($3);
    ast->while_stmt = unique_ptr<BaseAST>($5);
    $$ = ast;
  }
  | BREAK ';' {
    auto ast = new StmtAST();
    ast->kind = StmtAST::Kind::BREAK_STMT;
    $$ = ast;
  }
  | CONTINUE ';' {
    auto ast = new StmtAST();
    ast->kind = StmtAST::Kind::CONTINUE_STMT;
    $$ = ast;
  }
  ;

// Exp ::= LOrExp;
Exp
  : LOrExp {
    auto ast = new ExpAST();
    ast->kind = ExpAST::Kind::L_OR_EXP;
    ast->l_or_exp = unique_ptr<ExpAST>($1);
    $$ = ast;
  }
  ;

// UnaryExp ::= PrimaryExp | UnaryOp UnaryExp;
UnaryExp
  : PrimaryExp {
    auto ast = new UnaryExpAST();
    ast->kind = ExpAST::Kind::PRIMARY_EXP;
    ast->primary_exp = unique_ptr<ExpAST>($1);
    $$ = ast;
  }
  | UnaryOp UnaryExp {
    auto ast = new UnaryExpAST();
    ast->kind = ExpAST::Kind::UNARY_OP_EXP;
    ast->unary_op = $1;
    ast->unary_exp = unique_ptr<ExpAST>($2);
    $$ = ast;
  }
  ;

// PrimaryExp ::= "(" Exp ")" | LVal | Number;
PrimaryExp
  : '(' Exp ')' {
    auto ast = new PrimaryExpAST();
    ast->kind = ExpAST::Kind::EXP;
    ast->exp = unique_ptr<ExpAST>($2);
    $$ = ast;
  }
  | LVal {
    auto ast = new PrimaryExpAST();
    ast->kind = ExpAST::Kind::L_VAL;
    ast->l_val = *unique_ptr<string>($1);
    $$ = ast;
  }
  | Number {
    auto ast = new PrimaryExpAST();
    ast->kind = ExpAST::Kind::NUMBER;
    ast->number = $1;
    $$ = ast;
  }
  ;

LVal
  : IDENT {
    $$ = $1;
  }
  ;

// UnaryOp ::= "+" | "-" | "!";
UnaryOp
  : '+' {
    $$ = UnaryOpKind::Plus;
  }
  | '-' {
    $$ = UnaryOpKind::Minus;
  }
  | '!' {
    $$ = UnaryOpKind::Not;
  }
;

// AddExp ::= MulExp | AddExp ("+" | "-") MulExp;
AddExp
  : MulExp {
    auto ast = new AddExpAST();
    ast->kind = ExpAST::Kind::MUL_EXP;
    ast->mul_exp = unique_ptr<ExpAST>($1);
    $$ = ast;
  }
  | AddExp '+' MulExp {
    auto ast = new AddExpAST();
    ast->add_exp = unique_ptr<ExpAST>($1);
    ast->add_op = AddOpKind::Plus;
    ast->mul_exp = unique_ptr<ExpAST>($3);
    $$ = ast;
  }
  | AddExp '-' MulExp { 
    auto ast = new AddExpAST();
    ast->add_exp = unique_ptr<ExpAST>($1);
    ast->add_op = AddOpKind::Minus;
    ast->mul_exp = unique_ptr<ExpAST>($3);
    $$ = ast;
  }
  ;

// MulExp ::= UnaryExp | MulExp ("*" | "/" | "%") UnaryExp;
MulExp
  : UnaryExp {
    auto ast = new MulExpAST();
    ast->kind = ExpAST::Kind::UNARY_EXP;
    ast->unary_exp = unique_ptr<ExpAST>($1);
    $$ = ast;
  }
  | MulExp '*' UnaryExp {
    auto ast = new MulExpAST();
    ast->mul_exp = unique_ptr<ExpAST>($1);
    ast->mul_op = MulOpKind::Mul;
    ast->unary_exp = unique_ptr<ExpAST>($3);
    $$ = ast;
  }
  | MulExp '/' UnaryExp {
    auto ast = new MulExpAST();
    ast->mul_exp = unique_ptr<ExpAST>($1);
    ast->mul_op = MulOpKind::Div;
    ast->unary_exp = unique_ptr<ExpAST>($3);
    $$ = ast;
  }
  | MulExp '%' UnaryExp { 
    auto ast = new MulExpAST();
    ast->mul_exp = unique_ptr<ExpAST>($1);
    ast->mul_op = MulOpKind::Mod;
    ast->unary_exp = unique_ptr<ExpAST>($3);
    $$ = ast;
  }
  ;

// LOrExp ::= LAndExp | LOrExp "||" LAndExp;
LOrExp
  : LAndExp {
    auto ast = new LOrExpAST();
    ast->kind = ExpAST::Kind::L_AND_EXP;
    ast->l_and_exp = unique_ptr<ExpAST>($1);
    $$ = ast;
  }
  | LOrExp LOGICAL_OP_OR LAndExp {
    auto ast = new LOrExpAST();
    ast->kind = ExpAST::Kind::L_OR_EXP;
    ast->l_or_exp = unique_ptr<ExpAST>($1);
    ast->logical_op = LogicalOpKind::Or;
    ast->l_and_exp = unique_ptr<ExpAST>($3);
    $$ = ast;
  }
  ;

// LAndExp ::= EqExp | LAndExp "&&" EqExp;
LAndExp
  : EqExp {
    auto ast = new LAndExpAST();
    ast->kind = ExpAST::Kind::EQ_EXP;
    ast->eq_exp = unique_ptr<ExpAST>($1);
    $$ = ast;
  }
  | LAndExp LOGICAL_OP_AND EqExp {
    auto ast = new LAndExpAST();
    ast->kind = ExpAST::Kind::L_AND_EXP;
    ast->l_and_exp = unique_ptr<ExpAST>($1);
    ast->logical_op = LogicalOpKind::And;
    ast->eq_exp = unique_ptr<ExpAST>($3);
    $$ = ast;
  }
  ;

// EqExp ::= RelExp | EqExp "==" RelExp | EqExp "!=" RelExp;
EqExp
  : RelExp {
    auto ast = new EqExpAST();
    ast->kind = ExpAST::Kind::REL_EXP;
    ast->rel_exp = unique_ptr<ExpAST>($1);
    $$ = ast;
  }
  | EqExp LOGICAL_OP_EQUAL RelExp {
    auto ast = new EqExpAST();
    ast->eq_exp = unique_ptr<ExpAST>($1);
    ast->logical_op = LogicalOpKind::Equal;
    ast->rel_exp = unique_ptr<ExpAST>($3);
    $$ = ast;
  }
  | EqExp LOGICAL_OP_NOT_EQUAL RelExp {
    auto ast = new EqExpAST();
    ast->eq_exp = unique_ptr<ExpAST>($1);
    ast->logical_op = LogicalOpKind::NotEqual;
    ast->rel_exp = unique_ptr<ExpAST>($3);
    $$ = ast;
  }
  ;

// RelExp ::= AddExp | RelExp (">" | "<" | ">=" | "<=") AddExp;
RelExp
  : AddExp {
    auto ast = new RelExpAST();
    ast->kind = ExpAST::Kind::ADD_EXP;
    ast->add_exp = unique_ptr<ExpAST>($1);
    $$ = ast;
  }
  | RelExp LOGICAL_OP_GREATER AddExp {
    auto ast = new RelExpAST();
    ast->rel_exp = unique_ptr<ExpAST>($1);
    ast->logical_op = LogicalOpKind::Greater; 
    ast->add_exp = unique_ptr<ExpAST>($3);
    $$ = ast;
  }
  | RelExp LOGICAL_OP_LESS AddExp {
    auto ast = new RelExpAST();
    ast->rel_exp = unique_ptr<ExpAST>($1);
    ast->logical_op = LogicalOpKind::Less;
    ast->add_exp = unique_ptr<ExpAST>($3);
    $$ = ast;
  }
  | RelExp LOGICAL_OP_GREATER_EQUAL AddExp {
    auto ast = new RelExpAST();
    ast->rel_exp = unique_ptr<ExpAST>($1);
    ast->logical_op = LogicalOpKind::GreaterEqual;
    ast->add_exp = unique_ptr<ExpAST>($3);
    $$ = ast;
  }
  | RelExp LOGICAL_OP_LESS_EQUAL AddExp {
    auto ast = new RelExpAST();
    ast->rel_exp = unique_ptr<ExpAST>($1);
    ast->logical_op = LogicalOpKind::LessEqual;
    ast->add_exp = unique_ptr<ExpAST>($3);
    $$ = ast;
  }
  ;

// ConstExp ::= Exp;
ConstExp
  : Exp {
    $$ = $1;
  }
  ;

Number
  : INT_CONST {
    $$ = $1;
  }
  ;

%%

// 定义错误处理函数, 其中第二个参数是错误信息
// parser 如果发生错误 (例如输入的程序出现了语法错误), 就会调用这个函数
void yyerror(unique_ptr<BaseAST> &ast, const char *s) {
  cerr << "error: " << s << endl;
  /* ast->Dump(); */
}
