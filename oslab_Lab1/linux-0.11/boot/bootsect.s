! 声明了几个全局符号，用来标识程序的代码段、数据段和未初始化数据段的起始和结束位置
.globl begtext, begdata, begbss, endtext, enddata, endbss
.text			! 接下来的指令是 代码段
begtext:		! 标识代码段的起始位置
.data			! 接下来的指令是 数据段
begdata:		! 标识数据段的起始位置
.bss			! 接下来的指令是 未初始化数据段
begbss:			! 标识未初始化数据段的起始位置
.text			! 接下来的指令是 代码段


BOOTSEG = 0x07c0;	! bootsect 程序开始的地址
SETUPSEG = 0x07e0;	! setup 程序开始的地址（没有移动到 0x9000 处）
SETUPLEN = 4;		! setup 占用扇区数量

! 指定程序的入口点 为 _start
entry _start
_start:

! 从go标号处开始执行，并设置代码段 cs = BOOTSEG
	jmpi	go,BOOTSEG
! 设置 ds=es=cs
go:	mov	ax,cs
	mov	ds,ax  
	mov	es,ax

! 输出一些信息
	! 读入光标位置
	mov ah, #0x03
	xor bh, bh 			! 显示页码设置
	int 0x10

	! 设置开机启动字符串
	mov cx, #26			! 字符串长度（20字符+3个回车+3个换行）
	mov bx, #0x0007		! 设置 显示页号 和 字符属性
	mov ax, #BOOTSEG	! 字符串 段地址
	mov es, ax			! 不能将立即数直接赋给段寄存器，ax 充当临时寄存器
	mov bp, #msg1		! 字符串 偏移地址
	mov ax, #0x1301		! ah = 13H -> 显示， al = 01H -> 设置文本模式
	int 0x10			! 显示服务中断

! 加载 setup.s 程序
load_setup:
	mov	dx,#0x0000			! 设置驱动器和磁头(drive 0, head 0)
	mov	cx,#0x0002			! 设置扇区号和磁道(sector 2, track 0)
	mov	bx,#0x0200			! 偏移地址为 512 bytes
	mov	ax,#0x0200+SETUPLEN	! ah=02H：从磁盘读数据到内存；al=04H，读入四个扇区
	int	0x13				! 低级磁盘服务中断
	jnc	ok_load_setup		! 加载成功，跳转
	！加载错误
	mov	dx,#0x0000
	mov	ax,#0x0000			! 复位软驱
	int	0x13
	jmp load_setup			！再次尝试

! 加载成功后，开始执行setup代码
ok_load_setup:
	jmpi 0,SETUPSEG


! 显示字符串
msg1:
	.byte	13,10		! 回车 + 换行
	.ascii	"Joker is booting ..."
	.byte	13,10,13,10

! 将当前位置设置为内存地址 510 字节处
.org 510				
! 设置引导扇区标记
	.word	0xAA55		! 指定引导扇区的最后两个字节，bootsect必须以它结尾，才能引导

.text			
endtext:		! 标识代码段的结束位置
.data
enddata:		! 标识数据段的结束位置
.bss
endbss:			! 标识未初始化数据段的结束位置


! 1、编译和运行
! cd ~/my_space/OS_HIT/oslab_Lab1/linux-0.11/boot
! as86 -0 -a -o bootsect.o bootsect.s
! ld86 -0 -s -o bootsect bootsect.o
! 
! ls -l
! -rwxrwxr-x 1 joker joker  544 4月  14 14:33 bootsect
! -rw-rw-r-- 1 joker joker  159 4月  14 14:33 bootsect.o
! -rw-r--r-- 1 joker joker  987 4月  14 14:39 bootsect.s
! -rw-r--r-- 1 joker joker 5938 8月  28  2008 head.s
! -rw-r--r-- 1 joker joker 5362 8月  28  2008 setup.s
! 
! hexdump -C bootsect
! 
! dd bs=1 if=bootsect of=Image skip=32
! cp ./Image ../Image
! ../../run
