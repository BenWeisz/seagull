#include "io/log.h"

#include <stdarg.h>
#include <stdio.h>

void LOG(const char* message, ...) {
    va_list args;
    va_start(args, message);
    printf("[LOG]: ");
    vprintf(message, args);
    va_end(args);
}

void LOG_warn(const char* message, ...) {
    va_list args;
    va_start(args, message);
    fprintf(stderr, "[WARN]: ");
    vfprintf(stderr, message, args);
    va_end(args);
}

void LOG_error(const char* message, ...) {
    va_list args;
    va_start(args, message);
    fprintf(stderr, "[ERROR]: ");
    vfprintf(stderr, message, args);
    va_end(args);
}