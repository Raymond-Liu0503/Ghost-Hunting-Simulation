#include "defs.h"

/**
 * Initializes a HunterType structure with provided attributes.
 *
 * Parameters:
 *   hunter - A pointer to the HunterType structure to be initialized.
 *   name - The name of the hunter.
 *   equipment - The type of equipment the hunter carries.
 *   room - The initial room of the hunter.
 *
 * Returns: None.
 */
void initHunter(HunterType *hunter, const char *name, EvidenceType equipment, RoomType *room) {
    if (!hunter) {
        fprintf(stderr, "Error: Null pointer provided to initHunter.\n");
        return;
    }
    if (strlen(name) < MAX_STR) {
        strcpy(hunter->name, name);
    } else {
        printf("Error: Name is too long in initHunter\n");
        return;
    }

    hunter->equipment = equipment;

    hunter->fear = 0;
    hunter->boredom = 0;
    hunter->room = room;

    hunter->evidenceArray = (EvidenceArrayType *)malloc(sizeof(EvidenceArrayType));
    if (hunter->evidenceArray) {
        initEvidenceArray(hunter->evidenceArray, MAX_EV);
    } else {
        fprintf(stderr, "Error: Memory allocation for evidence array failed.\n");
    }

}   

/**
 * Initializes a HunterArrayType structure with a specified initial capacity.
 * 
 * Parameters:
 *   hunterArray - A pointer to the HunterArrayType structure to be initialized.
 *   initial_capacity - The initial capacity for the hunter array.
 * 
 * Returns: None. The function initializes the hunter array and sets its capacity.
 */
void initHunterArray(HunterArrayType *hunterArray, int initial_capacity) {

    if (!hunterArray) {
        fprintf(stderr, "Error: Null pointer provided to initHunterArray.\n");
        return; 
    }

    // Validate the initial capacity
    if (initial_capacity <= 0) {
        fprintf(stderr, "Error: Invalid initial capacity provided.\n");
        hunterArray->hunter = NULL;
        hunterArray->size = 0;
        hunterArray->capacity = 0;
        return; 
    }

    // Allocate memory 
    hunterArray->hunter = (HunterType *)malloc(sizeof(HunterType) * initial_capacity);
    if (!hunterArray->hunter) {
        fprintf(stderr, "Error: Memory allocation for hunters failed.\n");
        hunterArray->size = 0;
        hunterArray->capacity = 0;
        return; 
    }

    // Initialize the size and capacity fields
    hunterArray->size = 0;
    hunterArray->capacity = initial_capacity;

    // Initialize the semaphore for thread safety
    if (sem_init(&hunterArray->sem, 0, 1) != 0) {
        fprintf(stderr, "Error: Semaphore initialization failed in initHunterArray.\n");
        free(hunterArray->hunter); // Clean up 
        hunterArray->hunter = NULL;
        hunterArray->size = 0;
        hunterArray->capacity = 0;
    }
}

/**
 * Adds a new hunter to the HunterArrayType structure.
 * 
 * Parameters:
 *   hunterArray - A pointer to the HunterArrayType structure where the new hunter will be added.
 *   newHunter - A pointer to the HunterType structure representing the new hunter to be added.
 * 
 * Returns:
 *   int - Returns 0 on successful addition, -1 on failure due to invalid input or full array.
 */
int addHunter(HunterArrayType *hunterArray, const HunterType *newHunter) {
    // Validate the input parameters
    if (!hunterArray || !newHunter) {
        fprintf(stderr, "Error: Invalid parameters provided to addHunter.\n");
        return -1;  
    }

    // Synchronize access to hunterArray using a semaphore
    sem_wait(&hunterArray->sem);

 
    if (hunterArray->size >= hunterArray->capacity) {
        fprintf(stderr, "Error: Hunter array has reached its capacity.\n");
        sem_post(&hunterArray->sem);  
        return -1;  
    }

    // Add the new hunter to the array
    hunterArray->hunter[hunterArray->size] = *newHunter;  // Copy 
    hunterArray->size++;  // Increment


    if (sem_post(&hunterArray->sem) != 0) {
        printf("Error: Failed to release semaphore in addHunter\n");
        return -1;
    }

    return 0;  // Indicate success
}

/**
 * Thread function for managing the behavior of a hunter in the game.
 *
 * Parameters:
 *   param - A pointer to HunterBehaviorContext containing hunter and game state information.
 *
 * Returns: None. The function controls the hunter's behavior until certain conditions are met.
 */
void *hunterBehaviour(void *param) {
    if (!param) {
        fprintf(stderr, "Error: Null parameter in hunterBehaviour\n");
        pthread_exit(NULL);
    }

    HunterBehaviorContext *context = (HunterBehaviorContext *)param;
    
    if (!context || !context->sharedState) {
        fprintf(stderr, "Error: Null context or shared state in hunterBehaviour\n");
        pthread_exit(NULL);
    }

    HunterType *hunter = context->hunter;
    HouseType *house = context->house;
    EvidenceArrayType *sharedEvidence = context->sharedEvidence;
    SharedGameState *sharedState = context->sharedState;

    for (; hunter->fear < FEAR_MAX && hunter->boredom < BOREDOM_MAX && !sharedState->gameOver; usleep(HUNTER_WAIT)) {
        updateHunterState(hunter, context->ghosts, house, sharedEvidence, sharedState);

        if (house->hunterCount == 0 || sharedEvidence->size >= 3) {
            sharedState->gameOver = 1; // Set game over condition
            break; // Exit the loop
        }
    }

    pthread_exit(NULL);
}

/**
 * Updates the state of a hunter based on the current game conditions.
 *
 * Parameters:
 *   hunter - A pointer to the HunterType structure representing the hunter.
 *   ghosts - A pointer to the GhostType structure representing the ghosts.
 *   house - A pointer to the HouseType structure representing the house.
 *   sharedEvidence - A pointer to the EvidenceArrayType structure for shared evidence.
 *   sharedState - A pointer to the SharedGameState structure representing the game's shared state.
 *
 * Returns: None. The function updates the hunter's state based on interactions with ghosts and evidence.
 */
void updateHunterState(HunterType *hunter, GhostType *ghosts, HouseType *house, EvidenceArrayType *sharedEvidence, SharedGameState *sharedState) {
    // Validate input parameters
    if (!hunter || !ghosts || !house || !sharedEvidence || !sharedState) {
        fprintf(stderr, "Error: Invalid parameter(s) provided to updateHunterState.\n");
        return; 
    }

    // Check for ghost presence 
    int ghostPresence = isGhostPresent(ghosts, hunter);
    if (ghostPresence) {
        hunter->fear = (hunter->fear < FEAR_MAX) ? hunter->fear + 1 : FEAR_MAX;
        hunter->boredom = 0;
    } else {
        hunter->boredom = (hunter->boredom < BOREDOM_MAX) ? hunter->boredom + 1 : BOREDOM_MAX;
    }


    if (hunter->fear >= FEAR_MAX || hunter->boredom >= BOREDOM_MAX) {
        logHunterExit(hunter); 
        decrementHunterCount(house); 
        pthread_exit(NULL);
    }

    // Perform actions based on random choice
    performHunterAction(hunter, house, sharedEvidence); // Assuming performHunterAction is defined elsewhere
}

/**
 * Logs the exit of a hunter from the game.
 *
 * Parameters:
 *   hunter - A pointer to the HunterType structure representing the exiting hunter.
 *   logMessage - A message indicating the reason for the hunter's exit.
 */
void logHunterExit(HunterType *hunter) {
    if (!hunter) {
        fprintf(stderr, "Error: Invalid parameter(s) provided to logHunterExit.\n");
        return;
    }

    // Log the hunter's exit with the provided message
    printf("Hunter %s has exited the game\n", hunter->name);
}

/**
 * Decrements the count of hunters in the house.
 *
 * Parameters:
 *   house - A pointer to the HouseType structure representing the game's house.
 */
void decrementHunterCount(HouseType *house) {
    if (!house) {
        fprintf(stderr, "Error: Null house provided to decrementHunterCount.\n");
        return;
    }

    // Ensure the hunter count doesn't fall below zero
    if (house->hunterCount > 0) {
        house->hunterCount--;
    } else {
        fprintf(stderr, "Warning: Attempted to decrement hunter count below zero.\n");
    }
}

/**
 * Helper function to perform a random hunter action.
 *
 * Parameters:
 *   hunter - A pointer to the HunterType structure representing the hunter.
 *   ghosts - A pointer to the GhostType structure representing the ghosts.
 *   house - A pointer to the HouseType structure representing the house.
 *   sharedEvidence - A pointer to the EvidenceArrayType structure for shared evidence.
 *   sharedState - A pointer to the SharedGameState structure representing the game's shared state.
 *
 * Returns: None. The function updates the hunter's state based on interactions with ghosts and evidence.
 */
void performHunterAction(HunterType *hunter, HouseType *house, EvidenceArrayType *sharedEvidence) {
       switch (randInt(0, 3)) {
        case 0: // Move to a random, connected room
            moveToRandomRoomHunter(hunter, house);
            l_hunterMove(hunter->name, hunter->room->name);
            break;
        case 1: // Collect evidence
            collectEvidenceIfNeeded(hunter, sharedEvidence);
            break;
        case 2: // Review evidence
            reviewEvidenceAndExitIfNeeded(hunter, sharedEvidence);
            break;
    }
}

// Helper function to collect evidence if present in the hunter's room
void collectEvidenceIfNeeded(HunterType *hunter, EvidenceArrayType *sharedEvidence) {
    if (hunter == NULL || sharedEvidence == NULL) {
        printf("Error: Invalid pointers in collectEvidenceIfNeeded\n");
        return;
    }

    EvidenceType collectedEv = doesEvidenceExist(hunter->room, hunter->equipment);
    if (collectedEv != EV_UNKNOWN) {
        if (addEvidenceAndLog(hunter, sharedEvidence, collectedEv) == 0) {
            printf("Error: Failed to add evidence to shared array\n");
        }
    }
}

// Helper function to add evidence to the shared array and log the collection
int addEvidenceAndLog(HunterType *hunter, EvidenceArrayType *sharedEvidence, EvidenceType collectedEv) {
    int added = collectEv(sharedEvidence, collectedEv);
    if (added != 0) {
        l_hunterCollect(hunter->name, collectedEv, hunter->room->name);
    }
    return added;
}

// Helper function to review evidence and exit if sufficient
void reviewEvidenceAndExitIfNeeded(HunterType *hunter, EvidenceArrayType *sharedEvidence) {
    if (isSufficientEvidence(sharedEvidence) >= 3) {
        l_hunterReview(hunter->name, LOG_SUFFICIENT);
        pthread_exit(NULL);
    } else {
        l_hunterReview(hunter->name, LOG_INSUFFICIENT);
    }
}


/**
 * Checks if there is sufficient evidence collected in the shared evidence array.
 *
 * Parameters:
 *   sharedEvidence - A pointer to the EvidenceArrayType structure containing collected evidence.
 *
 * Returns:
 *   int - The count of unique evidence types collected. Typically, a game condition might be met if 3 or more unique types are collected.
 */
int isSufficientEvidence(EvidenceArrayType *sharedEvidence) {
    // Validate input parameter
    if (!sharedEvidence) {
        fprintf(stderr, "Error: Null shared evidence provided to isSufficientEvidence.\n");
        return 0; 
    }

    // Array to track occurrences of each evidence type
    int evidenceOccurrence[MAX_EV] = {0};

    for (int i = 0; i < sharedEvidence->size; i++) {
        EvidenceType evType = sharedEvidence->evidence[i];
        if (evType >= 0 && evType < MAX_EV) {
            evidenceOccurrence[evType]++;
        } else {
            fprintf(stderr, "Warning: Encountered invalid evidence type: %d\n", evType);
        }
    }
int uniqueEvidenceTypes = 0;
// Count types of evidence
int *evidenceEnd = evidenceOccurrence + MAX_EV; 
for (int *ptr = evidenceOccurrence; ptr < evidenceEnd; ptr++) {
    if (*ptr > 0) {
        uniqueEvidenceTypes++;
    }
}

    return uniqueEvidenceTypes;
}


/**
 * Assigns a unique random piece of equipment to each hunter in the array.
 * 
 * Parameters:
 *   hunters - A pointer to the HunterArrayType structure containing the hunters.
 *   numHunters - The number of hunters to assign equipment to.
 *
 * Returns: None.
 */
void assignRandomEquipment(HunterArrayType *hunters, int numHunters) {
    // Validate input parameters
    if (!hunters || !hunters->hunter || numHunters <= 0 || hunters->size < numHunters) {
        fprintf(stderr, "Error: Invalid parameters provided to assignRandomEquipment.\n");
        return; 
    }


    if (numHunters > EV_COUNT) {
        fprintf(stderr, "Error: Number of hunters exceeds the number of available equipment types.\n");
        return; 
    }

    int assignedEquipment[EV_COUNT] = {0};

    for (int i = 0; i < numHunters; i++) {
        int equipmentIndex;
        do {
            equipmentIndex = randInt(0, EV_COUNT - 1);
        } while (assignedEquipment[equipmentIndex]); 

        hunters->hunter[i].equipment = equipmentIndex;
        assignedEquipment[equipmentIndex] = 1;
    }
}

/**
 * Removes a specific hunter from the hunters list.
 *
 * Parameters:
 *   hunters_list - A pointer to the HunterArrayType structure containing the list of hunters.
 *   hunter - A pointer to the HunterType structure representing the hunter to be removed.
 *
 * Returns: None. The function removes the specified hunter from the list.
 */
void removeHunter(HunterArrayType *hunters_list, HunterType *hunter) {
    // Validate input parameters
    if (!hunters_list || !hunter) {
        fprintf(stderr, "Error: Invalid parameters provided to removeHunter.\n");
        return;
    }

    // Iterate over the hunters list 
    for (int index = 0; index < hunters_list->size; index++) {
        if (strcmp(hunters_list->hunter[index].name, hunter->name) == 0) {

            for (int j = index; j < hunters_list->size - 1; j++) {
                hunters_list->hunter[j] = hunters_list->hunter[j + 1];
            }
            hunters_list->size--; // after removal
            break; 
        }
    }
}

/**
 * Clears the hunter array, freeing all allocated resources.
 *
 * Parameters:
 *   hunterArray - A pointer to the HunterArrayType structure to be cleared.
 *
 * Returns: None. The function clears the hunter array and frees associated memory.
 */
void clearHunterArray(HunterArrayType *hunterArray) {
    // Validate the hunterArray pointer
    if (!hunterArray) {
        fprintf(stderr, "Error: Null pointer provided to clearHunterArray.\n");
        return;
    }

   
    for (int i = 0; i < hunterArray->size; i++) {
        freeHunterResources(&hunterArray->hunter[i]);
    }


    free(hunterArray->hunter);
    hunterArray->hunter = NULL; 
    hunterArray->size = 0;
    hunterArray->capacity = 0;
}

/**
 * Frees resources allocated for a single hunter.
 *
 * Parameters:
 *   hunter - A pointer to the HunterType structure whose resources are to be freed.
 *
 * Returns: None. The function frees the hunter's allocated resources.
 */
void freeHunterResources(HunterType *hunter) {
    if (hunter && hunter->evidenceArray) {
        free(hunter->evidenceArray->evidence);
        free(hunter->evidenceArray);
    }
}