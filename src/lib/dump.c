#include <stdio.h>

#include <data.h>
#include <dump.h>

static const char* tok_kind_to_str[27] = {
    [TOK_PROC_KW] = "proc keyword",
    [TOK_DO_KW] = "do keyword",
    [TOK_END_KW] = "end keyword",
    [TOK_RET_KW] = "ret keyword",
    [TOK_DIM_KW] = "dim keyword",
    [TOK_ASM_KW] = "asm keyword",
    [TOK_IF_KW] = "if keyword",
    [TOK_ELSE_KW] = "else keyword",
    [TOK_WHILE_KW] = "while keyword",

    [TOK_INT_KW] = "Int type keyword",

    [TOK_IDENTIFIER] = "identifier",
    [TOK_NUMBER] = "number",
    [TOK_STRING] = "string",

    [TOK_AS] = "as operator",
    [TOK_PLUS] = "plus operator",
    [TOK_MINUS] = "minus operator",
    [TOK_STAR] = "star",
    [TOK_SLASH] = "slash",

    [TOK_ASSIGN] = "assign",
    [TOK_EQUALS] = "equals",
    [TOK_NOTEQ] = "not equals",

    [TOK_LPAREN] = "left parentheses",
    [TOK_RPAREN] = "right parentheses",
    [TOK_SEMICOLON] = "semicolon",

    [TOK_UNKNOWN] = "unknown token",
    [TOK_EOF] = "end-of-file token"
};

static const char* op_to_sign(tok_kind_t op)
{
    switch(op)
    {
        case TOK_AS:
            return "::";

        case TOK_PLUS:
            return "+";

        case TOK_MINUS:
            return "-";

        case TOK_STAR:
            return "*";

        case TOK_SLASH:
            return "/";
        
        default:
            return "no-op";
    }
};

static const char* datatype_to_str[3] = {
    [DATA_VOID] = "void type",
    [DATA_INT] = "int type",
    [DATA_STRING] = "string type"
};

const char* token_kind_to_str(tok_kind_t kind)
{
    return tok_kind_to_str[kind];
}

void token_print(FILE* fp, token_t tok)
{
    fprintf(fp, "Token %s (%d) \"%.*s\" at (%lu:%lu)\n", token_kind_to_str(tok.kind), tok.kind, (int)tok.text.len, tok.text.str, tok.line, tok.column);
}

static void do_indent(FILE* stream, size_t indent)
{
    while(indent-- > 0) fputc(' ', stream);
}

static void dump_value(FILE* stream, ast_value_t value, size_t indent)
{
    switch(value.type)
    {
        case AST_VAL_STRING:
            do_indent(stream, indent);
            fprintf(stream, "-> \"%.*s\"\n", (int)value.string.len, value.string.str);
            break;

        case AST_VAL_UNSIGNED:
            do_indent(stream, indent);
            fprintf(stream, "-> %lu\n", value._unsigned);
            break;
        
        default:
            do_indent(stream, indent);
            fprintf(stream, "-> value ???\n");
            fprintf(stderr, "WARN: value type %d does not have a dump case\n", value.type);
            break;
    }
}

static void dump_expression(FILE* stream, ast_expression_t* expression, size_t indent)
{
    switch (expression->type)
    {
        case AST_EXPR_VALUE:
            dump_value(stream, expression->value, indent);
            break;

        case AST_EXPR_BINARY:
            do_indent(stream, indent);
            fprintf(stream, "-> %s\n", op_to_sign(expression->binary.op));
            dump_expression(stream, expression->binary.left, indent + 2);
            dump_expression(stream, expression->binary.right, indent + 2);
            break;

        default:
            do_indent(stream, indent);
            fprintf(stream, "-> expression ???\n");
            fprintf(stderr, "WARN: expression type %d does not have a dump case\n", expression->type);
            break;
    }
}

static void dump_statement(FILE* stream, ast_statement_t* statement, size_t indent)
{
    switch(statement->type)
    {
        case AST_STMNT_BLOCK:
            for(size_t i = 0; i < statement->block.statement_num; i++)
                dump_statement(stream, statement->block.statements[i], indent);
            
            break;

        case AST_STMNT_RET:
            do_indent(stream, indent);
            fprintf(stream, "-> ret\n");
            break;

        case AST_STMNT_VAR_DECL:
            do_indent(stream, indent);
            fprintf(stream, "-> dim %.*s :: %s\n", (int)statement->var_decl.name.len, statement->var_decl.name.str, datatype_to_str[statement->var_decl.type]);
            dump_expression(stream, statement->var_decl.value, indent + 2);
            break;

        case AST_STMNT_IF:
            do_indent(stream, indent);
            fprintf(stream, "-> if\n");
            dump_expression(stream, statement->_if.cond, indent + 2);
            dump_statement(stream, statement->_if.body, indent);
            if(statement->_if._else)
            {
                do_indent(stream, indent);
                fprintf(stream, "-> else\n");
                dump_statement(stream, statement->_if._else, indent + 2);
            }
            break;

        case AST_STMNT_EXPR:
            dump_expression(stream, statement->expr, indent);
            break;
        
        default:
            do_indent(stream, indent);
            fprintf(stream, "-> statement ???\n");
            fprintf(stderr, "WARN: statement type %d does not have a dump case\n", statement->type);
            break;
    }
}

static void dump_proc(FILE* stream, ast_proc_t proc)
{
    fprintf(stream, "proc %.*s :: %s:\n", (int)proc.name.len, proc.name.str, datatype_to_str[proc.ret_type]);
    dump_statement(stream, proc.body, 3);
}

void dump_ast(FILE* stream, ast_program_t* program)
{
    for(size_t i = 0; i < program->procnum; i++)
        dump_proc(stream, program->procs[i]);
}
