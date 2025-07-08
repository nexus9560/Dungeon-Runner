#include "PathFinder.h"
#include "DungeonTypes.h"



DR_LIST_IMPL(PF_Cell)

//Revisit this for later optimization

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
	PFCL closedSet;
	PF_Cell__List_init(&openSet, 0);
	PF_Cell__List_push(&openSet, *startCell); // Add start cell to open set
	PF_Cell__List_init(&closedSet, 0); // Initialize closed set

	while (openSet.size > 0) {
		PF_Cell currentCell;
		PF_Cell__List_pop(&openSet, &currentCell); // Pop the cell with the lowest total cost from open set
		currentCell.isVisited = true; // Mark the current cell as visited
		PF_Cell__List_push(&closedSet, currentCell); // Add it to the closed set

		if(currentCell.pos.x == goal.x && currentCell.pos.y == goal.y) {
			// Goal reached, construct the path
			PF_Cell* cell = &currentCell;
			while (cell != NULL) {
				PF_Cell__List_push(&path, *cell); // Push the cell onto the path
				cell = &map[cell->parent->x][cell->parent->y]; // Move to the parent cell
			}
			return path;
		}

		DCQ neighbors = getNeighbors(currentCell.pos); // Get neighbors of the current cell

		for (int i = 0;i < neighbors.capacity;i++) {
			PF_Cell neighbor = map[neighbors.items[i].x][neighbors.items[i].y]; // Get the neighbor cell
			if (neighbor.isVisited && !isinPFCL(&closedSet, &neighbor)) {
				PF_Cell__List_push(&closedSet, neighbor); // If already visited, add to closed set
				continue;
			}
			else {
				if (ignoreWalls || neighbor.isWalkable) { // Check if the neighbor is walkable or if walls are ignored
					unsigned int tentativeCost = currentCell.cost + 1; // Cost to reach the neighbor
					if (tentativeCost < neighbor.cost) { // If this path to the neighbor is better
						neighbor.parent = &currentCell.pos; // Set the parent to the current cell
						neighbor.cost = tentativeCost; // Update cost
						neighbor.totalCost = tentativeCost + neighbor.heuristic; // Update total cost
						if (!isinPFCL(&openSet, &neighbor)) {
							PF_Cell__List_push(&openSet, neighbor); // Add neighbor to open set if not already present
						}
					}
				}
			}

		}

	}

	return path;
}

bool isinPFCL(PFCL* list, PF_Cell* cell) {
	for (unsigned int i = 0; i < list->size; i++) {
		if (list->items[i].pos.x == cell->pos.x && list->items[i].pos.y == cell->pos.y) {
			return true; // Cell is already in the list
		}
	}
	return false; // Cell is not in the list
}

DCQ getNeighbors(Dun_Coord pos) {
	DCQ neighbors;
	DCQ_init(&neighbors, 4); // Initialize with a capacity for 4 neighbors
	// Define the possible neighbor offsets (up, down, left, right)
	Dun_Coord offsets[4] = {
		{ pos.x + up.dx,	pos.y + up.dy	}, // Up
		{ pos.x + right.dx,	pos.y + right.dy}, // Right
		{ pos.x + down.dx,	pos.y + down.dy	}, // Down
		{ pos.x + left.dx,	pos.y + left.dy	}  // Left
	};
	for (int i = 0; i < 4; i++) {
		Dun_Coord neighbor = offsets[i];
		if (neighbor.x >= 0 && neighbor.x < XBOUND && neighbor.y >= 0 && neighbor.y < YBOUND) {
			if (world[neighbor.x][neighbor.y].passable == 1 && world[neighbor.x][neighbor.y].occupied == 0) {
				DCQ_append(&neighbors, neighbor); // Append valid neighbors to the queue
			}
		}
	}
	return neighbors;
}

DCQ ExtractPath(PFCL path) {
	DCQ result;

	DCQ_init(&result, (unsigned int)path.size);
	for (unsigned int i = 0; i < path.size; i++) {
		PF_Cell cell;
		PF_Cell__List_pop(&path, &cell); // Pop each cell from the path
		if (&cell != NULL) {
			Dun_Coord coord = cell.pos;
			DCQ_append(&result, coord); // Append the coordinate to the result queue
		}
	}
	return result;
}

int isThereAPath(Dun_Coord start, Dun_Coord end) {
	PFCL path = AStar(start, end, false);
	int result = (path.size > 0) ? 1 : 0; // Return 1 if a path exists, otherwise 0
	PF_Cell__List_destroy(&path);
	return result;
}