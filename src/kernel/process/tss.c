#include "tss.h"
// 初始化一个tss对象,并返回:只设置0特权级栈的ss与esp
void initTss(TSS *tss)
{
    log("tss_init start\n");
    memset_(tss, 0, sizeof(TSS));
    tss->ss0 = 0b0000000000010000;
    tss->io_base = sizeof(TSS);
    asm volatile("ltr %w0" : : "r"(5 << 3));
    log("tss_init done\n");
}

// 设置0特权级栈
/* 更新tss中esp0字段的值为pthread的0级线 */
void update_tss_esp(TSS *tss, uint32_t esp0)
{
    tss->esp0 = (uint32_t *)esp0;
}