#ifndef __KERNEL_GLOBAL_H
#define __KERNEL_GLOBAL_H
#include "types.h"
#include "memory.h"
typedef struct
{
    uint32_t IVN;
    uint32_t EDI;
    uint32_t ESI;
    uint32_t EBP;
    uint32_t newESP;
    uint32_t EBX;
    uint32_t EDX;
    uint32_t ECX;
    uint32_t EAX;
    uint32_t GS;
    uint32_t FS;
    uint32_t ES;
    uint32_t DS;
    uint32_t ERROCODE;
    uint32_t EIP;
    uint32_t CS;
    uint32_t EFLAGS;
    uint32_t oldESP;
    uint32_t oldSS;
} StackInfo;

typedef struct
{
    uint32_t backlink;
    uint32_t *esp0;
    uint32_t ss0;
    uint32_t *esp1;
    uint32_t ss1;
    uint32_t *esp2;
    uint32_t ss2;
    uint32_t cr3;
    uint32_t (*eip)(void);
    uint32_t eflags;
    uint32_t eax;
    uint32_t ecx;
    uint32_t edx;
    uint32_t ebx;
    uint32_t esp;
    uint32_t ebp;
    uint32_t esi;
    uint32_t edi;
    uint32_t es;
    uint32_t cs;
    uint32_t ss;
    uint32_t ds;
    uint32_t fs;
    uint32_t gs;
    uint32_t ldt;
    uint32_t trace;
    uint32_t io_base;
} TSS;
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
#define RPL0 0
#define RPL1 1
#define RPL2 2
#define RPL3 3

#define TI_GDT 0
#define TI_LDT 1

#define SELECTOR_K_CODE ((1 << 3) + (TI_GDT << 2) + RPL0)
#define SELECTOR_K_DATA ((2 << 3) + (TI_GDT << 2) + RPL0)
#define SELECTOR_K_STACK SELECTOR_K_DATA
// #define SELECTOR_K_GS ((3 << 3) + (TI_GDT << 2) + RPL0)

//--------------   IDT描述符属性  ------------
#define IDT_DESC_P 1
#define IDT_DESC_DPL0 0
#define IDT_DESC_DPL3 3
#define IDT_DESC_32_TYPE 0xE // 32位的门
#define IDT_DESC_16_TYPE 0x6 // 16位的门，不用，定义它只为和32位门区分
#define IDT_DESC_ATTR_DPL0 ((IDT_DESC_P << 7) + (IDT_DESC_DPL0 << 5) + IDT_DESC_32_TYPE)
#define IDT_DESC_ATTR_DPL3 ((IDT_DESC_P << 7) + (IDT_DESC_DPL3 << 5) + IDT_DESC_32_TYPE)
#include "memory.h"
#include "process.h"
#include "io.h"
#include "types.h"

#endif