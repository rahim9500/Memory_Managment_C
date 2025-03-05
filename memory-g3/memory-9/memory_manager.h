/* memory_manager.h */

/* ---------------------------------------------------------------- */
/*                Komplett von uns                                  */
/* ---------------------------------------------------------------- */

#ifndef __MEMORY_MANAGER__
#define __MEMORY_MANAGER__

#include "bs_types.h"

extern MemoryManager_t memoryManager;
extern Boolean isMemoryInitialized;

void initializeMemory(void);
MemoryBlock_t* findFreeBlock(unsigned size);
void splitBlock(MemoryBlock_t* block, unsigned size);
Boolean allocateMemory(PCB_t* process);
void deallocateMemory(PCB_t* process);
void mergeAdjacentFreeBlocks(void);
Boolean needsCompaction(unsigned requiredSize);
void compactMemory(void);

#endif /* __MEMORY_MANAGER__ */