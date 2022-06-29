#include <data.h>

int datatype_check(token_t token)
{
    switch(token.kind)
    {
        case TOK_NUMBER: return DATA_INT;
        case TOK_STRING: return DATA_STRING;
        default: return -1;
    }
}

int keyword_to_datatype(token_t token)
{
    switch(token.kind)
    {
        case TOK_INT_KW: return DATA_INT;
        default: return -1;
    }
}
