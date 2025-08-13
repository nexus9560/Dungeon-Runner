#include "PathFinder.h"
#include "DungeonTypes.h"



DR_LIST_IMPL(PF_Cell)
DR_LIST_IMPL(Dun_Coord)

//Revisit this for later optimization

PFCL AStar(Dun_Coord start, Dun_Coord goal, bool ignoreWalls) {
	//if(DEBUG) {
	//	printf("AStar called with start: (%d, %d) and goal: (%d, %d)\n", start.x, start.y, goal.x, goal.y);
	//}
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
		.parent = (Dun_Coord){XBOUND+1,YBOUND+1},
		.parentCounter = 0,
		.isWalkable = (world[start.x][start.y].passable == 1 && world[start.x][start.y].occupied == 0) || ignoreWalls,
		.isVisited = false
    };
	PFCL openSet;
	PFCL closedSet;
	PF_Cell__List_init(&openSet, (2 * distance));
	PF_Cell__List_push(&openSet, startCell); // Add start cell to open set
	PF_Cell__List_init(&closedSet, (2 * distance)); // Initialize closed set

	//if(DEBUG) {
	//	printf("Start cell initialized at (%d, %d) with cost %u, heuristic %u, total cost %u\n",
	//		startCell.pos.x, startCell.pos.y, startCell.cost, startCell.heuristic, startCell.totalCost);
	//}

	while (openSet.size > 0) {
		PF_Cell currentCell;
		PF_Cell neighborCell;
		PF_Cell__List_pop(&openSet, &currentCell); // Pop the cell with the lowest total cost
		//if(DEBUG) {
		//	printf("Current cell popped: (%d, %d) with cost %u, heuristic %u, total cost %u\n",
		//		currentCell.pos.x, currentCell.pos.y, currentCell.cost, currentCell.heuristic, currentCell.totalCost);
		//}
		if ((start.x == XBOUND + 1 && start.y == YBOUND + 1) || (goal.x == XBOUND + 1 && goal.y == YBOUND + 1)) {
			printf("Error: Start or goal coordinates are invalid.\n");
			return path; // Return empty path if start or goal coordinates are invalid
		}

		if( currentCell.pos.x == goal.x && currentCell.pos.y == goal.y) {
//			printf("-----------------------------------------------------------------------------------------------\n");
			PF_Cell__List_init(&path, currentCell.parentCounter + 1); // Initialize path with the number of parents
			//if(DEBUG) {
			//	printf("Goal reached at (%d, %d)\n", goal.x, goal.y);
			//	printf("Path size: %u\n", (int)path.size);
			//	printf("Current cell parent counter: %u\n", currentCell.parentCounter);
			//	printf("Current cell parent position: (%d, %d)\n", currentCell.parent.x, currentCell.parent.y);
			//	printf("Current cell cost: %u, heuristic: %u, total cost: %u\n",
			//		currentCell.cost, currentCell.heuristic, currentCell.totalCost);
			//	printf("Current cell isWalkable: %d, isVisited: %d\n",
			//		currentCell.isWalkable, currentCell.isVisited);
			//	printf("Current cell position: (%d, %d)\n", currentCell.pos.x, currentCell.pos.y);
			//	printf("Current cell parent is not NULL: %s\n", (currentCell.parent.x != XBOUND+1 && currentCell.parent.y != YBOUND+1) ? "true" : "false");
			//	if( currentCell.parent.x != XBOUND+1 && currentCell.parent.y != YBOUND+1) {
			//		printf("Current cell parent position: (%d, %d)\n", currentCell.parent.x, currentCell.parent.y);
			//	}
			//}

			while(currentCell.parent.x != XBOUND+1 || currentCell.parent.y != YBOUND+1) {
				PF_Cell__List_push(&path, currentCell); // Push the cell to the path
				//if(DEBUG) {
				//	printf("Adding cell to grand path: (%d, %d)\n", currentCell.pos.x, currentCell.pos.y);
				//}
				currentCell = PFCL_Find_by_coords(&closedSet, currentCell.parent); // Remove the cell from closed set
				if(currentCell.parent.x != XBOUND+1 || currentCell.parent.y != YBOUND+1) {
					//if(DEBUG) {
					//	printf("Current cell parent position: (%d, %d)\n", currentCell.parent.x, currentCell.parent.y);
					//}
				} else {
					//if(DEBUG) {
					//	printf("Current cell has no parent.\n");
					//}
				}
			}
			PF_Cell__List_push(&path, startCell); // Add the start cell to the path
//			printf("-----------------------------------------------------------------------------------------------\n");
			break; // Exit the loop if the goal is reached
		}
		else {
			currentCell.isVisited = true; // Mark the current cell as visited
			PF_Cell__List_push(&closedSet, currentCell); // Add the current cell to closed set
			//if(DEBUG) {
			//	printf("Current cell (%d, %d) added to closed set\n", currentCell.pos.x, currentCell.pos.y);
			//}
			Dun_Coord neighbors[4];
			if(!getNeighbors(currentCell.pos, neighbors, ignoreWalls)) {
				printf("Error: Could not get neighbors for cell (%d, %d)\n", currentCell.pos.x, currentCell.pos.y);
				continue; // Skip to the next iteration if neighbors cannot be retrieved
			}
			for (unsigned int i = 0; i < 4; i++) {
				if(neighbors[i].x >= XBOUND+1 || neighbors[i].y >= YBOUND+1) {
					continue; // Skip invalid neighbors
				}else if(neighbors[i].x < 0 || neighbors[i].y < 0 || neighbors[i].x >= XBOUND || neighbors[i].y >= YBOUND) {
					//if(DEBUG) {
					//	printf("Skipping out of bounds neighbor (%d, %d)\n", neighbors[i].x, neighbors[i].y);
					//}
					continue; // Skip out of bounds neighbors
				}

				if(areCoordsInPFCL(&closedSet, neighbors[i])) {
					//if(DEBUG) {
					//	printf("Skipping neighbor (%d, %d) as it is already in closed set\n", neighbors[i].x, neighbors[i].y);
					//}
					continue; // Skip if the neighbor is already in closed set
				}else if(areCoordsInPFCL(&openSet, neighbors[i])) {
					//if(DEBUG) {
					//	printf("Skipping neighbor (%d, %d) as it is already in open set\n", neighbors[i].x, neighbors[i].y);
					//}
					continue; // Skip if the neighbor is already in open set
				}else if(world[neighbors[i].x][neighbors[i].y].occupied == 1 && !ignoreWalls) {
					//if(DEBUG) {
					//	printf("Skipping neighbor (%d, %d) as it is occupied\n", neighbors[i].x, neighbors[i].y);
					//}
					continue; // Skip if the neighbor is occupied and walls are not ignored
				}
				
				// Calculate the cost to reach the neighbor
				unsigned int cost = currentCell.cost + 1; // Assuming uniform cost for each step
				unsigned int heuristic = abs((int)(neighbors[i].x - goal.x)) + abs((int)(neighbors[i].y - goal.y));
				unsigned int totalCost = cost + heuristic;
				//if (DEBUG) {
				//	printf("Neighbor (%d, %d) - Cost: %u, Heuristic: %u, Total Cost: %u, Location: (%d, %d)\n",
				//		neighbors[i].x, neighbors[i].y, cost, heuristic, totalCost, neighbors[i].x, neighbors[i].y);
				//}
				// Create a new PF_Cell for the neighbor
				Dun_Coord loc;
				loc.x = currentCell.pos.x;
				loc.y = currentCell.pos.y;
				neighborCell = (PF_Cell){
					.pos = neighbors[i],
					.cost = cost,
					.heuristic = heuristic,
					.totalCost = totalCost,
					.parent = (Dun_Coord) { loc.x, loc.y }, // Set the parent to the current cell's position
					.parentCounter = currentCell.parentCounter + 1, // Increment parent counter
					.isWalkable = (world[neighbors[i].x][neighbors[i].y].passable == 1 && world[neighbors[i].x][neighbors[i].y].occupied == 0) || ignoreWalls,
					.isVisited = false
				};

				//if(DEBUG) {
				//	printf("Adding neighbor (%d, %d) with cost %u, heuristic %u, total cost %u\n",
				//		neighborCell.pos.x, neighborCell.pos.y, neighborCell.cost, neighborCell.heuristic, neighborCell.totalCost);
				//	printf("Neighbor parent position: (%d, %d)\n", neighborCell.parent.x, neighborCell.parent.y);
				//}
				if (!neighborCell.isVisited && !isInARoom(neighborCell.pos) && neighborCell.heuristic < currentCell.heuristic) {
					PF_Cell__List_push(&openSet, neighborCell); // Add the neighbor to open set
					//if (DEBUG) {
					//	printf("Neighbor (%d, %d) added to open set with cost %u, heuristic %u, total cost %u\n",
					//		neighborCell.pos.x, neighborCell.pos.y, neighborCell.cost, neighborCell.heuristic, neighborCell.totalCost);
					//}
				}
				else {
					PF_Cell__List_push(&closedSet, neighborCell); // If the total cost is too high, add to closed set
					//if (DEBUG) {
					//	printf("Neighbor (%d, %d) added to closed set due to high cost %u\n",
					//		neighborCell.pos.x, neighborCell.pos.y, neighborCell.totalCost);
					//}
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

bool areCoordsInPFCL(PFCL* list, Dun_Coord coords) {
	for (unsigned int i = 0; i < list->size; i++) {
		if (list->items[i].pos.x == coords.x && list->items[i].pos.y == coords.y) {
			return true; // Coordinates are already in the list
		}
	}
	return false; // Coordinates are not in the list
}

PF_Cell PFCL_Find_by_coords(PFCL* list, Dun_Coord coords) {
	PF_Cell ret;
	for (unsigned int i = 0; i < list->size; i++) {
		if (list->items[i].pos.x == coords.x && list->items[i].pos.y == coords.y) {
			ret = list->items[i]; // Store the matching cell in the ret variable
			return ret; // Return the cell found
		}
	}
	return (PF_Cell){ .pos = { XBOUND+1, YBOUND+1 }, .cost = INT_MAX, .heuristic = INT_MAX, .totalCost = INT_MAX, .parent = (Dun_Coord){ XBOUND+1, YBOUND+1 }, .parentCounter = 0, .isWalkable = false, .isVisited = false }; // Cell with the specified coordinates not found in the list
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
	*cell = (PF_Cell){ .pos = { XBOUND+1, YBOUND+1 }, .cost = INT_MAX, .heuristic = INT_MAX, .totalCost = INT_MAX, .parent = (Dun_Coord){ XBOUND+1, YBOUND+1 }, .parentCounter = 0, .isWalkable = false, .isVisited = false }; // Initialize cell to invalid state
	return false; // Cell with the specified coordinates not found in the list
}

bool getNeighbors(Dun_Coord pos, Dun_Coord* neighbors, int ignoreWalls) {
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
			if ((world[neighbor.x][neighbor.y].passable == 1 && world[neighbor.x][neighbor.y].occupied == 0)||ignoreWalls) {
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

DCL ExtractCoordinates(PFCL list) {
	DCL coords;
	Dun_Coord__List_init(&coords, list.size);
	for (unsigned int i = 0; i < list.size; i++) {
		Dun_Coord coord = list.items[i].pos;
		if (coord.x < XBOUND && coord.y < YBOUND) {
			Dun_Coord__List_push(&coords, coord); // Add valid coordinates to the list
		}
	}
	return coords; // Return the list of coordinates
}


int isThereAPath(Dun_Coord start, Dun_Coord end) {
	PFCL path = AStar(start, end, false);
	int result = (path.size > 0) ? 1 : 0; // Return 1 if a path exists, otherwise 0
	PF_Cell__List_destroy(&path);
	return result;
}