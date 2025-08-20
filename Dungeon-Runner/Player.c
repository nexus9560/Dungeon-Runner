#include "Player.h"

DR_LIST_IMPL(Limb)
DR_LIST_IMPL(Item)

// Makes a basic player with human characteristics.
int player_init(Player* p) {
	if (!p) return 0; // Check for null pointer
	Limb leftArm = { .arm = 1, .left = 1 };
	Limb rightArm = { .arm = 1, .left = 0 };
	Limb leftLeg = { .arm = 0, .left = 1 };
	Limb rightLeg = { .arm = 0, .left = 0 };
	Segment torso = { .name = "Torso" };
	Limb__List_init(&torso.limbs, 2); // Initialize the limbs list for the torso
	Segment lowerBody = { .name = "Lower Body" };
	Limb__List_init(&lowerBody.limbs, 2); // Initialize the limbs list for the lower body
	add_Limb(&leftArm, &torso);
	add_Limb(&rightArm, &torso);
	add_Limb(&leftLeg, &lowerBody);
	add_Limb(&rightLeg, &lowerBody);

	Item__List_init(&p->inventory, 32); // Initialize the inventory with a capacity of 32 items

	return 1;
}

int add_Limb(Limb* limb, Segment* seg) {
	if (!limb || !seg) return 0; // Check for null pointers
	Limb__List_push(&seg->limbs, *limb); // Add the limb to the segment
	return 1;
}

int remove_Limb(Limb* limb, Segment* seg) {
	if (!limb || !seg) return 0; // Check for null pointers
	for (unsigned int i = 0; i < seg->limbs.size; i++) {
		if (seg->limbs.items[i].arm == limb->arm && seg->limbs.items[i].left == limb->left) {
			Limb__List_pop(&seg->limbs, &seg->limbs.items[i]); // Remove the limb from the segment
			return 1; // Successfully removed
		}
	}
	return 0;
}

int add_Item_to_Inventory(Player* p, Item* item) {
	if (!p || !item) return 0; // Check for null pointers
	Item__List_push(&p->inventory, *item); // Add the item to the inventory
	return 1;
}

int remove_Item_from_Inventory(Player* p, Item* item) {
	if (!p || !item) return 0; // Check for null pointers
	for (unsigned int i = 0; i < p->inventory.size; i++) {
		if (p->inventory.items[i].id == item->id) {
			Item__List_pop(&p->inventory, &p->inventory.items[i]); // Remove the item from the inventory
			return 1; // Successfully removed
		}
	}
	return 0;
}

int equip_Item_to_Limb(Item* item, Limb* limb) {
	if(!limb || !item) {
		return 0; // Check for null pointers
	}
	if (item->equipped) {
		return 0; // Item is already equipped
	}
	item->equipped = 1;
	if(item->type == WEAPON && limb->arm) {
		limb->weapon = item; // Equip the item as a weapon
		return 1;
	} elif(item->type == ARMOR) {
		limb->armor = item; // Equip the item as armor
		return 1;
	}
	return 0;
}

int unequip_Item_from_Limb(Item* item, Limb* limb) {
	if(!item || !limb) {
		return 0; // Check for null pointers
	}
	if (!item->equipped) {
		return 0; // Item is not equipped
	}
	item->equipped = 0;


	return 1;
}
