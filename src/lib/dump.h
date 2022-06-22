#ifndef __CIZ__LIB__DUMP_H
#define __CIZ__LIB__DUMP_H

#include "../lexer/lexer.h"
#include "../parser/ast.h"

const char* token_kind_to_str(tok_kind_t kind);
void token_print(FILE* fp, token_t tok);
void dump_ast(FILE* stream, ast_program_t* program);

#endif /* !__CIZ__LIB__DUMP_H */
