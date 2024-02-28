// 目录文件相关接口
#include "fs.h"
extern bool register_file_type(file_type *type);

uint32_t open_dir(file *f)
{
    // 打开当前inode,计数器加1,然后保存数据

    // todo 当前那个文件正在写,加锁?先不管!
}
int32_t read_dir(file *f, uint32_t addr, char *buf, uint32_t size)
{
    // 读取inode文件数据
}
int32_t write_dir(file *f, uint32_t addr, char *buf, uint32_t size)
{
}
uint32_t control_dir(file *f, uint32_t cmd, int32_t *args, uint32_t n)
{
}
void info_dir(file *f, char buff[DEVINFOSIZE])
{
} // 返回设备文件信息
void close_dir(file *f)
{
}

file_type dir_type = {
    .type = FT_DIRECTORY,
    .open = open_dir,
    .read = read_dir,
    .write = write_dir,
    .control = control_dir,
    .info = info_dir,
    .close = close_dir,
};
void init_direcory()
{
    register_file_type(&dir_type);
}