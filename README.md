# SysKoopa

A teaching compiler that translates **SysY** language to **Koopa IR** and then to **RISC-V Assembly**.  
Developed as part of the compiler course project.

## 🧠 Project Overview

This project implements a two-phase compiler pipeline:

1. **Frontend**: Parses and compiles SysY code into Koopa IR  
2. **Backend**: Translates Koopa IR into RISC-V assembly

Koopa IR is a simplified intermediate representation designed specifically for educational use, inspired by LLVM IR.

## 📦 Directory Structure
```
SysKoopa/
├── frontend/         # SysY → Koopa IR
├── backend/          # Koopa IR → RISC-V
├── CMakeLists.txt    # Build configuration
└── README.md
```
## 🚀 Usage

### Build

```bash
cmake -B build
cmake --build build
```
### Compile SysY code
```bash
build/compiler -koopa hello.c -o hello.koopa
build/compiler -riscv hello.c -o hello.s
```
Note: You can also use flags like -emit-koopa or -emit-riscv depending on your implementation.

## 📚 Dependencies
 - C++17 or later
 - CMake ≥ 3.15
 - Provided Koopa runtime library

## 🧪 Test Cases

Test cases are located in the tests/ directory. Use the provided script to run tests:
```bash
autotest -koopa -s lv1 .
```
## 🎓 Course Context

This compiler is developed for the [Compiler Principles Course] and focuses on hands-on implementation of core compiler components:
 - Lexical & syntax analysis
 - Intermediate representation (Koopa IR)
 - Code generation for RISC-V

## 📖 License

This project is intended for educational purposes. All rights reserved to the course instructors and authors.
