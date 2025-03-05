/* memory_manager.c */

/* ---------------------------------------------------------------- */
/*                Komplett von uns                                  */
/* ---------------------------------------------------------------- */

#include "memory_manager.h"
#include "globals.h"
#include "log.h"
#include <stdio.h>
#include <stdlib.h>

MemoryManager_t memoryManager;
Boolean isMemoryInitialized = FALSE;

void initializeMemory() {
    if (!isMemoryInitialized) {
        MemoryBlock_t* initialBlock = (MemoryBlock_t*)malloc(sizeof(MemoryBlock_t));
        initialBlock->address = 0;
        initialBlock->size = MEMORY_SIZE;
        initialBlock->isFree = TRUE;
        initialBlock->pid = 0;
        initialBlock->next = NULL;

        memoryManager.firstBlock = initialBlock;
        memoryManager.totalMemory = MEMORY_SIZE;
        memoryManager.freeMemory = MEMORY_SIZE;

        isMemoryInitialized = TRUE;
    }
}

MemoryBlock_t* findFreeBlock(unsigned size) {
    MemoryBlock_t* current = memoryManager.firstBlock;

    while (current != NULL) {
        if (current->isFree && current->size >= size) {
            return current;
        }
        current = current->next;
    }
    return NULL;
}

void splitBlock(MemoryBlock_t* block, unsigned size) {
    if (block->size > size) {
        MemoryBlock_t* newBlock = (MemoryBlock_t*)malloc(sizeof(MemoryBlock_t));
        newBlock->address = block->address + size;
        newBlock->size = block->size - size;
        newBlock->isFree = TRUE;
        newBlock->pid = 0;
        newBlock->next = block->next;

        block->size = size;
        block->next = newBlock;
    }
}

Boolean allocateMemory(PCB_t* process) {
    if (!isMemoryInitialized) {
        initializeMemory();
    }

    if (process->size > memoryManager.freeMemory) {
        return FALSE;
    }

    MemoryBlock_t* block = findFreeBlock(process->size);
    if (block == NULL) {
        return FALSE;
    }

    splitBlock(block, process->size);

    block->isFree = FALSE;
    block->pid = process->pid;

    process->baseAddress = block->address;
    process->isInMemory = TRUE;
    process->memBlock = block;

    memoryManager.freeMemory -= process->size;
    usedMemory += process->size;

    printf("\nAfter allocating memory for PID %u:\n", process->pid);
    printMemoryState();

    return TRUE;
}

void deallocateMemory(PCB_t* process) {
    if (!process->isInMemory || !process->memBlock) {
        return;
    }

    MemoryBlock_t* block = process->memBlock;
    memoryManager.freeMemory += process->size;
    usedMemory -= process->size;

    block->isFree = TRUE;
    block->pid = 0;

    process->isInMemory = FALSE;
    process->memBlock = NULL;
    process->baseAddress = 0;

    mergeAdjacentFreeBlocks();

    printf("\nAfter freeing memory from PID %u:\n", process->pid);
    printMemoryState();
}

void mergeAdjacentFreeBlocks() {
    Boolean merged;
    do {
        merged = FALSE;
        MemoryBlock_t* current = memoryManager.firstBlock;

        while (current != NULL && current->next != NULL) {
            if (current->isFree && current->next->isFree) {
                MemoryBlock_t* nextBlock = current->next;
                current->size += nextBlock->size;
                current->next = nextBlock->next;
                free(nextBlock);
                merged = TRUE;
                continue;
            }
            current = current->next;
        }
    } while (merged);
}

Boolean needsCompaction(unsigned requiredSize) {
    if (memoryManager.freeMemory < requiredSize) {
        return FALSE;
    }

    MemoryBlock_t* current = memoryManager.firstBlock;
    unsigned largestFreeBlock = 0;

    while (current != NULL) {
        if (current->isFree && current->size > largestFreeBlock) {
            largestFreeBlock = current->size;
        }
        current = current->next;
    }

    return (largestFreeBlock < requiredSize);
}

void compactMemory() {
    printf("\nBefore compaction:\n");
    printMemoryState();

    MemoryBlock_t* blocks[MAX_PROCESSES];
    int blockCount = 0;

    MemoryBlock_t* current = memoryManager.firstBlock;
    while (current != NULL) {
        if (!current->isFree) {
            blocks[blockCount] = malloc(sizeof(MemoryBlock_t));
            *blocks[blockCount] = *current; 
            blocks[blockCount]->next = NULL;
            blockCount++;
        }
        current = current->next;
    }

    while (memoryManager.firstBlock != NULL) {
        current = memoryManager.firstBlock;
        memoryManager.firstBlock = current->next;
        free(current);
    }

    memoryManager.firstBlock = NULL;
    unsigned nextAddress = 0;

    for (int i = 0; i < blockCount; i++) {
        MemoryBlock_t** insertPos = &memoryManager.firstBlock;
        while (*insertPos != NULL && (*insertPos)->address < nextAddress) {
            insertPos = &(*insertPos)->next;
        }

        blocks[i]->address = nextAddress;
        nextAddress += blocks[i]->size;

        PCB_t* process = &processTable[blocks[i]->pid];
        if (process->valid && process->isInMemory) {
            process->baseAddress = blocks[i]->address;
            process->memBlock = blocks[i];
        }

        blocks[i]->next = *insertPos;
        *insertPos = blocks[i];
    }

    if (nextAddress < MEMORY_SIZE) {
        MemoryBlock_t* freeBlock = malloc(sizeof(MemoryBlock_t));
        freeBlock->address = nextAddress;
        freeBlock->size = MEMORY_SIZE - nextAddress;
        freeBlock->isFree = TRUE;
        freeBlock->pid = 0;

        MemoryBlock_t** insertPos = &memoryManager.firstBlock;
        while (*insertPos != NULL) {
            insertPos = &(*insertPos)->next;
        }
        *insertPos = freeBlock;
        freeBlock->next = NULL;
    }

    printf("\nAfter compaction:\n");
    printMemoryState();
}