// 普通文件
// 目录文件相关接口
#include "fs.h"

// 写接口写完要需要统计文件的大小!,更新到inode
// 文件大小就简单统计文件获得的磁盘大小即可
// addr 第几个512字节

file_type regular_type = {
    .type = FT_DIRECTORY,
    .open = open_file,
    .read = read_file, // 读接口用默认的
    .write = write_file,
    .control = control_file,
    .info = info_file,
    .close = close_file,
};

void init_regular()
{
    register_file_type(&regular_type);
}