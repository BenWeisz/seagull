#ifndef NESQUIK_LIST_H
#define NESQUIK_LIST_H

#include "types.h"

#include <stdlib.h>

#define LIST_MIN_CAPACITY 8

#define LIST_DECLARE(T)                                                                                 \
    typedef struct LIST_##T {                                                                           \
        u32 size;                                                                                       \
        u32 capacity;                                                                                   \
        T* data;                                                                                        \
    } LIST_##T;                                                                                         \
                                                                                                        \
    u8 LIST_##T##_init(LIST_##T* list, u32 capacity);                                                   \
    LIST_##T* LIST_##T##_create(u32 capacity);                                                          \
                                                                                                        \
    void LIST_##T##_deinit(LIST_##T* list);                                                             \
    void LIST_##T##_destroy(LIST_##T* list);                                                            \
                                                                                                        \
    u8 LIST_##T##_grow_capacity(LIST_##T* list);                                                        \
                                                                                                        \
    u8 LIST_##T##_push(LIST_##T* list, T v);                                                            \
    u8 LIST_##T##_pop(LIST_##T* list);                                                                  \
    u8 LIST_##T##_popv(LIST_##T* list, T* v);                                                           \
    u8 LIST_##T##_get(const LIST_##T* list, u32 i, T* v);                                               \
    u8 LIST_##T##_set(const LIST_##T* list, u32 i, T v);

#define LIST_DEFINE(T)                                                                                  \
    u8 LIST_##T##_init(LIST_##T* list, u32 capacity) {                                                  \
        if (list == NULL) return 0;                                                                     \
                                                                                                        \
        list->size = 0;                                                                                 \
        list->capacity = capacity < LIST_MIN_CAPACITY ? LIST_MIN_CAPACITY : capacity;                   \
                                                                                                        \
        list->data = (T*)malloc(sizeof(T) * list->capacity);                                            \
        if (list->data == NULL) {                                                                       \
            list->capacity = 0;                                                                         \
            return 0;                                                                                   \
        }                                                                                               \
                                                                                                        \
        return 1;                                                                                       \
    }                                                                                                   \
                                                                                                        \
    LIST_##T* LIST_##T##_create(const u32 capacity) {                                                   \
        LIST_##T* list = (LIST_##T*)malloc(sizeof(LIST_##T));                                           \
        if (list == NULL) {                                                                             \
            return NULL;                                                                                \
        }                                                                                               \
                                                                                                        \
        const u8 r = LIST_##T##_init(list, capacity);                                                   \
        if (r == 0) {                                                                                   \
            free(list);                                                                                 \
            return NULL;                                                                                \
        }                                                                                               \
                                                                                                        \
        return list;                                                                                    \
    }                                                                                                   \
                                                                                                        \
    void LIST_##T##_deinit(LIST_##T* list) {                                                            \
        if (list == NULL) return;                                                                       \
                                                                                                        \
        list->size = 0;                                                                                 \
        list->capacity = 0;                                                                             \
        if (list->data != NULL) {                                                                       \
            free(list->data);                                                                           \
            list->data = NULL;                                                                          \
        }                                                                                               \
    }                                                                                                   \
                                                                                                        \
    void LIST_##T##_destroy(LIST_##T* list) {                                                           \
        if (list == NULL) return;                                                                       \
                                                                                                        \
        LIST_##T##_deinit(list);                                                                        \
        free(list);                                                                                     \
    }                                                                                                   \
                                                                                                        \
    u8 LIST_##T##_grow_capacity(LIST_##T* list) {                                                       \
        if (list == NULL) return 0;                                                                     \
                                                                                                        \
        if (list->capacity == 0xFFFFFFFF) return 0;                                                     \
                                                                                                        \
        u32 capacity = 0;                                                                               \
        if (list->capacity == 0) {                                                                      \
            capacity = LIST_MIN_CAPACITY;                                                               \
        }                                                                                               \
        else if (list->capacity & (1 << 31)) capacity = 0xFFFFFFFF;                                     \
        else {                                                                                          \
            for (s32 i = 30; i >= 0; i--) {                                                             \
                if ((1 << i) & list->capacity) {                                                        \
                    capacity = 1 << (i + 1);                                                            \
                    break;                                                                              \
                }                                                                                       \
            }                                                                                           \
        }                                                                                               \
                                                                                                        \
        T* data = realloc(list->data, sizeof(T) * capacity);                                            \
        if (data == NULL) {                                                                             \
            return 0;                                                                                   \
        }                                                                                               \
        list->data = data;                                                                              \
        list->capacity = capacity;                                                                      \
                                                                                                        \
        return 1;                                                                                       \
    }                                                                                                   \
                                                                                                        \
    u8 LIST_##T##_push(LIST_##T* list, T v) {                                                           \
        if (list == NULL) return 0;                                                                     \
        if (list->size == 0xFFFFFFFF) return 0;                                                         \
                                                                                                        \
        if (list->size + 1 > list->capacity) {                                                          \
            const u8 r = LIST_##T##_grow_capacity(list);                                                \
            if (r == 0) return 0;                                                                       \
        }                                                                                               \
                                                                                                        \
        list->data[list->size++] = v;                                                                   \
        return 1;                                                                                       \
    }                                                                                                   \
                                                                                                        \
    u8 LIST_##T##_pop(LIST_##T* list) {                                                                 \
        if (list == NULL) return 0;                                                                     \
        if (list->size == 0) return 0;                                                                  \
                                                                                                        \
        list->size--;                                                                                   \
        return 1;                                                                                       \
    }                                                                                                   \
                                                                                                        \
    u8 LIST_##T##_popv(LIST_##T* list, T* v) {                                                          \
        if (list == NULL) return 0;                                                                     \
        if (list->size == 0) return 0;                                                                  \
                                                                                                        \
        *v = list->data[--list->size];                                                                  \
        return 1;                                                                                       \
    }                                                                                                   \
                                                                                                        \
    u8 LIST_##T##_get(const LIST_##T* list, u32 i, T* v) {                                              \
        if (list == NULL) return 0;                                                                     \
        if (i >= list->size) return 0;                                                                  \
                                                                                                        \
        *v = list->data[i];                                                                             \
        return 1;                                                                                       \
    }                                                                                                   \
                                                                                                        \
    u8 LIST_##T##_set(const LIST_##T* list, u32 i, T v) {                                               \
        if (list == NULL) return 0;                                                                     \
        if (i >= list->size) return 0;                                                                  \
                                                                                                        \
        list->data[i] = v;                                                                              \
        return 1;                                                                                       \
    }

#endif //NESQUIK_LIST_H