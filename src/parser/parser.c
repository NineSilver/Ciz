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

    // Include EOF token
    parser.tokens = realloc(parser.tokens, (parser.toknum + 1) * sizeof(token_t));
    parser.tokens[parser.toknum++] = tok;

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


static datatype_t expression_type(context_t* ctx, ast_expression_t* expression)
{
    switch(expression->type)
    {
        case AST_EXPR_ASSIGN:
            fprintf(stderr, "ERROR: assignment expression type is not implemented yet\n");
            exit(-1);
        case AST_EXPR_BINARY:
            return DATA_INT;
        case AST_EXPR_VALUE:
            break;
        case AST_EXPR_VAR_REF:
            return ctx->decls[expression->var_ref.idx].type;
    }

    switch(expression->value.type)
    {
        case AST_VAL_UNSIGNED: return DATA_INT;
        case AST_VAL_STRING: return DATA_STRING;
        default:
            fprintf(stderr, "ERROR: should not reach\n");
            exit(-1);
    }
}

static ast_expression_t* parser_parse_expression(parser_t* parser);
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
        case TOK_STRING:
            parser->state.inside_str_expr = 1;
            expression->type = AST_EXPR_VALUE;
            expression->value.type = AST_VAL_STRING;
            expression->value.string = current.text;
            parser_advance(parser);
            return expression;
        
        case TOK_NUMBER:
            expression->type = AST_EXPR_VALUE;
            expression->value.type = AST_VAL_UNSIGNED;
            expression->value._unsigned = strtoul(current.text.str, NULL, 10);
            parser_advance(parser);
            return expression;

        case TOK_IDENTIFIER:
        {
            token_t cur = parser_current(parser);
            size_t var_idx = 0;
            if(!context_search_var(parser->state.current_ctx, cur.text, &var_idx))
            {
                fprintf(stderr, "ERROR: reference to undeclared variable %.*s (%lu:%lu)\n", (int)cur.text.len, cur.text.str, cur.line, cur.column);
                exit(-1);
            }
            datatype_t type = parser->state.current_ctx->decls[var_idx].type;
            parser_advance(parser);
            if(parser_current(parser).kind == TOK_EQUALS)
            {
                ast_expression_t* new = parser_parse_expression(parser);
                datatype_t expr_type = expression_type(parser->state.current_ctx, new);
                if(type != expr_type)
                {
                    fprintf(stderr, "ERROR: impossible to assign value of type %d to variable of type %d (%lu:%lu)\n", expr_type, type, cur.line, cur.column);
                    exit(-1);
                }
                expression->type = AST_EXPR_ASSIGN;
                expression->assign.var_idx = var_idx;
                expression->assign.new_value = new;
            }
            else
            {
                expression->type = AST_EXPR_VAR_REF;
                expression->var_ref.idx = var_idx;
            }

            return expression;
        }
        
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
        if(parser->state.inside_str_expr)
        {
            fprintf(stderr, "ERROR: string literal may not be followed by a numeric operator (%lu:%lu)\n", parser_current(parser).line, parser_current(parser).column);
            exit(-1);
        }

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
        if(parser->state.inside_str_expr)
        {
            fprintf(stderr, "ERROR: string literal may not be followed by a numeric operator (%lu:%lu)\n", parser_current(parser).line, parser_current(parser).column);
            exit(-1);
        }

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
    ast_expression_t* expr = parser_parse_expression_0(parser);
    parser->state.inside_str_expr = 0;
    return expr;
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

    if(parser_current(parser).kind == TOK_AS)
    {
        parser_advance(parser);
        token_t curr = parser_current(parser);
        int type = keyword_to_datatype(curr);
        if(type < 0)
        {
            fprintf(stderr, "ERROR: expected type after token as-operator; got %s (%lu:%lu)\n", token_kind_to_str(curr.kind), curr.line, curr.column);
            exit(-1);
        }

        dim->var_decl.type = type;
        parser_advance(parser);
    }

    parser_eat(parser, TOK_EQUALS);

    size_t line = parser_current(parser).line, column = parser_current(parser).column;
    ast_expression_t* expr = parser_parse_expression(parser);
    if(dim->var_decl.type != 0)
    {
        if(expression_type(parser->state.current_ctx, expr) != dim->var_decl.type)
        {
            fprintf(stderr, "ERROR: expression type does not match variable type (%lu:%lu)\n", line, column);
            exit(-1);
        }
    }
    else
    {
        dim->var_decl.type = expression_type(parser->state.current_ctx, expr);
    }

    dim->var_decl.value = expr;
    parser_eat(parser, TOK_SEMICOLON);

    dim->var_decl.idx = context_add_var(parser->state.current_ctx, (var_t){ .name = dim->var_decl.name, .type = dim->var_decl.type });

    return dim;
}

static ast_statement_t* parser_parse_ret(parser_t* parser)
{
    if(!parser->state.current_proc)
    {
        fprintf(stderr, "ERROR: ret statements can only be placed inside of a procedure (%lu:%lu)\n", parser_current(parser).line, parser_current(parser).column);
        exit(-1);
    }

    parser_advance(parser);

    ast_statement_t* ret = calloc(1, sizeof(ast_statement_t));
    ret->type = AST_STMNT_RET;

    if(parser->state.current_proc->ret_type != DATA_VOID)
    {
        size_t line = parser_current(parser).line, column = parser_current(parser).column;
        ast_expression_t* expr = parser_parse_expression(parser);
        datatype_t type = expression_type(parser->state.current_ctx, expr);

        if(parser->state.current_proc->ret_type != type)
        {
            fprintf(stderr, "ERROR: expression type does not match parent procedure return type (%lu:%lu)\n", line, column);
            exit(-1);
        }

        ret->ret.expr = expr;
    }
    
    parser_eat(parser, TOK_SEMICOLON);
    
    return ret;
}

static ast_statement_t* parser_parse_block(parser_t* parser)
{
    parser_advance(parser); // eat do keyword without checking kind

    ast_statement_t* block = calloc(1, sizeof(ast_statement_t));
    block->type = AST_STMNT_BLOCK;

    token_t current = parser_current(parser);
    while(current.kind != TOK_END_KW && current.kind != TOK_EOF)
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
        {
            ast_statement_t* expr = calloc(1, sizeof(ast_statement_t));
            expr->type = AST_STMNT_EXPR;
            expr->expr = parser_parse_expression(parser);
            parser_eat(parser, TOK_SEMICOLON);
            return expr;
        }
    }
}

static ast_proc_t parser_parse_proc(parser_t* parser)
{
    ast_proc_t proc;
    
    parser_expect(parser, TOK_IDENTIFIER);
    parser_advance(parser); // eat proc keyword without checking kind

    proc.name = parser_current(parser).text;
    parser_advance(parser);

    if(parser_current(parser).kind == TOK_AS)
    {
        parser_advance(parser);

        token_t curr = parser_current(parser);
        int type = keyword_to_datatype(curr);
        if(type < 0)
        {
            fprintf(stderr, "ERROR: expected type after token as-operator; got %s (%lu:%lu)\n", token_kind_to_str(curr.kind), curr.line, curr.column);
            exit(-1);
        }
        parser_advance(parser);
        proc.ret_type = type;
    }
    else
    {
        proc.ret_type = DATA_VOID;
    }

    parser_eat(parser, TOK_LPAREN);
    parser_expect(parser, TOK_DO_KW);
    parser_eat(parser, TOK_RPAREN);

    proc.ctx = calloc(1, sizeof(context_t));
    parser->state.current_proc = &proc;
    parser->state.current_ctx = proc.ctx;

    proc.body = parser_parse_block(parser);

    parser->state.current_proc = NULL;

    return proc;
}

ast_program_t parser_parse(parser_t* parser)
{
    ast_program_t program = {0};

    token_t tok;
    while((tok = parser_current(parser)).kind != TOK_EOF)
    {
        if(tok.kind == TOK_PROC_KW)
        {
            program.procs = realloc(program.procs, (program.procnum + 1) * sizeof(ast_proc_t));
            program.procs[program.procnum++] = parser_parse_proc(parser);
        }
    }

    return program;
}
