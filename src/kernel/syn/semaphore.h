#ifndef SEMAPHORE_ROODOS
#define SEMAPHORE_ROODOS
#include "../include/types.h"
#include "../include/linkedQueue.h"
#include "../include/process.h"
#define SEMSIZE 1024
typedef struct
{
    bool _pshared;
    uint16_t value;
    uint32_t pcb;
    linkedQueue block;
} sem_t;
#endif