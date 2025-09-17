#pragma once
#ifndef ENTITYLIST_H
#define ENTITYLIST_H
#include "DungeonTypes.h"

struct EntityList{
	Entity* entities; // Pointer to the array of entities
	unsigned int size; // Current number of entities in the list
	unsigned int capacity; // Maximum number of entities that can be stored in the list

};

typedef struct EntityList EL;

int EL_init(EL* list, unsigned int capacity);

int EL_destroy(EL* list);

int EL_add(EL* list, Entity entity);

void EL_remove(EL* list, unsigned int index);

int EL_get(EL* list, unsigned int index, Entity* entity);

int EL_set(EL* list, unsigned int index, Entity entity);

int EL_Pop(EL* list, Entity* entity);


#endif