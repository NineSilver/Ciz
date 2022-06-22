#include <stdio.h>
#include <stdlib.h>

#include <dump.h>
#include <file.h>
#include <strview.h>

#include "lexer/lexer.h"
#include "parser/ast.h"
#include "parser/parser.h"

static void usage(char* argv0)
{
    fprintf(stderr, "Usage: %s <file>\n", argv0);
}

int main(int argc, char* argv[])
{
    if(argc < 2)
    {
        usage(argv[0]);
        return -1;
    }

    size_t len;
    char* file = io_read_file(argv[1], &len);
    if(!file) return -1;

    lexer_t lexer = lexer_create(strview_from_arr_len(file, len));
    parser_t parser = parser_create(&lexer);

    ast_program_t program = parser_parse(&parser);
    dump_ast(stdout, &program);
    
    free(file);

    return 0;
}
