/* Include-file defining elementary data types used by the operating system */
#ifndef __BS_TYPES__
#define __BS_TYPES__

/* ---------------------------------------------------------------- */
/*                Start Teil vom Herrn Blaurock                    */
/* ---------------------------------------------------------------- */

typedef enum { FALSE = 0, TRUE } Boolean;

typedef unsigned int pid_t;

/* data type for the possible types of processes */
/* the process type determines the IO-characteristic */
typedef enum
{
    os, interactive, batch, background, foreground
} ProcessType_t;

/* data type for the process-states used for scheduling and life-cycle management */
typedef enum
{
    init, running, ready, blocked, ended
} Status_t;

/* data type for the different events that cause the scheduler to become active */
typedef enum
{
    completed, io, quantumOver, start, none
} SchedulingEvent_t;

/* ---------------------------------------------------------------- */
/*                Ende Teil vom Herrn Blaurock                    */
/* ---------------------------------------------------------------- */


/* ---------------------------------------------------------------- */
/*                Start Teil von uns                                  */
/* ---------------------------------------------------------------- */
typedef struct WaitQueueElement {
    pid_t pid;                          // PID des wartenden Prozesses
    struct WaitQueueElement* next;      // Nächstes Element in der Warteschlange
} WaitQueueElement_t;

// Warteschlangen-Struktur
typedef struct {
    WaitQueueElement_t* first;          // Erstes Element in der Warteschlange
    WaitQueueElement_t* last;           // Letztes Element in der Warteschlange
    unsigned count;                      // Anzahl der Elemente in der Warteschlange
} WaitQueue_t;

// Struktur für die Speicherverwaltung
typedef struct MemoryBlock {
    unsigned address;       // Physische Startadresse des Blocks
    unsigned size;         // Größe des Blocks
    Boolean isFree;        // Ist der Block frei?
    pid_t pid;            // Prozess-ID falls belegt
    struct MemoryBlock* next;  // Nächster Block in der Liste
} MemoryBlock_t;

/* ---------------------------------------------------------------- */
/*                Ende Teil von uns                                  */
/* ---------------------------------------------------------------- */
/* ---------------------------------------------------------------- */
/*                Start Teil vom Herrn Blaurock                    */
/* ---------------------------------------------------------------- */
/* data type for the Process Control Block */
typedef struct
{
    Boolean valid;           // Gibt an ob der PCB-Eintrag gültig ist
    pid_t pid;              // Process ID
    pid_t ppid;             // Parent Process ID
    unsigned ownerID;       // Besitzer des Prozesses
    unsigned start;         // Startzeitpunkt
    unsigned duration;      // Prozessdauer
    unsigned size;          // Benötigte Speichergröße
    unsigned usedCPU;       // Bereits verbrauchte CPU-Zeit
    ProcessType_t type;     // Prozesstyp
    Status_t status;        // Aktueller Prozessstatus

/* ---------------------------------------------------------------- */
/*                Ende Teil vom Herrn Blaurock                    */
/* ---------------------------------------------------------------- */
/* ---------------------------------------------------------------- */
/*                Start Teil von uns                                  */
/* ---------------------------------------------------------------- */

    // Neue Felder für Speicherverwaltung
    unsigned baseAddress;   // Basisadresse im physischen Speicher
    Boolean isInMemory;     // Ist der Prozess im Speicher?
    MemoryBlock_t* memBlock; // Zeiger auf zugehörigen Speicherblock
} PCB_t;

// Struktur für die Speicherverwaltung
typedef struct {
    MemoryBlock_t* firstBlock;  // Erster Block in der Speicherliste
    unsigned totalMemory;       // Gesamter verfügbarer Speicher
    unsigned freeMemory;        // Aktuell freier Speicher
} MemoryManager_t;

/* ---------------------------------------------------------------- */
/*                Ende Teil von uns                                  */
/* ---------------------------------------------------------------- */
#endif  /* __BS_TYPES__ */