#include "defs.h"

// Initializes an evidence array with a specified capacity.
//
// Parameters:
//   evidenceArray - A pointer to the evidence array to be initialized.
//   capacity - The capacity of the evidence array.
//
// Returns: None.
void initEvidenceArray(EvidenceArrayType *evidenceArray, int capacity) {
    if (evidenceArray == NULL) {
        fprintf(stderr, "Error: Null pointer provided to initEvidenceArray.\n");
        return; // if null exit
    }

    if (capacity <= 0) {
        fprintf(stderr, "Error: Invalid capacity for evidence array (%d).\n", capacity);
        evidenceArray->evidence = NULL;
        evidenceArray->size = 0;
        evidenceArray->capacity = 0;
        return; 
    }

    // alocate memory 
    evidenceArray->evidence = (EvidenceType *)malloc(capacity * sizeof(EvidenceType));
    if (evidenceArray->evidence == NULL) {
        fprintf(stderr, "Error: Failed to allocate memory for evidence array.\n");
        evidenceArray->size = 0;
        evidenceArray->capacity = 0;
        return; 
    }

    //array fieds
    evidenceArray->size = 0;
    evidenceArray->capacity = capacity;

    // semaphore initaliziaion
    if (sem_init(&evidenceArray->sem, 0, 1) != 0) {
        fprintf(stderr, "Error: Failed to initialize semaphore for evidence array.\n");
        free(evidenceArray->evidence); // clean up mem
        evidenceArray->evidence = NULL;
        evidenceArray->size = 0;
        evidenceArray->capacity = 0;
    }
}


// Initializes an evidence list, setting head and tail pointers to NULL
// and initializing a semaphore for thread safety.
//
// Parameters:
//   evidenceList - A pointer to the evidence list to be initialized.
//
// Returns: None.
void initEvidenceList(EvidenceListType *evidenceList) {
    //check if null
    if (!evidenceList) {
        fprintf(stderr, "Error: Attempted to initialize a null EvidenceListType pointer.\n");
        return; 
    }

    evidenceList->etail = NULL;
    evidenceList->ehead = NULL;

    // thread safety
    if (sem_init(&evidenceList->sem, 0, 1) != 0) {
        fprintf(stderr, "Error: Semaphore initialization failed for EvidenceListType.\n");
        
    }
}

// Adds evidence to the room's evidence list based on the ghost's type.
//
// Parameters:
//   ghost - A pointer to the ghost whose evidence is to be added.
//
// Returns:
//   EvidenceType - The type of evidence added to the list.
//                  Returns EV_UNKNOWN if invalid parameters are provided or memory allocation fails.
EvidenceType addEv(GhostType* ghost) {
    // validate ghost
    if (!ghost || !ghost->room || !ghost->room->evidencelist) {
        fprintf(stderr, "Error: Invalid ghost or room configuration.\n");
        return EV_UNKNOWN; //ret unknow ev
    }

    // det ev type and use helper func
    EvidenceType evidenceToAdd = determineEvidenceType(ghost->ghostType); // Assume this function is defined elsewhere

    EvidenceNodeType* newNode = (EvidenceNodeType*)malloc(sizeof(EvidenceNodeType));
    if (!newNode) {
        fprintf(stderr, "Error: Failed to allocate memory for new evidence node.\n");
        return EV_UNKNOWN; // failed
    }
    //new node
    newNode->evidence = evidenceToAdd;
    newNode->next = NULL;

    // Tthread safety
    sem_wait(&ghost->room->evidencelist->sem); 
    if (!ghost->room->evidencelist->ehead) {
        
        //new node is both head and tail now 
        ghost->room->evidencelist->ehead = ghost->room->evidencelist->etail = newNode;
    } else {
        // append
        ghost->room->evidencelist->etail->next = newNode;
        ghost->room->evidencelist->etail = newNode;
    }
    sem_post(&ghost->room->evidencelist->sem); 

    return evidenceToAdd; 
}

// Helper function to determine the type of evidence based on the ghost's class.
//
// Parameters:
//   ghostType - The class/type of the ghost.
//
// Returns:
//   EvidenceType - The determined type of evidence associated with the given ghost class.
EvidenceType determineEvidenceType(GhostClass ghostType) {
    int choice = randInt(0, 3); 
    switch (ghostType) {
        case POLTERGEIST: return (choice == 0) ? EMF : (choice == 1) ? TEMPERATURE : FINGERPRINTS;
        case BANSHEE:     return (choice == 0) ? EMF : (choice == 1) ? TEMPERATURE : SOUND;
        case BULLIES:     return (choice == 0) ? EMF : (choice == 1) ? FINGERPRINTS : SOUND;
        case PHANTOM:     return (choice == 0) ? TEMPERATURE : (choice == 1) ? FINGERPRINTS : SOUND;
        default:          return EV_UNKNOWN; 
    }
}

// Collects a specific type of evidence and adds it to the evidence array.
//
// Parameters:
//   evidenceArray - A pointer to the evidence array where the evidence will be added.
//   evidence - The type of evidence to be collected.
//
// Returns:
//   int - Returns 1 if evidence is successfully collected and added, -1 if there is an error.
int collectEv(EvidenceArrayType *evidenceArray, EvidenceType evidence) {
    
    //validate array
    if (!evidenceArray) {
        fprintf(stderr, "Error: collectEvidence called with NULL evidence array.\n");
        return -1; }

    // check the evidence type
    if (evidence >= EV_UNKNOWN || evidence < EMF) {
        fprintf(stderr, "Error: Invalid evidence type provided: %d.\n", evidence);
        return -1;  
    }

    sem_wait(&evidenceArray->sem);


    //add if enough space in arr
    if (evidenceArray->size >= MAX_EV) {
        fprintf(stderr, "Error: Cannot collect evidence, array is full.\n");
    } else if (isEvidenceCollected(evidenceArray, evidence)) {
        fprintf(stderr, "Error: Evidence type %d already collected.\n", evidence);
    } else {
        // add new evidence to the array
        evidenceArray->evidence[evidenceArray->size++] = evidence;
        fprintf(stdout, "Collected evidence type %d, total count: %d.\n", evidence, evidenceArray->size);
        sem_post(&evidenceArray->sem);  
        return 1;  
    }


    sem_post(&evidenceArray->sem);
    return -1; 
}

// Helper function to check if a specific type of evidence is already collected in the evidence array.
//
// Parameters:
//   evidenceArray - A pointer to the evidence array to be checked.
//   evidence - The type of evidence to check for.
//
// Returns:
//   int - Returns 1 if the evidence is already collected, 0 otherwise.
int isEvidenceCollected(EvidenceArrayType *evidenceArray, EvidenceType evidence) {
    for (int i = 0; i < evidenceArray->size; i++) {
        if (evidenceArray->evidence[i] == evidence) {
            return 1;  // ev is already collected
        }
    }
    return 0;  // ev not collected yet
}


// Checks if a specific type of evidence exists in a room.
//
// Parameters:
//   room - A pointer to the room where evidence is being checked.
//   hunterEquipment - The type of evidence the hunter can detect.
//
// Returns:
//   EvidenceType - Returns the type of evidence found that matches the hunter's equipment,
//                  or EV_UNKNOWN if not found.
EvidenceType doesEvidenceExist(RoomType *room, EvidenceType hunterEquipment) {

    if (!room || !room->evidencelist) {
        fprintf(stderr, "Error: Invalid room or evidence list.\n");
        return EV_UNKNOWN;  
    }

    for (EvidenceNodeType *node = room->evidencelist->ehead; node != NULL; node = node->next) {
        if (node->evidence == hunterEquipment) {
            return node->evidence; // ret  matching evidence type
        }
    }

    return EV_UNKNOWN; // No evidence found
}

// Reviews the collected evidence in an array to identify the ghost type.
//
// Parameters:
//   evidenceArray - A pointer to the array of collected evidence.
//   ghost - A pointer to the ghost being investigated.
//
// Returns: None. Prints the result of the ghost identification based on the evidence.
void reviewEv(EvidenceArrayType *evidenceArray, GhostType *ghost) {
    // validate 
    if (!evidenceArray || evidenceArray->size != 3) {
        printf("Not enough evidence collected.\n");
        return;
    }

    sem_wait(&evidenceArray->sem);

    GhostClass identifiedGhostType = identifyGhostFromEvidence(evidenceArray->evidence);
    sem_post(&evidenceArray->sem);

    char ghostName[MAX_STR]; 
    ghostToString(identifiedGhostType, ghostName); 

    // output  result 
    if (identifiedGhostType == ghost->ghostType) {
        printf("Correctly identified the ghost type as %s!\n", ghostName);
    } else {
        printf("Incorrect ghost type. Further investigation needed.\n");
    }
}

// Helper function to identify the ghost from given evidence.
//
// Parameters:
//   evidence[3] - An array of three collected evidence types.
//
// Returns:
//   GhostClass - The identified class of the ghost based on the evidence.
GhostClass identifyGhostFromEvidence(EvidenceType evidence[3]) {
    int evidenceFlags[4] = {0}; 
    
    for (int i = 0; i < 3; i++) {
        if (evidence[i] >= EMF && evidence[i] <= SOUND) {
            evidenceFlags[evidence[i]] = 1;
        }
    }

    // det the ghost type based on evidence
    if (evidenceFlags[EMF] && evidenceFlags[TEMPERATURE] && evidenceFlags[FINGERPRINTS]) {
        return POLTERGEIST;
    } else if (evidenceFlags[EMF] && evidenceFlags[TEMPERATURE] && evidenceFlags[SOUND]) {
        return BANSHEE;
    } else if (evidenceFlags[EMF] && evidenceFlags[FINGERPRINTS] && evidenceFlags[SOUND]) {
        return BULLIES;
    } else if (evidenceFlags[TEMPERATURE] && evidenceFlags[FINGERPRINTS] && evidenceFlags[SOUND]) {
        return PHANTOM;
    }

    return GH_UNKNOWN; // Return unknown if no match 
}

// Frees the memory allocated for an evidence array and destroys its semaphore.
//
// Parameters:
//   evidenceArray - A pointer to the evidence array to be freed.
//
// Returns: None. Frees the allocated memory and destroys the semaphore.
void freeEvidenceArray(EvidenceArrayType *evidenceArray) {
    // Check if not null
    if (!evidenceArray) {
        fprintf(stderr, "Error: Attempted to free a NULL EvidenceArrayType.\n");
        return; }

    // ree the evidence array inside the structure
    if (evidenceArray->evidence) {
        free(evidenceArray->evidence);
        evidenceArray->evidence = NULL; 
    }

    sem_destroy(&evidenceArray->sem);

}

// Frees the memory allocated for all nodes in an evidence list.
//
// Parameters:
//   evidenceList - A pointer to the evidence list to be freed.
//
// Returns: None. Frees all nodes in the list and resets the list's head and tail pointers.
void freeEvidenceList(EvidenceListType *evidenceList) {

    if (!evidenceList) {
        fprintf(stderr, "Error: Attempted to free a NULL EvidenceListType.\n");
        return;
    }

    EvidenceNodeType *current = evidenceList->ehead;
    while (current) {
        EvidenceNodeType *nextNode = current->next;
        free(current);  
        current = nextNode; 

    // Reset
    evidenceList->ehead = NULL;
    evidenceList->etail = NULL;
}
}