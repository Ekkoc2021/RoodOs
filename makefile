LINUXBOCHSRC=make/bochsconfig/bochsrc_gdb.bxrc
# LINUXBOCHSRC=make/bochsconfig/bochsrc_vmlinux.bxrc
LINUXMASTERDISK=make/bochsconfig/myvd.vhd
LINUXBOCHS=bochs-gdb/bin/bochs
# /mnt/e/workspace/vscode_workspace/RoodOs/
WINDOWSGCC=x86_64-elf-gcc
WINDOWSLD=x86_64-elf-ld
WINDOWSBOCHSRC=make/bochsconfig/bochsrc.bxrc # windows下bochs配置文件
WINDOWSBOCHS=bochsdbg
WINDOWSMASTERDISK=make/bochsconfig/disk1.img # windows下启动盘


BUILD=build/

#-----------now----------------
debug : lstart lqemuDebug

clear : lclear

#-----------now----------------




#-------------------------linux------------------------
lkernel : 
	make kernel -f make/makefile \
	MASTERDISK=$(LINUXMASTERDISK) \
	OUTPUT=$(BUILD)

lstart :
	make -f make/makefile \
	MASTERDISK=$(LINUXMASTERDISK) \
	OUTPUT=$(BUILD)

lqemuDebug :
	qemu-system-i386 $(LINUXMASTERDISK) -m 128M -S -s

lqemu :
	qemu-system-i386 $(LINUXMASTERDISK) -m 128M

# linux下清除功能
lclear:
	rm -f build/*.bin
	rm -f build/*.o
	rm -f build/*.sym


#-------------------------win---------------------------
#只编译内核
wKernel :
	make kernel -f make/makefile \
	MASTERDISK=$(WINDOWSMASTERDISK) \
	CC=$(WINDOWSGCC) \
	LD=$(WINDOWSLD) \
	OUTPUT=$(BUILD)

# 完整编译!
wStart : 
	make -f make/makefile \
	MASTERDISK=$(WINDOWSMASTERDISK) \
	CC=$(WINDOWSGCC) \
	LD=$(WINDOWSLD) \
	OUTPUT=$(BUILD)


# windows下的qemu
wqemu :
	qemu-system-i386 make/bochsconfig/disk1.img -m 32M -S -s

# windows下的bochs启动
wbochs :
	$(WINDOWSBOCHS) -f $(WINDOWSBOCHSRC) -q

wclear:
	-del build\*.bin
	-del build\*.o
	-del build\*.sym