#ifndef __CIZ__PARSER__PARSER_H
#define __CIZ__PARSER__PARSER_H

#include <stddef.h>

#include "../lexer/lexer.h"
#include "ast.h"

typedef struct parser
{
    token_t* tokens;
    size_t toknum;
    size_t index;

    struct
    {
        ast_proc_t* current_proc;
        context_t* current_ctx;
        int inside_str_expr;
    } state;
} parser_t;

parser_t parser_create(lexer_t* lexer);
ast_program_t parser_parse(parser_t* parser);

#endif /* !__CIZ__PARSER__PARSER_H */
