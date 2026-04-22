SECTION MBR vstart=0x7c00       ;告诉编译器,代码加载到内存的起始位置是 0x7c00
    mov ax,cs                   ;代码段的寄存器 cs->ax
    mov ds,ax                   ;用ax初始化其他寄存器
    mov es,ax
    mov ss,ax
    mov fs,ax
    mov sp,0x7c00               ;设置栈顶指针sp,栈向下增长,不会覆盖,栈是31kb往下,mbr是31kb往上到31.5kb

    mov ax,0x600                ;ax是16位,拆分两个8位,ah(a high 高位)放06(0省略),表示清屏  向上滚动窗口内容 ,al(a low 低位)放00 ,如果al=0,则清空屏幕
    mov bx,0x700                ;bh = 07 白底黑字
    mov cx,0                    ;ch = 00 左上角
    mov dx,0x184f               ;dh = 0x18(24),清算的时候从dh里取值清到24,dl = 0x4f(79),清理从(0,0)到(24,79)整个矩形区域

    int 0x10                    ;CPU中断,执行BIOS预设好的一段程序,执行上述汇编代码

    mov ah,3                    ;ah = 0x03,获取光标任务
    mov bh,0                    ;bh = 0 页号为0

    int 0x10                    ;继续中断,获取光标任务

    mov ax,message              ;字符串的偏移地址存入ax
    mov bp,ax                   ;无法直接将message给bp,只能先给ax,间接给bp

    mov cx,5                    ;字符串长度为5
    mov ax,0x1301               ;ah = 0x13,表示打印,al = 0x01(光标跟随移动)

    mov bx,0x2                  ;bh = 0x00 第0页,bl = 0x02 (黑底绿字)

    int 0x10                    ;执行显示操作

    jmp $                       ;跳转回我现在的内存行,防止CPU读取后面未知的程序

    message db "1 MBR"          ;db()
    times 510-($-$$) db 0       ;times(重复指令,$当前地址,$$本节的起始地址--0x7c00,($$-$)计算从开始到现在写了多少行的代码和数据,计算距离510字节还有多少,然后全部补0)
    db 0x55,0xaa                ;检查最后两位数是不是