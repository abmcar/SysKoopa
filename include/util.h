#pragma once
#include <cassert>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>

#include "ast.h"
#include "koopa.h"

void write_file(std::string file_name, std::string file_content);
