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
