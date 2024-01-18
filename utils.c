#include "defs.h"

/*
    Returns a pseudo randomly generated number, in the range min to (max - 1), inclusively
        in:   lower end of the range of the generated number
        in:   upper end of the range of the generated number
    return:   randomly generated integer in the range [0, max-1) 
*/
int randInt(int min, int max)
{
    return (int) randFloat(min, max);
}

/*
    Returns a pseudo randomly generated floating point number.
    A few tricks to make this thread safe, just to reduce any chance of issues using random
        in:   lower end of the range of the generated number
        in:   upper end of the range of the generated number
    return:   randomly generated floating point number in the range [min, max)
*/
float randFloat(float min, float max) {
    static __thread unsigned int seed = 0;
    if (seed == 0) {
        seed = (unsigned int)time(NULL) ^ (unsigned int)pthread_self();
    }

    float random = ((float) rand_r(&seed)) / (float) RAND_MAX;
    float diff = max - min;
    float r = random * diff;
    return min + r;
}

/* 
    Returns a random enum GhostClass.
*/
enum GhostClass randomGhost() {
    return (enum GhostClass) randInt(0, GHOST_COUNT);
}


/*
    Returns the string representation of the given enum EvidenceType.
        in: type - the enum EvidenceType to convert
        out: str - the string representation of the given enum EvidenceType, minimum 16 characters
*/
void evidenceToString(enum EvidenceType type, char* str) {
    switch (type) {
        case EMF:
            strcpy(str, "EMF");
            break;
        case TEMPERATURE:
            strcpy(str, "TEMPERATURE");
            break;
        case FINGERPRINTS:
            strcpy(str, "FINGERPRINTS");
            break;
        case SOUND:
            strcpy(str, "SOUND");
            break;
        default:
            strcpy(str, "UNKNOWN");
            break;
    }
}

/* 
    Returns the string representation of the given enum GhostClass.
        in: ghost - the enum GhostClass to convert
        out: buffer - the string representation of the given enum GhostClass, minimum 16 characters
*/
void ghostToString(enum GhostClass ghost, char* buffer) {
    switch(ghost) {
        case BANSHEE:
            strcpy(buffer, "Banshee");
            break;
        case BULLIES:
            strcpy(buffer, "Bullies");
            break;
        case PHANTOM:
            strcpy(buffer, "Phantom");
            break;
        case POLTERGEIST:
            strcpy(buffer, "Poltergeist");
            break;
        default:
            strcpy(buffer, "Unknown");
            break;
        
    }
}


/*
    Checks if a hunter is present in the same room as the ghost.

    @param ghost - A pointer to the GhostType struct.
    @param list - A pointer to the HunterArrayType struct.
    @param numHunters - The number of hunters in the list.
    @return 0/true if a hunter is in the same room as the ghost
*/
int isHunterPresent(GhostType* ghost, HunterArrayType* list, int numHunters) {
   // Validate the ghost and the list of hunters
    if (!isValidGhostAndHunterList(ghost, list, numHunters)) {
        return 0; // Indicates no hunter is present
    }

    // Iterate through the list of hunters
    for (int i = 0; i < list->size; ++i) {
        if (isSameRoom(ghost->room, list->hunter[i].room)) {
            return 1; // Indicates a hunter is present in the same room
        }
    }

    return 0; // Indicates no hunter is present in the same room
}

/**
 * Validates the ghost and hunter list.
 *
 * Parameters:
 *   ghost - Pointer to the GhostType structure.
 *   list - Pointer to the HunterArrayType structure.
 *   numHunters - Number of hunters in the list.
 *
 * Returns:
 *   An integer, 1 if both ghost and list are valid, 0 otherwise.
 */
int isValidGhostAndHunterList(GhostType* ghost, HunterArrayType* list, int numHunters) {
    return ghost != NULL && list != NULL && list->size > 0 && numHunters > 0;
}

/**
 * Checks if two rooms are the same.
 *
 * Parameters:
 *   room1 - Pointer to the first RoomType structure.
 *   room2 - Pointer to the second RoomType structure.
 *
 * Returns:
 *   An integer, 1 if rooms are the same, 0 otherwise.
 */
int isSameRoom(RoomType* room1, RoomType* room2) {
    return room1 != NULL && room2 != NULL && room1 == room2;
}

/**
 * Checks if a ghost is present in the same room as a hunter.
 *
 * Parameters:
 *   ghost - Pointer to the GhostType structure.
 *   hunter - Pointer to the HunterType structure.
 *
 * Returns:
 *   An integer, 1 if the ghost is in the same room as the hunter, 0 otherwise.
 */
int isGhostPresent(GhostType* ghost, HunterType *hunter) {
    if (!ghost || !hunter) {
        fprintf(stderr, "Error: Invalid ghost or hunter reference in checkGhostPresence.\n");
        return 0; 
    }

    return (hunter->room == ghost->room) ? 1 : 0;
}

/**
 * Moves the ghost to a random connected room.
 *
 * Parameters:
 *   ghost - Pointer to the GhostType structure.
 *
 * Returns: None.
 */
void moveToRandomRoomGhost(GhostType *ghost) {
    if (!ghost || !ghost->room || !ghost->room->roomlist) {
        fprintf(stderr, "Error: Invalid ghost or room in repositionGhost.\n");
        return;
    }

    RoomListType *roomList = ghost->room->roomlist;
    int targetIndex = randInt(0, roomList->size - 1); 
    RoomNodeType *targetRoomNode = roomList->rhead;


    for (int i = 0; i < targetIndex && targetRoomNode; i++) {
        targetRoomNode = targetRoomNode->next;
    }


    if (targetRoomNode) {
        ghost->room = targetRoomNode->room;
    } else {
        fprintf(stderr, "Error: Target room not found in repositionGhost.\n");
    }

}


/**
 * Moves a hunter to a random connected room within the house.
 *
 * Parameters:
 *   hunter - Pointer to the HunterType structure.
 *   house - Pointer to the HouseType structure.
 *
 * Returns: None.
 */
void moveToRandomRoomHunter(HunterType *hunter, HouseType *house) {
      // Validate the inputs
    if (!isHunterAndHouseValid(hunter, house)) {
        fprintf(stderr, "Error: Invalid hunter or house reference in relocateHunter.\n");
        return;
    }

    RoomListType *roomList = hunter->room->roomlist;
    int targetRoomIndex = getRandomRoomIndex(roomList->size);

    RoomType *newRoom = getRoomAtIndex(roomList, targetRoomIndex);
    
    if (newRoom) {
        updateHunterLocation(hunter, newRoom);
    } else {
        fprintf(stderr, "Error: Target room not found in relocateHunter.\n");
    }
}

/**
 * Validates the hunter and house.
 */
int isHunterAndHouseValid(HunterType *hunter, HouseType *house) {
    return hunter != NULL && hunter->room != NULL && house != NULL;
}

/**
 * Gets a random index for a room.
 */
int getRandomRoomIndex(int roomCount) {
    return randInt(0, roomCount - 1);
}

/**
 * Retrieves a room at a specific index in the room list.
 */
RoomType* getRoomAtIndex(RoomListType *roomList, int index) {
    RoomNodeType *current = roomList->rhead;
    for (int i = 0; i < index && current; i++) {
        current = current->next;
    }
    return current ? current->room : NULL;
}

/**
 * Updates the location of the hunter to the new room.
 */
void updateHunterLocation(HunterType *hunter, RoomType *newRoom) {
    removeHunter(hunter->room->hunterArray, hunter); // Assuming removeHunter function exists
    hunter->room = newRoom;
    addHunter(newRoom->hunterArray, hunter); // Assuming addHunter function exists
}



/**
 * Selects a random room from the house, excluding the 'Van' room.
 *
 * Parameters:
 *   house - Pointer to the HouseType structure.
 *
 * Returns:
 *   Pointer to a randomly selected RoomType, excluding 'Van'. Returns NULL if no valid room is found.
 */
RoomType* getRandomRoomExcludeVan(HouseType *house) {
    if (!isValidHouse(house)) {
        fprintf(stderr, "Error: Invalid house structure in selectRandomNonVanRoom.\n");
        return NULL;
    }

    int totalRooms = countNonVanRooms(house->rooms);

    if (totalRooms <= 0) {
        fprintf(stderr, "Error: No valid rooms available apart from 'Van'.\n");
        return NULL;
    }

    int randomIndex = rand() % totalRooms;
    return findRoomByIndex(house->rooms, randomIndex);
}

/**
 * Validates the house structure.
 */
int isValidHouse(HouseType *house) {
    return house != NULL && house->rooms != NULL && house->rooms->rhead != NULL;
}

/**
 * Counts the number of rooms excluding the 'Van' room.
 */
int countNonVanRooms(RoomListType *roomList) {
    int count = 0;
    RoomNodeType *current = roomList->rhead;
    while (current) {
        if (strcmp(current->room->name, "Van") != 0) {
            count++;
        }
        current = current->next;
    }
    return count;
}

/**
 * Finds and returns the room at a specific index, excluding 'Van'.
 */
RoomType* findRoomByIndex(RoomListType *roomList, int index) {
    RoomNodeType *current = roomList->rhead;
    while (current) {
        if (strcmp(current->room->name, "Van") != 0) {
            if (index == 0) {
                return current->room;
            }
            index--;
        }
        current = current->next;
    }
    return NULL;
}