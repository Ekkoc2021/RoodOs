
;----------基本的宏--------------------------
org 0x1000 ;加载器:默认加载到0x1000位置运行
KernelAddr equ 0xc000_0000
kernelEntry equ 0xc0011000
kernelLoaction equ 0x11000 
; 11000之后可用内存为571KB kernel最终应该不会超过200KB
memCountLocation equ memCount + KernelAddr
memDataLocation equ memScanData + KernelAddr
kernelStartSection equ 10
NumOfkernelSection equ 400

PageTableAddr equ 0x2000
FirstPageItermAddr equ 0x3000

;--------------   gdt描述符属性  -----------
DESC_G_4K   equ	  1_000_0000_0000_0000_0000_0000b   
DESC_D_32   equ	   1_00_0000_0000_0000_0000_0000b
DESC_L	    equ	    0_0_0000_0000_0000_0000_0000b	;  64位代码标记，此处标记为0便可。
DESC_AVL    equ	     0_0000_0000_0000_0000_0000b	;  cpu不用此位，暂置为0  

; 内核 高地址段界限
DESC_KERNEL_LIMIT_CODE2  equ 1111_0000_0000_0000_0000b
DESC_KERNEL_LIMIT_DATA2  equ DESC_KERNEL_LIMIT_CODE2

; 用户 高地址段界限
DESC_USER_LIMIT_CODE2 equ 1100_0000_0000_0000_0000b
DESC_USER_LIMIT_DATA2  equ DESC_USER_LIMIT_CODE2

DESC_P	    equ		  1_000_0000_0000_0000b
DESC_DPL_0  equ		   00_0_0000_0000_0000b
DESC_DPL_1  equ		   01_0_0000_0000_0000b
DESC_DPL_2  equ		   10_0_0000_0000_0000b
DESC_DPL_3  equ		   11_0_0000_0000_0000b

; 12位,s =1 代码段和数据段
DESC_S_CODE equ		     1_0000_0000_0000b
DESC_S_DATA equ	  DESC_S_CODE
DESC_S_sys  equ		     0_0000_0000_0000b

DESC_TYPE_CODE  equ	      1000_0000_0000b	;x=1,c=0,r=0,a=0 代码段是可执行的,非依从的,不可读的,已访问位a清0.  
DESC_TYPE_DATA  equ	      0010_0000_0000b	;x=0,e=0,w=1,a=0 数据段是不可执行的,向上扩展的,可写的,已访问位a清0.


; 内核数据段和代码段
DESC_KERNEL_CODE_HIGH4 equ (0x00 << 24) + DESC_G_4K + DESC_D_32 + DESC_L + DESC_AVL +DESC_KERNEL_LIMIT_CODE2 + DESC_P + DESC_DPL_0 + DESC_S_CODE + DESC_TYPE_CODE + 0x00
DESC_KERNEL_DATA_HIGH4 equ (0x00 << 24) + DESC_G_4K + DESC_D_32 + DESC_L + DESC_AVL + DESC_KERNEL_LIMIT_DATA2 + DESC_P + DESC_DPL_0 + DESC_S_DATA + DESC_TYPE_DATA + 0x00

; 用户数据段和代码段
DESC_USER_CODE_HIGH4 equ (0x00 << 24) + DESC_G_4K + DESC_D_32 + DESC_L + DESC_AVL +DESC_KERNEL_LIMIT_CODE2 + DESC_P + DESC_DPL_3 + DESC_S_CODE + DESC_TYPE_CODE + 0x00
DESC_USER_DATA_HIGH4 equ (0x00 << 24) + DESC_G_4K + DESC_D_32 + DESC_L + DESC_AVL + DESC_KERNEL_LIMIT_DATA2 + DESC_P + DESC_DPL_3 + DESC_S_DATA + DESC_TYPE_DATA + 0x00

TSS_D equ 0
TSS_LIMIT_SIZE1 equ 0
TSS_LIMIT_SIZE2 equ 108
TSS_S equ 0
TSS_TYPE equ 1001_0000_0000b
; TSS 描述符
DESC_TSS_HIGH equ (0x00 << 24) + DESC_G_4K + TSS_D + DESC_L + DESC_AVL +TSS_LIMIT_SIZE1 + DESC_P + DESC_DPL_0 +TSS_S + TSS_TYPE + 0x00

;--------------   选择子属性  ---------------
RPL0  equ   00b
RPL1  equ   01b
RPL2  equ   10b
RPL3  equ   11b
TI_GDT	 equ   000b
TI_LDT	 equ   100b

;---------------------------程序入口--------------------------------------------
jmp loaderStar

vt_gdt_addr:
    dw (0x50-1) ; gdt界限:gdt的大小-1
    dd gdt_start+KernelAddr
gdt_addr: ;gdtr 第48位表示
    dw (0x50-1) ; gdt界限:gdt的大小-1
    dd gdt_start

gdt_start:
    ;gdt前4个字是空的
    dw 0,0,0,0

;0 特权级代码段 ==选择子: 0b1000
GlobalZeroDPLCodeDescription:
    dd 0x0000FFFF
    dd DESC_KERNEL_CODE_HIGH4

;0 特权级数据段 ==选择子: 0b10000
GlobalZeroDPLDataDescription:
    dd 0x0000FFFF
    dd DESC_KERNEL_DATA_HIGH4

;3 特权级代码段 ==选择子: 0b11011
GlobalThreeDPLCodeDescription:
    dd 0x0000FFFF
    dd DESC_USER_CODE_HIGH4

;3 特权级数据段 ==选择子: 0b100011
GlobalThreeDPLDataDescription:
    dd 0x0000FFFF
    dd DESC_USER_DATA_HIGH4

; TSS 描述符 == 选择子 : 0b101000
GlobalTSSDescription:
    dd 0
    dd DESC_TSS_HIGH

emptyDataDescription:
    dw 0,0,0,0
    dw 0,0,0,0
    dw 0,0,0,0
    dw 0,0,0,0
    dw 0,0,0,0
    dw 0,0,0,0
;后续补充其他特权级描述符

loaderStar:
; -----------------扫描可用内存--------------------
; 进入保护模式后,关闭中断,直到新中断初始化,需要调用bios的中断读取可用内存
; 搜索可用内存信息
    xor ebx, ebx		      ;第一次调用时，ebx值要为0
    mov edx, 0x534d4150	      ;edx只赋值一次，循环体中不会改变
    mov di, memScanData	      ;ards结构缓冲区
    
.e820_mem_get_loop:	      ;循环获取每个ARDS内存范围描述结构
    mov eax, 0x0000e820	      ;执行int 0x15后,eax值变为0x534d4150,所以每次执行int前都要更新为子功能号。
    mov ecx, 20		      ;ARDS地址范围描述符结构大小是20字节
    int 0x15
    
    add di, cx		      ;使di增加20字节指向缓冲区中新的ARDS结构位置
    inc word [memCount]	      ;记录数量
    cmp ebx, 0		      ;若ebx为0且cf不为1,这说明ards全部返回，当前已是最后一个
    jnz .e820_mem_get_loop


;----------------开启保护模式----------------------------
    ; (TSS+KernelAddr)<<16 + 108
    mov ebx,TSS+KernelAddr
    shl ebx,16
    add ebx,107
    mov [GlobalTSSDescription],ebx
    
    ; 设置TSS 描述符
    
    mov ebx,TSS+KernelAddr
    and ebx,0xFF000000
    mov eax,TSS+KernelAddr
    shr eax,16
    and eax,0xFF
    add ebx,eax
    mov eax,[GlobalTSSDescription+4]
    add eax,ebx 
    mov [GlobalTSSDescription+4],eax

; - 进入保护模式==>使用平坦模型==>后续代码都用32位操作模式
    cli ; 关中断 
    ;为GDT分配位置:GDT不会太大
    lgdt [gdt_addr]

    ;打开第21根地线:默认是打开的
    in al,0x92
    or al,0b0000_0010
    out 0x92,al

    ;设置cr0寄存器
    mov eax, cr0
    or eax, 1
    mov cr0, eax

    ; 后面进入保护模式后已经修改默认操作尺寸为32位
    ; 实模式下操作尺寸默认16位
    ; jmp 强行刷新流水线! 流水线技术可能会导致指令解析错误!
    jmp  dword  0b1000:protect_mode

;告诉编译器,下面用32位操作尺寸进行编译,同理cpu也要用32位操作尺寸进行译码
bits 32 
;--------------进入保护模式----------------------
protect_mode:
    mov ax,0b0000_0000_0001_0000
    mov ds,ax
    mov es,ax
    mov ss,ax
    mov gs,ax
    mov fs,ax
    mov esp,5000  ; 默认esp为1000

    ;加载内核程序,es已经初始化,内核可能会比较大,实模式下加载不方便
    mov edi,kernelLoaction
    mov cl,NumOfkernelSection ;多少扇区
    mov ebx,kernelStartSection ; 起始位置
    ;加载加载内核
    call readDisk

; -------------------开启分页--------------------------------
    ;页目录独占4K==>这个位置应该放在哪里?==>第3个页 0x2000
    ;清空0x2000-0x3000一个页的内容
    mov edi,PageTableAddr
    call clear_page
    ;单个页表独占4K==>第4个页 0x3000,先只设置一个页
    mov edi,FirstPageItermAddr
    call clear_page
    ;1mb的内存映射到0xc000_0000-0xc010_0000
    ;对应页表 1100 0000 00 ==>第768(0x300)个页表项

    mov eax,FirstPageItermAddr ;构建页目录项
    ;页目录项属性设置为0b0000 0000 0000 0011
    or eax,0b111
    mov [PageTableAddr+(KernelAddr>>22)*4],eax
    ;第一个页目录项写完毕

    mov [PageTableAddr],eax ; 过渡,防止出错,进入kernel会立即重置
    
    ;配置页表项
    ;页表项属性和页目录项属性一致
    mov ecx,256 ;共映射256个页
    mov ebp,FirstPageItermAddr ;页表起始位置
    mov edi,0 ;物理地址起始值
    mov esi,0b111 ;属性项

.set_pageTableItems:
    sub ebp,4
.setLoop:
    mov eax,edi
    or eax,esi
    add edi,4*1024
    ;计算填入位置
    add ebp,4
    mov [ebp],eax
    loop .setLoop

    ;页表映射工作结束
    
    mov ebx,[memCount] ; 扫描内存的数据
    mov ecx,memScanData+KernelAddr ; 扫描到内存块数据的地址
    mov edx,vt_gdt_addr+KernelAddr ; gdt的虚拟地址
    mov ebp,TSS+KernelAddr ; tss的虚拟地址
    
    ;
    ;设置页表项物理地址
    mov eax,PageTableAddr
    mov cr3,eax ;刷新TLB

    mov eax,cr0
    or eax,0b1000_0000_0000_0000_0000_0000_0000_0000
    mov cr0,eax

    lgdt [vt_gdt_addr+KernelAddr] ; 将lgdt配置到虚拟地址
    
    mov eax,cr0
    or eax,1
    mov cr0,eax

;-----------------进入kernel-------------------

kernel_start:jmp dword 0b1000:kernelEntry ; 第三个选择子

;--------------------loader的函数定义------------------
;清理一页数据的函数
clear_page:
    ;edi偏移
    push ecx
    mov ecx,4096
.clear:
    mov byte [edi],0
    loop .clear
    pop ecx
    ret

;读取磁盘的函数
readDisk:
    ;数据写入位置:es
    ;数据写入偏移位置:edi
    ;读入多少个扇区:ecx ; 只能是8位最大256个扇区,0表示256
    ;读取起始扇区:ebx
    
    mov dx,0x1f2
    ;读入一个扇区
    mov al,cl ;
    out dx,al 

    ;设置读取起始位置,读取模式,读取硬盘
    mov al,bl
    inc dx ; 0x1f3
    out dx,al


    shr bx,8   
    mov al,bl
    inc dx ;0x1f4
    out dx,al

    shr bx,8  ;  
    mov al,bl
    inc dx ;1f5 
    out dx,al

    ; 0xe0,高4位表示LBA模式读取主硬盘
    shr bx,8    
    mov al,bl ;
    and al,0b1111 ; 
    or al,0b11100000
    inc dx ;1f6
    out dx,al

    mov al,0x20 ;表示读
    inc dx ;1f7
    out dx,al 

    ;读取次数
    xor ebx,ebx
    mov bl,cl ;扇区数量
    mov ecx,256

;每读512个字节就要wait一下,硬盘读取是以512为单位
.readLoop:
    mov cx,256 ;256个字,单次读取量
.wait: 
    nop
    nop
    nop 

    mov dx,0x1f7
    in al, dx
    and al, 0b1000_1000
    cmp al, 0b0000_1000
    jnz .wait

.read_256:
    nop
    nop
    nop 
    mov dx,0x1f0
    in ax,dx
    mov [es:edi],ax 
    inc edi
    inc edi

    loop .read_256

    dec ebx 
    cmp ebx,0
    jnz .readLoop

    ret

;----TSS------
TSS:
    times 110 db 0
;----内存扫描数据存储----
memCount:
    dw 0,0,0
memScanData:
    dw 0