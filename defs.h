#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <semaphore.h>
#include <time.h>

#define MAX_STR         64
#define MAX_RUNS        50
#define BOREDOM_MAX     100
#define C_TRUE          1
#define C_FALSE         0
#define HUNTER_WAIT     5000
#define GHOST_WAIT      600
#define NUM_HUNTERS     4
#define FEAR_MAX        10
#define LOGGING         C_TRUE
#define MAX_EV    3

typedef enum EvidenceType EvidenceType;
typedef enum GhostClass GhostClass;

typedef     struct  Ghost   GhostType;
typedef     struct  Room    RoomType;
typedef     struct  House   HouseType;
typedef     struct  RoomList    RoomListType;
typedef     struct  RoomNode    RoomNodeType;
typedef     struct  EvidenceList    EvidenceListType;
typedef     struct  EvidenceNode    EvidenceNodeType;
typedef     struct  Hunter   HunterType;
typedef    struct  EvidenceArray EvidenceArrayType;
typedef    struct  HunterArray HunterArrayType;
typedef    struct  sharedState SharedGameState;



enum EvidenceType { EMF, TEMPERATURE, FINGERPRINTS, SOUND, EV_COUNT, EV_UNKNOWN };
enum GhostClass { POLTERGEIST, BANSHEE, BULLIES, PHANTOM, GHOST_COUNT, GH_UNKNOWN };
enum LoggerDetails { LOG_FEAR, LOG_BORED, LOG_EVIDENCE, LOG_SUFFICIENT, LOG_INSUFFICIENT, LOG_UNKNOWN };

// Helper Utilies
int randInt(int,int);        // Pseudo-random number generator function
float randFloat(float, float);  // Pseudo-random float generator function
enum GhostClass randomGhost();  // Return a randomly selected a ghost type
void ghostToString(enum GhostClass, char*); // Convert a ghost type to a string, stored in output paremeter
void evidenceToString(enum EvidenceType, char*); // Convert an evidence type to a string, stored in output parameter

// Logging Utilities
void l_hunterInit(char* name, enum EvidenceType equipment);
void l_hunterMove(char* name, char* room);
void l_hunterReview(char* name, enum LoggerDetails reviewResult);
void l_hunterCollect(char* name, enum EvidenceType evidence, char* room);
void l_hunterExit(char* name, enum LoggerDetails reason);
void l_ghostInit(enum GhostClass type, char* room);
void l_ghostMove(char* room);
void l_ghostEvidence(enum EvidenceType evidence, char* room);
void l_ghostExit(enum LoggerDetails reason);

void populateRooms(HouseType* house);
void freeHouse(HouseType *house); 

struct Room {
    char name[MAX_STR];
    EvidenceListType *evidencelist;
    HunterArrayType *hunterArray;
    RoomListType *roomlist; 
    GhostType *ghost;
};

struct Ghost {
  RoomType *room;
  GhostClass ghostType;
  int boredomTime;
  
};

struct RoomNode {
    RoomType *room;
    struct RoomNode *next;
};

struct RoomList {
  RoomNodeType *rhead;
  RoomNodeType *rtail;
  int size; 
    sem_t sem;
} ;

 struct House{
    RoomListType* rooms;
    HunterArrayType* hunterArray;
    EvidenceArrayType* evidenceArray;
    int hunterCount;

};

struct EvidenceList {
  EvidenceNodeType *ehead;
  EvidenceNodeType *etail;
    sem_t sem;
} ;

struct EvidenceNode {
    enum EvidenceType evidence;
    struct EvidenceNode *next;
} ;

struct Hunter {
    char name[MAX_STR];
    EvidenceType equipment;
    EvidenceArrayType *evidenceArray; 
    int fear;
    int boredom;
    RoomType *room;
    pthread_t thread;
} ;

struct EvidenceArray {
    EvidenceType *evidence;
    int size,capacity;
    sem_t sem;
} ;

struct HunterArray {
    HunterType *hunter;
    int size,capacity;
    sem_t sem;
};

typedef struct GhostBehaviorContext {
    
    HunterArrayType* hunters; 
    GhostType* ghost;
    HouseType* house;
    int numHunters;
    SharedGameState *sharedState;  

} GhostBehaviorContext;

typedef struct HunterBehaviorContext {
    HunterType *hunter;            
    GhostType *ghosts;            
    HouseType *house;              
    EvidenceArrayType *sharedEvidence;   
    HunterArrayType *allHunters;      
    SharedGameState *sharedState;  

} HunterBehaviorContext;

struct sharedState{
    int gameOver;
};

//main helpers
void setupHouse(HouseType *house);
GhostType* prepareGhost(HouseType *house);
void inputHunterNames(char names[][MAX_STR]); 
void initializeHunters(HouseType *house, char names[][MAX_STR]);
void logHunterInitialization(HunterArrayType *hunterArray);
void setupThreads(pthread_t *ghostThread, pthread_t hunterThreads[], SharedGameState *gameState, GhostType *ghost, HouseType *house);
void waitForThreadsCompletion(pthread_t ghostThread, pthread_t hunterThreads[]);
void evaluateGameOutcome(HouseType *house, GhostType *ghost);
void cleanupResources(GhostType *ghost, HouseType *house);
    

void initEvidence(EvidenceType *evidence, enum EvidenceType type);
void initEvidenceList(EvidenceListType *list);
void initEvidenceArray(EvidenceArrayType *evidenceArray, int size);
EvidenceType addEv(GhostType* ghost);
EvidenceType determineEvidenceType(GhostClass ghostType);
int isEvidenceCollected(EvidenceArrayType *evidenceArray, EvidenceType evidence);
void reviewEv(EvidenceArrayType *evidenceArray, GhostType *ghost);
GhostClass identifyGhostFromEvidence(EvidenceType evidence[3]);

EvidenceType doesEvidenceExist(RoomType *room, EvidenceType hunterEquipment);
int collectEv(EvidenceArrayType *evidenceArray, EvidenceType evidence);
void freeEvidenceList(EvidenceListType *evidenceList);
void freeHunterArray(HunterArrayType *hunterArray);

void initHunterArray(HunterArrayType *hunterArray, int size);
void initHunter(HunterType *hunter, const char *name, EvidenceType equipment, RoomType *room); 
int isHunterPresent(GhostType* ghost, HunterArrayType* list, int numHunters);
void *hunterBehaviour(void *param);
int addHunter(HunterArrayType *hunterArray, const HunterType *newHunter);
void moveToRandomRoomHunter(HunterType *hunter, HouseType *house);
void updateHunterState(HunterType *hunter, GhostType *ghosts, HouseType *house, EvidenceArrayType *sharedEvidence, SharedGameState *sharedState); 
int isSufficientEvidence(EvidenceArrayType *sharedEvidence); 
void assignRandomEquipment(HunterArrayType* hunters, int numHunters);
void freeEvidenceArray(EvidenceArrayType *evidenceArray);
void removeHunter(HunterArrayType *hunters_list, HunterType* hunter);
void clearHunterArray(HunterArrayType *hunterArray);
void performHunterAction(HunterType *hunter, HouseType *house, EvidenceArrayType *sharedEvidence);
void logHunterExit(HunterType *hunter);
void decrementHunterCount(HouseType *house);
void collectEvidenceIfNeeded(HunterType *hunter, EvidenceArrayType *sharedEvidence);
void reviewEvidenceAndExitIfNeeded(HunterType *hunter, EvidenceArrayType *sharedEvidence);
void freeHunterResources(HunterType *hunter);

int addEvidenceAndLog(HunterType *hunter, EvidenceArrayType *sharedEvidence, EvidenceType collectedEv);
void initHouse(HouseType *house);
void freeRoomList(RoomListType *roomList);
void initGhost(GhostType *ghost, enum GhostClass type, RoomType *room);
void *ghostBehaviour(void *param);
void updateGhost(GhostType *ghost, HunterArrayType *hunters, int numHunters, SharedGameState *sharedState); 
int isGhostPresent(GhostType* ghost, HunterType *hunter);
void moveToRandomRoomGhost(GhostType *ghost);
void freeGhost(GhostType *ghost);
void initGhostBehavior(GhostBehaviorContext *context, GhostType *ghost, HouseType *house, HunterArrayType *hunters, SharedGameState *sharedState); 

void initRoom(RoomType *room, const char *name);
RoomType* createRoom(const char* name) ;
//RoomListType *createRoom();
// RoomListType *createRoomList();
// HunterArrayType *createHunterArray(int initialCapacity);
// EvidenceArrayType *createEvidenceArray(int initialCapacity);
void initRoomList(RoomListType *list);
void addRoom(RoomListType* list, RoomType* room);
void connectRooms(RoomType* room1, RoomType* room2);
void initializeAndConnect(RoomType* room, RoomType* otherRoom);
RoomType* getRandomRoom(HouseType *house);
void freeRoom(RoomType *room); 
void safelyFreeRoom(RoomType *room) ;
int usleep(int);
RoomType* getRandomRoomExcludeVan(HouseType *house); 
int isValidGhostAndHunterList(GhostType* ghost, HunterArrayType* list, int numHunters);
int isSameRoom(RoomType* room1, RoomType* room2);
int isHunterAndHouseValid(HunterType *hunter, HouseType *house) ;
int getRandomRoomIndex(int roomCount);
RoomType* getRoomAtIndex(RoomListType *roomList, int index);
void updateHunterLocation(HunterType *hunter, RoomType *newRoom);
int isValidHouse(HouseType *house);
int countNonVanRooms(RoomListType *roomList) ;
RoomType* findRoomByIndex(RoomListType *roomList, int index);