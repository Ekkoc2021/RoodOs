#ifndef TIMER_H
#define TIMER_H
#include "../include/roodos.h"
// 一秒钟发生1193180次修改计数器0的值,计数器0值减为0后发生一次中断
// 1193180 / 计数器 0 的初始计数值 = 中断信号的频率
// 1193180 / 中断信号的频率 = 计数器 0 的初始计数值
// 设置 计数器 0 的初始值,只要确定中断信号的频率即可

// 《操作系统真象还原》
#define IRQ0_FREQUENCY SCHEDULE_FREQUENCY // 中断信号的频率 1 秒钟 100次 tick大约5ms
#define INPUT_FREQUENCY 1193180
#define COUNTER0_VALUE INPUT_FREQUENCY / IRQ0_FREQUENCY // 寄存器数值
#define CONTRER0_PORT 0x40
#define COUNTER0_NO 0
#define COUNTER_MODE 2
#define READ_WRITE_LATCH 3
#define PIT_CONTROL_PORT 0x43
void timer_init();
#endif