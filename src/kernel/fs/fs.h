#ifndef FS_H
#define FS_H
#include "../include/types.h"
#include "../include/roodos.h"
#include "../include/device.h"

#define MAX_FILE_NAME_LEN 16
#define SUPERBLOCKMAGIC 2000626
#define OPENINODEPAGESIZE 117      // 100扇区
#define OPENINODESIZE 7 * 1006 + 1 // 整齐一点,一个扇区大概放7个inode,多一个用于循环队列

// 数据结构在《操作系统真相还原》一书的基础上修改

/* 文件类型 */
enum file_types
{
    FT_UNKNOWN,   // 不支持的文件类型
    FT_REGULAR,   // 普通文件
    FT_DIRECTORY, // 目录
    DEVICE,       // 设备
    PARTITION,    // 挂载的分区
};
/* inode结构 */

typedef struct
{
    uint32_t i_no; // inode编号,

    /* 当此inode是文件时,i_size是指文件大小,
    若此inode是目录,i_size是指该目录下所有目录项大小之和*/
    uint32_t i_size;

    uint32_t i_open_cnts; // 记录此文件被打开的次数
    bool write_deny;      // 写文件不能并行,进程写文件前检查此标识

    /* i_sectors[0-11]是直接块, i_sectors[12]用来存储一级间接块指针 */
    uint32_t i_sectors[13];
    // struct list_elem inode_tag; // 缓冲区标记
} inode; // 68字节

/* 目录结构 */
typedef struct dir
{
    struct inode *inode;
    uint32_t dir_pos;     // 记录在目录内的偏移,buf不一定能一次读完所有的目录项,记录偏移继续读取
    uint8_t dir_buf[512]; // 目录的数据缓存
} dir;                    // 520字节

/* 目录项结构 */
typedef struct dir_entry
{
    char filename[MAX_FILE_NAME_LEN]; // 普通文件或目录名称
    uint32_t i_no;                    // 普通文件或目录对应的inode编号
    enum file_types f_type;           // 文件类型
} dir_entry;                          // 24字节 一个扇区21个目录项

typedef struct
{
    linkedList amount_partions; // 已经挂载的分区
} file_sys;
#endif