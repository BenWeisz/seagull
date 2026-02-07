#include "../../include/io/io.h"

#include <stdlib.h>
#include <stdio.h>

#include "io/log.h"

// There are a couple of shoty things with the function
// but, it should be good for files smaller than ~4Gb
u8* IO_read_u8_padded(const char* path, u32* size, const u32 padding) {
    if (size != NULL) *size = 0;

    FILE* file = fopen(path, "rb");
    if (file == NULL) {
        LOG_error("Could not open file %s\n", path);
        fclose(file);
        return NULL;
    }

    // Figure out the size of the file
    s32 r = fseek(file, 0, SEEK_END);
    if (r != 0) {
        LOG_error("Could not seek file %s\n", path);
        fclose(file);
        return NULL;
    }

    const s64 len = ftell(file);
    if (len == -1) {
        LOG_error("Could not read file %s\n", path);
        fclose(file);
        return NULL;
    }

    r = fseek(file, 0, SEEK_SET);
    if (r != 0) {
        LOG_error("Could not seek file %s\n", path);
        fclose(file);
        return NULL;
    }

    // Allocate memory for the file data
    u8* data = (u8*)malloc(sizeof(u8) * (len + padding));
    if (data == NULL) {
        LOG_error("Could not allocate memory for file %s\n", path);
        fclose(file);
        return NULL;
    }

    // Read the file data
    const u64 read_len = fread(data, sizeof(u8), len, file);
    if ((s64)read_len != len) {
        LOG_error("Could not read file %s\n", path);
        fclose(file);
        return NULL;
    }

    if (size != NULL) *size = (u32)read_len;

    return data;
}

u8* IO_read_u8(const char* path, u32* size) {
    return IO_read_u8_padded(path, size, 0);
}

char* IO_read_char(const char* path, u32* size) {
    u32 local_size;
    char* data = (char*)IO_read_u8_padded(path, &local_size, 1);
    if (data == NULL) return NULL;

    data[local_size] = '\0';
    if (size != NULL) *size = local_size;

    return data;
}