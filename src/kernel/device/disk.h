#ifndef DISK_H
#define DISK_H
#include "device.h"
#include "../include/assert.h"
#include "../include/bitmap.h"
#include "../include/linkedList.h"
#include "../include/process.h"
struct disk;

// 向上取整
#define DIV_ROUND_UP(X, STEP) ((X + STEP - 1) / (STEP))
#define UNUSED __attribute__((unused))
/* 分区结构 */
struct partition
{
    uint32_t start_lba;   // 起始扇区
    uint32_t sec_cnt;     // 扇区数
    struct disk *my_disk; // 分区所属的硬盘
    // linkedList part_tag;     // 用于队列中的标记
    char name[8];           // 分区名称
    struct super_block *sb; // 本分区的超级块
    Bitmap block_bitmap;    // 块位图
    Bitmap inode_bitmap;    // i结点位图
    linkedList open_inodes; // 本分区打开的i结点队列
};

/* 硬盘结构 */
struct disk
{
    char name[8];                   // 本硬盘的名称，如sda等
    struct ide_channel *my_channel; // 此块硬盘归属于哪个ide通道
    uint8_t dev_no;                 // 本硬盘是主0还是从1
    uint32_t sectors;               // 多少扇区
    struct partition prim_parts[4];  // 主分区顶多是4个
    struct partition logic_parts[8]; // 逻辑分区数量无限,但总得有个支持的上限,那就支持8个
};

/* ata通道结构 */
struct ide_channel
{
    char name[8];        // 本ata通道名称
    uint16_t port_base;  // 本通道的起始端口号
    uint8_t irq_no;      // 本通道所用的中断号
    int32_t lock;        // 通道锁
    bool expecting_intr; // 表示等待硬盘的中断
    // struct semaphore disk_done; // 用于阻塞、唤醒驱动程序
    int32_t dataLock;       // 每次访问只能有一个进程,数据锁
    struct disk devices[2]; // 一个通道上连接两个硬盘，一主一从

    // 异步读写用的一些变量
    uint32_t user;         // 当前使用的用户
    uint32_t secs_op;      // 每次操作的扇区数
    uint32_t secs_done;    // 已完成的扇区数
    uint32_t workStartLba; // 记录当前工作的起始扇区
    uint32_t last_op_time; // 用于记录上一次操作的tick
    // 两次操作之间应该间隔一段时间,否则大概率第二次操作读到数据也是错误数据
    // 每次发送读或写命令后,直接就能拿到数据了,不用进入阻塞
    // 也说不上来哪里有问题
    bool isWait; // 标识当前在磁盘的用户是否经过等待
};

#endif