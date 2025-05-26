#ifndef LOADINGDOCK_H
#define LOADINGDOCK_H
#include "DungeonTypes.h"

int loadPlayer();
void savePlayer();

void loadEntities(int ovr);

Room* loadRooms(int ovr);
void saveRooms();

void loadItems(int ovr);

#endif// LOADINGDOCK_H