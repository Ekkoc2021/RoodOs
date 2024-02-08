#ifndef DEVICE_H
#define DEVICE_H
#include "../include/types.h"
#include "../include/str.h"
#include "../include/roodos.h"

#define DEVSIZE 128
#define DEVNAMESIZE 128
struct dev_type;
typedef enum devType
{
    UNKNOW,
    TTY,
    DISK,
} devType;
typedef struct device
{
    struct dev_type *type; // 设备类型,以及对应的处理接口
    uint32_t mode;         // 设备的模式,不做过多考虑
    uint32_t deviceId;     // 次设备号,该类型设备下有多少
    void *data;            // 记录设备信息
    uint32_t open;         // 记录打开次数
} device;

typedef struct dev_type
{
    char name[DEVNAMESIZE];
    uint32_t typeId; // 类型ID
    uint32_t (*open)(device *dev);
    int32_t (*read)(device *dev, uint32_t addr, char *buf, uint32_t size);
    int32_t (*write)(device *dev, uint32_t addr, char *buf, uint32_t size);
    uint32_t (*control)(device *dev, uint32_t cmd, int32_t *args, uint32_t n);
    void (*close)(device *dev);
} dev_type;

#endif