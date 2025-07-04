#ifndef QUEUE_H
#define QUEUE_H

#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#define QUEUE_TYPEDEF(TYPE) \
typedef struct { \
    size_t capacity; \
    size_t head; \
    size_t tail; \
    TYPE* items; \
} TYPE##__Queue;

#define QUEUE_INIT_DEF(TYPE) \
void TYPE##__Queue_init(TYPE##__Queue * q, size_t capacity);

#define QUEUE_INIT_IMPL(TYPE) \
void TYPE##__Queue_init(TYPE##__Queue * q, size_t capacity) { \
    q->capacity = capacity; \
    q->head = 0; \
    q->tail = 0; \
    q->items = malloc(capacity * sizeof(TYPE)); \
}

// Function to check if the queue is empty
#define QUEUE_EMPTY_DEF(TYPE) \
bool TYPE##__Queue_empty(TYPE##__Queue* q);

#define QUEUE_EMPTY_IMPL(TYPE) \
bool TYPE##__Queue_empty(TYPE##__Queue* q) { return (q->head == q->tail); }

#define QUEUE_FULL_DEF(TYPE) \
bool TYPE##__Queue_full(TYPE##__Queue* q);

#define QUEUE_FULL_IMPL(TYPE) \
bool TYPE##__Queue_full(TYPE##__Queue* q) { return (q->head == ((q->tail + 1) % q->capacity)); }

#define QUEUE_SIZE_DEF(TYPE) \
size_t TYPE##__Queue_size(TYPE##__Queue* q);

#define QUEUE_SIZE_IMPL(TYPE) \
size_t TYPE##__Queue_size(TYPE##__Queue* q) { return (q->tail - q->head) % q->capacity; }

#define QUEUE_ENQUEUE_DEF(TYPE) \
bool TYPE##__Queue_enqueue(TYPE##__Queue* q, TYPE item);

#define QUEUE_ENQUEUE_IMPL(TYPE) \
bool TYPE##__Queue_enqueue(TYPE##__Queue* q, TYPE item) { \
    if (TYPE##__Queue_full(q)){ \
        size_t new_capacity = q->capacity * 2; \
        q->items = (TYPE *)realloc(q->items, new_capacity * sizeof(TYPE)); \
        if (q->head > q->tail) { \
            memcpy( \
                &(q->items[q->head]), \
                &(q->items[q->head + q->capacity]), \
                (q->capacity - q->head) * sizeof(TYPE) \
            ); \
            q->head += q->capacity; \
        } \
        q->capacity = new_capacity; \
    } \
    q->items[q->tail] = item; \
    q->tail = (q->tail + 1) % q->capacity; \
    return true; \
}

#define QUEUE_DEQUEUE_DEF(TYPE) \
bool TYPE##__Queue_dequeue(TYPE##__Queue* q, TYPE* item);

#define QUEUE_DEQUEUE_IMPL(TYPE) \
bool TYPE##__Queue_dequeue(TYPE##__Queue* q, TYPE* item) { \
    if (TYPE##__Queue_empty(q)){ \
        item = NULL; \
        return false;\
    } \
    *item = q->items[q->head]; \
    q->head = (q->head + 1) % q->capacity; \
    return true; \
}

#define QUEUE_PEEK_DEF(TYPE) \
bool TYPE##__Queue_peek(TYPE##__Queue* q, TYPE* item);

#define QUEUE_PEEK_IMPL(TYPE) \
bool TYPE##__Queue_peek(TYPE##__Queue* q, TYPE* item) { \
    if (TYPE##__Queue_empty(q)){ \
        item = NULL; \
        return false;\
    } \
    *item = q->items[q->head]; \
    return true; \
}

#define QUEUE_DESTROY_DEF(TYPE) \
void TYPE##__Queue_destroy(TYPE##__Queue* q);

#define QUEUE_DESTROY_IMPL(TYPE) \
void TYPE##__Queue_destroy(TYPE##__Queue* q) { \
    free(q->items); \
    q->items = NULL; \
    q->capacity = 0; \
    q->head = 0; \
    q->tail = 0; \
}


#define QUEUE_DEF(TYPE) \
QUEUE_TYPEDEF(TYPE) \
QUEUE_INIT_DEF(TYPE) \
QUEUE_EMPTY_DEF(TYPE) \
QUEUE_FULL_DEF(TYPE) \
QUEUE_SIZE_DEF(TYPE) \
QUEUE_ENQUEUE_DEF(TYPE) \
QUEUE_DEQUEUE_DEF(TYPE) \
QUEUE_PEEK_DEF(TYPE) \
QUEUE_DESTROY_DEF(TYPE)


#define QUEUE_IMPL(TYPE) \
QUEUE_INIT_IMPL(TYPE) \
\
QUEUE_EMPTY_IMPL(TYPE) \
\
QUEUE_FULL_IMPL(TYPE) \
\
QUEUE_SIZE_IMPL(TYPE) \
\
QUEUE_ENQUEUE_IMPL(TYPE) \
\
QUEUE_DEQUEUE_IMPL(TYPE) \
\
QUEUE_PEEK_IMPL(TYPE) \
\
QUEUE_DESTROY_IMPL(TYPE)


#endif /* QUEUE_H */
