#include "DungeonTypes.h"
#include "list.h"

const Dun_Vec up	= { -1,  0 };
const Dun_Vec right = {  0,  1 };
const Dun_Vec down	= {  1,  0 };
const Dun_Vec left	= {  0, -1 };
const Dun_Vec directions[4] = { { -1, 0 }, { 0, 1 }, { 1, 0 }, { 0, -1 } };

DR_LIST_IMPL(Entity)

Dun_Vec getVector(Dun_Coord start, Dun_Coord end) {
    Dun_Vec delta;
    delta.dx = end.x - start.x;
    delta.dy = end.y - start.y;
    return delta;
}

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

char* Entity_tostring(Entity ent) {
    char* buffer = malloc(256 * sizeof(char));
    snprintf(buffer, 256, "[HP:%3d,ATK:%3d,AGR:%3d,TOH:%3d,DEF:%3d,EXP:%3d,EVA:%3d,LVL:%3d]:%32s\n",
             ent.health, ent.atk, ent.agr, ent.hit, ent.def, ent.exp, ent.eva, ent.level, ent.name);
    return buffer;
}

char* Entity__List_tostring(Entity__List *l) {
    char* buffer = malloc(l->size * 256 * sizeof(char));
    buffer[0] = '\0'; // Initialize the buffer
    for (unsigned int i = 0; i < l->size; i++) {
        char * as_string = Entity_tostring(l->items[i]);
        strncat(buffer, as_string, 256);
        free(as_string);
    }
    return buffer;
}
