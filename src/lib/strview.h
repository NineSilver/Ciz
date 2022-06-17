#ifndef __CIZ__LIB__STRVIEW_H
#define __CIZ__LIB__STRVIEW_H

#include <stddef.h>

typedef struct stringview
{
    char* str;
    size_t len;
} strview_t;

strview_t strview_from_array(char* str);
strview_t strview_from_arr_len(char* str, size_t len);

strview_t strview_slice(strview_t str, size_t start, size_t end);
#define strview_slice_from(str, from) (strview_slice((str), (from), (str).len - (from))

int strview_compare(strview_t str1, strview_t str2);

inline char strview_at(strview_t str, size_t pos)
{
    if(pos >= str.len) return '\0';
    return str.str[pos];
}

#endif /* !__CIZ__LIB__STRVIEW_H */
