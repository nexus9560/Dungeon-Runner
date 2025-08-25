#include "DungeonTypes.h"
#include "pathlib.h"
#include <stdlib.h>
#include "LoadingDock.h"
#include <stdio.h>
#include <errno.h>

Entity__List masterEntityList;

#if defined(_WIN32) || defined(_WIN64)
    #include <direct.h>
    #define MKDIR(path) _mkdir(path)
    #define DIR_EXISTS(path) (dir_exists_win(path))
    #include <sys/stat.h>
    int dir_exists_win(const char *path) {
        struct _stat st;
        return (_stat(path, &st) == 0 && (st.st_mode & _S_IFDIR));
    }
#else
    #include <sys/stat.h>
    #include <sys/types.h>
    #define MKDIR(path) mkdir(path, 0755)
    #define DIR_EXISTS(path) (dir_exists_unix(path))
    int dir_exists_unix(const char *path) {
        struct stat st;
        return (stat(path, &st) == 0 && S_ISDIR(st.st_mode));
    }
#endif

// Ensures a directory exists, creates it if not
void ensure_directory(const char *path) {
    if (!DIR_EXISTS(path)) {
        if (MKDIR(path) == 0) {
            printf("Directory created: %s\n", path);
        } else if (errno == EEXIST) {
            // Directory already exists (race condition)
        } else {
            perror("mkdir failed");
        }
    }
}

void savePlayer() {
    char player_path[1024];
    path__join("data", "player.dat", player_path);
    ensure_directory("data");
    #if DEBUG
        printf("Saving player to %s\n", player_path);
    #endif
	FILE* file = fopen(player_path, "w");
	if (file == NULL) {
		printf("Error: Could not open or create file for saving.\n");
		return;
	}
	/*
		Location:[  17,  35]
		Name: Nex
		Health:10
		CurrentHealth:10
		Attack:1
		To-Hit:1
		Defense:1
		Experience:0
		Evasion:1
		Level:1
	*/
	// Write the player's position to the file
	fprintf(file, "Location:[%4d,%4d]\n", you.base.location.x, you.base.location.y);
	fprintf(file, "Name:%31s\n", you.base.name);
	fprintf(file, "Health:%4d\n", you.base.health);
	fprintf(file, "CurrentHealth:%4d\n", you.base.curHealth);
	fprintf(file, "Attack:%4d\n", you.base.atk);
	fprintf(file, "To-Hit:%4d\n", you.base.hit);
	fprintf(file, "Defense:%4d\n", you.base.def);
	fprintf(file, "Experience:%4d\n", you.base.exp);
	fprintf(file, "Evasion:%4d\n", you.base.eva);
	fprintf(file, "Level:%4d\n", you.base.level);
//	fprintf(file, "BodyPlan:%63s\n", you.base.bodyPlan);

	fclose(file);
	printf("Player position saved successfully.\n");

}

int loadPlayer() {
    char player_path[1024];
    path__join("data", "player.dat", player_path);
    ensure_directory("data");
	if(DEBUG) {
		printf("Loading player from %s\n", player_path);
	}

	FILE* file = fopen(player_path, "r");
	if (file == NULL) {
		printf("Error: Could not open player data file.\n");
		return 0;
	}

	int lineCount = countLines(file);
	for (int x = 0;x < lineCount;x++) {
		char line[256];
		if (fgets(line, sizeof(line), file) != NULL) {
			if (sscanf(line, "Location:[%4d,%4d]", &you.base.location.x, &you.base.location.y) == 2) {
				continue;
			}
			else if (sscanf(line, "Name:%31s", you.base.name) == 1) {
				continue;
			}
			else if (sscanf(line, "Health:%4d", &you.base.health) == 1) {
				continue;
			}
			else if (sscanf(line, "CurrentHealth:%4d", &you.base.curHealth) == 1) {
				continue;
			}
			else if (sscanf(line, "Attack:%4d", &you.base.atk) == 1) {
				continue;
			}
			else if (sscanf(line, "Aggro:%4d", &you.base.agr) == 1) {
				continue;
			}
			else if (sscanf(line, "To-Hit:%4d", &you.base.hit) == 1) {
				continue;
			}
			else if (sscanf(line, "Defense:%4d", &you.base.def) == 1) {
				continue;
			}
			else if (sscanf(line, "Experience:%4d", &you.base.exp) == 1) {
				continue;
			}
			else if (sscanf(line, "Evasion:%4d", &you.base.eva) == 1) {
				continue;
			}
			else if (sscanf(line, "Level:%4d", &you.base.level) == 1) {
				continue;
			}
		}
		else {
			printf("Error: Failed to read player data.\n");
			return 0;
		}
	}

	//// Refactor this to allow for dynamically sized saved files, if a value is missing, it defaults to a specific value


	fclose(file);
	//#if DEBUG
	//	printf("Player data loaded successfully.\n");
	//#endif
	return 1;
}

void loadEntities(int ovr) {
    char entities_path[1024];
    path__join("data", "entities.dat", entities_path);
    ensure_directory("data");
    #if DEBUG
        printf("Loading entities from %s\n", entities_path);
    #endif
	FILE* file = fopen(entities_path, "r");
	if (file == NULL) {
		printf("Error: Could not open file for loading.\n");

		return;
	}
	Entity__List_init(&masterEntityList, countLines(file));
	Entity e;
    for (int i = 0; i < masterEntityList.size; i++) {
		// Use [HP:  2,ATK:  1,AGR:  0,TOH:  1,DEF:  0,EXP:  1,EVA:  0,LVL:  1]:SLUG as the format
		int agr = 0;
		char namebuf[32] = {0};
		int ret = fscanf(file, "[HP:%3d,ATK:%3d,AGR:%3d,TOH:%3d,DEF:%3d,EXP:%3d,EVA:%3d,LVL:%3d]:%31s\n",
		&e.health,
		&e.atk,
		&agr,
		&e.hit,
		&e.def,
		&e.exp,
		&e.eva,
		&e.level,
		namebuf
		);
		if (ret == 9) {
			e.agr = agr;
			strncpy(e.name, namebuf, sizeof(e.name) - 1);
			e.name[sizeof(e.name) - 1] = '\0';
			Entity__List_push(&masterEntityList, e);
		} else {
			printf("Warning: Failed to parse entity line %d (fscanf returned %d)\n", i + 1, ret);
		}
    }
	if (DEBUG || ovr) {
		for (int i = 0; i < masterEntityList.size; i++) {
			printf("Entity %3d: %31s:[HP:%3d,ATK:%3d,ARG:%3d,TOH:%3d,DEF:%3d,EXP:%3d,EVA:%3d,LVL:%3d]\n",
				i + 1,
				masterEntityList.items[i].name,
				masterEntityList.items[i].health,
				masterEntityList.items[i].atk,
				masterEntityList.items[i].agr,
				masterEntityList.items[i].hit,
				masterEntityList.items[i].def,
				masterEntityList.items[i].exp,
				masterEntityList.items[i].eva,
				masterEntityList.items[i].level
			);
		}
		printf("\nEntities loaded successfully.\n");
	}
	fclose(file);

}


void loadRooms(Room__List* r, int ovr) {
    char rooms_path[1024];
    path__join("data", "rooms.dat", rooms_path);
    ensure_directory("data");
    FILE* file = fopen(rooms_path, "r");
	if (file == NULL) {
		printf("Error: Could not open rooms.dat for reading.\n");
		return;
	}
	unsigned int roomCount = countLines(file);
	if (roomCount == 0) {
		printf("Error: No rooms found in rooms.dat.\n");
		fclose(file);
		return;
	}
	Room__List_init(r, roomCount);
	Room room;
	memset(&room, 0, sizeof(Room));
	room.startLocation = (Dun_Coord){ XBOUND + 1,YBOUND + 1 };
	room.xdim = XBOUND + 1;
	room.ydim = YBOUND + 1;
	room.roomID = 0;
	for (int side = 0; side < 4; ++side)
		for (int node = 0; node < 4; ++node)
			room.exitNodes[side][node] = (Dun_Coord){ XBOUND + 1, YBOUND + 1 };

	for (unsigned int i = 0; i < roomCount; i++) {
		fscanf(file, "Room %4d: Start Location: [%6d,%6d], Dimensions: [%6d,%6d]\n",
			&room.roomID,
			&room.startLocation.x,
			&room.startLocation.y,
			&room.xdim,
			&room.ydim
		);
		for (unsigned int j = 0; j < 4;j++) {
			room.exitNodes[j][0] = (Dun_Coord){ XBOUND + 1,YBOUND + 1 };
			room.exitNodes[j][1] = (Dun_Coord){ XBOUND + 1,YBOUND + 1 };
			room.exitNodes[j][2] = (Dun_Coord){ XBOUND + 1,YBOUND + 1 };
			room.exitNodes[j][3] = (Dun_Coord){ XBOUND + 1,YBOUND + 1 };
		}
		Room__List_push(r, room);
	}
	fclose(file);

}


void saveRooms() {
    char rooms_path[1024];
    path__join("data", "rooms.dat", rooms_path);
    ensure_directory("data");
	FILE* file = fopen(rooms_path, "w");
	if (file == NULL) {
		printf("Error: Could not open rooms.dat for writing.\n");
		return;
	}
	for (unsigned int i = 0; i < rooms.size; i++) {
		fprintf(file, "Room %4d: Start Location: [%6d,%6d], Dimensions: [%6d,%6d]\n",
			i + 1,
			rooms.items[i].startLocation.x,
			rooms.items[i].startLocation.y,
			rooms.items[i].xdim,
			rooms.items[i].ydim
		);
	}
	fclose(file);
}


void loadItems(int ovr) {
    char items_path[1024];
    path__join("data", "items.dat", items_path);
    ensure_directory("data");
	FILE* file = fopen(items_path, "r");
	if (file == NULL) {
		printf("Error: Could not open file for loading.\n");
		return;
	}
	int numLines = countLines(file);

	Item__List_init(&itemGlossary, numLines);

	Item pusher;

	for (int i = 0; i < numLines; i++) {
		//		[ATK:001,TYPE:0]:PLAIN-SWORD
		fscanf(file, "[%3s:%3d,TYPE:%d]:%31s\n",
			pusher.stat, &pusher.bonus, &pusher.type, pusher.name
		);
		pusher.id = i + 1; // Assign an ID based on the line number
		Item__List_push(&itemGlossary, pusher);
	}
	fclose(file);

	if (DEBUG || ovr) {
		for (int i = 0; i < (int)(itemGlossary.size); i++) {
			printf("Item %3d: %31s:[%3s:%3d,TYPE:%d]\n",
				itemGlossary.items[i].id,
				itemGlossary.items[i].name,
				itemGlossary.items[i].stat,
				itemGlossary.items[i].bonus,
				itemGlossary.items[i].type
			);
		}
		printf("\nItems loaded successfully.\n");
	}

}
