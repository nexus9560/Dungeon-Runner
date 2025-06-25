#include "Effect_Manager.h"
#include <stdio.h>
#include <stdlib.h>

Effect newEffect(int id, char* name, int duration, int strength, int type, int subType, char* description, int exclusive) {
	Effect effect;
	effect.id = id; // Unique ID for the master list
	effect.name = name;
	effect.duration = duration;
	effect.maxDuration = duration; // Initialize maxDuration to the same value as duration
	effect.strength = strength;
	effect.type = type;
	effect.subType = subType;
	effect.description = description;
	effect.exclusive = exclusive;
	return effect;
}

EffectList newMasterEffectList() {
	EffectList list;
	list.effects = malloc(0 * sizeof(Effect)); // Initialize with zero capacity
	if (list.effects == NULL) {
		printf("Error: Memory allocation failed for master effect list.\n");
		exit(EXIT_FAILURE);
	}
	list.effectCount = 0;
	list.maxEffects = 0; // No initial capacity
	list.size = 0;
	list.masterList = 1; // Indicate this is a master list
	return list;
}

EffectList newEffectList() {
	EffectList list;
	list.effects = malloc(0 * sizeof(Effect));
	if (list.effects == NULL) {
		printf("Error: Memory allocation failed for effect list.\n");
		exit(EXIT_FAILURE);
	}
	list.effectCount = 0;
	list.maxEffects = 0;
	list.size = 0;
	list.masterList = 0; // Initialize masterList to 0
	return list;
}

EffectList newEffectListWithCapacity(unsigned int capacity) {
	EffectList list;
	list.effects = malloc(capacity * sizeof(Effect));
	if (list.effects == NULL) {
		printf("Error: Memory allocation failed for effect list with capacity %u.\n", capacity);
		exit(EXIT_FAILURE);
	}
	list.effectCount = 0;
	list.maxEffects = capacity;
	list.size = 0;
	return list;
}

Effect getEffect(EffectList list, int id) {
	for (unsigned int i = 0; i < list.size; i++) {
		if (list.effects[i].id == id) {
			return list.effects[i];
		}
	}
	printf("Error: Effect with ID %d not found.\n", id);
	return (Effect) { 0 };
}

Effect getEffectByName(EffectList list, char* name) {
	for (unsigned int i = 0; i < list.size; i++) {
		if (strcmp(list.effects[i].name, name) == 0) {
			return list.effects[i];
		}
	}
	printf("Error: Effect with name '%s' not found.\n", name);
	return (Effect) { 0 };
}

Effect getEffectByMasterID(EffectList list, unsigned int masterID) {
	for (unsigned int i = 0; i < list.size; i++) {
		if (list.effects[i].masterListID == masterID) {
			return list.effects[i];
		}
	}
	printf("Error: Effect with master ID %u not found.\n", masterID);
	return (Effect) { 0 };
}

int isEffectInList(EffectList list, int id) {
	for (unsigned int i = 0; i < list.size; i++) {
		if (list.effects[i].id == id) {
			return 1; // Effect found
		}
	}
	return 0; // Effect not found
}

int isEffectInListByName(EffectList list, char* name) {
	for (unsigned int i = 0; i < list.size; i++) {
		if (strcmp(list.effects[i].name, name) == 0) {
			return 1; // Effect found
		}
	}
	return 0; // Effect not found
}



EffectList addEffect(EffectList list, Effect effect) {
	if (list.size == 0) {
		list.maxEffects = 4; // Start with a small capacity
		list.effects = realloc(list.effects, list.maxEffects * sizeof(Effect));
		if (list.effects == NULL) {
			printf("Error: Memory allocation failed while adding effect.\n");
			exit(EXIT_FAILURE);
		}
	}
	else if (list.size == list.maxEffects) {
		list.maxEffects *= 2; // Double the capacity
		list.effects = realloc(list.effects, list.maxEffects * sizeof(Effect));
		if (list.effects == NULL) {
			printf("Error: Memory allocation failed while resizing effect list.\n");
			exit(EXIT_FAILURE);
		}
	}
	list.effects[list.size] = effect;
	list.effectCount++;
	list.size++;
	return list;
}

EffectList copyEffectList(EffectList list) {
	EffectList newList;
	newList.effects = malloc(list.maxEffects * sizeof(Effect));
	if (newList.effects == NULL) {
		printf("Error: Memory allocation failed while copying effect list.\n");
		exit(EXIT_FAILURE);
	}
	newList.effectCount = list.effectCount;
	newList.maxEffects = list.maxEffects;
	newList.size = list.size;
	for (unsigned int i = 0; i < list.size; i++) {
		newList.effects[i] = list.effects[i];
	}
	return newList;
}

EffectList insertEffectAtIndex(EffectList list, Effect effect, unsigned int index) {
	if (index > list.size) {
		printf("Error: Index out of bounds while inserting effect.\n");
		return list;
	}
	if (list.size == list.maxEffects || (list.size + 1) == list.maxEffects) {
		list.maxEffects *= 2; // Double the capacity
		list.effects = realloc(list.effects, list.maxEffects * sizeof(Effect));
		if (list.effects == NULL) {
			printf("Error: Memory allocation failed while resizing effect list.\n");
			exit(EXIT_FAILURE);
		}
	}

	EffectList retList = newEffectList();
	for( unsigned int i = 0; i < index; i++) {
		addEffect(retList, getEffect(list,i));
	}
	addEffect(retList, effect);
	for (unsigned int i = index; i < list.size; i++) {
		addEffect(retList, getEffect(list,i));
	}
	return retList;
}

EffectList removeEffect(EffectList list, int id) {
	for (unsigned int i = 0; i < list.size; i++) {
		if (list.effects[i].id == id) {
			// Shift elements to the left
			for (unsigned int j = i; j < list.size - 1; j++) {
				list.effects[j] = list.effects[j + 1];
			}
			list.size--;
			list.effectCount--;
			break;
		}
	}
	return list;
}

EffectList removeEffectByName(EffectList list, char* name) {
	for (unsigned int i = 0; i < list.size; i++) {
		if (strcmp(list.effects[i].name, name) == 0) {
			// Shift elements to the left
			for (unsigned int j = i; j < list.size - 1; j++) {
				list.effects[j] = list.effects[j + 1];
			}
			list.size--;
			list.effectCount--;
			break;
		}
	}
	return list;
}

EffectList popEffect(EffectList list) {

	if(list.masterList) {
		printf("Error: Cannot pop effects from master effect list.\n");
		return list;
	}

	if (list.size == 0) {
		printf("Error: No effects to pop.\n");
		return list;
	}
	list.effects[list.size - 1] = (Effect){ 0 }; // Clear the last effect
	list.size--;
	list.effectCount--;
	return list;
}

EffectList clearEffectList(EffectList list) {
	if (list.effects == NULL) {
		printf("Error: Effect list is already empty.\n");
		return list;
	}
	if (list.masterList) {
		printf("Error: Cannot clear master effect list.\n");
		return list;
	}
	free(list.effects);
	list.effects = malloc(0 * sizeof(Effect));
	if (list.effects == NULL) {
		printf("Error: Memory allocation failed while clearing effect list.\n");
		exit(EXIT_FAILURE);
	}
	list.effectCount = 0;
	list.maxEffects = 0;
	list.size = 0;
	return list;
}

char* dumpEffects(EffectList list) {
	char* buffer = malloc(2048 * sizeof(char)); // Allocate a buffer for the output
	if (buffer == NULL) {
		printf("Error: Memory allocation failed while dumping effects.\n");
		exit(EXIT_FAILURE);
	}
	buffer[0] = '\0'; // Initialize the buffer
	for (unsigned int i = 0; i < list.size; i++) {
		char effectInfo[256];
		sprintf(effectInfo, "ID: %3d, Name: %s, Duration: %d, Strength: %d, Type: %d, SubType: %d, Description: %s, Exclusive: %d\n",
			list.effects[i].id, list.effects[i].name, list.effects[i].duration,
			list.effects[i].strength, list.effects[i].type, list.effects[i].subType,
			list.effects[i].description, list.effects[i].exclusive);
		strcat(buffer, effectInfo);
	}
	return buffer;
}

