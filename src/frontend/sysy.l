%option noyywrap
%option nounput
%option noinput

%{

#include <cstdlib>
#include <string>

// 因为 Flex 会用到 Bison 中关于 token 的定义
// 所以需要 include Bison 生成的头文件
#include "sysy.tab.hpp"

using namespace std;

%}

/* 空白符和注释 */
WhiteSpace    [ \t\n\r]*
LineComment   "//".*
BlockComment   "/*"(.|\n)*?"*/"

/* 标识符 */
Identifier    [a-zA-Z_][a-zA-Z0-9_]*

/* 整数字面量 */
Decimal       [1-9][0-9]*
Octal         0[0-7]*
Hexadecimal   0[xX][0-9a-fA-F]+

LogicalOperator   (?:>=|<=|==|!=|&&|\|\||[<>])

%%

{WhiteSpace}    { /* 忽略, 不做任何操作 */ }
{LineComment}   { /* 忽略, 不做任何操作 */ }
{BlockComment}  { /* 忽略, 不做任何操作 */ }

"void"          { return VOID; }
"int"           { return INT; }
"return"        { return RETURN; }
"const"         { return CONST; }
"if"            { return IF; }
"else"          { return ELSE; }
"while"         { return WHILE; }
"break"         { return BREAK; }
"continue"      { return CONTINUE; }

{Identifier}    { yylval.str_val = new string(yytext); return IDENT; }

{Decimal}       { yylval.int_val = strtol(yytext, nullptr, 0); return INT_CONST; }
{Octal}         { yylval.int_val = strtol(yytext, nullptr, 0); return INT_CONST; }
{Hexadecimal}   { yylval.int_val = strtol(yytext, nullptr, 0); return INT_CONST; }

{LogicalOperator} {
    std::string op = yytext;
    if (op == ">=") {
        return LOGICAL_OP_GREATER_EQUAL;
    } else if (op == "<=") {
        return LOGICAL_OP_LESS_EQUAL;
    } else if (op == "==") {
        return LOGICAL_OP_EQUAL;
    } else if (op == "!=") {
        return LOGICAL_OP_NOT_EQUAL;
    } else if (op == "||") {
        return LOGICAL_OP_OR;
    } else if (op == "&&") {
        return LOGICAL_OP_AND;
    } else if (op == ">") {
        return LOGICAL_OP_GREATER;
    } else if (op == "<") {
        return LOGICAL_OP_LESS;
    } else {
        assert(false);
    }
}   

.               { return yytext[0]; }

%%
