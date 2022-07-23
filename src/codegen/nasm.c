#include <stdio.h>
#include <stdlib.h>

#include <context.h>
#include <file.h>

#include "nasm.h"

static void generate_value(FILE* stream, ast_value_t* value)
{
    switch(value->type)
    {
        case AST_VAL_UNSIGNED:
            fprintf(stream, "  mov rax, %lu\n", value->_unsigned);
            break;

        default:
            fprintf(stderr, "ERROR: value type %d generation is not implemented\n", value->type);
            exit(-1);
    }
}

static void generate_expression(FILE* stream, generator_t* gen, ast_expression_t* expression);

static void generate_binary_expression(FILE* stream, generator_t* gen, ast_expression_t* expression)
{
    generate_expression(stream, gen, expression->binary.right);
    fprintf(stream, "  push rdx\n");
    fprintf(stream, "  mov rdx, rax\n");
    generate_expression(stream, gen, expression->binary.left);

    switch(expression->binary.op)
    {
        case TOK_PLUS:
            fprintf(stream, "  add rax, rdx\n");
            break;

        case TOK_MINUS:
            fprintf(stream, "  sub rax, rdx\n");
            break;

        case TOK_STAR:
            fprintf(stream, "  mov rcx, rdx\n");
            fprintf(stream, "  mul rcx\n");
            break;

        case TOK_SLASH:
            fprintf(stream, "  mov rcx, rdx\n");
            fprintf(stream, "  div rcx\n");
            break;
        
        default:
            fprintf(stderr, "ERROR: unknown operand \"%s\"\n", token_kind_to_str(expression->binary.op));
            exit(-1);
    }

    fprintf(stream, "  pop rdx\n");
}

static void generate_expression(FILE* stream, generator_t* gen, ast_expression_t* expression)
{
    switch(expression->type)
    {
        case AST_EXPR_ASSIGN:
            generate_expression(stream, gen, expression->assign.new_value);
            fprintf(stream, "  mov [rbp + %d], rax\n", gen->curr_ctx->offsets[expression->assign.var_idx]);
            break;
            
        case AST_EXPR_BINARY:
            generate_binary_expression(stream, gen, expression);
            break;

        case AST_EXPR_VALUE:
            generate_value(stream, &expression->value);
            break;
        
        case AST_EXPR_VAR_REF:
            fprintf(stream, "  mov rax, [rbp + %d]\n", gen->curr_ctx->offsets[expression->var_ref.idx]);
            break;

        default:
            fprintf(stderr, "ERROR: expression type %d generation is not implemented\n", expression->type);
            exit(-1);
    }
}

static void generate_ret(FILE* stream, generator_t* gen, ast_statement_t* statement)
{
    if(statement->ret.expr) generate_expression(stream, gen, statement->ret.expr);

    fprintf(stream, "  jmp .%.*s_leave\n", (int)gen->curr_proc->name.len, gen->curr_proc->name.str);
}

static void generate_var_decl(FILE* stream, generator_t* gen, ast_statement_t* statement)
{
    if(statement->var_decl.value)
    {
        generate_expression(stream, gen, statement->var_decl.value);
        int offset = gen->curr_ctx->offsets[statement->var_decl.idx];
        fprintf(stream, "  mov [rbp + %d], rax\n", offset);
    }
}

static void generate_statement(FILE* stream, generator_t* gen, ast_statement_t* statement)
{
    switch(statement->type)
    {
        case AST_STMNT_BLOCK:
            for(size_t i = 0; i < statement->block.statement_num; i++)
                generate_statement(stream, gen, statement->block.statements[i]);
            break;
        
        case AST_STMNT_RET:
            generate_ret(stream, gen, statement);
            break;

        case AST_STMNT_VAR_DECL:
            generate_var_decl(stream, gen, statement);
            break;
        
        default:
            fprintf(stderr, "ERROR: statement type %d generation is not implemented\n", statement->type);
            exit(-1);
    }
}

static void generate_proc(FILE* stream, generator_t* gen, ast_proc_t* proc)
{
    gen->curr_proc = proc;
    gen->curr_ctx = proc->ctx;

    fprintf(stream, "global %.*s\n", (int)proc->name.len, proc->name.str);
    fprintf(stream, "%.*s:\n", (int)proc->name.len, proc->name.str);
    
    if(proc->ctx->decl_num)
    {
        fprintf(stream, "  push rbp\n");
        fprintf(stream, "  mov rbp, rsp\n");
        fprintf(stream, "  sub rsp, %lu\n", proc->ctx->decl_num * 8);
    }

    generate_statement(stream, gen, proc->body);
    
    fprintf(stream, ".%.*s_leave:\n", (int)proc->name.len, proc->name.str);
    if(proc->ctx->decl_num)
    {
        fprintf(stream, "  mov rsp, rbp\n");
        fprintf(stream, "  pop rbp\n");
    }

    fprintf(stream, "  ret\n\n");
}

void codegen_nasm(const char* output, ast_program_t* root)
{
    FILE* out = io_open_output(output);
    if(!out) return;

    generator_t* gen = calloc(1, sizeof(generator_t));

    for(size_t i = 0; i < root->procnum; i++)
        generate_proc(out, gen, &root->procs[i]);
    
    fclose(out);
}
