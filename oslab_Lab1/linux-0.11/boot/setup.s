
BOOTSEG = 0x07c0;		! bootsect 读入段地址
INITSEG = 0x9000;		! 初始数据段存放的位置
SETUPSEG = 0x07e0;		! setup 程序开始的地址（没有移动到 0x9000 处）

! 指定程序的入口点 为 _start
entry _start
_start:

! 从go标号处开始执行，并设置代码段 cs = SETUPSEG
	jmpi	go,SETUPSEG
! 设置 ds=es=cs
go:	mov	ax,cs
	mov	ds,ax  
	mov	es,ax

! 输出一些信息
	! 读入光标位置
	mov ah, #0x03
	xor bh, bh
	int 0x10

	! 设置开机启动字符串
	mov cx, #25			! 字符串长度（20字符+3个回车+3个换行）
	mov bx, #0x0007		! 设置 显示页号 和 字符属性
	mov ax, cs			! 使用cs值获取 段地址
	mov es, ax			! ax 充当临时寄存器
	mov bp, #msg2		! 字符串 偏移地址
	mov ax, #0x1301		! ah = 13H -> 显示， al = 01H -> 设置文本模式
	int 0x10


! 设置数据段ds，将硬件参数取出来放在内存 0x90000
	mov ax, #INITSEG
	mov ds, ax			! 数据段地址 ds = 0x9000

! 读光标位置，存入数据段
	mov ah, #0x03		! AH = 3 -> 读光标位置
	xor bh, bh			! 显示页数 = 0
	int 0x10
	! 将光标位置写入 0x90000.
	mov [0], dx

! 读入内存大小位置
	mov ah, #0x88		! AH = 0x88 -> 读入内存大小
	int 0x15
	mov [2], ax			! 存入 9000 后面两个偏移

! 获取磁盘参数表，从 0x41 处拷贝 16 个字节
	mov ax, #0x0000		! 不允许直接将立即数加载到段寄存器，需要使用通用寄存器ax
	mov ds, ax			! 数据段地址 ds = 0x0000
	lds	si,[4*0x41]		! 取中断向量 41 的值，即 hd0 参数表的地址 ds:si
	mov	ax,INITSEG
	mov	es,ax
	mov	di,#0x0080		! 传输的目的地址: es:di = 9000:0080
	mov	cx,#0x10		! 重复执行次数 = 16
	rep 				! 重复执行movsb, DS:SI -> ES:DI
	movsb				! 每执行一次 movsb 指令，源地址和目的地址的偏移都会自动递增


! 开始显示参数
! 前面修改了ds数据段寄存器，这里将其设置为0x9000
	mov ax,#INITSEG		! 设置数据段地址，后续显示硬件参数使用
	mov ds,ax			! 0x9000
	mov ax,#SETUPSEG	! 设置附加段地址，后续显示字符串使用
	mov	es,ax  			! 0x07e0

! 显示 光标位置
	! 获取光标位置 存放在dx中
	mov	ah,#0x03		
	xor	bh,bh
	int	0x10
	! 显示字符串   显示位置从dx中取出
	mov bp, #cur		! 串 偏移地址
	mov cx, #11			! 串 长度
	mov bx, #0x0003		! 页号 = 0 + 颜色设置（03：青色，07：普通白色）
	mov ax, #0x1301		! 显示字符串 + char...
	int 0x10
	! 显示数值
	mov ax, [0]			! 打印函数参数存入 ax
	call print_hex		! 调用打印寄存器函数
	call print_nl		! 打印回车换行

! 显示 内存大小
	! 获取光标位置
	mov	ah,#0x03		
	xor	bh,bh
	int	0x10
	! 显示字符串提示
	mov bp, #mem
	mov cx, #12
	mov bx, #0x0007		
	mov ax, #0x1301		! 显示字符串 + char...
	int 0x10
	! 显示数值
	mov ax, [2]
	call print_hex
	! 显示 KB
	mov	ah,#0x03		! read cursor pos
	xor	bh,bh
	int	0x10
	mov	cx,#6
	mov	bx,#0x0007		! page 0, attribute c 
	mov	bp,#cyl
	mov	ax,#0x1301		! write string, move cursor
	int	0x10


! 无限循环
inf_loop:
	jmp inf_loop

! 以 16 进制 打印寄存器 ax 中的16位数
print_hex:
	mov cx, #4		! 循环次数s
	mov dx,ax   	! 将ax所指的值放入dx中，ax作为参数传递寄存器
print_digit:
	rol dx,#4  		! 将 DX 寄存器中的值向左循环移位 4 位，相当于将 高4位 移到 低4位
	mov ax,#0xe0f  	! ah = 0eh -> 显示字符, al = 字符：半字节(4个比特)掩码。
	and al,dl 		! 取 dl 的低4比特值。
	add al,#0x30  	! 给al数字加上十六进制 0x30, 0~9 + 0x30 = "0~9"
	cmp al,#0x3a	! 判断 al 是否大于 字符 9
	jl outp  		! al < 数字"9"后面的字符，说明是 0～9，则跳转
	add al,#0x07    ! al > "9", 是a~f,要多加7h
outp:
	int 0x10			! 打印字符存储在 al 中
	loop print_digit	! cx = 4,循环四次
	ret					! 函数返回指令，表示函数执行结束，返回到调用该函数的位置

! 打印回车换行
print_nl:
	mov ax, #0x0e0d
	int 0x10
	mov al, #0x0a
	int 0x10
	ret

! 显示字符串
msg2:
	.byte	13,10		! 回车 + 换行
	.ascii	"Now we are in SETUP"
	.byte	13,10,13,10

! 光标位置
cur:
	.ascii	"Cursor POS:"

! 内存大小
mem:
	.ascii	"Memory SIZE:"

! 提示信息
cyl:
	.ascii "KB"
	.byte 13,10,13,10
