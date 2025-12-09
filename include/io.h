#ifndef IO_H
#define IO_H

#include "base.h"
#include <stdio.h>

typedef struct {
    u1* data;
    u4 size;
    u4 offset;
} Buffer;

Status buffer_from_file(const char* path, Buffer* buffer);
Status read_u1(Buffer* buffer, u1* value);
Status read_u2(Buffer* buffer, u2* value);
Status read_u4(Buffer* buffer, u4* value);
Status read_bytes(Buffer* buffer, u1* dest, u4 count);
u4 buffer_tell(Buffer* buffer);
void buffer_free(Buffer* buffer);

#endif // IO_H

