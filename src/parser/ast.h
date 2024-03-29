#ifndef __CIZ__PARSER__AST_H
#define __CIZ__PARSER__AST_H

#include <context.h>
#include <data.h>
#include <strview.h>

#include "../lexer/lexer.h"

typedef enum ast_val_type
{
    AST_VAL_STRING,
    AST_VAL_UNSIGNED
} ast_val_type_t;

typedef struct ast_value
{
    ast_val_type_t type;
    union
    {
        strview_t string;
        unsigned long _unsigned;
    };
} ast_value_t;

typedef enum ast_expr
{
    AST_EXPR_VALUE,
    AST_EXPR_UNARY,
    AST_EXPR_BINARY,
    AST_EXPR_ASSIGN,
    AST_EXPR_VAR_REF
} ast_expr_type_t;

typedef struct ast_expression ast_expression_t;
struct ast_expression
{
    ast_expr_type_t type;
    union
    {
        ast_value_t value;

        struct
        {
            tok_kind_t op;
            ast_expression_t* expr;
        } unary;
        
        struct
        {
            tok_kind_t op;
            ast_expression_t* left;
            ast_expression_t* right;
        } binary;

        struct
        {
            size_t var_idx;
            ast_expression_t* new_value;
        } assign;

        struct
        {
            size_t idx;
        } var_ref;
    };
};

typedef enum ast_stmnt_type
{
    AST_STMNT_BLOCK,
    AST_STMNT_RET,

    AST_STMNT_VAR_DECL,
    AST_STMNT_ASM,

    AST_STMNT_IF,
    AST_STMNT_WHILE,

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
            datatype_t type;
            size_t idx;
            ast_expression_t* value;
        } var_decl;

        struct
        {
            strview_t literal;
        } _asm;
        

        struct
        {
            ast_expression_t* cond;
            ast_statement_t* body;
            ast_statement_t* _else;
        } _if;
        
        struct
        {
            ast_expression_t* cond;
            ast_statement_t* body;
        } _while;

        ast_expression_t* expr;
    };
};

typedef struct ast_proc
{
    strview_t name;
    datatype_t ret_type;
    ast_statement_t* body;
    context_t* ctx;
    size_t label_num;
} ast_proc_t;

typedef struct ast_program
{
    ast_proc_t* procs;
    size_t procnum;
} ast_program_t;

#endif /* !__CIZ__PARSER__AST_H */
