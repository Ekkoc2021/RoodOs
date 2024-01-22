
#ifndef KERNEL_H
#define KERNEL_H
#define kernelStart
#include "memory.h"
#include "process.h"
#include "io.h"
#include "types.h"
typedef struct
{
    uint32_t low;
    uint32_t hight;
} SegmentDescriptor;
typedef struct __attribute__((packed))
{
    uint16_t limit; // 占用字节数-1
    SegmentDescriptor *sd;
} GDT;
typedef struct
{
    char *version;
    char *name;
    uint32_t virtualAddr;
    uint32_t physicalAddr;
    memoryMarket *market;
    TSS *tss;
    GDT *gdt;
} kernel;

#endif