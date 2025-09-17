#pragma once
#ifndef LOADINGDOCK_H
#define LOADINGDOCK_H

#include "DungeonTypes.h"
#include "Player.h"
int loadPlayer();
void savePlayer();

void loadEntities(int ovr);
void ensure_directory(const char* path);


void loadRooms(Room__List* r, int ovr);
void saveRooms();

void loadItems(int ovr);

#endif// LOADINGDOCK_H
