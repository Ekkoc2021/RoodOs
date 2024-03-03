// 普通文件
// 目录文件相关接口
#include "fs.h"

// 写接口写完要需要统计文件的大小!,更新到inode
// 文件大小就简单统计文件获得的磁盘大小即可
// addr 第几个512字节

uint32_t write_regular(uint32_t inode_no, uint32_t addr, char *buf, uint32_t size)
{
    write_file(inode_no, addr, buf, size);
    update_inode_size(inode_no);
}

file_type regular_type = {
    .type = FT_DIRECTORY,
    .open = open_file,
    .read = read_file, // 读接口用默认的
    .write = write_regular,
    .control = control_file,
    .info = info_file,
    .close = close_file,
};

void init_regular()
{
    regular_type.type = FT_REGULAR;
    regular_type.open = open_file;
    regular_type.read = read_file; // 读接口用默认的
    regular_type.write = write_regular;
    regular_type.control = control_file;
    regular_type.info = info_file;
    regular_type.close = close_file;
    register_file_type(&regular_type);
}
