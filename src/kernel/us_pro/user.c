
#include "../include/str.h"
void print(char *mess)
{
    asm volatile(
        "movl $1, %%eax\n"
        "movl %0, %%ebx\n"
        "int $0x30\n"
        :
        : "r"(mess)
        : "%eax", "%ebx");
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
int main()
{
    int i = 0;
    char buff[128];
    // print("Test successful !\n");
    // exit();
    while (1)
    {
        i++;
        sprintf_(buff, "i=%d\n", i);
        // print(buff);
    }
    return 0;
}
