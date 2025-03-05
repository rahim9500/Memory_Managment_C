/* wait_queue.h */

/* ---------------------------------------------------------------- */
/*                Komplett von uns                                  */
/* ---------------------------------------------------------------- */

#ifndef __WAIT_QUEUE__
#define __WAIT_QUEUE__

#include "bs_types.h"

extern WaitQueue_t waitQueue;

void initWaitQueue(void);
void addToWaitQueue(pid_t pid);
pid_t removeFromWaitQueue(void);
Boolean isInWaitQueue(pid_t pid);
void tryStartWaitingProcesses(void);

#endif /* __WAIT_QUEUE__ */