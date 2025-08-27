#pragma once
#ifndef PATHFINDER_H
#define PATHFINDER_H


#include "list.h"
#include "DungeonTypes.h"

// Ensure PF_Cell__List is properly defined before use


typedef struct {
	Dun_Coord pos; // Position of the cell in the world
	unsigned int cost; // Cost to reach this cell
	unsigned int heuristic; // Heuristic cost estimate to the goal
	unsigned int totalCost; // Total cost (cost + heuristic)
	Dun_Coord parent; // Parent cell in the path
	unsigned int parentCounter; // number of parents (should be 0 or 1)
	bool isWalkable; // Whether the cell can be walked on
	bool isVisited; // Whether the cell has been visited
} PF_Cell;


DR_LIST_DEF(PF_Cell)
DR_LIST_DEF(Dun_Coord)

typedef Dun_Coord__List DCL;

typedef PF_Cell__List PFCL;

void AStar(Dun_Coord start, Dun_Coord goal, bool ignoreWalls, DCL* ret);

bool getNeighbors(Dun_Coord pos, Dun_Coord *neighbors, int ignoreWalls);

void ExtractCoordinates(PFCL list, DCL* ret);

bool isinPFCL(PFCL* list, PF_Cell* cell);

bool areCoordsInPFCL(PFCL* list, Dun_Coord coords);

PF_Cell PFCL_Find_by_coords(PFCL* list, Dun_Coord coords);

bool PFCL_List_pop_by_coords(PFCL* list, Dun_Coord coords, PF_Cell* cell);

int isThereAPath(Dun_Coord start, Dun_Coord end);

#endif