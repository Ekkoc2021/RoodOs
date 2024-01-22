#ifndef INTERRUPT_H
#define INTERRUPT_H
#include "../include/roodos.h"
#include "../include/stdarg.h"
#include "../include/global.h"
#include "../include/str.h"
#include "pic.h"
#include "timer.h"
#define IETSize 0x21
#define SELECTOR_GDT

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
    uint32_t EFLAGS;
    uint32_t oldESP
} StackInfo;

typedef struct
{
    uint16_t offset_low;  // 低地址处的中断处理程序入口偏移量
    uint16_t selector;    // 中断处理程序所在代码段的段选择子
    uint8_t zero;         // 必须为0
    uint8_t type_attr;    // 中断门描述符的类型和属性
    uint16_t offset_high; // 高地址处的中断处理程序入口偏移量
} InterruptDescriptor;

void initIntr_name();
void disable_irq();
uint32_t areInterruptsEnabled();
void enable_irq();
void logStackInfo(StackInfo *s);
void interruptHandler(uint32_t IVN, ...);
void init_idt_desc(InterruptDescriptor *interruptDes, uint8_t attr, void *function);
InterruptDescriptor *interruptInit();

#endif