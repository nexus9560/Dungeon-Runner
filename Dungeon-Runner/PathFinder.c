#include "PathFinder.h"
#include "DungeonTypes.h"



DR_LIST_IMPL(PF_Cell)

//Revisit this for later optimization

PFCL AStar(Dun_Coord start, Dun_Coord goal, bool ignoreWalls) {
	PFCL path;
	PF_Cell__List_init(&path, 0);

	if (start.x >= XBOUND || start.y >= YBOUND || goal.x >= XBOUND || goal.y >= YBOUND) {
		printf("Error: Start or goal coordinates are out of bounds.\n");
		return path;
	}

	Dun_Vec delta = getVector(start, goal);
	unsigned int distance = abs(delta.dx) + abs(delta.dy);

	PF_Cell__List_init(&path, 2*distance);


	
    PF_Cell startCell;
    startCell = (PF_Cell){
		.pos = start,
		.cost = 0,
		.heuristic = abs((int)(start.x - goal.x)) + abs((int)(start.y - goal.y)),
		.totalCost = abs((int)(start.x - goal.x)) + abs((int)(start.y - goal.y)),
		.parent = NULL,
		.parentCounter = 0,
		.isWalkable = (world[start.x][start.y].passable == 1 && world[start.x][start.y].occupied == 0) || ignoreWalls,
		.isVisited = false
    };
	PFCL openSet;
	PFCL closedSet;
	PF_Cell__List_init(&openSet, 0);
	PF_Cell__List_push(&openSet, startCell); // Add start cell to open set
	PF_Cell__List_init(&closedSet, 0); // Initialize closed set

	while (openSet.size > 0) {
		PF_Cell currentCell;
		PF_Cell__List_pop(&openSet, &currentCell); // Pop the cell with the lowest total cost from open set
		currentCell.isVisited = true; // Mark the current cell as visited
		PF_Cell__List_push(&closedSet, currentCell); // Add it to the closed set

		if(currentCell.pos.x == goal.x && currentCell.pos.y == goal.y) {
			// Goal reached, construct the path
			PF_Cell* cell = &currentCell;
			while (cell->parent != NULL) {
				PF_Cell__List_push(&path, *cell); // Push the cell onto the path
				PFCL_List_pop_by_coords(&closedSet, *cell->parent, cell); // Pop the parent cell from closed set
			}
			return path;
		}

		Dun_Coord* neighbors = malloc(4 * sizeof(Dun_Coord));
		if (!neighbors) {
			printf("Error: Memory allocation failed for neighbors.\n");
			exit(1);
		}
		getNeighbors(currentCell.pos, neighbors); // Get neighbors of the current cell

		for (unsigned int i = 0; i < 4; i++) {
			Dun_Coord neighborPos = neighbors[i];
			if (neighborPos.x > XBOUND || neighborPos.y > YBOUND || neighborPos.x < 0 || neighborPos.y < 0) {
				continue; // Skip if the neighbor is out of bounds
			}
            PF_Cell neighborCell = (PF_Cell){
				.pos=neighborPos,.cost=currentCell.cost + 1,
				.heuristic=abs((int)(neighborPos.x - goal.x)) + abs((int)(neighborPos.y - goal.y)),
				.totalCost=currentCell.cost + 1 + abs((int)(neighborPos.x - goal.x)) + abs((int)(neighborPos.y - goal.y)),
				.parent=&currentCell.pos,
				.parentCounter=1,
				.isWalkable=(world[neighborPos.x][neighborPos.y].passable == 1 && world[neighborPos.x][neighborPos.y].occupied == 0) || ignoreWalls,
				.isVisited=false
            };
			if (!neighborCell.isWalkable || neighborCell.isVisited) {
				PF_Cell__List_push(&closedSet, neighborCell); // Add to closed set if not walkable or already visited
				continue; // Skip if the neighbor is not walkable or already visited
			}
			else {
				unsigned int tentativeCost = currentCell.cost + 1; // Assuming uniform cost for each step
				if (tentativeCost < neighborCell.cost) {
					// If the new cost is lower, update the cell
					neighborCell.parent = &currentCell.pos; // Set the parent to the current cell
					neighborCell.parentCounter = 1; // Set parent counter to 1
					neighborCell.cost = tentativeCost; // Update cost
					neighborCell.totalCost = tentativeCost + neighborCell.heuristic; // Update total cost
					if (!isinPFCL(&openSet, &neighborCell)) {
						PF_Cell__List_push(&openSet, neighborCell); // Add to open set if not already present
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

bool PFCL_List_pop_by_coords(PFCL* list, Dun_Coord coords, PF_Cell* cell) {
	bool ret;
	for (unsigned int i = 0; i < list->size; i++) {
		if (list->items[i].pos.x == coords.x && list->items[i].pos.y == coords.y) {
			*cell = list->items[i]; // Copy the cell to the output parameter
			ret = PF_Cell__List_pop(list, cell); // Pop the cell from the list
			return ret; // Return whether the pop was successful
		}
	}
	*cell = (PF_Cell){ .pos = { XBOUND+1, YBOUND+1 }, .cost = INT_MAX, .heuristic = INT_MAX, .totalCost = INT_MAX, .parent = NULL, .parentCounter = 0, .isWalkable = false, .isVisited = false }; // Initialize cell to invalid state
	return false; // Cell with the specified coordinates not found in the list
}

bool getNeighbors(Dun_Coord pos, Dun_Coord* neighbors) {
	if (!neighbors) return false;

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
				neighbors[i] = neighbor; // Append valid neighbors to the array
			}
			else
			{
				neighbors[i] = (Dun_Coord){ XBOUND+1, YBOUND+1 }; // Mark as invalid coordinate if not passable
			}
		}
	}
	return true;
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