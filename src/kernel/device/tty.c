#include "tty.h"
// 主动在device中注册tty设备:电传打字机,也就是屏幕+键盘
// 读和写都有一个环形缓冲区
// 允许多个进程同时读写,出问题后续再考虑

extern void initSem(uint16_t __value, int32_t *semId);
extern bool sem_open(uint32_t semId);
extern void semWait(int32_t semId);
extern void semSignal(int32_t semId);
extern void registerDev(device *dev);

extern void console_putchar(char p);
extern void console_puts(char *message);
extern processManager manager;

cirQueue outBuff; // 输出缓冲区,用不上,但也留着
char outbuff[TTYBUFFLEN];

cirQueue inBuff; // 输入缓冲区,读取缓冲区
char inbuff[TTYBUFFLEN];
int32_t readSemId_p;  // 用户之间不可同时读的互斥信号量
int32_t readSemId_kb; // 键盘与正在读的用户之间的信号量
uint32_t readIndex;   // 保存已经读取的位置
PCB *now;             // 正在使用的用户

uint32_t open_tty(device *dev)
{
    // 不需要检查是否合理
    dev->open++;
}

int32_t read_tty(device *dev, uint32_t addr, char *buf, uint32_t size)
{
    if (dev->open == 0)
    {
        return 0; // 没有打开过
    }

    if (now != manager.now)
    {
        // 如果是等于说明已经过获取资源了
        semWait(readSemId_p); // 能走这里的必然是用户进行了系统调用
        now = manager.now;    // 能够成功拿到资源
    }

    char *temp;
    for (uint32_t i = readIndex; i < size; i++)
    {

        semWait(readSemId_kb); // 申请资源
        cirDequeue(&inBuff, &temp);
        readIndex++;
        buf[i] = *temp;
    }
    readIndex = 0;          // 重置readIndex
    now = NULL;             // 重置now
    semSignal(readSemId_p); // 允许其他进程继续访问
}

int32_t write_tty(device *dev, uint32_t addr, char *buf, uint32_t size)
{
    // 就比较简单了,直接调用console中写数据往显示台写数据即可
    console_puts(buf); // 就不过缓冲区了,感觉脱裤子放屁
}

uint32_t control_tty(device *dev, uint32_t cmd, int32_t *args, uint32_t n)
{
    return 1;
}

void close_tty(device *dev)
{
    dev->open--;
}

void dataToInbuff(char data)
{
    if (cirEnqueue(&inBuff, &data))
    {
        semSignal(readSemId_kb);
    }
}

void info_tty(device *dev, char buff[DEVINFOSIZE])
{
    sprintf_(buff, "name:%s ,typeId : %d , dev_id : %d , open_cnt : %d\n", dev->type->name, dev->type->typeId, dev->deviceId, dev->open);
}

dev_type tty_type = {
    .open = open_tty,
    .read = read_tty,
    .write = write_tty,
    .close = close_tty,
    .control = control_tty,
    .typeId = TTY,
    .info = info_tty,
};

device tty_dev;

void initTTY()
{
    initCQueue(&outBuff, outbuff, sizeof(char), TTYBUFFLEN); // 输出缓冲区
    initCQueue(&inBuff, inbuff, sizeof(char), TTYBUFFLEN);   // 输入缓冲区

    // 初始化用户之间读信号量
    initSem(1, &readSemId_p);
    ASSERT(readSemId_p > 0);

    // 初始化键盘与读进程的同步信号量
    initSem(0, &readSemId_kb); // 0 个 最大值与缓冲区大小一致
    ASSERT(readSemId_kb > 0);

    now = NULL;

    char *name = "teletypewriters";
    memcpy_(tty_type.name, name, strlen_(name));
    tty_dev.type = &tty_type;

    registerDev(&tty_dev);
    ASSERT(tty_dev.deviceId != DEVNAMESIZE + 1);
}