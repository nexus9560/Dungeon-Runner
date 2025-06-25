// Effect_Manager.h

#ifndef EFFECT_MANAGER_H
#define EFFECT_MANAGER_H
#pragma once

typedef struct {
	unsigned int id;
	unsigned int masterListID; // Unique ID for the master list, used for identifying effects across different lists
	char* name;
	int duration; // Duration of the effect in turns, -1 for permanent effects
	int maxDuration; // Maximum duration of the effect, used for stacking effects
	int strength; // Strength of the effect (e.g., damage, healing, buff)
	int type; // Type of effect (e.g., damage, heal, buff, debuff)
	int subType;
	char* description; // Description of the effect
	int exclusive; // 0 = not exclusive, 1 = exclusive (only one of this type can be active at a time)
} Effect;

typedef struct {
	Effect* effects; // Array of effects
	unsigned int effectCount; // Number of effects, functions as last entry index
	unsigned int maxEffects; // Maximum number of effects that can be stored
	unsigned int size; // Current size of the effect list
	unsigned int masterList;
} EffectList;

// Function prototypes
Effect newEffect(int id, char* name, int duration, int strength, int type, int subType, char* description, int exclusive);
EffectList newMasterEffectList();
EffectList newEffectList();
EffectList newEffectListWithCapacity(unsigned int capacity);
Effect getEffect(EffectList list, int id);
Effect getEffectByName(EffectList list, char* name);
EffectList addEffect(EffectList list, Effect effect);
EffectList copyEffectList(EffectList list);
EffectList insertEffectAtIndex(EffectList list, Effect effect, unsigned int index);
EffectList removeEffect(EffectList list, int id);
EffectList removeEffectByName(EffectList list, char* name);
EffectList popEffect(EffectList list);
EffectList clearEffectList(EffectList list);
int isEffectInList(EffectList list, int id);
int isEffectInListByName(EffectList list, char* name);
char* dumpEffects(EffectList list);

#endif