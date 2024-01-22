/*
 * @Author: EKKO 189890049@qq.com
 * @Date: 2023-10-03 22:53:46
 * @LastEditors: EKKO 189890049@qq.com
 * @LastEditTime: 2024-01-13 22:13:09
 * @Description:
 */

#include "include/kernel.h";
kernel roodos;
void initRoodOs();
int main(int memCount, uint32_t memAddr, uint32_t KernelVAddr, uint32_t pTablePhAddr, uint32_t gdtVaddr, uint32_t tssVaddr)
{
    roodos.gdt = (GDT *)gdtVaddr;
    printf("gdt   count:%d \n", (roodos.gdt->limit + 1) / 8);
    printf("gdt virtual addr:0x%p\n", roodos.gdt->sd);

    initRoodOs();
    initMemoryManagement(memCount, (void *)memAddr, (void *)KernelVAddr, (void *)pTablePhAddr);
    interruptInit();
    initProcess(tssVaddr);

    enable_irq();
    while (1)
    {
        // uint32_t i = 10 / 0;
    }

    return 0;
}

void initRoodOs()
{
    printf("Kernel was loaded successful !\n");
    roodos.name = "RoodOs";
    roodos.version = "0.0.1";
    roodos.physicalAddr = 0x11000;
    roodos.virtualAddr = 0xc0011000;
    // __asm__("xchg %%bx,%%bx" ::); //断点
    // 初始化 内存管理模块
    printf("Welcom to RoodOs! ^_^\n");
    printf("___%s__%s_ \nkernel p\\v addr:0x%p\\0x%p\n", roodos.name, roodos.version, roodos.physicalAddr, roodos.virtualAddr);
}