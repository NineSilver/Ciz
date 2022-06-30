#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <file.h>

char* io_read_file(const char* path, size_t* out_flen)
{
    FILE* fp = fopen(path, "r");
    if(!fp)
    {
        fprintf(stderr, "ERROR: unable to open file %s: %s\n", path, strerror(errno));
        return NULL;
    }

    fseek(fp, 0, SEEK_END);
    size_t size = *out_flen = ftell(fp);
    rewind(fp);

    char* content = malloc((size + 1) * sizeof(char));
    if(!content)
    {
        fputs("ERROR: system ran out of memory", stderr);
        fclose(fp);
        return NULL;
    }

    if(fread(content, sizeof(char), size, fp) != size)
    {
        fprintf(stderr, "ERROR: 'something' was triggered while reading the file %s: %s\n", path, strerror(ferror(fp)));
        fclose(fp);
        free(content);
        return NULL;
    }

    fclose(fp);
    return content;
}

FILE* io_open_output(const char* path)
{
    FILE* out = fopen(path, "w");
    if(!out)
    {
        fprintf(stderr, "ERROR: unable to open output file %s: %s\n", path, strerror(errno));
        return NULL;
    }

    return out;
}
