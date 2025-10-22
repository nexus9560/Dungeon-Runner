#pragma once
#ifndef DUNGEONTYPES_H
#define DUNGEONTYPES_H


#ifdef _MSC_VER
#define _CRT_SECURE_NO_WARNINGS
#endif



#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <stdlib.h>
#include <ctype.h>
#include <time.h>
#include <math.h>
#include <limits.h>
#include "Effect_Manager.h"
#include "list.h"

#define XBOUND 256
#define YBOUND 512
#define DEBUG 1
#define MAX_ITEMS 10
#define PLAYER_INVENTORY_BASE 32
#define OLD_ACTIONS 0
#define LOG_BUFFER 4
#define inRange(x, min, max) ((x) >= (min) && (x) < (max)) // Check if x is in the range [min, max)
#define inRangeInclusive(x, min, max) ((x) >= (min) && (x) <= (max)) // Check if x is in the range [min, max)
#define inRangeExclusive(x, min, max) ((x) > (min) && (x) < (max)) // Check if x is in the range (min, max)
#define matchSign(x, y) (((x) < 0 && (y) < 0) || ((x) >= 0 && (y) >= 0)) // Check if x and y have the same sign
#define BUFFER 4 // Buffer to check for room collisions and out of bounds cases.
#define elif else if

typedef struct {
	unsigned int width;
	unsigned int height;
	signed int offset_x;
	signed int offset_y;
	char** content;
} Render_Window;

typedef Render_Window RW;

extern RW currentRenderWindow;

int initRenderWindow(RW* renwin, unsigned int width, unsigned int height, signed int offset_x, signed int offset_y);
int resizeRenderWindow(RW* renwin, unsigned int new_width, unsigned int new_height);

typedef struct {
	int dx;
	int dy;
} Dun_Vec;

typedef struct {
	unsigned int x;
	unsigned int y;
} Dun_Coord;

int isInARoom(Dun_Coord d);

Dun_Vec getVector(Dun_Coord start, Dun_Coord end);

int getKeyPress();

typedef struct {
	char name[32];
	char stat[3];
	int bonus;
	int type; // 0 = weapon, 1 = armor, 2 = consumable
	int equipped; // 0 = not equipped, 1 = equipped
	unsigned int sortingID; // used for sorting
	unsigned int id; // Item ID
} Item;

typedef struct {
	Dun_Coord loc;
	Item* item;
} Item_on_Ground;

DR_LIST_DEF(Item_on_Ground)

typedef Item_on_Ground__List IOG_List;

typedef enum {
	WEAPON = 0,
	ARMOR = 1,
	CONSUMABLE = 2,
} ItemType;

// This is used for determining Item EquipID, if it's the head, it's always 0, if it's equipped to a segment,
// it's 10 x segment ID, if it's equipped to a limb, it's segment ID + (limb ID * 1)
// there should not be more than 10 limbs to a segment
// further, the head is always 0, there should never be more than 1 head.

typedef enum {
	HEAD = 0,
	SEGMENT = 10,
	LIMB = 1
} EquipmentSlot;

DR_LIST_DEF(Item)

typedef Item__List IL;

typedef struct {
	unsigned int arm, left; // if arm == 1 -> arm else leg, if left == 1 -> left else right
	Item* armor;
	Item* weapon;
	int limID;

} Limb;

DR_LIST_DEF(Limb)

typedef Limb__List Limblist;

typedef struct {
	Limblist limbs;
	Item* armor;
	int segID;
	char name[32];
} Segment;

DR_LIST_DEF(Segment)

typedef Segment__List SegList;

typedef struct {
	Item* armor; // Pointer to the armor item
	int hID;
} Head;

typedef struct {
	Dun_Coord location;
	char name[32];
	int health;
	int curHealth;
	int atk;
	int agr;
	int hit;
	int def;
	int exp;
	int eva;
	unsigned int level;
	Dun_Coord stepLog[LOG_BUFFER];
	unsigned int currentPos;
} Entity;



typedef struct {
	Dun_Coord startLocation; // Corner location in the world map, as well as the initial offset
	unsigned int xdim;
	unsigned int ydim;
	unsigned int roomID; // Number of the room in the world
	// Side exit nodes, nodes[X][0,1] are the holes in the wall, nodes[X][2,3] are the nodes for pathing between rooms
	Dun_Coord exitNodes[4][2];
} Room;

DR_LIST_DEF(Room)

typedef struct {
	int locationID;
	char ref;
	int passable; // 0 = not passable, 1 = passable
	int occupied; // 0 = not occupied, 1 = occupied
	short admat[4]; // Adjacency matrix for the four directions: up, right, down, left
					// Positions: 0 = up, 1 = right, 2 = down, 3 = left
} Cell;

DR_LIST_DEF(Entity)


int checkItemOverlap(IOG_List* itemList, Item_on_Ground newItem);

extern const Dun_Vec up;
extern const Dun_Vec right;
extern const Dun_Vec down;
extern const Dun_Vec left;
extern const Dun_Vec directions[4];

extern Cell world[XBOUND][YBOUND];

extern Entity__List masterEntityList;
extern Entity__List enemiesOnFloor; // Took the Entities array out of Room, because Entities should be able to roam between rooms on the floor, plus it was making room-scaling difficult.

extern int enemyGlossarySize;

extern Item__List itemGlossary;

extern Room__List rooms;

int countLines(FILE* file);

#endif
