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
	Dun_Coord* parent; // Parent cell in the path
	unsigned int parentCounter; // number of parents (should be 0 or 1)
	bool isWalkable; // Whether the cell can be walked on
	bool isVisited; // Whether the cell has been visited
} PF_Cell;

DR_LIST_DEF(PF_Cell)

typedef PF_Cell__List PFCL;

PFCL AStar(Dun_Coord start, Dun_Coord goal, bool ignoreWalls);

DCQ getNeighbors(Dun_Coord pos);

bool isinPFCL(PFCL* list, PF_Cell* cell);

DCQ ExtractPath(PFCL path);

int isThereAPath(Dun_Coord start, Dun_Coord end);

#endif