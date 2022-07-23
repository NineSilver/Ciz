#ifndef __CIZ__CODEGEN__NASM_H
#define __CIZ__CODEGEN__NASM_H

#include "../parser/ast.h"

typedef struct generator_state
{
    ast_proc_t* curr_proc;
    context_t* curr_ctx;
} generator_t;

void codegen_nasm(const char* out, ast_program_t* ast_root);

#endif /* !__CIZ__CODEGEN__NASM_H */
