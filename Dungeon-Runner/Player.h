#pragma once
#ifndef PLAYER_H
#define PLAYER_H
#include "DungeonTypes.h"
#include "Effect_Manager.h"
#include "list.h"

DR_LIST_DEF(Item)

typedef Item__List IL;

typedef struct {
	unsigned int arm, left; // if arm == 1 -> arm else leg, if left == 1 -> left else right
	Item armor;
	Item weapon;

} Limb;

DR_LIST_DEF(Limb)

typedef Limb__List Limblist;


typedef struct {
	Entity base;
	int stepCount;
	Item__List inventory; // Inventory of items
} Player;

extern Player you;

int player_init(void);

int add_Limb(Limb* limb);

int remove_Limb(Limb* limb);

int add_Item(Item* item);

int remove_Item(Item* item);

int equip_Item_to_Limb(Item* item, Limb* limb);

#endif
