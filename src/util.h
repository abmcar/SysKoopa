#pragma once
#include <cassert>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>

#include "ast.h"
#include "koopa.h"

void write_file(std::string file_name, std::string file_content);
std::string convert_ir_riscv(std::string irs);
void Visit(const koopa_raw_program_t &program, std::stringstream &oss);
void Visit(const koopa_raw_slice_t &slice, std::stringstream &oss);
void Visit(const koopa_raw_function_t &func, std::stringstream &oss);
void Visit(const koopa_raw_basic_block_t &bb, std::stringstream &oss);
void Visit(const koopa_raw_value_t &value, std::stringstream &oss);
void Visit(const koopa_raw_return_t &ret, std::stringstream &oss);
int32_t get_value(const koopa_raw_value_t val);
void Visit(const koopa_raw_integer_t &i32, std::stringstream &oss);
