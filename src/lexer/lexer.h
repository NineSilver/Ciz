#ifndef __CIZ__LEXER_H
#define __CIZ__LEXER_H

#include <stddef.h>

#include <strview.h>

typedef enum tok_kind
{
    TOK_PROC_KW,
    TOK_DO_KW,
    TOK_END_KW,
    TOK_RET_KW,
    TOK_DIM_KW,
    TOK_ASM_KW,
    TOK_IF_KW,
    TOK_ELSE_KW,
    TOK_WHILE_KW,

    TOK_INT_KW,

    TOK_IDENTIFIER,
    TOK_NUMBER,
    TOK_STRING,

    TOK_AS,
    TOK_PLUS,
    TOK_MINUS,
    TOK_STAR,
    TOK_SLASH,

    TOK_MODULO,

    TOK_ASSIGN,
    TOK_EQUALS,
    TOK_NOTEQ,

    TOK_LOG_NOT,
    TOK_LOG_AND,
    TOK_LOG_OR,

    TOK_LPAREN,
    TOK_RPAREN,
    TOK_SEMICOLON,

    TOK_UNKNOWN,
    TOK_EOF
} tok_kind_t;

typedef struct token {
    tok_kind_t kind;
    strview_t text;
    size_t line;
    size_t column;
} token_t;

typedef struct lexer
{
    strview_t src;
    size_t pos;
    size_t line;
    size_t column;
} lexer_t;

const char* token_kind_to_str(tok_kind_t kind);

lexer_t lexer_create(strview_t src);
token_t lexer_next_token(lexer_t* lexer);

#endif /* !__CIZ__LEXER_H */
