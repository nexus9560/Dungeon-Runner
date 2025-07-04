#ifndef DR_LIST_H
#define DR_LIST_H

#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#define DR_LIST_TYPEDEF(TYPE) \
typedef struct { \
    size_t capacity; \
    size_t size; \
    TYPE* items; \
} TYPE##__List;

#define DR_LIST_INIT_DEF(TYPE) \
void TYPE##__List_init(TYPE##__List * l, size_t capacity);

#define DR_LIST_INIT_IMPL(TYPE) \
void TYPE##__List_init(TYPE##__List * l, size_t capacity) { \
    if (capacity == 0) { \
        capacity = 4; \
    } \
    l->capacity = capacity; \
    l->size = 0; \
    l->items = malloc(capacity * sizeof(TYPE)); \
}

// Function to check if the LIST is empty
#define DR_LIST_EMPTY_DEF(TYPE) \
bool TYPE##__List_empty(TYPE##__List* l);

#define DR_LIST_EMPTY_IMPL(TYPE) \
bool TYPE##__List_empty(TYPE##__List* l) { return (l->size == 0); }

#define DR_LIST_FULL_DEF(TYPE) \
bool TYPE##__List_full(TYPE##__List* l);

#define DR_LIST_FULL_IMPL(TYPE) \
bool TYPE##__List_full(TYPE##__List* l) { return (l->size == l->capacity); }

#define DR_LIST_PUSH_DEF(TYPE) \
void TYPE##__List_push(TYPE##__List* l, TYPE item);

#define DR_LIST_PUSH_IMPL(TYPE) \
void TYPE##__List_push(TYPE##__List* l, TYPE item) { \
    if (TYPE##__List_full(l)){ \
        size_t new_capacity = l->capacity * 2; \
        l->items = (TYPE *)realloc(l->items, new_capacity * sizeof(TYPE)); \
        l->capacity = new_capacity; \
    } \
    l->items[l->size++] = item; \
}

#define DR_LIST_POP_DEF(TYPE) \
bool TYPE##__List_pop(TYPE##__List* l, TYPE* item);

#define DR_LIST_POP_IMPL(TYPE) \
bool TYPE##__List_pop(TYPE##__List* l, TYPE* item) { \
    if (TYPE##__List_empty(l)){ \
        item = NULL; \
        return false;\
    } \
    if (item != NULL){\
        *item = l->items[l->size];\
    }\
    l->size--; \
    return true; \
}

#define DR_LIST_DESTROY_DEF(TYPE) \
void TYPE##__List_destroy(TYPE##__List* l);

#define DR_LIST_DESTROY_IMPL(TYPE) \
void TYPE##__List_destroy(TYPE##__List* l) { \
    free(l->items); \
    l->items = NULL; \
    l->size = 0; \
    l->capacity = 0; \
}


#define DR_LIST_DEF(TYPE) \
DR_LIST_TYPEDEF(TYPE) \
DR_LIST_INIT_DEF(TYPE) \
DR_LIST_EMPTY_DEF(TYPE) \
DR_LIST_FULL_DEF(TYPE) \
DR_LIST_PUSH_DEF(TYPE) \
DR_LIST_POP_DEF(TYPE) \
DR_LIST_DESTROY_DEF(TYPE)


#define DR_LIST_IMPL(TYPE) \
DR_LIST_INIT_IMPL(TYPE) \
\
DR_LIST_EMPTY_IMPL(TYPE) \
\
DR_LIST_FULL_IMPL(TYPE) \
\
DR_LIST_PUSH_IMPL(TYPE) \
\
DR_LIST_POP_IMPL(TYPE) \
\
DR_LIST_DESTROY_IMPL(TYPE)


#endif /* LIST_H */
