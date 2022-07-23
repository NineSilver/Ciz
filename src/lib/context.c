#include <stdlib.h>

#include "context.h"

size_t context_add_var(context_t* ctx, var_t var)
{
    ctx->decls = realloc(ctx->decls, (ctx->decl_num + 1) * sizeof(var_t));
    ctx->offsets = realloc(ctx->offsets, (ctx->decl_num + 1) * sizeof(int));
    ctx->decls[ctx->decl_num] = var;
    ctx->offsets[ctx->decl_num] = (1 + ctx->decl_num) * -8;
    return ctx->decl_num++;
}

int context_search_var(context_t* ctx, strview_t name, size_t* idx)
{
    for(size_t i = 0; i < ctx->decl_num; i++)
    {
        if(strview_compare(ctx->decls[i].name, name))
            *idx = i;
        
        return 1;
    }

    return 0;
}
