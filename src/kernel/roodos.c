
#include "include/roodos.h"
#include "include/types.h"

// 处理器停机
void hlt()
{
    asm volatile("hlt");
    return;
}
