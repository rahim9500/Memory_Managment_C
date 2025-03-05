/* wait_queue.c */


/* ---------------------------------------------------------------- */
/*                Komplett von uns                                  */
/* ---------------------------------------------------------------- */

#include "wait_queue.h"
#include "globals.h"
#include "memory_manager.h"
#include "log.h"
#include <stdlib.h>

WaitQueue_t waitQueue;

void initWaitQueue() {
    waitQueue.first = NULL;
    waitQueue.last = NULL;
    waitQueue.count = 0;
}

void addToWaitQueue(pid_t pid) {
    WaitQueueElement_t* newElement = (WaitQueueElement_t*)malloc(sizeof(WaitQueueElement_t));
    newElement->pid = pid;
    newElement->next = NULL;

    if (waitQueue.count == 0) {
        waitQueue.first = newElement;
        waitQueue.last = newElement;
    }
    else {
        waitQueue.last->next = newElement;
        waitQueue.last = newElement;
    }
    waitQueue.count++;
    logPid(pid, "Added to wait queue");
}

pid_t removeFromWaitQueue() {
    if (waitQueue.count == 0) {
        return 0;
    }

    WaitQueueElement_t* element = waitQueue.first;
    pid_t pid = element->pid;

    waitQueue.first = element->next;
    waitQueue.count--;

    if (waitQueue.count == 0) {
        waitQueue.last = NULL;
    }

    free(element);
    logPid(pid, "Removed from wait queue");
    return pid;
}

Boolean isInWaitQueue(pid_t pid) {
    WaitQueueElement_t* current = waitQueue.first;
    while (current != NULL) {
        if (current->pid == pid) {
            return TRUE;
        }
        current = current->next;
    }
    return FALSE;
}

void tryStartWaitingProcesses() {
    if (waitQueue.count == 0) {
        return;
    }

    WaitQueueElement_t* current = waitQueue.first;
    WaitQueueElement_t* prev = NULL;

    while (current != NULL) {
        PCB_t* process = &processTable[current->pid];

        if (memoryManager.freeMemory >= process->size) {
            if (findFreeBlock(process->size) == NULL &&
                needsCompaction(process->size)) {
                compactMemory();
            }
        }

        if (allocateMemory(process)) {
            pid_t pid = current->pid;
            process->status = running;
            runningCount++;
            systemTime = systemTime + LOADING_DURATION;
            logPidMem(pid, "Process from wait queue started");

            if (prev == NULL) {
                waitQueue.first = current->next;
            }
            else {
                prev->next = current->next;
            }
            if (current == waitQueue.last) {
                waitQueue.last = prev;
            }
            WaitQueueElement_t* temp = current;
            current = current->next;
            free(temp);
            waitQueue.count--;
        }
        else {
            prev = current;
            current = current->next;
        }
    }
}