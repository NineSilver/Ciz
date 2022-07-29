#include <stdio.h>
#include <string.h>

#include <strview.h>

strview_t strview_from_array(char* str)
{
    return (strview_t){
        .str = str,
        .len = strlen(str)
    };
}

strview_t strview_from_arr_len(char* str, size_t len)
{
    return (strview_t){
        .str = str,
        .len = len
    };
}

strview_t strview_slice(strview_t str, size_t start, size_t end)
{
    return (strview_t){
        .str =  str.str + start,
        .len = end - start
    };
}

int strview_compare(strview_t str1, strview_t str2)
{
    if(str1.len != str2.len) return 0;

    for(size_t i = 0; i < str1.len; i++)
        if(str1.str[i] != str2.str[i]) return 0;
    
    return 1;
}

char strview_at(strview_t str, size_t pos)
{
    if(pos >= str.len) return '\0';
    return str.str[pos];
}
