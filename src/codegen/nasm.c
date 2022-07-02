#include <stdio.h>
#include <stdlib.h>

#include <file.h>

#include "nasm.h"

#define GET_EXPR_VALUE(expr)                                                                                    \
    switch((expr)->value.type)                                                                                  \
    {                                                                                                           \
        case AST_VAL_UNSIGNED:                                                                                  \
            break;                                                                                              \
                                                                                                                \
        default:                                                                                                \
            fprintf(stderr, "ERROR: statement type %d generation is not implemented\n", (expr)->value.type);    \
            exit(-1);                                                                                           \
    }                                                       \

static void generate_ret(FILE* stream, ast_statement_t* statement)
{
    if(statement->ret.expr)
    {
        GET_EXPR_VALUE(statement->ret.expr)
        fprintf(stream, "  mov rax, %lu\n", statement->ret.expr->value._unsigned);
    }

    fprintf(stream, "  ret\n");
}

static void generate_statement(FILE* stream, ast_statement_t* statement)
{
    switch(statement->type)
    {
        case AST_STMNT_BLOCK:
            for(size_t i = 0; i < statement->block.statement_num; i++)
                generate_statement(stream, statement->block.statements[i]);
            break;
        
        case AST_STMNT_RET:
            generate_ret(stream, statement);
            break;
        
        default:
            fprintf(stderr, "ERROR: statement type %d generation is not implemented\n", statement->type);
            exit(-1);
    }
}

static void generate_proc(FILE* stream, ast_proc_t* proc)
{
    fprintf(stream, "global %.*s\n", (int)proc->name.len, proc->name.str);
    fprintf(stream, "%.*s:\n", (int)proc->name.len, proc->name.str);
    generate_statement(stream, proc->body);
}

void codegen_nasm(const char* output, ast_program_t* root)
{
    FILE* out = io_open_output(output);
    if(!out) return;

    for(size_t i = 0; i < root->procnum; i++)
        generate_proc(out, &root->procs[i]);
    
    fclose(out);
}
