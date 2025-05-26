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

#define XBOUND 256
#define YBOUND 512
#define DEBUG 1
#define MAX_ITEMS 10
#define PLAYER_INVENTORY_BASE 16
#define OLD_ACTIONS 0
#define LOG_BUFFER 4

typedef struct {
	int dx;
	int dy;
} Dun_Vec;

typedef struct {
	unsigned int x;
	unsigned int y;
} Dun_Coord;

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

} Room;

typedef struct {
	int locationID;
	char ref;
	int passable; // 0 = not passable, 1 = passable
	int occupied; // 0 = not occupied, 1 = occupied
	short admat[4]; // Adjacency matrix for the four directions: up, right, down, left
					// Positions: 0 = up, 1 = right, 2 = down, 3 = left
} Cell;

extern const Dun_Vec up;	// { -1,  0 };
extern const Dun_Vec right; // {  0,  1 };
extern const Dun_Vec down;	// {  1,  0 };
extern const Dun_Vec left;	// {  0, -1 };

extern Cell world[XBOUND][YBOUND];
extern Player you;
extern Entity* enemyGlossary;
extern Entity* enemiesOnFloor; // Took the Entities array out of Room, because Entities should be able to roam between rooms on the floor, plus it was making room-scaling difficult.
extern int enemyGlossarySize;
extern unsigned int roomCount;
extern Item itemGlossary[MAX_ITEMS];
extern Room* rooms;
extern unsigned int currRoomCount;

int countLines(FILE* file);

#endif

/*
Cell world[XBOUND][YBOUND];
Player you;
Entity* enemyGlossary;
Entity* enemiesOnFloor; // Took the Entities array out of Room, because Entities should be able to roam between rooms on the floor, plus it was making room-scaling difficult.
int enemyGlossarySize;
unsigned int roomCount;
Item itemGlossary[MAX_ITEMS];
const Dun_Vec up	= { -1,  0 };
const Dun_Vec right = {  0,  1 };
const Dun_Vec down	= {  1,  0 };
const Dun_Vec left	= {  0, -1 };
Room* rooms;
unsigned int currRoomCount = 0;
*/