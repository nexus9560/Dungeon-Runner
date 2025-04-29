#ifdef _MSC_VER
#define _CRT_SECURE_NO_WARNINGS
#endif

#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <stdlib.h>

#define XBOUND 9
#define YBOUND 9
#define LOGCOUNT 8
#define DEBUG 1
#define MAX_ENTITIES 10
#define MAX_ITEMS 10

#ifdef _WIN32
#define CLEAR_COMMAND "cls"
#elif __unix__ || __APPLE__
#define CLEAR_COMMAND "clear"
#else
#define CLEAR_COMMAND "" // Define it to nothing if OS is not detected
#endif

//Dungeon Vector ;)
typedef struct {
    int x;
    int y;
} dung_vec;

// Yes, the player struct is currently identical to the Entity struct. This is not going to stay this way, as the player will eventually get an inventory, which entities will not.
// The player will also have a different set of stats, as the player will be able to level up and gain experience, while entities will not.
typedef struct {
	dung_vec pos;
	char name[32];
	int health;
	int atk;
	int hit;
	int def;
	int exp;
	int eva;
	int level;
} Player;

typedef struct{
	char name[32];
	char stat[3];
	int bonus;
	int type; // 0 = weapon, 1 = armor, 2 = consumable
	int equipped; // 0 = not equipped, 1 = equipped
} Item;

typedef struct{
	dung_vec pos;
	char name[32];
	int health;
	int atk;
	int hit;
	int def;
	int exp;
	int eva;
	int level;
} Entity;

typedef struct{
	int roomID;
	char contents[32];
} Room;

Room collection[XBOUND][YBOUND];
Player you;

//Our current position in the stepLog ring buffer
unsigned int currentPos = 0;
dung_vec stepLog[LOGCOUNT] = {0};
Entity enemyGlossary[MAX_ENTITIES];
Item itemGlossary[MAX_ITEMS];
int stepCount = 0;

void roomGenerator();
void savePlayer();
int countLines(FILE* file);
int loadPlayer();
void loadEntities(int ovr);
void roomRunner();
void clearScreen();
void changePosition();
void inspectElement(int pos[]);
void actOnYourOwn();
void exitAction(int ec);
void logStep(dung_vec finalPos);
void loadItems(int ovr);
void drawMap();
int goUp();
int goRight();
int goDown();
int goLeft();

void main() {

	loadEntities(0);
	if (loadPlayer() != 0) {
		printf("Error loading player position. Starting at default position.\n");
		// Y Position
		you.pos.y = YBOUND / 2;
		// X Position
		you.pos.x = XBOUND / 2;
		// Name
		printf("Please enter your name:\n");
		if (scanf("%31s", you.name) != 1) { // Limit input to 31 characters to prevent buffer overflow
			strcoll(you.name, "Illiterate");
		}
		// Health
		you.health = 10;
		// Attack
		you.atk = 1;
		// To-Hit
		you.hit = 1;
		// Defense
		you.def = 0;
		// Experience
		you.exp = 0;
		// Evasion
		you.eva = 0;
		// Level
		you.level = 1;
	}
	else {
		printf("%s loaded successfully.\n", you.name);
	}
	roomGenerator();
	roomRunner();


}

int loadPlayer() {

	FILE* file = fopen("player.dat", "r");
	if (file == NULL) {
		printf("Error: Could not open file for loading.\n");
		return 1;
	} else 
    {
	// Check return value of fscanf and handle errors
	if (fscanf(file, "Location:[%4d,%4d]\n", &(you.pos.x), &(you.pos.y)) != 2) {
		printf("Error: Failed to read player location.\n");
		fclose(file);
		return 1;
	}
	if (fscanf(file, "Name:%s\n", you.name) != 1) {
		printf("Error: Failed to read player name.\n");
		fclose(file);
		return 1;
	}
	if (fscanf(file, "Health:%d\n", &you.health) != 1) {
		printf("Error: Failed to read player health.\n");
		fclose(file);
		return 1;
	}
	if (fscanf(file, "Attack:%d\n", &you.atk) != 1) {
		printf("Error: Failed to read player attack.\n");
		fclose(file);
		return 1;
	}
	if (fscanf(file, "To-Hit:%d\n", &you.hit) != 1) {
		printf("Error: Failed to read player to-hit.\n");
		fclose(file);
		return 1;
	}
	if (fscanf(file, "Defense:%d\n", &you.def) != 1) {
		printf("Error: Failed to read player defense.\n");
		fclose(file);
		return 1;
	}
	if (fscanf(file, "Experience:%d\n", &you.exp) != 1) {
		printf("Error: Failed to read player experience.\n");
		fclose(file);
		return 1;
	}
	if (fscanf(file, "Evasion:%d\n", &you.eva) != 1) {
		printf("Error: Failed to read player evasion.\n");
		fclose(file);
		return 1;
	}
	if (fscanf(file, "Level:%d\n", &you.level) != 1) {
		printf("Error: Failed to read player level.\n");
		fclose(file);
		return 1;
	}

	fclose(file);
	if(DEBUG)
		printf("Player position loaded successfully.\n");
	return 0;
    }
}

void roomGenerator() {
	for (int x = 0;x < XBOUND;x++) {
		for (int y = 0;y < YBOUND;y++) {
			collection[x][y].roomID = (x * 10) + y;
			strcpy(collection[x][y].contents, "Help me joeby juan kenobi");
		}
	}
}

void drawMap() {
	printf("\n+----------------------------------------------------------------+\n");
	for (int x = 0;x < XBOUND;x++) {
		printf("\t");
		for (int y = 0;y < YBOUND;y++) {
			if (((you.pos.y * 10) + you.pos.x) == collection[x][y].roomID) {
				printf("  ><");
			}
			else {
				printf("%4d", collection[x][y].roomID);
			}

		}
		printf("\n");
	}
	printf("\n\n\n+----------------------------------------------------------------+\n\n");
}

void clearScreen() {
	if (CLEAR_COMMAND[0] != '\0') { // Check if CLEAR_COMMAND is not empty
		system(CLEAR_COMMAND);
	}
	else {
		//printf("Operating system not detected.  Cannot clear screen.\n");
		//  You could use the following as a fallback.
		//  for (int i = 0; i < 50; ++i)
		//      printf("\n");
	}
}

//Prints the previous steps starting at the current step
void printPreviousSteps(){
    printf("Previous Steps:");
    for(unsigned int i = 0; i < LOGCOUNT; i ++){
        printf(" [%d, %d]", 
                stepLog[(currentPos - i - 1) % LOGCOUNT].x, 
                stepLog[(currentPos - i - 1) % LOGCOUNT].y
               );
    }
    printf("\n");
}

void roomRunner() {
	do {

		
		drawMap();
		if (DEBUG || stepCount!=0) {
            printPreviousSteps();
			//printf("Previous steps: [%d,%d] [%d,%d], [%d,%d]\n", steps[0][0], steps[0][1], steps[1][0], steps[1][1], steps[2][0], steps[2][1]);
		}
		int choice = 0;
		printf("\nYour current position is:%4d,%4d\n\n", you.pos.x, you.pos.y);
		printf("What will you do?\n");
		printf("0 - Exit\n");
		printf("1 - Move\n");
		printf("2 - Inspect\n");
		printf("3 - Act\n");
		printf("4 - Save\n");
		printf("5 - Load\n");
		if (DEBUG) {
			printf("6 - Debug\n");
		}
		printf("\n\n");
		scanf("%d", &choice);
		
		clearScreen();

		drawMap();

		switch (choice) {
		case 0:
			exitAction(0);
		case 1: changePosition();
			break;
		case 2: inspectElement(&you.pos); //ayyye lamo, it's c calm down
			break;
		case 3: actOnYourOwn();
			break;
		case 4: savePlayer();
			break;
		case 5: loadPlayer();
			break;
		case 6: if (DEBUG) {
			loadEntities(0);
			break;
		} else {continue;}
		default:printf("Could you try that again?\n");
			continue;
		}
		
	} while (1);
}

void changePosition() {
	printf("0 - Go back\n");
	printf("1 - Move Left\n");
	printf("2 - Move Down\n");
	printf("3 - Move Right\n");
	printf("4 - Move Up\n\n");
	int dir = 0;
	scanf("%d", &dir);
	switch (dir) {
	case 1: goLeft();break;
	case 2: goDown();break;
	case 3: goRight();break;
	case 4: goUp();break;
	default:break;
	}


}

void savePlayer() {
	FILE* file = fopen("player.dat", "w");
	if (file == NULL) {
		printf("Error: Could not open or create file for saving.\n");
		return;
	}

	// Write the player's position to the file  
	fprintf(file, "Location:[%4d,%4d]\n", you.pos.y, you.pos.x);
	fprintf(file, "Name:%s\n", you.name);
	fprintf(file, "Health:%d\n", you.health);
	fprintf(file, "Attack:%d\n", you.atk);
	fprintf(file, "To-Hit:%d\n", you.hit);
	fprintf(file, "Defense:%d\n", you.def);
	fprintf(file, "Experience:%d\n", you.exp);
	fprintf(file, "Evasion:%d\n", you.eva);
	fprintf(file, "Level:%d\n", you.level);

	fclose(file);
	printf("Player position saved successfully.\n");
}

void exitAction(int ec) {
	if (ec == 0) {
		savePlayer();
		printf("Exiting and saving...\n");
		exit(0);
	}
	else {
		printf("Exiting without saving...\n");
		exit(0);
	}

}

int countLines(FILE* file) {
	int lines = 0;
	char ch;
	while ((ch = fgetc(file)) != EOF) {
		if (ch == '\n') {
			lines++;
		}
	}
	rewind(file); // Reset file pointer to the beginning
	if (DEBUG) {
		printf("Number of lines: %d\n", lines);
	}
	return lines;
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

	for(int x = 0;x < numLines; x++)
        itemGlossary[x] = items[x];

	free(items);
}

void loadEntities(int ovr) {
	FILE* file = fopen("entities.dat", "r");
	if (file == NULL) {
		printf("Error: Could not open file for loading.\n");

		return;
	}
	int numLines = countLines(file);
	Entity* entities = malloc(numLines * sizeof(Entity));
	if (entities == NULL) {
		printf("Error: Memory allocation failed.\n");
		fclose(file);
		return;
	}
	for (int i = 0; i < numLines; i++) {
		// Use [HP:  2,ATK:  1,TOH:  1,DEF:  0,EXP:  2,EVA:  0,LVL:  1]:NAME as the format
		fscanf(file, "[HP: %d,ATK: %d,TOH: %d,DEF: %d,EXP: %d,EVA: %d,LVL: %d]:%31s\n",
			&entities[i].health, &entities[i].atk, &entities[i].hit, &entities[i].def, &entities[i].exp, &entities[i].eva, &entities[i].level, &entities[i].name
		);
	}
	if (DEBUG || ovr) {
		for (int i = 0; i < numLines; i++) {
			printf("Entity %3d: %31s:[HP:%3d,ATK:%3d,TOH:%3d,DEF:%3d,EXP:%3d,EVA:%3d,LVL:%3d]\n",
				i + 1,
				entities[i].name,
				entities[i].health,
				entities[i].atk,
				entities[i].hit,
				entities[i].def,
				entities[i].exp,
				entities[i].eva,
				entities[i].level
			);
		}
		printf("\nEntities loaded successfully.\n");
	}
	fclose(file);
	if (numLines > MAX_ENTITIES) {
		printf("Warning: More entities than expected. Only the first %d will be used.\n", MAX_ENTITIES);
		numLines = MAX_ENTITIES;
	}
	for (int i = 0; i < numLines; i++) {
		enemyGlossary[i] = entities[i];
	}
	// In the future, these may be massive, so for the sake of memory efficiency we will free them up.
	free(entities);
}

void inspectElement(int pos[]) {
	int res = 0;
	printf("Inspect what?\n");
	printf("0 - Go back\n");
	printf("1 - Inspect yourself\n");
	printf("2 - Inspect the room\n");
	printf("3 - Trace your steps\n\n");
	scanf("%d", &res);
	clearScreen();
	switch (res) {
		case 1: {
			printf("You are:\n");
			printf("Name: %s\n", you.name);
			printf("Health: %d\n", you.health);
			printf("Attack: %d\n", you.atk);
			printf("To-Hit: %d\n", you.hit);
			printf("Defense: %d\n", you.def);
			printf("Experience: %d\n", you.exp);
			printf("Evasion: %d\n", you.eva);
			printf("Level: %d\n", you.level);
			break;
		}
		case 2: {
			printf("You are in room %d\n", collection[pos[0]][pos[1]].roomID);
			printf("This room has the following things in it: %s\n", collection[pos[0]][pos[1]].contents);
			break;
		}
		case 3: printPreviousSteps();
                
		default: return;
    }
}

void actOnYourOwn() {
	printf("You act out your fantasies.\n\n");
}

// Generic displacement function for shifting player relative to current 
// position and bounded by 0 and X/YBOUNDS
int shiftPlayer(dung_vec disp){
    you.pos = (dung_vec){
        //clip displacement to bounds
        .y = max(0, min(YBOUND -1, you.pos.y + disp.y)),
        .x = max(0, min(XBOUND -1, you.pos.x + disp.x))
    };
}

int goLeft() {
    shiftPlayer((dung_vec){.x = -1, .y=0});
    logStep(you.pos);
    //It's not really important that you move, but the thought that counts
    stepCount ++;
}

int goDown() {
    shiftPlayer((dung_vec){.x = 0, .y=1});
    logStep(you.pos);
    //It's not really important that you move, but the thought that counts
    stepCount ++;
}

int goRight() {
    shiftPlayer((dung_vec){.x = 1, .y=0});
    logStep(you.pos);
    //It's not really important that you move, but the thought that counts
    stepCount ++;
}

int goUp() {
    shiftPlayer((dung_vec){.x = 0, .y=-1});
    logStep(you.pos);
    //It's not really important that you move, but the thought that counts
    stepCount ++;
}

void logStep(dung_vec finalPos) {

    //This produces a "ring buffer"
    stepLog[currentPos] = finalPos;

    //Just ask the machine about this one
    currentPos = currentPos >= LOGCOUNT ? 0: currentPos + 1;
}
