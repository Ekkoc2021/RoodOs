
#include "../include/str.h"
// 测试设备管理系统调用
typedef struct
{
    char *buf;
    unsigned int typeId;
    unsigned int deviceId;
    unsigned int addr;
    unsigned int size;
} devParam_;

int main()
{
    // char *test = "hello roodos ! \n";
    // while (1)
    // {

    //     asm volatile(
    //         "movl %0, %%eax\n "
    //         "int $0x30\n "
    //         :
    //         : "r"(test));
    // }

    char buff[128];
    devParam_ d;
    d.typeId = 1;
    d.deviceId = 0;
    d.buf = buff;
    d.size = 128;
    asm volatile(
        "movl $60, %%eax\n"
        "movl %0, %%ebx\n"
        "int $0x30\n"
        :
        : "r"(&d)
        : "%eax", "%ebx");

    while (1)
    {
        sprintf_(buff, "roodos$>");
        unsigned int len = strlen_(buff);
        d.buf = buff + len;
        d.size = 1;
        asm volatile(
            "movl $61, %%eax\n"
            "movl %0, %%ebx\n"
            "int $0x30\n"
            :
            : "r"(&d)
            : "%eax", "%ebx");
        buff[len + 1] = '\n';
        buff[len + 2] = '\0';
        d.buf = buff;
        asm volatile(
            "movl $62, %%eax\n"
            "movl %0, %%ebx\n"
            "int $0x30\n"
            :
            : "r"(&d)
            : "%eax", "%ebx");

        // asm volatile(
        //     "movl $53, %%eax\n"
        //     "movl %0, %%ebx\n"
        //     "int $0x30\n"
        //     :
        //     : "r"(seId)
        //     : "%eax", "%ebx");
        /* code */
    }

    return 0;
}
