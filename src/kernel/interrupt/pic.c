#include "pic.h"
// 中断控制器相关函数

// 初始化
void initPIC()
{
    log("start initializing pic..\n");
    /* 初始化主片 */
    outb(PIC_M_CTRL, 0x11); // ICW1: 边沿触发,级联8259, 需要ICW4.
    outb(PIC_M_DATA, 0x20); // ICW2: 起始中断向量号为0x20,也就是IR[0-7] 为 0x20 ~ 0x27.
    outb(PIC_M_DATA, 0x04); // ICW3: IR2接从片.
    outb(PIC_M_DATA, 0x01); // ICW4: 8086模式, 正常EOI
    log("The starting vector number of the master is 0x20 \n");

    /* 初始化从片 */
    outb(PIC_S_CTRL, 0x11); // ICW1: 边沿触发,级联8259, 需要ICW4.
    outb(PIC_S_DATA, 0x28); // ICW2: 起始中断向量号为0x28,也就是IR[8-15] 为 0x28 ~ 0x2F.
    outb(PIC_S_DATA, 0x02); // ICW3: 设置从片连接到主片的IR2引脚
    outb(PIC_S_DATA, 0x01); // ICW4: 8086模式, 正常EOI
    log("The starting vector number of the slaver is 0x28 \n");

    // 设置为全关闭
    outb(PIC_M_DATA, 0xff);
    outb(PIC_S_DATA, 0xff);
    log("Initial pic successful ! \n");
}

// 读取主片的IMR端口的值,查看主片哪些中断开启
unsigned char readMasterIMR()
{
    outb(0x20, 0x0A); // 发送 ICW3 到主片，选择读取 IMR 的操作
    return inb(0x21); // 读取 IMR 寄存器的值
}

// 读取从片的 IMR 寄存器
unsigned char readSlaveIMR()
{
    // 发送特殊命令到从片
    outb(0xA0, 0x0B);
    return inb(0xA1); // 读取从片 IMR 寄存器的值
}

void startTimerInterrupt()
{
    log("start clock interrupts...\n");
    unsigned char slaveIMR = readSlaveIMR();
    unsigned char masterIMR = readMasterIMR();
    /* 打开主片上IR0,也就是目前只接受时钟产生的中断 */
    outb(PIC_M_DATA, 0xfe); // 0为开启,1为关闭,打开master的1号中断,也就是0x20
    outb(PIC_S_DATA, 0xff); //

    unsigned char newSlaveIMR = readSlaveIMR();
    // 输出二进制表示中哪些 IRQ 线被屏蔽
    log("Slave IMR Value (binary): ");
    for (int16_t i = 7; i >= 0; i--)
    {
        printf("%d", (slaveIMR & (1 << i)) ? 1 : 0);
    }
    log("==>");
    for (int16_t i = 7; i >= 0; i--)
    {
        log("%d", (newSlaveIMR & (1 << i)) ? 1 : 0);
    }

    log("\n");
    unsigned char newMasterIMR = readMasterIMR();

    // 输出二进制表示中哪些 IRQ 线被屏蔽
    log("Master IMR Value (binary): ");
    for (int16_t i = 7; i >= 0; i--)
    {
        log("%d", (masterIMR & (1 << i)) ? 1 : 0);
    }
    log("==>");
    for (int16_t i = 7; i >= 0; i--)
    {
        log("%d", (newMasterIMR & (1 << i)) ? 1 : 0);
    }
    log("\n");
    log("start clock interrupts successful ...\n");
}
