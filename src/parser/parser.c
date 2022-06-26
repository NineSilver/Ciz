#include <stdio.h>
#include <stdlib.h>

#include "ast.h"
#include "parser.h"

parser_t parser_create(lexer_t* lexer)
{
    parser_t parser = {0};

    token_t tok;
    while((tok = lexer_next_token(lexer)).kind != TOK_EOF)
    {
        if(tok.kind != TOK_UNKNOWN)
        {
            parser.tokens = realloc(parser.tokens, (parser.toknum + 1) * sizeof(token_t));
            parser.tokens[parser.toknum++] = tok;
        }
        else
        {
            fprintf(stderr, "ERROR: unknown token \"%.*s\" at %lu:%lu\n", (int)tok.text.len, tok.text.str, tok.line, tok.column);
            fprintf(stderr, "Note: make sure your program does not contain unicode characters such as symbols or emoji.\n");
        }
    }

    return parser;
}

static token_t parser_current(parser_t* parser)
{
    return parser->tokens[parser->index];
}

static token_t parser_peek(parser_t* parser, size_t off)
{
    if(parser->index + off >= parser->toknum) return parser->tokens[parser->toknum - 1];

    return parser->tokens[parser->index + off];
}

static void parser_advance(parser_t* parser)
{
    parser->index++;
}

static void parser_expect(parser_t* parser, tok_kind_t kind)
{
    token_t expected = parser_peek(parser, 1);
    if(expected.kind != kind)
    {
        fprintf(stderr, "ERROR: expected token %s, found token %s (%lu:%lu)\n", token_kind_to_str(kind), token_kind_to_str(expected.kind), expected.line, expected.column);
        exit(-1);
    }
}

static void parser_eat(parser_t* parser, tok_kind_t kind)
{
    token_t current = parser_current(parser);
    if(current.kind != kind)
    {
        fprintf(stderr, "ERROR: expected token %s, found token %s (%lu:%lu)\n", token_kind_to_str(kind), token_kind_to_str(current.kind), current.line, current.column);
        exit(-1);
    }
    parser_advance(parser);
}

static ast_expression_t* parser_parse_expression_0(parser_t* parser);

static ast_expression_t* parser_parse_expression_2(parser_t* parser)
{
    if(parser_current(parser).kind == TOK_LPAREN)
    {
        parser_advance(parser);
        ast_expression_t* expr = parser_parse_expression_0(parser);
        parser_eat(parser, TOK_RPAREN);

        return expr;
    }

    ast_expression_t* expression = calloc(1, sizeof(ast_expression_t));
    token_t current = parser_current(parser);
    switch(current.kind)
    {
        case TOK_NUMBER:
            expression->type = AST_EXPR_VALUE;
            expression->value.type = AST_VAL_UNSIGNED;
            expression->value._unsigned = strtoul(current.text.str, NULL, 10);
            parser_advance(parser);
            return expression;
        
        default:
            fprintf(stderr, "TODO %s\n", token_kind_to_str(current.kind));
            exit(-1);
    }
}

static ast_expression_t* parser_parse_expression_1(parser_t* parser)
{
    ast_expression_t* left = parser_parse_expression_2(parser);

    while(parser_current(parser).kind == TOK_STAR || parser_current(parser).kind == TOK_SLASH)
    {
        tok_kind_t op = parser_current(parser).kind;
        parser_advance(parser);

        ast_expression_t* right = parser_parse_expression_2(parser);

        ast_expression_t* expr = calloc(1, sizeof(ast_expression_t));
        expr->type = AST_EXPR_BINARY;
        expr->binary.left = left;
        expr->binary.right = right;
        expr->binary.op = op;

        left = expr;
    }

    return left;
}

static ast_expression_t* parser_parse_expression_0(parser_t* parser)
{
    ast_expression_t* left = parser_parse_expression_1(parser);
    while(parser_current(parser).kind == TOK_PLUS || parser_current(parser).kind == TOK_MINUS)
    {
        tok_kind_t op = parser_current(parser).kind;
        parser_advance(parser);

        ast_expression_t* right = parser_parse_expression_1(parser);

        ast_expression_t* expr = calloc(1, sizeof(ast_expression_t));
        expr->type = AST_EXPR_BINARY;
        expr->binary.left = left;
        expr->binary.right = right;
        expr->binary.op = op;

        left = expr;
    }

    return left;
}

static ast_expression_t* parser_parse_expression(parser_t* parser)
{
    return parser_parse_expression_0(parser);
}

static ast_statement_t* parser_parse_statement(parser_t* parser);

static ast_statement_t* parser_parse_if(parser_t* parser)
{
    parser_advance(parser); // eat if keyword without checking kind
    
    ast_statement_t* if_st = calloc(1, sizeof(ast_statement_t));
    if_st->type = AST_STMNT_IF;
    if_st->_if.cond = parser_parse_expression(parser);
    if_st->_if.body = parser_parse_statement(parser);

    if(parser_current(parser).kind == TOK_ELSE_KW)
    {
        parser_advance(parser);
        if_st->_if._else = parser_parse_statement(parser);
    }

    return if_st;
}

static ast_statement_t* parser_parse_var_decl(parser_t* parser)
{
    parser_expect(parser, TOK_IDENTIFIER);
    parser_advance(parser); // eat dim keyword without checking kind
    
    ast_statement_t* dim = calloc(1, sizeof(ast_statement_t));
    dim->type = AST_STMNT_VAR_DECL;
    dim->var_decl.name = parser_current(parser).text;
    
    parser_advance(parser);
    parser_eat(parser, TOK_EQUALS);

    dim->var_decl.value = parser_parse_expression(parser);
    parser_eat(parser, TOK_SEMICOLON);

    return dim;
}

static ast_statement_t* parser_parse_ret(parser_t* parser)
{
    parser_advance(parser);

    ast_statement_t* ret = calloc(1, sizeof(ast_statement_t));
    ret->type = AST_STMNT_RET;
    
    parser_eat(parser, TOK_SEMICOLON);
    
    return ret;
}

static ast_statement_t* parser_parse_block(parser_t* parser)
{
    parser_advance(parser); // eat do keyword without checking kind

    ast_statement_t* block = calloc(1, sizeof(ast_statement_t));
    block->type = AST_STMNT_BLOCK;

    token_t current = parser_current(parser);
    while(current.kind != TOK_END_KW && parser->index < parser->toknum)
    {
        block->block.statements = realloc(block->block.statements, (block->block.statement_num + 1) * sizeof(ast_statement_t*));
        block->block.statements[block->block.statement_num++] = parser_parse_statement(parser);
        current = parser_current(parser);
    }

    parser_eat(parser, TOK_END_KW);

    return block;
}

static ast_statement_t* parser_parse_statement(parser_t* parser)
{
    token_t curr = parser_current(parser);

    switch(curr.kind)
    {
        case TOK_DO_KW:
            return parser_parse_block(parser);
        
        case TOK_RET_KW:
            return parser_parse_ret(parser);
        
        case TOK_DIM_KW:
            return parser_parse_var_decl(parser);
        
        case TOK_IF_KW:
            return parser_parse_if(parser);

        default:
            ast_statement_t* expr = calloc(1, sizeof(ast_statement_t));
            expr->type = AST_STMNT_EXPR;
            expr->expr = parser_parse_expression(parser);
            parser_eat(parser, TOK_SEMICOLON);
            return expr;
    }
}

static ast_proc_t parser_parse_proc(parser_t* parser)
{
    ast_proc_t proc;
    
    parser_expect(parser, TOK_IDENTIFIER);
    parser_advance(parser); // eat proc keyword without checking kind

    proc.name = parser_current(parser).text;
    parser_advance(parser);

    parser_eat(parser, TOK_LPAREN);
    parser_expect(parser, TOK_DO_KW);
    parser_eat(parser, TOK_RPAREN);

    proc.body = parser_parse_block(parser);

    return proc;
}

ast_program_t parser_parse(parser_t* parser)
{
    ast_program_t program = {0};

    token_t tok = parser_current(parser);
    while(parser->index < parser->toknum)
    {
        if(tok.kind == TOK_PROC_KW)
        {
            program.procs = realloc(program.procs, (program.procnum + 1) * sizeof(ast_proc_t));
            program.procs[program.procnum++] = parser_parse_proc(parser);
        }

        tok = parser_current(parser);
    }

    return program;
}
