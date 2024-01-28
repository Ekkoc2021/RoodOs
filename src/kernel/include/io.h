

#ifndef OSIO_H
#define OSIO_H
#include "str.h"
#include "types.h"
#include "roodos.h"
void printf(const char *format, ...);
void print(char *message); // 打印字符串
// global getCursorIndex
// global setCurso
extern void getCursorIndex(); // 光标位置放在bx中,返回
extern void setCursor();      // bx中存放要设置的光标位置
extern void readDisk();

char readDis(void *dest, unsigned int sectionSize, unsigned int startSection);
// 二次封装获取光标位置和设置光标位置的函数
int getCInd();
void setCInd(int index);
void scroll(); // 滚动
void clear();  // 清屏
void pInt(int a);
void putchar(char p);
void log_qemu_printf(const char *format, ...);
#endif