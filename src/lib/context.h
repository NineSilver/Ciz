#ifndef __CIZ__CONTEXT_H
#define __CIZ__CONTEXT_H

#include <stddef.h>

#include <strview.h>

#include "data.h"

typedef struct var
{
    strview_t name;
    datatype_t type;
} var_t;

typedef struct context context_t;
struct context
{
    var_t* decls;
    int* offsets;
    size_t decl_num;

    context_t* parent;
};

size_t context_add_var(context_t* ctx, var_t var);
int context_search_var(context_t* ctx, strview_t name, size_t* idx);

#endif /* !__CIZ__CONTEXT_H */
