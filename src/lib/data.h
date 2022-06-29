#ifndef __CIZ__DATA_H
#define __CIZ__DATA_H

#include "../lexer/lexer.h"

typedef enum datatype
{
    DATA_VOID,
    DATA_INT,
    DATA_STRING
} datatype_t;

int datatype_check(token_t token);
int keyword_to_datatype(token_t token);

#endif /* !__CIZ__DATA_H */
