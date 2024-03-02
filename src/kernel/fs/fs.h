#ifndef FS_H
#define FS_H
#include "../include/types.h"
#include "../include/roodos.h"
#include "../include/device.h"

#define MAX_FILE_NAME_LEN 16
#define SUPERBLOCKMAGIC 2000626
#define OPENINODEPAGESIZE 117      // 100扇区
#define OPENINODESIZE 7 * 1006 + 1 // 整齐一点,一个扇区大概放7个inode,多一个用于循环队列
#define MAX_FILE_TYPE 32           // 支持32种文件类型
#define INODE_SECTORS_SIZE 13      // 11==>2级 12==>3级

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
    uint32_t i_sectors[INODE_SECTORS_SIZE];
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

typedef struct file
{
    uint32_t size;
    uint32_t inode_no;
    uint32_t file_type;
    uint32_t index; // 下次操作的位置
    uint32_t mode;  // 1读,10写,0b11读写,0b100追加
} file;

// 文件类型的数据结构,包含了不同文件类型用到的接口
typedef struct file_type
{
    enum file_types type;
    inode *(*open)(uint32_t inode_no, uint32_t mode);
    int32_t (*read)(uint32_t inode_no, uint32_t addr, char *buf, uint32_t size);
    int32_t (*write)(uint32_t inode_no, uint32_t addr, char *buf, uint32_t size);
    uint32_t (*control)(uint32_t inode_no, uint32_t cmd, int32_t *args, uint32_t n);
    void (*info)(uint32_t inode_no, char buff[DEVINFOSIZE]); // 返回设备文件信息
    void (*close)(uint32_t inode_no);
} file_type;

typedef struct
{
    partition *amount_partions[PARTITIONSIZE];
    uint32_t amount_partions_length;
    partition *unknown_partions[PARTITIONSIZE];
    uint32_t unknown_partions_length;

    file_type *ft[MAX_FILE_TYPE];
    uint32_t file_type_length;
} file_sys;

//-- fs.c
extern bool identify_super_b(partition *p);
extern bool open_fs(uint32_t ino, enum file_types ft, uint32_t mode);
extern int32_t read_fs(uint32_t ino, enum file_types ft, uint32_t addr, char *buf, uint32_t size);
extern int32_t write_fs(uint32_t ino, enum file_types ft, uint32_t addr, char *buf, uint32_t size);
extern uint32_t control_fs(uint32_t ino, enum file_types ft, uint32_t cmd, int32_t *args, uint32_t n);
extern void info_fs(uint32_t ino, enum file_types ft, char buff[DEVINFOSIZE]);
extern void close_fs(uint32_t ino, enum file_types ft);
extern bool register_file_type(file_type *type);
extern inode *open_file(uint32_t inode_no, uint32_t mode);
extern int32_t read_file(uint32_t inode_no, uint32_t addr, char *buf, uint32_t size);
extern int32_t write_file(uint32_t inode_no, uint32_t addr, char *buf, uint32_t size);
extern uint32_t control_file(uint32_t inode_no, uint32_t cmd, int32_t *args, uint32_t n);
extern void info_file(uint32_t inode_no, char buff[DEVINFOSIZE]);
extern void close_file(uint32_t inode_no);
extern void fs_init();
extern uint32_t find_empty_fd();
extern uint32_t syscall_fs_open(char *filepath);
extern uint32_t syscall_write_fs(uint32_t fd, uint32_t addr, char *buf, uint32_t size);
extern uint32_t syscall_read_fs(uint32_t fd, uint32_t addr, char *buf, uint32_t size);
extern void syscall_close_fs(uint32_t fd);
extern bool make_dir(uint32_t fd, char *dir_name);
extern bool make_file(uint32_t fd, char *file_name);
//--
// partition.c
extern void read_partition(partition *p, char *buff, uint32_t sec, uint32_t size);
extern void write_partition(partition *p, char *buff, uint32_t sec, uint32_t size);
extern void malloc_mem(partition *p);
extern void save_block_bitmap(partition *p);
extern void load_block_bitmap(partition *p);
extern void save_inode_bitmap(partition *p);
extern void load_inode_bitmap(partition *p);
extern void load_super_block(partition *p);
extern void save_super_block(partition *p);
extern void buildSuperBlock(partition *p, uint32_t inode_cnt);

// inode.c
extern partition *get_partition_by_inode_no(uint32_t inode_no);
extern inode *searchInodeInBuff(partition *p, uint32_t inode_no);
extern void save_7_inode(partition *p, inode *ino);
extern inode *load_inode_by_inode_no(uint32_t inode_no);
extern void save_inode(uint32_t inode_no);
extern bool malloc_block(partition *p, inode *node, uint32_t sec_index);
extern void free_block(partition *p, inode *node, uint32_t sec_index);
extern uint32_t write_inode(uint32_t inode_no, char *buff, uint32_t sec_index);
extern uint32_t read_inode(uint32_t inode_no, char *buff, uint32_t sec_index);
extern inode *malloc_inode(partition *p);
extern void free_inode(uint32_t inode_no);
extern bool return_sector(partition *p, uint32_t sec);
extern void free_all_resource_inode(uint32_t inode_no);
// extern void free_overflow_resources_inode(uint32_t inode_no);

// directory.c

// 读取文件的通用的接口
extern void copy_filename(char *destination, char *source);
extern bool init_new_dir(uint32_t root_ino, uint32_t new_i_no);
extern bool create_dir(uint32_t root_ino, char *dir_name);
extern bool create_file(uint32_t root_ino, char *file_name);
extern bool delete_file(uint32_t root_ino, char *file_name);
extern bool search_file_by_name(uint32_t root_ino, char *file_name, dir_entry *dest);
extern bool search_file_by_path(char *file_path, uint32_t *ino, uint32_t *file_type);
extern void init_direcory();

//-- regular.c
extern void init_regular();
//--

#endif