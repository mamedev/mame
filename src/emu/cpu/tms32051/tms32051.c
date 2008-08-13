/*
   Texas Instruments TMS320C51 DSP Emulator

   Written by Ville Linde
*/

#include "debugger.h"
#include "cpuintrf.h"
#include "tms32051.h"

static void delay_slot(UINT16 startpc);
static void save_interrupt_context(void);
static void restore_interrupt_context(void);
static void check_interrupts(void);

#define INTERRUPT_INT1		0
#define INTERRUPT_INT2		1
#define INTERRUPT_INT3		2
#define INTERRUPT_TINT		3
#define INTERRUPT_RINT		4
#define INTERRUPT_XINT		5
#define INTERRUPT_TRNT		6
#define INTERRUPT_TXNT		7
#define INTERRUPT_INT4		8

enum
{
	TMS32051_PC = 1,
	TMS32051_ACC,
	TMS32051_ACCB,
	TMS32051_PREG,
	TMS32051_TREG0,
	TMS32051_TREG1,
	TMS32051_TREG2,
	TMS32051_BMAR,
	TMS32051_RPTC,
	TMS32051_BRCR,
	TMS32051_INDX,
	TMS32051_DBMR,
	TMS32051_ARCR,
	TMS32051_DP,
	TMS32051_ARP,
	TMS32051_ARB,
	TMS32051_AR0,
	TMS32051_AR1,
	TMS32051_AR2,
	TMS32051_AR3,
	TMS32051_AR4,
	TMS32051_AR5,
	TMS32051_AR6,
	TMS32051_AR7,
};

typedef struct
{
	UINT16 iptr;
	UINT16 avis;
	UINT16 ovly;
	UINT16 ram;
	UINT16 mpmc;
	UINT16 ndx;
	UINT16 trm;
	UINT16 braf;
} PMST;

typedef struct
{
	UINT16 dp;
	UINT16 intm;
	UINT16 ovm;
	UINT16 ov;
	UINT16 arp;
} ST0;

typedef struct
{
	UINT16 arb;
	UINT16 cnf;
	UINT16 tc;
	UINT16 sxm;
	UINT16 c;
	UINT16 hm;
	UINT16 xf;
	UINT16 pm;
} ST1;

typedef struct {
	UINT16 pc;
	UINT16 op;
	INT32 acc;
	INT32 accb;
	INT32 preg;
	UINT16 treg0;
	UINT16 treg1;
	UINT16 treg2;
	UINT16 ar[8];
	INT32 rptc;

	UINT16 bmar;
	INT32 brcr;
	UINT16 paer;
	UINT16 pasr;
	UINT16 indx;
	UINT16 dbmr;
	UINT16 arcr;

	ST0 st0;
	ST1 st1;
	PMST pmst;

	UINT16 ifr;
	UINT16 imr;

	UINT16 pcstack[8];
	int pcstack_ptr;

	UINT16 rpt_start, rpt_end;

	UINT16 cbcr;
	UINT16 cbsr1;
	UINT16 cber1;
	UINT16 cbsr2;
	UINT16 cber2;

	struct
	{
		int tddr;
		int psc;
		UINT16 tim;
		UINT16 prd;
	} timer;

	struct
	{
		INT32 acc;
		INT32 accb;
		UINT16 arcr;
		UINT16 indx;
		PMST pmst;
		INT32 preg;
		ST0 st0;
		ST1 st1;
		INT32 treg0;
		INT32 treg1;
		INT32 treg2;
	} shadow;

	int (*irq_callback)(int irqline);
} TMS_REGS;

static TMS_REGS tms;
static int tms_icount;

#define CYCLES(x)		(tms_icount -= x)

#define ROPCODE()		cpu_readop16((tms.pc++) << 1)

INLINE void CHANGE_PC(UINT16 new_pc)
{
	tms.pc = new_pc;
	change_pc(tms.pc << 1);
}

INLINE UINT16 PM_READ16(UINT16 address)
{
	return program_read_word_16le(address << 1);
}

INLINE void PM_WRITE16(UINT16 address, UINT16 data)
{
	program_write_word_16le(address << 1, data);
}

INLINE UINT16 DM_READ16(UINT16 address)
{
	return data_read_word_16le(address << 1);
}

INLINE void DM_WRITE16(UINT16 address, UINT16 data)
{
	data_write_word_16le(address << 1, data);
}

#include "32051ops.c"
#include "32051ops.h"

static void op_group_be(void)
{
	tms32051_opcode_table_be[tms.op & 0xff]();
}

static void op_group_bf(void)
{
	tms32051_opcode_table_bf[tms.op & 0xff]();
}

static void delay_slot(UINT16 startpc)
{
	tms.op = ROPCODE();
	tms32051_opcode_table[tms.op >> 8]();

	while (tms.pc - startpc < 2)
	{
		tms.op = ROPCODE();
		tms32051_opcode_table[tms.op >> 8]();
	}
}

/*****************************************************************************/

static void tms_init(int index, int clock, const void *_config, int (*irqcallback)(int))
{

}

static void tms_reset(void)
{
	int i;
	UINT16 src, dst, length;

	src = 0x7800;
	dst = DM_READ16(src++);
	length = DM_READ16(src++);

	CHANGE_PC(dst);

	for (i=0; i < length; i++)
	{
		UINT16 data = DM_READ16(src++);
		PM_WRITE16(dst++, data);
	}

	tms.st0.intm	= 1;
	tms.st0.ov		= 0;
	tms.st1.c		= 1;
	tms.st1.cnf		= 0;
	tms.st1.hm		= 1;
	tms.st1.pm		= 0;
	tms.st1.sxm		= 1;
	tms.st1.xf		= 1;
	tms.pmst.avis	= 0;
	tms.pmst.braf	= 0;
	tms.pmst.iptr	= 0;
	tms.pmst.ndx	= 0;
	tms.pmst.ovly	= 0;
	tms.pmst.ram	= 0;
	tms.pmst.trm	= 0;
	tms.ifr			= 0;
	tms.cbcr		= 0;
	tms.rptc		= -1;
}

static void check_interrupts(void)
{
	int i;

	if (tms.st0.intm == 0 && tms.ifr != 0)
	{
		for (i=0; i < 16; i++)
		{
			if (tms.ifr & (1 << i))
			{
				tms.st0.intm = 1;
				PUSH_STACK(tms.pc);

				tms.pc = (tms.pmst.iptr << 11) | ((i+1) << 1);
				tms.ifr &= ~(1 << i);

				save_interrupt_context();
				break;
			}
		}
	}
}

static void save_interrupt_context(void)
{
	tms.shadow.acc		= tms.acc;
	tms.shadow.accb		= tms.accb;
	tms.shadow.arcr		= tms.arcr;
	tms.shadow.indx		= tms.indx;
	tms.shadow.preg		= tms.preg;
	tms.shadow.treg0	= tms.treg0;
	tms.shadow.treg1	= tms.treg1;
	tms.shadow.treg2	= tms.treg2;
	memcpy(&tms.shadow.pmst, &tms.pmst, sizeof(PMST));
	memcpy(&tms.shadow.st0, &tms.st0, sizeof(ST0));
	memcpy(&tms.shadow.st1, &tms.st1, sizeof(ST1));
}

static void restore_interrupt_context(void)
{
	tms.acc				= tms.shadow.acc;
	tms.accb			= tms.shadow.accb;
	tms.arcr			= tms.shadow.arcr;
	tms.indx			= tms.shadow.indx;
	tms.preg			= tms.shadow.preg;
	tms.treg0			= tms.shadow.treg0;
	tms.treg1			= tms.shadow.treg1;
	tms.treg2			= tms.shadow.treg2;
	memcpy(&tms.pmst, &tms.shadow.pmst, sizeof(PMST));
	memcpy(&tms.st0, &tms.shadow.st0, sizeof(ST0));
	memcpy(&tms.st1, &tms.shadow.st1, sizeof(ST1));
}

static void tms_interrupt(int irq)
{
	if ((tms.imr & (1 << irq)) != 0)
	{
		tms.ifr |= 1 << irq;
	}

	check_interrupts();
}

static void tms_exit(void)
{
	/* TODO */
}

static void tms_get_context(void *dst)
{
	/* copy the context */
	if (dst)
		*(TMS_REGS *)dst = tms;
}

static void tms_set_context(void *src)
{
	/* copy the context */
	if (src)
		tms = *(TMS_REGS *)src;

	CHANGE_PC(tms.pc);
}

static int tms_execute(int num_cycles)
{
	tms_icount = num_cycles;

	while(tms_icount > 0)
	{
		UINT16 ppc;

		// handle block repeat
		if (tms.pmst.braf)
		{
			if (tms.pc == tms.paer)
			{
				if (tms.brcr > 0)
				{
					CHANGE_PC(tms.pasr);
				}

				tms.brcr--;
				if (tms.brcr <= 0)
				{
					tms.pmst.braf = 0;
				}
			}
		}

		ppc = tms.pc;
		debugger_instruction_hook(Machine, tms.pc);

		tms.op = ROPCODE();
		tms32051_opcode_table[tms.op >> 8]();

		// handle single repeat
		if (tms.rptc > 0)
		{
			if (ppc == tms.rpt_end)
			{
				CHANGE_PC(tms.rpt_start);
				tms.rptc--;
			}
		}
		else
		{
			tms.rptc = 0;
		}

		tms.timer.psc--;
		if (tms.timer.psc <= 0)
		{
			tms.timer.psc = tms.timer.tddr;
			tms.timer.tim--;
			if (tms.timer.tim <= 0)
			{
				// reset timer
				tms.timer.tim = tms.timer.prd;

				tms_interrupt(INTERRUPT_TINT);
			}
		}
	}
	return num_cycles - tms_icount;
}


/*****************************************************************************/

static READ16_HANDLER( cpuregs_r )
{
	switch (offset)
	{
		case 0x04:	return tms.imr;
		case 0x06:	return tms.ifr;

		case 0x07:		// PMST
		{
			UINT16 r = 0;
			r |= tms.pmst.iptr << 11;
			r |= tms.pmst.avis << 7;
			r |= tms.pmst.ovly << 5;
			r |= tms.pmst.ram << 4;
			r |= tms.pmst.mpmc << 3;
			r |= tms.pmst.ndx << 2;
			r |= tms.pmst.trm << 1;
			r |= tms.pmst.braf << 0;
			return r;
		}

		case 0x09:	return tms.brcr;
		case 0x10:	return tms.ar[0];
		case 0x11:	return tms.ar[1];
		case 0x12:	return tms.ar[2];
		case 0x13:	return tms.ar[3];
		case 0x14:	return tms.ar[4];
		case 0x15:	return tms.ar[5];
		case 0x16:	return tms.ar[6];
		case 0x17:	return tms.ar[7];
		case 0x1e:	return tms.cbcr;
		case 0x1f:	return tms.bmar;
		case 0x24:	return tms.timer.tim;
		case 0x25:	return tms.timer.prd;

		case 0x26:		// TCR
		{
			UINT16 r = 0;
			r |= (tms.timer.psc & 0xf) << 6;
			r |= (tms.timer.tddr & 0xf);
			return r;
		}

		case 0x28:	return 0;	// PDWSR
		default:	fatalerror("32051: cpuregs_r: unimplemented memory-mapped register %02X at %04X\n", offset, tms.pc-1);
	}

	return 0;
}

static WRITE16_HANDLER( cpuregs_w )
{
	switch (offset)
	{
		case 0x00:	break;
		case 0x04:	tms.imr = data; break;
		case 0x06:		// IFR
		{
			int i;
			for (i=0; i < 16; i++)
			{
				if (data & (1 << i))
				{
					tms.ifr &= ~(1 << i);
				}
			}
			break;
		}

		case 0x07:		// PMST
		{
			tms.pmst.iptr = (data >> 11) & 0x1f;
			tms.pmst.avis = (data & 0x80) ? 1 : 0;
			tms.pmst.ovly = (data & 0x20) ? 1 : 0;
			tms.pmst.ram = (data & 0x10) ? 1 : 0;
			tms.pmst.mpmc = (data & 0x08) ? 1 : 0;
			tms.pmst.ndx = (data & 0x04) ? 1 : 0;
			tms.pmst.trm = (data & 0x02) ? 1 : 0;
			tms.pmst.braf = (data & 0x01) ? 1 : 0;
			break;
		}

		case 0x09:	tms.brcr = data; break;
		case 0x0e:	tms.treg2 = data; break;
		case 0x0f:	tms.dbmr = data; break;
		case 0x10:	tms.ar[0] = data; break;
		case 0x11:	tms.ar[1] = data; break;
		case 0x12:	tms.ar[2] = data; break;
		case 0x13:	tms.ar[3] = data; break;
		case 0x14:	tms.ar[4] = data; break;
		case 0x15:	tms.ar[5] = data; break;
		case 0x16:	tms.ar[6] = data; break;
		case 0x17:	tms.ar[7] = data; break;
		case 0x18:	tms.indx = data; break;
		case 0x19:	tms.arcr = data; break;
		case 0x1a:	tms.cbsr1 = data; break;
		case 0x1b:	tms.cber1 = data; break;
		case 0x1c:	tms.cbsr2 = data; break;
		case 0x1d:	tms.cber2 = data; break;
		case 0x1e:	tms.cbcr = data; break;
		case 0x1f:	tms.bmar = data; break;
		case 0x24:	tms.timer.tim = data; break;
		case 0x25:	tms.timer.prd = data; break;

		case 0x26:		// TCR
		{
			tms.timer.tddr = data & 0xf;
			tms.timer.psc = (data >> 6) & 0xf;

			if (data & 0x20)
			{
				tms.timer.tim = tms.timer.prd;
				tms.timer.psc = tms.timer.tddr;
			}
			break;
		}

		case 0x28:	break;		// PDWSR
		default:	fatalerror("32051: cpuregs_w: unimplemented memory-mapped register %02X, data %04X at %04X\n", offset, data, tms.pc-1);
	}
}

/**************************************************************************
 * Internal memory map
 **************************************************************************/

static ADDRESS_MAP_START( internal_pgm, ADDRESS_SPACE_PROGRAM, 16 )
	AM_RANGE(0x2000, 0x23ff) AM_RAM					// SARAM
	AM_RANGE(0xfe00, 0xffff) AM_RAM AM_SHARE(11)	// DARAM B0
ADDRESS_MAP_END

static ADDRESS_MAP_START( internal_data, ADDRESS_SPACE_DATA, 16 )
	AM_RANGE(0x0000, 0x005f) AM_READWRITE(cpuregs_r, cpuregs_w)
	AM_RANGE(0x0060, 0x007f) AM_RAM					// DARAM B2
	AM_RANGE(0x0100, 0x02ff) AM_RAM AM_SHARE(11)	// DARAM B0
	AM_RANGE(0x0300, 0x04ff) AM_RAM					// DARAM B1
ADDRESS_MAP_END

/**************************************************************************
 * Generic set_info
 **************************************************************************/

static void tms_set_info(UINT32 state, cpuinfo *info)
{
	switch (state)
	{
		case CPUINFO_INT_PC:
		case CPUINFO_INT_REGISTER + TMS32051_PC:		tms.pc = info->i;						break;
	}
}

static int tms_debug_read(int space, UINT32 offset, int size, UINT64 *value)
{
	if (space == ADDRESS_SPACE_PROGRAM)
	{
		*value = (PM_READ16(offset>>1) >> ((offset & 1) ? 0 : 8)) & 0xff;
	}
	else if (space == ADDRESS_SPACE_DATA)
	{
		*value = (DM_READ16(offset>>1) >> ((offset & 1) ? 0 : 8)) & 0xff;
	}
	return 1;
}

static void tms_get_info(UINT32 state, cpuinfo *info)
{
	switch(state)
	{
		/* --- the following bits of info are returned as 64-bit signed integers --- */
		case CPUINFO_INT_CONTEXT_SIZE:					info->i = sizeof(tms);					break;
		case CPUINFO_INT_INPUT_LINES:					info->i = 6;							break;
		case CPUINFO_INT_DEFAULT_IRQ_VECTOR:			info->i = 0;							break;
		case CPUINFO_INT_ENDIANNESS:					info->i = CPU_IS_LE;					break;
		case CPUINFO_INT_CLOCK_MULTIPLIER:				info->i = 1;							break;
		case CPUINFO_INT_CLOCK_DIVIDER:					info->i = 1;							break;
		case CPUINFO_INT_MIN_INSTRUCTION_BYTES:			info->i = 2;							break;
		case CPUINFO_INT_MAX_INSTRUCTION_BYTES:			info->i = 4;							break;
		case CPUINFO_INT_MIN_CYCLES:					info->i = 1;							break;
		case CPUINFO_INT_MAX_CYCLES:					info->i = 5;							break;

		case CPUINFO_INT_DATABUS_WIDTH + ADDRESS_SPACE_PROGRAM:	info->i = 16;					break;
		case CPUINFO_INT_ADDRBUS_WIDTH + ADDRESS_SPACE_PROGRAM: info->i = 16;					break;
		case CPUINFO_INT_ADDRBUS_SHIFT + ADDRESS_SPACE_PROGRAM: info->i = -1;					break;
		case CPUINFO_INT_DATABUS_WIDTH + ADDRESS_SPACE_DATA:	info->i = 16;					break;
		case CPUINFO_INT_ADDRBUS_WIDTH + ADDRESS_SPACE_DATA: 	info->i = 16;					break;
		case CPUINFO_INT_ADDRBUS_SHIFT + ADDRESS_SPACE_DATA: 	info->i = -1;					break;
		case CPUINFO_INT_DATABUS_WIDTH + ADDRESS_SPACE_IO:		info->i = 0;					break;
		case CPUINFO_INT_ADDRBUS_WIDTH + ADDRESS_SPACE_IO: 		info->i = 0;					break;
		case CPUINFO_INT_ADDRBUS_SHIFT + ADDRESS_SPACE_IO: 		info->i = 0;					break;

		case CPUINFO_INT_INPUT_STATE:					info->i = CLEAR_LINE;					break;

		case CPUINFO_INT_PREVIOUSPC:					/* not implemented */					break;

		case CPUINFO_INT_PC:
		case CPUINFO_INT_REGISTER + TMS32051_PC:		info->i = tms.pc;						break;
		case CPUINFO_INT_REGISTER + TMS32051_ACC:		info->i = tms.acc;						break;
		case CPUINFO_INT_REGISTER + TMS32051_ACCB:		info->i = tms.accb;						break;
		case CPUINFO_INT_REGISTER + TMS32051_PREG:		info->i = tms.preg;						break;
		case CPUINFO_INT_REGISTER + TMS32051_TREG0:		info->i = tms.treg0;					break;
		case CPUINFO_INT_REGISTER + TMS32051_TREG1:		info->i = tms.treg1;					break;
		case CPUINFO_INT_REGISTER + TMS32051_TREG2:		info->i = tms.treg2;					break;
		case CPUINFO_INT_REGISTER + TMS32051_BMAR:		info->i = tms.bmar;						break;
		case CPUINFO_INT_REGISTER + TMS32051_RPTC:		info->i = tms.rptc;						break;
		case CPUINFO_INT_REGISTER + TMS32051_BRCR:		info->i = tms.brcr;						break;
		case CPUINFO_INT_REGISTER + TMS32051_INDX:		info->i = tms.indx;						break;
		case CPUINFO_INT_REGISTER + TMS32051_DBMR:		info->i = tms.dbmr;						break;
		case CPUINFO_INT_REGISTER + TMS32051_ARCR:		info->i = tms.arcr;						break;
		case CPUINFO_INT_REGISTER + TMS32051_DP:		info->i = tms.st0.dp;					break;
		case CPUINFO_INT_REGISTER + TMS32051_ARP:		info->i = tms.st0.arp;					break;
		case CPUINFO_INT_REGISTER + TMS32051_ARB:		info->i = tms.st1.arb;					break;
		case CPUINFO_INT_REGISTER + TMS32051_AR0:		info->i = tms.ar[0];					break;
		case CPUINFO_INT_REGISTER + TMS32051_AR1:		info->i = tms.ar[1];					break;
		case CPUINFO_INT_REGISTER + TMS32051_AR2:		info->i = tms.ar[2];					break;
		case CPUINFO_INT_REGISTER + TMS32051_AR3:		info->i = tms.ar[3];					break;
		case CPUINFO_INT_REGISTER + TMS32051_AR4:		info->i = tms.ar[4];					break;
		case CPUINFO_INT_REGISTER + TMS32051_AR5:		info->i = tms.ar[5];					break;
		case CPUINFO_INT_REGISTER + TMS32051_AR6:		info->i = tms.ar[6];					break;
		case CPUINFO_INT_REGISTER + TMS32051_AR7:		info->i = tms.ar[7];					break;

		/* --- the following bits of info are returned as pointers to data or functions --- */
		case CPUINFO_PTR_GET_CONTEXT:					info->getcontext = tms_get_context;		break;
		case CPUINFO_PTR_SET_CONTEXT:					info->setcontext = tms_set_context;		break;
		case CPUINFO_PTR_INIT:							info->init = tms_init;					break;
		case CPUINFO_PTR_RESET:							info->reset = tms_reset;				break;
		case CPUINFO_PTR_EXIT:							info->exit = tms_exit;					break;
		case CPUINFO_PTR_EXECUTE:						info->execute = tms_execute;			break;
		case CPUINFO_PTR_BURN:							info->burn = NULL;						break;
		case CPUINFO_PTR_DISASSEMBLE:					info->disassemble = tms32051_dasm;		break;
		case CPUINFO_PTR_READ:							info->read = tms_debug_read;			break;
		case CPUINFO_PTR_INSTRUCTION_COUNTER:			info->icount = &tms_icount;				break;
		case CPUINFO_PTR_INTERNAL_MEMORY_MAP + ADDRESS_SPACE_PROGRAM: info->internal_map16 = address_map_internal_pgm; break;
		case CPUINFO_PTR_INTERNAL_MEMORY_MAP + ADDRESS_SPACE_DATA: info->internal_map16 = address_map_internal_data; break;

		/* --- the following bits of info are returned as NULL-terminated strings --- */
		case CPUINFO_STR_CORE_FAMILY:					strcpy(info->s, "TMS3205x");			break;
		case CPUINFO_STR_CORE_VERSION:					strcpy(info->s, "1.0");					break;
		case CPUINFO_STR_CORE_FILE:						strcpy(info->s, __FILE__);				break;
		case CPUINFO_STR_CORE_CREDITS:					strcpy(info->s, "Copyright Ville Linde"); break;

		case CPUINFO_STR_FLAGS:							strcpy(info->s, " ");					break;

		case CPUINFO_STR_REGISTER + TMS32051_PC:		sprintf(info->s, "PC: %04X", tms.pc); break;
		case CPUINFO_STR_REGISTER + TMS32051_ACC:		sprintf(info->s, "ACC: %08X", tms.acc); break;
		case CPUINFO_STR_REGISTER + TMS32051_ACCB:		sprintf(info->s, "ACCB: %08X", tms.accb); break;
		case CPUINFO_STR_REGISTER + TMS32051_PREG:		sprintf(info->s, "PREG: %08X", tms.preg); break;
		case CPUINFO_STR_REGISTER + TMS32051_TREG0:		sprintf(info->s, "TREG0: %04X", tms.treg0); break;
		case CPUINFO_STR_REGISTER + TMS32051_TREG1:		sprintf(info->s, "TREG1: %04X", tms.treg1); break;
		case CPUINFO_STR_REGISTER + TMS32051_TREG2:		sprintf(info->s, "TREG2: %04X", tms.treg2); break;
		case CPUINFO_STR_REGISTER + TMS32051_BMAR:		sprintf(info->s, "BMAR: %08X", tms.bmar); break;
		case CPUINFO_STR_REGISTER + TMS32051_RPTC:		sprintf(info->s, "RPTC: %08X", tms.rptc); break;
		case CPUINFO_STR_REGISTER + TMS32051_BRCR:		sprintf(info->s, "BRCR: %08X", tms.brcr); break;
		case CPUINFO_STR_REGISTER + TMS32051_INDX:		sprintf(info->s, "INDX: %04X", tms.indx); break;
		case CPUINFO_STR_REGISTER + TMS32051_DBMR:		sprintf(info->s, "DBMR: %04X", tms.dbmr); break;
		case CPUINFO_STR_REGISTER + TMS32051_ARCR:		sprintf(info->s, "ARCR: %04X", tms.arcr); break;
		case CPUINFO_STR_REGISTER + TMS32051_DP:		sprintf(info->s, "DP: %04X", tms.st0.dp); break;
		case CPUINFO_STR_REGISTER + TMS32051_ARP:		sprintf(info->s, "ARP: %04X", tms.st0.arp); break;
		case CPUINFO_STR_REGISTER + TMS32051_ARB:		sprintf(info->s, "ARB: %04X", tms.st1.arb); break;
		case CPUINFO_STR_REGISTER + TMS32051_AR0:		sprintf(info->s, "AR0: %04X", tms.ar[0]); break;
		case CPUINFO_STR_REGISTER + TMS32051_AR1:		sprintf(info->s, "AR1: %04X", tms.ar[1]); break;
		case CPUINFO_STR_REGISTER + TMS32051_AR2:		sprintf(info->s, "AR2: %04X", tms.ar[2]); break;
		case CPUINFO_STR_REGISTER + TMS32051_AR3:		sprintf(info->s, "AR3: %04X", tms.ar[3]); break;
		case CPUINFO_STR_REGISTER + TMS32051_AR4:		sprintf(info->s, "AR4: %04X", tms.ar[4]); break;
		case CPUINFO_STR_REGISTER + TMS32051_AR5:		sprintf(info->s, "AR5: %04X", tms.ar[5]); break;
		case CPUINFO_STR_REGISTER + TMS32051_AR6:		sprintf(info->s, "AR6: %04X", tms.ar[6]); break;
		case CPUINFO_STR_REGISTER + TMS32051_AR7:		sprintf(info->s, "AR7: %04X", tms.ar[7]); break;
	}
}

#if (HAS_TMS32051)
static void tms32051_set_info(UINT32 state, cpuinfo *info)
{
	if (state >= CPUINFO_INT_INPUT_STATE && state <= CPUINFO_INT_INPUT_STATE + 5)
	{
		return;
	}
	switch(state)
	{
		case CPUINFO_INT_PC:
		case CPUINFO_INT_REGISTER + TMS32051_PC:		tms.pc = info->i; 						break;
		case CPUINFO_INT_REGISTER + TMS32051_ACC:		tms.acc = info->i; 						break;
		case CPUINFO_INT_REGISTER + TMS32051_ACCB:		tms.accb = info->i;						break;
		case CPUINFO_INT_REGISTER + TMS32051_PREG:		tms.preg = info->i;						break;
		case CPUINFO_INT_REGISTER + TMS32051_TREG0:		tms.treg0 = info->i;					break;
		case CPUINFO_INT_REGISTER + TMS32051_TREG1:		tms.treg1 = info->i;					break;
		case CPUINFO_INT_REGISTER + TMS32051_TREG2:		tms.treg2 = info->i;					break;
		case CPUINFO_INT_REGISTER + TMS32051_BMAR:		tms.bmar = info->i;						break;
		case CPUINFO_INT_REGISTER + TMS32051_BRCR:		tms.brcr = info->i;						break;
		case CPUINFO_INT_REGISTER + TMS32051_INDX:		tms.indx = info->i;						break;
		case CPUINFO_INT_REGISTER + TMS32051_DBMR:		tms.dbmr = info->i;						break;
		case CPUINFO_INT_REGISTER + TMS32051_ARCR:		tms.arcr = info->i;						break;
		case CPUINFO_INT_REGISTER + TMS32051_DP:		tms.st0.dp = info->i;					break;
		case CPUINFO_INT_REGISTER + TMS32051_ARP:		tms.st0.arp = info->i;					break;
		case CPUINFO_INT_REGISTER + TMS32051_ARB:		tms.st1.arb = info->i;					break;
		case CPUINFO_INT_REGISTER + TMS32051_AR0:		tms.ar[0] = info->i; 					break;
		case CPUINFO_INT_REGISTER + TMS32051_AR1:		tms.ar[1] = info->i; 					break;
		case CPUINFO_INT_REGISTER + TMS32051_AR2:		tms.ar[2] = info->i; 					break;
		case CPUINFO_INT_REGISTER + TMS32051_AR3:		tms.ar[3] = info->i; 					break;
		case CPUINFO_INT_REGISTER + TMS32051_AR4:		tms.ar[4] = info->i; 					break;
		case CPUINFO_INT_REGISTER + TMS32051_AR5:		tms.ar[5] = info->i; 					break;
		case CPUINFO_INT_REGISTER + TMS32051_AR6:		tms.ar[6] = info->i; 					break;
		case CPUINFO_INT_REGISTER + TMS32051_AR7:		tms.ar[7] = info->i; 					break;

		default:										tms_set_info(state, info);				break;
	}
}

void tms32051_get_info(UINT32 state, cpuinfo *info)
{
	switch(state)
	{
		/* --- the following bits of info are returned as pointers to data or functions --- */
		case CPUINFO_PTR_SET_INFO:						info->setinfo = tms32051_set_info;		break;

		/* --- the following bits of info are returned as NULL-terminated strings --- */
		case CPUINFO_STR_NAME:							strcpy(info->s, "TMS32051");			break;

		default:										tms_get_info(state, info);				break;
	}
}
#endif
