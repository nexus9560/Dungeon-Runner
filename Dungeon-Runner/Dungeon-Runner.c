#ifdef _MSC_VER
#define _CRT_SECURE_NO_WARNINGS
#endif



#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <stdlib.h>

#define XBOUND 21
#define YBOUND 21
#define DEBUG 1
#define MAX_ENTITIES 10
#define MAX_ITEMS 10
#define PLAYER_INVENTORY_BASE 16
#define OLD_MAP 0

#ifdef _WIN32
#define CLEAR_COMMAND "cls"
#elif __unix__ || __APPLE__
#define CLEAR_COMMAND "clear"
#else
#define CLEAR_COMMAND "" // Define it to nothing if OS is not detected
#endif

typedef struct {
	int dx;
	int dy;
} dun_vec;

typedef struct{
	char name[32];
	char stat[3];
	int bonus;
	int type; // 0 = weapon, 1 = armor, 2 = consumable
	int equipped; // 0 = not equipped, 1 = equipped
} Item;

typedef struct{
	int location[2];
	char name[32];
	int health;
	int atk;
	int hit;
	int def;
	int exp;
	int eva;
	int level;
} Entity;

typedef struct {
	Entity base;
	int stepCount;
	int steps[3][2];
	Item inventory[]; // Inventory of items
} Player;

typedef struct{
	int locationID;
	char contents[32];
	int passable; // 0 = not passable, 1 = passable
	int occupied; // 0 = not occupied, 1 = occupied
} Cell;

Cell collection[XBOUND][YBOUND];
Player you;
Entity enemyGlossary[MAX_ENTITIES];
Item itemGlossary[MAX_ITEMS];
const dun_vec up = { -1, 0 };
const dun_vec right = { 0, 1 };
const dun_vec down = { 1, 0 };
const dun_vec left = { 0, -1 };

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
void logStep(int step[2]);
void loadItems(int ovr);
void drawMap();
Entity shiftEntity(Entity e, dun_vec delta);
int checkBounds( int newPos[], dun_vec delta);
//int goUp();
//int goRight();
//int goDown();
//int goLeft();

void main() {

	loadEntities(0);
	loadItems(0);
	if (loadPlayer() != 0) {
		printf("Error loading player position. Starting at default position.\n");
		// Y Position
		you.base.location[0] = YBOUND / 2;
		// X Position
		you.base.location[1] = XBOUND / 2;
		// Name
		printf("Please enter your name:\n");
		if (scanf("%31s", you.base.name) != 1) { // Limit input to 31 characters to prevent buffer overflow
			strcoll(you.base.name, "Illiterate");
		}
		// Health
		you.base.health = 10;
		// Attack
		you.base.atk = 1;
		// To-Hit
		you.base.hit = 1;
		// Defense
		you.base.def = 0;
		// Experience
		you.base.exp = 0;
		// Evasion
		you.base.eva = 0;
		// Level
		you.base.level = 1;
	}
	else {
		printf("%s loaded successfully.\n", you.base.name);
	}
	roomGenerator();
	roomRunner();


}

int loadPlayer() {
	for(int x=0;x<(sizeof(you.steps)/3);x++)
		for (int y = 0;y < (sizeof(you.steps) / 2);y++)
			you.steps[x][y] = -1;
	FILE* file = fopen("player.dat", "r");
	if (file == NULL) {
		printf("Error: Could not open file for loading.\n");
		return 1;
	} else 

	// Check return value of fscanf and handle errors
	if (fscanf(file, "Location:[%4d,%4d]\n", &you.base.location[0], &you.base.location[1]) != 2) {
		printf("Error: Failed to read player location.\n");
		fclose(file);
		return 1;
	}
	if (fscanf(file, "Name:%s\n", you.base.name) != 1) {
		printf("Error: Failed to read player name.\n");
		fclose(file);
		return 1;
	}
	if (fscanf(file, "Health:%d\n", &you.base.health) != 1) {
		printf("Error: Failed to read player health.\n");
		fclose(file);
		return 1;
	}
	if (fscanf(file, "Attack:%d\n", &you.base.atk) != 1) {
		printf("Error: Failed to read player attack.\n");
		fclose(file);
		return 1;
	}
	if (fscanf(file, "To-Hit:%d\n", &you.base.hit) != 1) {
		printf("Error: Failed to read player to-hit.\n");
		fclose(file);
		return 1;
	}
	if (fscanf(file, "Defense:%d\n", &you.base.def) != 1) {
		printf("Error: Failed to read player defense.\n");
		fclose(file);
		return 1;
	}
	if (fscanf(file, "Experience:%d\n", &you.base.exp) != 1) {
		printf("Error: Failed to read player experience.\n");
		fclose(file);
		return 1;
	}
	if (fscanf(file, "Evasion:%d\n", &you.base.eva) != 1) {
		printf("Error: Failed to read player evasion.\n");
		fclose(file);
		return 1;
	}
	if (fscanf(file, "Level:%d\n", &you.base.level) != 1) {
		printf("Error: Failed to read player level.\n");
		fclose(file);
		return 1;
	}



	fclose(file);
	if(DEBUG)
		printf("Player data loaded successfully.\n");
	return 0;
}

void roomGenerator() {
	for (int x = 0;x < XBOUND;x++) {
		for (int y = 0;y < YBOUND;y++) {
			collection[x][y].locationID = (x * 10) + y;
			strcpy(collection[x][y].contents, "Help me joeby juan kenobi");
			if (x > 0 && x < XBOUND - 1 && y > 0 && y < YBOUND - 1) {
				collection[x][y].passable = 1;
			}
			else {
				collection[x][y].passable = 0;
			}
		}
	}
}

void drawMap() {
	printf("\n+----------------------------------------------------------------+\n");
	if (OLD_MAP) {

		for (int y = 0;y < YBOUND;y++) {
			printf("\t");
			for (int x = 0;x < XBOUND;x++) {
				if (((you.base.location[0] * 10) + you.base.location[1]) == collection[x][y].locationID) {
					printf("  ><");
				}
				else {
					printf("%4d", collection[x][y].locationID);
				}

			}
			printf("\n");
		}
	}
	else {
		for (int x = 0;x < XBOUND;x++) {
			printf("\t");
			for (int y = 0;y < YBOUND;y++) {
				if ((x == 0 && y == 0)||(x==0&&y==(YBOUND-1))||(x==(XBOUND-1)&&y==0)||(x==(XBOUND-1)&&y==(YBOUND-1))) {
					printf("+");
				}
				else if (x == 0 || x == XBOUND-1) {
					printf("-");
				}
				else if (y == 0 || y == YBOUND-1) {
					printf("|");
				}
				else if (you.base.location[0] == x && you.base.location[1] == y) {
					printf("@");
				}
				else {
					printf(" ");
				}
			}
			printf("\n");
		}
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

void roomRunner() {
	do {

		
		drawMap();
		if (DEBUG || you.stepCount!=0) {
			printf("Previous steps: [%d,%d] [%d,%d], [%d,%d]\n", you.steps[0][0], you.steps[0][1], you.steps[1][0], you.steps[1][1], you.steps[2][0], you.steps[2][1]);
		}
		int choice = 0;
		printf("\nYour current position is:%4d,%4d\n\n", you.base.location[0], you.base.location[1]);
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
		case 2: inspectElement(you.base.location);
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
	if(dir!=0)
		you.stepCount++;
	switch (dir) {
		case 1: you.base = shiftEntity(you.base,left);break;
		case 2: you.base = shiftEntity(you.base,down);break;
		case 3: you.base = shiftEntity(you.base,right);break;
		case 4: you.base = shiftEntity(you.base,up);break;
		default:break;
	}
	if(!DEBUG)
		clearScreen();
}

void savePlayer() {
	FILE* file = fopen("player.dat", "w");
	if (file == NULL) {
		printf("Error: Could not open or create file for saving.\n");
		return;
	}

	// Write the player's position to the file  
	fprintf(file, "Location:[%4d,%4d]\n", you.base.location[0], you.base.location[1]);
	fprintf(file, "Name:%s\n", you.base.name);
	fprintf(file, "Health:%d\n", you.base.health);
	fprintf(file, "Attack:%d\n", you.base.atk);
	fprintf(file, "To-Hit:%d\n", you.base.hit);
	fprintf(file, "Defense:%d\n", you.base.def);
	fprintf(file, "Experience:%d\n", you.base.exp);
	fprintf(file, "Evasion:%d\n", you.base.eva);
	fprintf(file, "Level:%d\n", you.base.level);

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

//	free(items);
//	free(file);
//	free(numLines);
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
//	free(entities);
//	free(file);
//	free(numLines);

	
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
			printf("Name: %s\n", you.base.name);
			printf("Health: %d\n", you.base.health);
			printf("Attack: %d\n", you.base.atk);
			printf("To-Hit: %d\n", you.base.hit);
			printf("Defense: %d\n", you.base.def);
			printf("Experience: %d\n", you.base.exp);
			printf("Evasion: %d\n", you.base.eva);
			printf("Level: %d\n", you.base.level);
			break;
		}
		case 2: {
			printf("You are in room %d\n", collection[pos[0]][pos[1]].locationID);
			printf("This room has the following things in it: %s\n", collection[pos[0]][pos[1]].contents);
			break;
		}
		case 3: printf("Previous steps: [%d,%d] [%d,%d], [%d,%d]\n", you.steps[0][0], you.steps[0][1], you.steps[1][0], you.steps[1][1], you.steps[2][0], you.steps[2][1]); break;
		default: return;
	}
}

void actOnYourOwn() {
	printf("You act out your fantasies.\n\n");
}

int checkBounds(int newPos[], dun_vec delta) {  
	int temp[2] = { newPos[0], newPos[1] }; 
	// Ensure both arrays have exactly 2 elements  
   if (sizeof(newPos) / sizeof(newPos[0]) != 2 ) {  
       printf("Error: Invalid array size. Both arrays must have exactly 2 elements.\n");  
       return 0;  
   }  

   temp[0] += delta.dx;  
   temp[1] += delta.dy;  

   return (temp[0] >= 0 && temp[0] < XBOUND) && (temp[1] >= 0 && temp[1] < YBOUND) && collection[temp[0]][temp[1]].passable;  
}

Entity shiftEntity(Entity e, dun_vec delta) {
	if(DEBUG)
		printf("Entity %s is moving from [%d,%d] to [%d,%d]\n", e.name, e.location[0], e.location[1], e.location[0] + delta.dx, e.location[1] + delta.dy);
	if (checkBounds(e.location, delta)) {
		e.location[0] += delta.dx;
		e.location[1] += delta.dy;
	}
	else {
		printf("Error: %s cannot move out of bounds.\n",e.name);
	}
	logStep(e.location);
	
	
	return e;
}

/*
int goLeft() {
	you.stepCount++;
	if (checkBounds(you.base.location, {0,-1})) {
		printf("You can't go any further that way...\n");
		you.base.location[1] = 0;
		return 1;
	}
	else {
		you.base.location[1] -= 1;
		logStep(you.base.location);
		return 0;
	}
}

int goDown() {
	you.stepCount++;
	if (you.base.location[0] >= (XBOUND - 1)) {
		printf("You can't go any further that way...\n");
		you.base.location[0] = (XBOUND - 1);
		return 1;
	}
	else {
		you.base.location[0] += 1;
		logStep(you.base.location);
		return 0;
	}
}

int goRight() {
	you.stepCount++;
	if (you.base.location[1] >= (YBOUND - 1)) {
		printf("You can't go any further that way...\n");
		you.base.location[1] = (YBOUND - 1);
		return 1;
	}
	else {
		you.base.location[1] += 1;
		logStep(you.base.location);
		return 0;
	}
}

int goUp() {
	you.stepCount++;
	if (you.base.location[0] <= 0) {
		printf("You can't go any further that way...\n");
		you.base.location[0] = 0;
		return 1;
	}
	else {
		you.base.location[0] -= 1;
		logStep(you.base.location);
		return 0;
	}
}
*/

void logStep(int step[2]) {
	int temp[2][2] = { { -1,-1 },{ -1,-1 } };
	temp[0][0] = you.steps[0][0];
	temp[0][1] = you.steps[0][1];
	temp[1][0] = you.steps[1][0];
	temp[1][1] = you.steps[1][1];
	you.steps[0][0] = step[0];
	you.steps[0][1] = step[1];
	you.steps[1][0] = temp[0][0];
	you.steps[1][1] = temp[0][1];
	you.steps[2][0] = temp[1][0];
	you.steps[2][1] = temp[1][1];
	free(temp);

}
