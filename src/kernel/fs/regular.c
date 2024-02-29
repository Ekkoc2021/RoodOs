// 普通文件
// 目录文件相关接口
#include "fs.h"
file_type regular_type = {
    .type = FT_DIRECTORY,
    .open = open_file,
    .read = read_file,
    .write = write_file,
    .control = control_file,
    .info = info_file,
    .close = close_file,
};
void init_regular()
{
}