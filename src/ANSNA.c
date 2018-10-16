#include "ANSNA.h"


long currentTime = CONCEPT_WAIT_TIME+1;
void ANSNA_INIT()
{
    SDR_INIT();
    Memory_INIT(); //clear data structures
    Event_INIT(); //reset base id counter
    currentTime = CONCEPT_WAIT_TIME+1; //reset time
}

void ANSNA_Cycles(int cycles)
{
    for(int i=0; i<cycles; i++)
    {
        IN_DEBUG( printf("\nNew inference cycle:\n----------\n"); )
        cycle(currentTime);
        currentTime++;
    }
}

void ANSNA_AddInput(SDR sdr, char type, Truth truth)
{
    Event ev = Event_InputEvent(sdr, type, truth, currentTime);
    Memory_addEvent(&ev);
    IN_OUTPUT( printf("INPUT EVENT"); Event_Print(&ev); )
}

void ANSNA_AddOperation(SDR *sdr, Action procedure)
{
    Memory_addOperation((Operation) {.sdr = *sdr, .action = procedure});
}
