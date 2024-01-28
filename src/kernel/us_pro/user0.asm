; 跳转到c执行
[bits 32]
extern main

section .text


;跳转的main函数
_start:
    call main
    ;退出内核,相当于关机
    jmp $

    
