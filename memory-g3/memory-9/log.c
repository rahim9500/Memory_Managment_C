/* Implementation of the log functions */
/* for comments on the functions see the associated .h-file */

/* ---------------------------------------------------------------- */
/* Include required external definitions */
#include "globals.h"
#include <math.h>
#include "bs_types.h"
#include "log.h"
#include "memory_manager.h"


/* ---------------------------------------------------------------- */
/*                Declarations of local helper functions            */

/* ---------------------------------------------------------------- */
/* Declarations of global variables visible only in this file 		*/
// array with strings associated to scheduling events for log outputs
char eventString[3][12] = {"completed", "io", "quantumOver"};

/* ---------------------------------------------------------------- */
/*                Externally available functions					*/
/* ---------------------------------------------------------------- */

void logGeneric(char* message)
{
	printf("%6u : %s\n", systemTime, message); 
}
	
void logPid(unsigned pid, char * message)
{
	printf("%6u : PID %3u : %s\n", systemTime, pid, message); 
}
		
void logPidCompleteness(unsigned pid, unsigned done, unsigned length, 
						char * message)
{
	printf("%6u : PID %3u : completeness: %u/%u | %s\n", systemTime, 
			pid, done, length, message); 
}
		
void logPidMem(unsigned pid, char * message)
{
	printf("%6u : PID %3u : Used memory: %6u | %s\n", systemTime, 
			pid, usedMemory, message); 
}
		
void logLoadedProcessData(PCB_t* pProcess)
{
	char processTypeStr[21] = ""; 	// buffer process type-string

	switch (pProcess->type) {
	case os: 
		strcpy(processTypeStr, "os"); 
		break; 
	case interactive: 
		strcpy(processTypeStr, "interactive"); 		
		break;
	case batch:
		strcpy(processTypeStr, "batch");		
		break;
	case background:
		strcpy(processTypeStr, "background");	
		break;
	case foreground:
		strcpy(processTypeStr, "foreground");	
		break;
	default:
		strcpy(processTypeStr, "no type");	
	}
	
	printf("%6u : Sim: Loaded process properties: %u %u %u %u %s\n", systemTime,
		pProcess->ownerID, pProcess->start, pProcess->duration, 
		pProcess->size, processTypeStr);
}

// printMemoryState ist von uns der rest von Blaurock

void printMemoryState() {
	MemoryBlock_t* current = memoryManager.firstBlock;
	printf("\n=== Memory State (Total: %u, Used: %u, Free: %u) ===\n",
		memoryManager.totalMemory,
		memoryManager.totalMemory - memoryManager.freeMemory,
		memoryManager.freeMemory);

	printf("| Address | Size  | Status | PID |\n");
	printf("|---------|-------|--------|-----|\n");

	while (current != NULL) {
		printf("| %7u | %5u | %6s | %3u |\n",
			current->address,
			current->size,
			current->isFree ? "FREE" : "USED",
			current->pid);
		current = current->next;
	}
	printf("========================================\n\n");
}


/* ----------------------------------------------------------------- */
/*                       Local helper functions                      */
/* ----------------------------------------------------------------- */



