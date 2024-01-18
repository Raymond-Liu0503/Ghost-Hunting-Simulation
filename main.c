#include "defs.h"

int main() {
    srand(time(NULL));

    HouseType house;
    setupHouse(&house);

    GhostType *ghost = prepareGhost(&house);

    char hunterNames[NUM_HUNTERS][MAX_STR];
    inputHunterNames(hunterNames);

    initializeHunters(&house, hunterNames);

    assignRandomEquipment(house.hunterArray, house.hunterArray->size);
    logHunterInitialization(house.hunterArray);

    SharedGameState gameState = {0};

    pthread_t ghostThread, hunterThreads[NUM_HUNTERS];
    setupThreads(&ghostThread, hunterThreads, &gameState, ghost, &house);

    waitForThreadsCompletion(ghostThread, hunterThreads);

    evaluateGameOutcome(&house, ghost);

    cleanupResources(ghost, &house);
    return 0;
}

/**
 * Sets up the house by initializing it and populating rooms.
 * 
 * Parameters:
 *   house - Pointer to HouseType structure to be set up.
 */
void setupHouse(HouseType *house) {
    initHouse(house);
    populateRooms(house);
}

/**
 * Prepares a ghost by allocating memory and initializing it in a random room.
 * 
 * Parameters:
 *   house - Pointer to HouseType structure containing the rooms.
 * 
 * Returns:
 *   Pointer to the allocated and initialized GhostType.
 */
GhostType* prepareGhost(HouseType *house) {
    GhostType *ghost = malloc(sizeof(GhostType));
    RoomType *randomRoom = getRandomRoomExcludeVan(house);
    initGhost(ghost, randomGhost(), randomRoom);
    return ghost;
}

/**
 * Inputs names for hunters.
 * 
 * Parameters:
 *   names - Two-dimensional array to store hunter names.
 */
void inputHunterNames(char names[][MAX_STR]) {
    for (int i = 0; i < NUM_HUNTERS; i++) {
        printf("Enter name for hunter %d: ", i + 1);
        fgets(names[i], MAX_STR, stdin);
        names[i][strcspn(names[i], "\n")] = 0;
    }
}

/**
 * Initializes hunters and adds them to the house and van room.
 * 
 * Parameters:
 *   house - Pointer to HouseType structure.
 *   names - Two-dimensional array containing hunter names.
 */
void initializeHunters(HouseType *house, char names[][MAX_STR]) {
    RoomType *vanRoom = house->rooms->rhead->room;
    for (int i = 0; i < NUM_HUNTERS; i++) {
        HunterType hunter;
        initHunter(&hunter, names[i], EV_UNKNOWN, vanRoom);
        addHunter(house->hunterArray, &hunter);
        addHunter(vanRoom->hunterArray, &hunter);
    }
}

/**
 * Logs the initialization of each hunter.
 * 
 * Parameters:
 *   hunterArray - Pointer to HunterArrayType structure containing hunters.
 */
void logHunterInitialization(HunterArrayType *hunterArray) {
    for (int i = 0; i < hunterArray->size; i++) {
        l_hunterInit(hunterArray->hunter[i].name, hunterArray->hunter[i].equipment);
    }
}

/**
 * Sets up and starts threads for the ghost and each hunter.
 * 
 * Parameters:
 *   ghostThread - Pointer to pthread_t for the ghost thread.
 *   hunterThreads - Array of pthread_t for hunter threads.
 *   gameState - Pointer to SharedGameState structure.
 *   ghost - Pointer to GhostType structure.
 *   house - Pointer to HouseType structure.
 */
void setupThreads(pthread_t *ghostThread, pthread_t hunterThreads[], SharedGameState *gameState, GhostType *ghost, HouseType *house) {
    GhostBehaviorContext *ghostContext = malloc(sizeof(GhostBehaviorContext));
    initGhostBehavior(ghostContext, ghost, house, house->hunterArray, gameState);
    pthread_create(ghostThread, NULL, ghostBehaviour, (void *)ghostContext);

    for (int i = 0; i < NUM_HUNTERS; i++) {
        HunterBehaviorContext *hunterContext = malloc(sizeof(HunterBehaviorContext));
        hunterContext->hunter = &house->hunterArray->hunter[i];
        hunterContext->ghosts = ghost;
        hunterContext->house = house;
        hunterContext->sharedEvidence = house->evidenceArray;
        hunterContext->sharedState = gameState;
        pthread_create(&hunterThreads[i], NULL, hunterBehaviour, (void *)hunterContext);
    }
}
/**
 * Waits for all threads to complete.
 * 
 * Parameters:
 *   ghostThread - pthread_t for the ghost thread.
 *   hunterThreads - Array of pthread_t for hunter threads.
 */
void waitForThreadsCompletion(pthread_t ghostThread, pthread_t hunterThreads[]) {
    pthread_join(ghostThread, NULL);
    for (int i = 0; i < NUM_HUNTERS; i++) {
        pthread_join(hunterThreads[i], NULL);
    }
}

/**
 * Evaluates the outcome of the game based on the state of hunters and ghost.
 * 
 * Parameters:
 *   house - Pointer to HouseType structure.
 *   ghost - Pointer to GhostType structure.
 */
void evaluateGameOutcome(HouseType *house, GhostType *ghost) {
    int fear_count = 0;
    int boredom_count_hunter = 0;

    printf("=================================\n");
    printf("All done! Let's tally the results...\n");
    printf("=================================\n");

    // Analyze each hunter's fear 
    if (house->hunterArray->size == 0) {
        printf("There are no hunters left in the house.\n");
        fear_count = NUM_HUNTERS;
    } else {
        for (int i = 0; i < house->hunterArray->size; i++) {
            printf("%s's fear level is %d\n", house->hunterArray->hunter[i].name, house->hunterArray->hunter[i].fear);
            if (house->hunterArray->hunter[i].fear >= 100) {
                fear_count++;
            }
        }
    }

    // Analyze each hunter's boredom
    for (int i = 0; i < house->hunterArray->size; i++) {
        printf("%s's boredom level is %d\n", house->hunterArray->hunter[i].name, house->hunterArray->hunter[i].boredom);
        if (house->hunterArray->hunter[i].boredom >= 100) {
            boredom_count_hunter++;
        }
    }

    // Print ghost's boredom level
    printf("The ghost's boredom level is %d\n", ghost->boredomTime);

    // Review evidence
    reviewEv(house->evidenceArray, ghost);

    // Determine the game's outcome
    if (fear_count == NUM_HUNTERS || boredom_count_hunter == NUM_HUNTERS) {
        printf("The ghost has won the game!\n");
    } else if (house->evidenceArray->size == 3 && ghost->boredomTime < 100) {
        printf("The hunters have won the game!\n");
    } else {
        printf("The ghost got bored and left.\n");
    }
}


/**
 * Cleans up and frees allocated resources.
 * 
 * Parameters:
 *   ghost - Pointer to GhostType to be freed.
 *   house - Pointer to HouseType structure to be freed.
 */
void cleanupResources(GhostType *ghost, HouseType *house) {
    freeGhost(ghost);
    freeHouse(house);
}