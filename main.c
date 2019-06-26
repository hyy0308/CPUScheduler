#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <sys/param.h>
#include <assert.h>
#include <time.h>

#define NONE 0
#define SJF 1
#define FCFS 2
#define RR 3
#define QUANTUM 4
typedef struct {
    char name[16];
} DataNode;

typedef struct PQN {
    int priority;
    void *data;
    struct PQN *next;
} PQueueNode;

typedef struct {
    PQueueNode *head;
    PQueueNode *tail;
} PQueue;

/*--------------------------------------------------------------------*/
/* insert at tail */

int enqueue(PQueue *pqueue, int priority, void *data);

/*--------------------------------------------------------------------*/

int printQueue(PQueue *pqueue);

/*--------------------------------------------------------------------*/
/* remove from head */

void *dequeue(PQueue *pqueue);

/*--------------------------------------------------------------------*/
/* peek at head node */

void *peek(PQueue *pqueue);

/*--------------------------------------------------------------------*/
/* get priority of head node */

int getMinPriority(PQueue *pqueue);

int enqueue(PQueue *pqueue, int priority, void *data) {
    PQueueNode *queueNode, *newNode, *prevNode;

    newNode = (PQueueNode *) malloc(sizeof(PQueueNode));
    newNode->next = NULL;
    newNode->priority = priority;
    newNode->data = data;

    if (pqueue->head == NULL) {
        pqueue->head = newNode;
        pqueue->tail = newNode;
    } else {
        if (priority < pqueue->head->priority) {
            /* insert at front */
            newNode->next = pqueue->head;
            pqueue->head = newNode;
        } else if (priority >= pqueue->tail->priority) {
            /* insert at end */
            pqueue->tail->next = newNode;
            pqueue->tail = newNode;
        } else {
            prevNode = NULL;
            queueNode = pqueue->head;
            while (queueNode != pqueue->tail && priority >= queueNode->priority) {
                prevNode = queueNode;
                queueNode = queueNode->next;
            }

            assert(prevNode != NULL);

            /* insert in middle */
            prevNode->next = newNode;
            newNode->next = queueNode;
        }
    }
    return(0);
}

/*--------------------------------------------------------------------*/

int printQueue(PQueue *pqueue) {
    PQueueNode *queueNode;
    DataNode *dataNode;

    printf("|");
    queueNode = pqueue->head;
    while (queueNode != NULL) {
        dataNode = (DataNode *) queueNode->data;
        printf("%d %s|", queueNode->priority, dataNode->name);
        queueNode = queueNode->next;
    }
    if (pqueue->head == NULL)
        printf("|");
    printf("\n");
    return(0);
}

/*--------------------------------------------------------------------*/
/* remove from head */

void *dequeue(PQueue *pqueue) {
    PQueueNode *queueNode;
    void *rtnval;

    if (pqueue->head == NULL) {
        return(NULL);
    } else {
        queueNode = pqueue->head;
        pqueue->head = queueNode->next;
        if (pqueue->head == NULL)
            pqueue->tail = NULL;
        //queueNode->next = NULL;
        rtnval = queueNode->data;
        free(queueNode);
        return(rtnval);
    }
}

/*--------------------------------------------------------------------*/
/* peek at head */

void *peek(PQueue *pqueue) {
    if (pqueue->head != NULL)
        return(pqueue->head->data);
    else
        return(NULL);
}

//--------------------------------------------------------------------

int getMinPriority(PQueue *pqueue) {
    if (pqueue->head != NULL)
        return(pqueue->head->priority);
    else
        return(-1);
}
typedef struct {
    int pid;
    int burstTime;
    int waitTime;
    int numPreemptions;
    int lastTime;
} Process;

typedef enum EventTypeEnum {
    PROCESS_SUBMITTED,
    PROCESS_STARTS,
    PROCESS_ENDS,
    PROCESS_TIMESLICE_EXPIRES
} EventType;

typedef struct {
    EventType eventType;
    Process *process;
} Event;

int totalWaitTime;

//----------------------------------------------------------------

void print_event(Event *event) {
    if (event->eventType == PROCESS_SUBMITTED)
        printf(" PROCESS_SUBMITTED");
    else if (event->eventType == PROCESS_STARTS)
        printf(" PROCESS_STARTS");
    else if (event->eventType == PROCESS_ENDS)
        printf(" PROCESS_ENDS");
    else if (event->eventType == PROCESS_TIMESLICE_EXPIRES)
        printf(" PROCESS_TIMESLICE_EXPIRES");
    printf(" pid=%d", event->process->pid);
    if (event->eventType == PROCESS_ENDS)
        printf(" wt=%d", event->process->waitTime);
    printf(" ---\n");
}

//----------------------------------------------------------------
const char* getEventName(enum EventTypeEnum eventType){
    switch (eventType) {
        case PROCESS_STARTS: return "PROCESS_STARTS";
        case PROCESS_ENDS: return "PROCESS_ENDS";
        case PROCESS_SUBMITTED: return "PROCESS_SUBMITTED";
        case PROCESS_TIMESLICE_EXPIRES: return "PROCESS_TIMESLICE_EXPIRES";
    }
}
void printEventWait(Event *event, int time){
    if(event->eventType == PROCESS_ENDS){
        printf("t = %d %s pid = %d wait time = %d\n",time, getEventName(event->eventType),event->process->pid, event->process->waitTime);
    }else{
        printf("t = %d %s pid = %d\n",time, getEventName(event->eventType),event->process->pid);
    }
}
void runSimulation(
        int schedulerType,
        PQueue eventQueue,
        PQueue cpuQueue) {
    int currentTime;
    Event *event, *endEvent;
    Process *process;
    int active_pid;
    int readyQueueLength;

    active_pid = -1;
    currentTime = getMinPriority(&eventQueue);

    event = (Event *) dequeue(&eventQueue);
    while (event != NULL) {
        // printf("--- time=%d", currentTime);

        //print_event(event);
        printEventWait(event,currentTime);

        if (event->eventType == PROCESS_SUBMITTED) {
            process = event->process;

            // printf("at time %d, enqueue process %d\n", currentTime, process->pid);

            process->lastTime = currentTime;

            if (active_pid == -1) {
                Event *newEvent = (Event *) malloc(sizeof(Event));
                newEvent->process = process;
                newEvent->eventType = PROCESS_STARTS;
                //printf("new event at %d: PROCESS_STARTS(%d) pid=%d\n", currentTime, newEvent->eventType, process->pid);
                enqueue(&eventQueue, currentTime, newEvent);
                active_pid = process->pid;
            } else {
                // put process in CPU queue (= ready state)
                if (schedulerType == FCFS) {
                    enqueue(&cpuQueue, 0, process);
                } else if (schedulerType == SJF) {
                    enqueue(&cpuQueue, process->burstTime, process);
                } else if (schedulerType == RR) {
                    enqueue(&cpuQueue, 0, event->process);
                }
            } // if-then-else for cpu is idle
        } // PROCESS_SUBMITTED

        else if (event->eventType == PROCESS_ENDS) {
            active_pid = -1;
            process = event->process;
//            printf("process %d finishes at %d, waitTime=%d\n", process->pid, currentTime, process->waitTime);
//            printf("* P%d finishes at %d\n", process->pid, currentTime);
            totalWaitTime = totalWaitTime + process->waitTime;

            // check cpu queue
            process = (Process *) dequeue(&cpuQueue);
            if (process != NULL) {
                endEvent = (Event *) malloc(sizeof(Event));
                endEvent->process = process;
                endEvent->eventType = PROCESS_STARTS;
                //printf("new event at %d: PROCESS_STARTS(%d) pid=%d\n", currentTime, endEvent->eventType, process->pid);
                enqueue(&eventQueue, currentTime, endEvent);
            }
        } // PROCESS_ENDS

        else if (event->eventType == PROCESS_TIMESLICE_EXPIRES) {
            event->process->burstTime  = event->process->burstTime- QUANTUM;
            enqueue(&cpuQueue, 0, event->process);
            if(cpuQueue.head != NULL){
                Process *newProcess = (Process *) dequeue(&cpuQueue);
                Event *newEvent = (Event *) malloc(sizeof(Event));
                newEvent->eventType = PROCESS_STARTS;
                newEvent->process = newProcess;
                enqueue(&eventQueue, currentTime, newEvent);
            }
        } // PROCESS_TIMESLICE_EXPIRES

        else if (event->eventType == PROCESS_STARTS) {
            process = event->process;
            process->waitTime = process->waitTime + (currentTime - process->lastTime);
//            printf("* P%d starts at %d (wt=%d, %d)\n",
//                   process->pid, currentTime,
//                   currentTime - process->lastTime, process->waitTime);

            endEvent = (Event *) malloc(sizeof(Event));
            endEvent->process = process;

            if (schedulerType == RR) {
                if(event->process->burstTime > QUANTUM){
                    Event *newEvent;
                    newEvent = (Event *) malloc(sizeof(Event));
                    newEvent->eventType = PROCESS_TIMESLICE_EXPIRES;
                    newEvent->process = event->process;
                    enqueue(&eventQueue, currentTime+QUANTUM, newEvent);
                }else{
                    Event *newEvent;
                    newEvent = (Event *) malloc(sizeof(Event));
                    newEvent->eventType = PROCESS_ENDS;
                    newEvent->process = event->process;
                    enqueue(&eventQueue, currentTime+event->process->burstTime, newEvent);
                    active_pid = process->pid;
                }
            } else {
                endEvent->eventType = PROCESS_ENDS;
                //printf("new event at %d: PROCESS_ENDS(%d) pid=%d\n", currentTime + process->burstTime, endEvent->eventType, process->pid);
                enqueue(&eventQueue, currentTime + process->burstTime, endEvent);
            }

            active_pid = process->pid;
        } // PROCESS_STARTS

        free(event);
        currentTime = getMinPriority(&eventQueue);
        event = dequeue(&eventQueue);
    } /* while event queue is not empty */

} // runSimulation()

//---------------------------------------------------------------------
//int gen_exprand(double mean) {
//    double r, t;
//    int rtnval;
//    r = drand48();
//    t = -log(1-r) * (mean - 0.5);
//    rtnval = 1 + (int) floor(t);
//    return(rtnval);
//}
int main(int argc, char *argv[]) {
    int i;
    long seed;
    int numProcesses;
    int schedulerType;
    Process *process;
    Event *event;
    PQueue eventQueue;
    PQueue cpuQueue;

//these calls will be useful for part II
//    time(&seed);
//    srand48(seed);

    totalWaitTime = 0;

    eventQueue.head = NULL;
    eventQueue.tail = NULL;
    cpuQueue.head = NULL;
    cpuQueue.tail = NULL;

    numProcesses = 5;
    int startTimes[] = {0, 3, 4, 6, 6};
    int burstTimes[] = {6, 7, 2, 5, 2};

    for (i=0; i<numProcesses; ++i) {
        process = (Process *) malloc(sizeof(Process));
        process->pid = i+1; // start the PIDs at one instead of zero
        process->burstTime = burstTimes[i];
        process->waitTime = 0;
        process->numPreemptions = 0;
        process->lastTime = 0;

        //printf("created process %d; burstTime = %d start_time=%d\n", process->pid, process->burstTime, startTimes[i]);

        event = (Event *) malloc(sizeof(Event));
        event->eventType = PROCESS_SUBMITTED;
        event->process = process;
        enqueue(&eventQueue, startTimes[i], event);
    }
//    double proc_interarrival_time_mean = 10;
//    double proc_burst_time_mean = 5;
//    int proc_interarrival_time, t;
//    int nprocesses;
//    nprocesses = 50;
//    t = 0;
//    for (i=0; i<nprocesses; ++i) {
//        Process* proc = (Process *) malloc(sizeof(Process));
//        proc->pid = i+1; // start the process IDs at 1 instead of zero
//        proc->waitTime = 0;
//        proc->lastTime = 0;
//        proc->numPreemptions = 0;
//        proc->burstTime = gen_exprand(proc_burst_time_mean);
//
//        event = (Event *) malloc(sizeof(Event));
//        event->eventType = PROCESS_SUBMITTED;
//        event->process = proc;
//        enqueue(&eventQueue, t, event);
//        proc_interarrival_time = gen_exprand(proc_interarrival_time_mean);
//        t = t + proc_interarrival_time;
//    }

    schedulerType = RR; // or SJF

    runSimulation(schedulerType, eventQueue, cpuQueue);

    printf("\n");
    printf("total wait time = %d\n", totalWaitTime);
    printf("mean wait time = %.2f\n", (float) totalWaitTime / numProcesses);
    return(0);
}

