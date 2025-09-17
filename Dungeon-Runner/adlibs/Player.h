#pragma once
#ifndef PLAYER_H
#define PLAYER_H
#include "DungeonTypes.h"
#include "Effect_Manager.h"
#include "list.h"

typedef struct {
	Entity base;
	int stepCount;
	IL inventory; // Inventory of items
	char* last_action;
	SegList segments; // Segments of the player, each segment can have multiple limbs
} Player;

extern Player you;

int player_init(Player* p);

int add_Limb(Limb* limb, Segment* seg);

int remove_Limb(Limb* limb, Segment* seg);

int add_Item_to_Inventory(Player* p, Item* item);

int remove_Item_from_Inventory(Player* p, Item* item);

int equip_Item_to_Limb(Item* item, Limb* limb);

int unequip_Item_from_Limb(Item* item, Limb* limb);

#endif
