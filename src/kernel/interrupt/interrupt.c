
#include "interrupt.h"

extern uint32_t intr_entry_table[IETSize]; // 中断入口表
InterruptDescriptor IDT[IETSize];          // 中断描述符表
char *intr_name[IETSize];                  // 中断名称

uint32_t i = 0;
void interruptHandler(uint32_t IVN, ...)
{
    log("interrupt test successful ! IVN = %d , time = %d\n ", IVN, i);
    if (IVN == 0x27 || IVN == 0x2f)
    {           // 0x2f是从片8259A上的最后一个irq引脚，保留
        return; // IRQ7和IRQ15会产生伪中断(spurious interrupt),无须处理。
    }
    // 通过IVN的栈针地址,定位
    va_list args;        // 定义参数
    va_start(args, IVN); // 初始化
    StackInfo s;
    s.EDI = (uint32_t)va_arg(args, uint32_t);
    s.ESI = (uint32_t)va_arg(args, uint32_t);
    s.EBP = (uint32_t)va_arg(args, uint32_t);
    s.newESP = (uint32_t)va_arg(args, uint32_t);
    s.EBX = (uint32_t)va_arg(args, uint32_t);
    s.EDX = (uint32_t)va_arg(args, uint32_t);
    s.ECX = (uint32_t)va_arg(args, uint32_t);
    s.EAX = (uint32_t)va_arg(args, uint32_t);
    s.GS = (uint32_t)va_arg(args, uint32_t);
    s.FS = (uint32_t)va_arg(args, uint32_t);
    s.ES = (uint32_t)va_arg(args, uint32_t);
    s.DS = (uint32_t)va_arg(args, uint32_t);
    s.ERROCODE = (uint32_t)va_arg(args, uint32_t);
    s.EFLAGS = (uint32_t)va_arg(args, uint32_t);
    s.oldESP = (uint32_t)va_arg(args, uint32_t);
    s.IVN = IVN;
    // log("---------------------------------------------\n");
    // logStackInfo(&s);
    // log("---------%s------------\n", intr_name[IVN]);
    StackInfo *t;
    t = (StackInfo *)(&IVN);
    StackInfo s2;
    memcpy_(&s2, t, sizeof(StackInfo));

    switch (IVN)
    {
    case 32:
        log("interrupt test successful ! IVN = %d , time = %d\n ", IVN, i);
        // 发生切换
        break;

    default:
        log("---------%s------------\n", intr_name[IVN]);
        break;
    }

    i++;
    va_end(args);
}

void initIntr_name()
{
    for (uint32_t i = 0; i < IETSize; i++)
    {
        intr_name[i] = "unknown!";
    }

    intr_name[0] = "#DE Divide Error";
    intr_name[1] = "#DB Debug Exception";
    intr_name[2] = "NMI Interrupt";
    intr_name[3] = "#BP Breakpoint Exception";
    intr_name[4] = "#OF Overflow Exception";
    intr_name[5] = "#BR BOUND Range Exceeded Exception";
    intr_name[6] = "#UD Invalid Opcode Exception";
    intr_name[7] = "#NM Device Not Available Exception";
    intr_name[8] = "#DF Double Fault Exception";
    intr_name[9] = "Coprocessor Segment Overrun";
    intr_name[10] = "#TS Invalid TSS Exception";
    intr_name[11] = "#NP Segment Not Present";
    intr_name[12] = "#SS Stack Fault Exception";
    intr_name[13] = "#GP General Protection Exception";
    intr_name[14] = "#PF Page-Fault Exception";
    // intr_name[15] = "unknown!"; // 第15项是intel保留项，未使用
    intr_name[16] = "#MF x87 FPU Floating-Point Error";
    intr_name[17] = "#AC Alignment Check Exception";
    intr_name[18] = "#MC Machine-Check Exception";
    intr_name[19] = "#XF SIMD Floating-Point Exception";
}

void disable_irq()
{
    asm volatile("cli");
}

uint32_t areInterruptsEnabled()
{
    uint32_t flags;
    asm volatile("pushf; pop %0" : "=g"(flags));
    return flags & (1 << 9);
}

// 可能会存在,嵌套函数频繁开关中断,enable_irq应该要考虑原中断状态
void enable_irq()
{
    //
    asm volatile("sti");
}

void logStackInfo(StackInfo *s)
{
    log("Stack info:\n interrupt vector number: %d\n"
        " general register: \n      EDI=%d \n      ESI=%d \n      EBP=%d \n      ESP=%d \n      EBX=%d \n      EDX=%d \n      ECX=%d \n      EAX=%d \n"
        " segment registers: \n      GS=%d \n      FS=%d \n      ES=%d \n      DS=%d\n"
        " error code: %d\n",
        s->IVN,
        s->EDI, s->ESI, s->EBP, s->newESP, s->EBX, s->EDX, s->ECX, s->EAX,
        s->GS, s->FS, s->ES, s->DS,

        s->ERROCODE);
}

// 初始化一个描述符
void init_idt_desc(InterruptDescriptor *interruptDes, uint8_t attr, void *function)
{
    interruptDes->offset_low = (uint32_t)function & 0x0000FFFF;
    interruptDes->selector = SELECTOR_K_CODE;
    interruptDes->zero = 0;
    interruptDes->type_attr = attr;
    interruptDes->offset_high = ((uint32_t)function & 0xFFFF0000) >> 16;
}

// 返回IDT
InterruptDescriptor *interruptInit()
{
    // IDT设置好应该不会再动了,返回的必要性不是很大
    log("init interrupt!\n");
    // 初始化IDT
    for (uint16_t i = 0; i < IETSize; i++)
    {
        // 初始化中断描述符
        init_idt_desc(IDT + i, IDT_DESC_ATTR_DPL0, intr_entry_table[i]);
    }

    // 初始化中断名称
    initIntr_name();
    // 设置IDTR:前16 界限 后32地址
    uint64_t idt_operand = ((sizeof(IDT) - 1) | ((uint64_t)(uint32_t)IDT << 16));
    asm volatile("lidt %0" : : "m"(idt_operand));

    // 初始化pic:8259A可编程中断控制器
    initPIC();

    // 初始化时钟中断
    timer_init();
    // 开启时钟中断
    startTimerInterrupt();

    return IDT;
}