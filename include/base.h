#ifndef BASE_H
#define BASE_H

#include <stdint.h>

typedef uint8_t u1;
typedef uint16_t u2;
typedef uint32_t u4;

typedef enum {
    OK = 0,
    ERR_EOF = 1,
    ERR_BOUNDS = 2,
    ERR_MEMORY = 3,
    ERR_FILE = 4
} Status;

#endif // BASE_H

