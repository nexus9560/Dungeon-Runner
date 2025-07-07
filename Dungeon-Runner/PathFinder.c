#include "PathFinder.h"
#include "DungeonTypes.h"



DR_LIST_IMPL(PF_Cell)

PFCL AStar(Dun_Coord start, Dun_Coord goal, bool ignoreWalls) {
	PF_Cell map[XBOUND][YBOUND]; // Map of PF_Cells representing the world
	PFCL path;
	PF_Cell__List_init(&path, 0);

	if (start.x >= XBOUND || start.y >= YBOUND || goal.x >= XBOUND || goal.y >= YBOUND) {
		printf("Error: Start or goal coordinates are out of bounds.\n");
		return path;
	}

	Dun_Vec delta = getVector(start, goal);
	unsigned int distance = abs(delta.dx) + abs(delta.dy);

	PF_Cell__List_init(&path, 2*distance);

	for (unsigned int x = 0; x < XBOUND; x++) {
		for (unsigned int y = 0; y < YBOUND; y++) {
			PF_Cell* cell = &map[x][y];
			cell->pos = (Dun_Coord){ x, y };
			cell->cost = INT_MAX; // Initialize cost to a large value
			cell->heuristic = abs((int)(x - goal.x)) + abs((int)(y - goal.y)); // Manhattan distance heuristic
			cell->totalCost = INT_MAX; // Initialize total cost to a large value
			cell->parent = NULL; // No parent initially
			cell->parentCounter = 0; // No parents initially
			cell->isWalkable = (world[x][y].passable == 1 && world[x][y].occupied == 0); // Walkable if passable and not occupied
			cell->isVisited = false; // Not visited initially

		}
	}

	PF_Cell* startCell = &map[start.x][start.y];
	startCell->cost = 0; // Cost to reach start cell is 0
	startCell->totalCost = startCell->heuristic; // Total cost is just the heuristic at the start
	PFCL openSet;
	PF_Cell__List_init(&openSet, 2 * distance);
	PF_Cell__List_append(&openSet, *startCell); // Add start cell to open set
	

	return path;
}