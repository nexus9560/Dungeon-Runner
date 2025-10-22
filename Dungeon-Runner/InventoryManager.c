#include "InventoryManager.h"


void displayInventory(Player* p) {
	if (!p) {
		printf("Error: Player pointer is NULL.\n");
		return;
	}
	if (p->inventory.size == 0) {
		return;
	}
}
