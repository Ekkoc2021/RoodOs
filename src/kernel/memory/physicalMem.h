#ifndef PHYMEM_H
#define PHYMEM_H
#include "../include/types.h"
#include "../include/bitmap.h"
#include "../include/io.h"
#include "../include/str.h"
#include "../include/assert.h"
#include "../include/arrayList.h"
#include "../include/roodos.h"
#define START 0xc0000000
#define OFFSET 0x43000
// 4GB映射到bitmap最多128KB
typedef struct
{
    uint64_t addr; /* start of memory segment */
    uint64_t size; /* size of memory segment */
    uint32_t type; /* type of memory segment */
} e820entry;

typedef struct
{
    uint32_t length;
    e820entry *entries;
} e820map;

//-------存放可用内存------
typedef struct
{
    uint32_t addr;
    uint32_t memSize;
    uint32_t bitSize;
    uint32_t bitMinIndex;
    uint32_t bitMaxIndex;
} block;

typedef struct
{
    uint16_t size;
    block *memBlocks;
} availableMem;

typedef struct
{
    uint32_t startIndexOfBitmap;
    Bitmap indexBit;
} indexNode;

// 根据内存块映射内存
typedef struct
{
    availableMem *e;
    arrayList indexList;
    Bitmap map; // 内存映射集合
} memPool;

void showIndexNode(indexNode *tempP);
void showPool(memPool *p);
memPool *initPhysicalMem(e820map *e);
char compareTo(void *A, void *B);
uint32_t phyAddrToIndexList(memPool *p, uint32_t bitmapIndex);
uint32_t getPhyPage(memPool *p);
uint32_t phyAddrToMemBlocks(memPool *p, uint32_t phyAdd);
uint32_t ReturnPhyPage(memPool *p, uint32_t phyAddr);
uint32_t initKernelPhysicalMem(memPool *phyPool);
#endif