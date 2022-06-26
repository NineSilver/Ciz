#ifndef __CIZ__PARSER__AST_H
#define __CIZ__PARSER__AST_H

#include <strview.h>

#include "../lexer/lexer.h"

typedef enum ast_val_type
{
    AST_VAL_UNSIGNED
} ast_val_type_t;

typedef struct ast_value
{
    ast_val_type_t type;
    union
    {
        unsigned int _unsigned;
    };
} ast_value_t;

typedef enum ast_expr
{
    AST_EXPR_VALUE,
    AST_EXPR_BINARY
} ast_expr_type_t;

typedef struct ast_expression ast_expression_t;
typedef struct ast_expression
{
    ast_expr_type_t type;
    union
    {
        ast_value_t value;
        
        struct
        {
            tok_kind_t op;
            ast_expression_t* left;
            ast_expression_t* right;
        } binary;
    };
} ast_expression_t;

typedef enum ast_stmnt_type
{
    AST_STMNT_BLOCK,
    AST_STMNT_RET,

    AST_STMNT_VAR_DECL,

    AST_STMNT_IF,

    AST_STMNT_EXPR
} ast_stmnt_type_t;

typedef struct ast_statement ast_statement_t;
struct ast_statement
{
    ast_stmnt_type_t type;
    union
    {
        struct
        {
            ast_statement_t** statements;
            size_t statement_num;
        } block;

        struct
        {
            ast_expression_t* expr; 
        } ret;

        struct
        {
            strview_t name;
            ast_expression_t* value;
        } var_decl;

        struct
        {
            ast_expression_t* cond;
            ast_statement_t* body;
            ast_statement_t* _else;
        } _if;
        

        ast_expression_t* expr;
    };
};

typedef struct ast_proc
{
    strview_t name;
    ast_statement_t* body;
} ast_proc_t;

typedef struct ast_program
{
    ast_proc_t* procs;
    size_t procnum;
} ast_program_t;

#endif /* !__CIZ__PARSER__AST_H */
