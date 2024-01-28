

int main()
{
    char *test = "hello roodos ! \n";
    while (1)
    {

        asm volatile(
            "movl %0, %%eax\n "
            "int $0x30\n "
            :
            : "r"(test));
    }
    return 0;
}
