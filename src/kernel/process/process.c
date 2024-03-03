#include "process.h"

#define EFLAGS_MBS (1 << 1)     // 此项必须要设置
#define EFLAGS_IF_1 (1 << 9)    // if为1,开中断
#define EFLAGS_IF_0 0           // if为0,关中断
#define EFLAGS_IOPL_3 (3 << 12) // IOPL3,用于测试用户程序在非系统调用下进行IO
#define EFLAGS_IOPL_0 (0 << 12) // IOPL0

#define USER_CODE_SELECTOR (3 << 3 | 0b11)
#define USER_DATA_SELECTOR (4 << 3 | 0b11)

#define INITPCB 0xc0007000

processManager manager;
extern memoryMarket market;
extern kernel roodos;
extern PCB *theNextProcess();
extern void insertWait(PCB *pcb);
extern void schedule();
extern void initSchedule();
extern void removeProcess(PCB *pcb);
extern void yeid();
extern void switchProcess();

extern void initSem(uint16_t __value, int32_t *semId);
extern bool sem_open(uint32_t semId);
extern void semWait(int32_t semId);
extern void semSignal(int32_t semId);

extern void switchUserPage(virtualMemPool *pd, userPageDir *upd, uint32_t paddr, uint32_t vaddr);
extern TSS *Tss;

//---

// 修改EIP为进行系统调用的位置
void backInt30()
{
    StackInfo *s = (manager.now->esp0) - sizeof(StackInfo);
    s->EIP = s->EIP - 2; // 回到系统调用的eip位置
}

// 返回系统时间和给的tick的差距
uint32_t getTickDifferent(uint32_t yourTick)
{
    if (manager.ticks > yourTick)
    {
        if (manager.ticks - yourTick > MAX / 2)
        {
            return MAX - manager.ticks + yourTick;
        }

        return manager.ticks - yourTick;
    }

    if (yourTick - manager.ticks > MAX / 2)
    {
        return MAX - yourTick + manager.ticks;
    }

    return yourTick - manager.ticks;
}

// 睡眠
void sleep(uint32_t ms)
{
    // 先判断当前进程是否刚进行过睡眠然后有到运行态!!
    if (manager.now->justWakeUp)
    {
        return;
    }

    //  计算多少tick
    uint32_t tick = ms % (1000 / SCHEDULE_FREQUENCY) == 0 ? ms / (1000 / SCHEDULE_FREQUENCY) : ms / (1000 / SCHEDULE_FREQUENCY) + 1;

    // 修改进程vruntime
    manager.now->vruntime = manager.now->vruntime + tick / manager.now->weight;

    // 修改进程runtime
    manager.now->runtime = tick % manager.now->weight;

    // 修改EIP
    backInt30();

    // 修改状态
    manager.now->status = SLEEP;

    // 标识下一次状态
    manager.now->justWakeUp = true;

    // 发起调度
    insertWait(manager.now);
    // 找到下一个进程
    manager.now = theNextProcess();
    manager.minVruntime = manager.now->vruntime;
    manager.now->status = RUNNING;
    // 修改tss 0特权级栈
    update_tss_esp(Tss, manager.now->esp0);
    // 切换页表
    switchUserPage(market.virMemPool, &(manager.now->u), manager.now->pagePAddr, manager.now->pageVAddr);
    switchProcess();
}

// 将当前进程放入对应的阻塞队列,显然是通过系统调用才会走到这里
void blockProcess(linkedQueue *blockQueue)
{
    PCB *p = manager.now;
    enqueue(blockQueue, &(p->blockTag));
    p->status = BLOCKED;
    // 找到下一个进程
    manager.now = theNextProcess();

    manager.now->status = RUNNING;
    // 修改tss 0特权级栈
    update_tss_esp(Tss, manager.now->esp0);

    // 切换页表
    switchUserPage(market.virMemPool, &(manager.now->u), manager.now->pagePAddr, manager.now->pageVAddr);

    switchProcess();
}

// 解析某个elf文件后,将程序展开后放入内存中,然后调用createProcess创建一个进程
// 调用createProcess处于内核态,分配好pcb后,直接且先切换到pcb对应的页表,根据elf头文件数据将数据读出,然后写入
// 有时候程序也不一定是从磁盘读出:就简单的认为程序就是只从磁盘启动
// 用户指定了一个路径,内核拿到路径,解析后,按照解析结果读取放到用户空间对应的位置,然后调用createProcess创建进程

PCB *createPCB(char *name, uint32_t id, uint16_t weight)
{
    // 分配内存
    uint32_t paddr;
    uint32_t pcbm = mallocPage_k(&market, &paddr);
    if (pcbm == 0)
    {
        return 0; // 分配pcb的内存都不够
    }

    // 创建pcb
    uint32_t s0 = mallocPage_k(&market, &paddr);
    if (s0 == 0)
    {
        freePage(&market, pcbm);
        return 0; // 分配0特权级栈内存不够
    }

    // 初始化名称
    PCB *pcb = (PCB *)pcbm;
    for (uint16_t i = 0; i < 127; i++)
    {
        if (name[i] == '\0')
        {
            pcb->name[i] = '\0';
            break;
        }
        pcb->name[i] = name[i];
    }

    pcb->status = CREATE;
    pcb->weight = weight;
    pcb->runtime = 0;

    // 初始化
    pcb->id = id;
    pcb->vruntime = 0;
    pcb->esp0 = s0 + 4080; // 留一点间隙

    uint32_t pageVaddr = mallocPage_k(&market, &paddr);
    if (pageVaddr == 0)
    {
        freePage(&market, pcbm);
        freePage(&market, s0);
        return 0;
    }

    // 分配文件描述符
    pcb->file_descriptors = mallocPage_k(&market, &pcb->file_descriptors);
    if (pcb->file_descriptors == 0)
    {
        freePage(&market, pcbm);
        freePage(&market, s0);
        freePage(&market, pageVaddr);
        return 0;
    }

    for (uint32_t i = 0; i < FD_MEM_SIZE / sizeof(file_descriptor); i++)
    {
        pcb->file_descriptors[i].file_type = 0;
    }

    pcb->pageVAddr = pageVaddr;
    pcb->pagePAddr = paddr;
    initUserPd((&market)->virMemPool, pageVaddr);
    initBitmap2(&(pcb->u.map), 768, pcb->u.bit);

    // 初始化pd
    for (uint16_t i = 0; i < 768; i++)
    {
        pcb->u.PT[i] = -1;
    }
    pcb->father = manager.now;
    initLinkedList(&(pcb->children)); // 初始化孩子链表
    pcb->blockTag.data = pcb;
    add(&(manager.now->children), &(pcb->tag));

    return pcb;
}
void loaderUserProgram(PCB *userPCB)
{
}
uint16_t getPID()
{
    for (uint16_t i = 1; i < TASKSIZE; i++)
    {
        if (manager.task[i] == 0)
        {
            return i;
        }
    }
    return 0;
}

void usePID(uint32_t pcbAddr, uint16_t pid)
{
    manager.task[pid] = pcbAddr;
}

void initStack(StackInfo *s, uint32_t eip, uint32_t esp3)
{
    s->EIP = eip;
    s->IVN = 32;
    s->EFLAGS = (EFLAGS_IOPL_0 | EFLAGS_MBS | EFLAGS_IF_1);
    s->CS = 0b11011;
    s->oldSS = 0b100011;
    s->oldESP = esp3;
    s->GS = 0b100011;
    s->FS = 0b100011;
    s->ES = 0b100011;
    s->DS = 0b100011;
}
// 反正也是在内核空间测试进程调度

//--------测试----------------------------------------------------------
// 实现 shell 命令行
// 测试设备管理系统调用
typedef struct
{
    char *buf;
    unsigned int typeId;
    unsigned int deviceId;
    unsigned int addr;
    unsigned int size; // 即是返回值有是传入值
} devParam_;

devParam_ dev_stdio;
void open_stdio()
{
    dev_stdio.typeId = 1;
    dev_stdio.deviceId = 0;
    dev_stdio.size = 128;
    asm volatile(
        "movl $60, %%eax\n"
        "movl %0, %%ebx\n"
        "int $0x30\n"
        :
        : "r"(&dev_stdio)
        : "%eax", "%ebx");
}
unsigned int read_stdio(char *buff, unsigned int size)
{
    dev_stdio.size = size;
    dev_stdio.buf = buff;
    uint32_t read_size = 0;
    asm volatile(
        "movl $61, %%eax\n"
        "movl %0, %%ebx\n"
        "movl %1, %%ecx\n"
        "int $0x30\n"

        :
        : "r"(&dev_stdio), "r"(&read_size)
        : "%eax", "%ebx", "%ecx");
    return read_size;
}

unsigned int write_stdio(char *buff, unsigned int size)
{
    dev_stdio.size = size;
    dev_stdio.buf = buff;
    uint32_t write_size = 0;
    asm volatile(
        "movl $62, %%eax\n"
        "movl %0, %%ebx\n"
        "movl %1, %%ecx\n"
        "int $0x30\n"
        :
        : "r"(&dev_stdio), "r"(&write_size)
        : "%eax", "%ebx", "%ecx");
    return write_size;
}
typedef struct
{
    char *cmd;
    void (*resolve)(char *cmd, char *args);
} command;
char output_buff[512];
// ls 列出当前文件夹下的文件
// cd 文件夹
// mkdir 创建文件夹
// mkfile 创建文件
// rm 删除文件
// rmdir 删除文件夹,如果文件夹内有文件,则无法删除
// exec 加载某个可执行的文件

typedef struct
{
    uint32_t fd;
    uint32_t addr;
    char *buf;
    uint32_t size;
} f_param;

enum f_types
{
    FT_UNKNOWN,   // 不支持的文件类型
    FT_REGULAR,   // 普通文件
    FT_DIRECTORY, // 目录
    DEVICE,       // 设备
    PARTITION,    // 挂载的分区
};

typedef struct dir_entry
{
    char filename[16];   // 普通文件或目录名称
    int i_no;            // 普通文件或目录对应的inode编号
    enum f_types f_type; // 文件类型
} dir_en;

typedef struct
{
    uint16_t weight;
    uint16_t argsLength;
    char *exec_file_path;
    char *args;
} proc_inf;

int work_dir_fd;    // 当前位置的文件描述符
char work_dir[256]; // 当前所在位置
unsigned int open_f(char *file_path)
{
    unsigned int return_value;
    asm volatile(
        "movl $100, %%eax\n"
        "movl %0, %%ebx\n"
        "movl %1, %%ecx\n"
        "int $0x30\n"
        :
        : "r"(file_path), "r"(&return_value)
        : "%eax", "%ebx", "%ecx");
    return return_value;
}

void close_f(int fd)
{
    asm volatile(
        "movl $103, %%eax\n"
        "movl %0, %%ebx\n"
        "int $0x30\n"
        :
        : "r"(fd)
        : "%eax", "%ebx");
}

// sector 扇区
int read_f(int fd, char *buff, int sector, int size)
{
    f_param fp;
    fp.fd = fd;
    fp.addr = sector;
    fp.size = size;
    fp.buf = buff;

    int read_size;
    asm volatile(
        "movl $101, %%eax\n"
        "movl %0, %%ebx\n"
        "movl %1, %%ecx\n"
        "int $0x30\n"
        :
        : "r"(&fp), "r"(&read_size)
        : "%eax", "%ebx", "%ecx");
    return read_size;
}

// write
int write_f(int fd, char *buff, int sector, int size)
{
    f_param fp;
    fp.fd = fd;
    fp.addr = sector;
    fp.size = size;
    fp.buf = buff;
    int read_size;

    asm volatile(
        "movl $102, %%eax\n"
        "movl %0, %%ebx\n"
        "movl %1, %%ecx\n"
        "int $0x30\n"
        :
        : "r"(&fp), "r"(&read_size)
        : "%eax", "%ebx", "%ecx");
    return read_size;
}

bool make_d(int fd, char *dir_name)
{
    f_param fp;
    fp.fd = fd;
    fp.buf = dir_name;

    char return_value;
    asm volatile(
        "movl $104, %%eax\n"
        "movl %0, %%ebx\n"
        "movl %1, %%ecx\n"
        "int $0x30\n"
        :
        : "r"(&fp), "r"(&return_value)
        : "%eax", "%ebx", "%ecx");
    return return_value;
}

bool make_f(int fd, char *dir_name)
{
    f_param fp;
    fp.fd = fd;
    fp.buf = dir_name;

    char return_value;
    asm volatile(
        "movl $105, %%eax\n"
        "movl %0, %%ebx\n"
        "movl %1, %%ecx\n"
        "int $0x30\n"
        :
        : "r"(&fp), "r"(&return_value)
        : "%eax", "%ebx", "%ecx");
    return return_value;
}

bool remove_f(int fd, char *file_name)
{
    f_param fp;
    fp.fd = fd;
    fp.buf = file_name;

    char return_value;
    asm volatile(
        "movl $106, %%eax\n"
        "movl %0, %%ebx\n"
        "movl %1, %%ecx\n"
        "int $0x30\n"
        :
        : "r"(&fp), "r"(&return_value)
        : "%eax", "%ebx", "%ecx");
    return return_value;
}

unsigned int get_ft(int fd)
{
    f_param fp;
    fp.fd = fd;
    unsigned int return_value;
    asm volatile(
        "movl $108, %%eax\n"
        "movl %0, %%ebx\n"
        "movl %1, %%ecx\n"
        "int $0x30\n"
        :
        : "r"(&fp), "r"(&return_value)
        : "%eax", "%ebx", "%ecx");
    return return_value;
}
bool remove_d(int fd, char *dir_name)
{
    f_param fp;
    fp.fd = fd;
    fp.buf = dir_name;

    char return_value;
    asm volatile(
        "movl $107, %%eax\n"
        "movl %0, %%ebx\n"
        "movl %1, %%ecx\n"
        "int $0x30\n"
        :
        : "r"(&fp), "r"(&return_value)
        : "%eax", "%ebx", "%ecx");
    return return_value;
}
char file_buff[512];
char reslove_ls(char *cmd, char *args)
{
    // 在本地文件夹
    int sec_index = 0;
    dir_en *entries = file_buff;
    int32_t file_count = 0;
    dir_en output_dir_en_buff[2]; // 输出缓冲区
    int dir_en_buff_length = 0;
    while (read_f(work_dir_fd, file_buff, sec_index, 512) != 0)
    {

        for (int i = 0; i < 512 / 24; i++)
        {
            if (entries[i].f_type != FT_UNKNOWN)
            {
                memcpy_(output_dir_en_buff + dir_en_buff_length, entries + i, sizeof(dir_en));
                file_count++;
                dir_en_buff_length++;
                if (dir_en_buff_length == 2)
                {
                    char *t1;
                    if (output_dir_en_buff[0].f_type == FT_REGULAR)
                    {
                        t1 = "file";
                    }
                    else
                    {
                        t1 = "dir";
                    }
                    char *t2;
                    if (output_dir_en_buff[1].f_type == FT_REGULAR)
                    {
                        t2 = "file";
                    }
                    else
                    {
                        t2 = "dir";
                    }
                    // 清空buff 64字节的数据
                    for (int j = 0; j < 64; j++)
                    {
                        output_buff[j] = ' ';
                    }
                    output_buff[64] = '\0';
                    output_buff[63] = '\n';
                    char *temp_buff = output_buff + 66;
                    sprintf_(temp_buff, "  %s [%s]", output_dir_en_buff[0].filename, t1);
                    memcpy_(output_buff, temp_buff, strlen_(temp_buff));
                    sprintf_(temp_buff, "  %s [%s]", output_dir_en_buff[1].filename, t2);
                    memcpy_(output_buff + 30, temp_buff, strlen_(temp_buff));
                    write_stdio(output_buff, strlen_(output_buff));
                    dir_en_buff_length = 0;
                }
            }
        }
        sec_index++;
    }
    if (dir_en_buff_length != 0)
    {
        char *t1;
        if (output_dir_en_buff[0].f_type == FT_REGULAR)
        {
            t1 = "file";
        }
        else
        {
            t1 = "dir";
        }
        sprintf_(output_buff, "  %s [%s]", output_dir_en_buff[0].filename, t1);
        write_stdio(output_buff, strlen_(output_buff));
    }

    sprintf_(output_buff, "\ntotal   %d  \n", file_count);
    write_stdio(output_buff, strlen_(output_buff));
    return 1;
}
char reslove_cd(char *cmd, char *args)
{

    // 处理args
    char *file_path = args;
    // 解析参数
    for (int index = 0; index < 256; index++)
    {
        if (args[index] == ' ')
        {
            file_path = args + index + 1;
        }
        else
        {
            break;
        }
    }
    if (file_path[0] == '\0')
    {
        return 0;
    }
    int work_dir_length = strlen_(work_dir);
    for (int i = 0; i < 256; i++)
    {
        if (file_path[i] == ' ' || file_path[i] == '\0')
        {
            file_path[i] = '\0';
            if (file_path[i - 1] == '/')
            {
                // 处理 /hello/ 的情况
                if (i != 1)
                {
                    file_path[i - 1] = '\0';
                }
            }
            break;
        }
    }
    if (strcmp_(".", file_path) == 0)
    {
        return true;
    }
    if (strcmp_("..", file_path) == 0)
    {

        if (work_dir[work_dir_length - 1] == '/')
        {
            return true;
        }

        // 找到最后一个 / 重置为 \0
        for (int i = work_dir_length - 1; i >= 0; i--)
        {
            if (work_dir[i] == '/')
            {
                close_f(work_dir_fd); // 关闭原来的fd
                // file_buff得到新的工作目录
                if (i == 0)
                {
                    work_dir[1] = '\0';
                }
                else
                {
                    work_dir[i] = '\0';
                }
                work_dir_fd = open_f(work_dir);
                return true;
            }
        }
    }

    // 判断是否基于当前目录
    if (file_path[0] != '/')
    {
        // 基于当前目录
        if (work_dir_length == 1)
        {
            sprintf_(file_buff, "/%s", file_path);
        }
        else
        {
            sprintf_(file_buff, "%s/%s", work_dir, file_path);
        }
    }
    else
    {
        sprintf_(file_buff, file_path);
        if (strcmp_(file_buff, work_dir) == 0)
        {
            return true;
        }
    }

    // file_buff得到新的工作目录
    int new_fd = open_f(file_buff);

    if (new_fd == 1024)
    {
        // 检查文件描述符是否有效
        sprintf_(output_buff, "cd: No such directory (%s)\n", file_buff);
        write_stdio(output_buff, strlen_(output_buff));
        return false;
    }
    // 检查文件类型是否是文件夹
    unsigned int ft = get_ft(new_fd);
    if (ft != FT_DIRECTORY)
    {
        // 检查文件描述符是否有效
        sprintf_(output_buff, "cd: No such directory (%s)\n", file_buff);
        write_stdio(output_buff, strlen_(output_buff));
        close_f(new_fd);
        return false;
    }

    // 满足切换条件
    close_f(work_dir_fd); // 关闭原来的fd
    work_dir_fd = new_fd;
    strcpy_(work_dir, file_buff);
    return true;
}
char reslove_mkdir(char *cmd, char *args)
{
    char *file_name = args;
    // 解析参数
    for (int index = 0; index < 256; index++)
    {
        if (args[index] == ' ')
        {
            file_name = args + index + 1;
            continue;
        }
        else
        {
            break;
        }
    }
    if (file_name[0] == '\0')
    {
        return 0;
    }

    for (int i = 0; i < 256; i++)
    {
        if (file_name[i] == ' ' || file_name[i] == '\0')
        {
            file_name[i] = '\0';
            break;
        }
    }

    return make_d(work_dir_fd, file_name);
}
char reslove_mkfile(char *cmd, char *args)
{
    char *file_name = args;
    // 解析参数
    for (int index = 0; index < 256; index++)
    {
        if (args[index] == ' ')
        {
            file_name = args + index + 1;
            continue;
        }
        else
        {
            break;
        }
    }
    if (file_name[0] == '\0')
    {
        return 0;
    }

    for (int i = 0; i < 256; i++)
    {
        if (file_name[i] == ' ' || file_name[i] == '\0')
        {
            file_name[i] = '\0';
            break;
        }
    }

    return make_f(work_dir_fd, file_name);
}
char reslove_rm(char *cmd, char *args)
{
    char *file_name = args;
    // 解析参数
    for (int index = 0; index < 256; index++)
    {
        if (args[index] == ' ')
        {
            file_name = args + index + 1;
            continue;
        }
        else
        {
            break;
        }
    }
    if (file_name[0] == '\0')
    {
        return 0;
    }

    for (int i = 0; i < 256; i++)
    {
        if (file_name[i] == ' ' || file_name[i] == '\0')
        {
            file_name[i] = '\0';
            break;
        }
    }

    if (!remove_f(work_dir_fd, file_name))
    {
        sprintf_(output_buff, "%s is not a file type \n", file_name);
        write_stdio(output_buff, strlen_(output_buff));
    }
}
char reslove_rmdir(char *cmd, char *args)
{
    char *dir_name = args;
    // 解析参数
    for (int index = 0; index < 256; index++)
    {
        if (args[index] == ' ')
        {
            dir_name = args + index + 1;
            continue;
        }
        else
        {
            break;
        }
    }
    if (dir_name[0] == '\0')
    {
        return 0;
    }

    for (int i = 0; i < 256; i++)
    {
        if (dir_name[i] == ' ' || dir_name[i] == '\0')
        {
            dir_name[i] = '\0';
            break;
        }
    }

    if (!remove_d(work_dir_fd, dir_name))
    {
        sprintf_(output_buff, "%s is not a directory type \n", dir_name);
        write_stdio(output_buff, strlen_(output_buff));
    }
}

char reslove_exec(char *cmd, char *args)
{
    write_stdio("exec\n", 5);
}

#define CMD_LIST_LENGTH 16
command cmd_list[CMD_LIST_LENGTH] = {
    {.cmd = "ls", .resolve = reslove_ls},
    {.cmd = "cd", .resolve = reslove_cd},
    {.cmd = "mkdir", .resolve = reslove_mkdir},
    {.cmd = "mkfile", .resolve = reslove_mkfile},
    {.cmd = "rm", .resolve = reslove_rm},
    {.cmd = "rmdir", .resolve = reslove_rmdir},
    {.cmd = "exec", .resolve = reslove_exec},
};
// 参数解析
char resolve(char *cmd, char *args)
{
    for (int i = 0; i < CMD_LIST_LENGTH; i++)
    {
        if (cmd_list[i].cmd != NULL && strcmp_(cmd, cmd_list[i].cmd) == 0 && cmd_list[i].resolve != NULL)
        {
            cmd_list[i].resolve(cmd, args);
            return true;
        }
    }
    return false;
}
// 根据传入参数解析命令行并响应,返回是否退出shell程序,1退出
char resolve_and_respond(char *command)
{
    // 首先确定是那一条指令,然后根据指令确定其参数
    char *cmd = command;
    char *args = command;
    // 确定cmd_start
    for (int index = 0; index < 256; index++)
    {
        if (command[index] == ' ')
        {
            cmd = command + index + 1;
            continue;
        }
        else
        {
            break;
        }
    }
    if (cmd[0] == '\0')
    {
        return false;
    }
    // 确定cmd的结尾
    for (int index = 0; index < 256; index++)
    {
        if (cmd[index] == ' ' || cmd[index] == '\0')
        {
            cmd[index] = '\0';
            args = cmd + index + 1; // 如果args恰好在哨兵位置,说明到达输入结尾
            break;
        }
    }
    // 确定调用那个命令去处理
    if (!resolve(cmd, args))
    {
        {
            sprintf_(output_buff, "%s : command not found\n", cmd);
            write_stdio(output_buff, strlen_(output_buff));
        }
    }
}
// shell 实现
// 一个个从键盘读取字符然后往控制台丢数据,同时记录读取到数据,当按下回车的时候,解析读取到的数据

void shell()
{
    char data;
    char cmd_str[256]; // 最多256个字符,不能再多了
    int cmd_length;
    char *prompt = "roodOs@ekko>";

    work_dir[0] = '/';
    work_dir[1] = '\0';
    work_dir_fd = open_f(work_dir);
    open_stdio();

    while (1)
    {
        // 写入标识
        cmd_length = 0;
        sprintf_(output_buff, "%s:%s >", prompt, work_dir);
        write_stdio(output_buff, strlen_(output_buff));

        while (1)
        {
            // 读取到如果是回车则往缓冲区中放数据
            read_stdio(&data, 1);

            if (data == '\r' || cmd_length == 253)
            {
                data = '\n';
                write_stdio(&data, 1); // 写入换行
                cmd_str[cmd_length] = '\0';
                cmd_str[cmd_length + 1] = '\0'; // 哨兵防止溢出
                break;
            }

            if (data == '\b')
            {

                if (cmd_length != 0)
                {
                    cmd_length--;
                    write_stdio(&data, 1);
                }
                // 如果buff_size为0不管
            }
            else
            {
                cmd_str[cmd_length] = data;
                cmd_length++;
                write_stdio(&data, 1);
            }
        }

        // 解析命令
        resolve_and_respond(cmd_str);

        // 不提供退出命令行的方式
    }
}

void *malloc_page(unsigned int page_size)
{
    void *return_value;
    asm volatile(
        "movl $200, %%eax\n"
        "movl %0, %%ebx\n"
        "movl %1, %%ecx\n"
        "int $0x30\n"
        :
        : "r"(page_size), "r"(&return_value)
        : "%eax", "%ebx", "%ecx");
    return return_value;
}
unsigned short exec_(unsigned short weight, char *exec_file_path, uint16_t argsLength)
{
    proc_inf p;
    p.weight = weight;
    p.exec_file_path = exec_file_path;
    p.argsLength = argsLength;
    unsigned short PID;
    asm volatile(
        "movl $13, %%eax\n"
        "movl %0, %%ebx\n"
        "movl %1, %%ecx\n"
        "int $0x30\n"
        :
        : "r"(&p), "r"(&PID)
        : "%eax", "%ebx", "%ecx");
    return PID;
}
void exit()
{
    asm volatile(
        "movl $12, %%eax\n"
        "int $0x30\n"
        :
        :
        :);
}
extern uint16_t createProcess3(uint16_t weight, char *exec_file_path, uint16_t argsLength, ...);
void function()
{
    // shell();
    for (uint32_t i = 0; i < 10000; i++)
    {
        i++;
    }
    // int fd = open_f("/test");
    // char *bu = malloc_page(3);
    // readDis(bu, 10, 410);
    // write_f(fd, bu, 0, 3 * 4096);
    exec_(7, "/test", 0);
    int i = 0;
    exit();
    while (1)
    {
        i++;
    }
}

//-----------------------------------------------------------------------------
// 进程结束
void exit_pro()
{
    manager.ticks++;

    PCB *p = manager.now;
    p->runtime++;
    // 判断是否要切换
    if (p->runtime <= p->weight)
    {
        return;
    }
    p->vruntime++;
    p->runtime = 0;
    p->status = END;

    // insertWait(p);
    // 找到下一个进程
    manager.now = theNextProcess();
    manager.minVruntime = manager.now->vruntime;
    manager.now->status = RUNNING;
    // 修改tss 0特权级栈
    update_tss_esp(Tss, manager.now->esp0);

    // 切换栈
    asm volatile(
        "movl %0, %%eax\n "
        "movl %%eax, %%esp\n "
        :
        : "r"((manager.now->esp0) - sizeof(StackInfo)) // 输入操作数：myVar表示源操作数
    );

    // 切换页表
    switchUserPage(market.virMemPool, &(manager.now->u), manager.now->pagePAddr, manager.now->pageVAddr);
    // 回收进程
    destroyPCB(p);

    switchProcess();
}

/*
    获取可用PID
    创建PCB:分配PCB内存,分配0特权级栈内存
    切换到对应PCB页表,解析用户程序头确定分配内存,展开用户程序到指定内存
    初始化0特权级栈:EIP
*/
uint16_t createProcess(uint16_t weight, uint16_t argsLength, char *name, ...)
{
    // 创建pcb,读取磁盘,设置esp0
    uint16_t pid = getPID();
    if (pid == 0)
    {
        return 0;
    }

    // 创建PCB:pcb页,以及0特权级栈
    PCB *pcb = createPCB(name, pid, weight);
    usePID(pcb, pid);

    // 切换到用户页表项
    switchUserPage(market.virMemPool, &pcb->u, pcb->pagePAddr, pcb->pageVAddr);

    /*
        todo 24 1 31:
        根据名称在磁盘加载用户程序到内存,这里不涉及加载
        直接将内核某个函数设置为用户程序,同时直接分配内存
    */
    uint32_t paddr;
    uint32_t me = mallocMultpage_u(&market, 1);
    StackInfo *s = (StackInfo *)(pcb->esp0 - sizeof(StackInfo));
    // 初始化0特权级栈
    initStack(s, function, (me) + 4096 * (1) - 32 - argsLength * sizeof(char) - 4);

    //------todo 24 1 31 -------
    /*
        正确性有待具体测试,目前看来缺陷很大
        根据传入参数个数,将数据压入用户栈中
    */
    va_list args;               // 定义参数
    va_start(args, argsLength); // 初始化
    memcpy_(s->oldESP + 4, name, argsLength);
    va_end(args);
    //-----------------------------

    // 设置pcb为就绪
    pcb->status = WAIT;
    // 往就绪队列中加入该pcb
    insertWait(pcb);

    // 切回原用户
    switchUserPage(market.virMemPool, &(manager.now->u), manager.now->pagePAddr, manager.now->pageVAddr);
    return pid;
}

uint16_t createProcess2(uint16_t weight, uint16_t argsLength, char *name, ...)
{
    // 创建pcb,读取磁盘,设置esp0
    uint16_t pid = getPID();
    if (pid == 0)
    {
        return 0;
    }

    // 创建PCB:pcb页,以及0特权级栈
    PCB *pcb = createPCB(name, pid, weight);
    usePID(pcb, pid);

    // 切换到用户页表项
    switchUserPage(market.virMemPool, &pcb->u, pcb->pagePAddr, pcb->pageVAddr);

    /*
        todo 24 1 31:
        根据名称在磁盘加载用户程序到内存,这里不涉及加载
        直接将内核某个函数设置为用户程序,同时直接分配内存
    */
    uint32_t paddr;

    // 分配所需内存
    uint32_t me = mallocMultpage_u(&market, 20);
    StackInfo *s = (StackInfo *)(pcb->esp0 - sizeof(StackInfo));
    // 读取程序进入内存

    readDis(me, 10, 410);

    // 初始化0特权级栈
    initStack(s, 0, (me) + 4096 * (20) - 32 - argsLength * sizeof(char) - 4);

    //------todo 24 1 31 -------
    /*
        正确性有待具体测试,目前看来缺陷很大
        根据传入参数个数,将数据压入用户栈中
    */
    va_list args;               // 定义参数
    va_start(args, argsLength); // 初始化
    memcpy_(s->oldESP + 4, name, argsLength);
    va_end(args);
    //-----------------------------

    // 设置pcb为就绪
    pcb->status = WAIT;
    // 往就绪队列中加入该pcb
    insertWait(pcb);

    // 切回原用户
    switchUserPage(market.virMemPool, &(manager.now->u), manager.now->pagePAddr, manager.now->pageVAddr);
    return pid;
}

extern bool search_file_by_path(char *file_path, uint32_t *ino, uint32_t *file_type);
extern uint32_t size_of_file(uint32_t inode_no);
extern int32_t read_file(uint32_t inode_no, uint32_t addr, char *buf, uint32_t size);
uint16_t createProcess3(uint16_t weight, char *exec_file_path, uint16_t argsLength, ...)
{

    // path中提取名称
    char name[20];

    uint32_t path_length = strlen_(exec_file_path);
    for (int32_t i = path_length - 1; i >= 0; i--)
    {
        if (exec_file_path[i] == '/')
        {
            if (i == path_length - 1)
            {
                return 0;
            }
            memcpy_(name, exec_file_path + i + 1, path_length - 1 - i);
        }
    }

    // 创建pcb,读取磁盘,设置esp0
    uint16_t pid = getPID();
    if (pid == 0)
    {
        return 0;
    }

    // 创建PCB:pcb页,以及0特权级栈

    PCB *pcb = createPCB(name, pid, weight);
    usePID(pcb, pid);

    // 切换到用户页表项
    switchUserPage(market.virMemPool, &pcb->u, pcb->pagePAddr, pcb->pageVAddr);

    /*
        todo 24 1 31:
        根据名称在磁盘加载用户程序到内存,这里不涉及加载
        直接将内核某个函数设置为用户程序,同时直接分配内存
    */
    uint32_t paddr;

    uint32_t ino;
    uint32_t ftype;
    if (!search_file_by_path(exec_file_path, &ino, &ftype))
    {
        return 0;
    }
    if (ftype != FT_REGULAR)
    {
        return 0;
    }

    uint32_t file_size = size_of_file(ino);
    if (file_size == 0)
    {
        return 0;
    }

    // 计算要分配多少页
    uint32_t code_page = file_size % 4096 == 0 ? file_size / 4096 : file_size / 4096 + 1;

    // 分配所需内存
    uint32_t me = mallocMultpage_u(&market, code_page + 2);
    StackInfo *s = (StackInfo *)(pcb->esp0 - sizeof(StackInfo));
    // 读取程序进入内存

    read_file(ino, 0, me, file_size);

    // 初始化0特权级栈
    initStack(s, 0, (me) + 4096 * (code_page + 2) - 32 - argsLength * sizeof(char) - 4);

    //------todo 24 1 31 -------
    /*
        正确性有待具体测试,目前看来缺陷很大
        根据传入参数个数,将数据压入用户栈中
    */
    va_list args;               // 定义参数
    va_start(args, argsLength); // 初始化
    memcpy_(s->oldESP + 4, name, argsLength);
    va_end(args);
    //-----------------------------

    // 设置pcb为就绪
    pcb->status = WAIT;
    // 往就绪队列中加入该pcb
    insertWait(pcb);

    // 切回原用户
    switchUserPage(market.virMemPool, &(manager.now->u), manager.now->pagePAddr, manager.now->pageVAddr);
    return pid;
}
void destroy_PCB_waitQ(PCB *pcb)
{
    // 是不是应该检查一下删除的pcb是否有效?

    // 从对应的度队列中删除pcb
    // 现在只有等待队列
    removeProcess(pcb);
    // 在父亲节点中删除pcb
    PCB *f = (PCB *)(pcb->father);
    deleteNode(&(f->children), &(pcb->tag));

    // 任务列表中删除
    manager.task[pcb->id] = 0;

    // 处理其子进程,合并到init线程
    mergeList(&(manager.init->children), &(pcb->children));
    /*首先回收用户内存
        // 搜索用户非空页表项
        // 找到非空页表项,释放其页表项的页目录中物理内存
        // 释放页目录项对应内存
    */

    switchUserPage(market.virMemPool, &(pcb->u), pcb->pagePAddr, pcb->pageVAddr);
    userPageDir *u = &(pcb->u);
    uint32_t PTPaddr;
    uint32_t temp;
    uint16_t count;
    for (uint16_t i = 0; i < 768; i++)
    {
        if (u->PT[i] >= 0)
        {

            // 获取物理页
            PTPaddr = getPDE(market.virMemPool, i) >> 12 << 12;
            // 说明分配有内存,至少分配有页目录
            if (u->PT[i] > 0)
            {
                // 清理页
                count = 0;
                for (uint16_t j = 0; j < 1024; j++)
                {

                    temp = getPTE(PTPaddr, j) & 0xFFFFF000;
                    if (temp != 0)
                    {
                        ReturnPhyPage(market.phyPool, temp);
                        count++;
                    }
                    if (count == u->PT[i])
                    {
                        break;
                    }
                }
            }
            // 归还
            ReturnPhyPage(market.phyPool, PTPaddr);
        }
    }

    switchUserPage(market.virMemPool, &manager.now->u, manager.now->pagePAddr, manager.now->pageVAddr);
    // 回收页表内存
    freePage(&market, pcb->pageVAddr);
    // 回收esp0内存
    freePage(&market, pcb->esp0);
    // 回收文件描述符表内存
    freePage(&market, pcb->file_descriptors);
    // 回收pcb占用内存
    freePage(&market, pcb);
}
// 将某个pcb占用的内存回收,其他进程对某个进程pcb的回收
// 进程自己结束自己
void destroyPCB(PCB *pcb)
{
    // 是不是应该检查一下删除的pcb是否有效?

    // 从对应的度队列中删除pcb
    // 现在只有等待队列 todo : 需要考虑其他队列的情况
    // 结束进程只可能是自己结束自己,显然不需要去移除pcb
    // removeProcess(pcb);
    // 在父亲节点中删除pcb
    PCB *f = (PCB *)(pcb->father);
    deleteNode(&(f->children), &(pcb->tag));

    // 任务列表中删除
    manager.task[pcb->id] = 0;

    // 处理其子进程,合并到init线程
    mergeList(&(manager.init->children), &(pcb->children));
    /*首先回收用户内存
        // 搜索用户非空页表项
        // 找到非空页表项,释放其页表项的页目录中物理内存
        // 释放页目录项对应内存
    */

    switchUserPage(market.virMemPool, &(pcb->u), pcb->pagePAddr, pcb->pageVAddr);
    userPageDir *u = &(pcb->u);
    uint32_t PTPaddr;
    uint32_t temp;
    uint16_t count;
    for (uint16_t i = 0; i < 768; i++)
    {
        if (u->PT[i] >= 0)
        {

            // 获取物理页
            PTPaddr = getPDE(market.virMemPool, i) >> 12 << 12;
            // 说明分配有内存,至少分配有页目录
            if (u->PT[i] > 0)
            {
                // 清理页
                count = 0;
                for (uint16_t j = 0; j < 1024; j++)
                {

                    temp = getPTE(PTPaddr, j) & 0xFFFFF000;
                    if (temp != 0)
                    {
                        ReturnPhyPage(market.phyPool, temp);
                        count++;
                    }
                    if (count == u->PT[i])
                    {
                        break;
                    }
                }
            }
            // 归还
            ReturnPhyPage(market.phyPool, PTPaddr);
        }
    }

    switchUserPage(market.virMemPool, &manager.now->u, manager.now->pagePAddr, manager.now->pageVAddr);
    // 回收页表内存
    freePage(&market, pcb->pageVAddr);
    // 回收esp0内存
    freePage(&market, pcb->esp0);
    // 回收文件描述符表内存
    freePage(&market, pcb->file_descriptors);
    // 回收pcb占用内存
    freePage(&market, pcb);
}

// 进程拷贝

// pcb共享==>线程

// 进程模块

// 切换到3特权级,在等待队列中至少一个用户程序
void switch_to_user_mode()
{
    // 找到一个可运行的进程,切换到对应进程==>切换到3特权级
    manager.now = theNextProcess();
    switchUserPage(market.virMemPool, &manager.now->u, manager.now->pagePAddr, manager.now->pageVAddr);
    update_tss_esp(Tss, manager.now->esp0);
    switchProcess();
}

// init进程在内核态执行,可以做一些需要在内核完成的事情
// 没有任何进程,也许是都阻塞了,死锁!!
// todo 24/3/2 栈恢复时有问题,但暂时就先这样放着了
void initFunction()
{
    uint32_t i = 0;
    while (1)
    {
        i++;
        if (i % 100 == 0)
        {
            log("no process , init process is running:%d\n", i);
        }
    }
}
void initProcess(TSS *tss, GDT *gdt)
{
    // 初始化task
    for (uint16_t i = 1; i < TASKSIZE; i++)
    {
        manager.task[i] = 0;
    }

    initTss(tss, gdt);

    // 初始化调度器
    initSchedule();

    // 初始化init线程,设置pcb
    PCB *initP = (PCB *)INITPCB;
    initP->id = 0;
    manager.task[0] = initP;
    manager.ticks = 0;
    initP->pagePAddr = market.virMemPool->paddr;
    initP->pageVAddr = market.virMemPool->vaddr;
    initP->esp0 = 0xc0008000 + 4080;
    char *name = "init";
    memcpy_(initP->name, name, strlen_(name));
    initP->blockTag.data = initP;
    initP->weight = 1;
    memcpy_(&(initP->u), market.virMemPool->userPD, sizeof(userPageDir));
    manager.now = initP;
    manager.init = initP;
    initLinkedList(&(manager.now->children));

    StackInfo *s = initP->esp0 - sizeof(StackInfo);
    s->EIP = initFunction;
    s->IVN = 32;
    s->EFLAGS = (EFLAGS_IOPL_0 | EFLAGS_MBS | EFLAGS_IF_1);
    s->CS = 0b1000;
    s->oldSS = 0;
    s->oldESP = 0;
    s->GS = 0b10000;
    s->FS = 0b10000;
    s->ES = 0b10000;
    s->DS = 0b10000;
}
