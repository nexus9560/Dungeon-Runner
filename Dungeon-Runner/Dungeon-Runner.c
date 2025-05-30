﻿
#include "DungeonTypes.h"
#include "LoadingDock.h"

#ifdef _WIN32
#define CLEAR_COMMAND "cls"
#include <conio.h>
#include <windows.h>
#elif __unix__ || __APPLE__
#define CLEAR_COMMAND "clear"
#include <termios.h>
#include <sys/ioctl.h>
#include <unistd.h>

#else
#define CLEAR_COMMAND "" // Define it to nothing if OS is not detected
#endif

Cell world[XBOUND][YBOUND];

Player you;

Entity* enemyGlossary;
Entity* enemiesOnFloor; // Took the Entities array out of Room, because Entities should be able to roam between rooms on the floor, plus it was making room-scaling difficult.

int enemyGlossarySize;

unsigned int roomCount;

Item* itemGlossary;

const Dun_Vec up	= { -1,  0 };
const Dun_Vec right = {  0,  1 };
const Dun_Vec down	= {  1,  0 };
const Dun_Vec left	= {  0, -1 };

Room* rooms;

unsigned int currRoomCount = 0;

void mapClearing();
void makeRoomSpace(Room r);
void roomRunner();
void clearScreen();
void changePosition();
void inspectElement(Dun_Coord pos);
void actOnYourOwn();
void exitAction(int ec);
void actionChecker();
void delay(int seconds);
void showPlayerInventory();
void cutPaths();
void drawMap();
void printMap();

char* printPlayerStatus(int brief);

int checkBounds( Dun_Coord newPos, Dun_Vec delta);
int checkOccupied(Dun_Coord newPos, Dun_Vec delta);
int checkOverlappingArea(Room room1, Room room2);
int* getConsoleWindow();
int getRandomEnemyIndex();
int isInRoom(Room r, Dun_Coord d);
int isInARoom(Dun_Coord d);
int getVectorDirection(Dun_Vec d);

Room* makeRooms();
Room getRoomByLocation(Dun_Coord d);
Room* getNearest2Rooms(Room r);

Dun_Coord getNearestSafeLocation(Dun_Coord d, int searchRadius);
Dun_Coord getRoomCenter(Room r);
Dun_Coord getRandomSpotOnWall(Room r, Dun_Vec d);

Dun_Vec getVector(Dun_Coord start, Dun_Coord end);
Dun_Vec getVectorToWallFromCenter(Room r, Dun_Vec v);

Entity logStep(Entity e);
Entity shiftEntity(Entity e, Dun_Vec delta);
Entity moveEntity(Entity e, Dun_Coord newLoc);


//int goUp();
//int goRight();
//int goDown();
//int goLeft();

void main() {

	clearScreen();
	loadEntities(0);
	loadItems(0);
	mapClearing();
	rooms = loadRooms(0);
	if (roomCount == 0) {
		rooms = makeRooms();
	}

	for(unsigned int i = 0; i < roomCount; i++) {
		makeRoomSpace(rooms[i]);
		if (DEBUG) {
			printf("Room %4d: [%4d,%4d] - [%4d,%4d]\n", i, rooms[i].startLocation.x, rooms[i].startLocation.y, rooms[i].xdim, rooms[i].ydim);
		}
	}
	saveRooms();
	cutPaths();

	printMap();


	if (loadPlayer() == 0) {
		you.base.currentPos = 0;
		printf("Error loading player position. Starting at default position.\n");
		you.base.location = getNearestSafeLocation(you.base.location, 1);
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
		if (isInARoom(you.base.location)) {
			if(DEBUG)
				printf("Player position is within bounds.\n");
		}
		else {
			if(DEBUG)
				printf("Player position is out of bounds, teleporting you to the middle of the nearest room.\n");
			you.base.location = getNearestSafeLocation(you.base.location, 1);
		}
	}
	//int* consoleDimensions = getConsoleWindow();
	//printf("Console dimensions: %d rows, %d columns\n", consoleDimensions[0], consoleDimensions[1]);
	//roomRunner();


}


void mapClearing() {
	for (int x = 0;x < XBOUND;x++) {
		for (int y = 0;y < YBOUND;y++) {
			world[x][y].locationID = (x * YBOUND) + y;
			world[x][y].passable = 0;
			world[x][y].occupied = 0;
			world[x][y].ref = '#';
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
	
	int* conDims = getConsoleWindow();
	int renderX = (int)((conDims[0] * 0.75) > XBOUND ? XBOUND : (conDims[0] * 0.55));
	int renderY = (int)((conDims[1] * 0.80) > YBOUND ? YBOUND : (conDims[1] * 0.80));
	char* map = (char*)malloc(renderX * renderY);
	if (map == NULL) {
		printf("Memory allocation failed\n");
		return;
	}
	/*
		
	if ((xoffset == 0 && yoffset == 0) || (xoffset == 0 && yoffset == (YBOUND - 1)) || (xoffset == (XBOUND - 1) && yoffset == 0) || (xoffset == (XBOUND - 1) && yoffset == (YBOUND - 1))) {
				map[x * renderY + y] = '+';
				continue;
			}
			else if ((xoffset == 0 || xoffset == XBOUND - 1) && !(yoffset < 0 || yoffset >= YBOUND)) {
				map[x * renderY + y] = '-';
				continue;
			}
			else if ((yoffset == 0 || yoffset == YBOUND - 1) && !(xoffset < 0 || xoffset >= XBOUND) ) {
				map[x * renderY + y] = '|';
				continue;
			}
	*/
		
		
	for (int x = 0; x < renderX;x++) {
		int xoffset = you.base.location.x - (renderX / 2) + x;
		for (int y = 0;y < renderY;y++) {
			int yoffset = you.base.location.y - (renderY / 2) + y;
			if ((renderX/2)==x && (renderY/2)==y) {
				map[x * renderY + y] = '@';
				continue;
			}
			else if (xoffset < 0 || xoffset >= XBOUND || yoffset < 0 || yoffset >= YBOUND) {
				map[x * renderY + y] = ' ';
				continue;
			}
			else if (world[xoffset][yoffset].occupied == 1) {
				map[x * renderY + y] = 'X';
				continue;
			}
			else {
				map[x * renderY + y] = world[xoffset][yoffset].ref;
				continue;
			}
		}
		printf("%.*s\n", renderY, &map[x * renderY]);
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
		printf("\n");
		printf("%s\n",printPlayerStatus(isBrief));
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
			if(DEBUG)
				printf("Key pressed: %c : %d\n", ch,ch);
			switch (ch) {
				case 'w':
				case 72: // Up arrow
					you.base = shiftEntity(you.base, up);
					break;
				case 'W':
					you.base = shiftEntity(shiftEntity(you.base, up), up);
					break;
				case 'a':
				case 75: // Left arrow
					you.base = shiftEntity(you.base, left);
					break;
				case 'A':
					you.base = shiftEntity(shiftEntity(you.base, left), left);
					break;
				case 'd':
				case 77: // Right arrow
					you.base = shiftEntity(you.base, right);
					break;
				case 'D':
					you.base = shiftEntity(shiftEntity(you.base, right), right);
					break;
				case 's':
				case 80: // Down arrow
					you.base = shiftEntity(you.base, down);
					break;
				case 'S':
					you.base = shiftEntity(shiftEntity(you.base, down), down);
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
	   if (DEBUG)
		   printf("Key pressed: %c : %d\n", ch, ch);
	   switch (ch) {
		   case 'w':
		   case 72: // Up arrow
			   you.base = shiftEntity(you.base, up);
			   break;
		   case 'W':
			   you.base = shiftEntity(shiftEntity(you.base, up), up);
			   break;
		   case 'a':
		   case 75: // Left arrow
			   you.base = shiftEntity(you.base, left);
			   break;
		   case 'A':
			   you.base = shiftEntity(shiftEntity(you.base, left), left);
			   break;
		   case 'd':
		   case 77: // Right arrow
			   you.base = shiftEntity(you.base, right);
			   break;
		   case 'D':
			   you.base = shiftEntity(shiftEntity(you.base, right), right);
			   break;
		   case 's':
		   case 80: // Down arrow
			   you.base = shiftEntity(you.base, down);
			   break;
		   case 'S':
			   you.base = shiftEntity(shiftEntity(you.base, down), down);
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
		   default:
			   break;
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

void exitAction(int ec) {
	if (DEBUG) {
		printMap();
		saveRooms();
	}
	clearScreen();
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

   return world[temp[0]][temp[1]].passable && (temp[0] > 0 && temp[0] < XBOUND) && (temp[1] > 0 && temp[1] < YBOUND) ;
}

int checkOverlappingArea(Room room1, Room room2) {
	Dun_Coord r1[5] = { {room1.startLocation.x, room1.startLocation.y}, // Top Left
									{room1.startLocation.x + room1.xdim - 1, room1.startLocation.y}, // Top Right
									{room1.startLocation.x, room1.startLocation.y + room1.ydim - 1}, // Bottom Left
									{room1.startLocation.x + room1.xdim - 1, room1.startLocation.y + room1.ydim - 1}, // Bottom Right
									getRoomCenter(room1) }; // Center of Room 1
	Dun_Coord r2[5] = { {room2.startLocation.x, room2.startLocation.y}, // Top Left
									{room2.startLocation.x + room2.xdim - 1, room2.startLocation.y}, // Top Right
									{room2.startLocation.x, room2.startLocation.y + room2.ydim - 1}, // Bottom Left
									{room2.startLocation.x + room2.xdim - 1, room2.startLocation.y + room2.ydim - 1}, // Bottom Right
									getRoomCenter(room2) }; // Center of Room 2
	

	
	//check and see if two rooms overlap, if they do, return 1 (true)
	for (int x = 0; x < 5; x++) {
		if (isInRoom(room1,r2[x]) || isInRoom(room2,r1[x])) {
			return 1;
		}
	}

	return 0;
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
	return inRangeInclusive(d.x, r.startLocation.x, r.startLocation.x + r.xdim - 1) && inRangeInclusive(d.y, r.startLocation.y, r.startLocation.y + r.ydim - 1);
}

int isInARoom(Dun_Coord d) {
	if (currRoomCount > 1) {
		for (unsigned int i = 0; i < roomCount; i++) {
			if (isInRoom(rooms[i], d)) {
				return 1; // In a room
			}
		}
	}
	return 0; // Not in a room
}

Dun_Coord getNearestSafeLocation(Dun_Coord d, int searchRadius) {
	// Ensure searchRadius is within map bounds
	if (searchRadius > XBOUND && searchRadius > YBOUND) {
		// Fallback: center of map
		return (Dun_Coord) { XBOUND / 2, YBOUND / 2 };
	}

	// Check 4 cardinal directions
	if (d.x + searchRadius < XBOUND && isInARoom((Dun_Coord) { d.x + searchRadius, d.y })) {
		return (Dun_Coord) { d.x + searchRadius, d.y };
	}
	if (d.x - searchRadius >= 0 && isInARoom((Dun_Coord) { d.x - searchRadius, d.y })) {
		return (Dun_Coord) { d.x - searchRadius, d.y };
	}
	if (d.y + searchRadius < YBOUND && isInARoom((Dun_Coord) { d.x, d.y + searchRadius })) {
		return (Dun_Coord) { d.x, d.y + searchRadius };
	}
	if (d.y - searchRadius >= 0 && isInARoom((Dun_Coord) { d.x, d.y - searchRadius })) {
		return (Dun_Coord) { d.x, d.y - searchRadius };
	}
	
	// If not found, increase search radius and recurse
	return getNearestSafeLocation(d, searchRadius + 1);
}

Room getRoomByLocation(Dun_Coord d) {
	Room temp = { {0,0},XBOUND,YBOUND };
	
	for (unsigned int i = 0; i < roomCount;i++) {
		if (isInRoom(rooms[i], d)) {
			return rooms[i];
		}
	}

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
	double experienceValue = (pow(2, you.base.level));
	if (brief) {
		ret = malloc(256);
		if (ret == NULL) {
			printf("Memory allocation failed\n");
			return NULL;
		}
		snprintf(ret, 256,
			"+--------------------------------------------------->\n"
			"| %s\n"
			"| HP : %4d / %4d\n"
			"| EXP: %4d / %4.0f\n"
			"| Location: [%4d,%4d]\n"
			"|\n"
			"V\n",
			you.base.name, you.base.curHealth, you.base.health, you.base.exp, experienceValue, you.base.location.y, you.base.location.x);
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
			"| Experience: %4d /%4.0f\n"
			"| Location: [%4d,%4d]\n"
			"| Evasion: %d\n"
			"| Level: %d\n"
			"|\n"
			"V\n",
			you.base.name, you.base.curHealth, you.base.health, you.base.atk,
			you.base.hit, you.base.def, you.base.exp, experienceValue, you.base.location.y, you.base.location.x, you.base.eva, you.base.level);
	}
	return ret;
}

Room* makeRooms() {
	roomCount = (unsigned int)(pow((XBOUND * YBOUND), (1.0 / 3.0)));
	Room* temp = malloc(roomCount * sizeof(Room));
	if (temp == NULL) {
		printf("Memory allocation failed\n");
		return NULL;
	}
    // srand returns void, so call it separately, then use rand() for random numbers
    srand((unsigned int)time(NULL));

	for (unsigned int i = 0; i < roomCount; i++) {
		unsigned int randX = (unsigned int)(rand() % (unsigned int)(XBOUND * 0.08));
		unsigned int randY = (unsigned int)(rand() % (unsigned int)(YBOUND * 0.06));
		temp[i].xdim = (randX < 5 ? 5 : randX);
		temp[i].ydim = (randY < 5 ? 5 : randY);
		temp[i].startLocation.x = (unsigned int)(rand() % (XBOUND - temp[i].xdim));
		temp[i].startLocation.y = (unsigned int)(rand() % (YBOUND - temp[i].ydim));
		if(temp[i].startLocation.x == 0)
			temp[i].startLocation.x = 2;
		if (temp[i].startLocation.y == 0)
			temp[i].startLocation.y = 2; 
		if(temp[i].startLocation.x + temp[i].xdim >= XBOUND)
			temp[i].startLocation.x = XBOUND - temp[i].xdim - 2; // Adjust xdim if it exceeds bounds
		if (temp[i].startLocation.y + temp[i].ydim >= YBOUND)
			temp[i].startLocation.y = YBOUND - temp[i].ydim - 2; // Adjust ydim if it exceeds bounds
		for (unsigned int j = 0; j < i; j++) {
			if (checkOverlappingArea(temp[i], temp[j])) {
				if (DEBUG) {
					printf("Room %d overlaps with Room %d, regenerating...\n", i, j);
				}
				i--; // Decrement i to regenerate this room
				break; // Break out of the inner loop to regenerate
			}
		}
		
	}
	for(unsigned int x=0;x<roomCount;x++)
		temp[x].roomID = x;
	return temp;
}

void makeRoomSpace(Room r) {
	for (unsigned int x = (r.startLocation.x<0?0:r.startLocation.x); x < r.startLocation.x + r.xdim; x++) {
		for (unsigned int y = (r.startLocation.y<0?0:r.startLocation.y); y < r.startLocation.y + r.ydim; y++) {
			if(x == r.startLocation.x && y == r.startLocation.y) {
				world[x][y].ref = '+';
				world[x][y].passable = 0;
			}
			else if (x == r.startLocation.x + r.xdim - 1 && y == r.startLocation.y + r.ydim - 1) {
				world[x][y].ref = '+';
				world[x][y].passable = 0;
			}
			else if (x == r.startLocation.x && y == r.startLocation.y + r.ydim - 1) {
				world[x][y].ref = '+';
				world[x][y].passable = 0;
			}
			else if (x == r.startLocation.x + r.xdim - 1 && y == r.startLocation.y) {
				world[x][y].ref = '+';
				world[x][y].passable = 0;
			}
			else if (x == r.startLocation.x || x == r.startLocation.x + r.xdim - 1) {
				world[x][y].ref = '-';
				world[x][y].passable = 0;
			}
			else if (y == r.startLocation.y || y == r.startLocation.y + r.ydim - 1) {
				world[x][y].ref = '|';
				world[x][y].passable = 0;
			}
			else {
				world[x][y].ref = ' ';
				world[x][y].passable = 1;
			}
		}
	}

	for(unsigned int x = r.startLocation.x; x < r.startLocation.x + r.xdim; x++) {
		for (unsigned int y = r.startLocation.y; y < r.startLocation.y + r.ydim; y++) {
			if (world[x][y].passable) {
				if(inRange(x + up.dx, 0, XBOUND))
					world[x][y].admat[0] = world[x + up.dx		][y + up.dy		].passable ? 1 : 0; // Up
				else
					world[x][y].admat[0] = 0; // Up
				if (inRange(y + right.dy, 0, YBOUND))
					world[x][y].admat[1] = world[x + right.dx	][y + right.dy	].passable ? 1 : 0; // Right
				else
					world[x][y].admat[1] = 0; // Right
				if (inRange(x + down.dx, 0, XBOUND))
					world[x][y].admat[2] = world[x + down.dx	][y + down.dy	].passable ? 1 : 0; // Down
				else
					world[x][y].admat[2] = 0; // Down
				if (inRange(y + left.dy, 0, YBOUND))
					world[x][y].admat[3] = world[x + left.dx	][y + left.dy	].passable ? 1 : 0; // Left
				else
					world[x][y].admat[3] = 0; // Left
				if (DEBUG && world[x][y].passable) {
					//world[x][y].ref = (char)(world[x][y].admat[0] + world[x][y].admat[1] + world[x][y].admat[2] + world[x][y].admat[3] + 48);
				}
			}else{
				switch(world[x][y].ref) {
					case '+':break;
					case '-':
						world[x][y].admat[0] = world[(x + up.dx >= 0 ? x + up.dx : 0)][y + up.dy].passable ? 1 : 0; // Up
						world[x][y].admat[2] = world[(x + down.dx < XBOUND ? x + down.dx : XBOUND - 1)][y + down.dy].passable ? 1 : 0; // Down
						break;
					case '|':
						world[x][y].admat[1] = world[x + right.dx][(y + right.dy < YBOUND ? y + right.dy : YBOUND - 1)].passable ? 1 : 0; // Right
						world[x][y].admat[3] = world[x + left.dx][(y + left.dy >= 0 ? y + left.dy : 0)].passable ? 1 : 0; // Left
						break;
					default:break;
				}
			}
		}
	}
	
}

Dun_Coord getRoomCenter(Room r) {
	Dun_Coord center;
	center.x = r.startLocation.x + (r.xdim / 2);
	center.y = r.startLocation.y + (r.ydim / 2);
	return center;
}

Entity logStep(Entity e) {
	e.stepLog[e.currentPos] = e.location;

	e.currentPos = e.currentPos >= LOG_BUFFER ? 0 : e.currentPos + 1;

	return e;
}

Dun_Vec getVector(Dun_Coord start, Dun_Coord end) {
	Dun_Vec delta;
	delta.dx = end.x - start.x;
	delta.dy = end.y - start.y;
	return delta;
}

Entity moveEntity(Entity e, Dun_Coord newLoc) {
	Dun_Vec delta = getVector(e.location, newLoc);
	if (checkOccupied(e.location, delta)) {
		if (DEBUG)
			printf("Error: This space is already occupied.\n");
		return logStep(e);
	}
	else if (checkBounds(e.location, delta)) {
		world[e.location.x][e.location.y].occupied = 0;
		e.location.x += delta.dx;
		e.location.y += delta.dy;
		world[e.location.x][e.location.y].occupied = 1;
	}
	else {
		if (DEBUG)
			printf("Error: %s cannot move out of bounds.\n", e.name);
	}
	return logStep(e);
}

void printMap() {
    FILE* file = fopen("map.dat", "w");
    if (file == NULL) {
        printf("Error: Could not open map.dat for writing.\n");
        return;
    }

	/*
	if ((x == 0 && y == 0) || (x == 0 && y == (YBOUND - 1)) ||
                (x == (XBOUND - 1) && y == 0) || (x == (XBOUND - 1) && y == (YBOUND - 1))) {
                mapChar = '+';
            } else if (x == 0 || x == XBOUND - 1) {
                mapChar = '-';
            } else if (y == 0 || y == YBOUND - 1) {
                mapChar = '|';
            } else 
	*/

    for (int x = 0; x < XBOUND; x++) {
        for (int y = 0; y < YBOUND; y++) {
            char mapChar;
            if (world[x][y].passable == 0) {
                mapChar = '#';
            } else if (world[x][y].occupied == 1) {
                mapChar = 'X';
            } else {
                mapChar = ' ';
            }
            fputc(mapChar, file);
        }
        fputc('\n', file);
    }
    fclose(file);
    printf("Full map written to map.dat\n");
}


Room* getNearest2Rooms(Room r) {
	Room* nearestRooms = malloc(2 * sizeof(Room));
	if (nearestRooms == NULL) {
		printf("Memory allocation failed\n");
		return NULL;
	}

	int* dismap = malloc(roomCount * sizeof(int));
	if (dismap == NULL) {
		printf("Memory allocation failed\n");
		free(nearestRooms);
		return NULL;
	}

	int nearestIndices[2] = { -1, -1 }; // Indices of the two nearest rooms

	int grandChecks = 0;
	for (unsigned int i = 0;i < roomCount; i++) {
		if (r.roomID == rooms[i].roomID) {
			dismap[i] = 0; // Distance to itself is zero
			continue; // Skip the room that matches the passed room
		}
		else {
			Dun_Vec vec = getVector(getRoomCenter(r), getRoomCenter(rooms[i]));
			dismap[i] = abs(vec.dx) + abs(vec.dy);
		}

	}
	do {
		for (unsigned int i = 0; i < roomCount; i++) {
			if(dismap[i] == 0) {
				continue; // Skip the room that matches the passed room
			}
			if(nearestIndices[0] == -1 || dismap[i] < dismap[nearestIndices[0]]) {
				nearestIndices[1] = nearestIndices[0]; // Shift the previous nearest to second
				nearestIndices[0] = i; // Update the nearest index
				if(DEBUG)
					printf("Room %d is now the nearest room with distance %d\n", i, dismap[i]);
			} else if (nearestIndices[1] == -1 || dismap[i] < dismap[nearestIndices[1]]) {
				nearestIndices[1] = i; // Update the second nearest index
			}
		}
		grandChecks++;
	} while (grandChecks < 2);

	return nearestRooms;
}

// Positions: 0 = up, 1 = right, 2 = down, 3 = left

int getVectorDirection(Dun_Vec d) {
	if (d.dx == 0 && d.dy == 0) {
		return -1; // Invalid direction
	}
	else if (abs(d.dx) > abs(d.dy)) {
		return (d.dx > 0) ? 2 : 0; // Up or Down
	}
	else {
		return (d.dy > 0) ? 3 : 1; // Left or Right
	}
}

// Presumes Vector d is a vector that points to another room's center from r's center, and passes from the center of room r to the center of the other room.
// This finds the vector from the center of room r to the wall of room r in the direction of d.

Dun_Vec getVectorToWallFromCenter(Room r, Dun_Vec d) {
	Dun_Vec ret = { d.dx, d.dy };
	
	if(d.dx == 0 && d.dy == 0) {
		printf("Error: Vector cannot be zero.\n");
		return (Dun_Vec) { 0, 0 }; // Return a zero vector
	}
	else if (abs(d.dx) > abs(d.dy)) {
		ret.dy = 0;
		ret.dx = (d.dx > 0) ? (r.xdim - 1) / 2 : -(signed int)((r.xdim - 1) / 2);
	}
	else {
		ret.dx = 0;
		ret.dy = (d.dy > 0) ? (r.ydim - 1) / 2 : -(signed int)((r.ydim - 1) / 2);
	}

	if (!matchSign(ret.dx, d.dx)) {
		ret.dx = -ret.dx; // Ensure the direction matches
	}
	if (!matchSign(ret.dy, d.dy)) {
		ret.dy = -ret.dy; // Ensure the direction matches
	}

	return ret;
}

Dun_Coord getRandomSpotOnWall(Room r, Dun_Vec d) {
	Dun_Coord wallloc = { 0, 0 };

	if (d.dx == 0 && d.dy == 0) {
		int wallSide = rand() % 4; // Randomly choose a wall side
		switch (wallSide) {
		case 0: // Top Wall
			wallloc.x = r.startLocation.x;
			wallloc.y = inRangeExclusive(r.startLocation.y + (rand() % r.ydim), r.startLocation.y, r.startLocation.y + r.ydim - 1) ? r.startLocation.y + (rand() % r.ydim) : r.startLocation.y + (r.ydim / 2);
			break;
		case 1: // Right Wall
			wallloc.x = inRangeExclusive(r.startLocation.x + (rand() % r.xdim), r.startLocation.x, r.startLocation.x + r.xdim - 1) ? r.startLocation.x + (r.xdim - 1) : r.startLocation.x + (r.xdim / 2);
			wallloc.y = r.startLocation.y + r.ydim;
			break;
		case 2: // Bottom Wall
			wallloc.x = r.startLocation.x + r.xdim;
			wallloc.y = inRangeExclusive(r.startLocation.y + (rand() % r.ydim), r.startLocation.y, r.startLocation.y + r.ydim - 1) ? r.startLocation.y + (rand() % r.ydim) : r.startLocation.y + (r.ydim / 2);
			break;
		case 3: // Left Wall
			wallloc.x = inRangeExclusive(r.startLocation.x + (rand() % r.xdim), r.startLocation.x, r.startLocation.x + r.xdim - 1) ? r.startLocation.x : r.startLocation.x + (r.xdim / 2);
			wallloc.y = r.startLocation.y;
			break;
		default:
			printf("Error: Invalid wall side chosen.\n");
			return (Dun_Coord) { -1, -1 }; // Return an invalid coordinate
		}
	}
	else {
		Dun_Vec wallVec = getVectorToWallFromCenter(r, d);
		if(wallVec.dx == 0 && wallVec.dy == 0) {
			printf("Error: Vector cannot be zero.\n");
			return (Dun_Coord) { -1, -1 }; // Return an invalid coordinate
		}
		if(wallVec.dx != 0) {
			int randSpot = rand() % r.ydim; // Randomly choose a spot on the wall
			wallloc.x = (wallVec.dx > 0) ? r.startLocation.x + r.xdim - 1 : r.startLocation.x;
			wallloc.y = inRangeExclusive(r.startLocation.y + randSpot, r.startLocation.y, r.startLocation.y + r.ydim - 1) ? r.startLocation.y + randSpot : r.startLocation.y + (r.ydim / 2);
		}
		else if(wallVec.dy != 0) {
			int randSpot = rand() % r.xdim; // Randomly choose a spot on the wall
			wallloc.y = (wallVec.dy > 0) ? r.startLocation.y + r.ydim - 1 : r.startLocation.y;
			wallloc.x = inRangeExclusive(r.startLocation.x + randSpot, r.startLocation.x, r.startLocation.x + r.xdim - 1) ? r.startLocation.x + randSpot : r.startLocation.x + (r.xdim / 2);
		}
		else {
			printf("Error: Invalid vector.\n");
			return (Dun_Coord) { -1, -1 }; // Return an invalid coordinate
		}
	}

	return wallloc;
}

void cutPaths() {
	int* has2Paths = malloc(roomCount * sizeof(int));
	int* visited = malloc(roomCount * sizeof(int));
	if (has2Paths == NULL) {
		printf("Memory allocation failed\n");
		return;
	}
	if (visited == NULL) {
		printf("Memory allocation failed\n");
		free(has2Paths);
		return;
	}

	for (unsigned int i = 0; i < roomCount; i++) {
		has2Paths[i] = 0; // Initialize all rooms to have no paths
		visited[i] = 0; // Initialize all rooms as unvisited
	}

	unsigned int runner = 0;

	do {
		Room* r = getNearest2Rooms(rooms[runner]);
		if (r == NULL) {
			printf("Error: Could not get nearest rooms.\n");
			free(has2Paths);
			free(visited);
			return;
		}
		if(visited[runner] < 3) {
			int room1ID = r[0].roomID;
			int room2ID = r[1].roomID;
			Dun_Coord wallLoc1 = getRandomSpotOnWall(rooms[runner], getVector(getRoomCenter(rooms[runner]), getRoomCenter(r[0]) ) );
			Dun_Coord farWallLoc1 = getRandomSpotOnWall(rooms[room1ID], getVector(getRoomCenter(r[0]), getRoomCenter(rooms[runner])));
			Dun_Coord wallLoc2 = getRandomSpotOnWall(rooms[runner], getVector(getRoomCenter(rooms[runner]), getRoomCenter(r[1])));
			Dun_Coord farWallLoc2 = getRandomSpotOnWall(rooms[room2ID], getVector(getRoomCenter(r[1]), getRoomCenter(rooms[runner])));
			Dun_Coord walker = { wallLoc1.x, wallLoc1.y }; // Start at the first wall location

			Dun_Vec vec1 = getVector(wallLoc1, farWallLoc1);
			Dun_Vec vec2 = getVector(wallLoc2, farWallLoc2);

			unsigned int trav1 = abs(vec1.dx) + abs(vec1.dy);
			unsigned int trav2 = abs(vec2.dx) + abs(vec2.dy);
			

			visited[runner]++; // Mark the current room as visited
			visited[r[0].roomID]++; // Mark the first nearest room as visited
			visited[r[1].roomID]++; // Mark the second nearest room as visited

			// Direction: 0 = up, 1 = right, 2 = down, 3 = left

			while (trav1 > 0 && walker.x != farWallLoc1.x && walker.y != farWallLoc1.y) {
				world[walker.x][walker.y].ref = ' '; // Mark the path with a dot
				world[walker.x][walker.y].passable = 1; // Make the path passable
				switch (getVectorDirection(vec1)) {
				case 0: walker.x += up.dx;
						walker.y += up.dy;
						vec1.dx -= up.dx;
						vec1.dy -= up.dy;
						vec1 = getVector(walker, farWallLoc1); // Update the vector
						break;
				case 1: walker.x += right.dx;
						walker.y += right.dy;
						vec1.dx -= right.dx;
						vec1.dy -= right.dy;
						vec1 = getVector(walker, farWallLoc1); // Update the vector
						break;
				case 2: walker.x += down.dx;
						walker.y += down.dy;
						vec1.dx -= down.dx;
						vec1.dy -= down.dy;
						vec1 = getVector(walker, farWallLoc1); // Update the vector
						break;
				case 3: walker.x += left.dx;
						walker.y += left.dy;
						vec1.dx -= left.dx;
						vec1.dy -= left.dy;
						vec1 = getVector(walker, farWallLoc1); // Update the vector
						break;
				default:
					printf("Error: Invalid vector direction.\n");
					free(has2Paths);
					free(visited);
					free(r);
					return;

				}
				trav1--;
			}
			walker = (Dun_Coord){ wallLoc2.x, wallLoc2.y }; // Reset walker to the second wall location
			while (trav2 > 0 && walker.x != farWallLoc2.x && walker.y != farWallLoc2.y) {
				world[walker.x][walker.y].ref = ' '; // Mark the path with a dot
				world[walker.x][walker.y].passable = 1; // Make the path passable
				switch (getVectorDirection(vec2)) {
				case 0: walker.x += up.dx;
						walker.y += up.dy;
						vec2.dx -= up.dx;
						vec2.dy -= up.dy;
						vec2 = getVector(walker, farWallLoc2); // Update the vector
						break;
				case 1: walker.x += right.dx;
						walker.y += right.dy;
						vec2.dx -= right.dx;
						vec2.dy -= right.dy;
						vec2 = getVector(walker, farWallLoc2); // Update the vector
						break;
				case 2: walker.x += down.dx;
						walker.y += down.dy;
						vec2.dx -= down.dx;
						vec2.dy -= down.dy;
						vec2 = getVector(walker, farWallLoc2); // Update the vector
						break;
				case 3: walker.x += left.dx;
						walker.y += left.dy;
						vec2.dx -= left.dx;
						vec2.dy -= left.dy;
						vec2 = getVector(walker, farWallLoc2); // Update the vector
						break;
				default:
					printf("Error: Invalid vector direction.\n");
					free(has2Paths);
					free(visited);
					free(r);
					return;
				}
				trav2--;
			}
			
		}
		runner++;
	} while (runner < roomCount);

}