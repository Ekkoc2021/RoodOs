
#include "interrupt.h"

extern uint32_t intr_entry_table[IETSize]; // 中断入口表
InterruptDescriptor IDT[IETSize];          // 中断描述符表
char *intr_name[IETSize];                  // 中断名称
extern processManager manager;             // 进程管理器
uint32_t i = 0;                            // 测试,可以删除
extern void intr_exit();
extern void schedule();

extern void intr_keyboard_handler();
extern void sys_call();
extern void initSemaphoreMoudle();
extern void intr_hd_handler(uint8_t irq_no);

// extern int32_t getAvailableSem();
// extern void setSem(uint32_t pid, uint16_t __value, uint16_t id);

void interruptHandler(uint32_t IVN, ...)
{
    if (IVN == 0x27 || IVN == 0x2f)
    {           // 0x2f是从片8259A上的最后一个irq引脚，保留
        return; // IRQ7和IRQ15会产生伪中断(spurious interrupt),无须处理。
    }

    // 根据IVN 调用不同中断函数
    switch (IVN)
    {
    case 14:
        uint32_t page_fault_vaddr = 0;
        // asm volatile("movl %0, %%cr3" : : "r"(paddr) : "memory");
        asm("movl %%cr2, %0" : "=r"(page_fault_vaddr) : :); // cr2是存放造成page_fault的地址
        log("#PF : %d / %p \n", page_fault_vaddr, page_fault_vaddr);
        logStackInfo(&IVN);
        hlt();
        break;
    case 32:
        // 时钟中断,发生切换
        // __asm__("xchg %%bx,%%bx" ::);
        schedule();

        // __asm__("xchg %%bx,%%bx" ::);
        break;
    case 0x21:
        // 键盘中断
        intr_keyboard_handler();
        break;
    case 0x2e:
        intr_hd_handler(0x2e);
        break;

    case 0x30:
        // system_call 系统调用
        sys_call(&IVN);
        break;

    default:
        log("---%d:-----%s------------\n", IVN, intr_name[IVN]);
        break;
    }
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
        init_idt_desc(IDT + i, IDT_DESC_ATTR_DPL3, intr_entry_table[i]);
    }
    init_idt_desc(IDT + IETSize - 1, IDT_DESC_ATTR_DPL3, intr_entry_table[IETSize - 1]);

    // 初始化中断名称
    initIntr_name();
    // 设置IDTR:前16 界限 后32地址
    uint64_t idt_operand = ((sizeof(IDT) - 1) | ((uint64_t)(uint32_t)IDT << 16));
    asm volatile("lidt %0" : : "m"(idt_operand));

    // 初始化pic:8259A可编程中断控制器
    initPIC();
    // 初始化时钟中断
    timer_init();

    // 允许时钟中断
    startTimerInterrupt();

    // 允许键盘中断
    startKeyboardInterrupt();

    // 允许硬盘中断:放在磁盘模块去开启,磁盘模块在这个函数之后执行
    // startIDEInterrupt();

    return IDT;
}