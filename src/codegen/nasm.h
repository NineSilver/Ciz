#ifndef __CIZ__CODEGEN__NASM_H
#define __CIZ__CODEGEN__NASM_H

#include "../parser/ast.h"

void codegen_nasm(const char* out, ast_program_t* ast_root);

#endif /* !__CIZ__CODEGEN__NASM_H */
