#ifndef SEAGULL_IO_H
#define SEAGULL_IO_H

#include "../types.h"

#ifdef SEAGULL_RELEASE
#define IO_RESOURCE(x) x
#else
#define IO_RESOURCE(x) "../"x
#endif

u8* IO_read_u8_padded(const char* path, u32* size, const u32 padding);
u8* IO_read_u8(const char* path, u32* size);

char* IO_read_char(const char* path, u32* size);

#endif //SEAGULL_IO_H