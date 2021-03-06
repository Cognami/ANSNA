#ifndef MEMORY_H
#define MEMORY_H

///////////////////
//  ANSNA Memory //
///////////////////
//The conce-based memory of ANSNA
//See voting mechanism to make it fast:
//https://github.com/patham9/ANSNA/wiki/Voting-Schema

//References//
//////////////
#include "Concept.h"
#include "PriorityQueue.h"

//Parameters//
//----------//
#define CONCEPTS_MAX 1000 //TODO statically alloc once on INIT, as can lead in value too large for the compiler to handle
#define EVENTS_MAX 64
#define VOTING 1
#define EXHAUSTIVE 2
#define USE_HASHING true
#define MATCH_STRATEGY EXHAUSTIVE
#define OPERATIONS_MAX 1000

//Data structure//
//--------------//
//Data structures
PriorityQueue concepts;
PriorityQueue events;
typedef void (*Action)(void);
typedef void (*EventInspector)(Event *);
typedef struct
{
    SDR sdr;
    Action action;
}Operation;
Operation operations[OPERATIONS_MAX];
EventInspector event_inspector;

//Methods//
//-------//
//Init memory
void Memory_INIT();
//Find a concept
bool Memory_FindConceptBySDR(SDR *sdr, SDR_HASH_TYPE sdr_hash, int *returnIndex);
//Create a new concept
Concept* Memory_Conceptualize(SDR *sdr, Attention attention);
//Return closest concept
bool Memory_getClosestConcept(Event *event, int *returnIndex);
//Add an already existing concept to memory that was taken out from the concept priority queue
bool Memory_addConcept(Concept *concept);
//Add event to memory
bool Memory_addEvent(Event *event);
//Add operation to memory
void Memory_addOperation(Operation op);
//Reset events
void Memory_ResetEvents();
//Reset concepts
void Memory_ResetConcepts();
#endif
