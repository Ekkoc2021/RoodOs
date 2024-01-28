; 跳转到c执行
[bits 32]
extern main

PageTableAddr equ 0x2000
FirstPageItermAddr equ 0x3000
KernelVAddr equ 0xc000_0000

section .text
global _start
global getCursorIndex
global setCursor
global readDisk

;跳转的main函数
_start:
    mov ax,0b0000_0000_0001_0000
    mov ds,ax
    mov es,ax
    mov ss,ax
    mov gs,ax
    mov fs,ax
    mov esp,0xc0009000  ; 栈是从高往低
    mov eax,0
    mov [0x2000+0xc0000000],eax
    ; mov ebx,[memCount+KernelAddr]
    ; mov ecx,[memScanData+KernelAddr]
    ; 入栈顺序从右往左
    push ebp
    push edx
    mov eax,PageTableAddr
    push eax
    mov eax,KernelVAddr
    push eax 
    push ecx ;memScanDataAddr
    push ebx ;memCount
    call main
    ;退出内核,相当于关机
    jmp $


getCursorIndex:
    ;光标位置放在bx中,返回
    push eax
    push edx
    xor eax,eax
    xor ebx,ebx
    
     ;获取高八位
    mov dx,0x3d4
    mov al,0x0e
    out dx,al 
    ;从0x3d5端口读取数据
    mov dx,0x3d5
    in al,dx
    mov bh,al

    ;获取低八位
    mov dx,0x3d4
    mov al,0x0f
    out dx,al
    mov dx,0x3d5
    in al,dx
    mov bl,al

    pop edx
    pop eax
    ret

setCursor:
    ; bx中存放要设置
    ;的光标 0x0f,0x0e的数据低端
    push edx
    push eax

    ;写入低位0x0f
    mov dx,0x3d4
    mov al,0x0f
    out dx,al
    mov dx,0x3d5
    mov al,bl
    out dx,al

    ;写入高位0x0e
    mov dx,0x3d4
    mov al,0x0e
    out dx,al
    mov dx,0x3d5
    mov al,bh
    out dx,al

    pop eax
    pop edx
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