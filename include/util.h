#pragma once
#ifndef UTIL_H
#define UTIL_H

#include <cassert>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>

#include "ast.h"

class ExpAST;

void write_file(std::string file_name, std::string file_content);
std::string get_koopa_exp_reg(ExpAST *exp);
std::string get_koopa_logical_exp_reg(ExpAST *exp);
void pushup_exp_reg(ExpAST *exp, ExpAST *parent);
#endif // UTIL_H