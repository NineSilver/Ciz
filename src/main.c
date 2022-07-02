#include <getopt.h>
#include <stdio.h>
#include <stdlib.h>

#include <dump.h>
#include <file.h>
#include <strview.h>

#include "codegen/nasm.h"
#include "lexer/lexer.h"
#include "parser/ast.h"
#include "parser/parser.h"

static const char* outfile = NULL;

static void usage(const char* argv0)
{
    fprintf(stderr, "Usage: %s [OPTIONS] <file>\n", argv0);
    fprintf(stderr, "Options:\n");
    fprintf(stderr, "\t-h, --help\t\tshows this message\n");
    fprintf(stderr, "\t-o, --output <file>\twrite the output to the specified argument\n");
}

static void ciz_compile_file(const char* in, const char* out)
{
    size_t len;
    char* file = io_read_file(in, &len);
    if(!file) return;

    lexer_t lexer = lexer_create(strview_from_arr_len(file, len));
    parser_t parser = parser_create(&lexer);

    ast_program_t program = parser_parse(&parser);
    //dump_ast(stdout, &program);
    codegen_nasm(out, &program);
    free(file);
}

int main(int argc, char* argv[])
{
    if(argc < 2)
    {
        usage(argv[0]);
        return -1;
    }

    int c;
    for(;;)
    {
        static struct option long_options[] = {
            {"help", no_argument, NULL, 'h'},
            {"output", required_argument, NULL, 'o'},
            {0, 0, 0, 0}
        };

        int option_idx = 0;
        c = getopt_long(argc, argv, "ho:", long_options, &option_idx);

        if(c == -1) break;

        switch(c)
        {
            case 'h':
                usage(argv[0]);
                return -1;

            case 'o':
                outfile = optarg;
                break;

            case '?':
                break;

            default:
                return -1;
        }
    }

    if(!outfile)
    {
        fprintf(stderr, "ERROR: expected output file\n");
        usage(argv[0]);
        return -1;
    }

    if(optind < argc)
    {
        ciz_compile_file(argv[optind], outfile); // just one file for the moment
    }
    else
    {
        fprintf(stderr, "ERROR: no input files found\n");
        usage(argv[0]);
        return -1;
    }

    return 0;
}
