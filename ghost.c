#include "defs.h"


/**
 * Initializes a GhostType structure with specified ghost class and associated room.
 * 
 * Parameters:
 *   ghost - A pointer to the GhostType structure to be initialized.
 *   type - The class/type of the ghost.
 *   room - A pointer to the RoomType structure where the ghost is initially located.
 *          Pass NULL if the ghost isn't in any room initially.
 * 
 * Returns: None.
 */
void initGhost(GhostType *ghost, enum GhostClass type, RoomType *room) {
    if (!ghost) {
        fprintf(stderr, "Error: NULL pointer provided to initGhost.\n");
        return;
    }

    ghost->ghostType = type;
    ghost->room = room;
    ghost->boredomTime = 0;

    //  a stack-allocated array to hold the room name if room is not NULL
    char roomName[MAX_STR]; 

    if (room && room->name) {
        strncpy(roomName, room->name, MAX_STR);
        roomName[MAX_STR - 1] = '\0'; 
        l_ghostInit(ghost->ghostType, roomName);
    } else {
        l_ghostInit(ghost->ghostType, "No Room");
    }
}


/**
 * Updates the state of the ghost based on the current game conditions.
 *
 * Parameters:
 *   ghost - A pointer to the GhostType structure to be updated.
 *   hunters - A pointer to the HunterArrayType containing all hunters.
 *   numHunters - The number of hunters in the game.
 *   sharedState - A pointer to the SharedGameState representing the game's shared state.
 *
 * Returns: None.
 */
void updateGhost(GhostType *ghost, HunterArrayType *hunters, int numHunters, SharedGameState *sharedState) {
    
    // Validate input parameters
    if (!ghost || !hunters || !sharedState) {
        fprintf(stderr, "Error: Invalid parameter provided to updateGhost.\n");
        return; 
    }

    int isHunterInRoom = isHunterPresent(ghost, hunters, numHunters);
    ghost->boredomTime = isHunterInRoom ? 0 : ghost->boredomTime + 1;
    
    if (ghost->boredomTime >= BOREDOM_MAX) {
        l_ghostExit(LOG_BORED);
        sharedState->gameOver = 1; 
        pthread_exit(NULL); 
    }


    switch (randInt(0, 3)) {
        case 0: 
            break;
        case 1: // add evidence 
            if (ghost->room) {
                EvidenceType ev = addEv(ghost);
                l_ghostEvidence(ev, ghost->room->name);
            }
            break;
        case 2: // if no hunter present then move to rand room
            if (!isHunterInRoom && ghost->room) {
                moveToRandomRoomGhost(ghost); 
                l_ghostMove(ghost->room->name);
            }
            break;
    }
}

/**
 * Initializes the GhostBehaviorContext structure with the necessary game components.
 *
 * Parameters:
 *   context - A pointer to the GhostBehaviorContext to be initialized.
 *   ghost - A pointer to the GhostType representing the ghost.
 *   house - A pointer to the HouseType representing the house.
 *   hunters - A pointer to the HunterArrayType representing the hunters.
 *   sharedState - A pointer to the SharedGameState representing the game's shared state.
 *
 * Returns: None. The function initializes the provided GhostBehaviorContext.
 */
void initGhostBehavior(GhostBehaviorContext *context, GhostType *ghost, HouseType *house, HunterArrayType *hunters, SharedGameState *sharedState) {
    // Validate input parameters
    if (!context || !ghost || !house || !hunters || !sharedState) {
        fprintf(stderr, "Error: Invalid parameter(s) provided to initGhostBehaviorContext.\n");
        return; 
    }

    // Initialize
    context->ghost = ghost;
    context->house = house;
    context->hunters = hunters;
    context->sharedState = sharedState;
}

/**
 * Thread function for managing the behavior of the ghost.
 * 
 * Parameters:
 *   param - A pointer to GhostBehaviorContext containing ghost and game state information.
 * 
 * Returns: None. The function controls the ghost's behavior in the game until certain conditions are met.
 */
void *ghostBehaviour(void *param) {
    // cast 
    GhostBehaviorContext* context = (GhostBehaviorContext *)param;

    // vlidate the context
    if (!context) {
        fprintf(stderr, "Error: Ghost behavior thread received a NULL context.\n");
        pthread_exit(NULL); 
    }

    printf("Ghost thread id: %lu\n", (unsigned long)pthread_self());


    for (; context->ghost->boredomTime < BOREDOM_MAX && !context->sharedState->gameOver; usleep(GHOST_WAIT)) {
    updateGhost(context->ghost, context->hunters, context->numHunters, context->sharedState);
    if (context->ghost->boredomTime >= BOREDOM_MAX) {
        context->sharedState->gameOver = 1; // game over
        break; 
    }
}

    pthread_exit(NULL);
}


/**
 * Frees the memory allocated for a GhostType structure, including its dynamically allocated members.
 * 
 * Parameters:
 *   ghost - A pointer to the GhostType structure to be freed.
 * 
 * Returns: None.
 */
void freeGhost(GhostType *ghost) {
    // Check if the pointer is not null
    if (!ghost) {
        return;
    }
    free(ghost);
}