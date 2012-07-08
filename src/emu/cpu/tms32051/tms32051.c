/*
   Texas Instruments TMS320C51 DSP Emulator

   Written by Ville Linde
*/

#include "emu.h"
#include "debugger.h"
#include "tms32051.h"

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

typedef struct _tms32051_state tms32051_state;
struct _tms32051_state
{
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

	device_irq_acknowledge_callback irq_callback;
	legacy_cpu_device *device;
	address_space *program;
	direct_read_data *direct;
	address_space *data;
	int icount;
};

INLINE tms32051_state *get_safe_token(device_t *device)
{
	assert(device != NULL);
	assert(device->type() == TMS32051);
	return (tms32051_state *)downcast<legacy_cpu_device *>(device)->token();
}

static void delay_slot(tms32051_state *cpustate, UINT16 startpc);
static void save_interrupt_context(tms32051_state *cpustate);
static void restore_interrupt_context(tms32051_state *cpustate);
static void check_interrupts(tms32051_state *cpustate);


#define CYCLES(x)		(cpustate->icount -= x)

#define ROPCODE(cpustate)		cpustate->direct->read_decrypted_word((cpustate->pc++) << 1)

INLINE void CHANGE_PC(tms32051_state *cpustate, UINT16 new_pc)
{
	cpustate->pc = new_pc;
}

INLINE UINT16 PM_READ16(tms32051_state *cpustate, UINT16 address)
{
	return cpustate->program->read_word(address << 1);
}

INLINE void PM_WRITE16(tms32051_state *cpustate, UINT16 address, UINT16 data)
{
	cpustate->program->write_word(address << 1, data);
}

INLINE UINT16 DM_READ16(tms32051_state *cpustate, UINT16 address)
{
	return cpustate->data->read_word(address << 1);
}

INLINE void DM_WRITE16(tms32051_state *cpustate, UINT16 address, UINT16 data)
{
	cpustate->data->write_word(address << 1, data);
}

#include "32051ops.c"
#include "32051ops.h"

static void op_group_be(tms32051_state *cpustate)
{
	tms32051_opcode_table_be[cpustate->op & 0xff](cpustate);
}

static void op_group_bf(tms32051_state *cpustate)
{
	tms32051_opcode_table_bf[cpustate->op & 0xff](cpustate);
}

static void delay_slot(tms32051_state *cpustate, UINT16 startpc)
{
	cpustate->op = ROPCODE(cpustate);
	tms32051_opcode_table[cpustate->op >> 8](cpustate);

	while (cpustate->pc - startpc < 2)
	{
		cpustate->op = ROPCODE(cpustate);
		tms32051_opcode_table[cpustate->op >> 8](cpustate);
	}
}

/*****************************************************************************/

static CPU_INIT( tms )
{
	tms32051_state *cpustate = get_safe_token(device);

	cpustate->device = device;
	cpustate->program = device->space(AS_PROGRAM);
	cpustate->direct = &cpustate->program->direct();
	cpustate->data = device->space(AS_DATA);
}

static CPU_RESET( tms )
{
	tms32051_state *cpustate = get_safe_token(device);
	int i;
	UINT16 src, dst, length;

	src = 0x7800;
	dst = DM_READ16(cpustate, src++);
	length = DM_READ16(cpustate, src++);

	CHANGE_PC(cpustate, dst);

	/* TODO: if you soft reset on Taito JC it tries to do a 0x7802->0x9007 (0xff00) transfer. */
	for (i=0; i < (length & 0x7ff); i++)
	{
		UINT16 data = DM_READ16(cpustate, src++);
		PM_WRITE16(cpustate, dst++, data);
	}

	cpustate->st0.intm	= 1;
	cpustate->st0.ov	= 0;
	cpustate->st1.c		= 1;
	cpustate->st1.cnf	= 0;
	cpustate->st1.hm	= 1;
	cpustate->st1.pm	= 0;
	cpustate->st1.sxm	= 1;
	cpustate->st1.xf	= 1;
	cpustate->pmst.avis	= 0;
	cpustate->pmst.braf	= 0;
	cpustate->pmst.iptr	= 0;
	cpustate->pmst.ndx	= 0;
	cpustate->pmst.ovly	= 0;
	cpustate->pmst.ram	= 0;
	cpustate->pmst.mpmc	= 0; // TODO: this is set to logical pin state at reset
	cpustate->pmst.trm	= 0;
	cpustate->ifr		= 0;
	cpustate->cbcr		= 0;
	cpustate->rptc		= -1;
}

static void check_interrupts(tms32051_state *cpustate)
{
	int i;

	if (cpustate->st0.intm == 0 && cpustate->ifr != 0)
	{
		for (i=0; i < 16; i++)
		{
			if (cpustate->ifr & (1 << i))
			{
				cpustate->st0.intm = 1;
				PUSH_STACK(cpustate, cpustate->pc);

				cpustate->pc = (cpustate->pmst.iptr << 11) | ((i+1) << 1);
				cpustate->ifr &= ~(1 << i);

				save_interrupt_context(cpustate);
				break;
			}
		}
	}
}

static void save_interrupt_context(tms32051_state *cpustate)
{
	cpustate->shadow.acc		= cpustate->acc;
	cpustate->shadow.accb		= cpustate->accb;
	cpustate->shadow.arcr		= cpustate->arcr;
	cpustate->shadow.indx		= cpustate->indx;
	cpustate->shadow.preg		= cpustate->preg;
	cpustate->shadow.treg0	= cpustate->treg0;
	cpustate->shadow.treg1	= cpustate->treg1;
	cpustate->shadow.treg2	= cpustate->treg2;
	memcpy(&cpustate->shadow.pmst, &cpustate->pmst, sizeof(PMST));
	memcpy(&cpustate->shadow.st0, &cpustate->st0, sizeof(ST0));
	memcpy(&cpustate->shadow.st1, &cpustate->st1, sizeof(ST1));
}

static void restore_interrupt_context(tms32051_state *cpustate)
{
	cpustate->acc				= cpustate->shadow.acc;
	cpustate->accb			= cpustate->shadow.accb;
	cpustate->arcr			= cpustate->shadow.arcr;
	cpustate->indx			= cpustate->shadow.indx;
	cpustate->preg			= cpustate->shadow.preg;
	cpustate->treg0			= cpustate->shadow.treg0;
	cpustate->treg1			= cpustate->shadow.treg1;
	cpustate->treg2			= cpustate->shadow.treg2;
	memcpy(&cpustate->pmst, &cpustate->shadow.pmst, sizeof(PMST));
	memcpy(&cpustate->st0, &cpustate->shadow.st0, sizeof(ST0));
	memcpy(&cpustate->st1, &cpustate->shadow.st1, sizeof(ST1));
}

static void tms_interrupt(tms32051_state *cpustate, int irq)
{
	if ((cpustate->imr & (1 << irq)) != 0)
	{
		cpustate->ifr |= 1 << irq;
	}

	check_interrupts(cpustate);
}

static CPU_EXIT( tms )
{
	/* TODO */
}

static CPU_EXECUTE( tms )
{
	tms32051_state *cpustate = get_safe_token(device);

	while(cpustate->icount > 0)
	{
		UINT16 ppc;

		// handle block repeat
		if (cpustate->pmst.braf)
		{
			if (cpustate->pc == cpustate->paer)
			{
				if (cpustate->brcr > 0)
				{
					CHANGE_PC(cpustate, cpustate->pasr);
				}

				cpustate->brcr--;
				if (cpustate->brcr <= 0)
				{
					cpustate->pmst.braf = 0;
				}
			}
		}

		ppc = cpustate->pc;
		debugger_instruction_hook(device, cpustate->pc);

		cpustate->op = ROPCODE(cpustate);
		tms32051_opcode_table[cpustate->op >> 8](cpustate);

		// handle single repeat
		if (cpustate->rptc > 0)
		{
			if (ppc == cpustate->rpt_end)
			{
				CHANGE_PC(cpustate, cpustate->rpt_start);
				cpustate->rptc--;
			}
		}
		else
		{
			cpustate->rptc = 0;
		}

		cpustate->timer.psc--;
		if (cpustate->timer.psc <= 0)
		{
			cpustate->timer.psc = cpustate->timer.tddr;
			cpustate->timer.tim--;
			if (cpustate->timer.tim <= 0)
			{
				// reset timer
				cpustate->timer.tim = cpustate->timer.prd;

				tms_interrupt(cpustate, INTERRUPT_TINT);
			}
		}
	}
}


/*****************************************************************************/

static READ16_HANDLER( cpuregs_r )
{
	tms32051_state *cpustate = get_safe_token(&space->device());

	switch (offset)
	{
		case 0x04:	return cpustate->imr;
		case 0x06:	return cpustate->ifr;

		case 0x07:		// PMST
		{
			UINT16 r = 0;
			r |= cpustate->pmst.iptr << 11;
			r |= cpustate->pmst.avis << 7;
			r |= cpustate->pmst.ovly << 5;
			r |= cpustate->pmst.ram << 4;
			r |= cpustate->pmst.mpmc << 3;
			r |= cpustate->pmst.ndx << 2;
			r |= cpustate->pmst.trm << 1;
			r |= cpustate->pmst.braf << 0;
			return r;
		}

		case 0x09:	return cpustate->brcr;
		case 0x10:	return cpustate->ar[0];
		case 0x11:	return cpustate->ar[1];
		case 0x12:	return cpustate->ar[2];
		case 0x13:	return cpustate->ar[3];
		case 0x14:	return cpustate->ar[4];
		case 0x15:	return cpustate->ar[5];
		case 0x16:	return cpustate->ar[6];
		case 0x17:	return cpustate->ar[7];
		case 0x1e:	return cpustate->cbcr;
		case 0x1f:	return cpustate->bmar;
		case 0x24:	return cpustate->timer.tim;
		case 0x25:	return cpustate->timer.prd;

		case 0x26:		// TCR
		{
			UINT16 r = 0;
			r |= (cpustate->timer.psc & 0xf) << 6;
			r |= (cpustate->timer.tddr & 0xf);
			return r;
		}

		case 0x28:	return 0;	// PDWSR
		default:
		if(!space->debugger_access())
			fatalerror("32051: cpuregs_r: unimplemented memory-mapped register %02X at %04X\n", offset, cpustate->pc-1);
	}

	return 0;
}

static WRITE16_HANDLER( cpuregs_w )
{
	tms32051_state *cpustate = get_safe_token(&space->device());

	switch (offset)
	{
		case 0x00:	break;
		case 0x04:	cpustate->imr = data; break;
		case 0x06:		// IFR
		{
			int i;
			for (i=0; i < 16; i++)
			{
				if (data & (1 << i))
				{
					cpustate->ifr &= ~(1 << i);
				}
			}
			break;
		}

		case 0x07:		// PMST
		{
			cpustate->pmst.iptr = (data >> 11) & 0x1f;
			cpustate->pmst.avis = (data & 0x80) ? 1 : 0;
			cpustate->pmst.ovly = (data & 0x20) ? 1 : 0;
			cpustate->pmst.ram = (data & 0x10) ? 1 : 0;
			cpustate->pmst.mpmc = (data & 0x08) ? 1 : 0;
			cpustate->pmst.ndx = (data & 0x04) ? 1 : 0;
			cpustate->pmst.trm = (data & 0x02) ? 1 : 0;
			cpustate->pmst.braf = (data & 0x01) ? 1 : 0;
			break;
		}

		case 0x09:	cpustate->brcr = data; break;
		case 0x0e:	cpustate->treg2 = data; break;
		case 0x0f:	cpustate->dbmr = data; break;
		case 0x10:	cpustate->ar[0] = data; break;
		case 0x11:	cpustate->ar[1] = data; break;
		case 0x12:	cpustate->ar[2] = data; break;
		case 0x13:	cpustate->ar[3] = data; break;
		case 0x14:	cpustate->ar[4] = data; break;
		case 0x15:	cpustate->ar[5] = data; break;
		case 0x16:	cpustate->ar[6] = data; break;
		case 0x17:	cpustate->ar[7] = data; break;
		case 0x18:	cpustate->indx = data; break;
		case 0x19:	cpustate->arcr = data; break;
		case 0x1a:	cpustate->cbsr1 = data; break;
		case 0x1b:	cpustate->cber1 = data; break;
		case 0x1c:	cpustate->cbsr2 = data; break;
		case 0x1d:	cpustate->cber2 = data; break;
		case 0x1e:	cpustate->cbcr = data; break;
		case 0x1f:	cpustate->bmar = data; break;
		case 0x24:	cpustate->timer.tim = data; break;
		case 0x25:	cpustate->timer.prd = data; break;

		case 0x26:		// TCR
		{
			cpustate->timer.tddr = data & 0xf;
			cpustate->timer.psc = (data >> 6) & 0xf;

			if (data & 0x20)
			{
				cpustate->timer.tim = cpustate->timer.prd;
				cpustate->timer.psc = cpustate->timer.tddr;
			}
			break;
		}

		case 0x28:	break;		// PDWSR
		default:
		if(!space->debugger_access())
			fatalerror("32051: cpuregs_w: unimplemented memory-mapped register %02X, data %04X at %04X\n", offset, data, cpustate->pc-1);
	}
}

/**************************************************************************
 * Internal memory map
 **************************************************************************/

static ADDRESS_MAP_START( internal_pgm, AS_PROGRAM, 16, legacy_cpu_device )
	AM_RANGE(0x0000, 0x1fff) AM_ROM							// ROM			TODO: is off-chip if MP/_MC = 0
	AM_RANGE(0x2000, 0x23ff) AM_RAM	AM_SHARE("saram")		// SARAM		TODO: is off-chip if RAM bit = 0
	AM_RANGE(0xfe00, 0xffff) AM_RAM AM_SHARE("daram_b0")	// DARAM B0		TODO: is off-chip if CNF = 0
ADDRESS_MAP_END

static ADDRESS_MAP_START( internal_data, AS_DATA, 16, legacy_cpu_device )
	AM_RANGE(0x0000, 0x005f) AM_READWRITE_LEGACY(cpuregs_r, cpuregs_w)
	AM_RANGE(0x0060, 0x007f) AM_RAM							// DARAM B2
	AM_RANGE(0x0100, 0x02ff) AM_RAM AM_SHARE("daram_b0")	// DARAM B0		TODO: is unconnected if CNF = 1
	AM_RANGE(0x0300, 0x04ff) AM_RAM							// DARAM B1
	AM_RANGE(0x0800, 0x0bff) AM_RAM AM_SHARE("saram")		// SARAM		TODO: is off-chip if OVLY = 0
ADDRESS_MAP_END

/**************************************************************************
 * Generic set_info
 **************************************************************************/

static CPU_SET_INFO( tms )
{
	tms32051_state *cpustate = get_safe_token(device);

	switch (state)
	{
		case CPUINFO_INT_PC:
		case CPUINFO_INT_REGISTER + TMS32051_PC:		cpustate->pc = info->i;						break;
	}
}

static CPU_READ( tms )
{
	tms32051_state *cpustate = get_safe_token(device);
	/* TODO: alignment if offset is odd */
	if (space == AS_PROGRAM)
	{
		*value = (PM_READ16(cpustate, offset>>1));
	}
	else if (space == AS_DATA)
	{
		*value = (DM_READ16(cpustate, offset>>1));
	}
	return 1;
}

static CPU_GET_INFO( tms )
{
	tms32051_state *cpustate = (device != NULL && device->token() != NULL) ? get_safe_token(device) : NULL;

	switch(state)
	{
		/* --- the following bits of info are returned as 64-bit signed integers --- */
		case CPUINFO_INT_CONTEXT_SIZE:					info->i = sizeof(tms32051_state);					break;
		case CPUINFO_INT_INPUT_LINES:					info->i = 6;							break;
		case CPUINFO_INT_DEFAULT_IRQ_VECTOR:			info->i = 0;							break;
		case DEVINFO_INT_ENDIANNESS:					info->i = ENDIANNESS_LITTLE;					break;
		case CPUINFO_INT_CLOCK_MULTIPLIER:				info->i = 1;							break;
		case CPUINFO_INT_CLOCK_DIVIDER:					info->i = 1;							break;
		case CPUINFO_INT_MIN_INSTRUCTION_BYTES:			info->i = 2;							break;
		case CPUINFO_INT_MAX_INSTRUCTION_BYTES:			info->i = 4;							break;
		case CPUINFO_INT_MIN_CYCLES:					info->i = 1;							break;
		case CPUINFO_INT_MAX_CYCLES:					info->i = 5;							break;

		case DEVINFO_INT_DATABUS_WIDTH + AS_PROGRAM:	info->i = 16;					break;
		case DEVINFO_INT_ADDRBUS_WIDTH + AS_PROGRAM: info->i = 16;					break;
		case DEVINFO_INT_ADDRBUS_SHIFT + AS_PROGRAM: info->i = -1;					break;
		case DEVINFO_INT_DATABUS_WIDTH + AS_DATA:	info->i = 16;					break;
		case DEVINFO_INT_ADDRBUS_WIDTH + AS_DATA:	info->i = 16;					break;
		case DEVINFO_INT_ADDRBUS_SHIFT + AS_DATA:	info->i = -1;					break;
		case DEVINFO_INT_DATABUS_WIDTH + AS_IO:		info->i = 0;					break;
		case DEVINFO_INT_ADDRBUS_WIDTH + AS_IO:		info->i = 0;					break;
		case DEVINFO_INT_ADDRBUS_SHIFT + AS_IO:		info->i = 0;					break;

		case CPUINFO_INT_INPUT_STATE:					info->i = CLEAR_LINE;					break;

		case CPUINFO_INT_PREVIOUSPC:					/* not implemented */					break;

		case CPUINFO_INT_PC:
		case CPUINFO_INT_REGISTER + TMS32051_PC:		info->i = cpustate->pc;						break;
		case CPUINFO_INT_REGISTER + TMS32051_ACC:		info->i = cpustate->acc;						break;
		case CPUINFO_INT_REGISTER + TMS32051_ACCB:		info->i = cpustate->accb;						break;
		case CPUINFO_INT_REGISTER + TMS32051_PREG:		info->i = cpustate->preg;						break;
		case CPUINFO_INT_REGISTER + TMS32051_TREG0:		info->i = cpustate->treg0;					break;
		case CPUINFO_INT_REGISTER + TMS32051_TREG1:		info->i = cpustate->treg1;					break;
		case CPUINFO_INT_REGISTER + TMS32051_TREG2:		info->i = cpustate->treg2;					break;
		case CPUINFO_INT_REGISTER + TMS32051_BMAR:		info->i = cpustate->bmar;						break;
		case CPUINFO_INT_REGISTER + TMS32051_RPTC:		info->i = cpustate->rptc;						break;
		case CPUINFO_INT_REGISTER + TMS32051_BRCR:		info->i = cpustate->brcr;						break;
		case CPUINFO_INT_REGISTER + TMS32051_INDX:		info->i = cpustate->indx;						break;
		case CPUINFO_INT_REGISTER + TMS32051_DBMR:		info->i = cpustate->dbmr;						break;
		case CPUINFO_INT_REGISTER + TMS32051_ARCR:		info->i = cpustate->arcr;						break;
		case CPUINFO_INT_REGISTER + TMS32051_DP:		info->i = cpustate->st0.dp;					break;
		case CPUINFO_INT_REGISTER + TMS32051_ARP:		info->i = cpustate->st0.arp;					break;
		case CPUINFO_INT_REGISTER + TMS32051_ARB:		info->i = cpustate->st1.arb;					break;
		case CPUINFO_INT_REGISTER + TMS32051_AR0:		info->i = cpustate->ar[0];					break;
		case CPUINFO_INT_REGISTER + TMS32051_AR1:		info->i = cpustate->ar[1];					break;
		case CPUINFO_INT_REGISTER + TMS32051_AR2:		info->i = cpustate->ar[2];					break;
		case CPUINFO_INT_REGISTER + TMS32051_AR3:		info->i = cpustate->ar[3];					break;
		case CPUINFO_INT_REGISTER + TMS32051_AR4:		info->i = cpustate->ar[4];					break;
		case CPUINFO_INT_REGISTER + TMS32051_AR5:		info->i = cpustate->ar[5];					break;
		case CPUINFO_INT_REGISTER + TMS32051_AR6:		info->i = cpustate->ar[6];					break;
		case CPUINFO_INT_REGISTER + TMS32051_AR7:		info->i = cpustate->ar[7];					break;

		/* --- the following bits of info are returned as pointers to data or functions --- */
		case CPUINFO_FCT_INIT:							info->init = CPU_INIT_NAME(tms);					break;
		case CPUINFO_FCT_RESET:							info->reset = CPU_RESET_NAME(tms);				break;
		case CPUINFO_FCT_EXIT:							info->exit = CPU_EXIT_NAME(tms);					break;
		case CPUINFO_FCT_EXECUTE:						info->execute = CPU_EXECUTE_NAME(tms);			break;
		case CPUINFO_FCT_BURN:							info->burn = NULL;						break;
		case CPUINFO_FCT_DISASSEMBLE:					info->disassemble = CPU_DISASSEMBLE_NAME(tms32051);		break;
		case CPUINFO_FCT_READ:							info->read = CPU_READ_NAME(tms);			break;
		case CPUINFO_PTR_INSTRUCTION_COUNTER:			info->icount = &cpustate->icount;				break;
		case DEVINFO_PTR_INTERNAL_MEMORY_MAP + AS_PROGRAM: info->internal_map16 = ADDRESS_MAP_NAME(internal_pgm); break;
		case DEVINFO_PTR_INTERNAL_MEMORY_MAP + AS_DATA: info->internal_map16 = ADDRESS_MAP_NAME(internal_data); break;

		/* --- the following bits of info are returned as NULL-terminated strings --- */
		case DEVINFO_STR_FAMILY:					strcpy(info->s, "TMS3205x");			break;
		case DEVINFO_STR_VERSION:					strcpy(info->s, "1.0");					break;
		case DEVINFO_STR_SOURCE_FILE:						strcpy(info->s, __FILE__);				break;
		case DEVINFO_STR_CREDITS:					strcpy(info->s, "Copyright Ville Linde"); break;

		case CPUINFO_STR_FLAGS:							strcpy(info->s, " ");					break;

		case CPUINFO_STR_REGISTER + TMS32051_PC:		sprintf(info->s, "PC: %04X", cpustate->pc); break;
		case CPUINFO_STR_REGISTER + TMS32051_ACC:		sprintf(info->s, "ACC: %08X", cpustate->acc); break;
		case CPUINFO_STR_REGISTER + TMS32051_ACCB:		sprintf(info->s, "ACCB: %08X", cpustate->accb); break;
		case CPUINFO_STR_REGISTER + TMS32051_PREG:		sprintf(info->s, "PREG: %08X", cpustate->preg); break;
		case CPUINFO_STR_REGISTER + TMS32051_TREG0:		sprintf(info->s, "TREG0: %04X", cpustate->treg0); break;
		case CPUINFO_STR_REGISTER + TMS32051_TREG1:		sprintf(info->s, "TREG1: %04X", cpustate->treg1); break;
		case CPUINFO_STR_REGISTER + TMS32051_TREG2:		sprintf(info->s, "TREG2: %04X", cpustate->treg2); break;
		case CPUINFO_STR_REGISTER + TMS32051_BMAR:		sprintf(info->s, "BMAR: %08X", cpustate->bmar); break;
		case CPUINFO_STR_REGISTER + TMS32051_RPTC:		sprintf(info->s, "RPTC: %08X", cpustate->rptc); break;
		case CPUINFO_STR_REGISTER + TMS32051_BRCR:		sprintf(info->s, "BRCR: %08X", cpustate->brcr); break;
		case CPUINFO_STR_REGISTER + TMS32051_INDX:		sprintf(info->s, "INDX: %04X", cpustate->indx); break;
		case CPUINFO_STR_REGISTER + TMS32051_DBMR:		sprintf(info->s, "DBMR: %04X", cpustate->dbmr); break;
		case CPUINFO_STR_REGISTER + TMS32051_ARCR:		sprintf(info->s, "ARCR: %04X", cpustate->arcr); break;
		case CPUINFO_STR_REGISTER + TMS32051_DP:		sprintf(info->s, "DP: %04X", cpustate->st0.dp); break;
		case CPUINFO_STR_REGISTER + TMS32051_ARP:		sprintf(info->s, "ARP: %04X", cpustate->st0.arp); break;
		case CPUINFO_STR_REGISTER + TMS32051_ARB:		sprintf(info->s, "ARB: %04X", cpustate->st1.arb); break;
		case CPUINFO_STR_REGISTER + TMS32051_AR0:		sprintf(info->s, "AR0: %04X", cpustate->ar[0]); break;
		case CPUINFO_STR_REGISTER + TMS32051_AR1:		sprintf(info->s, "AR1: %04X", cpustate->ar[1]); break;
		case CPUINFO_STR_REGISTER + TMS32051_AR2:		sprintf(info->s, "AR2: %04X", cpustate->ar[2]); break;
		case CPUINFO_STR_REGISTER + TMS32051_AR3:		sprintf(info->s, "AR3: %04X", cpustate->ar[3]); break;
		case CPUINFO_STR_REGISTER + TMS32051_AR4:		sprintf(info->s, "AR4: %04X", cpustate->ar[4]); break;
		case CPUINFO_STR_REGISTER + TMS32051_AR5:		sprintf(info->s, "AR5: %04X", cpustate->ar[5]); break;
		case CPUINFO_STR_REGISTER + TMS32051_AR6:		sprintf(info->s, "AR6: %04X", cpustate->ar[6]); break;
		case CPUINFO_STR_REGISTER + TMS32051_AR7:		sprintf(info->s, "AR7: %04X", cpustate->ar[7]); break;
	}
}

static CPU_SET_INFO( tms32051 )
{
	tms32051_state *cpustate = get_safe_token(device);

	if (state >= CPUINFO_INT_INPUT_STATE && state <= CPUINFO_INT_INPUT_STATE + 5)
	{
		return;
	}
	switch(state)
	{
		case CPUINFO_INT_PC:
		case CPUINFO_INT_REGISTER + TMS32051_PC:		cpustate->pc = info->i; 						break;
		case CPUINFO_INT_REGISTER + TMS32051_ACC:		cpustate->acc = info->i;						break;
		case CPUINFO_INT_REGISTER + TMS32051_ACCB:		cpustate->accb = info->i;						break;
		case CPUINFO_INT_REGISTER + TMS32051_PREG:		cpustate->preg = info->i;						break;
		case CPUINFO_INT_REGISTER + TMS32051_TREG0:		cpustate->treg0 = info->i;					break;
		case CPUINFO_INT_REGISTER + TMS32051_TREG1:		cpustate->treg1 = info->i;					break;
		case CPUINFO_INT_REGISTER + TMS32051_TREG2:		cpustate->treg2 = info->i;					break;
		case CPUINFO_INT_REGISTER + TMS32051_BMAR:		cpustate->bmar = info->i;						break;
		case CPUINFO_INT_REGISTER + TMS32051_BRCR:		cpustate->brcr = info->i;						break;
		case CPUINFO_INT_REGISTER + TMS32051_INDX:		cpustate->indx = info->i;						break;
		case CPUINFO_INT_REGISTER + TMS32051_DBMR:		cpustate->dbmr = info->i;						break;
		case CPUINFO_INT_REGISTER + TMS32051_ARCR:		cpustate->arcr = info->i;						break;
		case CPUINFO_INT_REGISTER + TMS32051_DP:		cpustate->st0.dp = info->i;					break;
		case CPUINFO_INT_REGISTER + TMS32051_ARP:		cpustate->st0.arp = info->i;					break;
		case CPUINFO_INT_REGISTER + TMS32051_ARB:		cpustate->st1.arb = info->i;					break;
		case CPUINFO_INT_REGISTER + TMS32051_AR0:		cpustate->ar[0] = info->i;					break;
		case CPUINFO_INT_REGISTER + TMS32051_AR1:		cpustate->ar[1] = info->i;					break;
		case CPUINFO_INT_REGISTER + TMS32051_AR2:		cpustate->ar[2] = info->i;					break;
		case CPUINFO_INT_REGISTER + TMS32051_AR3:		cpustate->ar[3] = info->i;					break;
		case CPUINFO_INT_REGISTER + TMS32051_AR4:		cpustate->ar[4] = info->i;					break;
		case CPUINFO_INT_REGISTER + TMS32051_AR5:		cpustate->ar[5] = info->i;					break;
		case CPUINFO_INT_REGISTER + TMS32051_AR6:		cpustate->ar[6] = info->i;					break;
		case CPUINFO_INT_REGISTER + TMS32051_AR7:		cpustate->ar[7] = info->i;					break;

		default:										CPU_SET_INFO_CALL(tms);				break;
	}
}

CPU_GET_INFO( tms32051 )
{
	switch(state)
	{
		/* --- the following bits of info are returned as pointers to data or functions --- */
		case CPUINFO_FCT_SET_INFO:						info->setinfo = CPU_SET_INFO_NAME(tms32051);		break;

		/* --- the following bits of info are returned as NULL-terminated strings --- */
		case DEVINFO_STR_NAME:							strcpy(info->s, "TMS32051");			break;

		default:										CPU_GET_INFO_CALL(tms);				break;
	}
}

DEFINE_LEGACY_CPU_DEVICE(TMS32051, tms32051);
