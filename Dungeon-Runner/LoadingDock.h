#ifndef LOADINGDOCK_H
#define LOADINGDOCK_H

#pragma once
#include "DungeonTypes.h"
int loadPlayer();
void savePlayer();

void loadEntities(int ovr);
void ensure_directory(const char* path);


Room* loadRooms(int ovr);
void saveRooms();

void loadItems(int ovr);

#endif// LOADINGDOCK_H
