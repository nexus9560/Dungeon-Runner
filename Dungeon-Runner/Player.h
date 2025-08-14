#pragma once
#ifndef PLAYER_H
#define PLAYER_H
#include "DungeonTypes.h"
#include "Effect_Manager.h"
#include "list.h"

DR_LIST_DEF(Item)

typedef struct {
	Entity base;
	int stepCount;
	Item__List inventory; // Inventory of items
} Player;

extern Player you;

#endif
