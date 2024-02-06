/*
 * @Author: EKKO 189890049@qq.com
 * @Date: 2023-10-03 22:53:46
 * @LastEditors: EKKO 189890049@qq.com
 * @LastEditTime: 2024-01-13 22:13:09
 * @Description:
 */

#include "include/kernel.h";

kernel roodos;
extern void switch_to_user_mode();
extern void destroyPCB(PCB *pcb); // 不会检查pcb是否正确
extern processManager manager;
extern void initSemaphoreMoudle();

uint16_t createProcess(uint16_t weight, uint16_t argsLength, char *name, ...);
void initRoodOs();
void init_all_module(int memCount, uint32_t memAddr, uint32_t KernelVAddr, uint32_t pTablePhAddr);
// extern memoryMarket *market; 已经定义过了
int main(int memCount, uint32_t memAddr, uint32_t KernelVAddr, uint32_t pTablePhAddr, uint32_t gdtVaddr, uint32_t tssVaddr)
{
    printf("Kernel was loaded successful !\n");
    // 初始化一些基础属性
    roodos.name = "RoodOs";
    roodos.version = "0.0.1";
    roodos.physicalAddr = 0x11000;
    roodos.virtualAddr = 0xc0011000;
    roodos.gdt = (GDT *)gdtVaddr;
    roodos.tss = (TSS *)tssVaddr;
    printf("___%s__%s_ \nkernel p\\v addr:0x%p\\0x%p\n", roodos.name, roodos.version, roodos.physicalAddr, roodos.virtualAddr);
    init_all_module(memCount, memAddr, KernelVAddr, pTablePhAddr);

    //-------测试-----
    // char buff[50];
    // for (uint16_t i = 1; i < 25; i++)
    // {
    //     sprintf_(buff, "task %d", i);

    //     createProcess(i % 3 + 1, strlen_(buff), buff);
    // }

    // destroyPCB(manager.task[1]);
    // destroyPCB(manager.task[4]);
    // destroyPCB(manager.task[6]);
    // destroyPCB(manager.task[7]);
    // destroyPCB(manager.task[9]);
    // char *name = "shell1";
    // createProcess(0, strlen_(name), name);
    // char *name2 = "shell2";
    // createProcess(0, strlen_(name2), name2);
    // char *name3 = "shell3";
    // createProcess(0, strlen_(name3), name3);
    switch_to_user_mode();
}
void init_all_module(int memCount, uint32_t memAddr, uint32_t KernelVAddr, uint32_t pTablePhAddr)
{

    printf("gdt   count:%d \n", (roodos.gdt->limit + 1) / 8);
    printf("gdt virtual addr:0x%p\n", roodos.gdt->sd);

    roodos.market = initMemoryManagement(memCount, (void *)memAddr, (void *)KernelVAddr, (void *)pTablePhAddr);
    interruptInit();
    initSemaphoreMoudle();
    initProcess(roodos.tss, roodos.gdt);
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