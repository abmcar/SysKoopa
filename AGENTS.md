# Project Guidelines

This repo contains a teaching compiler that translates SysY to Koopa IR and then to RISC-V. The directory layout is:

- `include/` – header files used across the project.
- `src/frontend/` – lexer (`sysy.l`), parser (`sysy.y`), AST definitions and symbol table management.
- `src/backend/` – translation from Koopa IR to RISC-V.
- `src/` – driver program (`main.cpp`) and utilities.
- `tests/` – sample SysY programs for regression testing.

## Testing

The current implementation supports integer constants, basic variable declarations, assignments, expression evaluation, and blocks with local scope. Other language features are not implemented yet.

To build the compiler and run test programs manually:

```bash
cmake -B build
cmake --build build
# Compile a SysY file to Koopa IR
build/compiler -koopa tests/basic/<file>.sy -o <file>.koopa
```

Inspect the generated `.koopa` output to verify correctness.

## Test cases

Sample programs reside in `tests/basic`:

- `expr_simple.sy` – arithmetic expression evaluation.
- `const_var.sy` – local constant declaration and usage.
- `assign_expr.sy` – variable assignment using an expression.
- `block_scope.sy` – nested block to check scope rules.
- `logical.sy` – logical and relational operators.

These files exercise the implemented features and should compile without errors.
