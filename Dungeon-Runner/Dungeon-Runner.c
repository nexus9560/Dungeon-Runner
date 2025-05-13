#ifdef _MSC_VER
#define _CRT_SECURE_NO_WARNINGS
#endif



#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <stdlib.h>
#include <ctype.h>
#include <time.h>
#include <math.h>

#define XBOUND 256
#define YBOUND 512
#define DEBUG 1
#define MAX_ENTITIES 10
#define MAX_ITEMS 10
#define PLAYER_INVENTORY_BASE 16
#define OLD_MAP 0
#define OLD_ACTIONS 0
#define LOG_BUFFER 4

#ifdef _WIN32
#define CLEAR_COMMAND "cls"
#include <conio.h>
#include <windows.h>
#elif __unix__ || __APPLE__
#define CLEAR_COMMAND "clear"
#include <termios.h>
#include <unistd.h>

#else
#define CLEAR_COMMAND "" // Define it to nothing if OS is not detected
#endif

typedef struct {
	int dx;
	int dy;
} Dun_Vec;

typedef struct {
	int x;
	int y;
} Dun_Coord;

typedef struct{
	char name[32];
	char stat[3];
	int bonus;
	int type; // 0 = weapon, 1 = armor, 2 = consumable
	int equipped; // 0 = not equipped, 1 = equipped
} Item;

typedef struct{
	Dun_Coord location;
	char name[32];
	int health;
	int curHealth;
	int atk;
	int agr;
	int hit;
	int def;
	int exp;
	int eva;
	unsigned int level;
	Dun_Coord stepLog[LOG_BUFFER];
	unsigned int currentPos;
} Entity;

typedef struct {
	Entity base;
	int stepCount;
	Item* inventory; // Inventory of items
} Player;

typedef struct {
	Dun_Coord startLocation; // Corner location in the world map, as well as the initial offset
	unsigned int xdim;
	unsigned int ydim;
	Entity* entities; // Array of pointers to entities

} Room;

typedef struct{
	int locationID;
	char contents[32];
	int passable; // 0 = not passable, 1 = passable
	int occupied; // 0 = not occupied, 1 = occupied
} Cell;



Cell world[XBOUND][YBOUND];
Player you;
Entity* enemyGlossary;
int enemyGlossarySize;
Item itemGlossary[MAX_ITEMS];
const Dun_Vec up	= { -1,  0 };
const Dun_Vec right = {  0,  1 };
const Dun_Vec down	= {  1,  0 };
const Dun_Vec left	= {  0, -1 };
Room test = { { 0, 0 }, XBOUND, YBOUND, NULL };

void roomGenerator();
void savePlayer();
int countLines(FILE* file);
int loadPlayer();
int* getConsoleWindow();
void loadEntities(int ovr);
void roomRunner();
void clearScreen();
void changePosition();
void inspectElement(Dun_Coord pos);
void actOnYourOwn();
void exitAction(int ec);
void actionChecker();
void delay(int seconds);
void showPlayerInventory();
char* printPlayerStatus(int brief);
Entity logStep(Entity e);
void loadItems(int ovr);
void drawMap();
Entity shiftEntity(Entity e, Dun_Vec delta);
int checkBounds( Dun_Coord newPos, Dun_Vec delta);
int checkOccupied(Dun_Coord newPos, Dun_Vec delta);
int checkArea(Room room1, Room room2);
Room makeRoom();
Room getRoomByLocation(Dun_Coord d);
int getRandomEnemyIndex();
int isInRoom(Room r, Dun_Coord d);
Dun_Coord getNearestSafeLocation(Dun_Coord d);
//int goUp();
//int goRight();
//int goDown();
//int goLeft();

void main() {

	loadEntities(0);
	loadItems(0);
	test = makeRoom();
	if (loadPlayer() != 0) {
		you.base.currentPos = 0;
		printf("Error loading player position. Starting at default position.\n");
		// Y Position
		you.base.location.y = YBOUND / 2;
		// X Position
		you.base.location.x = XBOUND / 2;
		// Name
		printf("Please enter your name:\n");
		if (scanf("%31s", you.base.name) != 1) { // Limit input to 31 characters to prevent buffer overflow
			strcoll(you.base.name, "Illiterate");
		}
		// Health
		you.base.health = 10;
		you.base.curHealth = 0+you.base.health;
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
		if (!checkBounds(you.base.location, (Dun_Vec) { 0, 0 })) {
			if(DEBUG)
				printf("Player position is within bounds.\n");
		}
		else {
			if(DEBUG)
				printf("Player position is out of bounds, teleporting you to the middle of the nearest room.\n");
			you.base.location.x = XBOUND / 2;
			you.base.location.y = YBOUND / 2;
		}
	}
	//int* consoleDimensions = getConsoleWindow();
	//printf("Console dimensions: %d rows, %d columns\n", consoleDimensions[0], consoleDimensions[1]);
	roomGenerator();
	roomRunner();


}

int loadPlayer() {
	FILE* file = fopen("player.dat", "r");
	
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
	if(DEBUG)
		printf("Player data loaded successfully.\n");
	return 0;
}

void roomGenerator() {
	for (int x = 0;x < XBOUND;x++) {
		for (int y = 0;y < YBOUND;y++) {
			world[x][y].locationID = (x * YBOUND) + y;
			strcpy(world[x][y].contents, "Help me joeby juan kenobi");
			if (x > 0 && x < XBOUND - 1 && y > 0 && y < YBOUND - 1) {
				world[x][y].passable = 1;
			}
			else {
				world[x][y].passable = 0;
			}
		}
	}
	
}

int* getConsoleWindow() {  
   static int dimensions[2] = {0, 0}; // [rows, columns]

#ifdef _WIN32
   CONSOLE_SCREEN_BUFFER_INFO csbi;
   if (GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &csbi)) {
       dimensions[0] = csbi.srWindow.Bottom - csbi.srWindow.Top + 1; // Rows
       dimensions[1] = csbi.srWindow.Right - csbi.srWindow.Left + 1; // Columns
   }
#elif __unix__ || __APPLE__
   struct winsize w;
   if (ioctl(STDOUT_FILENO, TIOCGWINSZ, &w) == 0) {
       dimensions[0] = w.ws_row;    // Rows
       dimensions[1] = w.ws_col;    // Columns
   }
#else
   dimensions[0] = 24; // Default rows
   dimensions[1] = 80; // Default columns
#endif

   return dimensions;
}

void drawMap() {
	
	if (OLD_MAP) {
		char* map = (char*)malloc(XBOUND * YBOUND);
		if (map == NULL) {
			printf("Memory allocation failed\n");
			return;
		}
		for (int x = 0;x < XBOUND;x++) {
			for (int y = 0;y < YBOUND;y++) {
				if ((x == 0 && y == 0) || (x == 0 && y == (YBOUND - 1)) || (x == (XBOUND - 1) && y == 0) || (x == (XBOUND - 1) && y == (YBOUND - 1))) {
					map[x * YBOUND + y] = '+';
				}
				else if (x == 0 || x == XBOUND - 1) {
					map[x * YBOUND + y] = '-';
				}
				else if (y == 0 || y == YBOUND - 1) {
					map[x * YBOUND + y] = '|';
				}
				else if (you.base.location.x == x && you.base.location.y == y) {
					map[x * YBOUND + y] = '@';
				}
				else if (world[x][y].passable == 0) {
					map[x * YBOUND + y] = '#';
				}
				else if (world[x][y].occupied == 1) {
					map[x * YBOUND + y] = 'X';
				}
				else {
					map[x * YBOUND + y] = ' ';
				}
			}
			printf("%.*s\n", YBOUND, &map[x * YBOUND]);

		}
	}
	else {
		int* conDims = getConsoleWindow();
		int renderX = (int)((conDims[0] * 0.75) > XBOUND ? XBOUND : (conDims[0] * 0.75));
		int renderY = (int)((conDims[1] * 0.80) > YBOUND ? YBOUND : (conDims[1] * 0.80));
		char* map = (char*)malloc(renderX * renderY);
		if (map == NULL) {
			printf("Memory allocation failed\n");
			return;
		}

		
		for (int x = 0; x < renderX;x++) {
			for (int y = 0;y < renderY;y++) {
				if ((x == 0 && y == 0) || (x == 0 && y == (renderY - 1)) || (x == (renderX - 1) && y == 0) || (x == (renderX - 1) && y == (renderY - 1))) {
					map[x * renderY + y] = '+';
				}
				else if (x == 0 || x == renderX - 1) {
					map[x * renderY + y] = '-';
				}
				else if (y == 0 || y == renderY - 1) {
					map[x * renderY + y] = '|';
				}
				else if (you.base.location.x == x && you.base.location.y == y) {
					map[x * renderY + y] = '@';
				}
				else if (world[x][y].passable == 0) {
					map[x * renderY + y] = '#';
				}
				else if (world[x][y].occupied == 1) {
					map[x * renderY + y] = 'X';
				}
				else {
					map[x * renderY + y] = ' ';
				}
			}
			printf("%.*s\n", renderY, &map[x * renderY]);
		}


	}
	
	
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
	int isBrief = 1;
	do {
		
		drawMap();
		printf(printPlayerStatus(isBrief));
		actionChecker();
		//delay(1);
		clearScreen();
		
	} while (1);
}


void actionChecker() {
   if (OLD_ACTIONS) {
	   if (DEBUG || you.stepCount != 0) {
		   for (int i = 0; i < (sizeof(you.base.stepLog) / LOG_BUFFER); i++) {
			   printf("Step %d: [%4d,%4d], ", i + 1, you.base.stepLog[i].x, you.base.stepLog[i].y);
		   }
		   printf("\n\n");
	   }
	   int choice = 0;
	   printf("\nYour current position is:%4d,%4d\n\n", you.base.location.x, you.base.location.y);
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
		   } else { return; }
		   default:
			   printf("Could you try that again?\n");
			   return;
	   }
   }
   else {
	   printf("WASD to move\n");
	   printf("Q to quit\n");
	   printf("I to Open Inventory\n");
	   printf("E to Interact\n");
	   printf("F to Inspect\n");
	   printf("Spacebar to Attack\n");
#ifdef _WIN32
	   
		int ch;
		if (_kbhit()) {
			ch = _getch(); // Get the actual key code
			ch = tolower(ch);
			if(DEBUG)
				printf("Key pressed: %c : %d\n", ch,ch);
			switch (ch) {
				case 'w':
				case 72: // Up arrow
					you.base = shiftEntity(you.base, up);
					break;
				case 'a':
				case 75: // Left arrow
					you.base = shiftEntity(you.base, left);
					break;
				case 'd':
				case 77: // Right arrow
					you.base = shiftEntity(you.base, right);
					break;
				case 's':
				case 80: // Down arrow
					you.base = shiftEntity(you.base, down);
					break;
				case 'i':
					printf("Inventory not implemented yet.\n");
					break;
				case 'e':
					printf("Interact not implemented yet.\n");
					break;
				case 'f':
					printf("Inspect not implemented yet.\n");
					break;
				case ' ':
					printf("Attack not implemented yet.\n");
					break;

				case 'q':
				case 81: // Q key
					exitAction(0);
				default:break;
			}
		}
	   
	   
	   

#elif __unix__ || __APPLE__
	   struct termios oldt, newt;
	   char ch;
	   // Get the terminal settings for stdin
	   tcgetattr(STDIN_FILENO, &oldt);
	   newt = oldt; // Copy the old settings to new settings
	   newt.c_lflag &= ~(ICANON | ECHO); // Disable canonical mode and echo
	   tcsetattr(STDIN_FILENO, TCSANOW, &newt); // Apply the new settings
	   ch = getchar(); // Read a single character
	   // Restore the old terminal settings
	   tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
	   ch = tolower(ch);
	   if (DEBUG)
		   printf("Key pressed: %c : %d\n", ch, ch);
	   switch (ch) {
	   case 'w':
		   you.base = shiftEntity(you.base, up);
		   break;
	   case 'a':
		   you.base = shiftEntity(you.base, left);
		   break;
	   case 'd':
		   you.base = shiftEntity(you.base, right);
		   break;
	   case 's':
		   you.base = shiftEntity(you.base, down);
		   break;
	   case 'i':
		   printf("Inventory not implemented yet.\n");
		   break;
	   case 'e':
		   printf("Interact not implemented yet.\n");
		   break;
	   case 'f':
		   printf("Inspect not implemented yet.\n");
		   break;
	   case ' ':
		   printf("Attack not implemented yet.\n");
		   break;
	   case 'q':
		   exitAction(0);
	   default:break;
	   }
#else
	   printf("Operating system not detected for raw input.\n");
	   exitAction(0);
#endif

   }
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
	enemyGlossarySize = countLines(file);
	Entity* entities = malloc(enemyGlossarySize * sizeof(Entity));
	if (entities == NULL) {
		printf("Error: Memory allocation failed.\n");
		fclose(file);
		return;
	}
	for (int i = 0; i < enemyGlossarySize; i++) {
		// Use [HP:  2,ATK:  1,TOH:  1,DEF:  0,EXP:  2,EVA:  0,LVL:  1]:NAME as the format
		fscanf(file, "[HP: %d,ATK: %d,TOH: %d,DEF: %d,EXP: %d,EVA: %d,LVL: %d]:%31s\n",
			&entities[i].health, &entities[i].atk, &entities[i].hit, &entities[i].def, &entities[i].exp, &entities[i].eva, &entities[i].level, &entities[i].name
		);
	}
	if (DEBUG || ovr) {
		for (int i = 0; i < enemyGlossarySize; i++) {
			printf("Entity %3d: %31s:[HP:%3d,ATK:%3d,ARG:%3d,TOH:%3d,DEF:%3d,EXP:%3d,EVA:%3d,LVL:%3d]\n",
				i + 1,
				entities[i].name,
				entities[i].health,
				entities[i].atk,
				entities[i].agr,
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
	enemyGlossary = malloc(enemyGlossarySize * sizeof(Entity));
	if (enemyGlossary == NULL) {
		printf("Error: Memory allocation failed.\n");
		free(entities);
		return;
	}
	for (int i = 0; i < enemyGlossarySize; i++) {
		enemyGlossary[i] = entities[i];
	}
	free(entities);

	
}

void inspectElement(Dun_Coord pos) {  
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
           printf("Health: %d / %d\n", you.base.curHealth, you.base.health);  
           printf("Attack: %d\n", you.base.atk);  
           printf("To-Hit: %d\n", you.base.hit);  
           printf("Defense: %d\n", you.base.def);  
           printf("Experience: %d\n", you.base.exp);  
           printf("Evasion: %d\n", you.base.eva);  
           printf("Level: %d\n", you.base.level);  
           break;  
       }  
       case 2: {  
           printf("You are in cell %d\n", world[pos.x][pos.y].locationID);
		   for (int x = 0;x < XBOUND;x++) {
			   for (int y = 0;y < YBOUND;y++) {
				   if (world[x][y].occupied == 1 && !(x == you.base.location.x && y == you.base.location.y)) {
					   printf("There is an entity in this room.\n");
					   break;
				   }
			   }
		   }
           break;  
       }  
       case 3:  
           for (int i = 0; i < LOG_BUFFER; i++) { // Ensure the loop respects the buffer size  
               printf("Step %d: [%4d,%4d], ", (i + 1), you.base.stepLog[i].x, you.base.stepLog[i].y);  
           }  
           printf("\n\n");  
           break;  
       default: return;  
   }
   scanf("");
}

void actOnYourOwn() {
	printf("You act out your fantasies.\n\n");
}

int checkOccupied(Dun_Coord newPos, Dun_Vec delta) {
	if (world[newPos.x+delta.dx][newPos.y+delta.dy].occupied == 1) {
		if(DEBUG)
			printf("Error: This space is already occupied.\n");
		return 1;
	}
	else {
		return 0;
	}
}

void delay(int seconds) {
#ifdef WIN32
		Sleep(seconds * 1000);
#elif __unix__ || __APPLE__
		struct timespec ts;
		ts.tv_sec = seconds;  // 1 second
		ts.tv_nsec = 0; // 0 nanoseconds
		nanosleep(&ts, NULL);
#endif
}

int checkBounds(Dun_Coord newPos, Dun_Vec delta) {  
	int temp[2] = { newPos.x, newPos.y }; 
	

   temp[0] += delta.dx;  
   temp[1] += delta.dy;  

   return (temp[0] >= 0 && temp[0] < XBOUND) && (temp[1] >= 0 && temp[1] < YBOUND) && world[temp[0]][temp[1]].passable;
}

int checkArea(Room room1, Room room2) {
	return 0;
	//Not implemented yet, once properly implemented it'll check and see if two rooms overlap.
}

Entity shiftEntity(Entity e, Dun_Vec delta) {
	if(DEBUG)
		printf("Entity %s is moving from [%d,%d] to [%d,%d]\n", e.name, e.location.x, e.location.y, e.location.x + delta.dx, e.location.y + delta.dy);
	if (checkBounds(e.location, delta)) {
		world[e.location.x][e.location.y].occupied = 0;
		e.location.x += delta.dx;
		e.location.y += delta.dy;
		world[e.location.x][e.location.y].occupied = 1;
	}
	else {
		if(DEBUG)
			printf("Error: %s cannot move out of bounds.\n",e.name);
	}
	
	
	
	return logStep(e);
}

int isInRoom(Room r, Dun_Coord d) {
	if (d.x >= r.startLocation.x && d.x < r.startLocation.x + r.xdim &&
		d.y >= r.startLocation.y && d.y < r.startLocation.y + r.ydim) {
		return 1; // In room
	}
	return 0; // Not in room
}

Dun_Coord getNearestSafeLocation(Dun_Coord d) {
	Dun_Coord ret = { XBOUND/2,YBOUND/2 };
	//fill in later
	//but what it should do is go through the array of rooms looking for the nearest place in a room
	// and return that location when it finds it
	/*
	int* dval = malloc(sizeof(int) * numberOfRooms);
	for (int i=0; i<numberOfRooms;i++){
		dval[i] = abs(d.x - room[i].startLocation.x) + abs(d.y - room[i].startLocation.y);
	}
	int dret = dval[0];
	for (int i=0; i<numberOfRooms;i++){
		if (dval[i] < dret){
			dret = dval[i];
		}
	}
	*/
	return ret;
}

Room getRoomByLocation(Dun_Coord d) {
	Room temp = { {0,0},XBOUND,YBOUND,NULL};
	//fill in later
	//but what it should do is go through the array of rooms looking for what coordinate is in the room
	// and return that room when it finds it
	/*
	for (int i=0; i<numberOfRooms;i++){
		if (isInRoom(room[i],d){
			return rooms[i];
		}
	}
	
	*/
	return temp;
}

void showPlayerInventory() {
	clearScreen();
	if (you.inventory == NULL) {
		printf("Inventory is empty.\n");
		scanf("");
		return;
	}
	else {
		printf("Inventory:\n");
		for (int i = 0; i < PLAYER_INVENTORY_BASE; i++) {
			if (you.inventory[i].equipped == 1) {
				printf("%s (Equipped)\n", you.inventory[i].name);
			}
			else {
				printf("%s\n", you.inventory[i].name);
			}
		}
		scanf("");
	}
}

int getRandomEnemyIndex() {
	srand((unsigned int)time(NULL)); // Seed the random number generator with the current time
	return rand() % sizeof(enemyGlossary); // Generate a random number between 0 and MAX_ENTITIES - 1
}

char* printPlayerStatus(int brief) {
	char* ret;
	if (brief) {
		ret = malloc(256);
		double experienceValue = (pow(2, abs(you.base.level)));
		if (ret == NULL) {
			printf("Memory allocation failed\n");
			return NULL;
		}
		snprintf(ret, 256,
			"+--------------------------------------------------->\n"
			"| %s\n"
			"| HP : %4d / %4d\n"
			"| EXP: %4d / %4.0f\n"
			"|\n"
			"V\n",
            you.base.name, you.base.curHealth, you.base.health, you.base.exp, experienceValue);
	}
	else {
		ret = malloc(512);
		if (ret == NULL) {
			printf("Memory allocation failed\n");
			return NULL;
		}
		snprintf(ret, 512,
			"+--------------------------------------------------->\n"
			"| %s\n"
			"| HP: %d / %d\n"
			"| Attack: %d\n"
			"| To-Hit: %d\n"
			"| Defense: %d\n"
			"| Experience: %d\n"
			"| Evasion: %d\n"
			"| Level: %d\n"
			"|\n"
			"V\n",
			you.base.name, you.base.curHealth, you.base.health, you.base.atk,
			you.base.hit, you.base.def, you.base.exp, you.base.eva, you.base.level);
	}
	return ret;
}

Room makeRoom() {
	Room temp;
	temp.startLocation.x = 0;
	temp.startLocation.y = 0;
	temp.xdim = XBOUND;
	temp.ydim = YBOUND;

	// Calculate max entities per room
	int halfSum = (temp.xdim + temp.ydim) / 2;
	int cubicRoot = (int)pow(temp.xdim * temp.ydim, 1.0 / 3.0);
	int maxEntities = abs(halfSum - 1) < abs(cubicRoot - 1) ? halfSum : cubicRoot;

	// Allocate memory for entities
	temp.entities = malloc(sizeof(Entity) * maxEntities);

	for (int i = 0;i < maxEntities;i++) {

        
		temp.entities[i] = enemyGlossary[getRandomEnemyIndex()];
	}

	return temp;
}

Entity logStep(Entity e) {
	e.stepLog[e.currentPos] = e.location;

	e.currentPos = e.currentPos >= LOG_BUFFER ? 0 : e.currentPos + 1;

	return e;
}
