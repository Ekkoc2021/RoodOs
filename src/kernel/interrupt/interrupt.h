#ifndef INTERRUPT_H
#define INTERRUPT_H
#include "../include/roodos.h"
#include "../include/stdarg.h"
#include "../include/global.h"
#include "../include/str.h"
#include "pic.h"
#include "timer.h"

#define IETSize 0x31
#define SELECTOR_GDT

typedef struct
{
    uint16_t offset_low;  // 低地址处的中断处理程序入口偏移量
    uint16_t selector;    // 中断处理程序所在代码段的段选择子
    uint8_t zero;         // 必须为0
    uint8_t type_attr;    // 中断门描述符的类型和属性
    uint16_t offset_high; // 高地址处的中断处理程序入口偏移量
} InterruptDescriptor;

uint32_t areInterruptsEnabled();
void enable_irq();
void logStackInfo(StackInfo *s);
void interruptHandler(uint32_t IVN, ...);
void init_idt_desc(InterruptDescriptor *interruptDes, uint8_t attr, void *function);
InterruptDescriptor *interruptInit();

#endif