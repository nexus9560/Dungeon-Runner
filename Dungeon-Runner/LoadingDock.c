#include "DungeonTypes.h"




void savePlayer() {
	FILE* file = fopen("player.dat", "w");
	if (file == NULL) {
		printf("Error: Could not open or create file for saving.\n");
		return;
	}

	// Write the player's position to the file
	fprintf(file, "Location:[%4d,%4d]\n", you.base.location.x, you.base.location.y);
	fprintf(file, "Name:%s\n", you.base.name);
	fprintf(file, "Health:%d\n", you.base.health);
	fprintf(file, "CurrentHealth:%d\n", you.base.curHealth);
	fprintf(file, "Attack:%d\n", you.base.atk);
	fprintf(file, "To-Hit:%d\n", you.base.hit);
	fprintf(file, "Defense:%d\n", you.base.def);
	fprintf(file, "Experience:%d\n", you.base.exp);
	fprintf(file, "Evasion:%d\n", you.base.eva);
	fprintf(file, "Level:%d\n", you.base.level);

	fclose(file);
	printf("Player position saved successfully.\n");
}

int loadPlayer() {

#ifdef _WIN32
	FILE* file = fopen("player.dat", "r");
	if (file == NULL) {
		printf("Error: Could not open player data file.\n");
		return 0;
	}
#else
	FILE* file = fopen("player.dat", "r");
	if (file == NULL) {
		printf("Error: Could not open player data file.\n");
		return 0;
	}
#endif

	int lineCount = countLines(file);
	for (int x = 0;x < lineCount;x++) {
		char line[256];
		if (fgets(line, sizeof(line), file) != NULL) {
			if (sscanf(line, "Location:[%4d,%4d]", &you.base.location.x, &you.base.location.y) == 2) {
				continue;
			}
			else if (sscanf(line, "Name:%s", you.base.name) == 1) {
				continue;
			}
			else if (sscanf(line, "Health:%d", &you.base.health) == 1) {
				continue;
			}
			else if (sscanf(line, "CurrentHealth:%d", &you.base.curHealth) == 1) {
				continue;
			}
			else if (sscanf(line, "Attack:%d", &you.base.atk) == 1) {
				continue;
			}
			else if (sscanf(line, "Aggro:%d", &you.base.agr) == 1) {
				continue;
			}
			else if (sscanf(line, "To-Hit:%d", &you.base.hit) == 1) {
				continue;
			}
			else if (sscanf(line, "Defense:%d", &you.base.def) == 1) {
				continue;
			}
			else if (sscanf(line, "Experience:%d", &you.base.exp) == 1) {
				continue;
			}
			else if (sscanf(line, "Evasion:%d", &you.base.eva) == 1) {
				continue;
			}
			else if (sscanf(line, "Level:%d", &you.base.level) == 1) {
				continue;
			}
		}
		else {
			printf("Error: Failed to read player data.\n");
		}
	}

	// Refactor this to allow for dynamically sized saved files, if a value is missing, it defaults to a specific value


	fclose(file);
	if (DEBUG)
		printf("Player data loaded successfully.\n");
	return 1;
}

void loadEntities(int ovr) {
	FILE* file = fopen("entities.dat", "r");
	if (file == NULL) {
		printf("Error: Could not open file for loading.\n");

		return;
	}
	enemyGlossarySize = countLines(file);
	enemyGlossary = malloc(enemyGlossarySize * sizeof(Entity));
	if (enemyGlossary == NULL) {
		printf("Error: Memory allocation failed.\n");
		fclose(file);
		return;
	}
    for (int i = 0; i < enemyGlossarySize; i++) {
		// Use [HP:  2,ATK:  1,AGR:  0,TOH:  1,DEF:  0,EXP:  1,EVA:  0,LVL:  1]:SLUG as the format
		int agr = 0;
		char namebuf[32] = {0};
		int ret = fscanf(file, "[HP:%d,ATK:%d,AGR:%d,TOH:%d,DEF:%d,EXP:%d,EVA:%d,LVL:%d]:%31s\n",
		&enemyGlossary[i].health,
		&enemyGlossary[i].atk,
		&agr,
		&enemyGlossary[i].hit,
		&enemyGlossary[i].def,
		&enemyGlossary[i].exp,
		&enemyGlossary[i].eva,
		&enemyGlossary[i].level,
		namebuf
		);
		if (ret == 9) {
			enemyGlossary[i].agr = agr;
			strncpy(enemyGlossary[i].name, namebuf, sizeof(enemyGlossary[i].name) - 1);
			enemyGlossary[i].name[sizeof(enemyGlossary[i].name) - 1] = '\0';
		} else {
			printf("Warning: Failed to parse entity line %d (fscanf returned %d)\n", i + 1, ret);
			memset(&enemyGlossary[i], 0, sizeof(enemyGlossary[i]));
		}
    }
	if (DEBUG || ovr) {
		for (int i = 0; i < enemyGlossarySize; i++) {
			printf("Entity %3d: %31s:[HP:%3d,ATK:%3d,ARG:%3d,TOH:%3d,DEF:%3d,EXP:%3d,EVA:%3d,LVL:%3d]\n",
				i + 1,
				enemyGlossary[i].name,
				enemyGlossary[i].health,
				enemyGlossary[i].atk,
				enemyGlossary[i].agr,
				enemyGlossary[i].hit,
				enemyGlossary[i].def,
				enemyGlossary[i].exp,
				enemyGlossary[i].eva,
				enemyGlossary[i].level
			);
		}
		printf("\nEntities loaded successfully.\n");
	}
	fclose(file);

}


Room* loadRooms(int ovr) {
	FILE* file = fopen("rooms.dat", "r");
	Room* temp = NULL;
	if (file == NULL) {
		printf("Error: Could not open rooms.dat for reading.\n");
		return 0;
	}
	roomCount = countLines(file);
	if (roomCount == 0) {
		printf("Error: No rooms found in rooms.dat.\n");
		fclose(file);
		return NULL;
	}
	exitNodes = malloc(roomCount * sizeof(Dun_Coord**));
	temp = malloc(roomCount * sizeof(Room));
	if (temp == NULL || exitNodes == NULL) {
		printf("Error: Memory allocation failed.\n");
		fclose(file);
		free(exitNodes);
		return NULL;
	}
	for (unsigned int i = 0; i < roomCount; i++) {
		exitNodes[i] = malloc(4 * sizeof(Dun_Coord*));
		if (exitNodes[i] == NULL) {
			printf("Error: Memory allocation failed.\n");
			fclose(file);
			free(exitNodes);
			return NULL;
		}
		for (unsigned int j = 0; j < 4; j++) {
			exitNodes[i][j] = malloc(2 * sizeof(Dun_Coord));
			if (exitNodes[i][j] == NULL) {
				printf("Error: Memory allocation failed.\n");
				fclose(file);
				free(exitNodes);
				return NULL;
			}
			exitNodes[i][j][0] = (Dun_Coord){ XBOUND + 1,YBOUND + 1 };
			exitNodes[i][j][1] = (Dun_Coord){ XBOUND + 1,YBOUND + 1 };
		}
	}

	for (unsigned int i = 0; i < roomCount; i++) {
		fscanf(file, "Room %4d: Start Location: [%6d,%6d], Dimensions: [%6d,%6d]\n",
			&temp[i].roomID,
			&temp[i].startLocation.x,
			&temp[i].startLocation.y,
			&temp[i].xdim,
			&temp[i].ydim
		);
		for (unsigned int j = 0; j < 4;j++) {
			temp[i].egressCount[j] = 2;
		}
	}
	fclose(file);

	return temp;
}


void saveRooms() {
	FILE* file = fopen("rooms.dat", "w");
	if (file == NULL) {
		printf("Error: Could not open rooms.dat for writing.\n");
		return;
	}
	for (unsigned int i = 0; i < roomCount; i++) {
		fprintf(file, "Room %4d: Start Location: [%6d,%6d], Dimensions: [%6d,%6d]\n",
			i + 1,
			rooms[i].startLocation.x,
			rooms[i].startLocation.y,
			rooms[i].xdim,
			rooms[i].ydim
		);
	}
	fclose(file);
}


void loadItems(int ovr) {
	FILE* file = fopen("items.dat", "r");
	if (file == NULL) {
		printf("Error: Could not open file for loading.\n");
		return;
	}
	int numLines = countLines(file);
	Item* items = malloc(numLines * sizeof(Item));
	if (items == NULL) {
		printf("Error: Memory allocation failed.\n");
		fclose(file);
		return;
	}
	for (int i = 0; i < numLines; i++) {
		//		[ATK:001,TYPE:0]:PLAIN-SWORD
		fscanf(file, "[%3s:%3d,TYPE:%d]:%31s\n",
			&items[i].stat, &items[i].bonus, &items[i].type, &items[i].name
		);
	}
	fclose(file);

	if (DEBUG || ovr) {
		for (int i = 0; i < numLines; i++) {
			printf("Item %3d: %31s:[%3s:%3d,TYPE:%d]\n",
				i + 1,
				items[i].name,
				items[i].stat,
				items[i].bonus,
				items[i].type
			);
		}
		printf("\nItems loaded successfully.\n");
	}

	itemGlossary = malloc(numLines * sizeof(Item));

	for (int x = 0;x < numLines; x++)
		itemGlossary[x] = items[x];
	free(items);

}
