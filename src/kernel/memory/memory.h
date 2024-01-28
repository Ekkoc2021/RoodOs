#ifndef K_MEMORY_H
#define K_MEMORY_H
#include "../include/types.h"
#include "../include/io.h"
#include "../include/str.h"
#include "../include/roodos.h"
#include "physicalMem.h"
#include "virtualMem.h"
typedef struct
{
    memPool *phyPool;
    virtualMemPool *virMemPool;
    userPageDir *initPageDir; // 默认的用户Pd!
} memoryMarket;
memoryMarket *initMemoryManagement(uint32_t size, void *addr, void *kernelVAddr, void *pTablePhAddr);
uint32_t mallocPage_u(memoryMarket *market, uint32_t *paddr);
uint32_t mallocPage_k(memoryMarket *market, uint32_t *paddr);
void freePage(memoryMarket *market, uint32_t vAddr);
void tidy(memoryMarket *market);
uint32_t mallocMultpage_k(memoryMarket *market, uint32_t n);
uint32_t mallocMultpage_u(memoryMarket *market, uint32_t n);
void freeNPage(memoryMarket *market, uint32_t vaddr, uint32_t n);
#endif