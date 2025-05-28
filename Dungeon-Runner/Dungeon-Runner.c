
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
Item itemGlossary[MAX_ITEMS];
const Dun_Vec up = { -1,  0 };
const Dun_Vec right = { 0,  1 };
const Dun_Vec down = { 1,  0 };
const Dun_Vec left = { 0, -1 };
Room* rooms;
unsigned int currRoomCount = 0;

void mapClearing();
void makeRoomSpace(Room r);
int* getConsoleWindow();
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
void drawMap();
Entity shiftEntity(Entity e, Dun_Vec delta);
Entity moveEntity(Entity e, Dun_Coord newLoc);
int checkBounds( Dun_Coord newPos, Dun_Vec delta);
int checkOccupied(Dun_Coord newPos, Dun_Vec delta);
int checkArea(Room room1, Room room2);
Room* makeRooms();
Room getRoomByLocation(Dun_Coord d);
int getRandomEnemyIndex();
int isInRoom(Room r, Dun_Coord d);
int isInARoom(Dun_Coord d);
Dun_Coord getNearestSafeLocation(Dun_Coord d, int searchRadius);
Dun_Vec getVector(Dun_Coord start, Dun_Coord end);
void printMap();
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
	roomRunner();


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

int checkArea(Room room1, Room room2) {
	if ((room1.startLocation.x + room1.xdim < room2.startLocation.x || room2.startLocation.x + room2.xdim < room1.startLocation.x) &&
		(room1.startLocation.y + room1.ydim < room2.startLocation.y || room2.startLocation.y + room2.ydim < room1.startLocation.y)) {
		return 0;
	} else
	return 1;
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

int isInARoom(Dun_Coord d) {
	if (currRoomCount != 1) {
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
	Room* temp = malloc(roomCount*sizeof(Room));
	if (temp == NULL) {
		printf("Memory allocation failed\n");
		return NULL;
	}
    // srand returns void, so call it separately, then use rand() for random numbers
    srand((unsigned int)time(NULL));

	for (unsigned int i = 0; i < roomCount; i++) {
		unsigned int randX = (unsigned int)(rand() % (unsigned int)(XBOUND * 0.10));
		unsigned int randY = (unsigned int)(rand() % (unsigned int)(YBOUND * 0.10));
		temp[i].xdim = (randX < 3 ? 3 : randX);
		temp[i].ydim = (randY < 3 ? 3 : randY);
		temp[i].startLocation.x = (unsigned int)(rand() % (XBOUND - temp[i].xdim));
		temp[i].startLocation.y = (unsigned int)(rand() % (YBOUND - temp[i].ydim));
		makeRoomSpace(temp[i]);

	} 
	return temp;
}

void makeRoomSpace(Room r) {
	for (unsigned int x = r.startLocation.x; x < r.startLocation.x + r.xdim; x++) {
		for (unsigned int y = r.startLocation.y; y < r.startLocation.y + r.ydim; y++) {
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
            if (you.base.location.x == x && you.base.location.y == y) {
                mapChar = '@';
            } else if (world[x][y].passable == 0) {
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

