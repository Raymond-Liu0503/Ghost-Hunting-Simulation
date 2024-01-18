#include "defs.h"

/*
    Dynamically allocates several rooms and populates the provided house.
    Note: You may modify this as long as room names and connections are maintained.
        out: house - the house to populate with rooms. Assumes house has been initialized.
*/
void populateRooms(HouseType* house) {
    // First, create each room

    // createRoom assumes that we dynamically allocate a room, initializes the values, and returns a RoomType*
    // create functions are pretty typical, but it means errors are harder to return aside from NULL
    struct Room* van                = createRoom("Van");
    struct Room* hallway            = createRoom("Hallway");
    struct Room* master_bedroom     = createRoom("Master Bedroom");
    struct Room* boys_bedroom       = createRoom("Boy's Bedroom");
    struct Room* bathroom           = createRoom("Bathroom");
    struct Room* basement           = createRoom("Basement");
    struct Room* basement_hallway   = createRoom("Basement Hallway");
    struct Room* right_storage_room = createRoom("Right Storage Room");
    struct Room* left_storage_room  = createRoom("Left Storage Room");
    struct Room* kitchen            = createRoom("Kitchen");
    struct Room* living_room        = createRoom("Living Room");
    struct Room* garage             = createRoom("Garage");
    struct Room* utility_room       = createRoom("Utility Room");

    // This adds each room to each other's room lists
    // All rooms are two-way connections
    // THIS IS INDIVIDUAL ROOM LISTS THAT CONTAINS ALL THE CONNECTIONS OF THE ROOMS IN THE HOUSE
    connectRooms(van, hallway);
    connectRooms(hallway, master_bedroom);
    connectRooms(hallway, boys_bedroom);
    connectRooms(hallway, bathroom);
    connectRooms(hallway, kitchen);
    connectRooms(hallway, basement);
    connectRooms(basement, basement_hallway);
    connectRooms(basement_hallway, right_storage_room);
    connectRooms(basement_hallway, left_storage_room);
    connectRooms(kitchen, living_room);
    connectRooms(kitchen, garage);
    connectRooms(garage, utility_room);

    // Add each room to the house's room list
    // THIS IS THE HOUSEROOM LIST OF ALL THE ROOMS IN THE HOUSE
    addRoom(house->rooms, van);
    addRoom(house->rooms, hallway);
    addRoom(house->rooms, master_bedroom);
    addRoom(house->rooms, boys_bedroom);
    addRoom(house->rooms, bathroom);
    addRoom(house->rooms, basement);
    addRoom(house->rooms, basement_hallway);
    addRoom(house->rooms, right_storage_room);
    addRoom(house->rooms, left_storage_room);
    addRoom(house->rooms, kitchen);
    addRoom(house->rooms, living_room);
    addRoom(house->rooms, garage);
    addRoom(house->rooms, utility_room);
}

/**
 * Initializes a HouseType structure, setting up the necessary components for the house.
 * 
 * Parameters:
 *   house - A pointer to the HouseType structure to be initialized.
 *
 * Returns: None. The function initializes the house's rooms, hunter array, and evidence array.
 */
void initHouse(HouseType *house) {

    if (!house) {
        fprintf(stderr, "Error: Null pointer provided to initHouse.\n");
        exit(EXIT_FAILURE); // Exit if house is NULL
    }

    house->rooms = (RoomListType *)malloc(sizeof(RoomListType));
    if (!house->rooms) {
        fprintf(stderr, "Error: Failed to allocate memory for room list.\n");
        exit(EXIT_FAILURE);
    }
    initRoomList(house->rooms); 

    house->hunterArray = (HunterArrayType *)malloc(sizeof(HunterArrayType));
    if (!house->hunterArray) {
        fprintf(stderr, "Error: Failed to allocate memory for hunter array.\n");
        exit(EXIT_FAILURE);
    }
    initHunterArray(house->hunterArray, NUM_HUNTERS); 

    house->evidenceArray = (EvidenceArrayType *)malloc(sizeof(EvidenceArrayType));
    if (!house->evidenceArray) {
        fprintf(stderr, "Error: Failed to allocate memory for evidence array.\n");
        exit(EXIT_FAILURE);
    }
    initEvidenceArray(house->evidenceArray, MAX_EV); 

    house->hunterCount = NUM_HUNTERS;
}

/**
 * Creates a new room with the given name.
 *
 * Parameters:
 *   name - The name of the new room to be created.
 *
 * Returns:
 *   RoomType* - A pointer to the newly created RoomType structure.
 */
RoomType* createRoom(const char* name) {
    // Validate the name parameter
    if (!name) {
        fprintf(stderr, "Error: Null name provided to createRoom.\n");
        exit(EXIT_FAILURE);
    }

    // Allocate memory 
    RoomType* newRoom = (RoomType*)malloc(sizeof(RoomType));
    if (!newRoom) {
        fprintf(stderr, "Error: Memory allocation for new room failed.\n");
        exit(EXIT_FAILURE);
    }

    initRoom(newRoom, name);  

    return newRoom;
}

/**
 * Frees all allocated resources within a HouseType structure.
 *
 * Parameters:
 *   house - A pointer to the HouseType structure whose resources are to be freed.
 *
 * Returns: None. The function frees all resources associated with the house.
 */
void freeHouse(HouseType *house) {
    if (!house) {return; }
    else if (house->rooms != NULL) {
        freeRoomList(house->rooms);
    }

    clearHunterArray(house->hunterArray); 
    freeEvidenceArray(house->evidenceArray); 
    
}