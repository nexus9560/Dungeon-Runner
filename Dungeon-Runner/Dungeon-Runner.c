
#include "DungeonTypes.h"
#include "LoadingDock.h"
#include "PathFinder.h"
#include "Player.h"
#include "pathlib.h"

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

Room* rooms;

unsigned int currRoomCount = 0;

//const Dun_Vec up = { -1, 0 };
//const Dun_Vec right =	{  0, 1 };
//const Dun_Vec down =	{  1, 0 };
//const Dun_Vec left =	{  0,-1 };
//const Dun_Vec directions[4] = { { -1, 0 }, { 0, 1 }, { 1, 0 }, { 0, -1 } };

void mapClearing();
void makeRoomSpace(Room r);
void roomRunner();
void clearScreen();
void changePosition();
void inspectElement(Dun_Coord pos);
void actOnYourOwn();
void exitAction(int ec);
void delay(int seconds);
void showPlayerInventory();
void cutPaths();
void drawMap();
void printMap();
void printAdMap();
void popAdMat(Dun_Coord d);
void updateWorldAdMat();

char* printPlayerStatus(int brief);

int checkBounds( Dun_Coord newPos, Dun_Vec delta);
int checkOccupied(Dun_Coord newPos, Dun_Vec delta);
int checkOverlappingArea(Room room1, Room room2);
int actionChecker();
int enemyTurn();
int* getConsoleWindow();
int getRandomEnemyIndex();
int isInRoom(Room r, Dun_Coord d);
int getVectorDirection(Dun_Vec d);
int closerToZero(int value); // Brings passed value closer to zero.
int closeToZero(int a, int b); // Returns a if it is closer to zero than b, otherwise returns b.
int isAdjacent(Dun_Coord a, Dun_Coord b); // Returns 1 if a and b are adjacent, otherwise returns 0.
int compareVectors(Dun_Vec a, Dun_Vec b); // Returns 1 if vectors are equal, otherwise returns 0.
int isSafeSpot(Dun_Coord d);
int isThereAPath(Dun_Coord start, Dun_Coord end);

Room* makeRooms();
Room getRoomByLocation(Dun_Coord d);
Room* getNearest2Rooms(Room r);

Dun_Coord getNearestSafeLocation(Dun_Coord d);
Dun_Coord getRoomCenter(Room r);
Dun_Coord getSpotOnWall(Room r, Dun_Vec d);
Dun_Coord copyCoord(Dun_Coord d);
Dun_Coord* getCellsOnVector(Dun_Coord start, Dun_Coord end);

Dun_Vec getVectorToWallFromCenter(Room r, Dun_Vec v);

Entity logStep(Entity e);
Entity shiftEntity(Entity e, Dun_Vec delta);
Entity moveEntity(Entity e, Dun_Coord newLoc);


//int goUp();
//int goRight();
//int goDown();
//int goLeft();

int main() {

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
	updateWorldAdMat();

	printAdMap();
	printMap();


	if (1 || loadPlayer() == 0) {
		you.base.currentPos = 0;
		printf("Error loading player position. Starting at default position.\n");
		you.base.location = getNearestSafeLocation(you.base.location);
		// Name
		printf("Please enter your name:\n");
		if (scanf("%31s", you.base.name) != 1) { // Limit input to 31 characters to prevent BUFFER overflow
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
		if (isInARoom(you.base.location) || isSafeSpot(you.base.location)) {
			if(DEBUG)
				printf("Player position is within bounds.\n");
		}
		else {
			if(DEBUG)
				printf("Player position is out of bounds, teleporting you to the middle of the nearest room.\n");
			you.base.location = getNearestSafeLocation(you.base.location);
		}
	}
	int* consoleDimensions = getConsoleWindow();
	printf("Console dimensions: %d rows, %d columns\n", consoleDimensions[0], consoleDimensions[1]);
	roomRunner();

	return 0;
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
	unsigned int isPlayerTurn = 1, hasSomethingchanged = 0;
	do {

		drawMap();
		printf("\n");
		printf("%s\n",printPlayerStatus(isBrief));
		if (isPlayerTurn) {
			if (actionChecker()) {
				isPlayerTurn = 0;
				hasSomethingchanged = 1; // Player did something, so we need to update the world.
			}
		}
		else {
			if (enemyTurn()) {
				isPlayerTurn = 1;
				hasSomethingchanged = 1; // Enemy did something, so we need to update the world.
			}
		}
		//delay(1);
		clearScreen();

	} while (1);
}

int enemyTurn() {
	return 1;
}


int actionChecker() {
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
			   return 1;
		   case 2: inspectElement(you.base.location);
			   return 1;
		   case 3: actOnYourOwn();
			   return 1;
		   case 4: savePlayer();
			   return 1;
		   case 5: loadPlayer();
			   return 1;
		   case 6: if (DEBUG) {
			   loadEntities(0);
			   return 1;
		   } else {
			   return 0;
		   }
		   default:
			   printf("Could you try that again?\n");
			   return 0;
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
					return 1;
				case 'W':
					you.base = shiftEntity(shiftEntity(you.base, up), up);
					return 1;
				case 'a':
				case 75: // Left arrow
					you.base = shiftEntity(you.base, left);
					return 1;
				case 'A':
					you.base = shiftEntity(shiftEntity(you.base, left), left);
					return 1;
				case 'd':
				case 77: // Right arrow
					you.base = shiftEntity(you.base, right);
					return 1;
				case 'D':
					you.base = shiftEntity(shiftEntity(you.base, right), right);
					return 1;
				case 's':
				case 80: // Down arrow
					you.base = shiftEntity(you.base, down);
					return 1;
				case 'S':
					you.base = shiftEntity(shiftEntity(you.base, down), down);
					return 1;
				case 'i':
					printf("Inventory not implemented yet.\n");
					return 1;
				case 'e':
					printf("Interact not implemented yet.\n");
					return 1;
				case 'f':
					printf("Inspect not implemented yet.\n");
					return 1;
				case ' ':
					printf("Attack not implemented yet.\n");
					return 1;

				case 'q':
				case 81: // Q key
					exitAction(0);
				default:return 0;
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
			   return 1;
		   case 'W':
			   you.base = shiftEntity(shiftEntity(you.base, up), up);
			   return 1;
		   case 'a':
		   case 75: // Left arrow
			   you.base = shiftEntity(you.base, left);
			   return 1;
		   case 'A':
			   you.base = shiftEntity(shiftEntity(you.base, left), left);
			   return 1;
		   case 'd':
		   case 77: // Right arrow
			   you.base = shiftEntity(you.base, right);
			   return 1;
		   case 'D':
			   you.base = shiftEntity(shiftEntity(you.base, right), right);
			   return 1;
		   case 's':
		   case 80: // Down arrow
			   you.base = shiftEntity(you.base, down);
			   return 1;
		   case 'S':
			   you.base = shiftEntity(shiftEntity(you.base, down), down);
			   return 1;
		   case 'i':
			   printf("Inventory not implemented yet.\n");
			   return 1;
		   case 'e':
			   printf("Interact not implemented yet.\n");
			   return 1;
		   case 'f':
			   printf("Inspect not implemented yet.\n");
			   return 1;
		   case ' ':
			   printf("Attack not implemented yet.\n");
			   return 1;

		   case 'q':
		   case 81: // Q key
			   exitAction(0);
		   default:
			   return 0;
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
		printAdMap();
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

int isAdjacent(Dun_Coord a, Dun_Coord b) {
	return (abs((signed int)(a.x) - (signed int)(b.x)) == 1 && a.y == b.y) || (abs((signed int)(a.y) - (signed int)(b.y)) == 1 && a.x == b.x);
}

int compareVectors(Dun_Vec a, Dun_Vec b) {
	return (a.dx == b.dx && a.dy == b.dy);
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


	Room temp1 = (Room){ {room1.startLocation.x - BUFFER,room1.startLocation.y - BUFFER}, room1.xdim - 1 + BUFFER, room1.ydim - 1 + BUFFER };
	Room temp2 = (Room){ {room2.startLocation.x - BUFFER,room2.startLocation.y - BUFFER}, room2.xdim - 1 + BUFFER, room2.ydim - 1 + BUFFER };

	Dun_Coord r1[5] = { {room1.startLocation.x - BUFFER, room1.startLocation.y - BUFFER}, // Top Left
						{room1.startLocation.x + room1.xdim - 1 + BUFFER, room1.startLocation.y - BUFFER}, // Top Right
						{room1.startLocation.x - BUFFER, room1.startLocation.y + room1.ydim - 1 + BUFFER}, // Bottom Left
						{room1.startLocation.x + room1.xdim - 1 + BUFFER, room1.startLocation.y + room1.ydim - 1 + BUFFER}, // Bottom Right
							getRoomCenter(room1) }; // Center of Room 1
	Dun_Coord r2[5] = { {room2.startLocation.x - BUFFER, room2.startLocation.y - BUFFER}, // Top Left
						{room2.startLocation.x + room2.xdim - 1 + BUFFER, room2.startLocation.y - BUFFER}, // Top Right
						{room2.startLocation.x - BUFFER, room2.startLocation.y + room2.ydim - 1 + BUFFER}, // Bottom Left
						{room2.startLocation.x + room2.xdim - 1 + BUFFER, room2.startLocation.y + room2.ydim - 1 + BUFFER}, // Bottom Right
							getRoomCenter(room2) }; // Center of Room 2



	//check and see if two rooms overlap, if they do, return 1 (true)
	for (int x = 0; x < 5; x++) {
		if (isInRoom(temp1,r2[x]) || isInRoom(temp2,r1[x])) {
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

int isSafeSpot(Dun_Coord d) {
	if (world[d.x][d.y].passable == 1 && world[d.x][d.y].occupied == 0 && inRangeExclusive((signed int)d.x, (signed int)0+BUFFER, (signed int)XBOUND-BUFFER) && inRangeExclusive((signed int)d.y, (signed int)0+BUFFER, (signed int)YBOUND-BUFFER)) {
		return 1; // Safe spot
	}
	return 0; // Not a safe spot
}

int isInRoom(Room r, Dun_Coord d) {
	if (d.x < r.startLocation.x-(BUFFER/2) || d.x >= r.startLocation.x + r.xdim+ (BUFFER / 2) || d.y < r.startLocation.y- (BUFFER / 2) || d.y >= r.startLocation.y + r.ydim+ (BUFFER / 2)) {
		return 0; // Not in the room
	}
	return 1; // In the room
	//return inRangeExclusive(d.x, r.startLocation.x, r.startLocation.x + r.xdim - 1) && inRangeExclusive(d.y, r.startLocation.y, r.startLocation.y + r.ydim - 1);
}

int isInARoom(Dun_Coord d) {
	if (roomCount > 1) {
		for (unsigned int i = 0; i < roomCount; i++) {
			if (isInRoom(rooms[i], d)) {
				return 1; // In a room
			}
		}
	}
	return 0; // Not in a room
}

Dun_Coord getNearestSafeLocation(Dun_Coord d) {
	Dun_Coord safeLocation = { XBOUND/2, YBOUND/2 };

	if (isInARoom(d))
		return d;

	if(roomCount == 0) {
		printf("Nowhere is safe, good luck.\n");
		return safeLocation; // Return default value if no rooms are available
	}

	int* roomDistances = malloc(roomCount * sizeof(int));
	if (roomDistances == NULL) {
		printf("Memory allocation failed\n");
		return safeLocation; // Return default value if memory allocation fails
	}

	for (unsigned int i = 0; i < roomCount; i++) {
		Dun_Vec vectorToCenter = getVector(d, getRoomCenter(rooms[i]));
		roomDistances[i] = abs(vectorToCenter.dx) + abs(vectorToCenter.dy); // Manhattan distance to room center
	}

	Dun_Coord closest = { 0,0 };

	int dCheck = 0, closePos = 0, close = INT_MAX;


	while (dCheck < 2) {
		for (unsigned int i = 0; i < roomCount; i++) {
			if (roomDistances[i] < close) {
				close = roomDistances[i];
				closePos = i;
			}
		}
		dCheck++;
	}

	safeLocation = getRoomCenter(rooms[closePos]); // Get the center of the closest room

	return safeLocation; // Default return value, should be replaced with actual logic
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
	return rand() % masterEntityList.size; // Generate a random number between 0 and MAX_ENTITIES - 1
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
	//roomCount = 3;
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
		temp[i].xdim = (randX < 9 ? 9 : randX);
		temp[i].ydim = (randY < 9 ? 9 : randY);
		temp[i].startLocation.x = (unsigned int)(rand() % (XBOUND - temp[i].xdim));
		temp[i].startLocation.y = (unsigned int)(rand() % (YBOUND - temp[i].ydim));
		if(temp[i].startLocation.x == 0)
			temp[i].startLocation.x = BUFFER + 1;
		if (temp[i].startLocation.y == 0)
			temp[i].startLocation.y = BUFFER + 1;
		if (temp[i].startLocation.x + temp[i].xdim >= XBOUND)
			temp[i].startLocation.x = XBOUND - temp[i].xdim - (2 * BUFFER); // Adjust xdim if it exceeds bounds
		if (temp[i].startLocation.y + temp[i].ydim >= YBOUND)
			temp[i].startLocation.y = YBOUND - temp[i].ydim - (2 * BUFFER); // Adjust ydim if it exceeds bounds
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
    char map_path[1024];
    path__join("debug-data", "map.dat", map_path);
	ensure_directory("debug-data");
    FILE* file = fopen(map_path, "w");
    if (file == NULL) {
        printf("Error: Could not open %s for writing.\n", map_path);
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
			char mapChar = world[x][y].ref;
            fputc(mapChar, file);
        }
        fputc('\n', file);
    }
    fclose(file);
    printf("Full map written to %s\n", map_path);
}

void printAdMap() {
    char map_path[1024];
    path__join("debug-data", "admap.dat", map_path);
	ensure_directory("debug-data");
    FILE* file = fopen(map_path, "w");
	if (file == NULL) {
		printf("Error: Could not open %s for writing.\n", map_path);
		return;
	}
	for (int x = 0; x < XBOUND; x++) {
		for (int y = 0; y < YBOUND; y++) {
			int sum = 0;
			for (int i = 0; i < 4; i++) {
				sum += world[x][y].admat[i];
			}

			char mapChar = (sum == 0 ? ' ' : sum + 48);

			fputc(mapChar, file);
		}
		fputc('\n', file);
	}
	fclose(file);
	printf("Full adjacency map written to %s\n", map_path);
}


Room* getNearest2Rooms(Room r) {
	Room* nearestRooms = malloc(2 * sizeof(Room));
	int* nearestRoomIndices = malloc(roomCount * sizeof(int));
	if (nearestRooms == NULL || nearestRoomIndices == NULL) {
		printf("Memory allocation failed\n");
		return NULL;
	}
	for(unsigned int i = 0; i < roomCount; i++) {
		nearestRoomIndices[i] = 0; // Initialize indices
	}
	for (unsigned int i = 0; i < roomCount; i++) {
		if(i == r.roomID-1) {
			if(DEBUG)
				printf("Skipping room %d as it is the current room.\n", i);
			nearestRoomIndices[i] = 0; // Mark as skipped
			continue; // Skip the current room
		}
		Dun_Vec d = getVector(getRoomCenter(r), getRoomCenter(rooms[i]));
		nearestRoomIndices[i] = abs(d.dx) + abs(d.dy); // Calculate the distance to the room
		if(DEBUG) {
			printf("Distance from room %d to room %d: %d\n", r.roomID, i, nearestRoomIndices[i]);
		}
	}

	int firstNearestIndex = -1, secondNearestIndex = -1, doubleCheck = 0;
	while (doubleCheck < 2) {
		int minDis = INT_MAX;
		for (unsigned int i = 0;i < roomCount;i++) {
			if(nearestRoomIndices[i] < minDis && nearestRoomIndices[i] != 0) {
				if (firstNearestIndex != -1) {
					secondNearestIndex = firstNearestIndex;
					firstNearestIndex = i;
				}else {
					firstNearestIndex = i;
				}
				minDis = nearestRoomIndices[i];

			}
		}
		doubleCheck++;
	}
	nearestRooms[0].roomID = firstNearestIndex + 1; // +1 to match room IDs
	nearestRooms[0].startLocation.x = rooms[firstNearestIndex].startLocation.x;
	nearestRooms[0].startLocation.y = rooms[firstNearestIndex].startLocation.y;
	nearestRooms[0].xdim = rooms[firstNearestIndex].xdim;
	nearestRooms[0].ydim = rooms[firstNearestIndex].ydim;
	nearestRooms[1].roomID = secondNearestIndex + 1; // +1 to match room IDs
	nearestRooms[1].startLocation.x = rooms[secondNearestIndex].startLocation.x;
	nearestRooms[1].startLocation.y = rooms[secondNearestIndex].startLocation.y;
	nearestRooms[1].xdim = rooms[secondNearestIndex].xdim;
	nearestRooms[1].ydim = rooms[secondNearestIndex].ydim;

	if(DEBUG) {
		printf("Nearest room 1: %d at [%d,%d] with a distance of %d\n", nearestRooms[0].roomID, nearestRooms[0].startLocation.x, nearestRooms[0].startLocation.y,nearestRoomIndices[firstNearestIndex]);
		printf("Nearest room 2: %d at [%d,%d] with a distance of %d\n", nearestRooms[1].roomID, nearestRooms[1].startLocation.x, nearestRooms[1].startLocation.y,nearestRoomIndices[secondNearestIndex]);
	}

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
		return (d.dy > 0) ? 1 : 3; // Left or Right
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


Dun_Coord* getCellsOnVector(Dun_Coord start, Dun_Coord end) {
	Dun_Vec d = getVector(start, end);
	int distance = abs(d.dx) + abs(d.dy);
	Dun_Coord* ret = malloc((distance+2)*sizeof(Dun_Coord));
	for(int i = 0; i < distance + 2; i++) {
		ret[i] = (Dun_Coord) { -1, -1 }; // Initialize with invalid coordinates
	}

	Dun_Coord current = copyCoord(start);

	if (ret == NULL) {
		printf("Memory allocation failed\n");
		return NULL;
	}
	int index = 0;
	ret[index++] = copyCoord(current); // Add the starting coordinate

	do {
		if (abs(d.dx) > abs(d.dy)) {
			if (d.dx > 0) {
				current.x += down.dx;
				current.y += down.dy;
			}
			else {
				current.x += up.dx;
				current.y += up.dy;
			}
			d.dx = closerToZero(d.dx);
		}
		else {
			if (d.dy > 0) {
				current.x += right.dx;
				current.y += right.dy;
			}
			else {
				current.x += left.dx;
				current.y += left.dy;
			}
			d.dy = closerToZero(d.dy);
		}
		ret[index++] = copyCoord(current); // Add the current coordinate to the array

	} while (current.x != end.x || current.y != end.y);

	return ret;

}

Dun_Coord getSpotOnWall(Room r, Dun_Vec d) {
	Dun_Coord wallloc = getRoomCenter(r);


	int wallSide = -1; // Initialize wallSide to an invalid value
	if (d.dx == 0 && d.dy == 0)
		wallSide = rand() % 4; // Randomly choose a wall side
	else
		wallSide = getVectorDirection(d); // Get the direction of the vector
	// 0 = Up, 1 = Right, 2 = Down, 3 = Left
	if (!(r.exitNodes[wallSide][0].x > XBOUND && r.exitNodes[wallSide][0].y > YBOUND) && !(r.exitNodes[wallSide][1].x > XBOUND && r.exitNodes[wallSide][1].y > YBOUND)) {
		srand((unsigned int)time(NULL)); // Seed the random number generator with the current time
		return (rand() % 2 == 0) ? r.exitNodes[wallSide][2] : r.exitNodes[wallSide][3]; // Randomly choose one of the exit nodes
	}

	int minSearch = 0;
	int maxSearch = 0;

	switch (wallSide) {
		case 0:
			minSearch = r.startLocation.y;
			maxSearch = r.startLocation.y + r.ydim - 1;
			break;
		case 1:
			minSearch = r.startLocation.x;
			maxSearch = r.startLocation.x + r.xdim - 1;
			break;
		case 2:
			minSearch = r.startLocation.y;
			maxSearch = r.startLocation.y + r.ydim - 1;
			break;
		case 3:
			minSearch = r.startLocation.x;
			maxSearch = r.startLocation.x + r.xdim - 1;
			break;
	}

	Dun_Coord t = (Dun_Coord){ XBOUND + 1,YBOUND + 1 };

	int runner = minSearch + 0;

	while (runner < maxSearch) {
		switch (wallSide) {
			case 0: // Top Wall
				if (world[r.startLocation.x][runner].ref == '.') {
					if (DEBUG)
						printf("Found a spot on the top wall at [%d,%d]\n", r.startLocation.x, runner);
					t = (Dun_Coord) { r.startLocation.x, runner };
					runner = maxSearch + 1;
					continue;
				}
				break;
			case 1: // Right Wall
				if (world[runner][r.startLocation.y + r.ydim - 1].ref == '.') {
					if (DEBUG)
						printf("Found a spot on the right wall at [%d,%d]\n", runner, r.startLocation.y + r.ydim - 1);
					t = (Dun_Coord) { runner, r.startLocation.y + r.ydim - 1 };
					runner = maxSearch + 1;
					continue;
				}
				break;
			case 2: // Bottom Wall
				if (world[r.startLocation.x + r.xdim - 1][runner].ref == '.') {
					if (DEBUG)
						printf("Found a spot on the bottom wall at [%d,%d]\n", r.startLocation.x + r.xdim - 1, runner);
					t = (Dun_Coord) { r.startLocation.x + r.xdim - 1, runner };
					runner = maxSearch + 1;
					continue;
				}
				break;
			case 3: // Left Wall
				if (world[runner][r.startLocation.y].ref == '.') {
					if (DEBUG)
						printf("Found a spot on the left wall at [%d,%d]\n", runner, r.startLocation.y);
					t = (Dun_Coord) { runner, r.startLocation.y };
					runner = maxSearch + 1;
					continue;
				}
				break;
			default:
				printf("Error: Invalid wall side chosen.\n");
				return (Dun_Coord) { XBOUND+1, YBOUND+1 }; // Return an invalid coordinate
		}
		runner++;
	}

	if (!(t.x < XBOUND && t.y < YBOUND)) {
		switch (wallSide) {
			case 0: // Top Wall
				t.x = r.startLocation.x;
				t.y = (r.ydim > 7 ? r.startLocation.y + 2 + (rand() % (r.ydim - 4)) : 2);
				break;
			case 1: // Right Wall
				t.x = (r.xdim > 7 ? r.startLocation.x + 2 + (rand() % (r.xdim - 4)) : 2);
				t.y = r.startLocation.y + r.ydim - 1;
				break;
			case 2: // Bottom Wall
				t.x = r.startLocation.x + r.xdim - 1;
				t.y = (r.ydim > 7 ? r.startLocation.y + 2 + (rand() % (r.ydim - 4)) : 2);
				break;
			case 3: // Left Wall
				t.x = (r.xdim > 7 ? r.startLocation.x + 2 + (rand() % (r.xdim - 4)) : 2);;
				t.y = r.startLocation.y;
				break;
			default:
				printf("Error: Invalid wall side chosen.\n");
				return (Dun_Coord) { XBOUND+1, YBOUND+1 }; // Return an invalid coordinate
		}
	}
	if (r.exitNodes[wallSide][0].x > XBOUND && r.exitNodes[wallSide][0].y > YBOUND){
		r.exitNodes[wallSide][0] = (Dun_Coord){ t.x,t.y };
	} else if ((!isAdjacent(r.exitNodes[wallSide][0], t)) && r.exitNodes[wallSide][1].x > XBOUND && r.exitNodes[wallSide][1].y > YBOUND){
		r.exitNodes[wallSide][1] = (Dun_Coord){ t.x,t.y };
	} else {
		Dun_Vec d = getVector(t,r.exitNodes[wallSide][0]);
		t.x += d.dx;
		t.y += d.dy;
		r.exitNodes[wallSide][1] = (Dun_Coord){ t.x,t.y };
	}
	wallloc.x = t.x;
	wallloc.y = t.y;
	world[wallloc.x][wallloc.y].ref = '.'; // Mark the wall location as passable
	world[wallloc.x][wallloc.y].passable = 1; // Mark the wall location as passable
	popAdMat(wallloc); // Update the adjacency matrix for the wall location
	

	for (unsigned int i = 0; i < BUFFER - 1; i++) {
		switch (wallSide) {
			case 0:wallloc.x += up.dx; wallloc.y += up.dy; break; // Up
			case 1:wallloc.x += right.dx; wallloc.y += right.dy; break; // Right
			case 2:wallloc.x += down.dx; wallloc.y += down.dy; break; // Down
			case 3:wallloc.x += left.dx; wallloc.y += left.dy; break; // Left
		}
		world[wallloc.x][wallloc.y].ref = '.'; // Mark the wall location as passable
		world[wallloc.x][wallloc.y].passable = 1; // Mark the wall location as passable
		popAdMat(wallloc); // Update the adjacency matrix for the wall location
	}

	if(r.exitNodes[wallSide][2].x > XBOUND && r.exitNodes[wallSide][2].y > YBOUND) {
		r.exitNodes[wallSide][2] = wallloc; // Update the exit node for the wall side
	} else {
		r.exitNodes[wallSide][3] = wallloc; // Update the exit node for the wall side
	}
	return wallloc;
}

Dun_Coord copyCoord(Dun_Coord d) {
	Dun_Coord copy = { d.x, d.y };
	return copy; // Returns a copy of the coordinate
}

int closeToZero(int a, int b) {
	return abs(a) < abs(b) ? a : b; // Returns the value closer to zero
}

int closerToZero(int value) {
	if (value > 0)
		return value - 1;
	else if (value < 0)
		return value + 1;
	else
		return 0; // Already zero
}

void popAdMat(Dun_Coord d) {
	Dun_Coord* adjacent = malloc(4 * sizeof(Dun_Coord));
	if (adjacent == NULL) {
		printf("Memory allocation failed\n");
		return;
	}
	if(inRange(d.x+up.dx, 0, XBOUND) && inRange(d.y+up.dy, 0, YBOUND))
		adjacent[0] = (Dun_Coord){ d.x + up.dx, d.y + up.dy }; // Up
	else
		adjacent[0] = (Dun_Coord){ -1, -1 }; // Invalid coordinate

	if (inRange(d.x + right.dx, 0, XBOUND) && inRange(d.y + right.dy, 0, YBOUND))
		adjacent[1] = (Dun_Coord){ d.x + right.dx, d.y + right.dy }; // Right
	else
		adjacent[1] = (Dun_Coord){ -1, -1 }; // Invalid coordinate

	if (inRange(d.x + down.dx, 0, XBOUND) && inRange(d.y + down.dy, 0, YBOUND))
		adjacent[2] = (Dun_Coord){ d.x + down.dx, d.y + down.dy }; // Down
	else
		adjacent[2] = (Dun_Coord){ -1, -1 }; // Invalid coordinate

	if (inRange(d.x + left.dx, 0, XBOUND) && inRange(d.y + left.dy, 0, YBOUND))
		adjacent[3] = (Dun_Coord){ d.x + left.dx, d.y + left.dy }; // Left
	else
		adjacent[3] = (Dun_Coord){ -1, -1 }; // Invalid coordinate

	for (int i = 0; i < 4; i++) {
		if(adjacent[i].x == -1 || adjacent[i].y == -1) {
			if (DEBUG) {
				printf("Adjacent cell [%d,%d] is out of bounds.\n", adjacent[i].x, adjacent[i].y);
			}
			world[d.x][d.y].admat[i] = 0; // Mark as not passable
			continue; // Skip to the next iteration
		}
		if(world[adjacent[i].x][adjacent[i].y].passable) {
			if (DEBUG) {
				printf("Adjacent cell [%d,%d] is passable.\n", adjacent[i].x, adjacent[i].y);
			}
			world[d.x][d.y].admat[i] = 1; // Mark as passable
		} else {
			if (DEBUG) {
				printf("Adjacent cell [%d,%d] is not passable.\n", adjacent[i].x, adjacent[i].y);
			}
			world[d.x][d.y].admat[i] = 0; // Mark as not passable
		}
	}
	free(adjacent); // Free the allocated memory for adjacent cells

}

void updateWorldAdMat() {
	for(int x = 0; x < XBOUND; x++) {
		for(int y = 0; y < YBOUND; y++) {
			if(world[x][y].passable) {
				if (DEBUG) {
					printf("Updating adjacency matrix for cell [%d,%d]\n", x, y);
				}
				popAdMat((Dun_Coord){x, y}); // Update the adjacency matrix for the cell
			}
		}
	}
}


void cutPaths() {
	for (unsigned int i = 0; i < roomCount; i++) {
		Room r = rooms[i];
		Room* nearestRooms = malloc(2 * sizeof(Room));
		int* visited = malloc(roomCount * sizeof(int));
		if (nearestRooms == NULL || visited == NULL) {
			printf("Memory allocation failed\n");
			return;
		}
		for (unsigned int j = 0; j < roomCount; j++)
			visited[j] = 0; // Initialize visited array
		nearestRooms = getNearest2Rooms(r);
		if (nearestRooms == NULL) {
			printf("Error: Could not get nearest rooms.\n");
			free(visited);
			return;
		}
		if (DEBUG) {
			printf("Cutting paths for room %d at [%d,%d]\n", r.roomID, r.startLocation.x, r.startLocation.y);
			printf("Nearest room 1: %d at [%d,%d]\n", nearestRooms[0].roomID, nearestRooms[0].startLocation.x, nearestRooms[0].startLocation.y);
			printf("Nearest room 2: %d at [%d,%d]\n", nearestRooms[1].roomID, nearestRooms[1].startLocation.x, nearestRooms[1].startLocation.y);
		}

		Dun_Coord wallLoc[2];
		Dun_Coord endLoc[2];


		if (DEBUG)
		    printf("Here!\n");
		wallLoc[0] = getSpotOnWall(r, getVectorToWallFromCenter(r, getVector(getRoomCenter(r), getRoomCenter(nearestRooms[0]))));
		wallLoc[1] = getSpotOnWall(r, getVectorToWallFromCenter(r, getVector(getRoomCenter(r), getRoomCenter(nearestRooms[1]))));
		if (DEBUG) {
			printf("Here 2!\n");
			printf("Wall Loc 0: (%d, %d)\n", wallLoc[0].x, wallLoc[0].y);
			printf("Wall Loc 1: (%d, %d)\n", wallLoc[1].x, wallLoc[1].y);
			printf("Here 2.5!\n");
		}

		if (DEBUG)
		    printf("Here 5!\n");
		PFCL path1, path2;
		DCL path1Coords, path2Coords;
		endLoc[0] = getSpotOnWall(nearestRooms[0], getVector(getRoomCenter(nearestRooms[0]), getRoomCenter(r)));
		endLoc[1] = getSpotOnWall(nearestRooms[1], getVector(getRoomCenter(nearestRooms[1]), getRoomCenter(r)));

		if (DEBUG) {
			printf("----->Beginning A Star Pathing for Room %d<-------\n",r.roomID);
		}

		if (!isThereAPath(wallLoc[0],endLoc[0])) {
			path1 = AStar(wallLoc[0], endLoc[0], 1);
			path1Coords = ExtractCoordinates(path1);
			for (unsigned int j = 0; j < path1Coords.size; j++) {
				world[path1Coords.items[j].x][path1Coords.items[j].y].ref = '.'; // Mark the path as passable
				world[path1Coords.items[j].x][path1Coords.items[j].y].passable = 1; // Mark the path as passable
				//popAdMat(path1Coords.items[j]); // Update the adjacency matrix for the path coordinates
			}
		}
		
		if(!isThereAPath(wallLoc[1], endLoc[1])) {
			path2 = AStar(wallLoc[1], endLoc[1], 1);
			path2Coords = ExtractCoordinates(path2);
			for (unsigned int j = 0; j < path2Coords.size; j++) {
				world[path2Coords.items[j].x][path2Coords.items[j].y].ref = '.'; // Mark the path as passable
				world[path2Coords.items[j].x][path2Coords.items[j].y].passable = 1; // Mark the path as passable
				//popAdMat(path2Coords.items[j]); // Update the adjacency matrix for the path coordinates
			}
		}

		
		

	}
	if(DEBUG)
	    printf("Here 10!\n");

}
