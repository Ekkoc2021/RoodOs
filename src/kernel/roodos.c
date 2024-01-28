
#include "include/roodos.h"
#include "include/types.h"

// 处理器停机
void hlt()
{
    asm volatile("hlt");
    return;
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

// 确保中断关闭,如果关闭这返回1,否则关闭中断且返回0
char BeSureDisable_irq()
{
    if (areInterruptsEnabled())
    {
        disable_irq();
        return 0;
    }
    return 1;
}
// 恢复中断请求,根据BeSureDisable_irq的返回值,恢复调用BeSureDisable_irq前的中断情况
void Resume_irq(char status)
{
    if (status == 0)
    {
        enable_irq();
    }
}
