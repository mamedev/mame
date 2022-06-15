// license:BSD-3-Clause
// copyright-holders:smf
/*
 * PlayStation CPU emulator
 *
 * Copyright 2003-2017 smf
 *
 * Known chip id's
 *   CXD8530AQ
 *   CXD8530BQ
 *   CXD8530CQ
 *   CXD8661R
 *   CXD8606BQ
 *   CXD8606CQ
 *
 * The PlayStation CPU is based on the LSI CoreWare CW33300 library, this was available packaged as an LSI LR33300.
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
 * MDEC is based on the LSI Jpeg CoreWare library (CW702?).
 *
 * Known limitations of the emulation:
 *
 *  Program counter, read data & write data break points are emulated, trace break points are not.
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
 *  Bus errors caused by instruction burst fetches are not supported.
 *
 */

#include "emu.h"
#include "psx.h"
#include "mdec.h"
#include "rcnt.h"
#include "sound/spu.h"

#include "psxdefs.h"

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

#define DCIC_DB ( 1L << 0 )
#define DCIC_PC ( 1L << 1 )
#define DCIC_DA ( 1L << 2 )
#define DCIC_R ( 1L << 3 )
#define DCIC_W ( 1L << 4 )
#define DCIC_T ( 1L << 5 ) // not emulated
// unknown ( 1L << 12 )
// unknown ( 1L << 13 )
// unknown ( 1L << 14 )
// unknown ( 1L << 15 )
#define DCIC_DE ( 1L << 23 )
#define DCIC_PCE ( 1L << 24 )
#define DCIC_DAE ( 1L << 25 )
#define DCIC_DR ( 1L << 26 )
#define DCIC_DW ( 1L << 27 )
#define DCIC_TE ( 1L << 28 ) // not emulated
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
// software interrupts
#define CAUSE_IP0 ( 1L << 8 )
#define CAUSE_IP1 ( 1L << 9 )
// hardware interrupts
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
DEFINE_DEVICE_TYPE(CXD8530AQ, cxd8530aq_device, "cxd8530aq", "Sony CXD8530AQ")
DEFINE_DEVICE_TYPE(CXD8530BQ, cxd8530bq_device, "cxd8530bq", "Sony CXD8530BQ")
DEFINE_DEVICE_TYPE(CXD8530CQ, cxd8530cq_device, "cxd8530cq", "Sony CXD8530CQ")
DEFINE_DEVICE_TYPE(CXD8661R,  cxd8661r_device,  "cxd8661r",  "Sony CXD8661R")
DEFINE_DEVICE_TYPE(CXD8606BQ, cxd8606bq_device, "cxd8606bq", "Sony CXD8606BQ")
DEFINE_DEVICE_TYPE(CXD8606CQ, cxd8606cq_device, "cxd8606cq", "Sony CXD8606CQ")

static const uint32_t mtc0_writemask[]=
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

uint32_t psxcpu_device::berr_r()
{
	if( !machine().side_effects_disabled() )
		m_berr = 1;
	return 0;
}

void psxcpu_device::berr_w(uint32_t data)
{
	if( !machine().side_effects_disabled() )
		m_berr = 1;
}

uint32_t psxcpu_device::exp_base_r()
{
	return m_exp_base;
}

void psxcpu_device::exp_base_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	COMBINE_DATA( &m_exp_base ); // TODO: check byte writes

	m_exp_base = 0x1f000000 | ( m_exp_base & 0xffffff );
}

uint32_t psxcpu_device::exp_base()
{
	return m_exp_base;
}

uint32_t psxcpu_device::exp_config_r()
{
	return m_exp_config;
}

void psxcpu_device::exp_config_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	COMBINE_DATA( &m_exp_config ); // TODO: check byte writes

	m_exp_config &= 0xaf1fffff;
}

uint32_t psxcpu_device::ram_config_r()
{
	return m_ram_config;
}

void psxcpu_device::ram_config_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	uint32_t old = m_ram_config;

	COMBINE_DATA( &m_ram_config ); // TODO: check byte writes

	if( ( ( m_ram_config ^ old ) & 0xff00 ) != 0 )
	{
		update_ram_config();
	}
}

uint32_t psxcpu_device::rom_config_r()
{
	return m_rom_config;
}

void psxcpu_device::rom_config_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	uint32_t old = m_rom_config;

	COMBINE_DATA( &m_rom_config ); // TODO: check byte writes

	if( ( ( m_rom_config ^ old ) & 0x001f0000 ) != 0 )
	{
		update_rom_config();
	}
}

uint32_t psxcpu_device::com_delay_r(offs_t offset, uint32_t mem_mask)
{
	//verboselog( p_psx, 1, "psx_com_delay_r( %08x )\n", mem_mask );
	return m_com_delay;
}

void psxcpu_device::com_delay_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	COMBINE_DATA( &m_com_delay ); // TODO: check byte writes
	//verboselog( p_psx, 1, "psx_com_delay_w( %08x %08x )\n", data, mem_mask );
}

uint32_t psxcpu_device::biu_r()
{
	return m_biu;
}

void psxcpu_device::biu_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	uint32_t old = m_biu;

	COMBINE_DATA( &m_biu ); // TODO: check byte writes

	if( ( old & ( BIU_RAM | BIU_DS ) ) != ( m_biu & ( BIU_RAM | BIU_DS ) ) )
	{
		update_scratchpad();
	}
}

void psxcpu_device::stop()
{
	machine().debug_break();
	debugger_instruction_hook( m_pc );
}

uint32_t psxcpu_device::cache_readword( uint32_t offset )
{
	uint32_t data = 0;

	if( ( m_biu & BIU_TAG ) != 0 )
	{
		if( ( m_biu & BIU_IS1 ) != 0 )
		{
			uint32_t tag = m_icacheTag[ ( offset / 16 ) % ( ICACHE_ENTRIES / 4 ) ];
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

void psxcpu_device::cache_writeword( uint32_t offset, uint32_t data )
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

uint8_t psxcpu_device::readbyte( uint32_t address )
{
	if( m_bus_attached )
	{
		return m_data.read_byte( address );
	}

	return cache_readword( address ) >> ( ( address & 3 ) * 8 );
}

uint16_t psxcpu_device::readhalf( uint32_t address )
{
	if( m_bus_attached )
	{
		return m_data.read_word( address );
	}

	return cache_readword( address ) >> ( ( address & 2 ) * 8 );
}

uint32_t psxcpu_device::readword( uint32_t address )
{
	if( m_bus_attached )
	{
		return m_data.read_dword( address );
	}

	return cache_readword( address );
}

uint32_t psxcpu_device::readword_masked( uint32_t address, uint32_t mask )
{
	if( m_bus_attached )
	{
		return m_data.read_dword( address, mask );
	}

	return cache_readword( address );
}

void psxcpu_device::writeword( uint32_t address, uint32_t data )
{
	if( m_bus_attached )
	{
		m_data.write_dword( address, data );
	}
	else
	{
		cache_writeword( address, data );
	}
}

void psxcpu_device::writeword_masked( uint32_t address, uint32_t data, uint32_t mask )
{
	if( m_bus_attached )
	{
		m_data.write_dword( address, data, mask );
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

uint32_t psxcpu_device::log_bioscall_parameter( int parm )
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
	uint32_t address;
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
		uint8_t c = readbyte( address );
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
					uint8_t c = readbyte( buffer );
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
							uint32_t format = log_bioscall_parameter( parm - 1 );
							const char *parmstr = nullptr;
							int percent = 0;

							for( ;; )
							{
								uint8_t c = readbyte( format );
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
			int64_t result = mul_32x32( (int32_t)m_multiplier_operand1, (int32_t)m_multiplier_operand2 );
			m_lo = result;
			m_hi = result >> 32;
		}
		break;

	case MULTIPLIER_OPERATION_MULTU:
		{
			uint64_t result = mulu_32x32( m_multiplier_operand1, m_multiplier_operand2 );
			m_lo = result;
			m_hi = result >> 32;
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
			if( (int32_t)m_multiplier_operand1 < 0 )
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
			m_lo = (int32_t)m_multiplier_operand1 / (int32_t)m_multiplier_operand2;
			m_hi = (int32_t)m_multiplier_operand1 % (int32_t)m_multiplier_operand2;
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

uint32_t psxcpu_device::get_hi()
{
	if( m_multiplier_operation != MULTIPLIER_OPERATION_IDLE )
	{
		multiplier_update();
	}

	return m_hi;
}

uint32_t psxcpu_device::get_lo()
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
		m_program->install_readwrite_handler( 0x1f800000, 0x1f8003ff, read32smo_delegate(*this, FUNC(psxcpu_device::berr_r)), write32smo_delegate(*this, FUNC(psxcpu_device::berr_w)) );
	}
	else if( ( m_biu & BIU_DS ) == 0 )
	{
		m_program->install_read_handler( 0x1f800000, 0x1f8003ff, read32smo_delegate(*this, FUNC(psxcpu_device::berr_r)) );
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

	uint32_t ram_size = m_ram->size();
	uint8_t *pointer = m_ram->pointer();

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

	m_program->install_readwrite_handler( 0x00000000 + window_size, 0x1effffff, read32smo_delegate(*this, FUNC(psxcpu_device::berr_r)), write32smo_delegate(*this, FUNC(psxcpu_device::berr_w)) );
	m_program->install_readwrite_handler( 0x80000000 + window_size, 0x9effffff, read32smo_delegate(*this, FUNC(psxcpu_device::berr_r)), write32smo_delegate(*this, FUNC(psxcpu_device::berr_w)) );
	m_program->install_readwrite_handler( 0xa0000000 + window_size, 0xbeffffff, read32smo_delegate(*this, FUNC(psxcpu_device::berr_r)), write32smo_delegate(*this, FUNC(psxcpu_device::berr_w)) );
}

void psxcpu_device::update_rom_config()
{
	int window_size = 1 << ( ( m_rom_config >> 16 ) & 0x1f );
	int max_window_size = 0x400000;
	if( window_size > max_window_size )
	{
		window_size = max_window_size;
	}

	uint32_t rom_size = m_rom->bytes();
	uint8_t *pointer = m_rom->base();

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
		m_program->install_readwrite_handler( 0x1fc00000 + window_size, 0x1fffffff, read32smo_delegate(*this, FUNC(psxcpu_device::berr_r)), write32smo_delegate(*this, FUNC(psxcpu_device::berr_w)) );
		m_program->install_readwrite_handler( 0x9fc00000 + window_size, 0x9fffffff, read32smo_delegate(*this, FUNC(psxcpu_device::berr_r)), write32smo_delegate(*this, FUNC(psxcpu_device::berr_w)) );
		m_program->install_readwrite_handler( 0xbfc00000 + window_size, 0xbfffffff, read32smo_delegate(*this, FUNC(psxcpu_device::berr_r)), write32smo_delegate(*this, FUNC(psxcpu_device::berr_w)) );
	}
}

void psxcpu_device::update_cop0(int reg)
{
	if (reg == CP0_SR)
	{
		update_memory_handlers();
		update_address_masks();
	}

	if ((reg == CP0_SR || reg == CP0_CAUSE) &&
		(m_cp0r[CP0_SR] & SR_IEC) != 0)
	{
		uint32_t ip = m_cp0r[CP0_SR] & m_cp0r[CP0_CAUSE] & CAUSE_IP;
		if (ip != 0)
		{
			if (ip & CAUSE_IP0) debugger_exception_hook(EXC_INT);
			if (ip & CAUSE_IP1) debugger_exception_hook(EXC_INT);
			//if (ip & CAUSE_IP2) debugger_interrupt_hook(PSXCPU_IRQ0);
			//if (ip & CAUSE_IP3) debugger_interrupt_hook(PSXCPU_IRQ1);
			//if (ip & CAUSE_IP4) debugger_interrupt_hook(PSXCPU_IRQ2);
			//if (ip & CAUSE_IP5) debugger_interrupt_hook(PSXCPU_IRQ3);
			//if (ip & CAUSE_IP6) debugger_interrupt_hook(PSXCPU_IRQ4);
			//if (ip & CAUSE_IP7) debugger_interrupt_hook(PSXCPU_IRQ5);
			m_op = m_instruction.read_dword(m_pc);
			execute_unstoppable_instructions(1);
			exception(EXC_INT);
		}
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
		uint32_t safepc = m_delayv & ~m_bad_word_address_mask;

		m_op = m_instruction.read_dword( safepc );
	}
	else
	{
		m_op = m_instruction.read_dword( m_pc + 4 );
	}
}

void psxcpu_device::advance_pc()
{
	if( m_delayr == PSXCPU_DELAYR_PC )
	{
		m_pc = m_delayv;
		m_delayr = 0;
		m_delayv = 0;
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
}

void psxcpu_device::load( uint32_t reg, uint32_t value )
{
	advance_pc();

	if( reg != 0 )
	{
		m_r[ reg ] = value;
	}
}

void psxcpu_device::delayed_load( uint32_t reg, uint32_t value )
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

void psxcpu_device::branch( uint32_t address )
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

void psxcpu_device::common_exception( int exception, uint32_t romOffset, uint32_t ramOffset )
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

	if (exception != EXC_INT)
	{
		if (LOG_BIOSCALL)
			logerror("%08x: Exception %d\n", m_pc, exception);

		debugger_exception_hook(exception);
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
		advance_pc();

		if( ( m_pc & m_bad_word_address_mask ) != 0 )
		{
			load_bad_address( m_pc );
			return;
		}

		fetch_next_op();
		execute_unstoppable_instructions( 0 );
	}

	common_exception( EXC_DBE, 0xbfc00180, 0x80000080 );
}

void psxcpu_device::load_bad_address( uint32_t address )
{
	m_cp0r[ CP0_BADA ] = address;
	exception( EXC_ADEL );
}

void psxcpu_device::store_bad_address( uint32_t address )
{
	m_cp0r[ CP0_BADA ] = address;
	exception( EXC_ADES );
}

int psxcpu_device::data_address_breakpoint( int dcic_rw, int dcic_status, uint32_t address )
{
	if( address < 0x1f000000 || address > 0x1fffffff )
	{
		if( ( m_cp0r[ CP0_DCIC ] & DCIC_DE ) != 0 &&
			( m_cp0r[ CP0_DCIC ] & dcic_rw ) == dcic_rw &&
			( ( ( m_cp0r[ CP0_DCIC ] & DCIC_KD ) != 0 && ( m_pc & 0x80000000 ) != 0 ) ||
			( ( m_cp0r[ CP0_DCIC ] & DCIC_UD ) != 0 && ( m_pc & 0x80000000 ) == 0 ) ) )
		{
			if( ( address & m_cp0r[ CP0_BDAM ] ) == ( m_cp0r[ CP0_BDA ] & m_cp0r[ CP0_BDAM ] ) )
			{
				m_cp0r[ CP0_DCIC ] |= dcic_status;

				if( ( m_cp0r[ CP0_DCIC ] & DCIC_TR ) != 0 )
				{
					return 1;
				}
			}
		}
	}

	return 0;
}

int psxcpu_device::program_counter_breakpoint()
{
	if( ( m_cp0r[ CP0_DCIC ] & DCIC_DE ) != 0 &&
		( m_cp0r[ CP0_DCIC ] & DCIC_PCE ) != 0 &&
		( ( ( m_cp0r[ CP0_DCIC ] & DCIC_KD ) != 0 && ( m_pc & 0x80000000 ) != 0 ) ||
		( ( m_cp0r[ CP0_DCIC ] & DCIC_UD ) != 0 && ( m_pc & 0x80000000 ) == 0 ) ) )
	{
		if( ( m_pc & m_cp0r[ CP0_BPCM ] ) == ( m_cp0r[ CP0_BPC ] & m_cp0r[ CP0_BPCM ] ) )
		{
			m_cp0r[ CP0_DCIC ] |= DCIC_PC | DCIC_DB;

			if( ( m_cp0r[ CP0_DCIC ] & DCIC_TR ) != 0 )
			{
				return 1;
			}
		}
	}

	return 0;
}

int psxcpu_device::load_data_address_breakpoint( uint32_t address )
{
	return data_address_breakpoint( DCIC_DR | DCIC_DAE, DCIC_DB | DCIC_DA | DCIC_R, address );
}

int psxcpu_device::store_data_address_breakpoint( uint32_t address )
{
	return data_address_breakpoint( DCIC_DW | DCIC_DAE, DCIC_DB | DCIC_DA | DCIC_W, address );
}

// On-board RAM and peripherals
void psxcpu_device::psxcpu_internal_map(address_map &map)
{
	map(0x1f800000, 0x1f8003ff).noprw(); /* scratchpad */
	map(0x1f800400, 0x1f800fff).rw(FUNC(psxcpu_device::berr_r), FUNC(psxcpu_device::berr_w));
	map(0x1f801000, 0x1f801003).rw(FUNC(psxcpu_device::exp_base_r), FUNC(psxcpu_device::exp_base_w));
	map(0x1f801004, 0x1f801007).ram();
	map(0x1f801008, 0x1f80100b).rw(FUNC(psxcpu_device::exp_config_r), FUNC(psxcpu_device::exp_config_w));
	map(0x1f80100c, 0x1f80100f).ram();
	map(0x1f801010, 0x1f801013).rw(FUNC(psxcpu_device::rom_config_r), FUNC(psxcpu_device::rom_config_w));
	map(0x1f801014, 0x1f80101f).ram();
	/* 1f801014 spu delay */
	/* 1f801018 dv delay */
	map(0x1f801020, 0x1f801023).rw(FUNC(psxcpu_device::com_delay_r), FUNC(psxcpu_device::com_delay_w));
	map(0x1f801024, 0x1f80102f).ram();
	map(0x1f801040, 0x1f80104f).rw("sio0", FUNC(psxsio_device::read), FUNC(psxsio_device::write));
	map(0x1f801050, 0x1f80105f).rw("sio1", FUNC(psxsio_device::read), FUNC(psxsio_device::write));
	map(0x1f801060, 0x1f801063).rw(FUNC(psxcpu_device::ram_config_r), FUNC(psxcpu_device::ram_config_w));
	map(0x1f801064, 0x1f80106f).ram();
	map(0x1f801070, 0x1f801077).rw("irq", FUNC(psxirq_device::read), FUNC(psxirq_device::write));
	map(0x1f801080, 0x1f8010ff).rw("dma", FUNC(psxdma_device::read), FUNC(psxdma_device::write));
	map(0x1f801100, 0x1f80112f).rw("rcnt", FUNC(psxrcnt_device::read), FUNC(psxrcnt_device::write));
	map(0x1f801800, 0x1f801803).rw(FUNC(psxcpu_device::cd_r), FUNC(psxcpu_device::cd_w));
	map(0x1f801810, 0x1f801817).rw(FUNC(psxcpu_device::gpu_r), FUNC(psxcpu_device::gpu_w));
	map(0x1f801820, 0x1f801827).rw("mdec", FUNC(psxmdec_device::read), FUNC(psxmdec_device::write));
	map(0x1f801c00, 0x1f801dff).rw(FUNC(psxcpu_device::spu_r), FUNC(psxcpu_device::spu_w));
	map(0x1f802020, 0x1f802033).ram(); /* ?? */
	/* 1f802030 int 2000 */
	/* 1f802040 dip switches */
	map(0x1f802040, 0x1f802043).nopw();
	map(0x20000000, 0x7fffffff).rw(FUNC(psxcpu_device::berr_r), FUNC(psxcpu_device::berr_w));
	map(0xc0000000, 0xfffdffff).rw(FUNC(psxcpu_device::berr_r), FUNC(psxcpu_device::berr_w));
	map(0xfffe0130, 0xfffe0133).rw(FUNC(psxcpu_device::biu_r), FUNC(psxcpu_device::biu_w));
}


//**************************************************************************
//  DEVICE INTERFACE
//**************************************************************************

//-------------------------------------------------
//  psxcpu_device - constructor
//-------------------------------------------------

psxcpu_device::psxcpu_device( const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock ) :
	cpu_device( mconfig, type, tag, owner, clock ),
	m_program_config( "program", ENDIANNESS_LITTLE, 32, 32, 0, address_map_constructor(FUNC(psxcpu_device::psxcpu_internal_map), this)),
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

cxd8530aq_device::cxd8530aq_device( const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock )
	: psxcpu_device( mconfig, CXD8530AQ, tag, owner, clock)
{
}

cxd8530bq_device::cxd8530bq_device( const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock )
	: psxcpu_device( mconfig, CXD8530BQ, tag, owner, clock)
{
}

cxd8530cq_device::cxd8530cq_device( const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock )
	: psxcpu_device( mconfig, CXD8530CQ, tag, owner, clock)
{
}

cxd8661r_device::cxd8661r_device( const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock )
	: psxcpu_device( mconfig, CXD8661R, tag, owner, clock)
{
}

cxd8606bq_device::cxd8606bq_device( const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock )
	: psxcpu_device( mconfig, CXD8606BQ, tag, owner, clock)
{
}

cxd8606cq_device::cxd8606cq_device( const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock )
	: psxcpu_device( mconfig, CXD8606CQ, tag, owner, clock)
{
}

//-------------------------------------------------
//  device_start - start up the device
//-------------------------------------------------

void psxcpu_device::device_start()
{
	// get our address spaces
	m_program = &space( AS_PROGRAM );
	m_program->cache(m_instruction);
	m_program->specific(m_data);

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
	save_item( NAME( m_exp_base ) );
	save_item( NAME( m_exp_config ) );
	save_item( NAME( m_ram_config ) );
	save_item( NAME( m_rom_config ) );

	state_add( STATE_GENPC, "GENPC", m_pc ).noshow();
	state_add( STATE_GENPCBASE, "CURPC", m_pc ).noshow();
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

	// initialize the registers once
	for(int i=0; i != 32; i++)
		m_r[i] = 0;

	// set our instruction counter
	set_icountptr(m_icount);

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
	psxdma->m_ram = (uint32_t *)m_ram->pointer();
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
	update_ram_config();
	update_rom_config();
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
//  disassemble - call the disassembly
//  helper function
//-------------------------------------------------

std::unique_ptr<util::disasm_interface> psxcpu_device::create_disassembler()
{
	return std::make_unique<psxcpu_disassembler>(static_cast<psxcpu_disassembler::config *>(this));
}


uint32_t psxcpu_device::get_register_from_pipeline( int reg )
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
	uint32_t address = m_r[ INS_RS( m_op ) ] + PSXCPU_WORD_EXTEND( INS_IMMEDIATE( m_op ) );
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
		uint32_t data = readword( address );

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
	uint32_t address = m_r[ INS_RS( m_op ) ] + PSXCPU_WORD_EXTEND( INS_IMMEDIATE( m_op ) );
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
		uint32_t data = 0;

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
	uint32_t ip;

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
		debugger_instruction_hook( m_pc );

		int breakpoint = program_counter_breakpoint();

		if( ( m_pc & m_bad_word_address_mask ) != 0 )
		{
			load_bad_address( m_pc );
		}
		else if( breakpoint )
		{
			breakpoint_exception();
		}
		else
		{
			m_op = m_instruction.read_dword(m_pc);

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
						load( INS_RD( m_op ), (int32_t)m_r[ INS_RT( m_op ) ] >> INS_SHAMT( m_op ) );
						break;

					case FUNCT_SLLV:
						load( INS_RD( m_op ), m_r[ INS_RT( m_op ) ] << ( m_r[ INS_RS( m_op ) ] & 31 ) );
						break;

					case FUNCT_SRLV:
						load( INS_RD( m_op ), m_r[ INS_RT( m_op ) ] >> ( m_r[ INS_RS( m_op ) ] & 31 ) );
						break;

					case FUNCT_SRAV:
						load( INS_RD( m_op ), (int32_t)m_r[ INS_RT( m_op ) ] >> ( m_r[ INS_RS( m_op ) ] & 31 ) );
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
							uint32_t result = m_r[ INS_RS( m_op ) ] + m_r[ INS_RT( m_op ) ];
							if( (int32_t)( ~( m_r[ INS_RS( m_op ) ] ^ m_r[ INS_RT( m_op ) ] ) & ( m_r[ INS_RS( m_op ) ] ^ result ) ) < 0 )
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
							uint32_t result = m_r[ INS_RS( m_op ) ] - m_r[ INS_RT( m_op ) ];
							if( (int32_t)( ( m_r[ INS_RS( m_op ) ] ^ m_r[ INS_RT( m_op ) ] ) & ( m_r[ INS_RS( m_op ) ] ^ result ) ) < 0 )
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
						load( INS_RD( m_op ), (int32_t)m_r[ INS_RS( m_op ) ] < (int32_t)m_r[ INS_RT( m_op ) ] );
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
						conditional_branch( (int32_t)m_r[ INS_RS( m_op ) ] < 0 );

						if( INS_RT( m_op ) == RT_BLTZAL )
						{
							m_r[ 31 ] = m_pc + 4;
						}
						break;

					case RT_BGEZ:
						conditional_branch( (int32_t)m_r[ INS_RS( m_op ) ] >= 0 );

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
					conditional_branch( (int32_t)m_r[ INS_RS( m_op ) ] < 0 || m_r[ INS_RS( m_op ) ] == m_r[ INS_RT( m_op ) ] );
					break;

				case OP_BGTZ:
					conditional_branch( (int32_t)m_r[ INS_RS( m_op ) ] >= 0 && m_r[ INS_RS( m_op ) ] != m_r[ INS_RT( m_op ) ] );
					break;

				case OP_ADDI:
					{
						uint32_t immediate = PSXCPU_WORD_EXTEND( INS_IMMEDIATE( m_op ) );
						uint32_t result = m_r[ INS_RS( m_op ) ] + immediate;
						if( (int32_t)( ~( m_r[ INS_RS( m_op ) ] ^ immediate ) & ( m_r[ INS_RS( m_op ) ] ^ result ) ) < 0 )
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
					load( INS_RT( m_op ), (int32_t)m_r[ INS_RS( m_op ) ] < PSXCPU_WORD_EXTEND( INS_IMMEDIATE( m_op ) ) );
					break;

				case OP_SLTIU:
					load( INS_RT( m_op ), m_r[ INS_RS( m_op ) ] < (uint32_t)PSXCPU_WORD_EXTEND( INS_IMMEDIATE( m_op ) ) );
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
									uint32_t data = ( m_cp0r[ reg ] & ~mtc0_writemask[ reg ] ) |
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
						uint32_t address = m_r[ INS_RS( m_op ) ] + PSXCPU_WORD_EXTEND( INS_IMMEDIATE( m_op ) );
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
							uint32_t data = PSXCPU_BYTE_EXTEND( readbyte( address ) );

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
						uint32_t address = m_r[ INS_RS( m_op ) ] + PSXCPU_WORD_EXTEND( INS_IMMEDIATE( m_op ) );
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
							uint32_t data = PSXCPU_WORD_EXTEND( readhalf( address ) );

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
						uint32_t address = m_r[ INS_RS( m_op ) ] + PSXCPU_WORD_EXTEND( INS_IMMEDIATE( m_op ) );
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
							uint32_t data = get_register_from_pipeline( INS_RT( m_op ) );

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
						uint32_t address = m_r[ INS_RS( m_op ) ] + PSXCPU_WORD_EXTEND( INS_IMMEDIATE( m_op ) );
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
							uint32_t data = readword( address );

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
						uint32_t address = m_r[ INS_RS( m_op ) ] + PSXCPU_WORD_EXTEND( INS_IMMEDIATE( m_op ) );
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
							uint32_t data = readbyte( address );

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
						uint32_t address = m_r[ INS_RS( m_op ) ] + PSXCPU_WORD_EXTEND( INS_IMMEDIATE( m_op ) );
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
							uint32_t data = readhalf( address );

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
						uint32_t address = m_r[ INS_RS( m_op ) ] + PSXCPU_WORD_EXTEND( INS_IMMEDIATE( m_op ) );
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
							uint32_t data = get_register_from_pipeline( INS_RT( m_op ) );

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
						uint32_t address = m_r[ INS_RS( m_op ) ] + PSXCPU_WORD_EXTEND( INS_IMMEDIATE( m_op ) );
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
						uint32_t address = m_r[ INS_RS( m_op ) ] + PSXCPU_WORD_EXTEND( INS_IMMEDIATE( m_op ) );
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
						uint32_t address = m_r[ INS_RS( m_op ) ] + PSXCPU_WORD_EXTEND( INS_IMMEDIATE( m_op ) );
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
						uint32_t address = m_r[ INS_RS( m_op ) ] + PSXCPU_WORD_EXTEND( INS_IMMEDIATE( m_op ) );
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
						uint32_t address = m_r[ INS_RS( m_op ) ] + PSXCPU_WORD_EXTEND( INS_IMMEDIATE( m_op ) );
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
		}

		m_icount--;
	} while( m_icount > 0 );
}

uint32_t psxcpu_device::getcp1dr( int reg )
{
	/* if a mtc/ctc precedes then this will get the value moved (which cop1 register is irrelevant). */
	/* if a mfc/cfc follows then it will get the same value as this one. */
	return m_program->read_dword( m_pc + 4 );
}

void psxcpu_device::setcp1dr( int reg, uint32_t value )
{
}

uint32_t psxcpu_device::getcp1cr( int reg )
{
	/* if a mtc/ctc precedes then this will get the value moved (which cop1 register is irrelevant). */
	/* if a mfc/cfc follows then it will get the same value as this one. */
	return m_program->read_dword( m_pc + 4 );
}

void psxcpu_device::setcp1cr( int reg, uint32_t value )
{
}


uint32_t psxcpu_device::getcp3dr( int reg )
{
	/* if you have mtc/ctc with an mfc/cfc directly afterwards then you get the value that was moved. */
	/* if you have an lwc with an mfc/cfc somewhere after it then you get the value that is loaded */
	/* otherwise you get the next opcode. which register you transfer to or from is irrelevant. */
	return m_program->read_dword( m_pc + 4 );
}

void psxcpu_device::setcp3dr( int reg, uint32_t value )
{
}

uint32_t psxcpu_device::getcp3cr( int reg )
{
	/* if you have mtc/ctc with an mfc/cfc directly afterwards then you get the value that was moved. */
	/* if you have an lwc with an mfc/cfc somewhere after it then you get the value that is loaded */
	/* otherwise you get the next opcode. which register you transfer to or from is irrelevant. */
	return m_program->read_dword( m_pc + 4 );
}

void psxcpu_device::setcp3cr( int reg, uint32_t value )
{
}

uint32_t psxcpu_device::gpu_r(offs_t offset, uint32_t mem_mask)
{
	return m_gpu_read_handler( offset, mem_mask );
}

void psxcpu_device::gpu_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	m_gpu_write_handler( offset, data, mem_mask );
}

uint16_t psxcpu_device::spu_r(offs_t offset, uint16_t mem_mask)
{
	return m_spu_read_handler( offset, mem_mask );
}

void psxcpu_device::spu_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	m_spu_write_handler( offset, data, mem_mask );
}

uint8_t psxcpu_device::cd_r(offs_t offset, uint8_t mem_mask)
{
	return m_cd_read_handler( offset, mem_mask );
}

void psxcpu_device::cd_w(offs_t offset, uint8_t data, uint8_t mem_mask)
{
	m_cd_write_handler( offset, data, mem_mask );
}

void psxcpu_device::set_disable_rom_berr(bool mode)
{
	m_disable_rom_berr = mode;
}

device_memory_interface::space_config_vector psxcpu_device::memory_space_config() const
{
	return space_config_vector {
		std::make_pair(AS_PROGRAM, &m_program_config),
	};
}

//-------------------------------------------------
//  device_add_mconfig - add device configuration
//-------------------------------------------------

void psxcpu_device::device_add_mconfig(machine_config &config)
{
	auto &irq(PSX_IRQ(config, "irq", 0));
	irq.irq().set_inputline(DEVICE_SELF, PSXCPU_IRQ0);

	auto &dma(PSX_DMA(config, "dma", 0));
	dma.irq().set("irq", FUNC(psxirq_device::intin3));

	auto &mdec(PSX_MDEC(config, "mdec", 0));
	dma.install_write_handler(0, psxdma_device::write_delegate(&psxmdec_device::dma_write, &mdec));
	dma.install_read_handler(1, psxdma_device::write_delegate(&psxmdec_device::dma_read, &mdec));

	auto &rcnt(PSX_RCNT(config, "rcnt", 0));
	rcnt.irq0().set("irq", FUNC(psxirq_device::intin4));
	rcnt.irq1().set("irq", FUNC(psxirq_device::intin5));
	rcnt.irq2().set("irq", FUNC(psxirq_device::intin6));

	auto &sio0(PSX_SIO0(config, "sio0", DERIVED_CLOCK(1, 2)));
	sio0.irq_handler().set("irq", FUNC(psxirq_device::intin7));

	auto &sio1(PSX_SIO1(config, "sio1", DERIVED_CLOCK(1, 2)));
	sio1.irq_handler().set("irq", FUNC(psxirq_device::intin8));

	RAM(config, "ram").set_default_value(0x00);
}
