// license:BSD-3-Clause
// copyright-holders:smf
/*
 * PlayStation CPU emulator
 *
 * Copyright 2003-2013 smf
 *
 * Known chip id's
 *   CXD8530AQ
 *   CXD8530BQ
 *   CXD8530CQ
 *   CXD8661R
 *   CXD8606BQ
 *   CXD8606CQ
 *
 * The PlayStation CPU is based on the LSI LR33300.
 *
 * Differences from the LR33300:
 *
 *  There is only 1k of data cache ram ( the LR33300 has 2k )
 *
 *  There is no data cache tag ram, so the data cache ram can only be used as a fast area
 *  of ram ( which is a standard LR33300 feature ).
 *
 *  If COP0 is disabled in user mode you get a coprocessor unusable exception, while
 *  the LR33300 is documented to generate a reserved instruction exception.
 *
 * Known limitations of the emulation:
 *
 *  Only read & write break points are emulated, trace and program counter breakpoints are not.
 *
 *  Load/Store timings are based on load scheduling turned off & no write cache. This affects when
 *  bus error exceptions occur and also when the read & write handlers are called. A scheduled
 *  load will complete if a load breakpoint fires, but an unscheduled load will not.
 *
 *  Reading from the data and instruction cache at the same time causes a bus conflict that
 *  corrupts the data in a reliable but strange way, which is not emulated.
 *
 *  Values written to COP1 & COP3 can be read back by the next instruction, which is not emulated.
 *  Because of loadscheduling the value loaded with LWC1/LWC3 can be read by more than the next
 *  instruction.
 *
 *  SWC0 writes stale data from a previous operation, this is only partially emulated as the timing
 *  is complicated. Left over instruction fetches are currently emulated as they are the most
 *  'interesting' and have no impact on the rest of the emulation.
 *
 *  MTC0 timing is not emulated, switching to user mode while in kernel space continues
 *  execution for another two instructions before taking an exception. Using RFE to do the same
 *  thing causes the exception straight away, unless the RFE is the first instruction that follows
 *  an MTC0 instruction.
 *
 *  The PRId register should be 1 on some revisions of the CPU ( there might be other values too ).
 *
 *  Moving to the HI/LO register after a multiply or divide, but before reading the results will
 *  always abort the operation as if you did it immediately. In reality it should complete on it's
 *  own, and aborting before it completes would result in returning the working results.
 *
 *  Running code in cached address space does not use or update the instruction cache.
 *
 *  Wait states are not emulated.
 *
 *  Bus errors caused by instruction fetches are not supported.
 *
 */

#include "emu.h"
#include "debugger.h"
#include "psx.h"
#include "mdec.h"
#include "rcnt.h"
#include "sound/spu.h"

#define LOG_BIOSCALL ( 0 )

#define EXC_INT ( 0 )
#define EXC_ADEL ( 4 )
#define EXC_ADES ( 5 )
#define EXC_IBE ( 6 )
#define EXC_DBE ( 7 )
#define EXC_SYS ( 8 )
#define EXC_BP ( 9 )
#define EXC_RI ( 10 )
#define EXC_CPU ( 11 )
#define EXC_OVF ( 12 )

#define CP0_INDEX ( 0 )
#define CP0_RANDOM ( 1 )
#define CP0_ENTRYLO ( 2 )
#define CP0_CONTEXT ( 4 )
#define CP0_ENTRYHI ( 10 )

#define CP0_BPC ( 3 )
#define CP0_BDA ( 5 )
#define CP0_TAR ( 6 )
#define CP0_DCIC ( 7 )
#define CP0_BADA ( 8 )
#define CP0_BDAM ( 9 )
#define CP0_BPCM ( 11 )
#define CP0_SR ( 12 )
#define CP0_CAUSE ( 13 )
#define CP0_EPC ( 14 )
#define CP0_PRID ( 15 )

#define DCIC_STATUS ( 0x3f )
#define DCIC_DB ( 1L << 0 )
#define DCIC_DA ( 1L << 2 )
#define DCIC_R ( 1L << 3 )
#define DCIC_W ( 1L << 4 )
#define DCIC_DE ( 1L << 23 )
#define DCIC_DAE ( 1L << 25 )
#define DCIC_DR ( 1L << 26 )
#define DCIC_DW ( 1L << 27 )
#define DCIC_KD ( 1L << 29 )
#define DCIC_UD ( 1L << 30 )
#define DCIC_TR ( 1L << 31 )

#define SR_IEC ( 1L << 0 )
#define SR_KUC ( 1L << 1 )
#define SR_ISC ( 1L << 16 )
#define SR_SWC ( 1L << 17 )
#define SR_BEV ( 1L << 22 )
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
#define CAUSE_BT ( 1L << 30 )
#define CAUSE_BD ( 1L << 31 )

#define BIU_LOCK ( 0x00000001 )
#define BIU_INV  ( 0x00000002 )
#define BIU_TAG  ( 0x00000004 )
#define BIU_RAM  ( 0x00000008 )
#define BIU_DS   ( 0x00000080 )
#define BIU_IS1  ( 0x00000800 )

#define TAG_MATCH_MASK ( 0 - ( ICACHE_ENTRIES * 4 ) )
#define TAG_MATCH ( 0x10 )
#define TAG_VALID ( 0x0f )

#define MULTIPLIER_OPERATION_IDLE ( 0 )
#define MULTIPLIER_OPERATION_MULT ( 1 )
#define MULTIPLIER_OPERATION_MULTU ( 2 )
#define MULTIPLIER_OPERATION_DIV ( 3 )
#define MULTIPLIER_OPERATION_DIVU ( 4 )

static const char *const delayn[] =
{
	"",   "at", "v0", "v1", "a0", "a1", "a2", "a3",
	"t0", "t1", "t2", "t3", "t4", "t5", "t6", "t7",
	"s0", "s1", "s2", "s3", "s4", "s5", "s6", "s7",
	"t8", "t9", "k0", "k1", "gp", "sp", "fp", "ra",
	"pc", "!pc"
};

// device type definition
const device_type CXD8530AQ = &device_creator<cxd8530aq_device>;
const device_type CXD8530BQ = &device_creator<cxd8530bq_device>;
const device_type CXD8530CQ = &device_creator<cxd8530cq_device>;
const device_type CXD8661R = &device_creator<cxd8661r_device>;
const device_type CXD8606BQ = &device_creator<cxd8606bq_device>;
const device_type CXD8606CQ = &device_creator<cxd8606cq_device>;

static const UINT32 mtc0_writemask[]=
{
	0x00000000, /* !INDEX */
	0x00000000, /* !RANDOM */
	0x00000000, /* !ENTRYLO */
	0xffffffff, /* BPC */
	0x00000000, /* !CONTEXT */
	0xffffffff, /* BDA */
	0x00000000, /* TAR */
	0xff80f03f, /* DCIC */
	0x00000000, /* BADA */
	0xffffffff, /* BDAM */
	0x00000000, /* !ENTRYHI */
	0xffffffff, /* BPCM */
	0xf04fff3f, /* SR */
	0x00000300, /* CAUSE */
	0x00000000, /* EPC */
	0x00000000  /* PRID */
};

READ32_MEMBER( psxcpu_device::berr_r )
{
	if( !space.debugger_access() )
		m_berr = 1;
	return 0;
}

WRITE32_MEMBER( psxcpu_device::berr_w )
{
	if( !space.debugger_access() )
		m_berr = 1;
}

READ32_MEMBER( psxcpu_device::exp_base_r )
{
	return m_exp_base;
}

WRITE32_MEMBER( psxcpu_device::exp_base_w )
{
	COMBINE_DATA( &m_exp_base ); // TODO: check byte writes

	m_exp_base = 0x1f000000 | ( m_exp_base & 0xffffff );
}

UINT32 psxcpu_device::exp_base()
{
	return m_exp_base;
}

READ32_MEMBER( psxcpu_device::exp_config_r )
{
	return m_exp_config;
}

WRITE32_MEMBER( psxcpu_device::exp_config_w )
{
	COMBINE_DATA( &m_exp_config ); // TODO: check byte writes

	m_exp_config &= 0xaf1fffff;
}

READ32_MEMBER( psxcpu_device::ram_config_r )
{
	return m_ram_config;
}

WRITE32_MEMBER( psxcpu_device::ram_config_w )
{
	UINT32 old = m_ram_config;

	COMBINE_DATA( &m_ram_config ); // TODO: check byte writes

	if( ( ( m_ram_config ^ old ) & 0xff00 ) != 0 )
	{
		update_ram_config();
	}
}

READ32_MEMBER( psxcpu_device::rom_config_r )
{
	return m_rom_config;
}

WRITE32_MEMBER( psxcpu_device::rom_config_w )
{
	UINT32 old = m_rom_config;

	COMBINE_DATA( &m_rom_config ); // TODO: check byte writes

	if( ( ( m_rom_config ^ old ) & 0x001f0000 ) != 0 )
	{
		update_rom_config();
	}
}

READ32_MEMBER( psxcpu_device::com_delay_r )
{
	//verboselog( p_psx, 1, "psx_com_delay_r( %08x )\n", mem_mask );
	return m_com_delay;
}

WRITE32_MEMBER( psxcpu_device::com_delay_w )
{
	COMBINE_DATA( &m_com_delay ); // TODO: check byte writes
	//verboselog( p_psx, 1, "psx_com_delay_w( %08x %08x )\n", data, mem_mask );
}

READ32_MEMBER( psxcpu_device::biu_r )
{
	return m_biu;
}

WRITE32_MEMBER( psxcpu_device::biu_w )
{
	UINT32 old = m_biu;

	COMBINE_DATA( &m_biu ); // TODO: check byte writes

	if( ( old & ( BIU_RAM | BIU_DS ) ) != ( m_biu & ( BIU_RAM | BIU_DS ) ) )
	{
		update_scratchpad();
	}
}

void psxcpu_device::stop()
{
	debugger_break( machine() );
	debugger_instruction_hook( this,  m_pc );
}

UINT32 psxcpu_device::cache_readword( UINT32 offset )
{
	UINT32 data = 0;

	if( ( m_biu & BIU_TAG ) != 0 )
	{
		if( ( m_biu & BIU_IS1 ) != 0 )
		{
			UINT32 tag = m_icacheTag[ ( offset / 16 ) % ( ICACHE_ENTRIES / 4 ) ];
			data |= tag & TAG_VALID;

			if( ( ( tag ^ offset ) & TAG_MATCH_MASK ) == 0 )
			{
				data |= TAG_MATCH;
			}
		}
	}
	else if( ( m_biu & ( BIU_LOCK | BIU_INV ) ) != 0 )
	{
	}
	else
	{
		if( ( m_biu & BIU_IS1 ) == BIU_IS1 )
		{
			data |= m_icache[ ( offset / 4 ) % ICACHE_ENTRIES ];
		}

		if( ( m_biu & BIU_DS ) == BIU_DS )
		{
			data |= m_dcache[ ( offset / 4 ) % DCACHE_ENTRIES ];
		}
	}

	return data;
}

void psxcpu_device::cache_writeword( UINT32 offset, UINT32 data )
{
	if( ( m_biu & BIU_TAG ) != 0 )
	{
		if( ( m_biu & BIU_IS1 ) != 0 )
		{
			m_icacheTag[ ( offset / 16 ) % ( ICACHE_ENTRIES / 4 ) ] = ( data & TAG_VALID ) | ( offset & TAG_MATCH_MASK );
		}
	}
	else if( ( m_biu & ( BIU_LOCK | BIU_INV ) ) != 0 )
	{
		if( ( m_biu & BIU_IS1 ) != 0 )
		{
			m_icacheTag[ ( offset / 16 ) % ( ICACHE_ENTRIES / 4 ) ] = ( offset & TAG_MATCH_MASK );
		}
	}
	else
	{
		if( ( m_biu & BIU_IS1 ) != 0 )
		{
			m_icache[ ( offset / 4 ) % ICACHE_ENTRIES ] = data;
		}

		if( ( m_biu & BIU_DS ) != 0 )
		{
			m_dcache[ ( offset / 4 ) % DCACHE_ENTRIES ] = data;
		}
	}
}

UINT8 psxcpu_device::readbyte( UINT32 address )
{
	if( m_bus_attached )
	{
		return m_program->read_byte( address );
	}

	return cache_readword( address ) >> ( ( address & 3 ) * 8 );
}

UINT16 psxcpu_device::readhalf( UINT32 address )
{
	if( m_bus_attached )
	{
		return m_program->read_word( address );
	}

	return cache_readword( address ) >> ( ( address & 2 ) * 8 );
}

UINT32 psxcpu_device::readword( UINT32 address )
{
	if( m_bus_attached )
	{
		return m_program->read_dword( address );
	}

	return cache_readword( address );
}

UINT32 psxcpu_device::readword_masked( UINT32 address, UINT32 mask )
{
	if( m_bus_attached )
	{
		return m_program->read_dword( address, mask );
	}

	return cache_readword( address );
}

void psxcpu_device::writeword( UINT32 address, UINT32 data )
{
	if( m_bus_attached )
	{
		m_program->write_dword( address, data );
	}
	else
	{
		cache_writeword( address, data );
	}
}

void psxcpu_device::writeword_masked( UINT32 address, UINT32 data, UINT32 mask )
{
	if( m_bus_attached )
	{
		m_program->write_dword( address, data, mask );
	}
	else
	{
		cache_writeword( address, data );
	}
}


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
	{ 0x00, 0x00, nullptr }
};

UINT32 psxcpu_device::log_bioscall_parameter( int parm )
{
	if( parm < 4 )
	{
		return m_r[ 4 + parm ];
	}

	return readword( m_r[ 29 ] + ( parm * 4 ) );
}

const char *psxcpu_device::log_bioscall_string( int parm )
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
		UINT8 c = readbyte( address );
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

const char *psxcpu_device::log_bioscall_hex( int parm )
{
	static char string[ 1024 ];

	sprintf( string, "0x%08x", log_bioscall_parameter( parm ) );

	return string;
}

const char *psxcpu_device::log_bioscall_char( int parm )
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

void psxcpu_device::log_bioscall()
{
	int address = m_pc - 0x04;
	if( address == 0xa0 ||
		address == 0xb0 ||
		address == 0xc0 )
	{
		char buf[ 1024 ];
		int operation = m_r[ 9 ] & 0xff;
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
			int buffer = log_bioscall_parameter( 1 );
			int nbytes = log_bioscall_parameter( 2 );

			if( fd == 1 )
			{
				while( nbytes > 0 )
				{
					UINT8 c = readbyte( buffer );
					putchar( c );
					nbytes--;
					buffer++;
				}
			}
		}

		while( bioscalls[ bioscall ].prototype != nullptr &&
			( bioscalls[ bioscall ].address != address ||
			bioscalls[ bioscall ].operation != operation ) )
		{
			bioscall++;
		}

		if( bioscalls[ bioscall ].prototype != nullptr )
		{
			const char *prototype = bioscalls[ bioscall ].prototype;
			const char *parmstart = nullptr;
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
							const char *parmstr = nullptr;
							int percent = 0;

							for( ;; )
							{
								UINT8 c = readbyte( format );
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

								if( parmstr != nullptr )
								{
									if( parm > 0 )
									{
										buf[ pos++ ] = ',';
									}
									buf[ pos++ ] = ' ';

									strcpy( &buf[ pos ], parmstr );
									pos += strlen( parmstr );
									parmstr = nullptr;

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
		logerror( "%08x: bioscall %s\n", (unsigned int)m_r[ 31 ] - 8, buf );
	}
}

void psxcpu_device::log_syscall()
{
	char buf[ 1024 ];
	int operation = m_r[ 4 ];

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
	logerror( "%08x: syscall %s\n", (unsigned int)m_r[ 31 ] - 8, buf );
}

void psxcpu_device::update_memory_handlers()
{
	if( ( m_cp0r[ CP0_SR ] & SR_ISC ) != 0 )
	{
		m_bus_attached = 0;
	}
	else
	{
		m_bus_attached = 1;
	}
}

void psxcpu_device::funct_mthi()
{
	m_multiplier_operation = MULTIPLIER_OPERATION_IDLE;
	m_hi = m_r[ INS_RS( m_op ) ];
}

void psxcpu_device::funct_mtlo()
{
	m_multiplier_operation = MULTIPLIER_OPERATION_IDLE;
	m_lo = m_r[ INS_RS( m_op ) ];
}

void psxcpu_device::funct_mult()
{
	m_multiplier_operation = MULTIPLIER_OPERATION_MULT;
	m_multiplier_operand1 = m_r[ INS_RS( m_op ) ];
	m_multiplier_operand2 = m_r[ INS_RT( m_op ) ];
	m_lo = m_multiplier_operand1;
}

void psxcpu_device::funct_multu()
{
	m_multiplier_operation = MULTIPLIER_OPERATION_MULTU;
	m_multiplier_operand1 = m_r[ INS_RS( m_op ) ];
	m_multiplier_operand2 = m_r[ INS_RT( m_op ) ];
	m_lo = m_multiplier_operand1;
}

void psxcpu_device::funct_div()
{
	m_multiplier_operation = MULTIPLIER_OPERATION_DIV;
	m_multiplier_operand1 = m_r[ INS_RS( m_op ) ];
	m_multiplier_operand2 = m_r[ INS_RT( m_op ) ];
	m_lo = m_multiplier_operand1;
	m_hi = 0;
}

void psxcpu_device::funct_divu()
{
	m_multiplier_operation = MULTIPLIER_OPERATION_DIVU;
	m_multiplier_operand1 = m_r[ INS_RS( m_op ) ];
	m_multiplier_operand2 = m_r[ INS_RT( m_op ) ];
	m_lo = m_multiplier_operand1;
	m_hi = 0;
}

void psxcpu_device::multiplier_update()
{
	switch( m_multiplier_operation )
	{
	case MULTIPLIER_OPERATION_MULT:
		{
			INT64 result = mul_32x32( (INT32)m_multiplier_operand1, (INT32)m_multiplier_operand2 );
			m_lo = EXTRACT_64LO( result );
			m_hi = EXTRACT_64HI( result );
		}
		break;

	case MULTIPLIER_OPERATION_MULTU:
		{
			UINT64 result = mulu_32x32( m_multiplier_operand1, m_multiplier_operand2 );
			m_lo = EXTRACT_64LO( result );
			m_hi = EXTRACT_64HI( result );
		}
		break;

	case MULTIPLIER_OPERATION_DIV:
		if( m_multiplier_operand1 == 0x80000000 && m_multiplier_operand2 == 0xffffffff)
		{
			m_hi = 0x00000000;
			m_lo = 0x80000000;
		}
		else if( m_multiplier_operand2 == 0 )
		{
			if( (INT32)m_multiplier_operand1 < 0 )
			{
				m_lo = 1;
			}
			else
			{
				m_lo = 0xffffffff;
			}

			m_hi = m_multiplier_operand1;
		}
		else
		{
			m_lo = (INT32)m_multiplier_operand1 / (INT32)m_multiplier_operand2;
			m_hi = (INT32)m_multiplier_operand1 % (INT32)m_multiplier_operand2;
		}
		break;

	case MULTIPLIER_OPERATION_DIVU:
		if( m_multiplier_operand2 == 0 )
		{
			m_lo = 0xffffffff;
			m_hi = m_multiplier_operand1;
		}
		else
		{
			m_lo = m_multiplier_operand1 / m_multiplier_operand2;
			m_hi = m_multiplier_operand1 % m_multiplier_operand2;
		}
		break;
	}

	m_multiplier_operation = MULTIPLIER_OPERATION_IDLE;
}

UINT32 psxcpu_device::get_hi()
{
	if( m_multiplier_operation != MULTIPLIER_OPERATION_IDLE )
	{
		multiplier_update();
	}

	return m_hi;
}

UINT32 psxcpu_device::get_lo()
{
	if( m_multiplier_operation != MULTIPLIER_OPERATION_IDLE )
	{
		multiplier_update();
	}

	return m_lo;
}

int psxcpu_device::execute_unstoppable_instructions( int executeCop2 )
{
	switch( INS_OP( m_op ) )
	{
	case OP_SPECIAL:
		switch( INS_FUNCT( m_op ) )
		{
		case FUNCT_MTHI:
			funct_mthi();
			break;

		case FUNCT_MTLO:
			funct_mtlo();
			break;

		case FUNCT_MULT:
			funct_mult();
			break;

		case FUNCT_MULTU:
			funct_multu();
			break;

		case FUNCT_DIV:
			funct_div();
			break;

		case FUNCT_DIVU:
			funct_divu();
			break;
		}
		break;

	case OP_COP2:
		if( executeCop2 )
		{
			switch( INS_CO( m_op ) )
			{
			case 1:
				if( ( m_cp0r[ CP0_SR ] & SR_CU2 ) == 0 )
				{
					return 0;
				}

				if( !m_gte.docop2( m_pc, INS_COFUN( m_op ) ) )
				{
					stop();
				}
				break;
			}
		}
	}

	return 1;
}

void psxcpu_device::update_address_masks()
{
	if( ( m_cp0r[ CP0_SR ] & SR_KUC ) != 0 )
	{
		m_bad_byte_address_mask = 0x80000000;
		m_bad_half_address_mask = 0x80000001;
		m_bad_word_address_mask = 0x80000003;
	}
	else
	{
		m_bad_byte_address_mask = 0;
		m_bad_half_address_mask = 1;
		m_bad_word_address_mask = 3;
	}
}

void psxcpu_device::update_scratchpad()
{
	if( ( m_biu & BIU_RAM ) == 0 )
	{
		m_program->install_readwrite_handler( 0x1f800000, 0x1f8003ff, read32_delegate( FUNC( psxcpu_device::berr_r ), this ), write32_delegate( FUNC( psxcpu_device::berr_w ), this ) );
	}
	else if( ( m_biu & BIU_DS ) == 0 )
	{
		m_program->install_read_handler( 0x1f800000, 0x1f8003ff, read32_delegate( FUNC( psxcpu_device::berr_r ), this ) );
		m_program->nop_write( 0x1f800000, 0x1f8003ff );
	}
	else
	{
		m_program->install_ram( 0x1f800000, 0x1f8003ff, m_dcache );
	}
}

void psxcpu_device::update_ram_config()
{
	/// TODO: find out what these values really control and confirm they are the same on each cpu type.

	int window_size = 0;
	switch( ( m_ram_config >> 8 ) & 0xf )
	{
	case 0x8: // konami gv
		window_size = 0x0200000;
		break;

	case 0xc: // zn1/konami gq/namco system 11/twinkle/system 573
		window_size = 0x0400000;
		break;

	case 0x3: // zn2
	case 0xb: // console/primal rage 2
		window_size = 0x0800000;
		break;

	case 0xf: // namco system 10/namco system 12
		window_size = 0x1000000;
		break;
	}

	UINT32 ram_size = m_ram->size();
	UINT8 *pointer = m_ram->pointer();

	if( ram_size > window_size )
	{
		ram_size = window_size;
	}

	if( ram_size > 0 )
	{
		int start = 0;
		while( start < window_size )
		{
			m_program->install_ram( start + 0x00000000, start + 0x00000000 + ram_size - 1, pointer );
			m_program->install_ram( start + 0x80000000, start + 0x80000000 + ram_size - 1, pointer );
			m_program->install_ram( start + 0xa0000000, start + 0xa0000000 + ram_size - 1, pointer );

			start += ram_size;
		}
	}

	m_program->install_readwrite_handler( 0x00000000 + window_size, 0x1effffff, read32_delegate( FUNC( psxcpu_device::berr_r ), this ), write32_delegate( FUNC( psxcpu_device::berr_w ), this ) );
	m_program->install_readwrite_handler( 0x80000000 + window_size, 0x9effffff, read32_delegate( FUNC( psxcpu_device::berr_r ), this ), write32_delegate( FUNC( psxcpu_device::berr_w ), this ) );
	m_program->install_readwrite_handler( 0xa0000000 + window_size, 0xbeffffff, read32_delegate( FUNC( psxcpu_device::berr_r ), this ), write32_delegate( FUNC( psxcpu_device::berr_w ), this ) );
}

void psxcpu_device::update_rom_config()
{
	int window_size = 1 << ( ( m_rom_config >> 16 ) & 0x1f );
	int max_window_size = 0x400000;
	if( window_size > max_window_size )
	{
		window_size = max_window_size;
	}

	UINT32 rom_size = m_rom->bytes();
	UINT8 *pointer = m_rom->base();

	if( rom_size > window_size )
	{
		rom_size = window_size;
	}

	if( rom_size > 0 )
	{
		int start = 0;
		while( start < window_size )
		{
			m_program->install_rom( start + 0x1fc00000, start + 0x1fc00000 + rom_size - 1, pointer );
			m_program->install_rom( start + 0x9fc00000, start + 0x9fc00000 + rom_size - 1, pointer );
			m_program->install_rom( start + 0xbfc00000, start + 0xbfc00000 + rom_size - 1, pointer );

			start += rom_size;
		}
	}

	if( window_size < max_window_size && !m_disable_rom_berr)
	{
		m_program->install_readwrite_handler( 0x1fc00000 + window_size, 0x1fffffff, read32_delegate( FUNC( psxcpu_device::berr_r ), this ), write32_delegate( FUNC( psxcpu_device::berr_w ), this ) );
		m_program->install_readwrite_handler( 0x9fc00000 + window_size, 0x9fffffff, read32_delegate( FUNC( psxcpu_device::berr_r ), this ), write32_delegate( FUNC( psxcpu_device::berr_w ), this ) );
		m_program->install_readwrite_handler( 0xbfc00000 + window_size, 0xbfffffff, read32_delegate( FUNC( psxcpu_device::berr_r ), this ), write32_delegate( FUNC( psxcpu_device::berr_w ), this ) );
	}
}

void psxcpu_device::update_cop0( int reg )
{
	if( reg == CP0_SR )
	{
		update_memory_handlers();
		update_address_masks();
	}

	if( ( reg == CP0_SR || reg == CP0_CAUSE ) &&
		( m_cp0r[ CP0_SR ] & SR_IEC ) != 0 &&
		( m_cp0r[ CP0_SR ] & m_cp0r[ CP0_CAUSE ] & CAUSE_IP ) != 0 )
	{
		m_op = m_direct->read_dword( m_pc );
		execute_unstoppable_instructions( 1 );
		exception( EXC_INT );
	}
	else if( reg == CP0_SR &&
		m_delayr != PSXCPU_DELAYR_PC &&
		( m_pc & m_bad_word_address_mask ) != 0 )
	{
		load_bad_address( m_pc );
	}
}

void psxcpu_device::commit_delayed_load()
{
	if( m_delayr != 0 )
	{
		m_r[ m_delayr ] = m_delayv;
		m_delayr = 0;
		m_delayv = 0;
	}
}

void psxcpu_device::set_pc( unsigned pc )
{
	m_pc = pc;
}

void psxcpu_device::fetch_next_op()
{
	if( m_delayr == PSXCPU_DELAYR_PC )
	{
		UINT32 safepc = m_delayv & ~m_bad_word_address_mask;

		m_op = m_direct->read_dword( safepc );
	}
	else
	{
		m_op = m_direct->read_dword( m_pc + 4 );
	}
}

int psxcpu_device::advance_pc()
{
	if( m_delayr == PSXCPU_DELAYR_PC )
	{
		m_pc = m_delayv;
		m_delayr = 0;
		m_delayv = 0;

		if( ( m_pc & m_bad_word_address_mask ) != 0 )
		{
			load_bad_address( m_pc );
			return 0;
		}
	}
	else if( m_delayr == PSXCPU_DELAYR_NOTPC )
	{
		m_delayr = 0;
		m_delayv = 0;
		m_pc += 4;
	}
	else
	{
		commit_delayed_load();
		m_pc += 4;
	}

	return 1;
}

void psxcpu_device::load( UINT32 reg, UINT32 value )
{
	advance_pc();

	if( reg != 0 )
	{
		m_r[ reg ] = value;
	}
}

void psxcpu_device::delayed_load( UINT32 reg, UINT32 value )
{
	if( m_delayr == reg )
	{
		m_delayr = 0;
		m_delayv = 0;
	}

	advance_pc();

	m_delayr = reg;
	m_delayv = value;
}

void psxcpu_device::branch( UINT32 address )
{
	advance_pc();

	m_delayr = PSXCPU_DELAYR_PC;
	m_delayv = address;
}

void psxcpu_device::conditional_branch( int takeBranch )
{
	advance_pc();

	if( takeBranch )
	{
		m_delayr = PSXCPU_DELAYR_PC;
		m_delayv = m_pc + ( PSXCPU_WORD_EXTEND( INS_IMMEDIATE( m_op ) ) << 2 );
	}
	else
	{
		m_delayr = PSXCPU_DELAYR_NOTPC;
		m_delayv = 0;
	}
}

void psxcpu_device::unconditional_branch()
{
	advance_pc();

	m_delayr = PSXCPU_DELAYR_PC;
	m_delayv = ( m_pc & 0xf0000000 ) + ( INS_TARGET( m_op ) << 2 );
}

void psxcpu_device::common_exception( int exception, UINT32 romOffset, UINT32 ramOffset )
{
	int cause = ( exception << 2 ) | ( ( ( m_op >> 26 ) & 3 ) << 28 );

	if( m_delayr == PSXCPU_DELAYR_PC )
	{
		cause |= CAUSE_BT;
		m_cp0r[ CP0_TAR ] = m_delayv;
	}
	else if( m_delayr == PSXCPU_DELAYR_NOTPC )
	{
		m_cp0r[ CP0_TAR ] = m_pc + 4;
	}
	else
	{
		commit_delayed_load();
	}

	if( m_delayr == PSXCPU_DELAYR_PC || m_delayr == PSXCPU_DELAYR_NOTPC )
	{
		cause |= CAUSE_BD;
		m_cp0r[ CP0_EPC ] = m_pc - 4;
	}
	else
	{
		m_cp0r[ CP0_EPC ] = m_pc;
	}

	if( LOG_BIOSCALL && exception != EXC_INT )
	{
		logerror( "%08x: Exception %d\n", m_pc, exception );
	}

	m_delayr = 0;
	m_delayv = 0;
	m_berr = 0;

	if( m_cp0r[ CP0_SR ] & SR_BEV )
	{
		set_pc( romOffset );
	}
	else
	{
		set_pc( ramOffset );
	}

	m_cp0r[ CP0_SR ] = ( m_cp0r[ CP0_SR ] & ~0x3f ) | ( ( m_cp0r[ CP0_SR ] << 2 ) & 0x3f );
	m_cp0r[ CP0_CAUSE ] = ( m_cp0r[ CP0_CAUSE ] & ~( CAUSE_EXC | CAUSE_BD | CAUSE_BT | CAUSE_CE ) ) | cause;
	update_cop0( CP0_SR );
}

void psxcpu_device::exception( int exception )
{
	common_exception( exception, 0xbfc00180, 0x80000080 );
}

void psxcpu_device::breakpoint_exception()
{
	fetch_next_op();
	execute_unstoppable_instructions( 1 );
	common_exception( EXC_BP, 0xbfc00140, 0x80000040 );
}

void psxcpu_device::fetch_bus_error_exception()
{
	common_exception( EXC_IBE, 0xbfc00180, 0x80000080 );
}

void psxcpu_device::load_bus_error_exception()
{
	fetch_next_op();
	execute_unstoppable_instructions( 0 );
	common_exception( EXC_DBE, 0xbfc00180, 0x80000080 );
}

void psxcpu_device::store_bus_error_exception()
{
	fetch_next_op();

	if( execute_unstoppable_instructions( 1 ) )
	{
		if( !advance_pc() )
		{
			return;
		}

		fetch_next_op();
		execute_unstoppable_instructions( 0 );
	}

	common_exception( EXC_DBE, 0xbfc00180, 0x80000080 );
}

void psxcpu_device::load_bad_address( UINT32 address )
{
	m_cp0r[ CP0_BADA ] = address;
	exception( EXC_ADEL );
}

void psxcpu_device::store_bad_address( UINT32 address )
{
	m_cp0r[ CP0_BADA ] = address;
	exception( EXC_ADES );
}

int psxcpu_device::data_address_breakpoint( int dcic_rw, int dcic_status, UINT32 address )
{
	if( address < 0x1f000000 || address > 0x1fffffff )
	{
		if( ( m_cp0r[ CP0_DCIC ] & DCIC_DE ) != 0 &&
			( ( ( m_cp0r[ CP0_DCIC ] & DCIC_KD ) != 0 && ( m_cp0r[ CP0_SR ] & SR_KUC ) == 0 ) ||
			( ( m_cp0r[ CP0_DCIC ] & DCIC_UD ) != 0 && ( m_cp0r[ CP0_SR ] & SR_KUC ) != 0 ) ) )
		{
			if( ( m_cp0r[ CP0_DCIC ] & dcic_rw ) == dcic_rw &&
				( address & m_cp0r[ CP0_BDAM ] ) == ( m_cp0r[ CP0_BDA ] & m_cp0r[ CP0_BDAM ] ) )
			{
				m_cp0r[ CP0_DCIC ] = ( m_cp0r[ CP0_DCIC ] & ~DCIC_STATUS ) | dcic_status;

				if( ( m_cp0r[ CP0_DCIC ] & DCIC_TR ) != 0 )
				{
					return 1;
				}
			}
		}
	}

	return 0;
}

int psxcpu_device::load_data_address_breakpoint( UINT32 address )
{
	return data_address_breakpoint( DCIC_DR | DCIC_DAE, DCIC_DB | DCIC_DA | DCIC_R, address );
}

int psxcpu_device::store_data_address_breakpoint( UINT32 address )
{
	return data_address_breakpoint( DCIC_DW | DCIC_DAE, DCIC_DB | DCIC_DA | DCIC_W, address );
}

// On-board RAM and peripherals
static ADDRESS_MAP_START( psxcpu_internal_map, AS_PROGRAM, 32, psxcpu_device )
	AM_RANGE( 0x1f800000, 0x1f8003ff ) AM_NOP /* scratchpad */
	AM_RANGE( 0x1f800400, 0x1f800fff ) AM_READWRITE( berr_r, berr_w )
	AM_RANGE( 0x1f801000, 0x1f801003 ) AM_READWRITE( exp_base_r, exp_base_w )
	AM_RANGE( 0x1f801004, 0x1f801007 ) AM_RAM
	AM_RANGE( 0x1f801008, 0x1f80100b ) AM_READWRITE( exp_config_r, exp_config_w )
	AM_RANGE( 0x1f80100c, 0x1f80100f ) AM_RAM
	AM_RANGE( 0x1f801010, 0x1f801013 ) AM_READWRITE( rom_config_r, rom_config_w )
	AM_RANGE( 0x1f801014, 0x1f80101f ) AM_RAM
	/* 1f801014 spu delay */
	/* 1f801018 dv delay */
	AM_RANGE( 0x1f801020, 0x1f801023 ) AM_READWRITE( com_delay_r, com_delay_w )
	AM_RANGE( 0x1f801024, 0x1f80102f ) AM_RAM
	AM_RANGE( 0x1f801040, 0x1f80104f ) AM_DEVREADWRITE( "sio0", psxsio_device, read, write )
	AM_RANGE( 0x1f801050, 0x1f80105f ) AM_DEVREADWRITE( "sio1", psxsio_device, read, write )
	AM_RANGE( 0x1f801060, 0x1f801063 ) AM_READWRITE( ram_config_r, ram_config_w )
	AM_RANGE( 0x1f801064, 0x1f80106f ) AM_RAM
	AM_RANGE( 0x1f801070, 0x1f801077 ) AM_DEVREADWRITE( "irq", psxirq_device, read, write )
	AM_RANGE( 0x1f801080, 0x1f8010ff ) AM_DEVREADWRITE( "dma", psxdma_device, read, write )
	AM_RANGE( 0x1f801100, 0x1f80112f ) AM_DEVREADWRITE( "rcnt", psxrcnt_device, read, write )
	AM_RANGE( 0x1f801800, 0x1f801803 ) AM_READWRITE8( cd_r, cd_w, 0xffffffff )
	AM_RANGE( 0x1f801810, 0x1f801817 ) AM_READWRITE( gpu_r, gpu_w )
	AM_RANGE( 0x1f801820, 0x1f801827 ) AM_DEVREADWRITE( "mdec", psxmdec_device, read, write )
	AM_RANGE( 0x1f801c00, 0x1f801dff ) AM_READWRITE16( spu_r, spu_w, 0xffffffff )
	AM_RANGE( 0x1f802020, 0x1f802033 ) AM_RAM /* ?? */
	/* 1f802030 int 2000 */
	/* 1f802040 dip switches */
	AM_RANGE( 0x1f802040, 0x1f802043 ) AM_WRITENOP
	AM_RANGE( 0x20000000, 0x7fffffff ) AM_READWRITE( berr_r, berr_w )
	AM_RANGE( 0xc0000000, 0xfffdffff ) AM_READWRITE( berr_r, berr_w )
	AM_RANGE( 0xfffe0130, 0xfffe0133 ) AM_READWRITE( biu_r, biu_w )
ADDRESS_MAP_END


//**************************************************************************
//  DEVICE INTERFACE
//**************************************************************************

//-------------------------------------------------
//  psxcpu_device - constructor
//-------------------------------------------------

psxcpu_device::psxcpu_device( const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, UINT32 clock, const char *shortname, const char *source ) :
	cpu_device( mconfig, type, name, tag, owner, clock, shortname, source ),
	m_program_config( "program", ENDIANNESS_LITTLE, 32, 32, 0, ADDRESS_MAP_NAME( psxcpu_internal_map ) ),
	m_gpu_read_handler( *this ),
	m_gpu_write_handler( *this ),
	m_spu_read_handler( *this ),
	m_spu_write_handler( *this ),
	m_cd_read_handler( *this ),
	m_cd_write_handler( *this ),
	m_ram( *this, "ram" )
{
	m_disable_rom_berr = false;
}

cxd8530aq_device::cxd8530aq_device( const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock )
	: psxcpu_device( mconfig, CXD8661R, "CXD8530AQ", tag, owner, clock, "cxd8530aq", __FILE__ )
{
}

cxd8530bq_device::cxd8530bq_device( const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock )
	: psxcpu_device( mconfig, CXD8661R, "CXD8530BQ", tag, owner, clock, "cxd8530bq", __FILE__ )
{
}

cxd8530cq_device::cxd8530cq_device( const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock )
	: psxcpu_device( mconfig, CXD8661R, "CXD8530CQ", tag, owner, clock, "cxd8530cq", __FILE__ )
{
}

cxd8661r_device::cxd8661r_device( const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock )
	: psxcpu_device( mconfig, CXD8661R, "CXD8661R", tag, owner, clock, "cxd8661r", __FILE__ )
{
}

cxd8606bq_device::cxd8606bq_device( const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock )
	: psxcpu_device( mconfig, CXD8606BQ, "CXD8606BQ", tag, owner, clock, "cxd8606bq", __FILE__ )
{
}

cxd8606cq_device::cxd8606cq_device( const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock )
	: psxcpu_device( mconfig, CXD8606CQ, "CXD8606CQ", tag, owner, clock, "cxd8606cq", __FILE__ )
{
}

//-------------------------------------------------
//  device_start - start up the device
//-------------------------------------------------

void psxcpu_device::device_start()
{
	// get our address spaces
	m_program = &space( AS_PROGRAM );
	m_direct = &m_program->direct();

	save_item( NAME( m_op ) );
	save_item( NAME( m_pc ) );
	save_item( NAME( m_delayv ) );
	save_item( NAME( m_delayr ) );
	save_item( NAME( m_hi ) );
	save_item( NAME( m_lo ) );
	save_item( NAME( m_biu ) );
	save_item( NAME( m_r ) );
	save_item( NAME( m_cp0r ) );
	save_item( NAME( m_gte.m_cp2cr ) );
	save_item( NAME( m_gte.m_cp2dr ) );
	save_item( NAME( m_icacheTag ) );
	save_item( NAME( m_icache ) );
	save_item( NAME( m_dcache ) );
	save_item( NAME( m_multiplier_operation ) );
	save_item( NAME( m_multiplier_operand1 ) );
	save_item( NAME( m_multiplier_operand2 ) );

	state_add( STATE_GENPC, "GENPC", m_pc ).noshow();
	state_add( PSXCPU_PC, "pc", m_pc );
	state_add( PSXCPU_DELAYR, "delayr", m_delayr ).formatstr("%8s");
	state_add( PSXCPU_DELAYV, "delayv", m_delayv );
	state_add( PSXCPU_HI, "hi", m_hi );
	state_add( PSXCPU_LO, "lo", m_lo );
	state_add( PSXCPU_BIU, "biu", m_biu );
	state_add( PSXCPU_R0, "zero", m_r[ 0 ] );
	state_add( PSXCPU_R1, "at", m_r[ 1 ] );
	state_add( PSXCPU_R2, "v0", m_r[ 2 ] );
	state_add( PSXCPU_R3, "v1", m_r[ 3 ] );
	state_add( PSXCPU_R4, "a0", m_r[ 4 ] );
	state_add( PSXCPU_R5, "a1", m_r[ 5 ] );
	state_add( PSXCPU_R6, "a2", m_r[ 6 ] );
	state_add( PSXCPU_R7, "a3", m_r[ 7 ] );
	state_add( PSXCPU_R8, "t0", m_r[ 8 ] );
	state_add( PSXCPU_R9, "t1", m_r[ 9 ] );
	state_add( PSXCPU_R10, "t2", m_r[ 10 ] );
	state_add( PSXCPU_R11, "t3", m_r[ 11 ] );
	state_add( PSXCPU_R12, "t4", m_r[ 12 ] );
	state_add( PSXCPU_R13, "t5", m_r[ 13 ] );
	state_add( PSXCPU_R14, "t6", m_r[ 14 ] );
	state_add( PSXCPU_R15, "t7", m_r[ 15 ] );
	state_add( PSXCPU_R16, "s0", m_r[ 16 ] );
	state_add( PSXCPU_R17, "s1", m_r[ 17 ] );
	state_add( PSXCPU_R18, "s2", m_r[ 18 ] );
	state_add( PSXCPU_R19, "s3", m_r[ 19 ] );
	state_add( PSXCPU_R20, "s4", m_r[ 20 ] );
	state_add( PSXCPU_R21, "s5", m_r[ 21 ] );
	state_add( PSXCPU_R22, "s6", m_r[ 22 ] );
	state_add( PSXCPU_R23, "s7", m_r[ 23 ] );
	state_add( PSXCPU_R24, "t8", m_r[ 24 ] );
	state_add( PSXCPU_R25, "t9", m_r[ 25 ] );
	state_add( PSXCPU_R26, "k0", m_r[ 26 ] );
	state_add( PSXCPU_R27, "k1", m_r[ 27 ] );
	state_add( PSXCPU_R28, "gp", m_r[ 28 ] );
	state_add( PSXCPU_R29, "sp", m_r[ 29 ] );
	state_add( PSXCPU_R30, "fp", m_r[ 30 ] );
	state_add( PSXCPU_R31, "ra", m_r[ 31 ] );
	state_add( PSXCPU_CP0R0, "!Index", m_cp0r[ 0 ] );
	state_add( PSXCPU_CP0R1, "!Random", m_cp0r[ 1 ] );
	state_add( PSXCPU_CP0R2, "!EntryLo", m_cp0r[ 2 ] );
	state_add( PSXCPU_CP0R3, "BPC", m_cp0r[ 3 ] );
	state_add( PSXCPU_CP0R4, "!Context", m_cp0r[ 4 ] );
	state_add( PSXCPU_CP0R5, "BDA", m_cp0r[ 5 ] );
	state_add( PSXCPU_CP0R6, "TAR", m_cp0r[ 6 ] );
	state_add( PSXCPU_CP0R7, "DCIC", m_cp0r[ 7 ] );
	state_add( PSXCPU_CP0R8, "BadA", m_cp0r[ 8 ] );
	state_add( PSXCPU_CP0R9, "BDAM", m_cp0r[ 9 ] );
	state_add( PSXCPU_CP0R10, "!EntryHi", m_cp0r[ 10 ] );
	state_add( PSXCPU_CP0R11, "BPCM", m_cp0r[ 11 ] );
	state_add( PSXCPU_CP0R12, "SR", m_cp0r[ 12 ] ).callimport();
	state_add( PSXCPU_CP0R13, "Cause", m_cp0r[ 13 ] ).callimport();
	state_add( PSXCPU_CP0R14, "EPC", m_cp0r[ 14 ] );
	state_add( PSXCPU_CP0R15, "PRId", m_cp0r[ 15 ] );
	state_add( PSXCPU_CP2DR0, "vxy0", m_gte.m_cp2dr[ 0 ].d );
	state_add( PSXCPU_CP2DR1, "vz0", m_gte.m_cp2dr[ 1 ].d );
	state_add( PSXCPU_CP2DR2, "vxy1", m_gte.m_cp2dr[ 2 ].d );
	state_add( PSXCPU_CP2DR3, "vz1", m_gte.m_cp2dr[ 3 ].d );
	state_add( PSXCPU_CP2DR4, "vxy2", m_gte.m_cp2dr[ 4 ].d );
	state_add( PSXCPU_CP2DR5, "vz2", m_gte.m_cp2dr[ 5 ].d );
	state_add( PSXCPU_CP2DR6, "rgb", m_gte.m_cp2dr[ 6 ].d );
	state_add( PSXCPU_CP2DR7, "otz", m_gte.m_cp2dr[ 7 ].d );
	state_add( PSXCPU_CP2DR8, "ir0", m_gte.m_cp2dr[ 8 ].d );
	state_add( PSXCPU_CP2DR9, "ir1", m_gte.m_cp2dr[ 9 ].d );
	state_add( PSXCPU_CP2DR10, "ir2", m_gte.m_cp2dr[ 10 ].d );
	state_add( PSXCPU_CP2DR11, "ir3", m_gte.m_cp2dr[ 11 ].d );
	state_add( PSXCPU_CP2DR12, "sxy0", m_gte.m_cp2dr[ 12 ].d );
	state_add( PSXCPU_CP2DR13, "sxy1", m_gte.m_cp2dr[ 13 ].d );
	state_add( PSXCPU_CP2DR14, "sxy2", m_gte.m_cp2dr[ 14 ].d );
	state_add( PSXCPU_CP2DR15, "sxyp", m_gte.m_cp2dr[ 15 ].d );
	state_add( PSXCPU_CP2DR16, "sz0", m_gte.m_cp2dr[ 16 ].d );
	state_add( PSXCPU_CP2DR17, "sz1", m_gte.m_cp2dr[ 17 ].d );
	state_add( PSXCPU_CP2DR18, "sz2", m_gte.m_cp2dr[ 18 ].d );
	state_add( PSXCPU_CP2DR19, "sz3", m_gte.m_cp2dr[ 19 ].d );
	state_add( PSXCPU_CP2DR20, "rgb0", m_gte.m_cp2dr[ 20 ].d );
	state_add( PSXCPU_CP2DR21, "rgb1", m_gte.m_cp2dr[ 21 ].d );
	state_add( PSXCPU_CP2DR22, "rgb2", m_gte.m_cp2dr[ 22 ].d );
	state_add( PSXCPU_CP2DR23, "res1", m_gte.m_cp2dr[ 23 ].d );
	state_add( PSXCPU_CP2DR24, "mac0", m_gte.m_cp2dr[ 24 ].d );
	state_add( PSXCPU_CP2DR25, "mac1", m_gte.m_cp2dr[ 25 ].d );
	state_add( PSXCPU_CP2DR26, "mac2", m_gte.m_cp2dr[ 26 ].d );
	state_add( PSXCPU_CP2DR27, "mac3", m_gte.m_cp2dr[ 27 ].d );
	state_add( PSXCPU_CP2DR28, "irgb", m_gte.m_cp2dr[ 28 ].d );
	state_add( PSXCPU_CP2DR29, "orgb", m_gte.m_cp2dr[ 29 ].d );
	state_add( PSXCPU_CP2DR30, "lzcs", m_gte.m_cp2dr[ 30 ].d );
	state_add( PSXCPU_CP2DR31, "lzcr", m_gte.m_cp2dr[ 31 ].d );
	state_add( PSXCPU_CP2CR0, "r11r12", m_gte.m_cp2cr[ 0 ].d );
	state_add( PSXCPU_CP2CR1, "r13r21", m_gte.m_cp2cr[ 1 ].d );
	state_add( PSXCPU_CP2CR2, "r22r23", m_gte.m_cp2cr[ 2 ].d );
	state_add( PSXCPU_CP2CR3, "r31r32", m_gte.m_cp2cr[ 3 ].d );
	state_add( PSXCPU_CP2CR4, "r33", m_gte.m_cp2cr[ 4 ].d );
	state_add( PSXCPU_CP2CR5, "trx", m_gte.m_cp2cr[ 5 ].d );
	state_add( PSXCPU_CP2CR6, "try", m_gte.m_cp2cr[ 6 ].d );
	state_add( PSXCPU_CP2CR7, "trz", m_gte.m_cp2cr[ 7 ].d );
	state_add( PSXCPU_CP2CR8, "l11l12", m_gte.m_cp2cr[ 8 ].d );
	state_add( PSXCPU_CP2CR9, "l13l21", m_gte.m_cp2cr[ 9 ].d );
	state_add( PSXCPU_CP2CR10, "l22l23", m_gte.m_cp2cr[ 10 ].d );
	state_add( PSXCPU_CP2CR11, "l31l32", m_gte.m_cp2cr[ 11 ].d );
	state_add( PSXCPU_CP2CR12, "l33", m_gte.m_cp2cr[ 12 ].d );
	state_add( PSXCPU_CP2CR13, "rbk", m_gte.m_cp2cr[ 13 ].d );
	state_add( PSXCPU_CP2CR14, "gbk", m_gte.m_cp2cr[ 14 ].d );
	state_add( PSXCPU_CP2CR15, "bbk", m_gte.m_cp2cr[ 15 ].d );
	state_add( PSXCPU_CP2CR16, "lr1lr2", m_gte.m_cp2cr[ 16 ].d );
	state_add( PSXCPU_CP2CR17, "lr31g1", m_gte.m_cp2cr[ 17 ].d );
	state_add( PSXCPU_CP2CR18, "lg2lg3", m_gte.m_cp2cr[ 18 ].d );
	state_add( PSXCPU_CP2CR19, "lb1lb2", m_gte.m_cp2cr[ 19 ].d );
	state_add( PSXCPU_CP2CR20, "lb3", m_gte.m_cp2cr[ 20 ].d );
	state_add( PSXCPU_CP2CR21, "rfc", m_gte.m_cp2cr[ 21 ].d );
	state_add( PSXCPU_CP2CR22, "gfc", m_gte.m_cp2cr[ 22 ].d );
	state_add( PSXCPU_CP2CR23, "bfc", m_gte.m_cp2cr[ 23 ].d );
	state_add( PSXCPU_CP2CR24, "ofx", m_gte.m_cp2cr[ 24 ].d );
	state_add( PSXCPU_CP2CR25, "ofy", m_gte.m_cp2cr[ 25 ].d );
	state_add( PSXCPU_CP2CR26, "h", m_gte.m_cp2cr[ 26 ].d );
	state_add( PSXCPU_CP2CR27, "dqa", m_gte.m_cp2cr[ 27 ].d );
	state_add( PSXCPU_CP2CR28, "dqb", m_gte.m_cp2cr[ 28 ].d );
	state_add( PSXCPU_CP2CR29, "zsf3", m_gte.m_cp2cr[ 29 ].d );
	state_add( PSXCPU_CP2CR30, "zsf4", m_gte.m_cp2cr[ 30 ].d );
	state_add( PSXCPU_CP2CR31, "flag", m_gte.m_cp2cr[ 31 ].d );

	// set our instruction counter
	m_icountptr = &m_icount;

	m_gpu_read_handler.resolve_safe( 0 );
	m_gpu_write_handler.resolve_safe();
	m_spu_read_handler.resolve_safe( 0 );
	m_spu_write_handler.resolve_safe();
	m_cd_read_handler.resolve_safe( 0 );
	m_cd_write_handler.resolve_safe();

	m_rom = memregion( "rom" );
}


//-------------------------------------------------
//  device_reset - reset the device
//-------------------------------------------------

void psxcpu_device::device_reset()
{
	m_ram_config = 0x800;
	update_ram_config();

	m_rom_config = 0x00130000;
	update_rom_config();

	/// TODO: get dma to access ram through the memory map?
	psxdma_device *psxdma = subdevice<psxdma_device>( "dma" );
	psxdma->m_ram = (UINT32 *)m_ram->pointer();
	psxdma->m_ramsize = m_ram->size();

	m_delayr = 0;
	m_delayv = 0;
	m_berr = 0;
	m_biu = 0;

	m_multiplier_operation = MULTIPLIER_OPERATION_IDLE;

	m_r[ 0 ] = 0;

	m_cp0r[ CP0_SR ] = SR_BEV;
	m_cp0r[ CP0_CAUSE ] = 0x00000000;
	m_cp0r[ CP0_PRID ] = 0x00000002;
	m_cp0r[ CP0_DCIC ] = 0x00000000;
	m_cp0r[ CP0_BPCM ] = 0xffffffff;
	m_cp0r[ CP0_BDAM ] = 0xffffffff;

	update_memory_handlers();
	update_address_masks();
	update_scratchpad();

	set_pc( 0xbfc00000 );
}


//-------------------------------------------------
//  device_post_load - device-specific post-load
//-------------------------------------------------

void psxcpu_device::device_post_load()
{
	update_memory_handlers();
	update_address_masks();
	update_scratchpad();
}


//-------------------------------------------------
//  state_import - import state into the device,
//  after it has been set
//-------------------------------------------------

void psxcpu_device::state_import( const device_state_entry &entry )
{
	switch( entry.index() )
	{
	case PSXCPU_CP0R12: // SR
	case PSXCPU_CP0R13: // CAUSE
		update_cop0( entry.index() - PSXCPU_CP0R0 );
		break;
	}
}


//-------------------------------------------------
//  state_string_export - export state as a string
//  for the debugger
//-------------------------------------------------

void psxcpu_device::state_string_export( const device_state_entry &entry, std::string &str ) const
{
	switch( entry.index() )
	{
	case PSXCPU_DELAYR:
		if( m_delayr <= PSXCPU_DELAYR_NOTPC )
		{
			str = string_format("%02x %-3s", m_delayr, delayn[m_delayr]);
		}
		else
		{
			str = string_format("%02x ---", m_delayr);
		}
		break;
	}
}


//-------------------------------------------------
//  disasm_disassemble - call the disassembly
//  helper function
//-------------------------------------------------

offs_t psxcpu_device::disasm_disassemble( char *buffer, offs_t pc, const UINT8 *oprom, const UINT8 *opram, UINT32 options )
{
	return DasmPSXCPU( this, buffer, pc, opram );
}


UINT32 psxcpu_device::get_register_from_pipeline( int reg )
{
	if( m_delayr == reg )
	{
		return m_delayv;
	}

	return m_r[ reg ];
}

int psxcpu_device::cop0_usable()
{
	if( ( m_cp0r[ CP0_SR ] & SR_KUC ) != 0 && ( m_cp0r[ CP0_SR ] & SR_CU0 ) == 0 )
	{
		exception( EXC_CPU );

		return 0;
	}

	return 1;
}

void psxcpu_device::lwc( int cop, int sr_cu )
{
	UINT32 address = m_r[ INS_RS( m_op ) ] + PSXCPU_WORD_EXTEND( INS_IMMEDIATE( m_op ) );
	int breakpoint = load_data_address_breakpoint( address );

	if( ( m_cp0r[ CP0_SR ] & sr_cu ) == 0 )
	{
		exception( EXC_CPU );
	}
	else if( ( address & m_bad_word_address_mask ) != 0 )
	{
		load_bad_address( address );
	}
	else if( breakpoint )
	{
		breakpoint_exception();
	}
	else
	{
		UINT32 data = readword( address );

		if( m_berr )
		{
			load_bus_error_exception();
		}
		else
		{
			int reg = INS_RT( m_op );

			advance_pc();

			switch( cop )
			{
			case 0:
				/* lwc0 doesn't update any cop0 registers */
				break;

			case 1:
				setcp1dr( reg, data );
				break;

			case 2:
				m_gte.setcp2dr( m_pc, reg, data );
				break;

			case 3:
				setcp3dr( reg, data );
				break;
			}
		}
	}
}

void psxcpu_device::swc( int cop, int sr_cu )
{
	UINT32 address = m_r[ INS_RS( m_op ) ] + PSXCPU_WORD_EXTEND( INS_IMMEDIATE( m_op ) );
	int breakpoint = store_data_address_breakpoint( address );

	if( ( m_cp0r[ CP0_SR ] & sr_cu ) == 0 )
	{
		exception( EXC_CPU );
	}
	else if( ( address & m_bad_word_address_mask ) != 0 )
	{
		store_bad_address( address );
	}
	else
	{
		UINT32 data = 0;

		switch( cop )
		{
		case 0:
			{
				int address;

				if( m_delayr == PSXCPU_DELAYR_PC )
				{
					switch( m_delayv & 0x0c )
					{
					case 0x0c:
						address = m_delayv;
						break;

					default:
						address = m_delayv + 4;
						break;
					}
				}
				else
				{
					switch( m_pc & 0x0c )
					{
					case 0x0:
					case 0xc:
						address = m_pc + 0x08;
						break;

					default:
						address = m_pc | 0x0c;
						break;
					}
				}

				data = m_program->read_dword( address );
			}
			break;

		case 1:
			data = getcp1dr( INS_RT( m_op ) );
			break;

		case 2:
			data = m_gte.getcp2dr( m_pc, INS_RT( m_op ) );
			break;

		case 3:
			data = getcp3dr( INS_RT( m_op ) );
			break;
		}

		writeword( address, data );

		if( breakpoint )
		{
			breakpoint_exception();
		}
		else if( m_berr )
		{
			store_bus_error_exception();
		}
		else
		{
			advance_pc();
		}
	}
}

void psxcpu_device::bc( int cop, int sr_cu, int condition )
{
	if( ( m_cp0r[ CP0_SR ] & sr_cu ) == 0 )
	{
		exception( EXC_CPU );
	}
	else
	{
		conditional_branch( !condition );
	}
}



/***************************************************************************
    CORE EXECUTION LOOP
***************************************************************************/


void psxcpu_device::execute_set_input( int inputnum, int state )
{
	UINT32 ip;

	switch( inputnum )
	{
	case PSXCPU_IRQ0:
		ip = CAUSE_IP2;
		break;

	case PSXCPU_IRQ1:
		ip = CAUSE_IP3;
		break;

	case PSXCPU_IRQ2:
		ip = CAUSE_IP4;
		break;

	case PSXCPU_IRQ3:
		ip = CAUSE_IP5;
		break;

	case PSXCPU_IRQ4:
		ip = CAUSE_IP6;
		break;

	case PSXCPU_IRQ5:
		ip = CAUSE_IP7;
		break;

	default:
		return;
	}

	switch( state )
	{
	case CLEAR_LINE:
		m_cp0r[ CP0_CAUSE ] &= ~ip;
		break;

	case ASSERT_LINE:
		m_cp0r[ CP0_CAUSE ] |= ip;
		break;
	}

	update_cop0( CP0_CAUSE );
}


void psxcpu_device::execute_run()
{
	do
	{
		if( LOG_BIOSCALL ) log_bioscall();
		debugger_instruction_hook( this,  m_pc );

		m_op = m_direct->read_dword( m_pc );

		if( m_berr )
		{
			fetch_bus_error_exception();
		}
		else
		{
			switch( INS_OP( m_op ) )
			{
			case OP_SPECIAL:
				switch( INS_FUNCT( m_op ) )
				{
				case FUNCT_SLL:
					load( INS_RD( m_op ), m_r[ INS_RT( m_op ) ] << INS_SHAMT( m_op ) );
					break;

				case FUNCT_SRL:
					load( INS_RD( m_op ), m_r[ INS_RT( m_op ) ] >> INS_SHAMT( m_op ) );
					break;

				case FUNCT_SRA:
					load( INS_RD( m_op ), (INT32)m_r[ INS_RT( m_op ) ] >> INS_SHAMT( m_op ) );
					break;

				case FUNCT_SLLV:
					load( INS_RD( m_op ), m_r[ INS_RT( m_op ) ] << ( m_r[ INS_RS( m_op ) ] & 31 ) );
					break;

				case FUNCT_SRLV:
					load( INS_RD( m_op ), m_r[ INS_RT( m_op ) ] >> ( m_r[ INS_RS( m_op ) ] & 31 ) );
					break;

				case FUNCT_SRAV:
					load( INS_RD( m_op ), (INT32)m_r[ INS_RT( m_op ) ] >> ( m_r[ INS_RS( m_op ) ] & 31 ) );
					break;

				case FUNCT_JR:
					branch( m_r[ INS_RS( m_op ) ] );
					break;

				case FUNCT_JALR:
					branch( m_r[ INS_RS( m_op ) ] );
					if( INS_RD( m_op ) != 0 )
					{
						m_r[ INS_RD( m_op ) ] = m_pc + 4;
					}
					break;

				case FUNCT_SYSCALL:
					if( LOG_BIOSCALL ) log_syscall();
					exception( EXC_SYS );
					break;

				case FUNCT_BREAK:
					exception( EXC_BP );
					break;

				case FUNCT_MFHI:
					load( INS_RD( m_op ), get_hi() );
					break;

				case FUNCT_MTHI:
					funct_mthi();
					advance_pc();
					break;

				case FUNCT_MFLO:
					load( INS_RD( m_op ), get_lo() );
					break;

				case FUNCT_MTLO:
					funct_mtlo();
					advance_pc();
					break;

				case FUNCT_MULT:
					funct_mult();
					advance_pc();
					break;

				case FUNCT_MULTU:
					funct_multu();
					advance_pc();
					break;

				case FUNCT_DIV:
					funct_div();
					advance_pc();
					break;

				case FUNCT_DIVU:
					funct_divu();
					advance_pc();
					break;

				case FUNCT_ADD:
					{
						UINT32 result = m_r[ INS_RS( m_op ) ] + m_r[ INS_RT( m_op ) ];
						if( (INT32)( ~( m_r[ INS_RS( m_op ) ] ^ m_r[ INS_RT( m_op ) ] ) & ( m_r[ INS_RS( m_op ) ] ^ result ) ) < 0 )
						{
							exception( EXC_OVF );
						}
						else
						{
							load( INS_RD( m_op ), result );
						}
					}
					break;

				case FUNCT_ADDU:
					load( INS_RD( m_op ), m_r[ INS_RS( m_op ) ] + m_r[ INS_RT( m_op ) ] );
					break;

				case FUNCT_SUB:
					{
						UINT32 result = m_r[ INS_RS( m_op ) ] - m_r[ INS_RT( m_op ) ];
						if( (INT32)( ( m_r[ INS_RS( m_op ) ] ^ m_r[ INS_RT( m_op ) ] ) & ( m_r[ INS_RS( m_op ) ] ^ result ) ) < 0 )
						{
							exception( EXC_OVF );
						}
						else
						{
							load( INS_RD( m_op ), result );
						}
					}
					break;

				case FUNCT_SUBU:
					load( INS_RD( m_op ), m_r[ INS_RS( m_op ) ] - m_r[ INS_RT( m_op ) ] );
					break;

				case FUNCT_AND:
					load( INS_RD( m_op ), m_r[ INS_RS( m_op ) ] & m_r[ INS_RT( m_op ) ] );
					break;

				case FUNCT_OR:
					load( INS_RD( m_op ), m_r[ INS_RS( m_op ) ] | m_r[ INS_RT( m_op ) ] );
					break;

				case FUNCT_XOR:
					load( INS_RD( m_op ), m_r[ INS_RS( m_op ) ] ^ m_r[ INS_RT( m_op ) ] );
					break;

				case FUNCT_NOR:
					load( INS_RD( m_op ), ~( m_r[ INS_RS( m_op ) ] | m_r[ INS_RT( m_op ) ] ) );
					break;

				case FUNCT_SLT:
					load( INS_RD( m_op ), (INT32)m_r[ INS_RS( m_op ) ] < (INT32)m_r[ INS_RT( m_op ) ] );
					break;

				case FUNCT_SLTU:
					load( INS_RD( m_op ), m_r[ INS_RS( m_op ) ] < m_r[ INS_RT( m_op ) ] );
					break;

				default:
					exception( EXC_RI );
					break;
				}
				break;

			case OP_REGIMM:
				switch( INS_RT_REGIMM( m_op ) )
				{
				case RT_BLTZ:
					conditional_branch( (INT32)m_r[ INS_RS( m_op ) ] < 0 );

					if( INS_RT( m_op ) == RT_BLTZAL )
					{
						m_r[ 31 ] = m_pc + 4;
					}
					break;

				case RT_BGEZ:
					conditional_branch( (INT32)m_r[ INS_RS( m_op ) ] >= 0 );

					if( INS_RT( m_op ) == RT_BGEZAL )
					{
						m_r[ 31 ] = m_pc + 4;
					}
					break;
				}
				break;

			case OP_J:
				unconditional_branch();
				break;

			case OP_JAL:
				unconditional_branch();
				m_r[ 31 ] = m_pc + 4;
				break;

			case OP_BEQ:
				conditional_branch( m_r[ INS_RS( m_op ) ] == m_r[ INS_RT( m_op ) ] );
				break;

			case OP_BNE:
				conditional_branch( m_r[ INS_RS( m_op ) ] != m_r[ INS_RT( m_op ) ] );
				break;

			case OP_BLEZ:
				conditional_branch( (INT32)m_r[ INS_RS( m_op ) ] < 0 || m_r[ INS_RS( m_op ) ] == m_r[ INS_RT( m_op ) ] );
				break;

			case OP_BGTZ:
				conditional_branch( (INT32)m_r[ INS_RS( m_op ) ] >= 0 && m_r[ INS_RS( m_op ) ] != m_r[ INS_RT( m_op ) ] );
				break;

			case OP_ADDI:
				{
					UINT32 immediate = PSXCPU_WORD_EXTEND( INS_IMMEDIATE( m_op ) );
					UINT32 result = m_r[ INS_RS( m_op ) ] + immediate;
					if( (INT32)( ~( m_r[ INS_RS( m_op ) ] ^ immediate ) & ( m_r[ INS_RS( m_op ) ] ^ result ) ) < 0 )
					{
						exception( EXC_OVF );
					}
					else
					{
						load( INS_RT( m_op ), result );
					}
				}
				break;

			case OP_ADDIU:
				load( INS_RT( m_op ), m_r[ INS_RS( m_op ) ] + PSXCPU_WORD_EXTEND( INS_IMMEDIATE( m_op ) ) );
				break;

			case OP_SLTI:
				load( INS_RT( m_op ), (INT32)m_r[ INS_RS( m_op ) ] < PSXCPU_WORD_EXTEND( INS_IMMEDIATE( m_op ) ) );
				break;

			case OP_SLTIU:
				load( INS_RT( m_op ), m_r[ INS_RS( m_op ) ] < (UINT32)PSXCPU_WORD_EXTEND( INS_IMMEDIATE( m_op ) ) );
				break;

			case OP_ANDI:
				load( INS_RT( m_op ), m_r[ INS_RS( m_op ) ] & INS_IMMEDIATE( m_op ) );
				break;

			case OP_ORI:
				load( INS_RT( m_op ), m_r[ INS_RS( m_op ) ] | INS_IMMEDIATE( m_op ) );
				break;

			case OP_XORI:
				load( INS_RT( m_op ), m_r[ INS_RS( m_op ) ] ^ INS_IMMEDIATE( m_op ) );
				break;

			case OP_LUI:
				load( INS_RT( m_op ), INS_IMMEDIATE( m_op ) << 16 );
				break;

			case OP_COP0:
				switch( INS_RS( m_op ) )
				{
				case RS_MFC:
					{
						int reg = INS_RD( m_op );

						if( reg == CP0_INDEX ||
							reg == CP0_RANDOM ||
							reg == CP0_ENTRYLO ||
							reg == CP0_CONTEXT ||
							reg == CP0_ENTRYHI )
						{
							exception( EXC_RI );
						}
						else if( reg < 16 )
						{
							if( cop0_usable() )
							{
								delayed_load( INS_RT( m_op ), m_cp0r[ reg ] );
							}
						}
						else
						{
							advance_pc();
						}
					}
					break;

				case RS_CFC:
					exception( EXC_RI );
					break;

				case RS_MTC:
					{
						int reg = INS_RD( m_op );

						if( reg == CP0_INDEX ||
							reg == CP0_RANDOM ||
							reg == CP0_ENTRYLO ||
							reg == CP0_CONTEXT ||
							reg == CP0_ENTRYHI )
						{
							exception( EXC_RI );
						}
						else if( reg < 16 )
						{
							if( cop0_usable() )
							{
								UINT32 data = ( m_cp0r[ reg ] & ~mtc0_writemask[ reg ] ) |
									( m_r[ INS_RT( m_op ) ] & mtc0_writemask[ reg ] );
								advance_pc();

								m_cp0r[ reg ] = data;
								update_cop0( reg );
							}
						}
						else
						{
							advance_pc();
						}
					}
					break;

				case RS_CTC:
					exception( EXC_RI );
					break;

				case RS_BC:
				case RS_BC_ALT:
					switch( INS_BC( m_op ) )
					{
					case BC_BCF:
						bc( 0, SR_CU0, 0 );
						break;

					case BC_BCT:
						bc( 0, SR_CU0, 1 );
						break;
					}
					break;

				default:
					switch( INS_CO( m_op ) )
					{
					case 1:
						switch( INS_CF( m_op ) )
						{
						case CF_TLBR:
						case CF_TLBWI:
						case CF_TLBWR:
						case CF_TLBP:
							exception( EXC_RI );
							break;

						case CF_RFE:
							if( cop0_usable() )
							{
								advance_pc();
								m_cp0r[ CP0_SR ] = ( m_cp0r[ CP0_SR ] & ~0xf ) | ( ( m_cp0r[ CP0_SR ] >> 2 ) & 0xf );
								update_cop0( CP0_SR );
							}
							break;

						default:
							advance_pc();
							break;
						}
						break;

					default:
						advance_pc();
						break;
					}
					break;
				}
				break;

			case OP_COP1:
				if( ( m_cp0r[ CP0_SR ] & SR_CU1 ) == 0 )
				{
					exception( EXC_CPU );
				}
				else
				{
					switch( INS_RS( m_op ) )
					{
					case RS_MFC:
						delayed_load( INS_RT( m_op ), getcp1dr( INS_RD( m_op ) ) );
						break;

					case RS_CFC:
						delayed_load( INS_RT( m_op ), getcp1cr( INS_RD( m_op ) ) );
						break;

					case RS_MTC:
						setcp1dr( INS_RD( m_op ), m_r[ INS_RT( m_op ) ] );
						advance_pc();
						break;

					case RS_CTC:
						setcp1cr( INS_RD( m_op ), m_r[ INS_RT( m_op ) ] );
						advance_pc();
						break;

					case RS_BC:
					case RS_BC_ALT:
						switch( INS_BC( m_op ) )
						{
						case BC_BCF:
							bc( 1, SR_CU1, 0 );
							break;

						case BC_BCT:
							bc( 1, SR_CU1, 1 );
							break;
						}
						break;

					default:
						advance_pc();
						break;
					}
				}
				break;

			case OP_COP2:
				if( ( m_cp0r[ CP0_SR ] & SR_CU2 ) == 0 )
				{
					exception( EXC_CPU );
				}
				else
				{
					switch( INS_RS( m_op ) )
					{
					case RS_MFC:
						delayed_load( INS_RT( m_op ), m_gte.getcp2dr( m_pc, INS_RD( m_op ) ) );
						break;

					case RS_CFC:
						delayed_load( INS_RT( m_op ), m_gte.getcp2cr( m_pc, INS_RD( m_op ) ) );
						break;

					case RS_MTC:
						m_gte.setcp2dr( m_pc, INS_RD( m_op ), m_r[ INS_RT( m_op ) ] );
						advance_pc();
						break;

					case RS_CTC:
						m_gte.setcp2cr( m_pc, INS_RD( m_op ), m_r[ INS_RT( m_op ) ] );
						advance_pc();
						break;

					case RS_BC:
					case RS_BC_ALT:
						switch( INS_BC( m_op ) )
						{
						case BC_BCF:
							bc( 2, SR_CU2, 0 );
							break;

						case BC_BCT:
							bc( 2, SR_CU2, 1 );
							break;
						}
						break;

					default:
						switch( INS_CO( m_op ) )
						{
						case 1:
							if( !m_gte.docop2( m_pc, INS_COFUN( m_op ) ) )
							{
								stop();
							}

							advance_pc();
							break;

						default:
							advance_pc();
							break;
						}
						break;
					}
				}
				break;

			case OP_COP3:
				if( ( m_cp0r[ CP0_SR ] & SR_CU3 ) == 0 )
				{
					exception( EXC_CPU );
				}
				else
				{
					switch( INS_RS( m_op ) )
					{
					case RS_MFC:
						delayed_load( INS_RT( m_op ), getcp3dr( INS_RD( m_op ) ) );
						break;

					case RS_CFC:
						delayed_load( INS_RT( m_op ), getcp3cr( INS_RD( m_op ) ) );
						break;

					case RS_MTC:
						setcp3dr( INS_RD( m_op ), m_r[ INS_RT( m_op ) ] );
						advance_pc();
						break;

					case RS_CTC:
						setcp3cr( INS_RD( m_op ), m_r[ INS_RT( m_op ) ] );
						advance_pc();
						break;

					case RS_BC:
					case RS_BC_ALT:
						switch( INS_BC( m_op ) )
						{
						case BC_BCF:
							bc( 3, SR_CU3, 0 );
							break;

						case BC_BCT:
							bc( 3, SR_CU3, 1 );
							break;
						}
						break;

					default:
						advance_pc();
						break;
					}
				}
				break;

			case OP_LB:
				{
					UINT32 address = m_r[ INS_RS( m_op ) ] + PSXCPU_WORD_EXTEND( INS_IMMEDIATE( m_op ) );
					int breakpoint = load_data_address_breakpoint( address );

					if( ( address & m_bad_byte_address_mask ) != 0 )
					{
						load_bad_address( address );
					}
					else if( breakpoint )
					{
						breakpoint_exception();
					}
					else
					{
						UINT32 data = PSXCPU_BYTE_EXTEND( readbyte( address ) );

						if( m_berr )
						{
							load_bus_error_exception();
						}
						else
						{
							delayed_load( INS_RT( m_op ), data );
						}
					}
				}
				break;

			case OP_LH:
				{
					UINT32 address = m_r[ INS_RS( m_op ) ] + PSXCPU_WORD_EXTEND( INS_IMMEDIATE( m_op ) );
					int breakpoint = load_data_address_breakpoint( address );

					if( ( address & m_bad_half_address_mask ) != 0 )
					{
						load_bad_address( address );
					}
					else if( breakpoint )
					{
						breakpoint_exception();
					}
					else
					{
						UINT32 data = PSXCPU_WORD_EXTEND( readhalf( address ) );

						if( m_berr )
						{
							load_bus_error_exception();
						}
						else
						{
							delayed_load( INS_RT( m_op ), data );
						}
					}
				}
				break;

			case OP_LWL:
				{
					UINT32 address = m_r[ INS_RS( m_op ) ] + PSXCPU_WORD_EXTEND( INS_IMMEDIATE( m_op ) );
					int load_type = address & 3;
					int breakpoint;

					address &= ~3;
					breakpoint = load_data_address_breakpoint( address );

					if( ( address & m_bad_byte_address_mask ) != 0 )
					{
						load_bad_address( address );
					}
					else if( breakpoint )
					{
						breakpoint_exception();
					}
					else
					{
						UINT32 data = get_register_from_pipeline( INS_RT( m_op ) );

						switch( load_type )
						{
						case 0:
							data = ( data & 0x00ffffff ) | ( readword_masked( address, 0x000000ff ) << 24 );
							break;

						case 1:
							data = ( data & 0x0000ffff ) | ( readword_masked( address, 0x0000ffff ) << 16 );
							break;

						case 2:
							data = ( data & 0x000000ff ) | ( readword_masked( address, 0x00ffffff ) << 8 );
							break;

						case 3:
							data = readword( address );
							break;
						}

						if( m_berr )
						{
							load_bus_error_exception();
						}
						else
						{
							delayed_load( INS_RT( m_op ), data );
						}
					}
				}
				break;

			case OP_LW:
				{
					UINT32 address = m_r[ INS_RS( m_op ) ] + PSXCPU_WORD_EXTEND( INS_IMMEDIATE( m_op ) );
					int breakpoint = load_data_address_breakpoint( address );

					if( ( address & m_bad_word_address_mask ) != 0 )
					{
						load_bad_address( address );
					}
					else if( breakpoint )
					{
						breakpoint_exception();
					}
					else
					{
						UINT32 data = readword( address );

						if( m_berr )
						{
							load_bus_error_exception();
						}
						else
						{
							delayed_load( INS_RT( m_op ), data );
						}
					}
				}
				break;

			case OP_LBU:
				{
					UINT32 address = m_r[ INS_RS( m_op ) ] + PSXCPU_WORD_EXTEND( INS_IMMEDIATE( m_op ) );
					int breakpoint = load_data_address_breakpoint( address );

					if( ( address & m_bad_byte_address_mask ) != 0 )
					{
						load_bad_address( address );
					}
					else if( breakpoint )
					{
						breakpoint_exception();
					}
					else
					{
						UINT32 data = readbyte( address );

						if( m_berr )
						{
							load_bus_error_exception();
						}
						else
						{
							delayed_load( INS_RT( m_op ), data );
						}
					}
				}
				break;

			case OP_LHU:
				{
					UINT32 address = m_r[ INS_RS( m_op ) ] + PSXCPU_WORD_EXTEND( INS_IMMEDIATE( m_op ) );
					int breakpoint = load_data_address_breakpoint( address );

					if( ( address & m_bad_half_address_mask ) != 0 )
					{
						load_bad_address( address );
					}
					else if( breakpoint )
					{
						breakpoint_exception();
					}
					else
					{
						UINT32 data = readhalf( address );

						if( m_berr )
						{
							load_bus_error_exception();
						}
						else
						{
							delayed_load( INS_RT( m_op ), data );
						}
					}
				}
				break;

			case OP_LWR:
				{
					UINT32 address = m_r[ INS_RS( m_op ) ] + PSXCPU_WORD_EXTEND( INS_IMMEDIATE( m_op ) );
					int breakpoint = load_data_address_breakpoint( address );

					if( ( address & m_bad_byte_address_mask ) != 0 )
					{
						load_bad_address( address );
					}
					else if( breakpoint )
					{
						breakpoint_exception();
					}
					else
					{
						UINT32 data = get_register_from_pipeline( INS_RT( m_op ) );

						switch( address & 3 )
						{
						case 0:
							data = readword( address );
							break;

						case 1:
							data = ( data & 0xff000000 ) | ( readword_masked( address, 0xffffff00 ) >> 8 );
							break;

						case 2:
							data = ( data & 0xffff0000 ) | ( readword_masked( address, 0xffff0000 ) >> 16 );
							break;

						case 3:
							data = ( data & 0xffffff00 ) | ( readword_masked( address, 0xff000000 ) >> 24 );
							break;
						}

						if( m_berr )
						{
							load_bus_error_exception();
						}
						else
						{
							delayed_load( INS_RT( m_op ), data );
						}
					}
				}
				break;

			case OP_SB:
				{
					UINT32 address = m_r[ INS_RS( m_op ) ] + PSXCPU_WORD_EXTEND( INS_IMMEDIATE( m_op ) );
					int breakpoint = store_data_address_breakpoint( address );

					if( ( address & m_bad_byte_address_mask ) != 0 )
					{
						store_bad_address( address );
					}
					else
					{
						int shift = 8 * ( address & 3 );
						writeword_masked( address, m_r[ INS_RT( m_op ) ] << shift, 0xff << shift );

						if( breakpoint )
						{
							breakpoint_exception();
						}
						else if( m_berr )
						{
							store_bus_error_exception();
						}
						else
						{
							advance_pc();
						}
					}
				}
				break;

			case OP_SH:
				{
					UINT32 address = m_r[ INS_RS( m_op ) ] + PSXCPU_WORD_EXTEND( INS_IMMEDIATE( m_op ) );
					int breakpoint = store_data_address_breakpoint( address );

					if( ( address & m_bad_half_address_mask ) != 0 )
					{
						store_bad_address( address );
					}
					else
					{
						int shift = 8 * ( address & 2 );
						writeword_masked( address, m_r[ INS_RT( m_op ) ] << shift, 0xffff << shift );

						if( breakpoint )
						{
							breakpoint_exception();
						}
						else if( m_berr )
						{
							store_bus_error_exception();
						}
						else
						{
							advance_pc();
						}
					}
				}
				break;

			case OP_SWL:
				{
					UINT32 address = m_r[ INS_RS( m_op ) ] + PSXCPU_WORD_EXTEND( INS_IMMEDIATE( m_op ) );
					int save_type = address & 3;
					int breakpoint;

					address &= ~3;
					breakpoint = store_data_address_breakpoint( address );

					if( ( address & m_bad_byte_address_mask ) != 0 )
					{
						store_bad_address( address );
					}
					else
					{
						switch( save_type )
						{
						case 0:
							writeword_masked( address, m_r[ INS_RT( m_op ) ] >> 24, 0x000000ff );
							break;

						case 1:
							writeword_masked( address, m_r[ INS_RT( m_op ) ] >> 16, 0x0000ffff );
							break;

						case 2:
							writeword_masked( address, m_r[ INS_RT( m_op ) ] >> 8, 0x00ffffff );
							break;

						case 3:
							writeword( address, m_r[ INS_RT( m_op ) ] );
							break;
						}

						if( breakpoint )
						{
							breakpoint_exception();
						}
						else if( m_berr )
						{
							store_bus_error_exception();
						}
						else
						{
							advance_pc();
						}
					}
				}
				break;

			case OP_SW:
				{
					UINT32 address = m_r[ INS_RS( m_op ) ] + PSXCPU_WORD_EXTEND( INS_IMMEDIATE( m_op ) );
					int breakpoint = store_data_address_breakpoint( address );

					if( ( address & m_bad_word_address_mask ) != 0 )
					{
						store_bad_address( address );
					}
					else
					{
						writeword( address, m_r[ INS_RT( m_op ) ] );

						if( breakpoint )
						{
							breakpoint_exception();
						}
						else if( m_berr )
						{
							store_bus_error_exception();
						}
						else
						{
							advance_pc();
						}
					}
				}
				break;

			case OP_SWR:
				{
					UINT32 address = m_r[ INS_RS( m_op ) ] + PSXCPU_WORD_EXTEND( INS_IMMEDIATE( m_op ) );
					int breakpoint = store_data_address_breakpoint( address );

					if( ( address & m_bad_byte_address_mask ) != 0 )
					{
						store_bad_address( address );
					}
					else
					{
						switch( address & 3 )
						{
						case 0:
							writeword( address, m_r[ INS_RT( m_op ) ] );
							break;

						case 1:
							writeword_masked( address, m_r[ INS_RT( m_op ) ] << 8, 0xffffff00 );
							break;

						case 2:
							writeword_masked( address, m_r[ INS_RT( m_op ) ] << 16, 0xffff0000 );
							break;

						case 3:
							writeword_masked( address, m_r[ INS_RT( m_op ) ] << 24, 0xff000000 );
							break;
						}

						if( breakpoint )
						{
							breakpoint_exception();
						}
						else if( m_berr )
						{
							store_bus_error_exception();
						}
						else
						{
							advance_pc();
						}
					}
				}
				break;

			case OP_LWC0:
				lwc( 0, SR_CU0 );
				break;

			case OP_LWC1:
				lwc( 1, SR_CU1 );
				break;

			case OP_LWC2:
				lwc( 2, SR_CU2 );
				break;

			case OP_LWC3:
				lwc( 3, SR_CU3 );
				break;

			case OP_SWC0:
				swc( 0, SR_CU0 );
				break;

			case OP_SWC1:
				swc( 1, SR_CU1 );
				break;

			case OP_SWC2:
				swc( 2, SR_CU2 );
				break;

			case OP_SWC3:
				swc( 3, SR_CU3 );
				break;

			default:
				logerror( "%08x: unknown opcode %08x\n", m_pc, m_op );
				stop();
				exception( EXC_RI );
				break;
			}
		}

		m_icount--;
	} while( m_icount > 0 );
}

UINT32 psxcpu_device::getcp1dr( int reg )
{
	/* if a mtc/ctc precedes then this will get the value moved (which cop1 register is irrelevant). */
	/* if a mfc/cfc follows then it will get the same value as this one. */
	return m_program->read_dword( m_pc + 4 );
}

void psxcpu_device::setcp1dr( int reg, UINT32 value )
{
}

UINT32 psxcpu_device::getcp1cr( int reg )
{
	/* if a mtc/ctc precedes then this will get the value moved (which cop1 register is irrelevant). */
	/* if a mfc/cfc follows then it will get the same value as this one. */
	return m_program->read_dword( m_pc + 4 );
}

void psxcpu_device::setcp1cr( int reg, UINT32 value )
{
}


UINT32 psxcpu_device::getcp3dr( int reg )
{
	/* if you have mtc/ctc with an mfc/cfc directly afterwards then you get the value that was moved. */
	/* if you have an lwc with an mfc/cfc somewhere after it then you get the value that is loaded */
	/* otherwise you get the next opcode. which register you transfer to or from is irrelevant. */
	return m_program->read_dword( m_pc + 4 );
}

void psxcpu_device::setcp3dr( int reg, UINT32 value )
{
}

UINT32 psxcpu_device::getcp3cr( int reg )
{
	/* if you have mtc/ctc with an mfc/cfc directly afterwards then you get the value that was moved. */
	/* if you have an lwc with an mfc/cfc somewhere after it then you get the value that is loaded */
	/* otherwise you get the next opcode. which register you transfer to or from is irrelevant. */
	return m_program->read_dword( m_pc + 4 );
}

void psxcpu_device::setcp3cr( int reg, UINT32 value )
{
}

psxcpu_device *psxcpu_device::getcpu( device_t &device, const char *cputag )
{
	return downcast<psxcpu_device *>( device.subdevice( cputag ) );
}

READ32_MEMBER( psxcpu_device::gpu_r )
{
	return m_gpu_read_handler( space, offset, mem_mask );
}

WRITE32_MEMBER( psxcpu_device::gpu_w )
{
	m_gpu_write_handler( space, offset, data, mem_mask );
}

READ16_MEMBER( psxcpu_device::spu_r )
{
	return m_spu_read_handler( space, offset, mem_mask );
}

WRITE16_MEMBER( psxcpu_device::spu_w )
{
	m_spu_write_handler( space, offset, data, mem_mask );
}

READ8_MEMBER( psxcpu_device::cd_r )
{
	return m_cd_read_handler( space, offset, mem_mask );
}

WRITE8_MEMBER( psxcpu_device::cd_w )
{
	m_cd_write_handler( space, offset, data, mem_mask );
}

void psxcpu_device::set_disable_rom_berr(bool mode)
{
	m_disable_rom_berr = mode;
}

static MACHINE_CONFIG_FRAGMENT( psx )
	MCFG_DEVICE_ADD( "irq", PSX_IRQ, 0 )
	MCFG_PSX_IRQ_HANDLER( INPUTLINE( DEVICE_SELF, PSXCPU_IRQ0 ) )

	MCFG_DEVICE_ADD( "dma", PSX_DMA, 0 )
	MCFG_PSX_DMA_IRQ_HANDLER( DEVWRITELINE("irq", psxirq_device, intin3 ) )

	MCFG_DEVICE_ADD( "mdec", PSX_MDEC, 0 )
	MCFG_PSX_DMA_CHANNEL_WRITE( DEVICE_SELF, 0, psx_dma_write_delegate( FUNC( psxmdec_device::dma_write ), (psxmdec_device *) device ) )
	MCFG_PSX_DMA_CHANNEL_READ( DEVICE_SELF, 1, psx_dma_read_delegate( FUNC( psxmdec_device::dma_read ), (psxmdec_device *) device ) )

	MCFG_DEVICE_ADD( "rcnt", PSX_RCNT, 0 )
	MCFG_PSX_RCNT_IRQ0_HANDLER( DEVWRITELINE( "irq", psxirq_device, intin4 ) )
	MCFG_PSX_RCNT_IRQ1_HANDLER( DEVWRITELINE( "irq", psxirq_device, intin5 ) )
	MCFG_PSX_RCNT_IRQ2_HANDLER( DEVWRITELINE( "irq", psxirq_device, intin6 ) )

	MCFG_DEVICE_ADD( "sio0", PSX_SIO0, 0 )
	MCFG_PSX_SIO_IRQ_HANDLER( DEVWRITELINE( "irq", psxirq_device, intin7 ) )

	MCFG_DEVICE_ADD( "sio1", PSX_SIO1, 0 )
	MCFG_PSX_SIO_IRQ_HANDLER( DEVWRITELINE( "irq", psxirq_device, intin8 ) )

	MCFG_RAM_ADD( "ram" )
	MCFG_RAM_DEFAULT_VALUE( 0x00 )
MACHINE_CONFIG_END

//-------------------------------------------------
//  machine_config_additions - return a pointer to
//  the device's machine fragment
//-------------------------------------------------

machine_config_constructor psxcpu_device::device_mconfig_additions() const
{
	return MACHINE_CONFIG_NAME( psx );
}
