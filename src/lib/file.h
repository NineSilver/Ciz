#ifndef __CIZ__LIB__IO_H
#define __CIZ__LIB__IO_H

#include <strview.h>

char* io_read_file(const char* path, size_t* out_flen);
FILE* io_open_output(const char* path);

#endif /* !__CIZ__LIB__IO_H */
