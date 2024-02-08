;主引导程序:读取加载器程序到内存
mov ax, 3
int 0x10 ; 清屏

;数据写入位置:bx
;数据写入偏移位置:di
;读入多少个扇区:cl
;读取起始扇区:ch
;-------------------------
;在实模式的1mb可用内存下==>0x500开始就属于可用区域
;将内核加载器加载到0x1000处
mov bx,0
mov di,0x1000
mov cl,2
mov ch,1
;加载加载器
call readDisk

jmp 0x0:0x1000 ;正好是第二个页的起始地址

readDisk:
    ;在loader中重写了readDisk这个子程序!!,功能一致
    ;数据写入位置:bx  ==>loader 中 修改为es
    ;数据写入偏移位置:di ==>loader 中 依旧是edi
    ;读入多少个扇区:cl ; 8位最大256个扇区,0表示256 ==>loader 中 ecx
    ;读取起始扇区:ch  ==>loader 中 ebx
    
    ; 这个比较没有意义 loader 中 删除
    cmp cl,0  ; loader 中 cmp ecx,0
    jz .readEnd
    cmp ch,0  ;loader 中 cmp ebx,0
    jz .readEnd

    ;----loader 中 可删-- es就是就是目标位置
    push es
    mov es,bx
    ;---------

    mov dx,0x1f2
    ;读入那个扇区
    mov al,cl ;
    out dx,al 

    ;设置读取起始位置,读取模式,读取硬盘
    mov al,ch  ;==> mov al,bl
    inc dx ; 0x1f3
    out dx,al


    mov al,0 ;loader 中 ==>  shr bx,8  ;  mov al,bl
    inc dx ;0x1f4
    out dx,al

    ;loader 中  ==> shr bx,8  ;  mov al,bl
    inc dx ;1f5 
    out dx,al

    mov al,0xe0 ;0xe0,高4位表LBA模式读取主硬盘 ; ==> loader 中 shr bx,8  ;  mov al,bl ; and al,0b1111 ; or al,0b11100000
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
    mov [es:di],ax
    inc di
    inc di

    loop .read_256

    dec bx 
    cmp bx,0
    jnz .readLoop

.readEnd:
    pop es
    ret
end times 446-($-$$) db 0
; end times 510-($-$$) db 0
; db 0x55,0xaa
