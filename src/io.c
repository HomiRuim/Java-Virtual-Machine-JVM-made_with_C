#include "../include/io.h"
#include <stdlib.h>
#include <string.h>

Status buffer_from_file(const char* path, Buffer* buffer) {
    FILE* fp = fopen(path, "rb");
    if (!fp) {
        return ERR_FILE;
    }

    fseek(fp, 0, SEEK_END);
    long file_size = ftell(fp);
    fseek(fp, 0, SEEK_SET);

    if (file_size < 0) {
        fclose(fp);
        return ERR_FILE;
    }

    buffer->data = (u1*)malloc(file_size);
    if (!buffer->data) {
        fclose(fp);
        return ERR_MEMORY;
    }

    buffer->size = (u4)file_size;
    buffer->offset = 0;

    size_t bytes_read = fread(buffer->data, 1, file_size, fp);
    fclose(fp);

    if (bytes_read != (size_t)file_size) {
        free(buffer->data);
        buffer->data = NULL;
        return ERR_FILE;
    }

    return OK;
}

Status read_u1(Buffer* buffer, u1* value) {
    if (buffer->offset >= buffer->size) {
        return ERR_EOF;
    }
    *value = buffer->data[buffer->offset++];
    return OK;
}

Status read_u2(Buffer* buffer, u2* value) {
    if (buffer->offset + 1 >= buffer->size) {
        return ERR_EOF;
    }
    *value = (u2)((buffer->data[buffer->offset] << 8) | buffer->data[buffer->offset + 1]);
    buffer->offset += 2;
    return OK;
}

Status read_u4(Buffer* buffer, u4* value) {
    if (buffer->offset + 3 >= buffer->size) {
        return ERR_EOF;
    }
    *value = (u4)(
        (buffer->data[buffer->offset] << 24) |
        (buffer->data[buffer->offset + 1] << 16) |
        (buffer->data[buffer->offset + 2] << 8) |
        buffer->data[buffer->offset + 3]
    );
    buffer->offset += 4;
    return OK;
}

Status read_bytes(Buffer* buffer, u1* dest, u4 count) {
    if (buffer->offset + count > buffer->size) {
        return ERR_EOF;
    }
    memcpy(dest, buffer->data + buffer->offset, count);
    buffer->offset += count;
    return OK;
}

u4 buffer_tell(Buffer* buffer) {
    return buffer->offset;
}

void buffer_free(Buffer* buffer) {
    if (buffer && buffer->data) {
        free(buffer->data);
        buffer->data = NULL;
        buffer->size = 0;
        buffer->offset = 0;
    }
}

