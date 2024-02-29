// todo 2/29 dir_entry存放的是全局inode编号

// 目录文件相关接口
#include "fs.h"

file_type dir_type = {
    .type = FT_DIRECTORY,
    .open = open_file,
    .read = read_file,
    .write = write_file,
    .control = control_file,
    .info = info_file,
    .close = close_file,
};

void copy_filename(char *destination, char *source)
{
    strlen_(source) < MAX_FILE_NAME_LEN ? memcpy_(destination, source, strlen_(source) + 1) : memcpy_(destination, source, MAX_FILE_NAME_LEN - 1);
    destination[MAX_FILE_NAME_LEN] = 0;
}

char dir_buff[1024]; // 后512专门设置为一个空白的inode文件的数据

bool init_new_dir(uint32_t root_ino, uint32_t new_i_no)
{
    // 初始化文件夹
    dir_entry *d_e = dir_buff + 512;
    d_e[0].i_no = new_i_no;
    d_e[0].filename[0] = '.';
    d_e[0].filename[1] = '\0';
    d_e[0].f_type = FT_DIRECTORY;

    d_e[1].i_no = root_ino;
    d_e[1].filename[0] = '.';
    d_e[1].filename[1] = '.';
    d_e[1].filename[2] = '\0';
    d_e[1].f_type = FT_DIRECTORY;
    if (write_file(new_i_no, 0, d_e, 512) == 0)
    {
        d_e[0].f_type = FT_UNKNOWN;
        d_e[1].f_type = FT_UNKNOWN;
        return false;
    }
    d_e[0].f_type = FT_UNKNOWN;
    d_e[1].f_type = FT_UNKNOWN;

    // 更新inode中的容量大小
    inode *node = load_inode_by_inode_no(new_i_no);
    node->i_size += 2;
    save_inode(new_i_no);
    return true;
}

bool create_dir(uint32_t root_ino, char *dir_name)
{
    // 读取文件
    dir_entry entry;
    entry.f_type = FT_DIRECTORY;
    copy_filename(entry.filename, dir_name);
    uint32_t sec_index = 0;
    dir_entry *d_e = dir_buff;
    int32_t target_sec_index = -1;
    int32_t entry_index = -1;
    while (read_file(root_ino, sec_index, dir_buff, 512) > 0)
    {
        for (uint32_t i = 0; i < 512 / 24; i++)
        {
            if (target_sec_index < 0 && d_e[i].f_type == FT_UNKNOWN)
            {
                // 找到了空位置
                target_sec_index = sec_index;
                entry_index = i;
            }
            else if (strcmp_(d_e[i].filename, entry.filename) == 0)
            {
                return false;
            }
        }
        sec_index++;
    }
    partition *p = get_partition_by_inode_no(root_ino);
    if (p == NULL)
    {
        return false;
    }
    inode *i = malloc_inode(p);

    if (i == NULL)
    {
        return false;
    }

    // 初始化文件夹
    if (!init_new_dir(root_ino, i->i_no))
    {
        free_all_resource_inode(i->i_no);
        return false;
    }

    if (target_sec_index != -1)
    {
        // 找到空位,且没有重名
        read_file(root_ino, target_sec_index, dir_buff, 512);
        // todo 2/29
        entry.i_no = i->i_no;
        memcpy_(d_e + entry_index, &entry, sizeof(dir_entry));
        write_file(root_ino, target_sec_index, d_e, 512);
        // 更新容量大小
        inode *node = load_inode_by_inode_no(root_ino);
        node->i_size += 1;
        save_inode(root_ino);
        return true;
    }

    // 没有空的且没有空间了
    d_e = dir_buff + 512;
    // todo 2/29
    entry.i_no = i->i_no;
    memcpy_(d_e, &entry, sizeof(dir_entry));
    if (write_file(root_ino, sec_index, d_e, 512) == 0)
    {
        d_e[0].f_type = FT_UNKNOWN;
        free_all_resource_inode(i->i_no);
        return false;
    }
    d_e[0].f_type = FT_UNKNOWN;
    // 更新容量大小
    inode *node = load_inode_by_inode_no(root_ino);
    node->i_size += 1;
    save_inode(root_ino);
    return true;
}

bool create_file(uint32_t root_ino, char *file_name)
{
    // 读取文件
    dir_entry entry;
    entry.f_type = FT_REGULAR;
    copy_filename(entry.filename, file_name);
    uint32_t sec_index = 0;
    dir_entry *d_e = dir_buff;
    int32_t target_sec_index = -1;
    int32_t entry_index = -1;
    while (read_file(root_ino, sec_index, dir_buff, 512) > 0)
    {
        for (uint32_t i = 0; i < 512 / 24; i++)
        {
            if (target_sec_index < 0 && d_e[i].f_type == FT_UNKNOWN)
            {
                // 找到了空位置
                target_sec_index = sec_index;
                entry_index = i;
            }
            else if (strcmp_(d_e[i].filename, entry.filename) == 0)
            {
                return false;
            }
        }
        sec_index++;
    }
    partition *p = get_partition_by_inode_no(root_ino);
    if (p == NULL)
    {
        return false;
    }
    inode *i = malloc_inode(p);

    if (i == NULL)
    {
        return false;
    }

    if (target_sec_index != -1)
    {
        // 找到空位,且没有重名
        read_file(root_ino, target_sec_index, dir_buff, 512);
        entry.i_no = i->i_no; // todo 2/29
        memcpy_(d_e + entry_index, &entry, sizeof(dir_entry));
        write_file(root_ino, target_sec_index, d_e, 512);
        // 更新容量大小
        inode *node = load_inode_by_inode_no(root_ino);
        node->i_size += 1;
        save_inode(root_ino);
        return true;
    }

    // 没有空的且没有空间了
    d_e = dir_buff + 512;

    entry.i_no = i->i_no; // todo : 2/29
    // -p->sb.inode_global_start_index
    memcpy_(d_e, &entry, sizeof(dir_entry));
    if (write_file(root_ino, sec_index, d_e, 512) == 0)
    {
        d_e[0].f_type = FT_UNKNOWN;
        free_all_resource_inode(i->i_no);
        return false;
    }
    d_e[0].f_type = FT_UNKNOWN;

    // 更新容量大小
    inode *node = load_inode_by_inode_no(root_ino);
    node->i_size += 1;
    save_inode(root_ino);
    return true;
}

// 删除文件
bool delete_file(uint32_t root_ino, char *file_name)
{
    // 读取文件
    dir_entry entry;
    entry.f_type = FT_REGULAR;
    copy_filename(entry.filename, file_name);
    uint32_t sec_index = 0;
    dir_entry *d_e = dir_buff;
    int32_t target_sec_index = -1;
    int32_t entry_index = -1;

    // todo : 只考虑一个分区的情况,多余的分区后续在考虑
    partition *p = get_partition_by_inode_no(root_ino);

    while (read_file(root_ino, sec_index, dir_buff, 512) > 0)
    {
        for (uint32_t i = 0; i < 512 / 24; i++)
        {
            if (strcmp_(d_e[i].filename, entry.filename) == 0)
            {

                // todo 2/29
                free_all_resource_inode(d_e[i].i_no);
                d_e[i].f_type = FT_UNKNOWN;
                // 更新容量大小
                inode *node = load_inode_by_inode_no(root_ino);
                node->i_size -= 1;
                save_inode(root_ino);
                // 写回
                write_file(root_ino, sec_index, dir_buff, 512);
                return true;
            }
        }
        sec_index++;
    }
    return false;
}

// 通过文件名称查找文件
bool search_file_by_name(uint32_t root_ino, char *file_name, dir_entry *dest)
{

    // 读取文件
    dir_entry entry;
    entry.f_type = FT_REGULAR;
    copy_filename(entry.filename, file_name);
    uint32_t sec_index = 0;
    dir_entry *d_e = dir_buff;
    int32_t target_sec_index = -1;
    int32_t entry_index = -1;

    // todo : 只考虑一个分区的情况,多余的分区后续在考虑
    partition *p = get_partition_by_inode_no(root_ino);

    while (read_file(root_ino, sec_index, dir_buff, 512) > 0)
    {
        for (uint32_t i = 0; i < 512 / 24; i++)
        {
            if (strcmp_(d_e[i].filename, entry.filename) == 0)
            {

                memcpy_(dest, d_e + i, sizeof(dir_entry));
                return true;
            }
        }
        sec_index++;
    }
    return false;
}

char test_buff[1024];
void init_direcory()
{
    dir_entry *d_e = dir_buff + 512;
    // 初始化 dir_buf的空白区域
    for (uint32_t i = 0; i < 512 / 24; i++)
    {
        d_e[i].f_type = FT_UNKNOWN;
    }

    register_file_type(&dir_type);
    d_e = test_buff;
    log(" \n %d \n", sizeof(dir_entry));
    // create_dir(0, "test_file");
    inode *zero = load_inode_by_inode_no(0);
    // create_file(0, "new_test");
    read_file(0, 0, test_buff, 512);

    for (uint32_t i = 0; i < 512 / 24; i++)
    {
        if (d_e[i].f_type != FT_UNKNOWN)
        {
            log("type:%d name:%s i_no:%d \n", d_e[i].f_type, d_e[i].filename, d_e[i].i_no);
        }
    }
    create_file(0, "new_test222");
    create_dir(0, "test_file");
    delete_file(0, "new_test");
    read_file(0, 0, test_buff, 512);
    for (uint32_t i = 0; i < 512 / 24; i++)
    {
        if (d_e[i].f_type != FT_UNKNOWN)
        {
            log("type:%d name:%s i_no:%d \n", d_e[i].f_type, d_e[i].filename, d_e[i].i_no);
        }
    }

    log("test 2 \n");
    dir_entry search_de;
    search_file_by_name(0, "test_file", &search_de);

    create_file(search_de.i_no, "hello");
    create_dir(search_de.i_no, "balbal");
    zero = load_inode_by_inode_no(search_de.i_no);
    read_file(search_de.i_no, 0, test_buff, 512);
    for (uint32_t i = 0; i < 512 / 24; i++)
    {
        if (d_e[i].f_type != FT_UNKNOWN)
        {
            log("type:%d name:%s i_no:%d \n", d_e[i].f_type, d_e[i].filename, d_e[i].i_no);
        }
    }
}