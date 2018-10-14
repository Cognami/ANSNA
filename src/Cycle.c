#include "Cycle.h"

void composition(Concept *B, Concept *A, Event *b, long currentTime)
{
    //temporal induction and intersection
    if(b->type == EVENT_TYPE_BELIEF && A->event_beliefs.itemsAmount > 0)
    {
        Event *a = &A->event_beliefs.array[0]; //most recent, highest revised
        if(!Stamp_checkOverlap(&a->stamp, &b->stamp))
        {
            Implication implication = Inference_BeliefInduction(a, b);
            Table_AddAndRevise(&B->precondition_beliefs, &implication);
            Table_AddAndRevise(&A->postcondition_beliefs, &implication);
            Event sequence = b->occurrenceTime > a->occurrenceTime ? Inference_BeliefIntersection(a, b) : Inference_BeliefIntersection(b, a);
            sequence.attention = Attention_deriveEvent(&B->attention, &sequence.truth, currentTime);
            Memory_addEvent(&sequence);
        }
    }
}

void decomposition(Concept *c, Event *e, long currentTime)
{
    //detachment
    if(c->postcondition_beliefs.itemsAmount>0)
    {
        Implication postcon = c->postcondition_beliefs.array[0];
        if(!Stamp_checkOverlap(&e->stamp, &postcon.stamp))
        {
            Event res = e->type == EVENT_TYPE_BELIEF ? Inference_BeliefDeduction(e, &postcon) : Inference_GoalAbduction(e, &postcon);
            res.attention = Attention_deriveEvent(&c->attention, &postcon.truth, currentTime);
            Memory_addEvent(&res);
            //add negative evidence to the used predictive hypothesis (assumption of failure, for extinction)
            Table_PopHighestTruthExpectationElement(&c->postcondition_beliefs);
            Implication updated = Inference_AssumptionOfFailure(&postcon);
            Table_Add(&c->postcondition_beliefs, &updated);
        }
    }
    if(c->precondition_beliefs.itemsAmount>0)
    {
        Implication precon = c->precondition_beliefs.array[0];
        if(!Stamp_checkOverlap(&e->stamp, &precon.stamp))
        {
            Event res = e->type == EVENT_TYPE_BELIEF ? Inference_BeliefAbduction(e, &precon) : Inference_GoalDeduction(e, &precon);
            res.attention = Attention_deriveEvent(&c->attention, &precon.truth, currentTime);
            Memory_addEvent(&res);
        }
    }
}

//Lazy relative forgetting, making sure the first element also deserves to be the first
Item popAndForget(PriorityQueue *queue, bool isConcept, long currentTime)
{
    //1. pop an item
    Item item = PriorityQueue_PopMax(queue);
    //2. forget item the more the longer it wasn't last forgotten
    double priority = 0;
    if(isConcept)
    {
        Concept *c = (Concept*) item.address;
        Attention ret = Attention_forgetConcept(&c->attention, &c->usage, currentTime);
        priority = ret.priority;
    }
    else
    {
        Event *e = (Event*) item.address;
        Attention ret = Attention_forgetEvent(&e->attention, currentTime);
        priority = ret.priority;
    }
    //3. push it again
    PriorityQueue_Push_Feedback feedback = PriorityQueue_Push(queue, priority);
    if(!feedback.added)
    {
        return (Item) {0};
    }
    feedback.addedItem.address = item.address;
    //4. check if the pushed item is still considered the Max
    Item item2 = PriorityQueue_PopMax(queue);
    if(item2.address != item.address)
    { //item did not deserve to be the highest, it just wasn't selected
        return (Item) {0}; //to be relative forgotten for some time
    }
    return item;
}

void cycle(long currentTime)
{
    for(int i=0; i<EVENT_SELECTIONS; i++)
    {
        //1. get an event from the event queue
        Item item = popAndForget(&events, false, currentTime);
        if(item.address == 0)
        {
            continue;
        }
        Event *e = item.address;
        //end pop forget push pop strategu
        //determine the concept it is related to
        int closest_concept_i = Memory_getClosestConcept(e);
        if(closest_concept_i != MEMORY_MATCH_NO_CONCEPT)
        {
            Concept *c = concepts.items[closest_concept_i].address;
            c->usage = Usage_use(&c->usage, currentTime);
            Truth matchTruth = SDR_Inheritance(&e->sdr, &c->sdr);
            Event eMatch = *e;
            eMatch.truth = Truth_Revision(e->truth, matchTruth);
            //apply decomposition-based inference: prediction/explanation
            decomposition(c, &eMatch, currentTime);
            //add event to the FIFO of the concept
            FIFO *fifo =  e->type == EVENT_TYPE_BELIEF ? &c->event_beliefs : &c->event_goals;
            Event revised = FIFO_AddAndRevise(&eMatch, fifo);
            if(revised.type != EVENT_TYPE_DELETED)
            {
                Memory_addEvent(&revised);
            }
            //relatively forget the event, as it was used, and add back to events
            e->attention = Attention_forgetEvent(&e->attention, currentTime);
            Memory_addEvent(e);
            //trigger composition-based inference hypothesis formation
            Item selectedItem[CONCEPT_SELECTIONS];
            int conceptsSelected = 0;
            for(int j=0; j<CONCEPT_SELECTIONS; j++)
            {   //by doing inference with the highest priority concepts
                selectedItem[j] = popAndForget(&concepts, true, currentTime);
                if(selectedItem[j].address == 0)
                {
                    continue;
                }
                composition(c, selectedItem[j].address, &eMatch, currentTime); // deriving a =/> b
                conceptsSelected++;
            }
            for(int j=0; j<conceptsSelected; j++)
            {   //push again what we popped
                PriorityQueue_Push_Feedback feedback = PriorityQueue_Push(&concepts, selectedItem[j].priority);
                feedback.addedItem.address = selectedItem[j].address;
            }
            //activate concepts attention with the event's attention
            c->attention = Attention_activateConcept(&c->attention, &e->attention); 
            PriorityQueue_IncreasePriority(&concepts,closest_concept_i, c->attention.priority); //priority was increased
        }
        //add a new concept for e too at the end, just before it needs to be identified with something existing
        Concept *eNativeConcept = Memory_addConcept(&e->sdr, e->attention);
        FIFO_Add(e, (e->type == EVENT_TYPE_BELIEF ? &eNativeConcept->event_beliefs : &eNativeConcept->event_goals));
    }
}
