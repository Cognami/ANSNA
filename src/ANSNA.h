#ifndef ANSNA_H
#define ANSNA_H

////////////////////////////////////////////////////
//  ANSNA - Adaptive Neuro-Symbolic Network Agent //
////////////////////////////////////////////////////

//References//
//-----------//
#include "Cycle.h"


//Parameters//
//----------//
#define ANSNA_DEFAULT_FREQUENCY  1.0
#define ANSNA_DEFAULT_CONFIDENCE 0.9
#define ANSNA_DEFAULT_TRUTH ((Truth) { .frequency = ANSNA_DEFAULT_FREQUENCY, .confidence = ANSNA_DEFAULT_CONFIDENCE })

//Callback function types//
//-----------------------//
typedef void (*Action)(void);
typedef void (*EventInspector)(Event *);

//Methods//
//-------//
//Init/Reset system
void ANSNA_INIT();
//Run the system for a certain amount of cycles
void ANSNA_Cycles(int cycles);
//Add input
void ANSNA_AddInput(SDR sdr, char type, Truth truth);
void ANSNA_AddInputBelief(SDR sdr);
void ANSNA_AddInputGoal(SDR sdr);
//Add an operation
void ANSNA_AddOperation(SDR sdr, Action procedure);
//Add an operation
void ANSNA_AddEventInspector(SDR sdr, EventInspector procedure);

#endif
