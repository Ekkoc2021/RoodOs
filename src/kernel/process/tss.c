/*
 * @Author: EKKO 189890049@qq.com
 * @Date: 2024-01-21 11:44:08
 * @LastEditors: EKKO 189890049@qq.com
 * @LastEditTime: 2024-01-24 10:51:28
 * @FilePath: \RoodOs\src\kernel\process\tss.c
 * @Description: 这是默认设置,请设置`customMade`, 打开koroFileHeader查看配置 进行设置: https://github.com/OBKoro1/koro1FileHeader/wiki/%E9%85%8D%E7%BD%AE
 */
#include "tss.h"
#include "../include/kernel.h"
TSS *Tss;
// 初始化一个tss对象,并返回:只设置0特权级栈的ss与esp
void initTss(TSS *tss, GDT *gdt)
{
    log("tss_init start\n");
    tss->ss0 = 0b10000;
    asm volatile(
        "ltr %%ax\n" ::"a"(5 << 3));
    log("tss_init done\n");
    Tss = tss;
}

// 设置0特权级栈
/* 更新tss中esp0字段的值为pthread的0级线 */
void update_tss_esp(TSS *tss, uint32_t esp0)
{
    tss->esp0 = (uint32_t *)esp0;
}