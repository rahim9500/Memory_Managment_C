/* Implementation of core functionality of the OS					*/
/* this includes the main scheduling loop							*/
/* for comments on the functions see the associated .h-file */

/* ---------------------------------------------------------------- */
/* Include required external definitions */
#include <math.h>
#include <time.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "bs_types.h"
#include "globals.h"
#include "loader.h"
#include "core.h"
#include "executer.h"
#include "memory_manager.h"
#include "wait_queue.h"



/* ----------------------------------------------------------------	*/
/* Declarations of global variables visible only in this file 		*/

PCB_t process;		// the only user process used for batch and FCFS
PCB_t* pNewProcess;	// pointer for new process read from batch

/* ---------------------------------------------------------------- */
/*                Declarations of local helper functions            */


/* ---------------------------------------------------------------- */
/*                Externally available functions                    */
/* ---------------------------------------------------------------- */

void initOS(void)
{

    unsigned i;					// iteration variable

    /* init the status of the OS */
    // mark all process entries invalid
    for (i = 0; i < MAX_PROCESSES; i++) processTable[i].valid = FALSE;
    process.pid = 0;			// reset pid
}

/* ---------------------------------------------------------------- */
/*                coreloop ein Teil vom Herrn Blaurock              */
/* ---------------------------------------------------------------- */

// Angepasste Core Loop mit verbesserter Warteschlangenlogik
void coreLoop(void) {
    SchedulingEvent_t nextEvent;
    unsigned int eventPid;
    PCB_t* candidateProcess = NULL;
    pid_t newPid;
    PCB_t* nextReady = NULL;
    unsigned delta = 0;
    Boolean isLaunchable = FALSE;

    initializeMemory();
    initWaitQueue();

    do {
        do {
            // 1. checkForProcess inBatch()
            if (!checkForProcessInBatch()) {
                continue;
            }

            // 2. isNewProcess Ready
            if (!isNewProcessReady()) {
                isLaunchable = FALSE;
                logGeneric("Process not yet ready to run");
                continue;
            }

            isLaunchable = TRUE;
            newPid = getNextPid();
            initNewProcess(newPid, getNewPCBptr());

            // 3. Process in Warteschlange
            if (waitQueue.count > 0) {
                // Versuche zuerst wartende Prozesse zu starten
                PCB_t* waitingProcess = &processTable[waitQueue.first->pid];
                if (memoryManager.freeMemory >= waitingProcess->size) {
                    if (findFreeBlock(waitingProcess->size) == NULL &&
                        needsCompaction(waitingProcess->size)) {
                        compactMemory();
                    }
                    if (allocateMemory(waitingProcess)) {
                        waitingProcess->status = running;
                        runningCount++;
                        removeFromWaitQueue();
                        systemTime = systemTime + LOADING_DURATION;
                        logPidMem(waitingProcess->pid, "Waiting process started");
                        continue;
                    }
                }
            }

           
            PCB_t* newProcess = &processTable[newPid];

            // 4. Prüfen ob Prozess zu groß ist
            if (newProcess->size > MEMORY_SIZE) {
                newProcess->status = blocked;
                logPidMem(newProcess->pid, "Process blocked and get deleted - too large");
                deleteProcess(newProcess);
                continue;
            }

            // 5. Prüfen ob genügend Speicher verfügbar ist
            if (memoryManager.freeMemory >= newProcess->size) {
                // 6. Kompaktierung wenn nötig
                if (findFreeBlock(newProcess->size) == NULL) {
                    if (needsCompaction(newProcess->size)) {
                        compactMemory();
                    }
                }

                // Speicher zuweisen
                if (allocateMemory(newProcess)) {
                    newProcess->status = running;
                    runningCount++;
                    systemTime = systemTime + LOADING_DURATION;
                    logPidMem(newProcess->pid, "Memory allocated");
                    flagNewProcessStarted();
                }
            }
            else {
                // Nicht genug Speicher verfügbar
                newProcess->status = blocked;
                addToWaitQueue(newProcess->pid);
                logPidMem(newProcess->pid, "Process blocked - added to wait queue (not enough memory)");
            }

        } while ((!batchComplete) && (isLaunchable));

        // Prozessausführung und -beendigung
        if (runningCount > 0) {
            delta = runToNextEvent(&nextEvent, &eventPid);
            updateAllVirtualTimes(delta);
            systemTime = systemTime + delta;

            if (nextEvent == completed) {
                PCB_t* process = &processTable[eventPid];
                deallocateMemory(process);
                logPidMem(eventPid, "Process completed and terminated, memory freed");
                deleteProcess(process);
                runningCount--;

                // Nach Speicherfreigabe: Versuche wartende Prozesse zu starten
                tryStartWaitingProcesses();
            }
        }

    } while ((runningCount > 0) || (batchComplete == FALSE));

    logGeneric("All processes completed, shutting down");
}

/* ---------------------------------------------------------------- */
/*                Start Teil vom Herrn Blaurock                    */
/* ---------------------------------------------------------------- */
unsigned getNextPid()
{
    static unsigned pidCounter = 1;
    unsigned i = 0;		// iteration variable;
    // determine next available pid make sure not to search infinitely
    while ((processTable[pidCounter].valid) && (i < MAX_PID))
    {
        // determine next available pid 
        pidCounter = (pidCounter + 1) % MAX_PID;
        if (pidCounter == 0) pidCounter++;	// pid=0 is invalid
        i++;		// count the checked entries
    }
    if (i == MAX_PID) return 0;			// indicate error
    else		   return pidCounter;
}

int initNewProcess(pid_t newPid, PCB_t* pProcess)
/* Initialised the PCB at the given index of the process table with the		*/
/* process information provided in the PCB-struct giben by the pointer		*/
{
    if (pProcess == NULL)
        return 0;
    else {	/* PCB correctly passed, now initialise it */
        processTable[newPid].pid = newPid;
        processTable[newPid].ppid = pProcess->ppid;
        processTable[newPid].ownerID = pProcess->ownerID;
        processTable[newPid].start = pProcess->start;
        processTable[newPid].duration = pProcess->duration;
        processTable[newPid].size = pProcess->size;
        processTable[newPid].usedCPU = pProcess->usedCPU;
        processTable[newPid].type = pProcess->type;
        processTable[newPid].status = init;
        processTable[newPid].valid = TRUE;
        // new process is initialised, now invalidate the PCB passed to this funtion
        pProcess->valid = FALSE;
        return 1;
    }
}

int deleteProcess(PCB_t* pProcess)
/* Voids the PCB handed over in pProcess, this includes setting the valid-	*/
/* flag to invalid and setting other values to invalid values.				*/
/* retuns 0 on error and 1 on success										*/
{
    if (pProcess == NULL)
        return 0;
    else {	/* PCB correctly passed, now delete it */
        pProcess->valid = FALSE;
        pProcess->pid = 0;
        pProcess->ppid = 0;
        pProcess->ownerID = 0;
        pProcess->start = 0;
        pProcess->duration = 0;
        pProcess->size = 0;
        pProcess->usedCPU = 0;
        pProcess->type = os;
        pProcess->status = ended;
        return 1;
    }
}

/* ---------------------------------------------------------------- */
/*                Ende Teil vom Herrn Blaurock                    */
/* ---------------------------------------------------------------- */



/* ----------------------------------------------------------------- */
/*                       Local helper functions                      */
/* ----------------------------------------------------------------- */


