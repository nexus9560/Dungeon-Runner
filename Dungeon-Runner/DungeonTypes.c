#include "DungeonTypes.h"

const Dun_Vec up	= { -1,  0 };
const Dun_Vec right = {  0,  1 };
const Dun_Vec down	= {  1,  0 };
const Dun_Vec left	= {  0, -1 };
const Dun_Vec directions[4] = {up, right, down, left};

void DCQ_init(DCQ* instance, unsigned int capacity) {
    instance->head = 0;
    instance->tail = 0;
    instance->capacity = capacity;
    instance->items = malloc(sizeof(Dun_Coord*) * capacity);
}

void DCQ_destroy(Dun_Coord_Queue *dcq) {
    free(dcq->items);
}

bool DCQ_is_empty(Dun_Coord_Queue *dcq) {
    return dcq->head == dcq->tail;
}

unsigned int DCQ_size(Dun_Coord_Queue *dcq) {
    return (dcq->tail - dcq->head + dcq->capacity) % dcq->capacity;
}

Dun_Coord DCQ_pop(Dun_Coord_Queue *dcq) {
  if (dcq->head == dcq->tail) {
    return (Dun_Coord){XBOUND + 1, YBOUND + 1};
  }
  Dun_Coord coord = dcq->items[dcq->head];
  dcq->head = (dcq->head + 1) % dcq->capacity;
  return coord;
}
void DCQ_resize(Dun_Coord_Queue *dcq, unsigned int new_capacity) {
  if (dcq->capacity >= new_capacity) {
    return;
  }

  // perform resize
  dcq->items =(Dun_Coord *) realloc(dcq->items, new_capacity * sizeof(Dun_Coord));

  if (dcq->head > dcq->tail) {
    memcpy(&(dcq->items)[new_capacity - (dcq->head - dcq->capacity)],
           &(dcq->items)[dcq->head],
           (dcq->capacity - dcq->head) * sizeof(Dun_Coord));
    dcq->head = 0;
    dcq->tail = dcq->capacity - (dcq->head - dcq->tail);
  }
  dcq->capacity = new_capacity;
}

void DCQ_append(Dun_Coord_Queue *dcq, Dun_Coord coord) {
  if (dcq->head == (dcq->tail + 1) % dcq->capacity) {
    DCQ_resize(dcq, dcq->capacity * 2);
  }
  dcq->items[dcq->tail] = coord;
  dcq->tail = (dcq->tail + 1) % dcq->capacity;
}
