#include <stdio.h>
#include <stdlib.h>

#include <file.h>
#include <strview.h>

#include "lexer/lexer.h"

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
    token_t tok;

    while((tok = lexer_next_token(&lexer)).kind != TOK_EOF)
        token_print(stdout, tok);
    
    free(file);

    return 0;
}
