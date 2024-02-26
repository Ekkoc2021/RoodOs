# make默认环境linux
OUTPUT=
#----------默认Linux下配置--
CC=gcc
LD=ld

MASTERDISK=bochsconfig/disk1.img
#--------------------------

BOOTLOADER=src/bootloader
KERNEL=src/kernel

DATAS=src/dataS

KERNEENTRY=0xc0011000  #c000_0000 内核入口地址,与boot加载位置和,loader跳转位置有关

LDFLAGS=-Ttext $(KERNEENTRY) -m elf_i386 -static
# CFLAGS=-m32 \
# 	-O0 \
# 	-static \
# 	-fno-asynchronous-unwind-tables \
# 	-fno-pic \
# 	-fno-pie  \
# 	-fno-stack-protector \
# 	-fno-builtin \
# 	-nostdlib \
# 	-nodefaultlibs \
# 	-nostdinc \
# 	-nostartfiles  
CFLAGS=-m32 \
	-fno-pic \
	-fno-pie  \
	-fno-stack-protector \
	-fno-builtin \
	-nostdlib \
	-nostdinc \
# CFLAGS:= -m32 					# 32 位的程序
# CFLAGS+= -march=pentium			# pentium 处理器
# CFLAGS+= -fno-builtin			# 不需要 gcc 内置函数
# CFLAGS+= -nostdinc				# 不需要标准头文件
# CFLAGS+= -fno-pic				# 不需要位置无关的代码  position independent code
# CFLAGS+= -fno-pie				# 不需要位置无关的可执行程序 position independent executable
# CFLAGS+= -nostdlib				# 不需要标准库
# CFLAGS+= -fno-stack-protector	# 不需要栈保护

DEBUG=-g # gcc调试参数,可以debug
INCLUDE=-Iinclude # 搜索编译的头文件的目录


# start : bootloader $(OUTPUT)system.bin sym $(OUTPUT)system.map

start : bootloader $(OUTPUT)system.bin sym $(OUTPUT)system.map

kernel: $(OUTPUT)system.bin sym

# 有bochs
bochsDebug : bootloader $(OUTPUT)system.bin sym startBochsdbg
	
bootloader : $(OUTPUT)boot.bin $(OUTPUT)loader.bin
 
sym : $(OUTPUT)kernel.sym

#--------------user_programer--------

user : $(OUTPUT)user.bin

$(OUTPUT)user0.o : $(KERNEL)/us_pro/user0.asm
	nasm -f elf32 $^ -o $@ $(DEBUG) 

$(OUTPUT)user.o : $(KERNEL)/us_pro/user.c
	$(CC) -c $^ -o $@ $(CFLAGS) $(DEBUG) 

$(OUTPUT)u.bin : $(OUTPUT)user0.o $(OUTPUT)user.o
	$(LD)  -m elf_i386 -static $^ -o $@ -Ttext 0

$(OUTPUT)user.bin: $(OUTPUT)u.bin
	objcopy -O binary $< $@
	-dd conv=notrunc if=$@ of=$(MASTERDISK) bs=512 count=10 seek=410

#------------dataS------------------
$(OUTPUT)str.o : $(DATAS)/str/str.c
	$(CC) -c $^ -o $@ $(CFLAGS) $(DEBUG) 

$(OUTPUT)arrayList.o : $(DATAS)/list/arrayList.c
	$(CC) -c $^ -o $@ $(CFLAGS) $(DEBUG) 

$(OUTPUT)Bitmap.o : $(DATAS)/set/Bitmap.c
	$(CC) -c $^ -o $@ $(CFLAGS) $(DEBUG) 

$(OUTPUT)RBTree.o : $(DATAS)/tree/ST/RBTree.c
	$(CC) -c $^ -o $@ $(CFLAGS) $(DEBUG) 

$(OUTPUT)linkedList.o : $(DATAS)/list/linkedList.c
	$(CC) -c $^ -o $@ $(CFLAGS) $(DEBUG) 

$(OUTPUT)linkedQueue.o : $(DATAS)/stack_queue/linkedQueue.c
	$(CC) -c $^ -o $@ $(CFLAGS) $(DEBUG)

$(OUTPUT)cirQueue.o : $(DATAS)/stack_queue/cirQueue.c
	$(CC) -c $^ -o $@ $(CFLAGS) $(DEBUG)

#------------kernel-----------------

kernelDep=$(OUTPUT)kernel.o $(OUTPUT)roodos.o \
	$(OUTPUT)arrayList.o $(OUTPUT)Bitmap.o $(OUTPUT)str.o $(OUTPUT)RBTree.o $(OUTPUT)linkedList.o $(OUTPUT)linkedQueue.o $(OUTPUT)cirQueue.o \
	$(OUTPUT)assert.o \
	$(OUTPUT)tss.o $(OUTPUT)process.o $(OUTPUT)schedule.o  \
	$(OUTPUT)timer.o $(OUTPUT)pic.o $(OUTPUT)interrupt.o $(OUTPUT)IET.o \
	$(OUTPUT)syscall.o \
	$(OUTPUT)physicalMem.o $(OUTPUT)virtualMem.o $(OUTPUT)memory.o \
	$(OUTPUT)semaphore.o \
	$(OUTPUT)console.o $(OUTPUT)keyboard.o $(OUTPUT)device.o $(OUTPUT)tty.o $(OUTPUT)disk.o\
	$(OUTPUT)fs.o $(OUTPUT)partition.o $(OUTPUT)inode.o\
	$(OUTPUT)kernel0.o \
	$(OUTPUT)io.o

$(OUTPUT)roodos.o : $(KERNEL)/roodos.c
	$(CC) -c $^ -o $@ $(CFLAGS) $(DEBUG) 

$(OUTPUT)fs.o : $(KERNEL)/fs/fs.c
	$(CC) -c $^ -o $@ $(CFLAGS) $(DEBUG) 

$(OUTPUT)partition.o : $(KERNEL)/fs/partition.c
	$(CC) -c $^ -o $@ $(CFLAGS) $(DEBUG) 

$(OUTPUT)inode.o : $(KERNEL)/fs/inode.c
	$(CC) -c $^ -o $@ $(CFLAGS) $(DEBUG)

$(OUTPUT)console.o : $(KERNEL)/device/console.c
	$(CC) -c $^ -o $@ $(CFLAGS) $(DEBUG) 

$(OUTPUT)disk.o : $(KERNEL)/device/disk.c
	$(CC) -c $^ -o $@ $(CFLAGS) $(DEBUG) 

$(OUTPUT)device.o : $(KERNEL)/device/device.c
	$(CC) -c $^ -o $@ $(CFLAGS) $(DEBUG) 

$(OUTPUT)tty.o : $(KERNEL)/device/tty.c
	$(CC) -c $^ -o $@ $(CFLAGS) $(DEBUG) 

$(OUTPUT)semaphore.o : $(KERNEL)/syn/semaphore.c
	$(CC) -c $^ -o $@ $(CFLAGS) $(DEBUG) 

$(OUTPUT)schedule.o : $(KERNEL)/process/schedule.c
	$(CC) -c $^ -o $@ $(CFLAGS) $(DEBUG) 

$(OUTPUT)process.o : $(KERNEL)/process/process.c
	$(CC) -c $^ -o $@ $(CFLAGS) $(DEBUG) 

$(OUTPUT)tss.o : $(KERNEL)/process/tss.c
	$(CC) -c $^ -o $@ $(CFLAGS) $(DEBUG) 

$(OUTPUT)timer.o : $(KERNEL)/interrupt/timer.c
	$(CC) -c $^ -o $@ $(CFLAGS) $(DEBUG) 

$(OUTPUT)IET.o : $(KERNEL)/interrupt/IET.asm
	nasm -f elf32 $^ -o $@ $(DEBUG) 

$(OUTPUT)pic.o : $(KERNEL)/interrupt/pic.c
	$(CC) -c $^ -o $@ $(CFLAGS) $(DEBUG) 

$(OUTPUT)syscall.o : $(KERNEL)/interrupt/syscall.c
	$(CC) -c $^ -o $@ $(CFLAGS) $(DEBUG)

$(OUTPUT)keyboard.o : $(KERNEL)/device/keyboard.c
	$(CC) -c $^ -o $@ $(CFLAGS) $(DEBUG) 

$(OUTPUT)assert.o : $(KERNEL)/assert/assert.c
	$(CC) -c $^ -o $@ $(CFLAGS) $(DEBUG) 

$(OUTPUT)interrupt.o : $(KERNEL)/interrupt/interrupt.c
	$(CC) -c $^ -o $@ $(CFLAGS) $(DEBUG) 

$(OUTPUT)kernel.o : $(KERNEL)/kernel.asm
	nasm -f elf32 $^ -o $@ $(DEBUG)

$(OUTPUT)memory.o : $(KERNEL)/memory/memory.c
	$(CC) -c $^ -o $@ $(CFLAGS) $(INCLUDE) $(DEBUG) 

$(OUTPUT)physicalMem.o : $(KERNEL)/memory/physicalMem.c
	$(CC) -c $^ -o $@ $(CFLAGS) $(INCLUDE) $(DEBUG) 

$(OUTPUT)virtualMem.o : $(KERNEL)/memory/virtualMem.c
	$(CC) -c $^ -o $@ $(CFLAGS) $(INCLUDE) $(DEBUG) 

$(OUTPUT)kernel0.o : $(KERNEL)/kernel0.c
	$(CC) -c $^ -o $@ $(CFLAGS) $(INCLUDE) $(DEBUG) 

$(OUTPUT)io.o : $(KERNEL)/io.c
	$(CC) -c $^ -o $@ $(CFLAGS) $(INCLUDE) $(DEBUG) 

$(OUTPUT)kernel.bin : $(kernelDep)
	$(LD)  -m elf_i386 -static $^ -o $@ -Ttext $(KERNEENTRY) 

$(OUTPUT)system.bin: $(OUTPUT)kernel.bin
	objcopy -O binary $< $@
	-dd conv=notrunc if=$@ of=$(MASTERDISK) bs=512 count=400 seek=10

$(OUTPUT)kernel.sym: $(OUTPUT)kernel.bin
	objcopy --only-keep-debug $^ $@

$(OUTPUT)system.map: $(OUTPUT)kernel.bin
	nm $< | sort > $@

#------------kernel-----------------

# - $*  不包含扩展名的目标文件名称
# - $+  所有的依赖文件，以空格分开，并以出现的先后为序，可能 包含重复的依赖文件
# - $<   第一个依赖文件的名称
# - $?   所有时间戳比目标文件晚的的依赖文件，并以空格分开
# - $@     目标文件的完整名称
# - $^   所有不重复的目标依赖文件，以空格分开
# - $%     如果目标是归档成员，则该变量表示目标的归档成员名称


#------------bootloader-----------------
$(OUTPUT)boot.bin : $(BOOTLOADER)/boot.asm
	nasm $^ -o $@ $(DEBUG)
	-dd conv=notrunc if=$@ of=$(MASTERDISK) bs=512 count=1

$(OUTPUT)loader.bin : $(BOOTLOADER)/loader.asm
	nasm $^ -o $@ $(DEBUG)
	-dd conv=notrunc if=$@ of=$(MASTERDISK) bs=512 count=2 seek=1

#------------bootloader-----------------

