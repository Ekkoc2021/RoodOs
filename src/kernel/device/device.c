// 设备管理
#include "device.h"

device all_dev[DEVSIZE];
extern void initTTY();
extern void diskInit();

void deviceModuleInit()
{
    for (uint32_t i = 0; i < DEVSIZE; i++)
    {
        all_dev[i].type = NULL;
    }
}

// 注册设备,后续设备可以设计成进程
void registerDev(device *dev)
{
    if (dev->type == NULL)
    {
        return;
    }

    uint32_t deviceId = 0;
    uint32_t emptyIndex = DEVSIZE + 1;
    // 找到空位置
    for (uint32_t i = 0; i < DEVSIZE; i++)
    {
        if (emptyIndex == DEVSIZE + 1 && all_dev[i].type == NULL)
        {
            emptyIndex = i;
        }

        if (all_dev[i].type != NULL && all_dev[i].type->typeId == dev->type->typeId)
        {
            deviceId++;
        }
    }
    if (emptyIndex != DEVSIZE + 1)
    {
        memcpy_(all_dev + emptyIndex, dev, sizeof(device));
        all_dev[emptyIndex].deviceId = deviceId;
        dev->deviceId = deviceId;
    }
}

// 打开一个指定类型的设备
uint32_t open(uint32_t typeId, uint32_t deviceId)
{
    for (uint16_t i = 0; i < DEVSIZE; i++)
    {
        if (all_dev[i].type->typeId == typeId && all_dev[i].deviceId == deviceId)
        {

            return all_dev[i].type->open(&(all_dev[i])); // 打开成功
        }
    }
    return false; // 打开失败
}

// 指定一个类型的设备读
int32_t read(uint32_t typeId, uint32_t deviceId, uint32_t addr, char *buf, uint32_t size)
{
    for (uint16_t i = 0; i < DEVSIZE; i++)
    {
        if (all_dev[i].type->typeId == typeId && all_dev[i].deviceId == deviceId)
        {

            return all_dev[i].type->read(&(all_dev[i]), addr, buf, size);
        }
    }
    return -1;
}

// 指定一个类型的设备写
int32_t write(uint32_t typeId, uint32_t deviceId, uint32_t addr, char *buf, uint32_t size)
{
    for (uint16_t i = 0; i < DEVSIZE; i++)
    {
        if (all_dev[i].type->typeId == typeId && all_dev[i].deviceId == deviceId)
        {

            return all_dev[i].type->write(&(all_dev[i]), addr, buf, size);
        }
    }
    return -1;
}

// 向指定设备发送控制信号
uint32_t control(uint32_t typeId, uint32_t deviceId, uint32_t cmd, int32_t *args, uint32_t n)
{
    for (uint16_t i = 0; i < DEVSIZE; i++)
    {
        if (all_dev[i].type->typeId == typeId && all_dev[i].deviceId == deviceId)
        {

            return all_dev[i].type->control(&(all_dev[i]), cmd, args, n);
        }
    }
    return false;
}

// 关闭一个指定类型的设备
void close(uint32_t typeId, uint32_t deviceId)
{
    for (uint16_t i = 0; i < DEVSIZE; i++)
    {
        if (all_dev[i].type->typeId == typeId && all_dev[i].deviceId == deviceId)
        {

            return all_dev[i].type->close(&(all_dev[i]));
        }
    }
}

void sysDevInit()
{
    log("device module init..\n");
    deviceModuleInit();
    log("device module init done!\n");
    log("tty dev init..\n");
    initTTY();
    log("tty dev init done!\n");

    // 测试 TTY设备
    // open(TTY, 0);
    // char *test = "hello tty!\n";
    // write(TTY, 0, test, test, strlen_(test));
    // enable_irq();
    // char buff[128];
    // read(TTY, 0, buff, buff, 3);
    // close(TTY, 0);

    diskInit();
}
