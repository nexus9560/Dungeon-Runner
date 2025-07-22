#include "EntityList.h"

int EL_init(EL* list, unsigned int capacity) {
	if (capacity == 0) {
		list->capacity = 4;
		list->entities = malloc(list->capacity * sizeof(Entity));
		return 1;
	}

	list->entities = (Entity*)malloc(capacity * sizeof(Entity));
	if (!list->entities) return -1; // Memory allocation failed
	list->size = 0;
	list->capacity = capacity;
	return 1; // Success
}

int EL_destroy(EL* list) {
	if (list->entities) {
		free(list->entities);
		list->entities = NULL;
	}
	list->size = 0;
	list->capacity = 0;
	return 1; // Success
}

int EL_add(EL* list, Entity entity) {
	if (list->size >= list->capacity) {
		// Resize the array if necessary
		unsigned int new_capacity = list->capacity * 2;
		Entity* new_entities = realloc(list->entities, new_capacity * sizeof(Entity));
		if (!new_entities) return -1; // Memory allocation failed
		list->entities = new_entities;
		list->capacity = new_capacity;
	}
	list->entities[list->size++] = entity; // Add the new entity and increment size
	return 1; // Success
}

int EL_get(EL* list, unsigned int index, Entity* entity) {
	if (index >= list->size) return -1; // Index out of bounds
	*entity = list->entities[index]; // Copy the entity to the provided pointer
	return 1; // Success
}

int EL_set(EL* list, unsigned int index, Entity entity) {
	if (index >= list->size) return -1; // Index out of bounds
	list->entities[index] = entity; // Set the entity at the specified index
	return 1; // Success
}

void EL_remove(EL* list, unsigned int index) {
	if (index >= list->size) return; // Index out of bounds
	// Shift entities to the left to remove the entity at the specified index
	for (unsigned int i = index; i < list->size - 1; i++) {
		list->entities[i] = list->entities[i + 1];
	}
	list->size--; // Decrease the size of the list
}

int EL_Pop(EL* list, Entity* entity) {
	if (list->size == 0) return -1; // List is empty
	*entity = list->entities[--list->size]; // Get the last entity and decrease size
	return 1; // Success
}