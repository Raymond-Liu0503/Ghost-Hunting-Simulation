#include "defs.h"

/**
 * Initializes a RoomType structure with a given name and allocates necessary resources.
 * 
 * Parameters:
 *   room - A pointer to the RoomType structure to be initialized.
 *   name - The name of the room.
 *
 * Returns: None. The function initializes the room and its associated lists.
 */
void initRoom(RoomType *room, const char *name) {
    // Validate the input parameters
    if (!room || !name) {
        fprintf(stderr, "Error: Invalid parameters provided to initRoom.\n");
        return;
    }

    strncpy(room->name, name, MAX_STR - 1);
    room->name[MAX_STR - 1] = '\0'; 

    room->evidencelist = (EvidenceListType *)malloc(sizeof(EvidenceListType));
    if (room->evidencelist) {
        initEvidenceList(room->evidencelist);
    } else {
        fprintf(stderr, "Error: Memory allocation for evidence list failed.\n");
    }

    room->hunterArray = (HunterArrayType *)malloc(sizeof(HunterArrayType));
    if (room->hunterArray) {
        initHunterArray(room->hunterArray, NUM_HUNTERS);
    } else {
        fprintf(stderr, "Error: Memory allocation for hunter array failed.\n");
    }

    room->ghost = NULL;
    room->roomlist = NULL;
}

/**
 * Initializes a RoomListType structure.
 * 
 * Parameters:
 *   roomList - A pointer to the RoomListType structure to be initialized.
 *
 * Returns: None. The function initializes the room list.
 */
void initRoomList(RoomListType *roomList) {
    if (!roomList) {
        fprintf(stderr, "Error: Null pointer provided to initRoomList.\n");
        return;
    }

    roomList->rhead = NULL;
    roomList->rtail = NULL;
    roomList->size = 0;

    if (sem_init(&roomList->sem, 0, 1) != 0) {
        fprintf(stderr, "Error: Semaphore initialization failed in initRoomList.\n");
    }
}


/**
 * Adds a room to a room list.
 *
 * Parameters:
 *   list - A pointer to the RoomListType structure representing the list of rooms.
 *   room - A pointer to the RoomType structure representing the room to add.
 *
 * Returns: None. The function adds a room to the end of the list.
 */
void addRoom(RoomListType* list, RoomType* room) {

    if (!list || !room) {
        fprintf(stderr, "Error: Null parameter passed to addRoom.\n");
        return;
    }

    RoomNodeType* newNode = (RoomNodeType*)malloc(sizeof(RoomNodeType));
    if (!newNode) {
        fprintf(stderr, "Error: Memory allocation for new room node failed.\n");
        exit(EXIT_FAILURE);
    }

    newNode->room = room;
    newNode->next = NULL;


    sem_wait(&list->sem);

    // Add the new node 
    if (!list->rhead) {
        list->rhead = list->rtail = newNode;
    } else {
        // append the new node and update the tail
        list->rtail->next = newNode;
        list->rtail = newNode;
    }

    list->size++;

    sem_post(&list->sem);
}


/**
 * Connects two rooms by adding each to the other's list of adjacent rooms.
 *
 * Parameters:
 *   room1 - A pointer to the first RoomType structure.
 *   room2 - A pointer to the second RoomType structure.
 *
 * Returns: None. The function connects two rooms bidirectionally.
 */
void connectRooms(RoomType* room1, RoomType* room2) {

    if (!room1 || !room2) {
        fprintf(stderr, "Error: Null room passed to connectRooms.\n");
        return;
    }

    initializeAndConnect(room1, room2);
    initializeAndConnect(room2, room1);
}

/**void safelyFreeRoom(RoomType *room) 
 * Initializes a room's room list if not already initialized and adds another room to it.
 *
 * Parameters:
 *   room - A pointer to the RoomType structure to be initialized and connected.
 *   otherRoom - A pointer to the other RoomType structure to connect with.
 *
 * Returns: None.
 */
void initializeAndConnect(RoomType* room, RoomType* otherRoom) {
    // Ensure the room's room list is initialized
    if (!room->roomlist) {
        room->roomlist = (RoomListType*)malloc(sizeof(RoomListType));
        if (!room->roomlist) {
            fprintf(stderr, "Error: Memory allocation for room list failed.\n");
            exit(EXIT_FAILURE);
        }
        initRoomList(room->roomlist);
    }

    // Add the other room to this room's list
    addRoom(room->roomlist, otherRoom);
}

/**
 * Frees all resources associated with a RoomListType structure.
 *
 * Parameters:
 *   roomList - A pointer to the RoomListType structure to be freed.
 *
 * Returns: None. The function frees each room node and the room list itself.
 */
void freeRoomList(RoomListType *roomList) {

    if (!roomList) {
        fprintf(stderr, "Error: Attempted to free a NULL room list.\n");
        return;
    }

    RoomNodeType *node = roomList->rhead;
    while (node) {
        RoomNodeType *nextNode = node->next;
        safelyFreeRoom(node->room); 
        free(node); 
        node = nextNode;
    }

    free(roomList); 
}

/**
 * helper function and frees a RoomType structure, if it exists.
 *
 * Parameters:
 *   room - A pointer to the RoomType structure to be freed.
 *
 * Returns: None.
 */
void safelyFreeRoom(RoomType *room) {
    if (room) {
  
        if (room->evidencelist) {
            freeEvidenceList(room->evidencelist); 
            free(room->evidencelist);
        }

        free(room); 
    }
}
/**
 *  function to free a RoomType structure.
 *
 * Parameters:
 *   room - A pointer to the RoomType structure to be freed.
 *
 * Returns: None.
 */

void freeRoom(RoomType *room) {
    safelyFreeRoom(room);
}