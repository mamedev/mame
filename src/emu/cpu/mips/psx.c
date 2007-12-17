/*
 * Sony CXD8530AQ/CXD8530BQ/CXD8530CQ/CXD8661R
 *
 * PSX CPU emulator for the MAME project written by smf
 * Thanks to Farfetch'd for information on the delay slot bug
 *
 * The PSX CPU is a custom r3000a with a built in
 * geometry transform engine, no mmu & no data cache.
 *
 * There is a stall circuit for load delays, but
 * it doesn't work if the load occurs in a branch
 * delay slot.
 *
 */

#include "ui.h"
#include "debugger.h"
#include "psx.h"
#include "osd_cpu.h"

#define LOG_BIOSCALL ( 0 )

#define EXC_INT ( 0 )
#define EXC_ADEL ( 4 )
#define EXC_ADES ( 5 )
#define EXC_SYS ( 8 )
#define EXC_BP ( 9 )
#define EXC_RI ( 10 )
#define EXC_CPU ( 11 )
#define EXC_OVF ( 12 )

#define CP0_RANDOM ( 1 )
#define CP0_BADVADDR ( 8 )
#define CP0_SR ( 12 )
#define CP0_CAUSE ( 13 )
#define CP0_EPC ( 14 )
#define CP0_PRID ( 15 )

#define SR_IEC ( 1L << 0 )
#define SR_KUC ( 1L << 1 )
#define SR_ISC ( 1L << 16 )
#define SR_SWC ( 1L << 17 )
#define SR_TS  ( 1L << 21 )
#define SR_BEV ( 1L << 22 )
#define SR_RE ( 1L << 25 )
#define SR_CU0 ( 1L << 28 )
#define SR_CU1 ( 1L << 29 )
#define SR_CU2 ( 1L << 30 )
#define SR_CU3 ( 1L << 31 )

#define CAUSE_EXC ( 31L << 2 )
#define CAUSE_IP ( 255L << 8 )
#define CAUSE_IP2 ( 1L << 10 )
#define CAUSE_IP3 ( 1L << 11 )
#define CAUSE_IP4 ( 1L << 12 )
#define CAUSE_IP5 ( 1L << 13 )
#define CAUSE_IP6 ( 1L << 14 )
#define CAUSE_IP7 ( 1L << 15 )
#define CAUSE_CE ( 3L << 28 )
#define CAUSE_CE0 ( 0L << 28 )
#define CAUSE_CE1 ( 1L << 28 )
#define CAUSE_CE2 ( 2L << 28 )
#define CAUSE_BD ( 1L << 31 )

static const char *delayn[] =
{
	"pc", "at", "v0", "v1", "a0", "a1", "a2", "a3",
	"t0", "t1", "t2", "t3", "t4", "t5", "t6", "t7",
	"s0", "s1", "s2", "s3", "s4", "s5", "s6", "s7",
	"t8", "t9", "k0", "k1", "gp", "sp", "fp", "ra",
	"pc"
};

#define REGPC ( 32 )

typedef struct
{
	UINT32 op;
	UINT32 pc;
	UINT32 delayv;
	UINT32 delayr;
	UINT32 hi;
	UINT32 lo;
	UINT32 r[ 32 ];
	UINT32 cp0r[ 32 ];
	PAIR cp2cr[ 32 ];
	PAIR cp2dr[ 32 ];
	int (*irq_callback)(int irqline);
} mips_cpu_context;

static mips_cpu_context mipscpu;

static int mips_ICount = 0;

static UINT32 mips_mtc0_writemask[]=
{
	0xffffffff, /* INDEX */
	0x00000000, /* RANDOM */
	0xffffff00, /* ENTRYLO */
	0x00000000,
	0xffe00000, /* CONTEXT */
	0x00000000,
	0x00000000,
	0x00000000,
	0x00000000, /* BADVADDR */
	0x00000000,
	0xffffffc0, /* ENTRYHI */
	0x00000000,
	0xf27fff3f, /* SR */
	0x00000300, /* CAUSE */
	0x00000000, /* EPC */
	0x00000000, /* PRID */
	0x00000000,
	0x00000000,
	0x00000000,
	0x00000000,
	0x00000000,
	0x00000000,
	0x00000000,
	0x00000000,
	0x00000000,
	0x00000000,
	0x00000000,
	0x00000000,
	0x00000000,
	0x00000000,
	0x00000000,
	0x00000000
};

#if 0
void GTELOG(const char *a,...)
{
	va_list va;
	char s_text[ 1024 ];
	va_start( va, a );
	vsprintf( s_text, a, va );
	va_end( va );
	logerror( "%08x: GTE: %08x %s\n", mipscpu.pc, INS_COFUN( mipscpu.op ), s_text );
}
#else
INLINE void GTELOG(const char *a, ...) {}
#endif

static UINT32 getcp2dr( int n_reg );
static void setcp2dr( int n_reg, UINT32 n_value );
static UINT32 getcp2cr( int n_reg );
static void setcp2cr( int n_reg, UINT32 n_value );
static void docop2( int gteop );
static void mips_exception( int exception );

static void mips_stop( void )
{
	DEBUGGER_BREAK;
	CALL_MAME_DEBUG;
}

#if LOG_BIOSCALL

static const struct
{
	int address;
	int operation;
	const char *prototype;
} bioscalls[] =
{
	{ 0xa0, 0x00, "int open(const char *name, int mode)" },
	{ 0xa0, 0x01, "int lseek(int fd, int offset, int whence)" },
	{ 0xa0, 0x02, "int read(int fd, void *buf, int nbytes)" },
	{ 0xa0, 0x03, "int write(int fd, void *buf, int nbytes)" },
	{ 0xa0, 0x04, "int close(int fd)" },
	{ 0xa0, 0x05, "int ioctl(int fd, int cmd, int arg)" },
	{ 0xa0, 0x06, "void exit(int code)" },
	{ 0xa0, 0x07, "sys_a0_07()" },
	{ 0xa0, 0x08, "char getc(int fd)" },
	{ 0xa0, 0x09, "void putc(char c, int fd)" },
	{ 0xa0, 0x0a, "todigit()" },
	{ 0xa0, 0x0b, "double atof(const char *s)" },
	{ 0xa0, 0x0c, "long strtoul(const char *s, char **ptr, int base)" },
	{ 0xa0, 0x0d, "unsigned long strtol(const char *s, char **ptr, int base)" },
	{ 0xa0, 0x0e, "int abs(int val)" },
	{ 0xa0, 0x0f, "long labs(long lval)" },
	{ 0xa0, 0x10, "long atoi(const char *s)" },
	{ 0xa0, 0x11, "int atol(const char *s)" },
	{ 0xa0, 0x12, "atob()" },
	{ 0xa0, 0x13, "int setjmp(jmp_buf *ctx)" },
	{ 0xa0, 0x14, "void longjmp(jmp_buf *ctx, int value)" },
	{ 0xa0, 0x15, "char *strcat(char *dst, const char *src)" },
	{ 0xa0, 0x16, "char *strncat(char *dst, const char *src, size_t n)" },
	{ 0xa0, 0x17, "int strcmp(const char *dst, const char *src)" },
	{ 0xa0, 0x18, "int strncmp(const char *dst, const char *src, size_t n)" },
	{ 0xa0, 0x19, "char *strcpy(char *dst, const char *src)" },
	{ 0xa0, 0x1a, "char *strncpy(char *dst, const char *src, size_t n)" },
	{ 0xa0, 0x1b, "size_t strlen(const char *s)" },
	{ 0xa0, 0x1c, "int index(const char *s, int c)" },
	{ 0xa0, 0x1d, "int rindex(const char *s, int c)" },
	{ 0xa0, 0x1e, "char *strchr(const char *s, int c)" },
	{ 0xa0, 0x1f, "char *strrchr(const char *s, int c)" },
	{ 0xa0, 0x20, "char *strpbrk(const char *dst, const char *src)" },
	{ 0xa0, 0x21, "size_t strspn(const char *s, const char *set)" },
	{ 0xa0, 0x22, "size_t strcspn(const char *s, const char *set)" },
	{ 0xa0, 0x23, "char *strtok(char *s, const char *set)" },
	{ 0xa0, 0x24, "char *strstr(const char *s, const char *set)" },
	{ 0xa0, 0x25, "int toupper(int c)" },
	{ 0xa0, 0x26, "int tolower(int c)" },
	{ 0xa0, 0x27, "void bcopy(const void *src, void *dst, size_t len)" },
	{ 0xa0, 0x28, "void bzero(void *ptr, size_t len)" },
	{ 0xa0, 0x29, "int bcmp(const void *ptr1, const void *ptr2, int len)" },
	{ 0xa0, 0x2a, "void *memcpy(void *dst, const void *src, size_t n)" },
	{ 0xa0, 0x2b, "void *memset(void *dst, char c, size_t n)" },
	{ 0xa0, 0x2c, "void *memmove(void *dst, const void *src, size_t n)" },
	{ 0xa0, 0x2d, "int memcmp(const void *dst, const void *src, size_t n)" },
	{ 0xa0, 0x2e, "void *memchr(const void *s, int c, size_t n)" },
	{ 0xa0, 0x2f, "int rand()" },
	{ 0xa0, 0x30, "void srand(unsigned int seed)" },
	{ 0xa0, 0x31, "void qsort(void *base, int nel, int width, int (*cmp)(void *, void *))" },
	{ 0xa0, 0x32, "double strtod(const char *s, char **endptr)" },
	{ 0xa0, 0x33, "void *malloc(int size)" },
	{ 0xa0, 0x34, "void free(void *buf)" },
	{ 0xa0, 0x35, "void *lsearch(void *key, void *base, int belp, int width, int (*cmp)(void *, void *))" },
	{ 0xa0, 0x36, "void *bsearch(void *key, void *base, int nel, int size, int (*cmp)(void *, void *))" },
	{ 0xa0, 0x37, "void *calloc(int size, int n)" },
	{ 0xa0, 0x38, "void *realloc(void *buf, int n)" },
	{ 0xa0, 0x39, "InitHeap(void *block, int size)" },
	{ 0xa0, 0x3a, "void _exit(int code)" },
	{ 0xa0, 0x3b, "char getchar(void)" },
	{ 0xa0, 0x3c, "void putchar(char c)" },
	{ 0xa0, 0x3d, "char *gets(char *s)" },
	{ 0xa0, 0x3e, "void puts(const char *s)" },
	{ 0xa0, 0x3f, "int printf(const char *fmt, ...)" },
	{ 0xa0, 0x40, "sys_a0_40()" },
	{ 0xa0, 0x41, "int LoadTest(const char *name, struct EXEC *header)" },
	{ 0xa0, 0x42, "int Load(const char *name, struct EXEC *header)" },
	{ 0xa0, 0x43, "int Exec(struct EXEC *header, int argc, char **argv)" },
	{ 0xa0, 0x44, "void FlushCache()" },
	{ 0xa0, 0x45, "void InstallInterruptHandler()" },
	{ 0xa0, 0x46, "GPU_dw(int x, int y, int w, int h, long *data)" },
	{ 0xa0, 0x47, "mem2vram(int x, int y, int w, int h, long *data)" },
	{ 0xa0, 0x48, "SendGPU(int status)" },
	{ 0xa0, 0x49, "GPU_cw(long cw)" },
	{ 0xa0, 0x4a, "GPU_cwb(long *pkt, int len)" },
	{ 0xa0, 0x4b, "SendPackets(void *ptr)" },
	{ 0xa0, 0x4c, "sys_a0_4c()" },
	{ 0xa0, 0x4d, "int GetGPUStatus()" },
	{ 0xa0, 0x4e, "GPU_sync()" },
	{ 0xa0, 0x4f, "sys_a0_4f()" },
	{ 0xa0, 0x50, "sys_a0_50()" },
	{ 0xa0, 0x51, "int LoadExec(const char *name, int, int)" },
	{ 0xa0, 0x52, "GetSysSp()" },
	{ 0xa0, 0x53, "sys_a0_53()" },
	{ 0xa0, 0x54, "_96_init()" },
	{ 0xa0, 0x55, "_bu_init()" },
	{ 0xa0, 0x56, "_96_remove()" },
	{ 0xa0, 0x57, "sys_a0_57()" },
	{ 0xa0, 0x58, "sys_a0_58()" },
	{ 0xa0, 0x59, "sys_a0_59()" },
	{ 0xa0, 0x5a, "sys_a0_5a()" },
	{ 0xa0, 0x5b, "dev_tty_init()" },
	{ 0xa0, 0x5c, "dev_tty_open()" },
	{ 0xa0, 0x5d, "dev_tty_5d()" },
	{ 0xa0, 0x5e, "dev_tty_ioctl()" },
	{ 0xa0, 0x5f, "dev_cd_open()" },
	{ 0xa0, 0x60, "dev_cd_read()" },
	{ 0xa0, 0x61, "dev_cd_close()" },
	{ 0xa0, 0x62, "dev_cd_firstfile()" },
	{ 0xa0, 0x63, "dev_cd_nextfile()" },
	{ 0xa0, 0x64, "dev_cd_chdir()" },
	{ 0xa0, 0x65, "dev_card_open()" },
	{ 0xa0, 0x66, "dev_card_read()" },
	{ 0xa0, 0x67, "dev_card_write()" },
	{ 0xa0, 0x68, "dev_card_close()" },
	{ 0xa0, 0x69, "dev_card_firstfile()" },
	{ 0xa0, 0x6a, "dev_card_nextfile()" },
	{ 0xa0, 0x6b, "dev_card_erase()" },
	{ 0xa0, 0x6c, "dev_card_undelete()" },
	{ 0xa0, 0x6d, "dev_card_format()" },
	{ 0xa0, 0x6e, "dev_card_rename()" },
	{ 0xa0, 0x6f, "dev_card_6f()" },
	{ 0xa0, 0x70, "_bu_init()" },
	{ 0xa0, 0x71, "_96_init()" },
	{ 0xa0, 0x72, "_96_remove()" },
	{ 0xa0, 0x73, "sys_a0_73()" },
	{ 0xa0, 0x74, "sys_a0_74()" },
	{ 0xa0, 0x75, "sys_a0_75()" },
	{ 0xa0, 0x76, "sys_a0_76()" },
	{ 0xa0, 0x77, "sys_a0_77()" },
	{ 0xa0, 0x78, "_96_CdSeekL()" },
	{ 0xa0, 0x79, "sys_a0_79()" },
	{ 0xa0, 0x7a, "sys_a0_7a()" },
	{ 0xa0, 0x7b, "sys_a0_7b()" },
	{ 0xa0, 0x7c, "_96_CdGetStatus()" },
	{ 0xa0, 0x7d, "sys_a0_7d()" },
	{ 0xa0, 0x7e, "_96_CdRead()" },
	{ 0xa0, 0x7f, "sys_a0_7f()" },
	{ 0xa0, 0x80, "sys_a0_80()" },
	{ 0xa0, 0x81, "sys_a0_81()" },
	{ 0xa0, 0x82, "sys_a0_82()" },
	{ 0xa0, 0x83, "sys_a0_83()" },
	{ 0xa0, 0x84, "sys_a0_84()" },
	{ 0xa0, 0x85, "_96_CdStop()" },
	{ 0xa0, 0x84, "sys_a0_84()" },
	{ 0xa0, 0x85, "sys_a0_85()" },
	{ 0xa0, 0x86, "sys_a0_86()" },
	{ 0xa0, 0x87, "sys_a0_87()" },
	{ 0xa0, 0x88, "sys_a0_88()" },
	{ 0xa0, 0x89, "sys_a0_89()" },
	{ 0xa0, 0x8a, "sys_a0_8a()" },
	{ 0xa0, 0x8b, "sys_a0_8b()" },
	{ 0xa0, 0x8c, "sys_a0_8c()" },
	{ 0xa0, 0x8d, "sys_a0_8d()" },
	{ 0xa0, 0x8e, "sys_a0_8e()" },
	{ 0xa0, 0x8f, "sys_a0_8f()" },
	{ 0xa0, 0x90, "sys_a0_90()" },
	{ 0xa0, 0x91, "sys_a0_91()" },
	{ 0xa0, 0x92, "sys_a0_92()" },
	{ 0xa0, 0x93, "sys_a0_93()" },
	{ 0xa0, 0x94, "sys_a0_94()" },
	{ 0xa0, 0x95, "sys_a0_95()" },
	{ 0xa0, 0x96, "AddCDROMDevice()" },
	{ 0xa0, 0x97, "AddMemCardDevice()" },
	{ 0xa0, 0x98, "DisableKernelIORedirection()" },
	{ 0xa0, 0x99, "EnableKernelIORedirection()" },
	{ 0xa0, 0x9a, "sys_a0_9a()" },
	{ 0xa0, 0x9b, "sys_a0_9b()" },
	{ 0xa0, 0x9c, "void SetConf(int Event, int TCB, int Stack)" },
	{ 0xa0, 0x9d, "void GetConf(int *Event, int *TCB, int *Stack)" },
	{ 0xa0, 0x9e, "sys_a0_9e()" },
	{ 0xa0, 0x9f, "void SetMem(int size)" },
	{ 0xa0, 0xa0, "_boot()" },
	{ 0xa0, 0xa1, "SystemError()" },
	{ 0xa0, 0xa2, "EnqueueCdIntr()" },
	{ 0xa0, 0xa3, "DequeueCdIntr()" },
	{ 0xa0, 0xa4, "sys_a0_a4()" },
	{ 0xa0, 0xa5, "ReadSector(int count, int sector, void *buffer)" },
	{ 0xa0, 0xa6, "get_cd_status()" },
	{ 0xa0, 0xa7, "bufs_cb_0()" },
	{ 0xa0, 0xa8, "bufs_cb_1()" },
	{ 0xa0, 0xa9, "bufs_cb_2()" },
	{ 0xa0, 0xaa, "bufs_cb_3()" },
	{ 0xa0, 0xab, "_card_info()" },
	{ 0xa0, 0xac, "_card_load()" },
	{ 0xa0, 0xad, "_card_auto()" },
	{ 0xa0, 0xae, "bufs_cb_4()" },
	{ 0xa0, 0xaf, "sys_a0_af()" },
	{ 0xa0, 0xb0, "sys_a0_b0()" },
	{ 0xa0, 0xb1, "sys_a0_b1()" },
	{ 0xa0, 0xb2, "do_a_long_jmp()" },
	{ 0xa0, 0xb3, "sys_a0_b3()" },
	{ 0xa0, 0xb4, "GetKernelInfo(int sub_function)" },
	{ 0xb0, 0x00, "SysMalloc()" },
	{ 0xb0, 0x01, "sys_b0_01()" },
	{ 0xb0, 0x02, "sys_b0_02()" },
	{ 0xb0, 0x03, "sys_b0_03()" },
	{ 0xb0, 0x04, "sys_b0_04()" },
	{ 0xb0, 0x05, "sys_b0_05()" },
	{ 0xb0, 0x06, "sys_b0_06()" },
	{ 0xb0, 0x07, "void DeliverEvent(u_long class, u_long event)" },
	{ 0xb0, 0x08, "long OpenEvent(u_long class, long spec, long mode, long (*func)())" },
	{ 0xb0, 0x09, "long CloseEvent(long event)" },
	{ 0xb0, 0x0a, "long WaitEvent(long event)" },
	{ 0xb0, 0x0b, "long TestEvent(long event)" },
	{ 0xb0, 0x0c, "long EnableEvent(long event)" },
	{ 0xb0, 0x0d, "long DisableEvent(long event)" },
	{ 0xb0, 0x0e, "OpenTh()" },
	{ 0xb0, 0x0f, "CloseTh()" },
	{ 0xb0, 0x10, "ChangeTh()" },
	{ 0xb0, 0x11, "sys_b0_11()" },
	{ 0xb0, 0x12, "int InitPAD(char *buf1, int len1, char *buf2, int len2)" },
	{ 0xb0, 0x13, "int StartPAD(void)" },
	{ 0xb0, 0x14, "int StopPAD(void)" },
	{ 0xb0, 0x15, "PAD_init(u_long nazo, u_long *pad_buf)" },
	{ 0xb0, 0x16, "u_long PAD_dr()" },
	{ 0xb0, 0x17, "void ReturnFromException(void)" },
	{ 0xb0, 0x18, "ResetEntryInt()" },
	{ 0xb0, 0x19, "HookEntryInt()" },
	{ 0xb0, 0x1a, "sys_b0_1a()" },
	{ 0xb0, 0x1b, "sys_b0_1b()" },
	{ 0xb0, 0x1c, "sys_b0_1c()" },
	{ 0xb0, 0x1d, "sys_b0_1d()" },
	{ 0xb0, 0x1e, "sys_b0_1e()" },
	{ 0xb0, 0x1f, "sys_b0_1f()" },
	{ 0xb0, 0x20, "UnDeliverEvent(int class, int event)" },
	{ 0xb0, 0x21, "sys_b0_21()" },
	{ 0xb0, 0x22, "sys_b0_22()" },
	{ 0xb0, 0x23, "sys_b0_23()" },
	{ 0xb0, 0x24, "sys_b0_24()" },
	{ 0xb0, 0x25, "sys_b0_25()" },
	{ 0xb0, 0x26, "sys_b0_26()" },
	{ 0xb0, 0x27, "sys_b0_27()" },
	{ 0xb0, 0x28, "sys_b0_28()" },
	{ 0xb0, 0x29, "sys_b0_29()" },
	{ 0xb0, 0x2a, "sys_b0_2a()" },
	{ 0xb0, 0x2b, "sys_b0_2b()" },
	{ 0xb0, 0x2c, "sys_b0_2c()" },
	{ 0xb0, 0x2d, "sys_b0_2d()" },
	{ 0xb0, 0x2e, "sys_b0_2e()" },
	{ 0xb0, 0x2f, "sys_b0_2f()" },
	{ 0xb0, 0x2f, "sys_b0_30()" },
	{ 0xb0, 0x31, "sys_b0_31()" },
	{ 0xb0, 0x32, "int open(const char *name, int access)" },
	{ 0xb0, 0x33, "int lseek(int fd, long pos, int seektype)" },
	{ 0xb0, 0x34, "int read(int fd, void *buf, int nbytes)" },
	{ 0xb0, 0x35, "int write(int fd, void *buf, int nbytes)" },
	{ 0xb0, 0x36, "close(int fd)" },
	{ 0xb0, 0x37, "int ioctl(int fd, int cmd, int arg)" },
	{ 0xb0, 0x38, "exit(int exitcode)" },
	{ 0xb0, 0x39, "sys_b0_39()" },
	{ 0xb0, 0x3a, "char getc(int fd)" },
	{ 0xb0, 0x3b, "putc(int fd, char ch)" },
	{ 0xb0, 0x3c, "char getchar(void)" },
	{ 0xb0, 0x3d, "putchar(char ch)" },
	{ 0xb0, 0x3e, "char *gets(char *s)" },
	{ 0xb0, 0x3f, "puts(const char *s)" },
	{ 0xb0, 0x40, "int cd(const char *path)" },
	{ 0xb0, 0x41, "int format(const char *fs)" },
	{ 0xb0, 0x42, "struct DIRENTRY* firstfile(const char *name, struct DIRENTRY *dir)" },
	{ 0xb0, 0x43, "struct DIRENTRY* nextfile(struct DIRENTRY *dir)" },
	{ 0xb0, 0x44, "int rename(const char *oldname, const char *newname)" },
	{ 0xb0, 0x45, "int delete(const char *name)" },
	{ 0xb0, 0x46, "undelete()" },
	{ 0xb0, 0x47, "AddDevice()" },
	{ 0xb0, 0x48, "RemoveDevice()" },
	{ 0xb0, 0x49, "PrintInstalledDevices()" },
	{ 0xb0, 0x4a, "InitCARD()" },
	{ 0xb0, 0x4b, "StartCARD()" },
	{ 0xb0, 0x4c, "StopCARD()" },
	{ 0xb0, 0x4d, "sys_b0_4d()" },
	{ 0xb0, 0x4e, "_card_write()" },
	{ 0xb0, 0x4f, "_card_read()" },
	{ 0xb0, 0x50, "_new_card()" },
	{ 0xb0, 0x51, "void *Krom2RawAdd(int code)" },
	{ 0xb0, 0x52, "sys_b0_52()" },
	{ 0xb0, 0x53, "sys_b0_53()" },
	{ 0xb0, 0x54, "long _get_errno(void)" },
	{ 0xb0, 0x55, "long _get_error(long fd)" },
	{ 0xb0, 0x56, "GetC0Table()" },
	{ 0xb0, 0x57, "GetB0Table()" },
	{ 0xb0, 0x58, "_card_chan()" },
	{ 0xb0, 0x59, "sys_b0_59()" },
	{ 0xb0, 0x5a, "sys_b0_5a()" },
	{ 0xb0, 0x5b, "ChangeClearPAD(int, int)" },
	{ 0xb0, 0x5c, "_card_status()" },
	{ 0xb0, 0x5d, "_card_wait()" },
	{ 0xc0, 0x00, "InitRCnt()" },
	{ 0xc0, 0x01, "InitException()" },
	{ 0xc0, 0x02, "SysEnqIntRP(int index, long *queue)" },
	{ 0xc0, 0x03, "SysDeqIntRP(int index, long *queue)" },
	{ 0xc0, 0x04, "int get_free_EvCB_slot(void)" },
	{ 0xc0, 0x05, "get_free_TCB_slot()" },
	{ 0xc0, 0x06, "ExceptionHandler()" },
	{ 0xc0, 0x07, "InstallExceptionHandlers()" },
	{ 0xc0, 0x08, "SysInitMemory()" },
	{ 0xc0, 0x09, "SysInitKMem()" },
	{ 0xc0, 0x0a, "ChangeClearRCnt()" },
	{ 0xc0, 0x0b, "SystemError()" },
	{ 0xc0, 0x0c, "InitDefInt()" },
	{ 0xc0, 0x0d, "sys_c0_0d()" },
	{ 0xc0, 0x0e, "sys_c0_0e()" },
	{ 0xc0, 0x0f, "sys_c0_0f()" },
	{ 0xc0, 0x10, "sys_c0_10()" },
	{ 0xc0, 0x11, "sys_c0_11()" },
	{ 0xc0, 0x12, "InstallDevices()" },
	{ 0xc0, 0x13, "FlushStdInOutPut()" },
	{ 0xc0, 0x14, "sys_c0_14()" },
	{ 0xc0, 0x15, "_cdevinput()" },
	{ 0xc0, 0x16, "_cdevscan()" },
	{ 0xc0, 0x17, "char _circgetc(struct device_buf *circ)" },
	{ 0xc0, 0x18, "_circputc(char c, struct device_buf *circ)" },
	{ 0xc0, 0x19, "ioabort(const char *str)" },
	{ 0xc0, 0x1a, "sys_c0_1a()" },
	{ 0xc0, 0x1b, "KernelRedirect(int flag)" },
	{ 0xc0, 0x1c, "PatchA0Table()" },
	{ 0x00, 0x00, NULL }
};

static UINT32 log_bioscall_parameter( int parm )
{
	if( parm < 4 )
	{
		return activecpu_get_reg( MIPS_R4 + parm );
	}
	else
	{
		return program_read_dword_32le( activecpu_get_reg( MIPS_R29 ) + ( parm * 4 ) );
	}
}

static const char *log_bioscall_string( int parm )
{
	int pos;
	UINT32 address;
	static char string[ 1024 ];

	address = log_bioscall_parameter( parm );
	if( address == 0 )
	{
		return "NULL";
	}

	pos = 0;
	string[ pos++ ] = '\"';

	for( ;; )
	{
		UINT8 c = program_read_byte_32le( address );
		if( c == 0 )
		{
			break;
		}
		else if( c == '\t' )
		{
			string[ pos++ ] = '\\';
			string[ pos++ ] = 't';
		}
		else if( c == '\r' )
		{
			string[ pos++ ] = '\\';
			string[ pos++ ] = 'r';
		}
		else if( c == '\n' )
		{
			string[ pos++ ] = '\\';
			string[ pos++ ] = 'n';
		}
		else if( c < 32 || c > 127 )
		{
			string[ pos++ ] = '\\';
			string[ pos++ ] = ( ( c / 64 ) % 8 ) + '0';
			string[ pos++ ] = ( ( c / 8 ) % 8 ) + '0';
			string[ pos++ ] = ( ( c / 1 ) % 8 ) + '0';
		}
		else
		{
			string[ pos++ ] = c;
		}
		address++;
	}

	string[ pos++ ] = '\"';
	string[ pos++ ] = 0;

	return string;
}

static const char *log_bioscall_hex( int parm )
{
	static char string[ 1024 ];

	sprintf( string, "0x%08x", log_bioscall_parameter( parm ) );

	return string;
}

static const char *log_bioscall_char( int parm )
{
	int c;
	static char string[ 1024 ];

	c = log_bioscall_parameter( parm );
	if( c < 32 || c > 127 )
	{
		sprintf( string, "0x%02x", c );
	}
	else
	{
		sprintf( string, "'%c'", c );
	}

	return string;
}

static void log_bioscall( void )
{
	int address = activecpu_get_reg( MIPS_PC ) - 0x04;
	if( address == 0xa0 ||
		address == 0xb0 ||
		address == 0xc0 )
	{
		char buf[ 1024 ];
		int operation = activecpu_get_reg( MIPS_R9 ) & 0xff;
		int bioscall = 0;

		if( ( address == 0xa0 && operation == 0x3c ) ||
			( address == 0xb0 && operation == 0x3d ) )
		{
			putchar( log_bioscall_parameter( 0 ) );
		}

		if( ( address == 0xa0 && operation == 0x03 ) ||
			( address == 0xb0 && operation == 0x35 ) )
		{
			int fd = log_bioscall_parameter( 0 );
			int buf = log_bioscall_parameter( 1 );
			int nbytes = log_bioscall_parameter( 2 );
			if( fd == 1 )
			{
				while( nbytes > 0 )
				{
					UINT8 c = program_read_byte_32le( buf );
					putchar( c );
					nbytes--;
					buf++;
				}
			}
		}

		while( bioscalls[ bioscall ].prototype != NULL &&
			( bioscalls[ bioscall ].address != address ||
			bioscalls[ bioscall ].operation != operation ) )
		{
			bioscall++;
		}

		if( bioscalls[ bioscall ].prototype != NULL )
		{
			const char *prototype = bioscalls[ bioscall ].prototype;
			const char *parmstart = NULL;
			int parm = 0;
			int parmlen = -1;
			int brackets = 0;
			int pos = 0;

			while( *( prototype ) != 0 )
			{
				int ch = *( prototype );
				switch( ch )
				{
				case '(':
					brackets++;
					prototype++;
					if( brackets == 1 )
					{
						buf[ pos++ ] = ch;
						parmstart = prototype;
					}
					break;
				case ')':
					if( brackets == 1 )
					{
						parmlen = prototype - parmstart;
					}
					prototype++;
					brackets--;
					break;
				case ',':
					if( brackets == 1 )
					{
						parmlen = prototype - parmstart;
					}
					prototype++;
					break;
				default:
					if( brackets == 0 )
					{
						buf[ pos++ ] = ch;
					}
					prototype++;
					break;
				}

				if( parmlen >= 0 )
				{
					while( parmlen > 0 && parmstart[ 0 ] == ' ' )
					{
						parmstart++;
						parmlen--;
					}
					while( parmlen > 0 && parmstart[ parmlen - 1 ] == ' ' )
					{
						parmlen--;
					}

					if( parmlen == 0 ||
						( parmlen == 4 && memcmp( parmstart, "void", 4 ) == 0 ) )
					{
						parm = -1;
					}
					else if( parmlen == 3 && memcmp( parmstart, "...", 3 ) == 0 )
					{
						if( parm > 0 )
						{
							UINT32 format = log_bioscall_parameter( parm - 1 );
							const char *parmstr = NULL;
							int percent = 0;
							for( ;; )
							{
								UINT8 c = program_read_byte_32le( format );
								if( c == 0 )
								{
									break;
								}
								if( percent == 0 )
								{
									if( c == '%' )
									{
										percent = 1;
									}
								}
								else
								{
									if( c == '%' )
									{
										percent = 0;
									}
									else if( c == '*' )
									{
										parmstr = log_bioscall_hex( parm );
									}
									else if( c == 's' )
									{
										parmstr = log_bioscall_string( parm );
										percent = 0;
									}
									else if( c == 'c' )
									{
										parmstr = log_bioscall_char( parm );
										percent = 0;
									}
									else if( c != '-' && c != '.' && c != 'l' && ( c < '0' || c > '9' ) )
									{
										parmstr = log_bioscall_hex( parm );
										percent = 0;
									}
								}

								if( parmstr != NULL )
								{
									if( parm > 0 )
									{
										buf[ pos++ ] = ',';
									}
									buf[ pos++ ] = ' ';

									strcpy( &buf[ pos ], parmstr );
									pos += strlen( parmstr );
									parmstr = NULL;

									parm++;
								}
								format++;
							}
						}
					}
					else if( parmlen > 0 )
					{
						const char *parmstr;

						int typelen = parmlen;
						while( typelen > 0 && parmstart[ typelen - 1 ] != ' ' && parmstart[ typelen - 1 ] != '*' )
						{
							typelen--;
						}

						if( typelen == 5 && memcmp( parmstart, "char ", 5 ) == 0 )
						{
							parmstr = log_bioscall_char( parm );
						}
						else if( typelen == 12 && memcmp( parmstart, "const char *", 12 ) == 0 )
						{
							parmstr = log_bioscall_string( parm );
						}
						else
						{
							parmstr = log_bioscall_hex( parm );
						}

						if( parm > 0 )
						{
							buf[ pos++ ] = ',';
						}
						buf[ pos++ ] = ' ';

						strcpy( &buf[ pos ], parmstr );
						pos += strlen( parmstr );
					}

					parmlen = -1;
					parm++;

					if( ch == ',' )
					{
						parmstart = prototype;
					}
					else
					{
						if( parm > 0 )
						{
							buf[ pos++ ] = ' ';
						}
						buf[ pos++ ] = ch;
					}
				}
			}
			buf[ pos ] = 0;
		}
		else
		{
			sprintf( buf, "unknown_%02x_%02x", address, operation );
		}
		logerror( "%08x: bioscall %s\n", (unsigned int)activecpu_get_reg( MIPS_R31 ) - 8, buf );
	}
}

static void log_syscall( void )
{
	char buf[ 1024 ];
	int operation = activecpu_get_reg( MIPS_R4 );
	switch( operation )
	{
	case 0:
		strcpy( buf, "void Exception()" );
		break;
	case 1:
		strcpy( buf, "void EnterCriticalSection()" );
		break;
	case 2:
		strcpy( buf, "void ExitCriticalSection()" );
		break;
	default:
		sprintf( buf, "unknown_%02x", operation );
		break;
	}
	logerror( "%08x: syscall %s\n", (unsigned int)activecpu_get_reg( MIPS_R31 ) - 8, buf );
}

#endif

INLINE void mips_set_cp0r( int reg, UINT32 value )
{
	mipscpu.cp0r[ reg ] = value;
	if( reg == CP0_SR || reg == CP0_CAUSE )
	{
		if( ( mipscpu.cp0r[ CP0_SR ] & SR_IEC ) != 0 && ( mipscpu.cp0r[ CP0_SR ] & mipscpu.cp0r[ CP0_CAUSE ] & CAUSE_IP ) != 0 )
		{
			mips_exception( EXC_INT );
		}
		else if( mipscpu.delayr != REGPC && ( mipscpu.pc & ( ( ( mipscpu.cp0r[ CP0_SR ] & SR_KUC ) << 30 ) | 3 ) ) != 0 )
		{
			mips_exception( EXC_ADEL );
			mips_set_cp0r( CP0_BADVADDR, mipscpu.pc );
		}
	}
}

INLINE void mips_commit_delayed_load( void )
{
	if( mipscpu.delayr != 0 )
	{
		mipscpu.r[ mipscpu.delayr ] = mipscpu.delayv;
		mipscpu.delayr = 0;
		mipscpu.delayv = 0;
	}
}

INLINE void mips_delayed_branch( UINT32 n_adr )
{
	if( ( n_adr & ( ( ( mipscpu.cp0r[ CP0_SR ] & SR_KUC ) << 30 ) | 3 ) ) != 0 )
	{
		mips_exception( EXC_ADEL );
		mips_set_cp0r( CP0_BADVADDR, n_adr );
	}
	else if( mipscpu.delayr == REGPC )
	{
		mipscpu.pc = mipscpu.delayv;
		change_pc( mipscpu.delayv );
		mipscpu.delayv = n_adr;
	}
	else
	{
		mips_commit_delayed_load();
		mipscpu.delayr = REGPC;
		mipscpu.delayv = n_adr;
		mipscpu.pc += 4;
	}
}

INLINE void mips_set_pc( unsigned val )
{
	mipscpu.pc = val;

	change_pc( val );
	mipscpu.delayr = 0;
	mipscpu.delayv = 0;
}

INLINE void mips_advance_pc( void )
{
	if( mipscpu.delayr == REGPC )
	{
		mips_set_pc( mipscpu.delayv );
	}
	else
	{
		mips_commit_delayed_load();
		mipscpu.pc += 4;
	}
}

INLINE void mips_load( UINT32 n_r, UINT32 n_v )
{
	mips_advance_pc();
	if( n_r != 0 )
	{
		mipscpu.r[ n_r ] = n_v;
	}
}

INLINE void mips_delayed_load( UINT32 n_r, UINT32 n_v )
{
	if( mipscpu.delayr == REGPC )
	{
		mips_set_pc( mipscpu.delayv );
		mipscpu.delayr = n_r;
		mipscpu.delayv = n_v;
	}
	else
	{
		mips_commit_delayed_load();
		mipscpu.pc += 4;
		if( n_r != 0 )
		{
			mipscpu.r[ n_r ] = n_v;
		}
	}
}

static void mips_exception( int exception )
{
	mips_set_cp0r( CP0_SR, ( mipscpu.cp0r[ CP0_SR ] & ~0x3f ) | ( ( mipscpu.cp0r[ CP0_SR ] << 2 ) & 0x3f ) );
	if( mipscpu.delayr == REGPC )
	{
		mips_set_cp0r( CP0_EPC, mipscpu.pc - 4 );
		mips_set_cp0r( CP0_CAUSE, ( mipscpu.cp0r[ CP0_CAUSE ] & ~CAUSE_EXC ) | CAUSE_BD | ( exception << 2 ) );
	}
	else
	{
		mips_commit_delayed_load();
		mips_set_cp0r( CP0_EPC, mipscpu.pc );
		mips_set_cp0r( CP0_CAUSE, ( mipscpu.cp0r[ CP0_CAUSE ] & ~( CAUSE_EXC | CAUSE_BD ) ) | ( exception << 2 ) );
	}
	if( mipscpu.cp0r[ CP0_SR ] & SR_BEV )
	{
		mips_set_pc( 0xbfc00180 );
	}
	else
	{
		mips_set_pc( 0x80000080 );
	}
}

static void mips_init( int index, int clock, const void *config, int (*irqcallback)(int) )
{
	mipscpu.irq_callback = irqcallback;

	state_save_register_item( "psxcpu", index, mipscpu.op );
	state_save_register_item( "psxcpu", index, mipscpu.pc );
	state_save_register_item( "psxcpu", index, mipscpu.delayv );
	state_save_register_item( "psxcpu", index, mipscpu.delayr );
	state_save_register_item( "psxcpu", index, mipscpu.hi );
	state_save_register_item( "psxcpu", index, mipscpu.lo );
	state_save_register_item_array( "psxcpu", index, mipscpu.r );
	state_save_register_item_array( "psxcpu", index, mipscpu.cp0r );
	state_save_register_item_array( "psxcpu", index, mipscpu.cp2cr );
	state_save_register_item_array( "psxcpu", index, mipscpu.cp2dr );
}

static void mips_reset( void )
{
	mips_set_cp0r( CP0_SR, ( mipscpu.cp0r[ CP0_SR ] & ~( SR_TS | SR_SWC | SR_KUC | SR_IEC ) ) | SR_BEV );
	mips_set_cp0r( CP0_RANDOM, 63 ); /* todo: */
	mips_set_cp0r( CP0_PRID, 0x00000200 ); /* todo: */
	mips_set_pc( 0xbfc00000 );
}

static void mips_exit( void )
{
}

static int mips_execute( int cycles )
{
	UINT32 n_res;

	mips_ICount = cycles;
	do
	{
#if LOG_BIOSCALL
		log_bioscall();
#endif

		CALL_MAME_DEBUG;

		mipscpu.op = cpu_readop32( mipscpu.pc );
		switch( INS_OP( mipscpu.op ) )
		{
		case OP_SPECIAL:
			switch( INS_FUNCT( mipscpu.op ) )
			{
			case FUNCT_SLL:
				mips_load( INS_RD( mipscpu.op ), mipscpu.r[ INS_RT( mipscpu.op ) ] << INS_SHAMT( mipscpu.op ) );
				break;
			case FUNCT_SRL:
				mips_load( INS_RD( mipscpu.op ), mipscpu.r[ INS_RT( mipscpu.op ) ] >> INS_SHAMT( mipscpu.op ) );
				break;
			case FUNCT_SRA:
				mips_load( INS_RD( mipscpu.op ), (INT32)mipscpu.r[ INS_RT( mipscpu.op ) ] >> INS_SHAMT( mipscpu.op ) );
				break;
			case FUNCT_SLLV:
				mips_load( INS_RD( mipscpu.op ), mipscpu.r[ INS_RT( mipscpu.op ) ] << ( mipscpu.r[ INS_RS( mipscpu.op ) ] & 31 ) );
				break;
			case FUNCT_SRLV:
				mips_load( INS_RD( mipscpu.op ), mipscpu.r[ INS_RT( mipscpu.op ) ] >> ( mipscpu.r[ INS_RS( mipscpu.op ) ] & 31 ) );
				break;
			case FUNCT_SRAV:
				mips_load( INS_RD( mipscpu.op ), (INT32)mipscpu.r[ INS_RT( mipscpu.op ) ] >> ( mipscpu.r[ INS_RS( mipscpu.op ) ] & 31 ) );
				break;
			case FUNCT_JR:
				if( INS_RD( mipscpu.op ) != 0 )
				{
					mips_exception( EXC_RI );
				}
				else
				{
					mips_delayed_branch( mipscpu.r[ INS_RS( mipscpu.op ) ] );
				}
				break;
			case FUNCT_JALR:
				n_res = mipscpu.pc + 8;
				mips_delayed_branch( mipscpu.r[ INS_RS( mipscpu.op ) ] );
				if( INS_RD( mipscpu.op ) != 0 )
				{
					mipscpu.r[ INS_RD( mipscpu.op ) ] = n_res;
				}
				break;
			case FUNCT_SYSCALL:
#if LOG_BIOSCALL
				log_syscall();
#endif
				mips_exception( EXC_SYS );
				break;
			case FUNCT_BREAK:
				mips_exception( EXC_BP );
				break;
			case FUNCT_MFHI:
				mips_load( INS_RD( mipscpu.op ), mipscpu.hi );
				break;
			case FUNCT_MTHI:
				if( INS_RD( mipscpu.op ) != 0 )
				{
					mips_exception( EXC_RI );
				}
				else
				{
					mips_advance_pc();
					mipscpu.hi = mipscpu.r[ INS_RS( mipscpu.op ) ];
				}
				break;
			case FUNCT_MFLO:
				mips_load( INS_RD( mipscpu.op ),  mipscpu.lo );
				break;
			case FUNCT_MTLO:
				if( INS_RD( mipscpu.op ) != 0 )
				{
					mips_exception( EXC_RI );
				}
				else
				{
					mips_advance_pc();
					mipscpu.lo = mipscpu.r[ INS_RS( mipscpu.op ) ];
				}
				break;
			case FUNCT_MULT:
				if( INS_RD( mipscpu.op ) != 0 )
				{
					mips_exception( EXC_RI );
				}
				else
				{
					INT64 n_res64;
					n_res64 = MUL_64_32_32( (INT32)mipscpu.r[ INS_RS( mipscpu.op ) ], (INT32)mipscpu.r[ INS_RT( mipscpu.op ) ] );
					mips_advance_pc();
					mipscpu.lo = LO32_32_64( n_res64 );
					mipscpu.hi = HI32_32_64( n_res64 );
				}
				break;
			case FUNCT_MULTU:
				if( INS_RD( mipscpu.op ) != 0 )
				{
					mips_exception( EXC_RI );
				}
				else
				{
					UINT64 n_res64;
					n_res64 = MUL_U64_U32_U32( mipscpu.r[ INS_RS( mipscpu.op ) ], mipscpu.r[ INS_RT( mipscpu.op ) ] );
					mips_advance_pc();
					mipscpu.lo = LO32_U32_U64( n_res64 );
					mipscpu.hi = HI32_U32_U64( n_res64 );
				}
				break;
			case FUNCT_DIV:
				if( INS_RD( mipscpu.op ) != 0 )
				{
					mips_exception( EXC_RI );
				}
				else
				{
					UINT32 n_div;
					UINT32 n_mod;
					if( mipscpu.r[ INS_RT( mipscpu.op ) ] != 0 )
					{
						n_div = (INT32)mipscpu.r[ INS_RS( mipscpu.op ) ] / (INT32)mipscpu.r[ INS_RT( mipscpu.op ) ];
						n_mod = (INT32)mipscpu.r[ INS_RS( mipscpu.op ) ] % (INT32)mipscpu.r[ INS_RT( mipscpu.op ) ];
						mips_advance_pc();
						mipscpu.lo = n_div;
						mipscpu.hi = n_mod;
					}
					else
					{
						mips_advance_pc();
					}
				}
				break;
			case FUNCT_DIVU:
				if( INS_RD( mipscpu.op ) != 0 )
				{
					mips_exception( EXC_RI );
				}
				else
				{
					UINT32 n_div;
					UINT32 n_mod;
					if( mipscpu.r[ INS_RT( mipscpu.op ) ] != 0 )
					{
						n_div = mipscpu.r[ INS_RS( mipscpu.op ) ] / mipscpu.r[ INS_RT( mipscpu.op ) ];
						n_mod = mipscpu.r[ INS_RS( mipscpu.op ) ] % mipscpu.r[ INS_RT( mipscpu.op ) ];
						mips_advance_pc();
						mipscpu.lo = n_div;
						mipscpu.hi = n_mod;
					}
					else
					{
						mips_advance_pc();
					}
				}
				break;
			case FUNCT_ADD:
				{
					n_res = mipscpu.r[ INS_RS( mipscpu.op ) ] + mipscpu.r[ INS_RT( mipscpu.op ) ];
					if( (INT32)( ~( mipscpu.r[ INS_RS( mipscpu.op ) ] ^ mipscpu.r[ INS_RT( mipscpu.op ) ] ) & ( mipscpu.r[ INS_RS( mipscpu.op ) ] ^ n_res ) ) < 0 )
					{
						mips_exception( EXC_OVF );
					}
					else
					{
						mips_load( INS_RD( mipscpu.op ), n_res );
					}
				}
				break;
			case FUNCT_ADDU:
				mips_load( INS_RD( mipscpu.op ), mipscpu.r[ INS_RS( mipscpu.op ) ] + mipscpu.r[ INS_RT( mipscpu.op ) ] );
				break;
			case FUNCT_SUB:
				n_res = mipscpu.r[ INS_RS( mipscpu.op ) ] - mipscpu.r[ INS_RT( mipscpu.op ) ];
				if( (INT32)( ( mipscpu.r[ INS_RS( mipscpu.op ) ] ^ mipscpu.r[ INS_RT( mipscpu.op ) ] ) & ( mipscpu.r[ INS_RS( mipscpu.op ) ] ^ n_res ) ) < 0 )
				{
					mips_exception( EXC_OVF );
				}
				else
				{
					mips_load( INS_RD( mipscpu.op ), n_res );
				}
				break;
			case FUNCT_SUBU:
				mips_load( INS_RD( mipscpu.op ), mipscpu.r[ INS_RS( mipscpu.op ) ] - mipscpu.r[ INS_RT( mipscpu.op ) ] );
				break;
			case FUNCT_AND:
				mips_load( INS_RD( mipscpu.op ), mipscpu.r[ INS_RS( mipscpu.op ) ] & mipscpu.r[ INS_RT( mipscpu.op ) ] );
				break;
			case FUNCT_OR:
				mips_load( INS_RD( mipscpu.op ), mipscpu.r[ INS_RS( mipscpu.op ) ] | mipscpu.r[ INS_RT( mipscpu.op ) ] );
				break;
			case FUNCT_XOR:
				mips_load( INS_RD( mipscpu.op ), mipscpu.r[ INS_RS( mipscpu.op ) ] ^ mipscpu.r[ INS_RT( mipscpu.op ) ] );
				break;
			case FUNCT_NOR:
				mips_load( INS_RD( mipscpu.op ), ~( mipscpu.r[ INS_RS( mipscpu.op ) ] | mipscpu.r[ INS_RT( mipscpu.op ) ] ) );
				break;
			case FUNCT_SLT:
				mips_load( INS_RD( mipscpu.op ), (INT32)mipscpu.r[ INS_RS( mipscpu.op ) ] < (INT32)mipscpu.r[ INS_RT( mipscpu.op ) ] );
				break;
			case FUNCT_SLTU:
				mips_load( INS_RD( mipscpu.op ), mipscpu.r[ INS_RS( mipscpu.op ) ] < mipscpu.r[ INS_RT( mipscpu.op ) ] );
				break;
			default:
				mips_exception( EXC_RI );
				break;
			}
			break;
		case OP_REGIMM:
			switch( INS_RT( mipscpu.op ) )
			{
			case RT_BLTZ:
				if( (INT32)mipscpu.r[ INS_RS( mipscpu.op ) ] < 0 )
				{
					mips_delayed_branch( mipscpu.pc + 4 + ( MIPS_WORD_EXTEND( INS_IMMEDIATE( mipscpu.op ) ) << 2 ) );
				}
				else
				{
					mips_advance_pc();
				}
				break;
			case RT_BGEZ:
				if( (INT32)mipscpu.r[ INS_RS( mipscpu.op ) ] >= 0 )
				{
					mips_delayed_branch( mipscpu.pc + 4 + ( MIPS_WORD_EXTEND( INS_IMMEDIATE( mipscpu.op ) ) << 2 ) );
				}
				else
				{
					mips_advance_pc();
				}
				break;
			case RT_BLTZAL:
				n_res = mipscpu.pc + 8;
				if( (INT32)mipscpu.r[ INS_RS( mipscpu.op ) ] < 0 )
				{
					mips_delayed_branch( mipscpu.pc + 4 + ( MIPS_WORD_EXTEND( INS_IMMEDIATE( mipscpu.op ) ) << 2 ) );
				}
				else
				{
					mips_advance_pc();
				}
				mipscpu.r[ 31 ] = n_res;
				break;
			case RT_BGEZAL:
				n_res = mipscpu.pc + 8;
				if( (INT32)mipscpu.r[ INS_RS( mipscpu.op ) ] >= 0 )
				{
					mips_delayed_branch( mipscpu.pc + 4 + ( MIPS_WORD_EXTEND( INS_IMMEDIATE( mipscpu.op ) ) << 2 ) );
				}
				else
				{
					mips_advance_pc();
				}
				mipscpu.r[ 31 ] = n_res;
				break;
			}
			break;
		case OP_J:
			mips_delayed_branch( ( ( mipscpu.pc + 4 ) & 0xf0000000 ) + ( INS_TARGET( mipscpu.op ) << 2 ) );
			break;
		case OP_JAL:
			n_res = mipscpu.pc + 8;
			mips_delayed_branch( ( ( mipscpu.pc + 4 ) & 0xf0000000 ) + ( INS_TARGET( mipscpu.op ) << 2 ) );
			mipscpu.r[ 31 ] = n_res;
			break;
		case OP_BEQ:
			if( mipscpu.r[ INS_RS( mipscpu.op ) ] == mipscpu.r[ INS_RT( mipscpu.op ) ] )
			{
				mips_delayed_branch( mipscpu.pc + 4 + ( MIPS_WORD_EXTEND( INS_IMMEDIATE( mipscpu.op ) ) << 2 ) );
			}
			else
			{
				mips_advance_pc();
			}
			break;
		case OP_BNE:
			if( mipscpu.r[ INS_RS( mipscpu.op ) ] != mipscpu.r[ INS_RT( mipscpu.op ) ] )
			{
				mips_delayed_branch( mipscpu.pc + 4 + ( MIPS_WORD_EXTEND( INS_IMMEDIATE( mipscpu.op ) ) << 2 ) );
			}
			else
			{
				mips_advance_pc();
			}
			break;
		case OP_BLEZ:
			if( INS_RT( mipscpu.op ) != 0 )
			{
				mips_exception( EXC_RI );
			}
			else if( (INT32)mipscpu.r[ INS_RS( mipscpu.op ) ] <= 0 )
			{
				mips_delayed_branch( mipscpu.pc + 4 + ( MIPS_WORD_EXTEND( INS_IMMEDIATE( mipscpu.op ) ) << 2 ) );
			}
			else
			{
				mips_advance_pc();
			}
			break;
		case OP_BGTZ:
			if( INS_RT( mipscpu.op ) != 0 )
			{
				mips_exception( EXC_RI );
			}
			else if( (INT32)mipscpu.r[ INS_RS( mipscpu.op ) ] > 0 )
			{
				mips_delayed_branch( mipscpu.pc + 4 + ( MIPS_WORD_EXTEND( INS_IMMEDIATE( mipscpu.op ) ) << 2 ) );
			}
			else
			{
				mips_advance_pc();
			}
			break;
		case OP_ADDI:
			{
				UINT32 n_imm;
				n_imm = MIPS_WORD_EXTEND( INS_IMMEDIATE( mipscpu.op ) );
				n_res = mipscpu.r[ INS_RS( mipscpu.op ) ] + n_imm;
				if( (INT32)( ~( mipscpu.r[ INS_RS( mipscpu.op ) ] ^ n_imm ) & ( mipscpu.r[ INS_RS( mipscpu.op ) ] ^ n_res ) ) < 0 )
				{
					mips_exception( EXC_OVF );
				}
				else
				{
					mips_load( INS_RT( mipscpu.op ), n_res );
				}
			}
			break;
		case OP_ADDIU:
			mips_load( INS_RT( mipscpu.op ), mipscpu.r[ INS_RS( mipscpu.op ) ] + MIPS_WORD_EXTEND( INS_IMMEDIATE( mipscpu.op ) ) );
			break;
		case OP_SLTI:
			mips_load( INS_RT( mipscpu.op ), (INT32)mipscpu.r[ INS_RS( mipscpu.op ) ] < MIPS_WORD_EXTEND( INS_IMMEDIATE( mipscpu.op ) ) );
			break;
		case OP_SLTIU:
			mips_load( INS_RT( mipscpu.op ), mipscpu.r[ INS_RS( mipscpu.op ) ] < (UINT32)MIPS_WORD_EXTEND( INS_IMMEDIATE( mipscpu.op ) ) );
			break;
		case OP_ANDI:
			mips_load( INS_RT( mipscpu.op ), mipscpu.r[ INS_RS( mipscpu.op ) ] & INS_IMMEDIATE( mipscpu.op ) );
			break;
		case OP_ORI:
			mips_load( INS_RT( mipscpu.op ), mipscpu.r[ INS_RS( mipscpu.op ) ] | INS_IMMEDIATE( mipscpu.op ) );
			break;
		case OP_XORI:
			mips_load( INS_RT( mipscpu.op ), mipscpu.r[ INS_RS( mipscpu.op ) ] ^ INS_IMMEDIATE( mipscpu.op ) );
			break;
		case OP_LUI:
			mips_load( INS_RT( mipscpu.op ), INS_IMMEDIATE( mipscpu.op ) << 16 );
			break;
		case OP_COP0:
			if( ( mipscpu.cp0r[ CP0_SR ] & SR_KUC ) != 0 && ( mipscpu.cp0r[ CP0_SR ] & SR_CU0 ) == 0 )
			{
				mips_exception( EXC_CPU );
				mips_set_cp0r( CP0_CAUSE, ( mipscpu.cp0r[ CP0_CAUSE ] & ~CAUSE_CE ) | CAUSE_CE0 );
			}
			else
			{
				switch( INS_RS( mipscpu.op ) )
				{
				case RS_MFC:
					mips_delayed_load( INS_RT( mipscpu.op ), mipscpu.cp0r[ INS_RD( mipscpu.op ) ] );
					break;
				case RS_CFC:
					/* todo: */
					logerror( "%08x: COP0 CFC not supported\n", mipscpu.pc );
					mips_stop();
					mips_advance_pc();
					break;
				case RS_MTC:
					n_res = ( mipscpu.cp0r[ INS_RD( mipscpu.op ) ] & ~mips_mtc0_writemask[ INS_RD( mipscpu.op ) ] ) |
						( mipscpu.r[ INS_RT( mipscpu.op ) ] & mips_mtc0_writemask[ INS_RD( mipscpu.op ) ] );
					mips_advance_pc();
					mips_set_cp0r( INS_RD( mipscpu.op ), n_res );
					break;
				case RS_CTC:
					/* todo: */
					logerror( "%08x: COP0 CTC not supported\n", mipscpu.pc );
					mips_stop();
					mips_advance_pc();
					break;
				case RS_BC:
					switch( INS_RT( mipscpu.op ) )
					{
					case RT_BCF:
						/* todo: */
						logerror( "%08x: COP0 BCF not supported\n", mipscpu.pc );
						mips_stop();
						mips_advance_pc();
						break;
					case RT_BCT:
						/* todo: */
						logerror( "%08x: COP0 BCT not supported\n", mipscpu.pc );
						mips_stop();
						mips_advance_pc();
						break;
					default:
						/* todo: */
						logerror( "%08x: COP0 unknown command %08x\n", mipscpu.pc, mipscpu.op );
						mips_stop();
						mips_advance_pc();
						break;
					}
					break;
				default:
					switch( INS_CO( mipscpu.op ) )
					{
					case 1:
						switch( INS_CF( mipscpu.op ) )
						{
						case CF_RFE:
							mips_advance_pc();
							mips_set_cp0r( CP0_SR, ( mipscpu.cp0r[ CP0_SR ] & ~0xf ) | ( ( mipscpu.cp0r[ CP0_SR ] >> 2 ) & 0xf ) );
							break;
						default:
							/* todo: */
							logerror( "%08x: COP0 unknown command %08x\n", mipscpu.pc, mipscpu.op );
							mips_stop();
							mips_advance_pc();
							break;
						}
						break;
					default:
						/* todo: */
						logerror( "%08x: COP0 unknown command %08x\n", mipscpu.pc, mipscpu.op );
						mips_stop();
						mips_advance_pc();
						break;
					}
					break;
				}
			}
			break;
		case OP_COP1:
			if( ( mipscpu.cp0r[ CP0_SR ] & SR_CU1 ) == 0 )
			{
				mips_exception( EXC_CPU );
				mips_set_cp0r( CP0_CAUSE, ( mipscpu.cp0r[ CP0_CAUSE ] & ~CAUSE_CE ) | CAUSE_CE1 );
			}
			else
			{
				switch( INS_RS( mipscpu.op ) )
				{
				case RS_MFC:
					/* todo: */
					logerror( "%08x: COP1 BCT not supported\n", mipscpu.pc );
					mips_stop();
					mips_advance_pc();
					break;
				case RS_CFC:
					/* todo: */
					logerror( "%08x: COP1 CFC not supported\n", mipscpu.pc );
					mips_stop();
					mips_advance_pc();
					break;
				case RS_MTC:
					/* todo: */
					logerror( "%08x: COP1 MTC not supported\n", mipscpu.pc );
					mips_stop();
					mips_advance_pc();
					break;
				case RS_CTC:
					/* todo: */
					logerror( "%08x: COP1 CTC not supported\n", mipscpu.pc );
					mips_stop();
					mips_advance_pc();
					break;
				case RS_BC:
					switch( INS_RT( mipscpu.op ) )
					{
					case RT_BCF:
						/* todo: */
						logerror( "%08x: COP1 BCF not supported\n", mipscpu.pc );
						mips_stop();
						mips_advance_pc();
						break;
					case RT_BCT:
						/* todo: */
						logerror( "%08x: COP1 BCT not supported\n", mipscpu.pc );
						mips_stop();
						mips_advance_pc();
						break;
					default:
						/* todo: */
						logerror( "%08x: COP1 unknown command %08x\n", mipscpu.pc, mipscpu.op );
						mips_stop();
						mips_advance_pc();
						break;
					}
					break;
				default:
					switch( INS_CO( mipscpu.op ) )
					{
					case 1:
						/* todo: */
						logerror( "%08x: COP1 unknown command %08x\n", mipscpu.pc, mipscpu.op );
						mips_stop();
						mips_advance_pc();
						break;
					default:
						/* todo: */
						logerror( "%08x: COP1 unknown command %08x\n", mipscpu.pc, mipscpu.op );
						mips_stop();
						mips_advance_pc();
						break;
					}
					break;
				}
			}
			break;
		case OP_COP2:
			if( ( mipscpu.cp0r[ CP0_SR ] & SR_CU2 ) == 0 )
			{
				mips_exception( EXC_CPU );
				mips_set_cp0r( CP0_CAUSE, ( mipscpu.cp0r[ CP0_CAUSE ] & ~CAUSE_CE ) | CAUSE_CE2 );
			}
			else
			{
				switch( INS_RS( mipscpu.op ) )
				{
				case RS_MFC:
					mips_delayed_load( INS_RT( mipscpu.op ), getcp2dr( INS_RD( mipscpu.op ) ) );
					break;
				case RS_CFC:
					mips_delayed_load( INS_RT( mipscpu.op ), getcp2cr( INS_RD( mipscpu.op ) ) );
					break;
				case RS_MTC:
					setcp2dr( INS_RD( mipscpu.op ), mipscpu.r[ INS_RT( mipscpu.op ) ] );
					mips_advance_pc();
					break;
				case RS_CTC:
					setcp2cr( INS_RD( mipscpu.op ), mipscpu.r[ INS_RT( mipscpu.op ) ] );
					mips_advance_pc();
					break;
				case RS_BC:
					switch( INS_RT( mipscpu.op ) )
					{
					case RT_BCF:
						/* todo: */
						logerror( "%08x: COP2 BCF not supported\n", mipscpu.pc );
						mips_stop();
						mips_advance_pc();
						break;
					case RT_BCT:
						/* todo: */
						logerror( "%08x: COP2 BCT not supported\n", mipscpu.pc );
						mips_stop();
						mips_advance_pc();
						break;
					default:
						/* todo: */
						logerror( "%08x: COP2 unknown command %08x\n", mipscpu.pc, mipscpu.op );
						mips_stop();
						mips_advance_pc();
						break;
					}
					break;
				default:
					switch( INS_CO( mipscpu.op ) )
					{
					case 1:
						docop2( INS_COFUN( mipscpu.op ) );
						mips_advance_pc();
						break;
					default:
						/* todo: */
						logerror( "%08x: COP2 unknown command %08x\n", mipscpu.pc, mipscpu.op );
						mips_stop();
						mips_advance_pc();
						break;
					}
					break;
				}
			}
			break;
		case OP_LB:
			if( ( mipscpu.cp0r[ CP0_SR ] & SR_ISC ) != 0 )
			{
				/* todo: */
				logerror( "%08x: LB SR_ISC not supported\n", mipscpu.pc );
				mips_stop();
				mips_advance_pc();
			}
			else if( ( mipscpu.cp0r[ CP0_SR ] & ( SR_RE | SR_KUC ) ) == ( SR_RE | SR_KUC ) )
			{
				UINT32 n_adr;
				n_adr = mipscpu.r[ INS_RS( mipscpu.op ) ] + MIPS_WORD_EXTEND( INS_IMMEDIATE( mipscpu.op ) );
				if( ( n_adr & ( ( mipscpu.cp0r[ CP0_SR ] & SR_KUC ) << 30 ) ) != 0 )
				{
					mips_exception( EXC_ADEL );
					mips_set_cp0r( CP0_BADVADDR, n_adr );
				}
				else
				{
					mips_delayed_load( INS_RT( mipscpu.op ), MIPS_BYTE_EXTEND( program_read_byte_32le( n_adr ^ 3 ) ) );
				}
			}
			else
			{
				UINT32 n_adr;
				n_adr = mipscpu.r[ INS_RS( mipscpu.op ) ] + MIPS_WORD_EXTEND( INS_IMMEDIATE( mipscpu.op ) );
				if( ( n_adr & ( ( mipscpu.cp0r[ CP0_SR ] & SR_KUC ) << 30 ) ) != 0 )
				{
					mips_exception( EXC_ADEL );
					mips_set_cp0r( CP0_BADVADDR, n_adr );
				}
				else
				{
					mips_delayed_load( INS_RT( mipscpu.op ), MIPS_BYTE_EXTEND( program_read_byte_32le( n_adr ) ) );
				}
			}
			break;
		case OP_LH:
			if( ( mipscpu.cp0r[ CP0_SR ] & SR_ISC ) != 0 )
			{
				/* todo: */
				logerror( "%08x: LH SR_ISC not supported\n", mipscpu.pc );
				mips_stop();
				mips_advance_pc();
			}
			else if( ( mipscpu.cp0r[ CP0_SR ] & ( SR_RE | SR_KUC ) ) == ( SR_RE | SR_KUC ) )
			{
				UINT32 n_adr;
				n_adr = mipscpu.r[ INS_RS( mipscpu.op ) ] + MIPS_WORD_EXTEND( INS_IMMEDIATE( mipscpu.op ) );
				if( ( n_adr & ( ( ( mipscpu.cp0r[ CP0_SR ] & SR_KUC ) << 30 ) | 1 ) ) != 0 )
				{
					mips_exception( EXC_ADEL );
					mips_set_cp0r( CP0_BADVADDR, n_adr );
				}
				else
				{
					mips_delayed_load( INS_RT( mipscpu.op ), MIPS_WORD_EXTEND( program_read_word_32le( n_adr ^ 2 ) ) );
				}
			}
			else
			{
				UINT32 n_adr;
				n_adr = mipscpu.r[ INS_RS( mipscpu.op ) ] + MIPS_WORD_EXTEND( INS_IMMEDIATE( mipscpu.op ) );
				if( ( n_adr & ( ( ( mipscpu.cp0r[ CP0_SR ] & SR_KUC ) << 30 ) | 1 ) ) != 0 )
				{
					mips_exception( EXC_ADEL );
					mips_set_cp0r( CP0_BADVADDR, n_adr );
				}
				else
				{
					mips_delayed_load( INS_RT( mipscpu.op ), MIPS_WORD_EXTEND( program_read_word_32le( n_adr ) ) );
				}
			}
			break;
		case OP_LWL:
			if( ( mipscpu.cp0r[ CP0_SR ] & SR_ISC ) != 0 )
			{
				/* todo: */
				logerror( "%08x: LWL SR_ISC not supported\n", mipscpu.pc );
				mips_stop();
				mips_advance_pc();
			}
			else if( ( mipscpu.cp0r[ CP0_SR ] & ( SR_RE | SR_KUC ) ) == ( SR_RE | SR_KUC ) )
			{
				UINT32 n_adr;
				n_adr = mipscpu.r[ INS_RS( mipscpu.op ) ] + MIPS_WORD_EXTEND( INS_IMMEDIATE( mipscpu.op ) );
				if( ( n_adr & ( ( mipscpu.cp0r[ CP0_SR ] & SR_KUC ) << 30 ) ) != 0 )
				{
					mips_exception( EXC_ADEL );
					mips_set_cp0r( CP0_BADVADDR, n_adr );
				}
				else
				{
					switch( n_adr & 3 )
					{
					case 0:
						n_res = ( mipscpu.r[ INS_RT( mipscpu.op ) ] & 0x00ffffff ) | ( (UINT32)program_read_byte_32le( n_adr + 3 ) << 24 );
						break;
					case 1:
						n_res = ( mipscpu.r[ INS_RT( mipscpu.op ) ] & 0x0000ffff ) | ( (UINT32)program_read_word_32le( n_adr + 1 ) << 16 );
						break;
					case 2:
						n_res = ( mipscpu.r[ INS_RT( mipscpu.op ) ] & 0x000000ff ) | ( (UINT32)program_read_byte_32le( n_adr - 1 ) << 8 ) | ( (UINT32)program_read_word_32le( n_adr ) << 16 );
						break;
					default:
						n_res = program_read_dword_32le( n_adr - 3 );
						break;
					}
					mips_delayed_load( INS_RT( mipscpu.op ), n_res );
				}
			}
			else
			{
				UINT32 n_adr;
				n_adr = mipscpu.r[ INS_RS( mipscpu.op ) ] + MIPS_WORD_EXTEND( INS_IMMEDIATE( mipscpu.op ) );
				if( ( n_adr & ( ( mipscpu.cp0r[ CP0_SR ] & SR_KUC ) << 30 ) ) != 0 )
				{
					mips_exception( EXC_ADEL );
					mips_set_cp0r( CP0_BADVADDR, n_adr );
				}
				else
				{
					switch( n_adr & 3 )
					{
					case 0:
						n_res = ( mipscpu.r[ INS_RT( mipscpu.op ) ] & 0x00ffffff ) | ( (UINT32)program_read_byte_32le( n_adr ) << 24 );
						break;
					case 1:
						n_res = ( mipscpu.r[ INS_RT( mipscpu.op ) ] & 0x0000ffff ) | ( (UINT32)program_read_word_32le( n_adr - 1 ) << 16 );
						break;
					case 2:
						n_res = ( mipscpu.r[ INS_RT( mipscpu.op ) ] & 0x000000ff ) | ( (UINT32)program_read_word_32le( n_adr - 2 ) << 8 ) | ( (UINT32)program_read_byte_32le( n_adr ) << 24 );
						break;
					default:
						n_res = program_read_dword_32le( n_adr - 3 );
						break;
					}
					mips_delayed_load( INS_RT( mipscpu.op ), n_res );
				}
			}
			break;
		case OP_LW:
			if( ( mipscpu.cp0r[ CP0_SR ] & SR_ISC ) != 0 )
			{
				/* todo: */
				logerror( "%08x: LW SR_ISC not supported\n", mipscpu.pc );
				mips_stop();
				mips_advance_pc();
			}
			else
			{
				UINT32 n_adr;
				n_adr = mipscpu.r[ INS_RS( mipscpu.op ) ] + MIPS_WORD_EXTEND( INS_IMMEDIATE( mipscpu.op ) );
				if( ( n_adr & ( ( ( mipscpu.cp0r[ CP0_SR ] & SR_KUC ) << 30 ) | 3 ) ) != 0 )
				{
					mips_exception( EXC_ADEL );
					mips_set_cp0r( CP0_BADVADDR, n_adr );
				}
				else
				{
					mips_delayed_load( INS_RT( mipscpu.op ), program_read_dword_32le( n_adr ) );
				}
			}
			break;
		case OP_LBU:
			if( ( mipscpu.cp0r[ CP0_SR ] & SR_ISC ) != 0 )
			{
				/* todo: */
				logerror( "%08x: LBU SR_ISC not supported\n", mipscpu.pc );
				mips_stop();
				mips_advance_pc();
			}
			else if( ( mipscpu.cp0r[ CP0_SR ] & ( SR_RE | SR_KUC ) ) == ( SR_RE | SR_KUC ) )
			{
				UINT32 n_adr;
				n_adr = mipscpu.r[ INS_RS( mipscpu.op ) ] + MIPS_WORD_EXTEND( INS_IMMEDIATE( mipscpu.op ) );
				if( ( n_adr & ( ( mipscpu.cp0r[ CP0_SR ] & SR_KUC ) << 30 ) ) != 0 )
				{
					mips_exception( EXC_ADEL );
					mips_set_cp0r( CP0_BADVADDR, n_adr );
				}
				else
				{
					mips_delayed_load( INS_RT( mipscpu.op ), program_read_byte_32le( n_adr ^ 3 ) );
				}
			}
			else
			{
				UINT32 n_adr;
				n_adr = mipscpu.r[ INS_RS( mipscpu.op ) ] + MIPS_WORD_EXTEND( INS_IMMEDIATE( mipscpu.op ) );
				if( ( n_adr & ( ( mipscpu.cp0r[ CP0_SR ] & SR_KUC ) << 30 ) ) != 0 )
				{
					mips_exception( EXC_ADEL );
					mips_set_cp0r( CP0_BADVADDR, n_adr );
				}
				else
				{
					mips_delayed_load( INS_RT( mipscpu.op ), program_read_byte_32le( n_adr ) );
				}
			}
			break;
		case OP_LHU:
			if( ( mipscpu.cp0r[ CP0_SR ] & SR_ISC ) != 0 )
			{
				/* todo: */
				logerror( "%08x: LHU SR_ISC not supported\n", mipscpu.pc );
				mips_stop();
				mips_advance_pc();
			}
			else if( ( mipscpu.cp0r[ CP0_SR ] & ( SR_RE | SR_KUC ) ) == ( SR_RE | SR_KUC ) )
			{
				UINT32 n_adr;
				n_adr = mipscpu.r[ INS_RS( mipscpu.op ) ] + MIPS_WORD_EXTEND( INS_IMMEDIATE( mipscpu.op ) );
				if( ( n_adr & ( ( ( mipscpu.cp0r[ CP0_SR ] & SR_KUC ) << 30 ) | 1 ) ) != 0 )
				{
					mips_exception( EXC_ADEL );
					mips_set_cp0r( CP0_BADVADDR, n_adr );
				}
				else
				{
					mips_delayed_load( INS_RT( mipscpu.op ), program_read_word_32le( n_adr ^ 2 ) );
				}
			}
			else
			{
				UINT32 n_adr;
				n_adr = mipscpu.r[ INS_RS( mipscpu.op ) ] + MIPS_WORD_EXTEND( INS_IMMEDIATE( mipscpu.op ) );
				if( ( n_adr & ( ( ( mipscpu.cp0r[ CP0_SR ] & SR_KUC ) << 30 ) | 1 ) ) != 0 )
				{
					mips_exception( EXC_ADEL );
					mips_set_cp0r( CP0_BADVADDR, n_adr );
				}
				else
				{
					mips_delayed_load( INS_RT( mipscpu.op ), program_read_word_32le( n_adr ) );
				}
			}
			break;
		case OP_LWR:
			if( ( mipscpu.cp0r[ CP0_SR ] & SR_ISC ) != 0 )
			{
				/* todo: */
				logerror( "%08x: LWR SR_ISC not supported\n", mipscpu.pc );
				mips_stop();
				mips_advance_pc();
			}
			else if( ( mipscpu.cp0r[ CP0_SR ] & ( SR_RE | SR_KUC ) ) == ( SR_RE | SR_KUC ) )
			{
				UINT32 n_adr;
				n_adr = mipscpu.r[ INS_RS( mipscpu.op ) ] + MIPS_WORD_EXTEND( INS_IMMEDIATE( mipscpu.op ) );
				if( ( n_adr & ( ( mipscpu.cp0r[ CP0_SR ] & SR_KUC ) << 30 ) ) != 0 )
				{
					mips_exception( EXC_ADEL );
					mips_set_cp0r( CP0_BADVADDR, n_adr );
				}
				else
				{
					switch( n_adr & 3 )
					{
					case 3:
						n_res = ( mipscpu.r[ INS_RT( mipscpu.op ) ] & 0xffffff00 ) | program_read_byte_32le( n_adr - 3 );
						break;
					case 2:
						n_res = ( mipscpu.r[ INS_RT( mipscpu.op ) ] & 0xffff0000 ) | program_read_word_32le( n_adr - 2 );
						break;
					case 1:
						n_res = ( mipscpu.r[ INS_RT( mipscpu.op ) ] & 0xff000000 ) | program_read_word_32le( n_adr - 1 ) | ( (UINT32)program_read_byte_32le( n_adr + 1 ) << 16 );
						break;
					default:
						n_res = program_read_dword_32le( n_adr );
						break;
					}
					mips_delayed_load( INS_RT( mipscpu.op ), n_res );
				}
			}
			else
			{
				UINT32 n_adr;
				n_adr = mipscpu.r[ INS_RS( mipscpu.op ) ] + MIPS_WORD_EXTEND( INS_IMMEDIATE( mipscpu.op ) );
				if( ( n_adr & ( ( mipscpu.cp0r[ CP0_SR ] & SR_KUC ) << 30 ) ) != 0 )
				{
					mips_exception( EXC_ADEL );
					mips_set_cp0r( CP0_BADVADDR, n_adr );
				}
				else
				{
					switch( n_adr & 3 )
					{
					case 3:
						n_res = ( mipscpu.r[ INS_RT( mipscpu.op ) ] & 0xffffff00 ) | program_read_byte_32le( n_adr );
						break;
					case 2:
						n_res = ( mipscpu.r[ INS_RT( mipscpu.op ) ] & 0xffff0000 ) | program_read_word_32le( n_adr );
						break;
					case 1:
						n_res = ( mipscpu.r[ INS_RT( mipscpu.op ) ] & 0xff000000 ) | program_read_byte_32le( n_adr ) | ( (UINT32)program_read_word_32le( n_adr + 1 ) << 8 );
						break;
					default:
						n_res = program_read_dword_32le( n_adr );
						break;
					}
					mips_delayed_load( INS_RT( mipscpu.op ), n_res );
				}
			}
			break;
		case OP_SB:
			if( ( mipscpu.cp0r[ CP0_SR ] & SR_ISC ) != 0 )
			{
				/* todo: */
				logerror( "%08x: SB SR_ISC not supported\n", mipscpu.pc );
				mips_stop();
				mips_advance_pc();
			}
			else if( ( mipscpu.cp0r[ CP0_SR ] & ( SR_RE | SR_KUC ) ) == ( SR_RE | SR_KUC ) )
			{
				UINT32 n_adr;
				n_adr = mipscpu.r[ INS_RS( mipscpu.op ) ] + MIPS_WORD_EXTEND( INS_IMMEDIATE( mipscpu.op ) );
				if( ( n_adr & ( ( mipscpu.cp0r[ CP0_SR ] & SR_KUC ) << 30 ) ) != 0 )
				{
					mips_exception( EXC_ADES );
					mips_set_cp0r( CP0_BADVADDR, n_adr );
				}
				else
				{
					program_write_byte_32le( n_adr ^ 3, mipscpu.r[ INS_RT( mipscpu.op ) ] );
					mips_advance_pc();
				}
			}
			else
			{
				UINT32 n_adr;
				n_adr = mipscpu.r[ INS_RS( mipscpu.op ) ] + MIPS_WORD_EXTEND( INS_IMMEDIATE( mipscpu.op ) );
				if( ( n_adr & ( ( mipscpu.cp0r[ CP0_SR ] & SR_KUC ) << 30 ) ) != 0 )
				{
					mips_exception( EXC_ADES );
					mips_set_cp0r( CP0_BADVADDR, n_adr );
				}
				else
				{
					program_write_byte_32le( n_adr, mipscpu.r[ INS_RT( mipscpu.op ) ] );
					mips_advance_pc();
				}
			}
			break;
		case OP_SH:
			if( ( mipscpu.cp0r[ CP0_SR ] & SR_ISC ) != 0 )
			{
				/* todo: */
				logerror( "%08x: SH SR_ISC not supported\n", mipscpu.pc );
				mips_stop();
				mips_advance_pc();
			}
			else if( ( mipscpu.cp0r[ CP0_SR ] & ( SR_RE | SR_KUC ) ) == ( SR_RE | SR_KUC ) )
			{
				UINT32 n_adr;
				n_adr = mipscpu.r[ INS_RS( mipscpu.op ) ] + MIPS_WORD_EXTEND( INS_IMMEDIATE( mipscpu.op ) );
				if( ( n_adr & ( ( ( mipscpu.cp0r[ CP0_SR ] & SR_KUC ) << 30 ) | 1 ) ) != 0 )
				{
					mips_exception( EXC_ADES );
					mips_set_cp0r( CP0_BADVADDR, n_adr );
				}
				else
				{
					program_write_word_32le( n_adr ^ 2, mipscpu.r[ INS_RT( mipscpu.op ) ] );
					mips_advance_pc();
				}
			}
			else
			{
				UINT32 n_adr;
				n_adr = mipscpu.r[ INS_RS( mipscpu.op ) ] + MIPS_WORD_EXTEND( INS_IMMEDIATE( mipscpu.op ) );
				if( ( n_adr & ( ( ( mipscpu.cp0r[ CP0_SR ] & SR_KUC ) << 30 ) | 1 ) ) != 0 )
				{
					mips_exception( EXC_ADES );
					mips_set_cp0r( CP0_BADVADDR, n_adr );
				}
				else
				{
					program_write_word_32le( n_adr, mipscpu.r[ INS_RT( mipscpu.op ) ] );
					mips_advance_pc();
				}
			}
			break;
		case OP_SWL:
			if( ( mipscpu.cp0r[ CP0_SR ] & SR_ISC ) != 0 )
			{
				/* todo: */
				logerror( "%08x: SWL SR_ISC not supported\n", mipscpu.pc );
				mips_stop();
				mips_advance_pc();
			}
			else if( ( mipscpu.cp0r[ CP0_SR ] & ( SR_RE | SR_KUC ) ) == ( SR_RE | SR_KUC ) )
			{
				UINT32 n_adr;
				n_adr = mipscpu.r[ INS_RS( mipscpu.op ) ] + MIPS_WORD_EXTEND( INS_IMMEDIATE( mipscpu.op ) );
				if( ( n_adr & ( ( mipscpu.cp0r[ CP0_SR ] & SR_KUC ) << 30 ) ) != 0 )
				{
					mips_exception( EXC_ADES );
					mips_set_cp0r( CP0_BADVADDR, n_adr );
				}
				else
				{
					switch( n_adr & 3 )
					{
					case 0:
						program_write_byte_32le( n_adr + 3, mipscpu.r[ INS_RT( mipscpu.op ) ] >> 24 );
						break;
					case 1:
						program_write_word_32le( n_adr + 1, mipscpu.r[ INS_RT( mipscpu.op ) ] >> 16 );
						break;
					case 2:
						program_write_byte_32le( n_adr - 1, mipscpu.r[ INS_RT( mipscpu.op ) ] >> 8 );
						program_write_word_32le( n_adr, mipscpu.r[ INS_RT( mipscpu.op ) ] >> 16 );
						break;
					case 3:
						program_write_dword_32le( n_adr - 3, mipscpu.r[ INS_RT( mipscpu.op ) ] );
						break;
					}
					mips_advance_pc();
				}
			}
			else
			{
				UINT32 n_adr;
				n_adr = mipscpu.r[ INS_RS( mipscpu.op ) ] + MIPS_WORD_EXTEND( INS_IMMEDIATE( mipscpu.op ) );
				if( ( n_adr & ( ( mipscpu.cp0r[ CP0_SR ] & SR_KUC ) << 30 ) ) != 0 )
				{
					mips_exception( EXC_ADES );
					mips_set_cp0r( CP0_BADVADDR, n_adr );
				}
				else
				{
					switch( n_adr & 3 )
					{
					case 0:
						program_write_byte_32le( n_adr, mipscpu.r[ INS_RT( mipscpu.op ) ] >> 24 );
						break;
					case 1:
						program_write_word_32le( n_adr - 1, mipscpu.r[ INS_RT( mipscpu.op ) ] >> 16 );
						break;
					case 2:
						program_write_word_32le( n_adr - 2, mipscpu.r[ INS_RT( mipscpu.op ) ] >> 8 );
						program_write_byte_32le( n_adr, mipscpu.r[ INS_RT( mipscpu.op ) ] >> 24 );
						break;
					case 3:
						program_write_dword_32le( n_adr - 3, mipscpu.r[ INS_RT( mipscpu.op ) ] );
						break;
					}
					mips_advance_pc();
				}
			}
			break;
		case OP_SW:
			if( ( mipscpu.cp0r[ CP0_SR ] & SR_ISC ) != 0 )
			{
				/* todo: */
/* used by bootstrap
                logerror( "%08x: SW SR_ISC not supported\n", mipscpu.pc );
                mips_stop();
*/
				mips_advance_pc();
			}
			else
			{
				UINT32 n_adr;
				n_adr = mipscpu.r[ INS_RS( mipscpu.op ) ] + MIPS_WORD_EXTEND( INS_IMMEDIATE( mipscpu.op ) );
				if( ( n_adr & ( ( ( mipscpu.cp0r[ CP0_SR ] & SR_KUC ) << 30 ) | 3 ) ) != 0 )
				{
					mips_exception( EXC_ADES );
					mips_set_cp0r( CP0_BADVADDR, n_adr );
				}
				else
				{
					program_write_dword_32le( n_adr, mipscpu.r[ INS_RT( mipscpu.op ) ] );
					mips_advance_pc();
				}
			}
			break;
		case OP_SWR:
			if( ( mipscpu.cp0r[ CP0_SR ] & SR_ISC ) != 0 )
			{
				/* todo: */
				logerror( "%08x: SWR SR_ISC not supported\n", mipscpu.pc );
				mips_stop();
				mips_advance_pc();
			}
			else if( ( mipscpu.cp0r[ CP0_SR ] & ( SR_RE | SR_KUC ) ) == ( SR_RE | SR_KUC ) )
			{
				UINT32 n_adr;
				n_adr = mipscpu.r[ INS_RS( mipscpu.op ) ] + MIPS_WORD_EXTEND( INS_IMMEDIATE( mipscpu.op ) );
				if( ( n_adr & ( ( mipscpu.cp0r[ CP0_SR ] & SR_KUC ) << 30 ) ) != 0 )
				{
					mips_exception( EXC_ADES );
					mips_set_cp0r( CP0_BADVADDR, n_adr );
				}
				else
				{
					switch( n_adr & 3 )
					{
					case 0:
						program_write_dword_32le( n_adr, mipscpu.r[ INS_RT( mipscpu.op ) ] );
						break;
					case 1:
						program_write_word_32le( n_adr - 1, mipscpu.r[ INS_RT( mipscpu.op ) ] );
						program_write_byte_32le( n_adr + 1, mipscpu.r[ INS_RT( mipscpu.op ) ] >> 16 );
						break;
					case 2:
						program_write_word_32le( n_adr - 2, mipscpu.r[ INS_RT( mipscpu.op ) ] );
						break;
					case 3:
						program_write_byte_32le( n_adr - 3, mipscpu.r[ INS_RT( mipscpu.op ) ] );
						break;
					}
					mips_advance_pc();
				}
			}
			else
			{
				UINT32 n_adr;
				n_adr = mipscpu.r[ INS_RS( mipscpu.op ) ] + MIPS_WORD_EXTEND( INS_IMMEDIATE( mipscpu.op ) );
				if( ( n_adr & ( ( mipscpu.cp0r[ CP0_SR ] & SR_KUC ) << 30 ) ) != 0 )
				{
					mips_exception( EXC_ADES );
					mips_set_cp0r( CP0_BADVADDR, n_adr );
				}
				else
				{
					switch( n_adr & 3 )
					{
					case 0:
						program_write_dword_32le( n_adr, mipscpu.r[ INS_RT( mipscpu.op ) ] );
						break;
					case 1:
						program_write_byte_32le( n_adr, mipscpu.r[ INS_RT( mipscpu.op ) ] );
						program_write_word_32le( n_adr + 1, mipscpu.r[ INS_RT( mipscpu.op ) ] >> 8 );
						break;
					case 2:
						program_write_word_32le( n_adr, mipscpu.r[ INS_RT( mipscpu.op ) ] );
						break;
					case 3:
						program_write_byte_32le( n_adr, mipscpu.r[ INS_RT( mipscpu.op ) ] );
						break;
					}
					mips_advance_pc();
				}
			}
			break;
		case OP_LWC1:
			/* todo: */
			logerror( "%08x: COP1 LWC not supported\n", mipscpu.pc );
			mips_stop();
			mips_advance_pc();
			break;
		case OP_LWC2:
			if( ( mipscpu.cp0r[ CP0_SR ] & SR_CU2 ) == 0 )
			{
				mips_exception( EXC_CPU );
				mips_set_cp0r( CP0_CAUSE, ( mipscpu.cp0r[ CP0_CAUSE ] & ~CAUSE_CE ) | CAUSE_CE2 );
			}
			else if( ( mipscpu.cp0r[ CP0_SR ] & SR_ISC ) != 0 )
			{
				/* todo: */
				logerror( "%08x: LWC2 SR_ISC not supported\n", mipscpu.pc );
				mips_stop();
				mips_advance_pc();
			}
			else
			{
				UINT32 n_adr;
				n_adr = mipscpu.r[ INS_RS( mipscpu.op ) ] + MIPS_WORD_EXTEND( INS_IMMEDIATE( mipscpu.op ) );
				if( ( n_adr & ( ( ( mipscpu.cp0r[ CP0_SR ] & SR_KUC ) << 30 ) | 3 ) ) != 0 )
				{
					mips_exception( EXC_ADEL );
					mips_set_cp0r( CP0_BADVADDR, n_adr );
				}
				else
				{
					/* todo: delay? */
					setcp2dr( INS_RT( mipscpu.op ), program_read_dword_32le( n_adr ) );
					mips_advance_pc();
				}
			}
			break;
		case OP_SWC1:
			/* todo: */
			logerror( "%08x: COP1 SWC not supported\n", mipscpu.pc );
			mips_stop();
			mips_advance_pc();
			break;
		case OP_SWC2:
			if( ( mipscpu.cp0r[ CP0_SR ] & SR_CU2 ) == 0 )
			{
				mips_exception( EXC_CPU );
				mips_set_cp0r( CP0_CAUSE, ( mipscpu.cp0r[ CP0_CAUSE ] & ~CAUSE_CE ) | CAUSE_CE2 );
			}
			else if( ( mipscpu.cp0r[ CP0_SR ] & SR_ISC ) != 0 )
			{
				/* todo: */
				logerror( "%08x: SWC2 SR_ISC not supported\n", mipscpu.pc );
				mips_stop();
				mips_advance_pc();
			}
			else
			{
				UINT32 n_adr;
				n_adr = mipscpu.r[ INS_RS( mipscpu.op ) ] + MIPS_WORD_EXTEND( INS_IMMEDIATE( mipscpu.op ) );
				if( ( n_adr & ( ( ( mipscpu.cp0r[ CP0_SR ] & SR_KUC ) << 30 ) | 3 ) ) != 0 )
				{
					mips_exception( EXC_ADES );
					mips_set_cp0r( CP0_BADVADDR, n_adr );
				}
				else
				{
					program_write_dword_32le( n_adr, getcp2dr( INS_RT( mipscpu.op ) ) );
					mips_advance_pc();
				}
			}
			break;
		default:
			logerror( "%08x: unknown opcode %08x\n", mipscpu.pc, mipscpu.op );
			mips_stop();
			mips_exception( EXC_RI );
			break;
		}
		mips_ICount--;
	} while( mips_ICount > 0 );

	return cycles - mips_ICount;
}

static void mips_get_context( void *dst )
{
	if( dst )
	{
		*(mips_cpu_context *)dst = mipscpu;
	}
}

static void mips_set_context( void *src )
{
	if( src )
	{
		mipscpu = *(mips_cpu_context *)src;
		change_pc( mipscpu.pc );
	}
}

static void set_irq_line( int irqline, int state )
{
	UINT32 ip;

	switch( irqline )
	{
	case MIPS_IRQ0:
		ip = CAUSE_IP2;
		break;
	case MIPS_IRQ1:
		ip = CAUSE_IP3;
		break;
	case MIPS_IRQ2:
		ip = CAUSE_IP4;
		break;
	case MIPS_IRQ3:
		ip = CAUSE_IP5;
		break;
	case MIPS_IRQ4:
		ip = CAUSE_IP6;
		break;
	case MIPS_IRQ5:
		ip = CAUSE_IP7;
		break;
	default:
		return;
	}

	switch( state )
	{
	case CLEAR_LINE:
		mips_set_cp0r( CP0_CAUSE, mipscpu.cp0r[ CP0_CAUSE ] & ~ip );
		break;
	case ASSERT_LINE:
		mips_set_cp0r( CP0_CAUSE, mipscpu.cp0r[ CP0_CAUSE ] |= ip );
		if( mipscpu.irq_callback )
		{
			/* HOLD_LINE interrupts are not supported by the architecture.
            By acknowledging the interupt here they are treated like PULSE_LINE
            interrupts, so if the interrupt isn't enabled it will be ignored.
            There is also a problem with PULSE_LINE interrupts as the interrupt
            pending bits aren't latched the emulated code won't know what caused
            the interrupt. */
			(*mipscpu.irq_callback)( irqline );
		}
		break;
	}
}

/****************************************************************************
 * Return a formatted string for a register
 ****************************************************************************/

#ifdef MAME_DEBUG
static offs_t mips_dasm(char *buffer, offs_t pc, const UINT8 *oprom, const UINT8 *opram)
{
	return DasmMIPS( buffer, pc, opram );
}
#endif /* MAME_DEBUG */

/* preliminary gte code */

#define VXY0 ( mipscpu.cp2dr[ 0 ].d )
#define VX0  ( mipscpu.cp2dr[ 0 ].w.l )
#define VY0  ( mipscpu.cp2dr[ 0 ].w.h )
#define VZ0  ( mipscpu.cp2dr[ 1 ].w.l )
#define VXY1 ( mipscpu.cp2dr[ 2 ].d )
#define VX1  ( mipscpu.cp2dr[ 2 ].w.l )
#define VY1  ( mipscpu.cp2dr[ 2 ].w.h )
#define VZ1  ( mipscpu.cp2dr[ 3 ].w.l )
#define VXY2 ( mipscpu.cp2dr[ 4 ].d )
#define VX2  ( mipscpu.cp2dr[ 4 ].w.l )
#define VY2  ( mipscpu.cp2dr[ 4 ].w.h )
#define VZ2  ( mipscpu.cp2dr[ 5 ].w.l )
#define RGB  ( mipscpu.cp2dr[ 6 ].d )
#define R    ( mipscpu.cp2dr[ 6 ].b.l )
#define G    ( mipscpu.cp2dr[ 6 ].b.h )
#define B    ( mipscpu.cp2dr[ 6 ].b.h2 )
#define CODE ( mipscpu.cp2dr[ 6 ].b.h3 )
#define OTZ  ( mipscpu.cp2dr[ 7 ].w.l )
#define IR0  ( mipscpu.cp2dr[ 8 ].d )
#define IR1  ( mipscpu.cp2dr[ 9 ].d )
#define IR2  ( mipscpu.cp2dr[ 10 ].d )
#define IR3  ( mipscpu.cp2dr[ 11 ].d )
#define SXY0 ( mipscpu.cp2dr[ 12 ].d )
#define SX0  ( mipscpu.cp2dr[ 12 ].w.l )
#define SY0  ( mipscpu.cp2dr[ 12 ].w.h )
#define SXY1 ( mipscpu.cp2dr[ 13 ].d )
#define SX1  ( mipscpu.cp2dr[ 13 ].w.l )
#define SY1  ( mipscpu.cp2dr[ 13 ].w.h )
#define SXY2 ( mipscpu.cp2dr[ 14 ].d )
#define SX2  ( mipscpu.cp2dr[ 14 ].w.l )
#define SY2  ( mipscpu.cp2dr[ 14 ].w.h )
#define SXYP ( mipscpu.cp2dr[ 15 ].d )
#define SXP  ( mipscpu.cp2dr[ 15 ].w.l )
#define SYP  ( mipscpu.cp2dr[ 15 ].w.h )
#define SZ0  ( mipscpu.cp2dr[ 16 ].w.l )
#define SZ1  ( mipscpu.cp2dr[ 17 ].w.l )
#define SZ2  ( mipscpu.cp2dr[ 18 ].w.l )
#define SZ3  ( mipscpu.cp2dr[ 19 ].w.l )
#define RGB0 ( mipscpu.cp2dr[ 20 ].d )
#define R0   ( mipscpu.cp2dr[ 20 ].b.l )
#define G0   ( mipscpu.cp2dr[ 20 ].b.h )
#define B0   ( mipscpu.cp2dr[ 20 ].b.h2 )
#define CD0  ( mipscpu.cp2dr[ 20 ].b.h3 )
#define RGB1 ( mipscpu.cp2dr[ 21 ].d )
#define R1   ( mipscpu.cp2dr[ 21 ].b.l )
#define G1   ( mipscpu.cp2dr[ 21 ].b.h )
#define B1   ( mipscpu.cp2dr[ 21 ].b.h2 )
#define CD1  ( mipscpu.cp2dr[ 21 ].b.h3 )
#define RGB2 ( mipscpu.cp2dr[ 22 ].d )
#define R2   ( mipscpu.cp2dr[ 22 ].b.l )
#define G2   ( mipscpu.cp2dr[ 22 ].b.h )
#define B2   ( mipscpu.cp2dr[ 22 ].b.h2 )
#define CD2  ( mipscpu.cp2dr[ 22 ].b.h3 )
#define RES1 ( mipscpu.cp2dr[ 23 ].d )
#define MAC0 ( mipscpu.cp2dr[ 24 ].d )
#define MAC1 ( mipscpu.cp2dr[ 25 ].d )
#define MAC2 ( mipscpu.cp2dr[ 26 ].d )
#define MAC3 ( mipscpu.cp2dr[ 27 ].d )
#define IRGB ( mipscpu.cp2dr[ 28 ].d )
#define ORGB ( mipscpu.cp2dr[ 29 ].d )
#define LZCS ( mipscpu.cp2dr[ 30 ].d )
#define LZCR ( mipscpu.cp2dr[ 31 ].d )

#define D1  ( mipscpu.cp2cr[ 0 ].d )
#define R11 ( mipscpu.cp2cr[ 0 ].w.l )
#define R12 ( mipscpu.cp2cr[ 0 ].w.h )
#define R13 ( mipscpu.cp2cr[ 1 ].w.l )
#define R21 ( mipscpu.cp2cr[ 1 ].w.h )
#define D2  ( mipscpu.cp2cr[ 2 ].d )
#define R22 ( mipscpu.cp2cr[ 2 ].w.l )
#define R23 ( mipscpu.cp2cr[ 2 ].w.h )
#define R31 ( mipscpu.cp2cr[ 3 ].w.l )
#define R32 ( mipscpu.cp2cr[ 3 ].w.h )
#define D3  ( mipscpu.cp2cr[ 4 ].d )
#define R33 ( mipscpu.cp2cr[ 4 ].w.l )
#define TRX ( mipscpu.cp2cr[ 5 ].d )
#define TRY ( mipscpu.cp2cr[ 6 ].d )
#define TRZ ( mipscpu.cp2cr[ 7 ].d )
#define L11 ( mipscpu.cp2cr[ 8 ].w.l )
#define L12 ( mipscpu.cp2cr[ 8 ].w.h )
#define L13 ( mipscpu.cp2cr[ 9 ].w.l )
#define L21 ( mipscpu.cp2cr[ 9 ].w.h )
#define L22 ( mipscpu.cp2cr[ 10 ].w.l )
#define L23 ( mipscpu.cp2cr[ 10 ].w.h )
#define L31 ( mipscpu.cp2cr[ 11 ].w.l )
#define L32 ( mipscpu.cp2cr[ 11 ].w.h )
#define L33 ( mipscpu.cp2cr[ 12 ].w.l )
#define RBK ( mipscpu.cp2cr[ 13 ].d )
#define GBK ( mipscpu.cp2cr[ 14 ].d )
#define BBK ( mipscpu.cp2cr[ 15 ].d )
#define LR1 ( mipscpu.cp2cr[ 16 ].w.l )
#define LR2 ( mipscpu.cp2cr[ 16 ].w.h )
#define LR3 ( mipscpu.cp2cr[ 17 ].w.l )
#define LG1 ( mipscpu.cp2cr[ 17 ].w.h )
#define LG2 ( mipscpu.cp2cr[ 18 ].w.l )
#define LG3 ( mipscpu.cp2cr[ 18 ].w.h )
#define LB1 ( mipscpu.cp2cr[ 19 ].w.l )
#define LB2 ( mipscpu.cp2cr[ 19 ].w.h )
#define LB3 ( mipscpu.cp2cr[ 20 ].w.l )
#define RFC ( mipscpu.cp2cr[ 21 ].d )
#define GFC ( mipscpu.cp2cr[ 22 ].d )
#define BFC ( mipscpu.cp2cr[ 23 ].d )
#define OFX ( mipscpu.cp2cr[ 24 ].d )
#define OFY ( mipscpu.cp2cr[ 25 ].d )
#define H   ( mipscpu.cp2cr[ 26 ].w.l )
#define DQA ( mipscpu.cp2cr[ 27 ].w.l )
#define DQB ( mipscpu.cp2cr[ 28 ].d )
#define ZSF3 ( mipscpu.cp2cr[ 29 ].w.l )
#define ZSF4 ( mipscpu.cp2cr[ 30 ].w.l )
#define FLAG ( mipscpu.cp2cr[ 31 ].d )

static UINT32 getcp2dr( int n_reg )
{
	if( n_reg == 1 || n_reg == 3 || n_reg == 5 || n_reg == 8 || n_reg == 9 || n_reg == 10 || n_reg == 11 )
	{
		mipscpu.cp2dr[ n_reg ].d = (INT32)(INT16)mipscpu.cp2dr[ n_reg ].d;
	}
	else if( n_reg == 17 || n_reg == 18 || n_reg == 19 )
	{
		mipscpu.cp2dr[ n_reg ].d = (UINT32)(UINT16)mipscpu.cp2dr[ n_reg ].d;
	}
	else if( n_reg == 29 )
	{
		ORGB = ( ( IR1 >> 7 ) & 0x1f ) | ( ( IR2 >> 2 ) & 0x3e0 ) | ( ( IR3 << 3 ) & 0x7c00 );
	}
	GTELOG( "get CP2DR%u=%08x", n_reg, mipscpu.cp2dr[ n_reg ].d );
	return mipscpu.cp2dr[ n_reg ].d;
}

static void setcp2dr( int n_reg, UINT32 n_value )
{
	GTELOG( "set CP2DR%u=%08x", n_reg, n_value );
	mipscpu.cp2dr[ n_reg ].d = n_value;

	if( n_reg == 15 )
	{
		SXY0 = SXY1;
		SXY1 = SXY2;
		SXY2 = SXYP;
	}
	else if( n_reg == 28 )
	{
		IR1 = ( IRGB & 0x1f ) << 4;
		IR2 = ( IRGB & 0x3e0 ) >> 1;
		IR3 = ( IRGB & 0x7c00 ) >> 6;
	}
	else if( n_reg == 30 )
	{
		UINT32 n_lzcs = LZCS;
		UINT32 n_lzcr = 0;

		if( ( n_lzcs & 0x80000000 ) == 0 )
		{
			n_lzcs = ~n_lzcs;
		}
		while( ( n_lzcs & 0x80000000 ) != 0 )
		{
			n_lzcr++;
			n_lzcs <<= 1;
		}
		LZCR = n_lzcr;
	}
}

static UINT32 getcp2cr( int n_reg )
{
	GTELOG( "get CP2CR%u=%08x", n_reg, mipscpu.cp2cr[ n_reg ].d );
	return mipscpu.cp2cr[ n_reg ].d;
}

static void setcp2cr( int n_reg, UINT32 n_value )
{
	GTELOG( "set CP2CR%u=%08x", n_reg, n_value );
	mipscpu.cp2cr[ n_reg ].d = n_value;
}

INLINE INT32 LIM( INT32 n_value, INT32 n_max, INT32 n_min, UINT32 n_flag )
{
	if( n_value > n_max )
	{
		FLAG |= n_flag;
		return n_max;
	}
	else if( n_value < n_min )
	{
		FLAG |= n_flag;
		return n_min;
	}
	return n_value;
}

INLINE INT64 BOUNDS( INT64 n_value, INT64 n_max, int n_maxflag, INT64 n_min, int n_minflag )
{
	if( n_value > n_max )
	{
		FLAG |= 1 << n_maxflag;
	}
	else if( n_value < n_min )
	{
		FLAG |= 1 << n_minflag;
	}
	return n_value;
}

#define A1( a ) BOUNDS( ( a ), 0x7fffffff, 30, -(INT64)0x80000000, 27 )
#define A2( a ) BOUNDS( ( a ), 0x7fffffff, 29, -(INT64)0x80000000, 26 )
#define A3( a ) BOUNDS( ( a ), 0x7fffffff, 28, -(INT64)0x80000000, 25 )
#define Lm_B1( a, l ) LIM( ( a ), 0x7fff, -0x8000 * !l, ( 1 << 31 ) | ( 1 << 24 ) )
#define Lm_B2( a, l ) LIM( ( a ), 0x7fff, -0x8000 * !l, ( 1 << 31 ) | ( 1 << 23 ) )
#define Lm_B3( a, l ) LIM( ( a ), 0x7fff, -0x8000 * !l, ( 1 << 31 ) | ( 1 << 22 ) )
#define Lm_C1( a ) LIM( ( a ), 0x00ff, 0x0000, ( 1 << 21 ) )
#define Lm_C2( a ) LIM( ( a ), 0x00ff, 0x0000, ( 1 << 20 ) )
#define Lm_C3( a ) LIM( ( a ), 0x00ff, 0x0000, ( 1 << 19 ) )
#define Lm_D( a ) LIM( ( a ), 0xffff, 0x0000, ( 1 << 31 ) | ( 1 << 18 ) )

INLINE UINT32 Lm_E( UINT32 n_z )
{
	if( n_z <= H / 2 )
	{
		n_z = H / 2;
		FLAG |= ( 1 << 31 ) | ( 1 << 17 );
	}
	if( n_z == 0 )
	{
		n_z = 1;
	}
	return n_z;
}

#define F( a ) BOUNDS( ( a ), 0x7fffffff, ( 1 << 31 ) | ( 1 << 16 ), -(INT64)0x80000000, ( 1 << 31 ) | ( 1 << 15 ) )
#define Lm_G1( a ) LIM( ( a ), 0x3ff, -0x400, ( 1 << 31 ) | ( 1 << 14 ) )
#define Lm_G2( a ) LIM( ( a ), 0x3ff, -0x400, ( 1 << 31 ) | ( 1 << 13 ) )
#define Lm_H( a ) LIM( ( a ), 0xfff, 0x000, ( 1 << 12 ) )

static void docop2( int gteop )
{
	int n_sf;
	int n_v;
	int n_lm;
	int n_pass;
	UINT16 n_v1;
	UINT16 n_v2;
	UINT16 n_v3;
	const UINT16 **p_n_mx;
	const UINT32 **p_n_cv;
	static const UINT16 n_zm = 0;
	static const UINT32 n_zc = 0;
	static const UINT16 *p_n_vx[] = { &VX0, &VX1, &VX2 };
	static const UINT16 *p_n_vy[] = { &VY0, &VY1, &VY2 };
	static const UINT16 *p_n_vz[] = { &VZ0, &VZ1, &VZ2 };
	static const UINT16 *p_n_rm[] = { &R11, &R12, &R13, &R21, &R22, &R23, &R31, &R32, &R33 };
	static const UINT16 *p_n_lm[] = { &L11, &L12, &L13, &L21, &L22, &L23, &L31, &L32, &L33 };
	static const UINT16 *p_n_cm[] = { &LR1, &LR2, &LR3, &LG1, &LG2, &LG3, &LB1, &LB2, &LB3 };
	static const UINT16 *p_n_zm[] = { &n_zm, &n_zm, &n_zm, &n_zm, &n_zm, &n_zm, &n_zm, &n_zm, &n_zm };
	static const UINT16 **p_p_n_mx[] = { p_n_rm, p_n_lm, p_n_cm, p_n_zm };
	static const UINT32 *p_n_tr[] = { &TRX, &TRY, &TRZ };
	static const UINT32 *p_n_bk[] = { &RBK, &GBK, &BBK };
	static const UINT32 *p_n_fc[] = { &RFC, &GFC, &BFC };
	static const UINT32 *p_n_zc[] = { &n_zc, &n_zc, &n_zc };
	static const UINT32 **p_p_n_cv[] = { p_n_tr, p_n_bk, p_n_fc, p_n_zc };

	switch( GTE_FUNCT( gteop ) )
	{
	case 0x01:
		if( gteop == 0x0180001 )
		{
			GTELOG( "RTPS" );
			FLAG = 0;

			MAC1 = A1( ( ( (INT64)(INT32)TRX << 12 ) + ( (INT16)R11 * (INT16)VX0 ) + ( (INT16)R12 * (INT16)VY0 ) + ( (INT16)R13 * (INT16)VZ0 ) ) >> 12 );
			MAC2 = A2( ( ( (INT64)(INT32)TRY << 12 ) + ( (INT16)R21 * (INT16)VX0 ) + ( (INT16)R22 * (INT16)VY0 ) + ( (INT16)R23 * (INT16)VZ0 ) ) >> 12 );
			MAC3 = A3( ( ( (INT64)(INT32)TRZ << 12 ) + ( (INT16)R31 * (INT16)VX0 ) + ( (INT16)R32 * (INT16)VY0 ) + ( (INT16)R33 * (INT16)VZ0 ) ) >> 12 );
			IR1 = Lm_B1( (INT32)MAC1, 0 );
			IR2 = Lm_B2( (INT32)MAC2, 0 );
			IR3 = Lm_B3( (INT32)MAC3, 0 );
			SZ0 = SZ1;
			SZ1 = SZ2;
			SZ2 = SZ3;
			SZ3 = Lm_D( (INT32)MAC3 );
			SXY0 = SXY1;
			SXY1 = SXY2;
			SX2 = Lm_G1( F( (INT64)(INT32)OFX + ( (INT64)(INT16)IR1 * ( ( (UINT32)H << 16 ) / Lm_E( SZ3 ) ) ) ) >> 16 );
			SY2 = Lm_G2( F( (INT64)(INT32)OFY + ( (INT64)(INT16)IR2 * ( ( (UINT32)H << 16 ) / Lm_E( SZ3 ) ) ) ) >> 16 );
			MAC0 = F( (INT64)(INT32)DQB + ( (INT64)(INT16)DQA * ( ( (UINT32)H << 16 ) / Lm_E( SZ3 ) ) ) );
			IR0 = Lm_H( (INT32)MAC0 >> 12 );
			return;
		}
		break;
	case 0x06:
		if( gteop == 0x0400006 ||
			gteop == 0x1400006 ||
			gteop == 0x0155cc6 )
		{
			GTELOG( "NCLIP" );
			FLAG = 0;

			MAC0 = F( ( (INT64)(INT16)SX0 * (INT16)SY1 ) + ( (INT16)SX1 * (INT16)SY2 ) + ( (INT16)SX2 * (INT16)SY0 ) - ( (INT16)SX0 * (INT16)SY2 ) - ( (INT16)SX1 * (INT16)SY0 ) - ( (INT16)SX2 * (INT16)SY1 ) );
			return;
		}
		break;
	case 0x0c:
		if( GTE_OP( gteop ) == 0x17 )
		{
			GTELOG( "OP" );
			n_sf = 12 * GTE_SF( gteop );
			FLAG = 0;

			MAC1 = A1( ( ( (INT64)(INT32)D2 * (INT16)IR3 ) - ( (INT64)(INT32)D3 * (INT16)IR2 ) ) >> n_sf );
			MAC2 = A2( ( ( (INT64)(INT32)D3 * (INT16)IR1 ) - ( (INT64)(INT32)D1 * (INT16)IR3 ) ) >> n_sf );
			MAC3 = A3( ( ( (INT64)(INT32)D1 * (INT16)IR2 ) - ( (INT64)(INT32)D2 * (INT16)IR1 ) ) >> n_sf );
			IR1 = Lm_B1( (INT32)MAC1, 0 );
			IR2 = Lm_B2( (INT32)MAC2, 0 );
			IR3 = Lm_B3( (INT32)MAC3, 0 );
			return;
		}
		break;
	case 0x10:
		if( gteop == 0x0780010 )
		{
			GTELOG( "DPCS" );
			FLAG = 0;

			MAC1 = A1( ( ( (INT64)R << 16 ) + ( (INT64)(INT16)IR0 * ( Lm_B1( (INT32)RFC - ( R << 4 ), 0 ) ) ) ) >> 12 );
			MAC2 = A2( ( ( (INT64)G << 16 ) + ( (INT64)(INT16)IR0 * ( Lm_B1( (INT32)GFC - ( G << 4 ), 0 ) ) ) ) >> 12 );
			MAC3 = A3( ( ( (INT64)B << 16 ) + ( (INT64)(INT16)IR0 * ( Lm_B1( (INT32)BFC - ( B << 4 ), 0 ) ) ) ) >> 12 );
			IR1 = Lm_B1( (INT32)MAC1, 0 );
			IR2 = Lm_B2( (INT32)MAC2, 0 );
			IR3 = Lm_B3( (INT32)MAC3, 0 );
			CD0 = CD1;
			CD1 = CD2;
			CD2 = CODE;
			R0 = R1;
			R1 = R2;
			R2 = Lm_C1( (INT32)MAC1 >> 4 );
			G0 = G1;
			G1 = G2;
			G2 = Lm_C2( (INT32)MAC2 >> 4 );
			B0 = B1;
			B1 = B2;
			B2 = Lm_C3( (INT32)MAC3 >> 4 );
			return;
		}
		break;
	case 0x11:
		if( gteop == 0x0980011 )
		{
			GTELOG( "INTPL" );
			FLAG = 0;

			MAC1 = A1( ( ( (INT64)(INT16)IR1 << 12 ) + ( (INT64)(INT16)IR0 * ( Lm_B1( (INT32)RFC - (INT16)IR1, 0 ) ) ) ) >> 12 );
			MAC2 = A2( ( ( (INT64)(INT16)IR2 << 12 ) + ( (INT64)(INT16)IR0 * ( Lm_B1( (INT32)GFC - (INT16)IR2, 0 ) ) ) ) >> 12 );
			MAC3 = A3( ( ( (INT64)(INT16)IR3 << 12 ) + ( (INT64)(INT16)IR0 * ( Lm_B1( (INT32)BFC - (INT16)IR3, 0 ) ) ) ) >> 12 );
			IR1 = Lm_B1( (INT32)MAC1, 0 );
			IR2 = Lm_B2( (INT32)MAC2, 0 );
			IR3 = Lm_B3( (INT32)MAC3, 0 );
			CD0 = CD1;
			CD1 = CD2;
			CD2 = CODE;
			R0 = R1;
			R1 = R2;
			R2 = Lm_C1( (INT32)MAC1 );
			G0 = G1;
			G1 = G2;
			G2 = Lm_C2( (INT32)MAC2 );
			B0 = B1;
			B1 = B2;
			B2 = Lm_C3( (INT32)MAC3 );
			return;
		}
		break;
	case 0x12:
		if( GTE_OP( gteop ) == 0x04 )
		{
			GTELOG( "MVMVA" );
			n_sf = 12 * GTE_SF( gteop );
			p_n_mx = p_p_n_mx[ GTE_MX( gteop ) ];
			n_v = GTE_V( gteop );
			if( n_v < 3 )
			{
				n_v1 = *p_n_vx[ n_v ];
				n_v2 = *p_n_vy[ n_v ];
				n_v3 = *p_n_vz[ n_v ];
			}
			else
			{
				n_v1 = IR1;
				n_v2 = IR2;
				n_v3 = IR3;
			}
			p_n_cv = p_p_n_cv[ GTE_CV( gteop ) ];
			n_lm = GTE_LM( gteop );
			FLAG = 0;

			MAC1 = A1( ( ( (INT64)(INT32)*p_n_cv[ 0 ] << 12 ) + ( (INT16)*p_n_mx[ 0 ] * (INT16)n_v1 ) + ( (INT16)*p_n_mx[ 1 ] * (INT16)n_v2 ) + ( (INT16)*p_n_mx[ 2 ] * (INT16)n_v3 ) ) >> n_sf );
			MAC2 = A2( ( ( (INT64)(INT32)*p_n_cv[ 1 ] << 12 ) + ( (INT16)*p_n_mx[ 3 ] * (INT16)n_v1 ) + ( (INT16)*p_n_mx[ 4 ] * (INT16)n_v2 ) + ( (INT16)*p_n_mx[ 5 ] * (INT16)n_v3 ) ) >> n_sf );
			MAC3 = A3( ( ( (INT64)(INT32)*p_n_cv[ 2 ] << 12 ) + ( (INT16)*p_n_mx[ 6 ] * (INT16)n_v1 ) + ( (INT16)*p_n_mx[ 7 ] * (INT16)n_v2 ) + ( (INT16)*p_n_mx[ 8 ] * (INT16)n_v3 ) ) >> n_sf );

			IR1 = Lm_B1( (INT32)MAC1, n_lm );
			IR2 = Lm_B2( (INT32)MAC2, n_lm );
			IR3 = Lm_B3( (INT32)MAC3, n_lm );
			return;
		}
		break;
	case 0x13:
		if( gteop == 0x0e80413 )
		{
			GTELOG( "NCDS" );
			FLAG = 0;

			MAC1 = A1( ( ( (INT64)(INT16)L11 * (INT16)VX0 ) + ( (INT16)L12 * (INT16)VY0 ) + ( (INT16)L13 * (INT16)VZ0 ) ) >> 12 );
			MAC2 = A2( ( ( (INT64)(INT16)L21 * (INT16)VX0 ) + ( (INT16)L22 * (INT16)VY0 ) + ( (INT16)L23 * (INT16)VZ0 ) ) >> 12 );
			MAC3 = A3( ( ( (INT64)(INT16)L31 * (INT16)VX0 ) + ( (INT16)L32 * (INT16)VY0 ) + ( (INT16)L33 * (INT16)VZ0 ) ) >> 12 );
			IR1 = Lm_B1( (INT32)MAC1, 1 );
			IR2 = Lm_B2( (INT32)MAC2, 1 );
			IR3 = Lm_B3( (INT32)MAC3, 1 );
			MAC1 = A1( ( ( (INT64)RBK << 12 ) + ( (INT16)LR1 * (INT16)IR1 ) + ( (INT16)LR2 * (INT16)IR2 ) + ( (INT16)LR3 * (INT16)IR3 ) ) >> 12 );
			MAC2 = A2( ( ( (INT64)GBK << 12 ) + ( (INT16)LG1 * (INT16)IR1 ) + ( (INT16)LG2 * (INT16)IR2 ) + ( (INT16)LG3 * (INT16)IR3 ) ) >> 12 );
			MAC3 = A3( ( ( (INT64)BBK << 12 ) + ( (INT16)LB1 * (INT16)IR1 ) + ( (INT16)LB2 * (INT16)IR2 ) + ( (INT16)LB3 * (INT16)IR3 ) ) >> 12 );
			IR1 = Lm_B1( (INT32)MAC1, 1 );
			IR2 = Lm_B2( (INT32)MAC2, 1 );
			IR3 = Lm_B3( (INT32)MAC3, 1 );
			MAC1 = A1( ( ( ( (INT64)R << 4 ) * (INT16)IR1 ) + ( (INT16)IR0 * Lm_B1( (INT32)RFC - ( ( R * (INT16)IR1 ) >> 8 ), 0 ) ) ) >> 12 );
			MAC2 = A2( ( ( ( (INT64)G << 4 ) * (INT16)IR2 ) + ( (INT16)IR0 * Lm_B2( (INT32)GFC - ( ( G * (INT16)IR2 ) >> 8 ), 0 ) ) ) >> 12 );
			MAC3 = A3( ( ( ( (INT64)B << 4 ) * (INT16)IR3 ) + ( (INT16)IR0 * Lm_B3( (INT32)BFC - ( ( B * (INT16)IR3 ) >> 8 ), 0 ) ) ) >> 12 );
			IR1 = Lm_B1( (INT32)MAC1, 1 );
			IR2 = Lm_B2( (INT32)MAC2, 1 );
			IR3 = Lm_B3( (INT32)MAC3, 1 );
			CD0 = CD1;
			CD1 = CD2;
			CD2 = CODE;
			R0 = R1;
			R1 = R2;
			R2 = Lm_C1( (INT32)MAC1 >> 4 );
			G0 = G1;
			G1 = G2;
			G2 = Lm_C2( (INT32)MAC2 >> 4 );
			B0 = B1;
			B1 = B2;
			B2 = Lm_C3( (INT32)MAC3 >> 4 );
			return;
		}
		break;
	case 0x14:
		if( gteop == 0x1280414 )
		{
			GTELOG( "CDP" );
			FLAG = 0;

			MAC1 = A1( ( ( (INT64)RBK << 12 ) + ( (INT16)LR1 * (INT16)IR1 ) + ( (INT16)LR2 * (INT16)IR2 ) + ( (INT16)LR3 * (INT16)IR3 ) ) >> 12 );
			MAC2 = A2( ( ( (INT64)GBK << 12 ) + ( (INT16)LG1 * (INT16)IR1 ) + ( (INT16)LG2 * (INT16)IR2 ) + ( (INT16)LG3 * (INT16)IR3 ) ) >> 12 );
			MAC3 = A3( ( ( (INT64)BBK << 12 ) + ( (INT16)LB1 * (INT16)IR1 ) + ( (INT16)LB2 * (INT16)IR2 ) + ( (INT16)LB3 * (INT16)IR3 ) ) >> 12 );
			IR1 = Lm_B1( MAC1, 1 );
			IR2 = Lm_B2( MAC2, 1 );
			IR3 = Lm_B3( MAC3, 1 );
			MAC1 = A1( ( ( ( (INT64)R << 4 ) * (INT16)IR1 ) + ( (INT16)IR0 * Lm_B1( (INT32)RFC - ( ( R * (INT16)IR1 ) >> 8 ), 0 ) ) ) >> 12 );
			MAC2 = A2( ( ( ( (INT64)G << 4 ) * (INT16)IR2 ) + ( (INT16)IR0 * Lm_B2( (INT32)GFC - ( ( G * (INT16)IR2 ) >> 8 ), 0 ) ) ) >> 12 );
			MAC3 = A3( ( ( ( (INT64)B << 4 ) * (INT16)IR3 ) + ( (INT16)IR0 * Lm_B3( (INT32)BFC - ( ( B * (INT16)IR3 ) >> 8 ), 0 ) ) ) >> 12 );
			IR1 = Lm_B1( MAC1, 1 );
			IR2 = Lm_B2( MAC2, 1 );
			IR3 = Lm_B3( MAC3, 1 );
			CD0 = CD1;
			CD1 = CD2;
			CD2 = CODE;
			R0 = R1;
			R1 = R2;
			R2 = Lm_C1( (INT32)MAC1 >> 4 );
			G0 = G1;
			G1 = G2;
			G2 = Lm_C2( (INT32)MAC2 >> 4 );
			B0 = B1;
			B1 = B2;
			B2 = Lm_C3( (INT32)MAC3 >> 4 );
			return;
		}
		break;
	case 0x16:
		if( gteop == 0x0f80416 )
		{
			GTELOG( "NCDT" );
			FLAG = 0;

			for( n_v = 0; n_v < 3; n_v++ )
			{
				MAC1 = A1( ( ( (INT64)(INT16)L11 * (INT16)*p_n_vx[ n_v ] ) + ( (INT16)L12 * (INT16)*p_n_vy[ n_v ] ) + ( (INT16)L13 * (INT16)*p_n_vz[ n_v ] ) ) >> 12 );
				MAC2 = A2( ( ( (INT64)(INT16)L21 * (INT16)*p_n_vx[ n_v ] ) + ( (INT16)L22 * (INT16)*p_n_vy[ n_v ] ) + ( (INT16)L23 * (INT16)*p_n_vz[ n_v ] ) ) >> 12 );
				MAC3 = A3( ( ( (INT64)(INT16)L31 * (INT16)*p_n_vx[ n_v ] ) + ( (INT16)L32 * (INT16)*p_n_vy[ n_v ] ) + ( (INT16)L33 * (INT16)*p_n_vz[ n_v ] ) ) >> 12 );
				IR1 = Lm_B1( (INT32)MAC1, 1 );
				IR2 = Lm_B2( (INT32)MAC2, 1 );
				IR3 = Lm_B3( (INT32)MAC3, 1 );
				MAC1 = A1( ( ( (INT64)RBK << 12 ) + ( (INT16)LR1 * (INT16)IR1 ) + ( (INT16)LR2 * (INT16)IR2 ) + ( (INT16)LR3 * (INT16)IR3 ) ) >> 12 );
				MAC2 = A2( ( ( (INT64)GBK << 12 ) + ( (INT16)LG1 * (INT16)IR1 ) + ( (INT16)LG2 * (INT16)IR2 ) + ( (INT16)LG3 * (INT16)IR3 ) ) >> 12 );
				MAC3 = A3( ( ( (INT64)BBK << 12 ) + ( (INT16)LB1 * (INT16)IR1 ) + ( (INT16)LB2 * (INT16)IR2 ) + ( (INT16)LB3 * (INT16)IR3 ) ) >> 12 );
				IR1 = Lm_B1( (INT32)MAC1, 1 );
				IR2 = Lm_B2( (INT32)MAC2, 1 );
				IR3 = Lm_B3( (INT32)MAC3, 1 );
				MAC1 = A1( ( ( ( (INT64)R << 4 ) * (INT16)IR1 ) + ( (INT16)IR0 * Lm_B1( (INT32)RFC - ( ( R * (INT16)IR1 ) >> 8 ), 0 ) ) ) >> 12 );
				MAC2 = A2( ( ( ( (INT64)G << 4 ) * (INT16)IR2 ) + ( (INT16)IR0 * Lm_B2( (INT32)GFC - ( ( G * (INT16)IR2 ) >> 8 ), 0 ) ) ) >> 12 );
				MAC3 = A3( ( ( ( (INT64)B << 4 ) * (INT16)IR3 ) + ( (INT16)IR0 * Lm_B3( (INT32)BFC - ( ( B * (INT16)IR3 ) >> 8 ), 0 ) ) ) >> 12 );
				IR1 = Lm_B1( (INT32)MAC1, 1 );
				IR2 = Lm_B2( (INT32)MAC2, 1 );
				IR3 = Lm_B3( (INT32)MAC3, 1 );
				CD0 = CD1;
				CD1 = CD2;
				CD2 = CODE;
				R0 = R1;
				R1 = R2;
				R2 = Lm_C1( (INT32)MAC1 >> 4 );
				G0 = G1;
				G1 = G2;
				G2 = Lm_C2( (INT32)MAC2 >> 4 );
				B0 = B1;
				B1 = B2;
				B2 = Lm_C3( (INT32)MAC3 >> 4 );
			}
			return;
		}
		break;
	case 0x1b:
		if( gteop == 0x108041b )
		{
			GTELOG( "NCCS" );
			FLAG = 0;

			MAC1 = A1( ( ( (INT64)(INT16)L11 * (INT16)VX0 ) + ( (INT16)L12 * (INT16)VY0 ) + ( (INT16)L13 * (INT16)VZ0 ) ) >> 12 );
			MAC2 = A2( ( ( (INT64)(INT16)L21 * (INT16)VX0 ) + ( (INT16)L22 * (INT16)VY0 ) + ( (INT16)L23 * (INT16)VZ0 ) ) >> 12 );
			MAC3 = A3( ( ( (INT64)(INT16)L31 * (INT16)VX0 ) + ( (INT16)L32 * (INT16)VY0 ) + ( (INT16)L33 * (INT16)VZ0 ) ) >> 12 );
			IR1 = Lm_B1( (INT32)MAC1, 1 );
			IR2 = Lm_B2( (INT32)MAC2, 1 );
			IR3 = Lm_B3( (INT32)MAC3, 1 );
			MAC1 = A1( ( ( (INT64)RBK << 12 ) + ( (INT16)LR1 * (INT16)IR1 ) + ( (INT16)LR2 * (INT16)IR2 ) + ( (INT16)LR3 * (INT16)IR3 ) ) >> 12 );
			MAC2 = A2( ( ( (INT64)GBK << 12 ) + ( (INT16)LG1 * (INT16)IR1 ) + ( (INT16)LG2 * (INT16)IR2 ) + ( (INT16)LG3 * (INT16)IR3 ) ) >> 12 );
			MAC3 = A3( ( ( (INT64)BBK << 12 ) + ( (INT16)LB1 * (INT16)IR1 ) + ( (INT16)LB2 * (INT16)IR2 ) + ( (INT16)LB3 * (INT16)IR3 ) ) >> 12 );
			IR1 = Lm_B1( (INT32)MAC1, 1 );
			IR2 = Lm_B2( (INT32)MAC2, 1 );
			IR3 = Lm_B3( (INT32)MAC3, 1 );
			MAC1 = A1( ( (INT64)R * (INT16)IR1 ) >> 8 );
			MAC2 = A2( ( (INT64)G * (INT16)IR2 ) >> 8 );
			MAC3 = A3( ( (INT64)B * (INT16)IR3 ) >> 8 );
			IR1 = Lm_B1( (INT32)MAC1, 1 );
			IR2 = Lm_B2( (INT32)MAC2, 1 );
			IR3 = Lm_B3( (INT32)MAC3, 1 );
			CD0 = CD1;
			CD1 = CD2;
			CD2 = CODE;
			R0 = R1;
			R1 = R2;
			R2 = Lm_C1( (INT32)MAC1 >> 4 );
			G0 = G1;
			G1 = G2;
			G2 = Lm_C2( (INT32)MAC2 >> 4 );
			B0 = B1;
			B1 = B2;
			B2 = Lm_C3( (INT32)MAC3 >> 4 );
			return;
		}
		break;
	case 0x1c:
		if( gteop == 0x138041c )
		{
			GTELOG( "CC" );
			FLAG = 0;

			MAC1 = A1( ( ( (INT64)RBK << 12 ) + ( (INT16)LR1 * (INT16)IR1 ) + ( (INT16)LR2 * (INT16)IR2 ) + ( (INT16)LR3 * (INT16)IR3 ) ) >> 12 );
			MAC2 = A2( ( ( (INT64)GBK << 12 ) + ( (INT16)LG1 * (INT16)IR1 ) + ( (INT16)LG2 * (INT16)IR2 ) + ( (INT16)LG3 * (INT16)IR3 ) ) >> 12 );
			MAC3 = A3( ( ( (INT64)BBK << 12 ) + ( (INT16)LB1 * (INT16)IR1 ) + ( (INT16)LB2 * (INT16)IR2 ) + ( (INT16)LB3 * (INT16)IR3 ) ) >> 12 );
			IR1 = Lm_B1( MAC1, 1 );
			IR2 = Lm_B2( MAC2, 1 );
			IR3 = Lm_B3( MAC3, 1 );
			MAC1 = A1( ( (INT64)R * (INT16)IR1 ) >> 8 );
			MAC2 = A2( ( (INT64)G * (INT16)IR2 ) >> 8 );
			MAC3 = A3( ( (INT64)B * (INT16)IR3 ) >> 8 );
			IR1 = Lm_B1( MAC1, 1 );
			IR2 = Lm_B2( MAC2, 1 );
			IR3 = Lm_B3( MAC3, 1 );
			CD0 = CD1;
			CD1 = CD2;
			CD2 = CODE;
			R0 = R1;
			R1 = R2;
			R2 = Lm_C1( (INT32)MAC1 >> 4 );
			G0 = G1;
			G1 = G2;
			G2 = Lm_C2( (INT32)MAC2 >> 4 );
			B0 = B1;
			B1 = B2;
			B2 = Lm_C3( (INT32)MAC3 >> 4 );
			return;
		}
		break;
	case 0x1e:
		if( gteop == 0x0c8041e )
		{
			GTELOG( "NCS" );
			FLAG = 0;

			MAC1 = A1( ( ( (INT64)(INT16)L11 * (INT16)VX0 ) + ( (INT16)L12 * (INT16)VY0 ) + ( (INT16)L13 * (INT16)VZ0 ) ) >> 12 );
			MAC2 = A2( ( ( (INT64)(INT16)L21 * (INT16)VX0 ) + ( (INT16)L22 * (INT16)VY0 ) + ( (INT16)L23 * (INT16)VZ0 ) ) >> 12 );
			MAC3 = A3( ( ( (INT64)(INT16)L31 * (INT16)VX0 ) + ( (INT16)L32 * (INT16)VY0 ) + ( (INT16)L33 * (INT16)VZ0 ) ) >> 12 );
			IR1 = Lm_B1( (INT32)MAC1, 1 );
			IR2 = Lm_B2( (INT32)MAC2, 1 );
			IR3 = Lm_B3( (INT32)MAC3, 1 );
			MAC1 = A1( ( ( (INT64)RBK << 12 ) + ( (INT16)LR1 * (INT16)IR1 ) + ( (INT16)LR2 * (INT16)IR2 ) + ( (INT16)LR3 * (INT16)IR3 ) ) >> 12 );
			MAC2 = A2( ( ( (INT64)GBK << 12 ) + ( (INT16)LG1 * (INT16)IR1 ) + ( (INT16)LG2 * (INT16)IR2 ) + ( (INT16)LG3 * (INT16)IR3 ) ) >> 12 );
			MAC3 = A3( ( ( (INT64)BBK << 12 ) + ( (INT16)LB1 * (INT16)IR1 ) + ( (INT16)LB2 * (INT16)IR2 ) + ( (INT16)LB3 * (INT16)IR3 ) ) >> 12 );
			IR1 = Lm_B1( (INT32)MAC1, 1 );
			IR2 = Lm_B2( (INT32)MAC2, 1 );
			IR3 = Lm_B3( (INT32)MAC3, 1 );
			CD0 = CD1;
			CD1 = CD2;
			CD2 = CODE;
			R0 = R1;
			R1 = R2;
			R2 = Lm_C1( (INT32)MAC1 >> 4 );
			G0 = G1;
			G1 = G2;
			G2 = Lm_C2( (INT32)MAC2 >> 4 );
			B0 = B1;
			B1 = B2;
			B2 = Lm_C3( (INT32)MAC3 >> 4 );
			return;
		}
		break;
	case 0x20:
		if( gteop == 0x0d80420 )
		{
			GTELOG( "NCT" );
			FLAG = 0;

			for( n_v = 0; n_v < 3; n_v++ )
			{
				MAC1 = A1( ( ( (INT64)(INT16)L11 * (INT16)*p_n_vx[ n_v ] ) + ( (INT16)L12 * (INT16)*p_n_vy[ n_v ] ) + ( (INT16)L13 * (INT16)*p_n_vz[ n_v ] ) ) >> 12 );
				MAC2 = A2( ( ( (INT64)(INT16)L21 * (INT16)*p_n_vx[ n_v ] ) + ( (INT16)L22 * (INT16)*p_n_vy[ n_v ] ) + ( (INT16)L23 * (INT16)*p_n_vz[ n_v ] ) ) >> 12 );
				MAC3 = A3( ( ( (INT64)(INT16)L31 * (INT16)*p_n_vx[ n_v ] ) + ( (INT16)L32 * (INT16)*p_n_vy[ n_v ] ) + ( (INT16)L33 * (INT16)*p_n_vz[ n_v ] ) ) >> 12 );
				IR1 = Lm_B1( (INT32)MAC1, 1 );
				IR2 = Lm_B2( (INT32)MAC2, 1 );
				IR3 = Lm_B3( (INT32)MAC3, 1 );
				MAC1 = A1( ( ( (INT64)RBK << 12 ) + ( (INT16)LR1 * (INT16)IR1 ) + ( (INT16)LR2 * (INT16)IR2 ) + ( (INT16)LR3 * (INT16)IR3 ) ) >> 12 );
				MAC2 = A2( ( ( (INT64)GBK << 12 ) + ( (INT16)LG1 * (INT16)IR1 ) + ( (INT16)LG2 * (INT16)IR2 ) + ( (INT16)LG3 * (INT16)IR3 ) ) >> 12 );
				MAC3 = A3( ( ( (INT64)BBK << 12 ) + ( (INT16)LB1 * (INT16)IR1 ) + ( (INT16)LB2 * (INT16)IR2 ) + ( (INT16)LB3 * (INT16)IR3 ) ) >> 12 );
				IR1 = Lm_B1( (INT32)MAC1, 1 );
				IR2 = Lm_B2( (INT32)MAC2, 1 );
				IR3 = Lm_B3( (INT32)MAC3, 1 );
				CD0 = CD1;
				CD1 = CD2;
				CD2 = CODE;
				R0 = R1;
				R1 = R2;
				R2 = Lm_C1( (INT32)MAC1 >> 4 );
				G0 = G1;
				G1 = G2;
				G2 = Lm_C2( (INT32)MAC2 >> 4 );
				B0 = B1;
				B1 = B2;
				B2 = Lm_C3( (INT32)MAC3 >> 4 );
			}
			return;
		}
		break;
	case 0x28:
		if( GTE_OP( gteop ) == 0x0a && GTE_LM( gteop ) == 1 )
		{
			GTELOG( "SQR" );
			n_sf = 12 * GTE_SF( gteop );
			FLAG = 0;

			MAC1 = A1( ( (INT64)(INT16)IR1 * (INT16)IR1 ) >> n_sf );
			MAC2 = A2( ( (INT64)(INT16)IR2 * (INT16)IR2 ) >> n_sf );
			MAC3 = A3( ( (INT64)(INT16)IR3 * (INT16)IR3 ) >> n_sf );
			IR1 = Lm_B1( MAC1, 1 );
			IR2 = Lm_B2( MAC2, 1 );
			IR3 = Lm_B3( MAC3, 1 );
			return;
		}
		break;
	// DCPL 0x29
	case 0x2a:
		if( gteop == 0x0f8002a )
		{
			GTELOG( "DPCT" );
			FLAG = 0;

			for( n_pass = 0; n_pass < 3; n_pass++ )
			{
				MAC1 = A1( ( ( (INT64)R0 << 16 ) + ( (INT64)(INT16)IR0 * ( Lm_B1( (INT32)RFC - ( R0 << 4 ), 0 ) ) ) ) >> 12 );
				MAC2 = A2( ( ( (INT64)G0 << 16 ) + ( (INT64)(INT16)IR0 * ( Lm_B1( (INT32)GFC - ( G0 << 4 ), 0 ) ) ) ) >> 12 );
				MAC3 = A3( ( ( (INT64)B0 << 16 ) + ( (INT64)(INT16)IR0 * ( Lm_B1( (INT32)BFC - ( B0 << 4 ), 0 ) ) ) ) >> 12 );
				IR1 = Lm_B1( (INT32)MAC1, 0 );
				IR2 = Lm_B2( (INT32)MAC2, 0 );
				IR3 = Lm_B3( (INT32)MAC3, 0 );
				CD0 = CD1;
				CD1 = CD2;
				CD2 = CODE;
				R0 = R1;
				R1 = R2;
				R2 = Lm_C1( (INT32)MAC1 >> 4 );
				G0 = G1;
				G1 = G2;
				G2 = Lm_C2( (INT32)MAC2 >> 4 );
				B0 = B1;
				B1 = B2;
				B2 = Lm_C3( (INT32)MAC3 >> 4 );
			}
			return;
		}
		break;
	case 0x2d:
		if( gteop == 0x158002d )
		{
			GTELOG( "AVSZ3" );
			FLAG = 0;

			MAC0 = F( ( (INT64)(INT16)ZSF3 * SZ1 ) + ( (INT16)ZSF3 * SZ2 ) + ( (INT16)ZSF3 * SZ3 ) );
			OTZ = Lm_D( (INT32)MAC0 >> 12 );
			return;
		}
		break;
	case 0x2e:
		if( gteop == 0x168002e )
		{
			GTELOG( "AVSZ4" );
			FLAG = 0;

			MAC0 = F( ( (INT64)(INT16)ZSF4 * SZ0 ) + ( (INT16)ZSF4 * SZ1 ) + ( (INT16)ZSF4 * SZ2 ) + ( (INT16)ZSF4 * SZ3 ) );
			OTZ = Lm_D( (INT32)MAC0 >> 12 );
			return;
		}
		break;
	case 0x30:
		if( gteop == 0x0280030 )
		{
			GTELOG( "RTPT" );
			FLAG = 0;

			for( n_v = 0; n_v < 3; n_v++ )
			{
				MAC1 = A1( ( ( (INT64)(INT32)TRX << 12 ) + ( (INT16)R11 * (INT16)*p_n_vx[ n_v ] ) + ( (INT16)R12 * (INT16)*p_n_vy[ n_v ] ) + ( (INT16)R13 * (INT16)*p_n_vz[ n_v ] ) ) >> 12 );
				MAC2 = A2( ( ( (INT64)(INT32)TRY << 12 ) + ( (INT16)R21 * (INT16)*p_n_vx[ n_v ] ) + ( (INT16)R22 * (INT16)*p_n_vy[ n_v ] ) + ( (INT16)R23 * (INT16)*p_n_vz[ n_v ] ) ) >> 12 );
				MAC3 = A3( ( ( (INT64)(INT32)TRZ << 12 ) + ( (INT16)R31 * (INT16)*p_n_vx[ n_v ] ) + ( (INT16)R32 * (INT16)*p_n_vy[ n_v ] ) + ( (INT16)R33 * (INT16)*p_n_vz[ n_v ] ) ) >> 12 );
				IR1 = Lm_B1( (INT32)MAC1, 0 );
				IR2 = Lm_B2( (INT32)MAC2, 0 );
				IR3 = Lm_B3( (INT32)MAC3, 0 );
				SZ0 = SZ1;
				SZ1 = SZ2;
				SZ2 = SZ3;
				SZ3 = Lm_D( (INT32)MAC3 );
				SXY0 = SXY1;
				SXY1 = SXY2;
				SX2 = Lm_G1( F( ( (INT64)(INT32)OFX + ( (INT64)(INT16)IR1 * ( ( (UINT32)H << 16 ) / Lm_E( SZ3 ) ) ) ) >> 16 ) );
				SY2 = Lm_G2( F( ( (INT64)(INT32)OFY + ( (INT64)(INT16)IR2 * ( ( (UINT32)H << 16 ) / Lm_E( SZ3 ) ) ) ) >> 16 ) );
				MAC0 = F( (INT64)(INT32)DQB + ( (INT64)(INT16)DQA * ( ( (UINT32)H << 16 ) / Lm_E( SZ3 ) ) ) );
				IR0 = Lm_H( (INT32)MAC0 >> 12 );
			}
			return;
		}
		break;
	case 0x3d:
		if( GTE_OP( gteop ) == 0x09 ||
			GTE_OP( gteop ) == 0x19 )
		{
			GTELOG( "GPF" );
			n_sf = 12 * GTE_SF( gteop );
			FLAG = 0;

			MAC1 = A1( ( (INT64)(INT16)IR0 * (INT16)IR1 ) >> n_sf );
			MAC2 = A2( ( (INT64)(INT16)IR0 * (INT16)IR2 ) >> n_sf );
			MAC3 = A3( ( (INT64)(INT16)IR0 * (INT16)IR3 ) >> n_sf );
			IR1 = Lm_B1( (INT32)MAC1, 0 );
			IR2 = Lm_B2( (INT32)MAC2, 0 );
			IR3 = Lm_B3( (INT32)MAC3, 0 );
			CD0 = CD1;
			CD1 = CD2;
			CD2 = CODE;
			R0 = R1;
			R1 = R2;
			R2 = Lm_C1( (INT32)MAC1 >> 4 );
			G0 = G1;
			G1 = G2;
			G2 = Lm_C2( (INT32)MAC2 >> 4 );
			B0 = B1;
			B1 = B2;
			B2 = Lm_C3( (INT32)MAC3 >> 4 );
			return;
		}
		break;
	case 0x3e:
		if( GTE_OP( gteop ) == 0x1a )
		{
			GTELOG( "GPL" );
			n_sf = 12 * GTE_SF( gteop );
			FLAG = 0;

			MAC1 = A1( ( ( (INT64)(INT32)MAC1 << n_sf ) + ( (INT16)IR0 * (INT16)IR1 ) ) >> n_sf );
			MAC2 = A2( ( ( (INT64)(INT32)MAC2 << n_sf ) + ( (INT16)IR0 * (INT16)IR2 ) ) >> n_sf );
			MAC3 = A3( ( ( (INT64)(INT32)MAC3 << n_sf ) + ( (INT16)IR0 * (INT16)IR3 ) ) >> n_sf );
			IR1 = Lm_B1( (INT32)MAC1, 0 );
			IR2 = Lm_B2( (INT32)MAC2, 0 );
			IR3 = Lm_B3( (INT32)MAC3, 0 );
			CD0 = CD1;
			CD1 = CD2;
			CD2 = CODE;
			R0 = R1;
			R1 = R2;
			R2 = Lm_C1( (INT32)MAC1 >> 4 );
			G0 = G1;
			G1 = G2;
			G2 = Lm_C2( (INT32)MAC2 >> 4 );
			B0 = B1;
			B1 = B2;
			B2 = Lm_C3( (INT32)MAC3 >> 4 );
			return;
		}
		break;
	case 0x3f:
		if( gteop == 0x108043f ||
			gteop == 0x118043f )
		{
			GTELOG( "NCCT" );
			FLAG = 0;

			for( n_v = 0; n_v < 3; n_v++ )
			{
				MAC1 = A1( ( ( (INT64)(INT16)L11 * (INT16)*p_n_vx[ n_v ] ) + ( (INT16)L12 * (INT16)*p_n_vy[ n_v ] ) + ( (INT16)L13 * (INT16)*p_n_vz[ n_v ] ) ) >> 12 );
				MAC2 = A2( ( ( (INT64)(INT16)L21 * (INT16)*p_n_vx[ n_v ] ) + ( (INT16)L22 * (INT16)*p_n_vy[ n_v ] ) + ( (INT16)L23 * (INT16)*p_n_vz[ n_v ] ) ) >> 12 );
				MAC3 = A3( ( ( (INT64)(INT16)L31 * (INT16)*p_n_vx[ n_v ] ) + ( (INT16)L32 * (INT16)*p_n_vy[ n_v ] ) + ( (INT16)L33 * (INT16)*p_n_vz[ n_v ] ) ) >> 12 );
				IR1 = Lm_B1( (INT32)MAC1, 1 );
				IR2 = Lm_B2( (INT32)MAC2, 1 );
				IR3 = Lm_B3( (INT32)MAC3, 1 );
				MAC1 = A1( ( ( (INT64)RBK << 12 ) + ( (INT16)LR1 * (INT16)IR1 ) + ( (INT16)LR2 * (INT16)IR2 ) + ( (INT16)LR3 * (INT16)IR3 ) ) >> 12 );
				MAC2 = A2( ( ( (INT64)GBK << 12 ) + ( (INT16)LG1 * (INT16)IR1 ) + ( (INT16)LG2 * (INT16)IR2 ) + ( (INT16)LG3 * (INT16)IR3 ) ) >> 12 );
				MAC3 = A3( ( ( (INT64)BBK << 12 ) + ( (INT16)LB1 * (INT16)IR1 ) + ( (INT16)LB2 * (INT16)IR2 ) + ( (INT16)LB3 * (INT16)IR3 ) ) >> 12 );
				IR1 = Lm_B1( (INT32)MAC1, 1 );
				IR2 = Lm_B2( (INT32)MAC2, 1 );
				IR3 = Lm_B3( (INT32)MAC3, 1 );
				MAC1 = A1( ( (INT64)R * (INT16)IR1 ) >> 8 );
				MAC2 = A2( ( (INT64)G * (INT16)IR2 ) >> 8 );
				MAC3 = A3( ( (INT64)B * (INT16)IR3 ) >> 8 );
				IR1 = Lm_B1( (INT32)MAC1, 1 );
				IR2 = Lm_B2( (INT32)MAC2, 1 );
				IR3 = Lm_B3( (INT32)MAC3, 1 );
				CD0 = CD1;
				CD1 = CD2;
				CD2 = CODE;
				R0 = R1;
				R1 = R2;
				R2 = Lm_C1( (INT32)MAC1 >> 4 );
				G0 = G1;
				G1 = G2;
				G2 = Lm_C2( (INT32)MAC2 >> 4 );
				B0 = B1;
				B1 = B2;
				B2 = Lm_C3( (INT32)MAC3 >> 4 );
			}
			return;
		}
		break;
	}
	ui_popup_time( 1, "unknown GTE op %08x", gteop );
	logerror( "%08x: unknown GTE op %08x\n", mipscpu.pc, gteop );
	mips_stop();
}

/**************************************************************************
 * Generic set_info
 **************************************************************************/

static void mips_set_info(UINT32 state, cpuinfo *info)
{
	switch (state)
	{
		/* --- the following bits of info are set as 64-bit signed integers --- */
		case CPUINFO_INT_INPUT_STATE + MIPS_IRQ0:		set_irq_line(MIPS_IRQ0, info->i);		break;
		case CPUINFO_INT_INPUT_STATE + MIPS_IRQ1:		set_irq_line(MIPS_IRQ1, info->i);		break;
		case CPUINFO_INT_INPUT_STATE + MIPS_IRQ2:		set_irq_line(MIPS_IRQ2, info->i);		break;
		case CPUINFO_INT_INPUT_STATE + MIPS_IRQ3:		set_irq_line(MIPS_IRQ3, info->i);		break;
		case CPUINFO_INT_INPUT_STATE + MIPS_IRQ4:		set_irq_line(MIPS_IRQ4, info->i);		break;
		case CPUINFO_INT_INPUT_STATE + MIPS_IRQ5:		set_irq_line(MIPS_IRQ5, info->i);		break;

		case CPUINFO_INT_PC:							mips_set_pc( info->i );					break;
		case CPUINFO_INT_REGISTER + MIPS_PC:			mips_set_pc( info->i );					break;
		case CPUINFO_INT_SP:							/* no stack */							break;
		case CPUINFO_INT_REGISTER + MIPS_DELAYV:		mipscpu.delayv = info->i;				break;
		case CPUINFO_INT_REGISTER + MIPS_DELAYR:		if( info->i <= REGPC ) mipscpu.delayr = info->i; break;
		case CPUINFO_INT_REGISTER + MIPS_HI:			mipscpu.hi = info->i;					break;
		case CPUINFO_INT_REGISTER + MIPS_LO:			mipscpu.lo = info->i;					break;
		case CPUINFO_INT_REGISTER + MIPS_R0:			mipscpu.r[ 0 ] = info->i;				break;
		case CPUINFO_INT_REGISTER + MIPS_R1:			mipscpu.r[ 1 ] = info->i;				break;
		case CPUINFO_INT_REGISTER + MIPS_R2:			mipscpu.r[ 2 ] = info->i;				break;
		case CPUINFO_INT_REGISTER + MIPS_R3:			mipscpu.r[ 3 ] = info->i;				break;
		case CPUINFO_INT_REGISTER + MIPS_R4:			mipscpu.r[ 4 ] = info->i;				break;
		case CPUINFO_INT_REGISTER + MIPS_R5:			mipscpu.r[ 5 ] = info->i;				break;
		case CPUINFO_INT_REGISTER + MIPS_R6:			mipscpu.r[ 6 ] = info->i;				break;
		case CPUINFO_INT_REGISTER + MIPS_R7:			mipscpu.r[ 7 ] = info->i;				break;
		case CPUINFO_INT_REGISTER + MIPS_R8:			mipscpu.r[ 8 ] = info->i;				break;
		case CPUINFO_INT_REGISTER + MIPS_R9:			mipscpu.r[ 9 ] = info->i;				break;
		case CPUINFO_INT_REGISTER + MIPS_R10:			mipscpu.r[ 10 ] = info->i;				break;
		case CPUINFO_INT_REGISTER + MIPS_R11:			mipscpu.r[ 11 ] = info->i;				break;
		case CPUINFO_INT_REGISTER + MIPS_R12:			mipscpu.r[ 12 ] = info->i;				break;
		case CPUINFO_INT_REGISTER + MIPS_R13:			mipscpu.r[ 13 ] = info->i;				break;
		case CPUINFO_INT_REGISTER + MIPS_R14:			mipscpu.r[ 14 ] = info->i;				break;
		case CPUINFO_INT_REGISTER + MIPS_R15:			mipscpu.r[ 15 ] = info->i;				break;
		case CPUINFO_INT_REGISTER + MIPS_R16:			mipscpu.r[ 16 ] = info->i;				break;
		case CPUINFO_INT_REGISTER + MIPS_R17:			mipscpu.r[ 17 ] = info->i;				break;
		case CPUINFO_INT_REGISTER + MIPS_R18:			mipscpu.r[ 18 ] = info->i;				break;
		case CPUINFO_INT_REGISTER + MIPS_R19:			mipscpu.r[ 19 ] = info->i;				break;
		case CPUINFO_INT_REGISTER + MIPS_R20:			mipscpu.r[ 20 ] = info->i;				break;
		case CPUINFO_INT_REGISTER + MIPS_R21:			mipscpu.r[ 21 ] = info->i;				break;
		case CPUINFO_INT_REGISTER + MIPS_R22:			mipscpu.r[ 22 ] = info->i;				break;
		case CPUINFO_INT_REGISTER + MIPS_R23:			mipscpu.r[ 23 ] = info->i;				break;
		case CPUINFO_INT_REGISTER + MIPS_R24:			mipscpu.r[ 24 ] = info->i;				break;
		case CPUINFO_INT_REGISTER + MIPS_R25:			mipscpu.r[ 25 ] = info->i;				break;
		case CPUINFO_INT_REGISTER + MIPS_R26:			mipscpu.r[ 26 ] = info->i;				break;
		case CPUINFO_INT_REGISTER + MIPS_R27:			mipscpu.r[ 27 ] = info->i;				break;
		case CPUINFO_INT_REGISTER + MIPS_R28:			mipscpu.r[ 28 ] = info->i;				break;
		case CPUINFO_INT_REGISTER + MIPS_R29:			mipscpu.r[ 29 ] = info->i;				break;
		case CPUINFO_INT_REGISTER + MIPS_R30:			mipscpu.r[ 30 ] = info->i;				break;
		case CPUINFO_INT_REGISTER + MIPS_R31:			mipscpu.r[ 31 ] = info->i;				break;
		case CPUINFO_INT_REGISTER + MIPS_CP0R0:			mips_set_cp0r( 0, info->i );			break;
		case CPUINFO_INT_REGISTER + MIPS_CP0R1:			mips_set_cp0r( 1, info->i );			break;
		case CPUINFO_INT_REGISTER + MIPS_CP0R2:			mips_set_cp0r( 2, info->i );			break;
		case CPUINFO_INT_REGISTER + MIPS_CP0R3:			mips_set_cp0r( 3, info->i );			break;
		case CPUINFO_INT_REGISTER + MIPS_CP0R4:			mips_set_cp0r( 4, info->i );			break;
		case CPUINFO_INT_REGISTER + MIPS_CP0R5:			mips_set_cp0r( 5, info->i );			break;
		case CPUINFO_INT_REGISTER + MIPS_CP0R6:			mips_set_cp0r( 6, info->i );			break;
		case CPUINFO_INT_REGISTER + MIPS_CP0R7:			mips_set_cp0r( 7, info->i );			break;
		case CPUINFO_INT_REGISTER + MIPS_CP0R8:			mips_set_cp0r( 8, info->i );			break;
		case CPUINFO_INT_REGISTER + MIPS_CP0R9:			mips_set_cp0r( 9, info->i );			break;
		case CPUINFO_INT_REGISTER + MIPS_CP0R10:		mips_set_cp0r( 10, info->i );			break;
		case CPUINFO_INT_REGISTER + MIPS_CP0R11:		mips_set_cp0r( 11, info->i );			break;
		case CPUINFO_INT_REGISTER + MIPS_CP0R12:		mips_set_cp0r( 12, info->i );			break;
		case CPUINFO_INT_REGISTER + MIPS_CP0R13:		mips_set_cp0r( 13, info->i );			break;
		case CPUINFO_INT_REGISTER + MIPS_CP0R14:		mips_set_cp0r( 14, info->i );			break;
		case CPUINFO_INT_REGISTER + MIPS_CP0R15:		mips_set_cp0r( 15, info->i );			break;
		case CPUINFO_INT_REGISTER + MIPS_CP0R16:		mips_set_cp0r( 16, info->i );			break;
		case CPUINFO_INT_REGISTER + MIPS_CP0R17:		mips_set_cp0r( 17, info->i );			break;
		case CPUINFO_INT_REGISTER + MIPS_CP0R18:		mips_set_cp0r( 18, info->i );			break;
		case CPUINFO_INT_REGISTER + MIPS_CP0R19:		mips_set_cp0r( 19, info->i );			break;
		case CPUINFO_INT_REGISTER + MIPS_CP0R20:		mips_set_cp0r( 20, info->i );			break;
		case CPUINFO_INT_REGISTER + MIPS_CP0R21:		mips_set_cp0r( 21, info->i );			break;
		case CPUINFO_INT_REGISTER + MIPS_CP0R22:		mips_set_cp0r( 22, info->i );			break;
		case CPUINFO_INT_REGISTER + MIPS_CP0R23:		mips_set_cp0r( 23, info->i );			break;
		case CPUINFO_INT_REGISTER + MIPS_CP0R24:		mips_set_cp0r( 24, info->i );			break;
		case CPUINFO_INT_REGISTER + MIPS_CP0R25:		mips_set_cp0r( 25, info->i );			break;
		case CPUINFO_INT_REGISTER + MIPS_CP0R26:		mips_set_cp0r( 26, info->i );			break;
		case CPUINFO_INT_REGISTER + MIPS_CP0R27:		mips_set_cp0r( 27, info->i );			break;
		case CPUINFO_INT_REGISTER + MIPS_CP0R28:		mips_set_cp0r( 28, info->i );			break;
		case CPUINFO_INT_REGISTER + MIPS_CP0R29:		mips_set_cp0r( 29, info->i );			break;
		case CPUINFO_INT_REGISTER + MIPS_CP0R30:		mips_set_cp0r( 30, info->i );			break;
		case CPUINFO_INT_REGISTER + MIPS_CP0R31:		mips_set_cp0r( 31, info->i );			break;
		case CPUINFO_INT_REGISTER + MIPS_CP2DR0:		mipscpu.cp2dr[ 0 ].d = info->i;			break;
		case CPUINFO_INT_REGISTER + MIPS_CP2DR1:		mipscpu.cp2dr[ 1 ].d = info->i;			break;
		case CPUINFO_INT_REGISTER + MIPS_CP2DR2:		mipscpu.cp2dr[ 2 ].d = info->i;			break;
		case CPUINFO_INT_REGISTER + MIPS_CP2DR3:		mipscpu.cp2dr[ 3 ].d = info->i;			break;
		case CPUINFO_INT_REGISTER + MIPS_CP2DR4:		mipscpu.cp2dr[ 4 ].d = info->i;			break;
		case CPUINFO_INT_REGISTER + MIPS_CP2DR5:		mipscpu.cp2dr[ 5 ].d = info->i;			break;
		case CPUINFO_INT_REGISTER + MIPS_CP2DR6:		mipscpu.cp2dr[ 6 ].d = info->i;			break;
		case CPUINFO_INT_REGISTER + MIPS_CP2DR7:		mipscpu.cp2dr[ 7 ].d = info->i;			break;
		case CPUINFO_INT_REGISTER + MIPS_CP2DR8:		mipscpu.cp2dr[ 8 ].d = info->i;			break;
		case CPUINFO_INT_REGISTER + MIPS_CP2DR9:		mipscpu.cp2dr[ 9 ].d = info->i;			break;
		case CPUINFO_INT_REGISTER + MIPS_CP2DR10:		mipscpu.cp2dr[ 10 ].d = info->i;		break;
		case CPUINFO_INT_REGISTER + MIPS_CP2DR11:		mipscpu.cp2dr[ 11 ].d = info->i;		break;
		case CPUINFO_INT_REGISTER + MIPS_CP2DR12:		mipscpu.cp2dr[ 12 ].d = info->i;		break;
		case CPUINFO_INT_REGISTER + MIPS_CP2DR13:		mipscpu.cp2dr[ 13 ].d = info->i;		break;
		case CPUINFO_INT_REGISTER + MIPS_CP2DR14:		mipscpu.cp2dr[ 14 ].d = info->i;		break;
		case CPUINFO_INT_REGISTER + MIPS_CP2DR15:		mipscpu.cp2dr[ 15 ].d = info->i;		break;
		case CPUINFO_INT_REGISTER + MIPS_CP2DR16:		mipscpu.cp2dr[ 16 ].d = info->i;		break;
		case CPUINFO_INT_REGISTER + MIPS_CP2DR17:		mipscpu.cp2dr[ 17 ].d = info->i;		break;
		case CPUINFO_INT_REGISTER + MIPS_CP2DR18:		mipscpu.cp2dr[ 18 ].d = info->i;		break;
		case CPUINFO_INT_REGISTER + MIPS_CP2DR19:		mipscpu.cp2dr[ 19 ].d = info->i;		break;
		case CPUINFO_INT_REGISTER + MIPS_CP2DR20:		mipscpu.cp2dr[ 20 ].d = info->i;		break;
		case CPUINFO_INT_REGISTER + MIPS_CP2DR21:		mipscpu.cp2dr[ 21 ].d = info->i;		break;
		case CPUINFO_INT_REGISTER + MIPS_CP2DR22:		mipscpu.cp2dr[ 22 ].d = info->i;		break;
		case CPUINFO_INT_REGISTER + MIPS_CP2DR23:		mipscpu.cp2dr[ 23 ].d = info->i;		break;
		case CPUINFO_INT_REGISTER + MIPS_CP2DR24:		mipscpu.cp2dr[ 24 ].d = info->i;		break;
		case CPUINFO_INT_REGISTER + MIPS_CP2DR25:		mipscpu.cp2dr[ 25 ].d = info->i;		break;
		case CPUINFO_INT_REGISTER + MIPS_CP2DR26:		mipscpu.cp2dr[ 26 ].d = info->i;		break;
		case CPUINFO_INT_REGISTER + MIPS_CP2DR27:		mipscpu.cp2dr[ 27 ].d = info->i;		break;
		case CPUINFO_INT_REGISTER + MIPS_CP2DR28:		mipscpu.cp2dr[ 28 ].d = info->i;		break;
		case CPUINFO_INT_REGISTER + MIPS_CP2DR29:		mipscpu.cp2dr[ 29 ].d = info->i;		break;
		case CPUINFO_INT_REGISTER + MIPS_CP2DR30:		mipscpu.cp2dr[ 30 ].d = info->i;		break;
		case CPUINFO_INT_REGISTER + MIPS_CP2DR31:		mipscpu.cp2dr[ 31 ].d = info->i;		break;
		case CPUINFO_INT_REGISTER + MIPS_CP2CR0:		mipscpu.cp2cr[ 0 ].d = info->i;			break;
		case CPUINFO_INT_REGISTER + MIPS_CP2CR1:		mipscpu.cp2cr[ 1 ].d = info->i;			break;
		case CPUINFO_INT_REGISTER + MIPS_CP2CR2:		mipscpu.cp2cr[ 2 ].d = info->i;			break;
		case CPUINFO_INT_REGISTER + MIPS_CP2CR3:		mipscpu.cp2cr[ 3 ].d = info->i;			break;
		case CPUINFO_INT_REGISTER + MIPS_CP2CR4:		mipscpu.cp2cr[ 4 ].d = info->i;			break;
		case CPUINFO_INT_REGISTER + MIPS_CP2CR5:		mipscpu.cp2cr[ 5 ].d = info->i;			break;
		case CPUINFO_INT_REGISTER + MIPS_CP2CR6:		mipscpu.cp2cr[ 6 ].d = info->i;			break;
		case CPUINFO_INT_REGISTER + MIPS_CP2CR7:		mipscpu.cp2cr[ 7 ].d = info->i;			break;
		case CPUINFO_INT_REGISTER + MIPS_CP2CR8:		mipscpu.cp2cr[ 8 ].d = info->i;			break;
		case CPUINFO_INT_REGISTER + MIPS_CP2CR9:		mipscpu.cp2cr[ 9 ].d = info->i;			break;
		case CPUINFO_INT_REGISTER + MIPS_CP2CR10:		mipscpu.cp2cr[ 10 ].d = info->i;		break;
		case CPUINFO_INT_REGISTER + MIPS_CP2CR11:		mipscpu.cp2cr[ 11 ].d = info->i;		break;
		case CPUINFO_INT_REGISTER + MIPS_CP2CR12:		mipscpu.cp2cr[ 12 ].d = info->i;		break;
		case CPUINFO_INT_REGISTER + MIPS_CP2CR13:		mipscpu.cp2cr[ 13 ].d = info->i;		break;
		case CPUINFO_INT_REGISTER + MIPS_CP2CR14:		mipscpu.cp2cr[ 14 ].d = info->i;		break;
		case CPUINFO_INT_REGISTER + MIPS_CP2CR15:		mipscpu.cp2cr[ 15 ].d = info->i;		break;
		case CPUINFO_INT_REGISTER + MIPS_CP2CR16:		mipscpu.cp2cr[ 16 ].d = info->i;		break;
		case CPUINFO_INT_REGISTER + MIPS_CP2CR17:		mipscpu.cp2cr[ 17 ].d = info->i;		break;
		case CPUINFO_INT_REGISTER + MIPS_CP2CR18:		mipscpu.cp2cr[ 18 ].d = info->i;		break;
		case CPUINFO_INT_REGISTER + MIPS_CP2CR19:		mipscpu.cp2cr[ 19 ].d = info->i;		break;
		case CPUINFO_INT_REGISTER + MIPS_CP2CR20:		mipscpu.cp2cr[ 20 ].d = info->i;		break;
		case CPUINFO_INT_REGISTER + MIPS_CP2CR21:		mipscpu.cp2cr[ 21 ].d = info->i;		break;
		case CPUINFO_INT_REGISTER + MIPS_CP2CR22:		mipscpu.cp2cr[ 22 ].d = info->i;		break;
		case CPUINFO_INT_REGISTER + MIPS_CP2CR23:		mipscpu.cp2cr[ 23 ].d = info->i;		break;
		case CPUINFO_INT_REGISTER + MIPS_CP2CR24:		mipscpu.cp2cr[ 24 ].d = info->i;		break;
		case CPUINFO_INT_REGISTER + MIPS_CP2CR25:		mipscpu.cp2cr[ 25 ].d = info->i;		break;
		case CPUINFO_INT_REGISTER + MIPS_CP2CR26:		mipscpu.cp2cr[ 26 ].d = info->i;		break;
		case CPUINFO_INT_REGISTER + MIPS_CP2CR27:		mipscpu.cp2cr[ 27 ].d = info->i;		break;
		case CPUINFO_INT_REGISTER + MIPS_CP2CR28:		mipscpu.cp2cr[ 28 ].d = info->i;		break;
		case CPUINFO_INT_REGISTER + MIPS_CP2CR29:		mipscpu.cp2cr[ 29 ].d = info->i;		break;
		case CPUINFO_INT_REGISTER + MIPS_CP2CR30:		mipscpu.cp2cr[ 30 ].d = info->i;		break;
		case CPUINFO_INT_REGISTER + MIPS_CP2CR31:		mipscpu.cp2cr[ 31 ].d = info->i;		break;
	}
}



/**************************************************************************
 * Generic get_info
 **************************************************************************/

static void mips_get_info(UINT32 state, cpuinfo *info)
{
	switch (state)
	{
		/* --- the following bits of info are returned as 64-bit signed integers --- */
		case CPUINFO_INT_CONTEXT_SIZE:					info->i = sizeof(mipscpu);				break;
		case CPUINFO_INT_INPUT_LINES:					info->i = 6;							break;
		case CPUINFO_INT_DEFAULT_IRQ_VECTOR:			info->i = 0;							break;
		case CPUINFO_INT_ENDIANNESS:					info->i = CPU_IS_LE;					break;
		case CPUINFO_INT_CLOCK_DIVIDER:					info->i = 1;							break;
		case CPUINFO_INT_MIN_INSTRUCTION_BYTES:			info->i = 4;							break;
		case CPUINFO_INT_MAX_INSTRUCTION_BYTES:			info->i = 4;							break;
		case CPUINFO_INT_MIN_CYCLES:					info->i = 1;							break;
		case CPUINFO_INT_MAX_CYCLES:					info->i = 40;							break;

		case CPUINFO_INT_DATABUS_WIDTH + ADDRESS_SPACE_PROGRAM:	info->i = 32;					break;
		case CPUINFO_INT_ADDRBUS_WIDTH + ADDRESS_SPACE_PROGRAM: info->i = 32;					break;
		case CPUINFO_INT_ADDRBUS_SHIFT + ADDRESS_SPACE_PROGRAM: info->i = 0;					break;
		case CPUINFO_INT_DATABUS_WIDTH + ADDRESS_SPACE_DATA:	info->i = 0;					break;
		case CPUINFO_INT_ADDRBUS_WIDTH + ADDRESS_SPACE_DATA: 	info->i = 0;					break;
		case CPUINFO_INT_ADDRBUS_SHIFT + ADDRESS_SPACE_DATA: 	info->i = 0;					break;
		case CPUINFO_INT_DATABUS_WIDTH + ADDRESS_SPACE_IO:		info->i = 0;					break;
		case CPUINFO_INT_ADDRBUS_WIDTH + ADDRESS_SPACE_IO: 		info->i = 0;					break;
		case CPUINFO_INT_ADDRBUS_SHIFT + ADDRESS_SPACE_IO: 		info->i = 0;					break;

		case CPUINFO_INT_INPUT_STATE + MIPS_IRQ0:		info->i = (mipscpu.cp0r[ CP0_CAUSE ] & 0x400) ? ASSERT_LINE : CLEAR_LINE; break;
		case CPUINFO_INT_INPUT_STATE + MIPS_IRQ1:		info->i = (mipscpu.cp0r[ CP0_CAUSE ] & 0x800) ? ASSERT_LINE : CLEAR_LINE; break;
		case CPUINFO_INT_INPUT_STATE + MIPS_IRQ2:		info->i = (mipscpu.cp0r[ CP0_CAUSE ] & 0x1000) ? ASSERT_LINE : CLEAR_LINE; break;
		case CPUINFO_INT_INPUT_STATE + MIPS_IRQ3:		info->i = (mipscpu.cp0r[ CP0_CAUSE ] & 0x2000) ? ASSERT_LINE : CLEAR_LINE; break;
		case CPUINFO_INT_INPUT_STATE + MIPS_IRQ4:		info->i = (mipscpu.cp0r[ CP0_CAUSE ] & 0x4000) ? ASSERT_LINE : CLEAR_LINE; break;
		case CPUINFO_INT_INPUT_STATE + MIPS_IRQ5:		info->i = (mipscpu.cp0r[ CP0_CAUSE ] & 0x8000) ? ASSERT_LINE : CLEAR_LINE; break;

		case CPUINFO_INT_PREVIOUSPC:					/* not implemented */					break;

		case CPUINFO_INT_PC:							info->i = mipscpu.pc;					break;
		case CPUINFO_INT_REGISTER + MIPS_PC:			info->i = mipscpu.pc;					break;
		case CPUINFO_INT_SP:
			/* because there is no hardware stack and the pipeline causes the cpu to execute the
            instruction after a subroutine call before the subroutine is executed there is little
            chance of cmd_step_over() in mamedbg.c working. */
								info->i = 0;													break;
		case CPUINFO_INT_REGISTER + MIPS_DELAYV:		info->i = mipscpu.delayv;				break;
		case CPUINFO_INT_REGISTER + MIPS_DELAYR:		info->i = mipscpu.delayr;				break;
		case CPUINFO_INT_REGISTER + MIPS_HI:			info->i = mipscpu.hi;					break;
		case CPUINFO_INT_REGISTER + MIPS_LO:			info->i = mipscpu.lo;					break;
		case CPUINFO_INT_REGISTER + MIPS_R0:			info->i = mipscpu.r[ 0 ];				break;
		case CPUINFO_INT_REGISTER + MIPS_R1:			info->i = mipscpu.r[ 1 ];				break;
		case CPUINFO_INT_REGISTER + MIPS_R2:			info->i = mipscpu.r[ 2 ];				break;
		case CPUINFO_INT_REGISTER + MIPS_R3:			info->i = mipscpu.r[ 3 ];				break;
		case CPUINFO_INT_REGISTER + MIPS_R4:			info->i = mipscpu.r[ 4 ];				break;
		case CPUINFO_INT_REGISTER + MIPS_R5:			info->i = mipscpu.r[ 5 ];				break;
		case CPUINFO_INT_REGISTER + MIPS_R6:			info->i = mipscpu.r[ 6 ];				break;
		case CPUINFO_INT_REGISTER + MIPS_R7:			info->i = mipscpu.r[ 7 ];				break;
		case CPUINFO_INT_REGISTER + MIPS_R8:			info->i = mipscpu.r[ 8 ];				break;
		case CPUINFO_INT_REGISTER + MIPS_R9:			info->i = mipscpu.r[ 9 ];				break;
		case CPUINFO_INT_REGISTER + MIPS_R10:			info->i = mipscpu.r[ 10 ];				break;
		case CPUINFO_INT_REGISTER + MIPS_R11:			info->i = mipscpu.r[ 11 ];				break;
		case CPUINFO_INT_REGISTER + MIPS_R12:			info->i = mipscpu.r[ 12 ];				break;
		case CPUINFO_INT_REGISTER + MIPS_R13:			info->i = mipscpu.r[ 13 ];				break;
		case CPUINFO_INT_REGISTER + MIPS_R14:			info->i = mipscpu.r[ 14 ];				break;
		case CPUINFO_INT_REGISTER + MIPS_R15:			info->i = mipscpu.r[ 15 ];				break;
		case CPUINFO_INT_REGISTER + MIPS_R16:			info->i = mipscpu.r[ 16 ];				break;
		case CPUINFO_INT_REGISTER + MIPS_R17:			info->i = mipscpu.r[ 17 ];				break;
		case CPUINFO_INT_REGISTER + MIPS_R18:			info->i = mipscpu.r[ 18 ];				break;
		case CPUINFO_INT_REGISTER + MIPS_R19:			info->i = mipscpu.r[ 19 ];				break;
		case CPUINFO_INT_REGISTER + MIPS_R20:			info->i = mipscpu.r[ 20 ];				break;
		case CPUINFO_INT_REGISTER + MIPS_R21:			info->i = mipscpu.r[ 21 ];				break;
		case CPUINFO_INT_REGISTER + MIPS_R22:			info->i = mipscpu.r[ 22 ];				break;
		case CPUINFO_INT_REGISTER + MIPS_R23:			info->i = mipscpu.r[ 23 ];				break;
		case CPUINFO_INT_REGISTER + MIPS_R24:			info->i = mipscpu.r[ 24 ];				break;
		case CPUINFO_INT_REGISTER + MIPS_R25:			info->i = mipscpu.r[ 25 ];				break;
		case CPUINFO_INT_REGISTER + MIPS_R26:			info->i = mipscpu.r[ 26 ];				break;
		case CPUINFO_INT_REGISTER + MIPS_R27:			info->i = mipscpu.r[ 27 ];				break;
		case CPUINFO_INT_REGISTER + MIPS_R28:			info->i = mipscpu.r[ 28 ];				break;
		case CPUINFO_INT_REGISTER + MIPS_R29:			info->i = mipscpu.r[ 29 ];				break;
		case CPUINFO_INT_REGISTER + MIPS_R30:			info->i = mipscpu.r[ 30 ];				break;
		case CPUINFO_INT_REGISTER + MIPS_R31:			info->i = mipscpu.r[ 31 ];				break;
		case CPUINFO_INT_REGISTER + MIPS_CP0R0:			info->i = mipscpu.cp0r[ 0 ];			break;
		case CPUINFO_INT_REGISTER + MIPS_CP0R1:			info->i = mipscpu.cp0r[ 1 ];			break;
		case CPUINFO_INT_REGISTER + MIPS_CP0R2:			info->i = mipscpu.cp0r[ 2 ];			break;
		case CPUINFO_INT_REGISTER + MIPS_CP0R3:			info->i = mipscpu.cp0r[ 3 ];			break;
		case CPUINFO_INT_REGISTER + MIPS_CP0R4:			info->i = mipscpu.cp0r[ 4 ];			break;
		case CPUINFO_INT_REGISTER + MIPS_CP0R5:			info->i = mipscpu.cp0r[ 5 ];			break;
		case CPUINFO_INT_REGISTER + MIPS_CP0R6:			info->i = mipscpu.cp0r[ 6 ];			break;
		case CPUINFO_INT_REGISTER + MIPS_CP0R7:			info->i = mipscpu.cp0r[ 7 ];			break;
		case CPUINFO_INT_REGISTER + MIPS_CP0R8:			info->i = mipscpu.cp0r[ 8 ];			break;
		case CPUINFO_INT_REGISTER + MIPS_CP0R9:			info->i = mipscpu.cp0r[ 9 ];			break;
		case CPUINFO_INT_REGISTER + MIPS_CP0R10:		info->i = mipscpu.cp0r[ 10 ];			break;
		case CPUINFO_INT_REGISTER + MIPS_CP0R11:		info->i = mipscpu.cp0r[ 11 ];			break;
		case CPUINFO_INT_REGISTER + MIPS_CP0R12:		info->i = mipscpu.cp0r[ 12 ];			break;
		case CPUINFO_INT_REGISTER + MIPS_CP0R13:		info->i = mipscpu.cp0r[ 13 ];			break;
		case CPUINFO_INT_REGISTER + MIPS_CP0R14:		info->i = mipscpu.cp0r[ 14 ];			break;
		case CPUINFO_INT_REGISTER + MIPS_CP0R15:		info->i = mipscpu.cp0r[ 15 ];			break;
		case CPUINFO_INT_REGISTER + MIPS_CP0R16:		info->i = mipscpu.cp0r[ 16 ];			break;
		case CPUINFO_INT_REGISTER + MIPS_CP0R17:		info->i = mipscpu.cp0r[ 17 ];			break;
		case CPUINFO_INT_REGISTER + MIPS_CP0R18:		info->i = mipscpu.cp0r[ 18 ];			break;
		case CPUINFO_INT_REGISTER + MIPS_CP0R19:		info->i = mipscpu.cp0r[ 19 ];			break;
		case CPUINFO_INT_REGISTER + MIPS_CP0R20:		info->i = mipscpu.cp0r[ 20 ];			break;
		case CPUINFO_INT_REGISTER + MIPS_CP0R21:		info->i = mipscpu.cp0r[ 21 ];			break;
		case CPUINFO_INT_REGISTER + MIPS_CP0R22:		info->i = mipscpu.cp0r[ 22 ];			break;
		case CPUINFO_INT_REGISTER + MIPS_CP0R23:		info->i = mipscpu.cp0r[ 23 ];			break;
		case CPUINFO_INT_REGISTER + MIPS_CP0R24:		info->i = mipscpu.cp0r[ 24 ];			break;
		case CPUINFO_INT_REGISTER + MIPS_CP0R25:		info->i = mipscpu.cp0r[ 25 ];			break;
		case CPUINFO_INT_REGISTER + MIPS_CP0R26:		info->i = mipscpu.cp0r[ 26 ];			break;
		case CPUINFO_INT_REGISTER + MIPS_CP0R27:		info->i = mipscpu.cp0r[ 27 ];			break;
		case CPUINFO_INT_REGISTER + MIPS_CP0R28:		info->i = mipscpu.cp0r[ 28 ];			break;
		case CPUINFO_INT_REGISTER + MIPS_CP0R29:		info->i = mipscpu.cp0r[ 29 ];			break;
		case CPUINFO_INT_REGISTER + MIPS_CP0R30:		info->i = mipscpu.cp0r[ 30 ];			break;
		case CPUINFO_INT_REGISTER + MIPS_CP0R31:		info->i = mipscpu.cp0r[ 31 ];			break;
		case CPUINFO_INT_REGISTER + MIPS_CP2DR0:		info->i = mipscpu.cp2dr[ 0 ].d;			break;
		case CPUINFO_INT_REGISTER + MIPS_CP2DR1:		info->i = mipscpu.cp2dr[ 1 ].d;			break;
		case CPUINFO_INT_REGISTER + MIPS_CP2DR2:		info->i = mipscpu.cp2dr[ 2 ].d;			break;
		case CPUINFO_INT_REGISTER + MIPS_CP2DR3:		info->i = mipscpu.cp2dr[ 3 ].d;			break;
		case CPUINFO_INT_REGISTER + MIPS_CP2DR4:		info->i = mipscpu.cp2dr[ 4 ].d;			break;
		case CPUINFO_INT_REGISTER + MIPS_CP2DR5:		info->i = mipscpu.cp2dr[ 5 ].d;			break;
		case CPUINFO_INT_REGISTER + MIPS_CP2DR6:		info->i = mipscpu.cp2dr[ 6 ].d;			break;
		case CPUINFO_INT_REGISTER + MIPS_CP2DR7:		info->i = mipscpu.cp2dr[ 7 ].d;			break;
		case CPUINFO_INT_REGISTER + MIPS_CP2DR8:		info->i = mipscpu.cp2dr[ 8 ].d;			break;
		case CPUINFO_INT_REGISTER + MIPS_CP2DR9:		info->i = mipscpu.cp2dr[ 9 ].d;			break;
		case CPUINFO_INT_REGISTER + MIPS_CP2DR10:		info->i = mipscpu.cp2dr[ 10 ].d;		break;
		case CPUINFO_INT_REGISTER + MIPS_CP2DR11:		info->i = mipscpu.cp2dr[ 11 ].d;		break;
		case CPUINFO_INT_REGISTER + MIPS_CP2DR12:		info->i = mipscpu.cp2dr[ 12 ].d;		break;
		case CPUINFO_INT_REGISTER + MIPS_CP2DR13:		info->i = mipscpu.cp2dr[ 13 ].d;		break;
		case CPUINFO_INT_REGISTER + MIPS_CP2DR14:		info->i = mipscpu.cp2dr[ 14 ].d;		break;
		case CPUINFO_INT_REGISTER + MIPS_CP2DR15:		info->i = mipscpu.cp2dr[ 15 ].d;		break;
		case CPUINFO_INT_REGISTER + MIPS_CP2DR16:		info->i = mipscpu.cp2dr[ 16 ].d;		break;
		case CPUINFO_INT_REGISTER + MIPS_CP2DR17:		info->i = mipscpu.cp2dr[ 17 ].d;		break;
		case CPUINFO_INT_REGISTER + MIPS_CP2DR18:		info->i = mipscpu.cp2dr[ 18 ].d;		break;
		case CPUINFO_INT_REGISTER + MIPS_CP2DR19:		info->i = mipscpu.cp2dr[ 19 ].d;		break;
		case CPUINFO_INT_REGISTER + MIPS_CP2DR20:		info->i = mipscpu.cp2dr[ 20 ].d;		break;
		case CPUINFO_INT_REGISTER + MIPS_CP2DR21:		info->i = mipscpu.cp2dr[ 21 ].d;		break;
		case CPUINFO_INT_REGISTER + MIPS_CP2DR22:		info->i = mipscpu.cp2dr[ 22 ].d;		break;
		case CPUINFO_INT_REGISTER + MIPS_CP2DR23:		info->i = mipscpu.cp2dr[ 23 ].d;		break;
		case CPUINFO_INT_REGISTER + MIPS_CP2DR24:		info->i = mipscpu.cp2dr[ 24 ].d;		break;
		case CPUINFO_INT_REGISTER + MIPS_CP2DR25:		info->i = mipscpu.cp2dr[ 25 ].d;		break;
		case CPUINFO_INT_REGISTER + MIPS_CP2DR26:		info->i = mipscpu.cp2dr[ 26 ].d;		break;
		case CPUINFO_INT_REGISTER + MIPS_CP2DR27:		info->i = mipscpu.cp2dr[ 27 ].d;		break;
		case CPUINFO_INT_REGISTER + MIPS_CP2DR28:		info->i = mipscpu.cp2dr[ 28 ].d;		break;
		case CPUINFO_INT_REGISTER + MIPS_CP2DR29:		info->i = mipscpu.cp2dr[ 29 ].d;		break;
		case CPUINFO_INT_REGISTER + MIPS_CP2DR30:		info->i = mipscpu.cp2dr[ 30 ].d;		break;
		case CPUINFO_INT_REGISTER + MIPS_CP2DR31:		info->i = mipscpu.cp2dr[ 31 ].d;		break;
		case CPUINFO_INT_REGISTER + MIPS_CP2CR0:		info->i = mipscpu.cp2cr[ 0 ].d;			break;
		case CPUINFO_INT_REGISTER + MIPS_CP2CR1:		info->i = mipscpu.cp2cr[ 1 ].d;			break;
		case CPUINFO_INT_REGISTER + MIPS_CP2CR2:		info->i = mipscpu.cp2cr[ 2 ].d;			break;
		case CPUINFO_INT_REGISTER + MIPS_CP2CR3:		info->i = mipscpu.cp2cr[ 3 ].d;			break;
		case CPUINFO_INT_REGISTER + MIPS_CP2CR4:		info->i = mipscpu.cp2cr[ 4 ].d;			break;
		case CPUINFO_INT_REGISTER + MIPS_CP2CR5:		info->i = mipscpu.cp2cr[ 5 ].d;			break;
		case CPUINFO_INT_REGISTER + MIPS_CP2CR6:		info->i = mipscpu.cp2cr[ 6 ].d;			break;
		case CPUINFO_INT_REGISTER + MIPS_CP2CR7:		info->i = mipscpu.cp2cr[ 7 ].d;			break;
		case CPUINFO_INT_REGISTER + MIPS_CP2CR8:		info->i = mipscpu.cp2cr[ 8 ].d;			break;
		case CPUINFO_INT_REGISTER + MIPS_CP2CR9:		info->i = mipscpu.cp2cr[ 9 ].d;			break;
		case CPUINFO_INT_REGISTER + MIPS_CP2CR10:		info->i = mipscpu.cp2cr[ 10 ].d;		break;
		case CPUINFO_INT_REGISTER + MIPS_CP2CR11:		info->i = mipscpu.cp2cr[ 11 ].d;		break;
		case CPUINFO_INT_REGISTER + MIPS_CP2CR12:		info->i = mipscpu.cp2cr[ 12 ].d;		break;
		case CPUINFO_INT_REGISTER + MIPS_CP2CR13:		info->i = mipscpu.cp2cr[ 13 ].d;		break;
		case CPUINFO_INT_REGISTER + MIPS_CP2CR14:		info->i = mipscpu.cp2cr[ 14 ].d;		break;
		case CPUINFO_INT_REGISTER + MIPS_CP2CR15:		info->i = mipscpu.cp2cr[ 15 ].d;		break;
		case CPUINFO_INT_REGISTER + MIPS_CP2CR16:		info->i = mipscpu.cp2cr[ 16 ].d;		break;
		case CPUINFO_INT_REGISTER + MIPS_CP2CR17:		info->i = mipscpu.cp2cr[ 17 ].d;		break;
		case CPUINFO_INT_REGISTER + MIPS_CP2CR18:		info->i = mipscpu.cp2cr[ 18 ].d;		break;
		case CPUINFO_INT_REGISTER + MIPS_CP2CR19:		info->i = mipscpu.cp2cr[ 19 ].d;		break;
		case CPUINFO_INT_REGISTER + MIPS_CP2CR20:		info->i = mipscpu.cp2cr[ 20 ].d;		break;
		case CPUINFO_INT_REGISTER + MIPS_CP2CR21:		info->i = mipscpu.cp2cr[ 21 ].d;		break;
		case CPUINFO_INT_REGISTER + MIPS_CP2CR22:		info->i = mipscpu.cp2cr[ 22 ].d;		break;
		case CPUINFO_INT_REGISTER + MIPS_CP2CR23:		info->i = mipscpu.cp2cr[ 23 ].d;		break;
		case CPUINFO_INT_REGISTER + MIPS_CP2CR24:		info->i = mipscpu.cp2cr[ 24 ].d;		break;
		case CPUINFO_INT_REGISTER + MIPS_CP2CR25:		info->i = mipscpu.cp2cr[ 25 ].d;		break;
		case CPUINFO_INT_REGISTER + MIPS_CP2CR26:		info->i = mipscpu.cp2cr[ 26 ].d;		break;
		case CPUINFO_INT_REGISTER + MIPS_CP2CR27:		info->i = mipscpu.cp2cr[ 27 ].d;		break;
		case CPUINFO_INT_REGISTER + MIPS_CP2CR28:		info->i = mipscpu.cp2cr[ 28 ].d;		break;
		case CPUINFO_INT_REGISTER + MIPS_CP2CR29:		info->i = mipscpu.cp2cr[ 29 ].d;		break;
		case CPUINFO_INT_REGISTER + MIPS_CP2CR30:		info->i = mipscpu.cp2cr[ 30 ].d;		break;
		case CPUINFO_INT_REGISTER + MIPS_CP2CR31:		info->i = mipscpu.cp2cr[ 31 ].d;		break;

		/* --- the following bits of info are returned as pointers to data or functions --- */
		case CPUINFO_PTR_SET_INFO:						info->setinfo = mips_set_info;			break;
		case CPUINFO_PTR_GET_CONTEXT:					info->getcontext = mips_get_context;	break;
		case CPUINFO_PTR_SET_CONTEXT:					info->setcontext = mips_set_context;	break;
		case CPUINFO_PTR_INIT:							info->init = mips_init;					break;
		case CPUINFO_PTR_RESET:							info->reset = mips_reset;				break;
		case CPUINFO_PTR_EXIT:							info->exit = mips_exit;					break;
		case CPUINFO_PTR_EXECUTE:						info->execute = mips_execute;			break;
		case CPUINFO_PTR_BURN:							info->burn = NULL;						break;
#ifdef MAME_DEBUG
		case CPUINFO_PTR_DISASSEMBLE:					info->disassemble = mips_dasm;			break;
#endif /* MAME_DEBUG */
		case CPUINFO_PTR_INSTRUCTION_COUNTER:			info->icount = &mips_ICount;			break;

		/* --- the following bits of info are returned as NULL-terminated strings --- */
		case CPUINFO_STR_NAME:							strcpy(info->s, "PSX CPU");				break;
		case CPUINFO_STR_CORE_FAMILY:					strcpy(info->s, "mipscpu");				break;
		case CPUINFO_STR_CORE_VERSION:					strcpy(info->s, "1.5");					break;
		case CPUINFO_STR_CORE_FILE:						strcpy(info->s, __FILE__);				break;
		case CPUINFO_STR_CORE_CREDITS:					strcpy(info->s, "Copyright 2005 smf");	break;

		case CPUINFO_STR_FLAGS:							strcpy(info->s, " ");					break;

		case CPUINFO_STR_REGISTER + MIPS_PC:			sprintf( info->s, "pc      :%08x", mipscpu.pc ); break;
		case CPUINFO_STR_REGISTER + MIPS_DELAYV:		sprintf( info->s, "delay   :%08x", mipscpu.delayv ); break;
		case CPUINFO_STR_REGISTER + MIPS_DELAYR:		sprintf( info->s, "delay %s:%02x", delayn[ mipscpu.delayr ], mipscpu.delayr ); break;
		case CPUINFO_STR_REGISTER + MIPS_HI:			sprintf( info->s, "hi      :%08x", mipscpu.hi ); break;
		case CPUINFO_STR_REGISTER + MIPS_LO:			sprintf( info->s, "lo      :%08x", mipscpu.lo ); break;
		case CPUINFO_STR_REGISTER + MIPS_R0:			sprintf( info->s, "zero    :%08x", mipscpu.r[ 0 ] ); break;
		case CPUINFO_STR_REGISTER + MIPS_R1:			sprintf( info->s, "at      :%08x", mipscpu.r[ 1 ] ); break;
		case CPUINFO_STR_REGISTER + MIPS_R2:			sprintf( info->s, "v0      :%08x", mipscpu.r[ 2 ] ); break;
		case CPUINFO_STR_REGISTER + MIPS_R3:			sprintf( info->s, "v1      :%08x", mipscpu.r[ 3 ] ); break;
		case CPUINFO_STR_REGISTER + MIPS_R4:			sprintf( info->s, "a0      :%08x", mipscpu.r[ 4 ] ); break;
		case CPUINFO_STR_REGISTER + MIPS_R5:			sprintf( info->s, "a1      :%08x", mipscpu.r[ 5 ] ); break;
		case CPUINFO_STR_REGISTER + MIPS_R6:			sprintf( info->s, "a2      :%08x", mipscpu.r[ 6 ] ); break;
		case CPUINFO_STR_REGISTER + MIPS_R7:			sprintf( info->s, "a3      :%08x", mipscpu.r[ 7 ] ); break;
		case CPUINFO_STR_REGISTER + MIPS_R8:			sprintf( info->s, "t0      :%08x", mipscpu.r[ 8 ] ); break;
		case CPUINFO_STR_REGISTER + MIPS_R9:			sprintf( info->s, "t1      :%08x", mipscpu.r[ 9 ] ); break;
		case CPUINFO_STR_REGISTER + MIPS_R10:			sprintf( info->s, "t2      :%08x", mipscpu.r[ 10 ] ); break;
		case CPUINFO_STR_REGISTER + MIPS_R11:			sprintf( info->s, "t3      :%08x", mipscpu.r[ 11 ] ); break;
		case CPUINFO_STR_REGISTER + MIPS_R12:			sprintf( info->s, "t4      :%08x", mipscpu.r[ 12 ] ); break;
		case CPUINFO_STR_REGISTER + MIPS_R13:			sprintf( info->s, "t5      :%08x", mipscpu.r[ 13 ] ); break;
		case CPUINFO_STR_REGISTER + MIPS_R14:			sprintf( info->s, "t6      :%08x", mipscpu.r[ 14 ] ); break;
		case CPUINFO_STR_REGISTER + MIPS_R15:			sprintf( info->s, "t7      :%08x", mipscpu.r[ 15 ] ); break;
		case CPUINFO_STR_REGISTER + MIPS_R16:			sprintf( info->s, "s0      :%08x", mipscpu.r[ 16 ] ); break;
		case CPUINFO_STR_REGISTER + MIPS_R17:			sprintf( info->s, "s1      :%08x", mipscpu.r[ 17 ] ); break;
		case CPUINFO_STR_REGISTER + MIPS_R18:			sprintf( info->s, "s2      :%08x", mipscpu.r[ 18 ] ); break;
		case CPUINFO_STR_REGISTER + MIPS_R19:			sprintf( info->s, "s3      :%08x", mipscpu.r[ 19 ] ); break;
		case CPUINFO_STR_REGISTER + MIPS_R20:			sprintf( info->s, "s4      :%08x", mipscpu.r[ 20 ] ); break;
		case CPUINFO_STR_REGISTER + MIPS_R21:			sprintf( info->s, "s5      :%08x", mipscpu.r[ 21 ] ); break;
		case CPUINFO_STR_REGISTER + MIPS_R22:			sprintf( info->s, "s6      :%08x", mipscpu.r[ 22 ] ); break;
		case CPUINFO_STR_REGISTER + MIPS_R23:			sprintf( info->s, "s7      :%08x", mipscpu.r[ 23 ] ); break;
		case CPUINFO_STR_REGISTER + MIPS_R24:			sprintf( info->s, "t8      :%08x", mipscpu.r[ 24 ] ); break;
		case CPUINFO_STR_REGISTER + MIPS_R25:			sprintf( info->s, "t9      :%08x", mipscpu.r[ 25 ] ); break;
		case CPUINFO_STR_REGISTER + MIPS_R26:			sprintf( info->s, "k0      :%08x", mipscpu.r[ 26 ] ); break;
		case CPUINFO_STR_REGISTER + MIPS_R27:			sprintf( info->s, "k1      :%08x", mipscpu.r[ 27 ] ); break;
		case CPUINFO_STR_REGISTER + MIPS_R28:			sprintf( info->s, "gp      :%08x", mipscpu.r[ 28 ] ); break;
		case CPUINFO_STR_REGISTER + MIPS_R29:			sprintf( info->s, "sp      :%08x", mipscpu.r[ 29 ] ); break;
		case CPUINFO_STR_REGISTER + MIPS_R30:			sprintf( info->s, "fp      :%08x", mipscpu.r[ 30 ] ); break;
		case CPUINFO_STR_REGISTER + MIPS_R31:			sprintf( info->s, "ra      :%08x", mipscpu.r[ 31 ] ); break;
		case CPUINFO_STR_REGISTER + MIPS_CP0R0:			sprintf( info->s, "Index   :%08x", mipscpu.cp0r[ 0 ] ); break;
		case CPUINFO_STR_REGISTER + MIPS_CP0R1:			sprintf( info->s, "Random  :%08x", mipscpu.cp0r[ 1 ] ); break;
		case CPUINFO_STR_REGISTER + MIPS_CP0R2:			sprintf( info->s, "EntryLo :%08x", mipscpu.cp0r[ 2 ] ); break;
		case CPUINFO_STR_REGISTER + MIPS_CP0R3:			sprintf( info->s, "cp0r3   :%08x", mipscpu.cp0r[ 3 ] ); break;
		case CPUINFO_STR_REGISTER + MIPS_CP0R4:			sprintf( info->s, "Context :%08x", mipscpu.cp0r[ 4 ] ); break;
		case CPUINFO_STR_REGISTER + MIPS_CP0R5:			sprintf( info->s, "cp0r5   :%08x", mipscpu.cp0r[ 5 ] ); break;
		case CPUINFO_STR_REGISTER + MIPS_CP0R6:			sprintf( info->s, "cp0r6   :%08x", mipscpu.cp0r[ 6 ] ); break;
		case CPUINFO_STR_REGISTER + MIPS_CP0R7:			sprintf( info->s, "cp0r7   :%08x", mipscpu.cp0r[ 7 ] ); break;
		case CPUINFO_STR_REGISTER + MIPS_CP0R8:			sprintf( info->s, "BadVAddr:%08x", mipscpu.cp0r[ 8 ] ); break;
		case CPUINFO_STR_REGISTER + MIPS_CP0R9:			sprintf( info->s, "cp0r9   :%08x", mipscpu.cp0r[ 9 ] ); break;
		case CPUINFO_STR_REGISTER + MIPS_CP0R10:		sprintf( info->s, "EntryHi :%08x", mipscpu.cp0r[ 10 ] ); break;
		case CPUINFO_STR_REGISTER + MIPS_CP0R11:		sprintf( info->s, "cp0r11  :%08x", mipscpu.cp0r[ 11 ] ); break;
		case CPUINFO_STR_REGISTER + MIPS_CP0R12:		sprintf( info->s, "SR      :%08x", mipscpu.cp0r[ 12 ] ); break;
		case CPUINFO_STR_REGISTER + MIPS_CP0R13:		sprintf( info->s, "Cause   :%08x", mipscpu.cp0r[ 13 ] ); break;
		case CPUINFO_STR_REGISTER + MIPS_CP0R14:		sprintf( info->s, "EPC     :%08x", mipscpu.cp0r[ 14 ] ); break;
		case CPUINFO_STR_REGISTER + MIPS_CP0R15:		sprintf( info->s, "PRId    :%08x", mipscpu.cp0r[ 15 ] ); break;
		case CPUINFO_STR_REGISTER + MIPS_CP0R16:		sprintf( info->s, "cp0r16  :%08x", mipscpu.cp0r[ 16 ] ); break;
		case CPUINFO_STR_REGISTER + MIPS_CP0R17:		sprintf( info->s, "cp0r17  :%08x", mipscpu.cp0r[ 17 ] ); break;
		case CPUINFO_STR_REGISTER + MIPS_CP0R18:		sprintf( info->s, "cp0r18  :%08x", mipscpu.cp0r[ 18 ] ); break;
		case CPUINFO_STR_REGISTER + MIPS_CP0R19:		sprintf( info->s, "cp0r19  :%08x", mipscpu.cp0r[ 19 ] ); break;
		case CPUINFO_STR_REGISTER + MIPS_CP0R20:		sprintf( info->s, "cp0r20  :%08x", mipscpu.cp0r[ 20 ] ); break;
		case CPUINFO_STR_REGISTER + MIPS_CP0R21:		sprintf( info->s, "cp0r21  :%08x", mipscpu.cp0r[ 21 ] ); break;
		case CPUINFO_STR_REGISTER + MIPS_CP0R22:		sprintf( info->s, "cp0r22  :%08x", mipscpu.cp0r[ 22 ] ); break;
		case CPUINFO_STR_REGISTER + MIPS_CP0R23:		sprintf( info->s, "cp0r23  :%08x", mipscpu.cp0r[ 23 ] ); break;
		case CPUINFO_STR_REGISTER + MIPS_CP0R24:		sprintf( info->s, "cp0r24  :%08x", mipscpu.cp0r[ 24 ] ); break;
		case CPUINFO_STR_REGISTER + MIPS_CP0R25:		sprintf( info->s, "cp0r25  :%08x", mipscpu.cp0r[ 25 ] ); break;
		case CPUINFO_STR_REGISTER + MIPS_CP0R26:		sprintf( info->s, "cp0r26  :%08x", mipscpu.cp0r[ 26 ] ); break;
		case CPUINFO_STR_REGISTER + MIPS_CP0R27:		sprintf( info->s, "cp0r27  :%08x", mipscpu.cp0r[ 27 ] ); break;
		case CPUINFO_STR_REGISTER + MIPS_CP0R28:		sprintf( info->s, "cp0r28  :%08x", mipscpu.cp0r[ 28 ] ); break;
		case CPUINFO_STR_REGISTER + MIPS_CP0R29:		sprintf( info->s, "cp0r29  :%08x", mipscpu.cp0r[ 29 ] ); break;
		case CPUINFO_STR_REGISTER + MIPS_CP0R30:		sprintf( info->s, "cp0r30  :%08x", mipscpu.cp0r[ 30 ] ); break;
		case CPUINFO_STR_REGISTER + MIPS_CP0R31:		sprintf( info->s, "cp0r31  :%08x", mipscpu.cp0r[ 31 ] ); break;
		case CPUINFO_STR_REGISTER + MIPS_CP2DR0:		sprintf( info->s, "vxy0    :%08x", mipscpu.cp2dr[ 0 ].d ); break;
		case CPUINFO_STR_REGISTER + MIPS_CP2DR1:		sprintf( info->s, "vz0     :%08x", mipscpu.cp2dr[ 1 ].d ); break;
		case CPUINFO_STR_REGISTER + MIPS_CP2DR2:		sprintf( info->s, "vxy1    :%08x", mipscpu.cp2dr[ 2 ].d ); break;
		case CPUINFO_STR_REGISTER + MIPS_CP2DR3:		sprintf( info->s, "vz1     :%08x", mipscpu.cp2dr[ 3 ].d ); break;
		case CPUINFO_STR_REGISTER + MIPS_CP2DR4:		sprintf( info->s, "vxy2    :%08x", mipscpu.cp2dr[ 4 ].d ); break;
		case CPUINFO_STR_REGISTER + MIPS_CP2DR5:		sprintf( info->s, "vz2     :%08x", mipscpu.cp2dr[ 5 ].d ); break;
		case CPUINFO_STR_REGISTER + MIPS_CP2DR6:		sprintf( info->s, "rgb     :%08x", mipscpu.cp2dr[ 6 ].d ); break;
		case CPUINFO_STR_REGISTER + MIPS_CP2DR7:		sprintf( info->s, "otz     :%08x", mipscpu.cp2dr[ 7 ].d ); break;
		case CPUINFO_STR_REGISTER + MIPS_CP2DR8:		sprintf( info->s, "ir0     :%08x", mipscpu.cp2dr[ 8 ].d ); break;
		case CPUINFO_STR_REGISTER + MIPS_CP2DR9:		sprintf( info->s, "ir1     :%08x", mipscpu.cp2dr[ 9 ].d ); break;
		case CPUINFO_STR_REGISTER + MIPS_CP2DR10:		sprintf( info->s, "ir2     :%08x", mipscpu.cp2dr[ 10 ].d ); break;
		case CPUINFO_STR_REGISTER + MIPS_CP2DR11:		sprintf( info->s, "ir3     :%08x", mipscpu.cp2dr[ 11 ].d ); break;
		case CPUINFO_STR_REGISTER + MIPS_CP2DR12:		sprintf( info->s, "sxy0    :%08x", mipscpu.cp2dr[ 12 ].d ); break;
		case CPUINFO_STR_REGISTER + MIPS_CP2DR13:		sprintf( info->s, "sxy1    :%08x", mipscpu.cp2dr[ 13 ].d ); break;
		case CPUINFO_STR_REGISTER + MIPS_CP2DR14:		sprintf( info->s, "sxy2    :%08x", mipscpu.cp2dr[ 14 ].d ); break;
		case CPUINFO_STR_REGISTER + MIPS_CP2DR15:		sprintf( info->s, "sxyp    :%08x", mipscpu.cp2dr[ 15 ].d ); break;
		case CPUINFO_STR_REGISTER + MIPS_CP2DR16:		sprintf( info->s, "sz0     :%08x", mipscpu.cp2dr[ 16 ].d ); break;
		case CPUINFO_STR_REGISTER + MIPS_CP2DR17:		sprintf( info->s, "sz1     :%08x", mipscpu.cp2dr[ 17 ].d ); break;
		case CPUINFO_STR_REGISTER + MIPS_CP2DR18:		sprintf( info->s, "sz2     :%08x", mipscpu.cp2dr[ 18 ].d ); break;
		case CPUINFO_STR_REGISTER + MIPS_CP2DR19:		sprintf( info->s, "sz3     :%08x", mipscpu.cp2dr[ 19 ].d ); break;
		case CPUINFO_STR_REGISTER + MIPS_CP2DR20:		sprintf( info->s, "rgb0    :%08x", mipscpu.cp2dr[ 20 ].d ); break;
		case CPUINFO_STR_REGISTER + MIPS_CP2DR21:		sprintf( info->s, "rgb1    :%08x", mipscpu.cp2dr[ 21 ].d ); break;
		case CPUINFO_STR_REGISTER + MIPS_CP2DR22:		sprintf( info->s, "rgb2    :%08x", mipscpu.cp2dr[ 22 ].d ); break;
		case CPUINFO_STR_REGISTER + MIPS_CP2DR23:		sprintf( info->s, "res1    :%08x", mipscpu.cp2dr[ 23 ].d ); break;
		case CPUINFO_STR_REGISTER + MIPS_CP2DR24:		sprintf( info->s, "mac0    :%08x", mipscpu.cp2dr[ 24 ].d ); break;
		case CPUINFO_STR_REGISTER + MIPS_CP2DR25:		sprintf( info->s, "mac1    :%08x", mipscpu.cp2dr[ 25 ].d ); break;
		case CPUINFO_STR_REGISTER + MIPS_CP2DR26:		sprintf( info->s, "mac2    :%08x", mipscpu.cp2dr[ 26 ].d ); break;
		case CPUINFO_STR_REGISTER + MIPS_CP2DR27:		sprintf( info->s, "mac3    :%08x", mipscpu.cp2dr[ 27 ].d ); break;
		case CPUINFO_STR_REGISTER + MIPS_CP2DR28:		sprintf( info->s, "irgb    :%08x", mipscpu.cp2dr[ 28 ].d ); break;
		case CPUINFO_STR_REGISTER + MIPS_CP2DR29:		sprintf( info->s, "orgb    :%08x", mipscpu.cp2dr[ 29 ].d ); break;
		case CPUINFO_STR_REGISTER + MIPS_CP2DR30:		sprintf( info->s, "lzcs    :%08x", mipscpu.cp2dr[ 30 ].d ); break;
		case CPUINFO_STR_REGISTER + MIPS_CP2DR31:		sprintf( info->s, "lzcr    :%08x", mipscpu.cp2dr[ 31 ].d ); break;
		case CPUINFO_STR_REGISTER + MIPS_CP2CR0:		sprintf( info->s, "r11r12  :%08x", mipscpu.cp2cr[ 0 ].d ); break;
		case CPUINFO_STR_REGISTER + MIPS_CP2CR1:		sprintf( info->s, "r13r21  :%08x", mipscpu.cp2cr[ 1 ].d ); break;
		case CPUINFO_STR_REGISTER + MIPS_CP2CR2:		sprintf( info->s, "r22r23  :%08x", mipscpu.cp2cr[ 2 ].d ); break;
		case CPUINFO_STR_REGISTER + MIPS_CP2CR3:		sprintf( info->s, "r31r32  :%08x", mipscpu.cp2cr[ 3 ].d ); break;
		case CPUINFO_STR_REGISTER + MIPS_CP2CR4:		sprintf( info->s, "r33     :%08x", mipscpu.cp2cr[ 4 ].d ); break;
		case CPUINFO_STR_REGISTER + MIPS_CP2CR5:		sprintf( info->s, "trx     :%08x", mipscpu.cp2cr[ 5 ].d ); break;
		case CPUINFO_STR_REGISTER + MIPS_CP2CR6:		sprintf( info->s, "try     :%08x", mipscpu.cp2cr[ 6 ].d ); break;
		case CPUINFO_STR_REGISTER + MIPS_CP2CR7:		sprintf( info->s, "trz     :%08x", mipscpu.cp2cr[ 7 ].d ); break;
		case CPUINFO_STR_REGISTER + MIPS_CP2CR8:		sprintf( info->s, "l11l12  :%08x", mipscpu.cp2cr[ 8 ].d ); break;
		case CPUINFO_STR_REGISTER + MIPS_CP2CR9:		sprintf( info->s, "l13l21  :%08x", mipscpu.cp2cr[ 9 ].d ); break;
		case CPUINFO_STR_REGISTER + MIPS_CP2CR10:		sprintf( info->s, "l22l23  :%08x", mipscpu.cp2cr[ 10 ].d ); break;
		case CPUINFO_STR_REGISTER + MIPS_CP2CR11:		sprintf( info->s, "l31l32  :%08x", mipscpu.cp2cr[ 11 ].d ); break;
		case CPUINFO_STR_REGISTER + MIPS_CP2CR12:		sprintf( info->s, "l33     :%08x", mipscpu.cp2cr[ 12 ].d ); break;
		case CPUINFO_STR_REGISTER + MIPS_CP2CR13:		sprintf( info->s, "rbk     :%08x", mipscpu.cp2cr[ 13 ].d ); break;
		case CPUINFO_STR_REGISTER + MIPS_CP2CR14:		sprintf( info->s, "gbk     :%08x", mipscpu.cp2cr[ 14 ].d ); break;
		case CPUINFO_STR_REGISTER + MIPS_CP2CR15:		sprintf( info->s, "bbk     :%08x", mipscpu.cp2cr[ 15 ].d ); break;
		case CPUINFO_STR_REGISTER + MIPS_CP2CR16:		sprintf( info->s, "lr1lr2  :%08x", mipscpu.cp2cr[ 16 ].d ); break;
		case CPUINFO_STR_REGISTER + MIPS_CP2CR17:		sprintf( info->s, "lr31g1  :%08x", mipscpu.cp2cr[ 17 ].d ); break;
		case CPUINFO_STR_REGISTER + MIPS_CP2CR18:		sprintf( info->s, "lg2lg3  :%08x", mipscpu.cp2cr[ 18 ].d ); break;
		case CPUINFO_STR_REGISTER + MIPS_CP2CR19:		sprintf( info->s, "lb1lb2  :%08x", mipscpu.cp2cr[ 19 ].d ); break;
		case CPUINFO_STR_REGISTER + MIPS_CP2CR20:		sprintf( info->s, "lb3     :%08x", mipscpu.cp2cr[ 20 ].d ); break;
		case CPUINFO_STR_REGISTER + MIPS_CP2CR21:		sprintf( info->s, "rfc     :%08x", mipscpu.cp2cr[ 21 ].d ); break;
		case CPUINFO_STR_REGISTER + MIPS_CP2CR22:		sprintf( info->s, "gfc     :%08x", mipscpu.cp2cr[ 22 ].d ); break;
		case CPUINFO_STR_REGISTER + MIPS_CP2CR23:		sprintf( info->s, "bfc     :%08x", mipscpu.cp2cr[ 23 ].d ); break;
		case CPUINFO_STR_REGISTER + MIPS_CP2CR24:		sprintf( info->s, "ofx     :%08x", mipscpu.cp2cr[ 24 ].d ); break;
		case CPUINFO_STR_REGISTER + MIPS_CP2CR25:		sprintf( info->s, "ofy     :%08x", mipscpu.cp2cr[ 25 ].d ); break;
		case CPUINFO_STR_REGISTER + MIPS_CP2CR26:		sprintf( info->s, "h       :%08x", mipscpu.cp2cr[ 26 ].d ); break;
		case CPUINFO_STR_REGISTER + MIPS_CP2CR27:		sprintf( info->s, "dqa     :%08x", mipscpu.cp2cr[ 27 ].d ); break;
		case CPUINFO_STR_REGISTER + MIPS_CP2CR28:		sprintf( info->s, "dqb     :%08x", mipscpu.cp2cr[ 28 ].d ); break;
		case CPUINFO_STR_REGISTER + MIPS_CP2CR29:		sprintf( info->s, "zsf3    :%08x", mipscpu.cp2cr[ 29 ].d ); break;
		case CPUINFO_STR_REGISTER + MIPS_CP2CR30:		sprintf( info->s, "zsf4    :%08x", mipscpu.cp2cr[ 30 ].d ); break;
		case CPUINFO_STR_REGISTER + MIPS_CP2CR31:		sprintf( info->s, "flag    :%08x", mipscpu.cp2cr[ 31 ].d ); break;
	}
}


#if (HAS_PSXCPU)
/**************************************************************************
 * CPU-specific set_info
 **************************************************************************/

void psxcpu_get_info(UINT32 state, cpuinfo *info)
{
	switch (state)
	{
		/* --- the following bits of info are returned as NULL-terminated strings --- */
		case CPUINFO_STR_NAME:							strcpy(info->s, "PSX CPU"); break;

		default:
			mips_get_info(state, info);
			break;
	}
}
#endif
