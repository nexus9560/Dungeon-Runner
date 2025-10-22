#include "DungeonTypes.h"
#include "list.h"

const Dun_Vec up	= { -1,  0 };
const Dun_Vec right = {  0,  1 };
const Dun_Vec down	= {  1,  0 };
const Dun_Vec left	= {  0, -1 };
const Dun_Vec directions[4] = { { -1, 0 }, { 0, 1 }, { 1, 0 }, { 0, -1 } };

DR_LIST_IMPL(Entity)

Dun_Vec getVector(Dun_Coord start, Dun_Coord end) {
    Dun_Vec delta;
    delta.dx = end.x - start.x;
    delta.dy = end.y - start.y;
    return delta;
}

int checkItemOverlap(IOG_List* itemList, Item_on_Ground newItem) {
    for (unsigned int i = 0; i < itemList->size; i++) {
        if (itemList->items[i].loc.x == newItem.loc.x && itemList->items[i].loc.y == newItem.loc.y) {
			return 1; // Return true if overlap is found
        }
    }
    return 0; // No overlap found
}

char* Entity_tostring(Entity ent) {
    char* buffer = malloc(256 * sizeof(char));
    snprintf(buffer, 256, "[HP:%3d,ATK:%3d,AGR:%3d,TOH:%3d,DEF:%3d,EXP:%3d,EVA:%3d,LVL:%3d]:%32s\n",
             ent.health, ent.atk, ent.agr, ent.hit, ent.def, ent.exp, ent.eva, ent.level, ent.name);
    return buffer;
}

char* Entity__List_tostring(Entity__List *l) {
    char* buffer = malloc(l->size * 256 * sizeof(char));
    buffer[0] = '\0'; // Initialize the buffer
    for (unsigned int i = 0; i < l->size; i++) {
        char * as_string = Entity_tostring(l->items[i]);
        strncat(buffer, as_string, 256);
        free(as_string);
    }
    return buffer;
}

int getKeyPress() {
    int ch = -1;

#ifdef _WIN32
#define kbhit _kbhit
#define getch _getch
    if (kbhit())
        ch = getch(); // Get the actual key code

#elif __unix__ || __APPLE__
#define getch getchar


    struct termios oldt, newt;
    tcgetattr(STDIN_FILENO, &oldt);
    newt = oldt;
    newt.c_lflag &= ~(ICANON | ECHO);
    tcsetattr(STDIN_FILENO, TCSANOW, &newt);

    struct timeval tv = { 0L, 0L }; // Zero timeout
    fd_set fds;
    FD_ZERO(&fds);
    FD_SET(STDIN_FILENO, &fds);
    int ret = select(STDIN_FILENO + 1, &fds, NULL, NULL, &tv);
    if (ret > 0) {
        ch = getchar();
    }
    else {
        ch = -1; // No input
    }

    tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
#endif

    return ch;
}
