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
#define PLAYER_INVENTORY_BASE 16
#define OLD_ACTIONS 0
#define LOG_BUFFER 4
#define inRange(x, min, max) ((x) >= (min) && (x) < (max)) // Check if x is in the range [min, max)
#define inRangeInclusive(x, min, max) ((x) >= (min) && (x) <= (max)) // Check if x is in the range [min, max)
#define inRangeExclusive(x, min, max) ((x) > (min) && (x) < (max)) // Check if x is in the range (min, max)
#define matchSign(x, y) (((x) < 0 && (y) < 0) || ((x) >= 0 && (y) >= 0)) // Check if x and y have the same sign
#define BUFFER 4 // Buffer to check for room collisions and out of bounds cases.


typedef struct {
	int dx;
	int dy;
} Dun_Vec;

typedef struct {
	unsigned int x;
	unsigned int y;
} Dun_Coord;

typedef struct {
    unsigned int capacity;
    unsigned int head;
    unsigned int tail;
    Dun_Coord * items;
} Dun_Coord_Queue;

typedef Dun_Coord_Queue DCQ;

void DCQ_init(DCQ* instance, unsigned int capacity);
void DCQ_destroy(DCQ *dcq);

bool DCQ_is_empty(DCQ *dcq);
void DCQ_append(DCQ *dcq, Dun_Coord coord);
Dun_Coord DCQ_pop(DCQ *dcq);

int isInARoom(Dun_Coord d);

void DCQ_resize(DCQ *dcq, unsigned int new_capacity);


Dun_Vec getVector(Dun_Coord start, Dun_Coord end);




// typedef struct {
//     Dun_Coord * items;
//     unsigned int capacity;
// } Dun_Coord_Vector;

// Dun_Coord* DCQ_to_DC_Vec(Dun_Coord_Queue *dcq);

typedef struct {
	char name[32];
	char stat[3];
	int bonus;
	int type; // 0 = weapon, 1 = armor, 2 = consumable
	int equipped; // 0 = not equipped, 1 = equipped
} Item;

typedef enum {
	WEAPON = 0,
	ARMOR = 1,
	CONSUMABLE = 2,
} ItemType;

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
	Entity base;
	int stepCount;
	Item* inventory; // Inventory of items
} Player;

typedef struct {
	Dun_Coord startLocation; // Corner location in the world map, as well as the initial offset
	unsigned int xdim;
	unsigned int ydim;
	unsigned int roomID; // Number of the room in the world
	// Side exit nodes, nodes[X][0,1] are the holes in the wall, nodes[X][2,3] are the nodes for pathing between rooms
	Dun_Coord exitNodes[4][4];
} Room;

typedef struct {
	int locationID;
	char ref;
	int passable; // 0 = not passable, 1 = passable
	int occupied; // 0 = not occupied, 1 = occupied
	short admat[4]; // Adjacency matrix for the four directions: up, right, down, left
					// Positions: 0 = up, 1 = right, 2 = down, 3 = left
} Cell;

DR_LIST_DEF(Entity)

extern const Dun_Vec up;
extern const Dun_Vec right;
extern const Dun_Vec down;
extern const Dun_Vec left;
extern const Dun_Vec directions[4];

extern Cell world[XBOUND][YBOUND];

extern Player you;

extern Entity* enemyGlossary;
extern Entity__List masterEntityList;
extern Entity* enemiesOnFloor; // Took the Entities array out of Room, because Entities should be able to roam between rooms on the floor, plus it was making room-scaling difficult.

extern int enemyGlossarySize;

extern unsigned int roomCount;
extern unsigned int currRoomCount;

extern Item* itemGlossary;

extern Room* rooms;

int countLines(FILE* file);

#endif
