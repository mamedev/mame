/***************************************************************************

    mips3.c
    Core implementation for the portable MIPS III/IV emulator.
    Written by Aaron Giles

    Still not implemented:
       * DMULT needs to be fixed properly

***************************************************************************/

#include "emu.h"
#include "debugger.h"
#include "mips3.h"
#include "mips3com.h"


#define ENABLE_OVERFLOWS	0

#ifndef MIPS3_USE_DRC

/***************************************************************************
    HELPER MACROS
***************************************************************************/

#define RSVAL32		((UINT32)mips3.core.r[RSREG])
#define RTVAL32		((UINT32)mips3.core.r[RTREG])
#define RDVAL32		((UINT32)mips3.core.r[RDREG])

#define RSVAL64		(mips3.core.r[RSREG])
#define RTVAL64		(mips3.core.r[RTREG])
#define RDVAL64		(mips3.core.r[RDREG])

#define FRVALS_FR0	(((float *)&mips3.core.cpr[1][0])[FRREG])
#define FTVALS_FR0	(((float *)&mips3.core.cpr[1][0])[FTREG])
#define FSVALS_FR0	(((float *)&mips3.core.cpr[1][0])[FSREG])
#define FDVALS_FR0	(((float *)&mips3.core.cpr[1][0])[FDREG])
#define FSVALW_FR0	(((UINT32 *)&mips3.core.cpr[1][0])[FSREG])
#define FDVALW_FR0	(((UINT32 *)&mips3.core.cpr[1][0])[FDREG])

#define FRVALD_FR0	(*(double *)&mips3.core.cpr[1][FRREG/2])
#define FTVALD_FR0	(*(double *)&mips3.core.cpr[1][FTREG/2])
#define FSVALD_FR0	(*(double *)&mips3.core.cpr[1][FSREG/2])
#define FDVALD_FR0	(*(double *)&mips3.core.cpr[1][FDREG/2])
#define FSVALL_FR0	(((UINT64 *)&mips3.core.cpr[1][0])[FSREG/2])
#define FDVALL_FR0	(((UINT64 *)&mips3.core.cpr[1][0])[FDREG/2])

#define FRVALS_FR1	(((float *)&mips3.core.cpr[1][FRREG])[BYTE_XOR_LE(0)])
#define FTVALS_FR1	(((float *)&mips3.core.cpr[1][FTREG])[BYTE_XOR_LE(0)])
#define FSVALS_FR1	(((float *)&mips3.core.cpr[1][FSREG])[BYTE_XOR_LE(0)])
#define FDVALS_FR1	(((float *)&mips3.core.cpr[1][FDREG])[BYTE_XOR_LE(0)])
#define FSVALW_FR1	(((UINT32 *)&mips3.core.cpr[1][FSREG])[BYTE_XOR_LE(0)])
#define FDVALW_FR1	(((UINT32 *)&mips3.core.cpr[1][FDREG])[BYTE_XOR_LE(0)])

#define FRVALD_FR1	(*(double *)&mips3.core.cpr[1][FRREG])
#define FTVALD_FR1	(*(double *)&mips3.core.cpr[1][FTREG])
#define FSVALD_FR1	(*(double *)&mips3.core.cpr[1][FSREG])
#define FDVALD_FR1	(*(double *)&mips3.core.cpr[1][FDREG])
#define FSVALL_FR1	(*(UINT64 *)&mips3.core.cpr[1][FSREG])
#define FDVALL_FR1	(*(UINT64 *)&mips3.core.cpr[1][FDREG])

#define ADDPC(x)	mips3.nextpc = mips3.core.pc + ((x) << 2)
#define ADDPCL(x,l)	{ mips3.nextpc = mips3.core.pc + ((x) << 2); mips3.core.r[l] = (INT32)(mips3.core.pc + 4); }
#define ABSPC(x)	mips3.nextpc = (mips3.core.pc & 0xf0000000) | ((x) << 2)
#define ABSPCL(x,l)	{ mips3.nextpc = (mips3.core.pc & 0xf0000000) | ((x) << 2); mips3.core.r[l] = (INT32)(mips3.core.pc + 4); }
#define SETPC(x)	mips3.nextpc = (x)
#define SETPCL(x,l)	{ mips3.nextpc = (x); mips3.core.r[l] = (INT32)(mips3.core.pc + 4); }

#define HIVAL		(UINT32)mips3.core.r[REG_HI]
#define LOVAL		(UINT32)mips3.core.r[REG_LO]
#define HIVAL64		mips3.core.r[REG_HI]
#define LOVAL64		mips3.core.r[REG_LO]
#define SR			mips3.core.cpr[0][COP0_Status]
#define CAUSE		mips3.core.cpr[0][COP0_Cause]

#define GET_FCC(n)	(mips3.cf[1][n])
#define SET_FCC(n,v) (mips3.cf[1][n] = (v))

#define IS_FR0		(!(SR & SR_FR))
#define IS_FR1		(SR & SR_FR)



/***************************************************************************
    STRUCTURES & TYPEDEFS
***************************************************************************/

/* MIPS3 Registers */
struct mips3_regs
{
	/* core state */
	mips3_state	core;

	/* internal stuff */
	UINT32		ppc;
	UINT32		nextpc;
	UINT32		pcbase;
	UINT8		cf[4][8];
	int			op;
	int			interrupt_cycles;
	UINT32		ll_value;
	UINT64		lld_value;
	UINT32		badcop_value;
	const vtlb_entry *tlb_table;

	/* endian-dependent load/store */
	void		(*lwl)(UINT32 op);
	void		(*lwr)(UINT32 op);
	void		(*swl)(UINT32 op);
	void		(*swr)(UINT32 op);
	void		(*ldl)(UINT32 op);
	void		(*ldr)(UINT32 op);
	void		(*sdl)(UINT32 op);
	void		(*sdr)(UINT32 op);
};



/***************************************************************************
    FUNCTION PROTOTYPES
***************************************************************************/

static void lwl_be(UINT32 op);
static void lwr_be(UINT32 op);
static void swl_be(UINT32 op);
static void swr_be(UINT32 op);

static void lwl_le(UINT32 op);
static void lwr_le(UINT32 op);
static void swl_le(UINT32 op);
static void swr_le(UINT32 op);

static void ldl_be(UINT32 op);
static void ldr_be(UINT32 op);
static void sdl_be(UINT32 op);
static void sdr_be(UINT32 op);

static void ldl_le(UINT32 op);
static void ldr_le(UINT32 op);
static void sdl_le(UINT32 op);
static void sdr_le(UINT32 op);

static const UINT8 fcc_shift[8] = { 23, 25, 26, 27, 28, 29, 30, 31 };



/***************************************************************************
    PRIVATE GLOBAL VARIABLES
***************************************************************************/

static mips3_regs mips3;



/***************************************************************************
    MEMORY ACCESSORS
***************************************************************************/

#define ROPCODE(pc)		mips3.core.direct->read_decrypted_dword(pc)


/***************************************************************************
    DRC COMPATIBILITY
***************************************************************************/

void mips3drc_set_options(device_t *device, UINT32 options)
{
}

void mips3drc_add_fastram(device_t *device, offs_t start, offs_t end, UINT8 readonly, void *base)
{
}

void mips3drc_add_hotspot(device_t *device, offs_t pc, UINT32 opcode, UINT32 cycles)
{
}

/***************************************************************************
    EXECEPTION HANDLING
***************************************************************************/

INLINE void generate_exception(int exception, int backup)
{
	UINT32 offset = 0x180;
/*
    useful for catching exceptions:

    if (exception != 0)
    {
        fprintf(stderr, "Exception: PC=%08X, PPC=%08X\n", mips3.core.pc, mips3.ppc);
        debugger_break(Machine);
    }
*/

	/* back up if necessary */
	if (backup)
		mips3.core.pc = mips3.ppc;

	/* translate our fake fill exceptions into real exceptions */
	if (exception == EXCEPTION_TLBLOAD_FILL || exception == EXCEPTION_TLBSTORE_FILL)
	{
		offset = 0;
		exception = (exception - EXCEPTION_TLBLOAD_FILL) + EXCEPTION_TLBLOAD;
	}

	/* set the exception PC */
	mips3.core.cpr[0][COP0_EPC] = mips3.core.pc;

	/* put the cause in the low 8 bits and clear the branch delay flag */
	CAUSE = (CAUSE & ~0x800000ff) | (exception << 2);

	/* set the appropriate bits for coprocessor exceptions */
	if(exception == EXCEPTION_BADCOP)
	{
		CAUSE |= mips3.badcop_value << 28;
	}

	/* if we were in a branch delay slot, adjust */
	if (mips3.nextpc != ~0)
	{
		mips3.nextpc = ~0;
		mips3.core.cpr[0][COP0_EPC] -= 4;
		CAUSE |= 0x80000000;
	}

	/* set the exception level */
	SR |= SR_EXL;

	/* based on the BEV bit, we either go to ROM or RAM */
	mips3.core.pc = (SR & SR_BEV) ? 0xbfc00200 : 0x80000000;

	/* most exceptions go to offset 0x180, except for TLB stuff */
	if (exception >= EXCEPTION_TLBMOD && exception <= EXCEPTION_TLBSTORE)
	{
		mame_printf_debug("TLB miss @ %08X\n", (UINT32)mips3.core.cpr[0][COP0_BadVAddr]);
	}
	mips3.core.pc += offset;

/*
    useful for tracking interrupts

    if ((CAUSE & 0x7f) == 0)
        logerror("Took interrupt -- Cause = %08X, PC =  %08X\n", (UINT32)CAUSE, mips3.core.pc);
*/
}


static void generate_tlb_exception(int exception, offs_t address)
{
	mips3.core.cpr[0][COP0_BadVAddr] = address;
	if(exception == EXCEPTION_TLBLOAD || exception == EXCEPTION_TLBSTORE || exception == EXCEPTION_TLBLOAD_FILL || exception == EXCEPTION_TLBSTORE_FILL)
	{
		mips3.core.cpr[0][COP0_Context] = (mips3.core.cpr[0][COP0_Context] & 0xff800000) | ((address >> 9) & 0x007ffff0);
		mips3.core.cpr[0][COP0_EntryHi] = (address & 0xffffe000) | (mips3.core.cpr[0][COP0_EntryHi] & 0xff);
	}
	generate_exception(exception, 1);
}


INLINE void invalid_instruction(UINT32 op)
{
	generate_exception(EXCEPTION_INVALIDOP, 1);
}



/***************************************************************************
    IRQ HANDLING
***************************************************************************/

static void check_irqs(void)
{
	if ((CAUSE & SR & 0xfc00) && (SR & SR_IE) && !(SR & SR_EXL) && !(SR & SR_ERL))
		generate_exception(EXCEPTION_INTERRUPT, 0);
}



/***************************************************************************
    CORE CALLBACKS
***************************************************************************/

static CPU_RESET( mips3 )
{
	/* common reset */
	mips3com_reset(&mips3.core);
	mips3.nextpc = ~0;
	memset(mips3.cf, 0, sizeof(mips3.cf));

	/* set up the endianness */
	if (mips3.core.bigendian)
	{
		mips3.lwl = lwl_be;
		mips3.lwr = lwr_be;
		mips3.swl = swl_be;
		mips3.swr = swr_be;
		mips3.ldl = ldl_be;
		mips3.ldr = ldr_be;
		mips3.sdl = sdl_be;
		mips3.sdr = sdr_be;
	}
	else
	{
		mips3.lwl = lwl_le;
		mips3.lwr = lwr_le;
		mips3.swl = swl_le;
		mips3.swr = swr_le;
		mips3.ldl = ldl_le;
		mips3.ldr = ldr_le;
		mips3.sdl = sdl_le;
		mips3.sdr = sdr_le;
	}
}


static CPU_TRANSLATE( mips3 )
{
	/* common translate */
	return mips3com_translate_address(&mips3.core, space, intention, address);
}


CPU_DISASSEMBLE( mips3 )
{
	/* common disassemble */
	return mips3com_dasm(&mips3.core, buffer, pc, oprom, opram);
}



/***************************************************************************
    TLB HANDLING
***************************************************************************/

INLINE int RBYTE(offs_t address, UINT32 *result)
{
	UINT32 tlbval = mips3.tlb_table[address >> 12];
	if (tlbval & VTLB_READ_ALLOWED)
	{
		*result = (*mips3.core.memory.read_byte)(mips3.core.program, (tlbval & ~0xfff) | (address & 0xfff));
	}
	else
	{
		if(tlbval & VTLB_FLAG_FIXED)
		{
			generate_tlb_exception(EXCEPTION_TLBLOAD, address);
		}
		else
		{
			generate_tlb_exception(EXCEPTION_TLBLOAD_FILL, address);
		}
		*result = 0;
		return 0;
	}
	return 1;
}


INLINE int RHALF(offs_t address, UINT32 *result)
{
	UINT32 tlbval = mips3.tlb_table[address >> 12];
	if (tlbval & VTLB_READ_ALLOWED)
	{
		*result = (*mips3.core.memory.read_word)(mips3.core.program, (tlbval & ~0xfff) | (address & 0xfff));
	}
	else
	{
		if(tlbval & VTLB_FLAG_FIXED)
		{
			generate_tlb_exception(EXCEPTION_TLBLOAD, address);
		}
		else
		{
			generate_tlb_exception(EXCEPTION_TLBLOAD_FILL, address);
		}
		*result = 0;
		return 0;
	}
	return 1;
}


INLINE int RWORD(offs_t address, UINT32 *result)
{
	UINT32 tlbval = mips3.tlb_table[address >> 12];
	if (tlbval & VTLB_READ_ALLOWED)
	{
		*result = (*mips3.core.memory.read_dword)(mips3.core.program, (tlbval & ~0xfff) | (address & 0xfff));
	}
	else
	{
		if(tlbval & VTLB_FLAG_FIXED)
		{
			generate_tlb_exception(EXCEPTION_TLBLOAD, address);
		}
		else
		{
			generate_tlb_exception(EXCEPTION_TLBLOAD_FILL, address);
		}
		*result = 0;
		return 0;
	}
	return 1;
}


INLINE int RWORD_MASKED(offs_t address, UINT32 *result, UINT32 mem_mask)
{
	UINT32 tlbval = mips3.tlb_table[address >> 12];
	if (tlbval & VTLB_READ_ALLOWED)
	{
		*result = (*mips3.core.memory.read_dword_masked)(mips3.core.program, (tlbval & ~0xfff) | (address & 0xfff), mem_mask);
	}
	else
	{
		if(tlbval & VTLB_FLAG_FIXED)
		{
			generate_tlb_exception(EXCEPTION_TLBLOAD, address);
		}
		else
		{
			generate_tlb_exception(EXCEPTION_TLBLOAD_FILL, address);
		}
		*result = 0;
		return 0;
	}
	return 1;
}


INLINE int RDOUBLE(offs_t address, UINT64 *result)
{
	UINT32 tlbval = mips3.tlb_table[address >> 12];
	if (tlbval & VTLB_READ_ALLOWED)
	{
		*result = (*mips3.core.memory.read_qword)(mips3.core.program, (tlbval & ~0xfff) | (address & 0xfff));
	}
	else
	{
		if(tlbval & VTLB_FLAG_FIXED)
		{
			generate_tlb_exception(EXCEPTION_TLBLOAD, address);
		}
		else
		{
			generate_tlb_exception(EXCEPTION_TLBLOAD_FILL, address);
		}
		*result = 0;
		return 0;
	}
	return 1;
}


INLINE int RDOUBLE_MASKED(offs_t address, UINT64 *result, UINT64 mem_mask)
{
	UINT32 tlbval = mips3.tlb_table[address >> 12];
	if (tlbval & VTLB_READ_ALLOWED)
	{
		*result = (*mips3.core.memory.read_qword_masked)(mips3.core.program, (tlbval & ~0xfff) | (address & 0xfff), mem_mask);
	}
	else
	{
		if(tlbval & VTLB_FLAG_FIXED)
		{
			generate_tlb_exception(EXCEPTION_TLBLOAD, address);
		}
		else
		{
			generate_tlb_exception(EXCEPTION_TLBLOAD_FILL, address);
		}
		*result = 0;
		return 0;
	}
	return 1;
}


INLINE void WBYTE(offs_t address, UINT8 data)
{
	UINT32 tlbval = mips3.tlb_table[address >> 12];
	if (tlbval & VTLB_WRITE_ALLOWED)
	{
		(*mips3.core.memory.write_byte)(mips3.core.program, (tlbval & ~0xfff) | (address & 0xfff), data);
	}
	else
	{
		if(tlbval & VTLB_READ_ALLOWED)
		{
			generate_tlb_exception(EXCEPTION_TLBMOD, address);
		}
		else if(tlbval & VTLB_FLAG_FIXED)
		{
			generate_tlb_exception(EXCEPTION_TLBSTORE, address);
		}
		else
		{
			generate_tlb_exception(EXCEPTION_TLBSTORE_FILL, address);
		}
	}
}


INLINE void WHALF(offs_t address, UINT16 data)
{
	UINT32 tlbval = mips3.tlb_table[address >> 12];
	if (tlbval & VTLB_WRITE_ALLOWED)
	{
		(*mips3.core.memory.write_word)(mips3.core.program, (tlbval & ~0xfff) | (address & 0xfff), data);
	}
	else
	{
		if(tlbval & VTLB_READ_ALLOWED)
		{
			generate_tlb_exception(EXCEPTION_TLBMOD, address);
		}
		else if(tlbval & VTLB_FLAG_FIXED)
		{
			generate_tlb_exception(EXCEPTION_TLBSTORE, address);
		}
		else
		{
			generate_tlb_exception(EXCEPTION_TLBSTORE_FILL, address);
		}
	}
}


INLINE void WWORD(offs_t address, UINT32 data)
{
	UINT32 tlbval = mips3.tlb_table[address >> 12];
	if (tlbval & VTLB_WRITE_ALLOWED)
	{
		(*mips3.core.memory.write_dword)(mips3.core.program, (tlbval & ~0xfff) | (address & 0xfff), data);
	}
	else
	{
		if(tlbval & VTLB_READ_ALLOWED)
		{
			generate_tlb_exception(EXCEPTION_TLBMOD, address);
		}
		else if(tlbval & VTLB_FLAG_FIXED)
		{
			generate_tlb_exception(EXCEPTION_TLBSTORE, address);
		}
		else
		{
			generate_tlb_exception(EXCEPTION_TLBSTORE_FILL, address);
		}
	}
}


INLINE void WWORD_MASKED(offs_t address, UINT32 data, UINT32 mem_mask)
{
	UINT32 tlbval = mips3.tlb_table[address >> 12];
	if (tlbval & VTLB_WRITE_ALLOWED)
	{
		(*mips3.core.memory.write_dword_masked)(mips3.core.program, (tlbval & ~0xfff) | (address & 0xfff), data, mem_mask);
	}
	else
	{
		if(tlbval & VTLB_READ_ALLOWED)
		{
			generate_tlb_exception(EXCEPTION_TLBMOD, address);
		}
		else if(tlbval & VTLB_FLAG_FIXED)
		{
			generate_tlb_exception(EXCEPTION_TLBSTORE, address);
		}
		else
		{
			generate_tlb_exception(EXCEPTION_TLBSTORE_FILL, address);
		}
	}
}


INLINE void WDOUBLE(offs_t address, UINT64 data)
{
	UINT32 tlbval = mips3.tlb_table[address >> 12];
	//printf("%08x: %08x\n", (UINT32)address, (UINT32)tlbval);
	if (tlbval & VTLB_WRITE_ALLOWED)
	{
		(*mips3.core.memory.write_qword)(mips3.core.program, (tlbval & ~0xfff)  | (address & 0xfff), data);
	}
	else
	{
		if(tlbval & VTLB_READ_ALLOWED)
		{
			generate_tlb_exception(EXCEPTION_TLBMOD, address);
		}
		else if(tlbval & VTLB_FLAG_FIXED)
		{
			generate_tlb_exception(EXCEPTION_TLBSTORE, address);
		}
		else
		{
			generate_tlb_exception(EXCEPTION_TLBSTORE_FILL, address);
		}
	}
}


INLINE void WDOUBLE_MASKED(offs_t address, UINT64 data, UINT64 mem_mask)
{
	UINT32 tlbval = mips3.tlb_table[address >> 12];
	if (tlbval & VTLB_WRITE_ALLOWED)
	{
		(*mips3.core.memory.write_qword_masked)(mips3.core.program, (tlbval & ~0xfff)  | (address & 0xfff), data, mem_mask);
	}
	else
	{
		if(tlbval & VTLB_READ_ALLOWED)
		{
			generate_tlb_exception(EXCEPTION_TLBMOD, address);
		}
		else if(tlbval & VTLB_FLAG_FIXED)
		{
			generate_tlb_exception(EXCEPTION_TLBSTORE, address);
		}
		else
		{
			generate_tlb_exception(EXCEPTION_TLBSTORE_FILL, address);
		}
	}
}



/***************************************************************************
    COP0 (SYSTEM) EXECUTION HANDLING
***************************************************************************/

INLINE UINT64 get_cop0_reg(int idx)
{
	if (idx == COP0_Count)
	{
		/* it doesn't really take 250 cycles to read this register, but it helps speed */
		/* up loops that hammer on it */
		if (mips3.core.icount >= MIPS3_COUNT_READ_CYCLES)
			mips3.core.icount -= MIPS3_COUNT_READ_CYCLES;
		else
			mips3.core.icount = 0;
		return (UINT32)((mips3.core.device->total_cycles() - mips3.core.count_zero_time) / 2);
	}
	else if (idx == COP0_Cause)
	{
		/* it doesn't really take 250 cycles to read this register, but it helps speed */
		/* up loops that hammer on it */
		if (mips3.core.icount >= MIPS3_CAUSE_READ_CYCLES)
			mips3.core.icount -= MIPS3_CAUSE_READ_CYCLES;
		else
			mips3.core.icount = 0;
	}
	else if (idx == COP0_Random)
	{
		int wired = mips3.core.cpr[0][COP0_Wired] & 0x3f;
		int range = 48 - wired;
		if (range > 0)
			return ((mips3.core.device->total_cycles() - mips3.core.count_zero_time) % range + wired) & 0x3f;
		else
			return 47;
	}
	return mips3.core.cpr[0][idx];
}

INLINE void set_cop0_reg(int idx, UINT64 val)
{
	switch (idx)
	{
		case COP0_Cause:
			CAUSE = (CAUSE & 0xfc00) | (val & ~0xfc00);
			if (CAUSE & 0x300)
			{
				/* if we're in a delay slot, propogate the target PC before generating the exception */
				if (mips3.nextpc != ~0)
				{
					mips3.core.pc = mips3.nextpc;
					mips3.nextpc = ~0;
				}
				generate_exception(EXCEPTION_INTERRUPT, 0);
			}
			break;

		case COP0_Status:
		{
			/* update interrupts and cycle counting */
			UINT32 diff = mips3.core.cpr[0][idx] ^ val;
//          if (val & 0xe0)
//              fatalerror("System set 64-bit addressing mode, SR=%08X\n", val);
			mips3.core.cpr[0][idx] = val;
			if (diff & 0x8000)
				mips3com_update_cycle_counting(&mips3.core);
			check_irqs();
			break;
		}

		case COP0_Count:
			mips3.core.cpr[0][idx] = val;
			mips3.core.count_zero_time = mips3.core.device->total_cycles() - ((UINT64)(UINT32)val * 2);
			mips3com_update_cycle_counting(&mips3.core);
			break;

		case COP0_Compare:
			mips3.core.compare_armed = 1;
			CAUSE &= ~0x8000;
			mips3.core.cpr[0][idx] = val & 0xffffffff;
			mips3com_update_cycle_counting(&mips3.core);
			break;

		case COP0_PRId:
			break;

		case COP0_Config:
			mips3.core.cpr[0][idx] = (mips3.core.cpr[0][idx] & ~7) | (val & 7);
			break;

		case COP0_EntryHi:
			/* if the ASID changes, remap */
			if ((mips3.core.cpr[0][idx] ^ val) & 0xff)
			{
				mips3.core.cpr[0][idx] = val;
				mips3com_asid_changed(&mips3.core);
			}
			mips3.core.cpr[0][idx] = val;
			break;

		default:
			mips3.core.cpr[0][idx] = val;
			break;
	}
}

INLINE UINT64 get_cop0_creg(int idx)
{
	return mips3.core.ccr[0][idx];
}

INLINE void set_cop0_creg(int idx, UINT64 val)
{
	mips3.core.ccr[0][idx] = val;
}

INLINE void handle_cop0(UINT32 op)
{
	if ((SR & SR_KSU_MASK) != SR_KSU_KERNEL && !(SR & SR_COP0))
	{
		mips3.badcop_value = 0;
		generate_exception(EXCEPTION_BADCOP, 1);
	}

	switch (RSREG)
	{
		case 0x00:	/* MFCz */		if (RTREG) RTVAL64 = (INT32)get_cop0_reg(RDREG);		break;
		case 0x01:	/* DMFCz */		if (RTREG) RTVAL64 = get_cop0_reg(RDREG);				break;
		case 0x02:	/* CFCz */		if (RTREG) RTVAL64 = (INT32)get_cop0_creg(RDREG);		break;
		case 0x04:	/* MTCz */		set_cop0_reg(RDREG, RTVAL32);							break;
		case 0x05:	/* DMTCz */		set_cop0_reg(RDREG, RTVAL64);							break;
		case 0x06:	/* CTCz */		set_cop0_creg(RDREG, RTVAL32);							break;
		case 0x08:	/* BC */
			switch (RTREG)
			{
				case 0x00:	/* BCzF */	if (!mips3.cf[0]) ADDPC(SIMMVAL);				break;
				case 0x01:	/* BCzF */	if (mips3.cf[0]) ADDPC(SIMMVAL);				break;
				case 0x02:	/* BCzFL */	invalid_instruction(op);							break;
				case 0x03:	/* BCzTL */	invalid_instruction(op);							break;
				default:	invalid_instruction(op);										break;
			}
			break;
		case 0x10:
		case 0x11:
		case 0x12:
		case 0x13:
		case 0x14:
		case 0x15:
		case 0x16:
		case 0x17:
		case 0x18:
		case 0x19:
		case 0x1a:
		case 0x1b:
		case 0x1c:
		case 0x1d:
		case 0x1e:
		case 0x1f:	/* COP */
			switch (op & 0x01ffffff)
			{
				case 0x01:	/* TLBR */
					mips3com_tlbr(&mips3.core);
					break;

				case 0x02:	/* TLBWI */
					mips3com_tlbwi(&mips3.core);
					break;

				case 0x06:	/* TLBWR */
					mips3com_tlbwr(&mips3.core);
					break;

				case 0x08:	/* TLBP */
					mips3com_tlbp(&mips3.core);
					break;

				case 0x10:	/* RFE */	invalid_instruction(op);							break;
				case 0x18:	/* ERET */	logerror("ERET\n"); mips3.core.pc = mips3.core.cpr[0][COP0_EPC]; SR &= ~SR_EXL; check_irqs(); mips3.lld_value ^= 0xffffffff; mips3.ll_value ^= 0xffffffff;	break;
				case 0x20:	/* WAIT */														break;
				default:	invalid_instruction(op);										break;
			}
			break;
		default:	invalid_instruction(op);												break;
	}
}



/***************************************************************************
    COP1 (FPU) EXECUTION HANDLING
***************************************************************************/

INLINE UINT32 get_cop1_reg32(int idx)
{
	if (IS_FR0)
		return ((UINT32 *)&mips3.core.cpr[1][0])[idx];
	else
		return mips3.core.cpr[1][idx];
}

INLINE UINT64 get_cop1_reg64(int idx)
{
	if (IS_FR0)
		return ((UINT64 *)&mips3.core.cpr[1][0])[idx/2];
	else
		return mips3.core.cpr[1][idx];
}

INLINE void set_cop1_reg32(int idx, UINT32 val)
{
	if (IS_FR0)
		((UINT32 *)&mips3.core.cpr[1][0])[idx] = val;
	else
		mips3.core.cpr[1][idx] = val;
}

INLINE void set_cop1_reg64(int idx, UINT64 val)
{
	if (IS_FR0)
		((UINT64 *)&mips3.core.cpr[1][0])[idx/2] = val;
	else
		mips3.core.cpr[1][idx] = val;
}

INLINE UINT64 get_cop1_creg(int idx)
{
	if (idx == 31)
	{
		UINT32 result = mips3.core.ccr[1][31] & ~0xfe800000;
		int i;

		for (i = 0; i < 8; i++)
			if (mips3.cf[1][i])
				result |= 1 << fcc_shift[i];
		return result;
	}
	return mips3.core.ccr[1][idx];
}

INLINE void set_cop1_creg(int idx, UINT64 val)
{
	mips3.core.ccr[1][idx] = val;
	if (idx == 31)
	{
		int i;

		for (i = 0; i < 8; i++)
			mips3.cf[1][i] = (val >> fcc_shift[i]) & 1;
	}
}

INLINE void handle_cop1_fr0(UINT32 op)
{
	double dtemp;

	/* note: additional condition codes available on R5000 only */

	if (!(SR & SR_COP1))
	{
		mips3.badcop_value = 1;
		generate_exception(EXCEPTION_BADCOP, 1);
	}

	switch (RSREG)
	{
		case 0x00:	/* MFCz */		if (RTREG) RTVAL64 = (INT32)get_cop1_reg32(RDREG);		break;
		case 0x01:	/* DMFCz */		if (RTREG) RTVAL64 = get_cop1_reg64(RDREG);				break;
		case 0x02:	/* CFCz */		if (RTREG) RTVAL64 = (INT32)get_cop1_creg(RDREG);		break;
		case 0x04:	/* MTCz */		set_cop1_reg32(RDREG, RTVAL32);							break;
		case 0x05:	/* DMTCz */		set_cop1_reg64(RDREG, RTVAL64);							break;
		case 0x06:	/* CTCz */		set_cop1_creg(RDREG, RTVAL32);							break;
		case 0x08:	/* BC */
			switch ((op >> 16) & 3)
			{
				case 0x00:	/* BCzF */	if (!GET_FCC((op >> 18) & 7)) ADDPC(SIMMVAL);	break;
				case 0x01:	/* BCzT */	if (GET_FCC((op >> 18) & 7)) ADDPC(SIMMVAL);	break;
				case 0x02:	/* BCzFL */	if (!GET_FCC((op >> 18) & 7)) ADDPC(SIMMVAL); else mips3.core.pc += 4;	break;
				case 0x03:	/* BCzTL */	if (GET_FCC((op >> 18) & 7)) ADDPC(SIMMVAL); else mips3.core.pc += 4;	break;
			}
			break;
		default:
			switch (op & 0x3f)
			{
				case 0x00:
					if (IS_SINGLE(op))	/* ADD.S */
						FDVALS_FR0 = FSVALS_FR0 + FTVALS_FR0;
					else				/* ADD.D */
						FDVALD_FR0 = FSVALD_FR0 + FTVALD_FR0;
					break;

				case 0x01:
					if (IS_SINGLE(op))	/* SUB.S */
						FDVALS_FR0 = FSVALS_FR0 - FTVALS_FR0;
					else				/* SUB.D */
						FDVALD_FR0 = FSVALD_FR0 - FTVALD_FR0;
					break;

				case 0x02:
					if (IS_SINGLE(op))	/* MUL.S */
						FDVALS_FR0 = FSVALS_FR0 * FTVALS_FR0;
					else				/* MUL.D */
						FDVALD_FR0 = FSVALD_FR0 * FTVALD_FR0;
					break;

				case 0x03:
					if (IS_SINGLE(op))	/* DIV.S */
						FDVALS_FR0 = FSVALS_FR0 / FTVALS_FR0;
					else				/* DIV.D */
						FDVALD_FR0 = FSVALD_FR0 / FTVALD_FR0;
					break;

				case 0x04:
					if (IS_SINGLE(op))	/* SQRT.S */
						FDVALS_FR0 = sqrt(FSVALS_FR0);
					else				/* SQRT.D */
						FDVALD_FR0 = sqrt(FSVALD_FR0);
					break;

				case 0x05:
					if (IS_SINGLE(op))	/* ABS.S */
						FDVALS_FR0 = fabs(FSVALS_FR0);
					else				/* ABS.D */
						FDVALD_FR0 = fabs(FSVALD_FR0);
					break;

				case 0x06:
					if (IS_SINGLE(op))	/* MOV.S */
						FDVALS_FR0 = FSVALS_FR0;
					else				/* MOV.D */
						FDVALD_FR0 = FSVALD_FR0;
					break;

				case 0x07:
					if (IS_SINGLE(op))	/* NEG.S */
						FDVALS_FR0 = -FSVALS_FR0;
					else				/* NEG.D */
						FDVALD_FR0 = -FSVALD_FR0;
					break;

				case 0x08:
					if (IS_SINGLE(op))	/* ROUND.L.S */
					{
						double temp = FSVALS_FR0;
						if (temp < 0)
							temp = ceil(temp - 0.5);
						else
							temp = floor(temp + 0.5);
						FDVALL_FR0 = (INT64)temp;
					}
					else				/* ROUND.L.D */
					{
						double temp = FSVALD_FR0;
						if (temp < 0)
							temp = ceil(temp - 0.5);
						else
							temp = floor(temp + 0.5);
						FDVALL_FR0 = (INT64)temp;
					}
					break;

				case 0x09:
					if (IS_SINGLE(op))	/* TRUNC.L.S */
					{
						double temp = FSVALS_FR0;
						if (temp < 0)
							temp = ceil(temp);
						else
							temp = floor(temp);
						FDVALL_FR0 = (INT64)temp;
					}
					else				/* TRUNC.L.D */
					{
						double temp = FSVALD_FR0;
						if (temp < 0)
							temp = ceil(temp);
						else
							temp = floor(temp);
						FDVALL_FR0 = (INT64)temp;
					}
					break;

				case 0x0a:
					if (IS_SINGLE(op))	/* CEIL.L.S */
						dtemp = ceil(FSVALS_FR0);
					else				/* CEIL.L.D */
						dtemp = ceil(FSVALD_FR0);
					FDVALL_FR0 = (INT64)dtemp;
					break;

				case 0x0b:
					if (IS_SINGLE(op))	/* FLOOR.L.S */
						dtemp = floor(FSVALS_FR0);
					else				/* FLOOR.L.D */
						dtemp = floor(FSVALD_FR0);
					FDVALL_FR0 = (INT64)dtemp;
					break;

				case 0x0c:
					if (IS_SINGLE(op))	/* ROUND.W.S */
					{
						dtemp = FSVALS_FR0;
						if (dtemp < 0)
							dtemp = ceil(dtemp - 0.5);
						else
							dtemp = floor(dtemp + 0.5);
						FDVALW_FR0 = (INT32)dtemp;
					}
					else				/* ROUND.W.D */
					{
						dtemp = FSVALD_FR0;
						if (dtemp < 0)
							dtemp = ceil(dtemp - 0.5);
						else
							dtemp = floor(dtemp + 0.5);
						FDVALW_FR0 = (INT32)dtemp;
					}
					break;

				case 0x0d:
					if (IS_SINGLE(op))	/* TRUNC.W.S */
					{
						dtemp = FSVALS_FR0;
						if (dtemp < 0)
							dtemp = ceil(dtemp);
						else
							dtemp = floor(dtemp);
						FDVALW_FR0 = (INT32)dtemp;
					}
					else				/* TRUNC.W.D */
					{
						dtemp = FSVALD_FR0;
						if (dtemp < 0)
							dtemp = ceil(dtemp);
						else
							dtemp = floor(dtemp);
						FDVALW_FR0 = (INT32)dtemp;
					}
					break;

				case 0x0e:
					if (IS_SINGLE(op))	/* CEIL.W.S */
						dtemp = ceil(FSVALS_FR0);
					else				/* CEIL.W.D */
						dtemp = ceil(FSVALD_FR0);
					FDVALW_FR0 = (INT32)dtemp;
					break;

				case 0x0f:
					if (IS_SINGLE(op))	/* FLOOR.W.S */
						dtemp = floor(FSVALS_FR0);
					else				/* FLOOR.W.D */
						dtemp = floor(FSVALD_FR0);
					FDVALW_FR0 = (INT32)dtemp;
					break;

				case 0x11:	/* R5000 */
					if (GET_FCC((op >> 18) & 7) == ((op >> 16) & 1))
					{
						if (IS_SINGLE(op))	/* MOVT/F.S */
							FDVALS_FR0 = FSVALS_FR0;
						else				/* MOVT/F.D */
							FDVALD_FR0 = FSVALD_FR0;
					}
					break;

				case 0x12:	/* R5000 */
					if (RTVAL64 == 0)
					{
						if (IS_SINGLE(op))	/* MOVZ.S */
							FDVALS_FR0 = FSVALS_FR0;
						else				/* MOVZ.D */
							FDVALD_FR0 = FSVALD_FR0;
					}
					break;

				case 0x13:	/* R5000 */
					if (RTVAL64 != 0)
					{
						if (IS_SINGLE(op))	/* MOVN.S */
							FDVALS_FR0 = FSVALS_FR0;
						else				/* MOVN.D */
							FDVALD_FR0 = FSVALD_FR0;
					}
					break;

				case 0x15:	/* R5000 */
					if (IS_SINGLE(op))	/* RECIP.S */
						FDVALS_FR0 = 1.0f / FSVALS_FR0;
					else				/* RECIP.D */
						FDVALD_FR0 = 1.0 / FSVALD_FR0;
					break;

				case 0x16:	/* R5000 */
					if (IS_SINGLE(op))	/* RSQRT.S */
						FDVALS_FR0 = 1.0f / sqrt(FSVALS_FR0);
					else				/* RSQRT.D */
						FDVALD_FR0 = 1.0 / sqrt(FSVALD_FR0);
					break;

				case 0x20:
					if (IS_INTEGRAL(op))
					{
						if (IS_SINGLE(op))	/* CVT.S.W */
							FDVALS_FR0 = (INT32)FSVALW_FR0;
						else				/* CVT.S.L */
							FDVALS_FR0 = (INT64)FSVALL_FR0;
					}
					else					/* CVT.S.D */
						FDVALS_FR0 = FSVALD_FR0;
					break;

				case 0x21:
					if (IS_INTEGRAL(op))
					{
						if (IS_SINGLE(op))	/* CVT.D.W */
							FDVALD_FR0 = (INT32)FSVALW_FR0;
						else				/* CVT.D.L */
							FDVALD_FR0 = (INT64)FSVALL_FR0;
					}
					else					/* CVT.D.S */
						FDVALD_FR0 = FSVALS_FR0;
					break;

				case 0x24:
					if (IS_SINGLE(op))	/* CVT.W.S */
						FDVALW_FR0 = (INT32)FSVALS_FR0;
					else
						FDVALW_FR0 = (INT32)FSVALD_FR0;
					break;

				case 0x25:
					if (IS_SINGLE(op))	/* CVT.L.S */
						FDVALL_FR0 = (INT64)FSVALS_FR0;
					else				/* CVT.L.D */
						FDVALL_FR0 = (INT64)FSVALD_FR0;
					break;

				case 0x30:
				case 0x38:
					if (IS_SINGLE(op))	/* C.F.S */
						SET_FCC((op >> 8) & 7, 0);
					else				/* C.F.D */
						SET_FCC((op >> 8) & 7, 0);
					break;

				case 0x31:
				case 0x39:
					if (IS_SINGLE(op))	/* C.UN.S */
						SET_FCC((op >> 8) & 7, 0);
					else				/* C.UN.D */
						SET_FCC((op >> 8) & 7, 0);
					break;

				case 0x32:
				case 0x3a:
					if (IS_SINGLE(op))	/* C.EQ.S */
						SET_FCC((op >> 8) & 7, (FSVALS_FR0 == FTVALS_FR0));
					else				/* C.EQ.D */
						SET_FCC((op >> 8) & 7, (FSVALD_FR0 == FTVALD_FR0));
					break;

				case 0x33:
				case 0x3b:
					if (IS_SINGLE(op))	/* C.UEQ.S */
						SET_FCC((op >> 8) & 7, (FSVALS_FR0 == FTVALS_FR0));
					else				/* C.UEQ.D */
						SET_FCC((op >> 8) & 7, (FSVALD_FR0 == FTVALD_FR0));
					break;

				case 0x34:
				case 0x3c:
					if (IS_SINGLE(op))	/* C.OLT.S */
						SET_FCC((op >> 8) & 7, (FSVALS_FR0 < FTVALS_FR0));
					else				/* C.OLT.D */
						SET_FCC((op >> 8) & 7, (FSVALD_FR0 < FTVALD_FR0));
					break;

				case 0x35:
				case 0x3d:
					if (IS_SINGLE(op))	/* C.ULT.S */
						SET_FCC((op >> 8) & 7, (FSVALS_FR0 < FTVALS_FR0));
					else				/* C.ULT.D */
						SET_FCC((op >> 8) & 7, (FSVALD_FR0 < FTVALD_FR0));
					break;

				case 0x36:
				case 0x3e:
					if (IS_SINGLE(op))	/* C.OLE.S */
						SET_FCC((op >> 8) & 7, (FSVALS_FR0 <= FTVALS_FR0));
					else				/* C.OLE.D */
						SET_FCC((op >> 8) & 7, (FSVALD_FR0 <= FTVALD_FR0));
					break;

				case 0x37:
				case 0x3f:
					if (IS_SINGLE(op))	/* C.ULE.S */
						SET_FCC((op >> 8) & 7, (FSVALS_FR0 <= FTVALS_FR0));
					else				/* C.ULE.D */
						SET_FCC((op >> 8) & 7, (FSVALD_FR0 <= FTVALD_FR0));
					break;

				default:
					fprintf(stderr, "cop1 %X\n", op);
					break;
			}
			break;
	}
}


INLINE void handle_cop1_fr1(UINT32 op)
{
	double dtemp;

	/* note: additional condition codes available on R5000 only */

	if (!(SR & SR_COP1))
	{
		mips3.badcop_value = 1;
		generate_exception(EXCEPTION_BADCOP, 1);
	}

	switch (RSREG)
	{
		case 0x00:	/* MFCz */		if (RTREG) RTVAL64 = (INT32)get_cop1_reg32(RDREG);		break;
		case 0x01:	/* DMFCz */		if (RTREG) RTVAL64 = get_cop1_reg64(RDREG);				break;
		case 0x02:	/* CFCz */		if (RTREG) RTVAL64 = (INT32)get_cop1_creg(RDREG);		break;
		case 0x04:	/* MTCz */		set_cop1_reg32(RDREG, RTVAL32);							break;
		case 0x05:	/* DMTCz */		set_cop1_reg64(RDREG, RTVAL64);							break;
		case 0x06:	/* CTCz */		set_cop1_creg(RDREG, RTVAL32);							break;
		case 0x08:	/* BC */
			switch ((op >> 16) & 3)
			{
				case 0x00:	/* BCzF */	if (!GET_FCC((op >> 18) & 7)) ADDPC(SIMMVAL);	break;
				case 0x01:	/* BCzT */	if (GET_FCC((op >> 18) & 7)) ADDPC(SIMMVAL);	break;
				case 0x02:	/* BCzFL */	if (!GET_FCC((op >> 18) & 7)) ADDPC(SIMMVAL); else mips3.core.pc += 4;	break;
				case 0x03:	/* BCzTL */	if (GET_FCC((op >> 18) & 7)) ADDPC(SIMMVAL); else mips3.core.pc += 4;	break;
			}
			break;
		default:
			switch (op & 0x3f)
			{
				case 0x00:
					if (IS_SINGLE(op))	/* ADD.S */
						FDVALS_FR1 = FSVALS_FR1 + FTVALS_FR1;
					else				/* ADD.D */
						FDVALD_FR1 = FSVALD_FR1 + FTVALD_FR1;
					break;

				case 0x01:
					if (IS_SINGLE(op))	/* SUB.S */
						FDVALS_FR1 = FSVALS_FR1 - FTVALS_FR1;
					else				/* SUB.D */
						FDVALD_FR1 = FSVALD_FR1 - FTVALD_FR1;
					break;

				case 0x02:
					if (IS_SINGLE(op))	/* MUL.S */
						FDVALS_FR1 = FSVALS_FR1 * FTVALS_FR1;
					else				/* MUL.D */
						FDVALD_FR1 = FSVALD_FR1 * FTVALD_FR1;
					break;

				case 0x03:
					if (IS_SINGLE(op))	/* DIV.S */
						FDVALS_FR1 = FSVALS_FR1 / FTVALS_FR1;
					else				/* DIV.D */
						FDVALD_FR1 = FSVALD_FR1 / FTVALD_FR1;
					break;

				case 0x04:
					if (IS_SINGLE(op))	/* SQRT.S */
						FDVALS_FR1 = sqrt(FSVALS_FR1);
					else				/* SQRT.D */
						FDVALD_FR1 = sqrt(FSVALD_FR1);
					break;

				case 0x05:
					if (IS_SINGLE(op))	/* ABS.S */
						FDVALS_FR1 = fabs(FSVALS_FR1);
					else				/* ABS.D */
						FDVALD_FR1 = fabs(FSVALD_FR1);
					break;

				case 0x06:
					if (IS_SINGLE(op))	/* MOV.S */
						FDVALS_FR1 = FSVALS_FR1;
					else				/* MOV.D */
						FDVALD_FR1 = FSVALD_FR1;
					break;

				case 0x07:
					if (IS_SINGLE(op))	/* NEG.S */
						FDVALS_FR1 = -FSVALS_FR1;
					else				/* NEG.D */
						FDVALD_FR1 = -FSVALD_FR1;
					break;

				case 0x08:
					if (IS_SINGLE(op))	/* ROUND.L.S */
					{
						double temp = FSVALS_FR1;
						if (temp < 0)
							temp = ceil(temp - 0.5);
						else
							temp = floor(temp + 0.5);
						FDVALL_FR1 = (INT64)temp;
					}
					else				/* ROUND.L.D */
					{
						double temp = FSVALD_FR1;
						if (temp < 0)
							temp = ceil(temp - 0.5);
						else
							temp = floor(temp + 0.5);
						FDVALL_FR1 = (INT64)temp;
					}
					break;

				case 0x09:
					if (IS_SINGLE(op))	/* TRUNC.L.S */
					{
						double temp = FSVALS_FR1;
						if (temp < 0)
							temp = ceil(temp);
						else
							temp = floor(temp);
						FDVALL_FR1 = (INT64)temp;
					}
					else				/* TRUNC.L.D */
					{
						double temp = FSVALD_FR1;
						if (temp < 0)
							temp = ceil(temp);
						else
							temp = floor(temp);
						FDVALL_FR1 = (INT64)temp;
					}
					break;

				case 0x0a:
					if (IS_SINGLE(op))	/* CEIL.L.S */
						dtemp = ceil(FSVALS_FR1);
					else				/* CEIL.L.D */
						dtemp = ceil(FSVALD_FR1);
					FDVALL_FR1 = (INT64)dtemp;
					break;

				case 0x0b:
					if (IS_SINGLE(op))	/* FLOOR.L.S */
						dtemp = floor(FSVALS_FR1);
					else				/* FLOOR.L.D */
						dtemp = floor(FSVALD_FR1);
					FDVALL_FR1 = (INT64)dtemp;
					break;

				case 0x0c:
					if (IS_SINGLE(op))	/* ROUND.W.S */
					{
						dtemp = FSVALS_FR1;
						if (dtemp < 0)
							dtemp = ceil(dtemp - 0.5);
						else
							dtemp = floor(dtemp + 0.5);
						FDVALW_FR1 = (INT32)dtemp;
					}
					else				/* ROUND.W.D */
					{
						dtemp = FSVALD_FR1;
						if (dtemp < 0)
							dtemp = ceil(dtemp - 0.5);
						else
							dtemp = floor(dtemp + 0.5);
						FDVALW_FR1 = (INT32)dtemp;
					}
					break;

				case 0x0d:
					if (IS_SINGLE(op))	/* TRUNC.W.S */
					{
						dtemp = FSVALS_FR1;
						if (dtemp < 0)
							dtemp = ceil(dtemp);
						else
							dtemp = floor(dtemp);
						FDVALW_FR1 = (INT32)dtemp;
					}
					else				/* TRUNC.W.D */
					{
						dtemp = FSVALD_FR1;
						if (dtemp < 0)
							dtemp = ceil(dtemp);
						else
							dtemp = floor(dtemp);
						FDVALW_FR1 = (INT32)dtemp;
					}
					break;

				case 0x0e:
					if (IS_SINGLE(op))	/* CEIL.W.S */
						dtemp = ceil(FSVALS_FR1);
					else				/* CEIL.W.D */
						dtemp = ceil(FSVALD_FR1);
					FDVALW_FR1 = (INT32)dtemp;
					break;

				case 0x0f:
					if (IS_SINGLE(op))	/* FLOOR.W.S */
						dtemp = floor(FSVALS_FR1);
					else				/* FLOOR.W.D */
						dtemp = floor(FSVALD_FR1);
					FDVALW_FR1 = (INT32)dtemp;
					break;

				case 0x11:	/* R5000 */
					if (GET_FCC((op >> 18) & 7) == ((op >> 16) & 1))
					{
						if (IS_SINGLE(op))	/* MOVT/F.S */
							FDVALS_FR1 = FSVALS_FR1;
						else				/* MOVT/F.D */
							FDVALD_FR1 = FSVALD_FR1;
					}
					break;

				case 0x12:	/* R5000 */
					if (RTVAL64 == 0)
					{
						if (IS_SINGLE(op))	/* MOVZ.S */
							FDVALS_FR1 = FSVALS_FR1;
						else				/* MOVZ.D */
							FDVALD_FR1 = FSVALD_FR1;
					}
					break;

				case 0x13:	/* R5000 */
					if (RTVAL64 != 0)
					{
						if (IS_SINGLE(op))	/* MOVN.S */
							FDVALS_FR1 = FSVALS_FR1;
						else				/* MOVN.D */
							FDVALD_FR1 = FSVALD_FR1;
					}
					break;

				case 0x15:	/* R5000 */
					if (IS_SINGLE(op))	/* RECIP.S */
						FDVALS_FR1 = 1.0f / FSVALS_FR1;
					else				/* RECIP.D */
						FDVALD_FR1 = 1.0 / FSVALD_FR1;
					break;

				case 0x16:	/* R5000 */
					if (IS_SINGLE(op))	/* RSQRT.S */
						FDVALS_FR1 = 1.0f / sqrt(FSVALS_FR1);
					else				/* RSQRT.D */
						FDVALD_FR1 = 1.0 / sqrt(FSVALD_FR1);
					break;

				case 0x20:
					if (IS_INTEGRAL(op))
					{
						if (IS_SINGLE(op))	/* CVT.S.W */
							FDVALS_FR1 = (INT32)FSVALW_FR1;
						else				/* CVT.S.L */
							FDVALS_FR1 = (INT64)FSVALL_FR1;
					}
					else					/* CVT.S.D */
						FDVALS_FR1 = FSVALD_FR1;
					break;

				case 0x21:
					if (IS_INTEGRAL(op))
					{
						if (IS_SINGLE(op))	/* CVT.D.W */
							FDVALD_FR1 = (INT32)FSVALW_FR1;
						else				/* CVT.D.L */
							FDVALD_FR1 = (INT64)FSVALL_FR1;
					}
					else					/* CVT.D.S */
						FDVALD_FR1 = FSVALS_FR1;
					break;

				case 0x24:
					if (IS_SINGLE(op))	/* CVT.W.S */
						FDVALW_FR1 = (INT32)FSVALS_FR1;
					else
						FDVALW_FR1 = (INT32)FSVALD_FR1;
					break;

				case 0x25:
					if (IS_SINGLE(op))	/* CVT.L.S */
						FDVALL_FR1 = (INT64)FSVALS_FR1;
					else				/* CVT.L.D */
						FDVALL_FR1 = (INT64)FSVALD_FR1;
					break;

				case 0x30:
				case 0x38:
					if (IS_SINGLE(op))	/* C.F.S */
						SET_FCC((op >> 8) & 7, 0);
					else				/* C.F.D */
						SET_FCC((op >> 8) & 7, 0);
					break;

				case 0x31:
				case 0x39:
					if (IS_SINGLE(op))	/* C.UN.S */
						SET_FCC((op >> 8) & 7, 0);
					else				/* C.UN.D */
						SET_FCC((op >> 8) & 7, 0);
					break;

				case 0x32:
				case 0x3a:
					if (IS_SINGLE(op))	/* C.EQ.S */
						SET_FCC((op >> 8) & 7, (FSVALS_FR1 == FTVALS_FR1));
					else				/* C.EQ.D */
						SET_FCC((op >> 8) & 7, (FSVALD_FR1 == FTVALD_FR1));
					break;

				case 0x33:
				case 0x3b:
					if (IS_SINGLE(op))	/* C.UEQ.S */
						SET_FCC((op >> 8) & 7, (FSVALS_FR1 == FTVALS_FR1));
					else				/* C.UEQ.D */
						SET_FCC((op >> 8) & 7, (FSVALD_FR1 == FTVALD_FR1));
					break;

				case 0x34:
				case 0x3c:
					if (IS_SINGLE(op))	/* C.OLT.S */
						SET_FCC((op >> 8) & 7, (FSVALS_FR1 < FTVALS_FR1));
					else				/* C.OLT.D */
						SET_FCC((op >> 8) & 7, (FSVALD_FR1 < FTVALD_FR1));
					break;

				case 0x35:
				case 0x3d:
					if (IS_SINGLE(op))	/* C.ULT.S */
						SET_FCC((op >> 8) & 7, (FSVALS_FR1 < FTVALS_FR1));
					else				/* C.ULT.D */
						SET_FCC((op >> 8) & 7, (FSVALD_FR1 < FTVALD_FR1));
					break;

				case 0x36:
				case 0x3e:
					if (IS_SINGLE(op))	/* C.OLE.S */
						SET_FCC((op >> 8) & 7, (FSVALS_FR1 <= FTVALS_FR1));
					else				/* C.OLE.D */
						SET_FCC((op >> 8) & 7, (FSVALD_FR1 <= FTVALD_FR1));
					break;

				case 0x37:
				case 0x3f:
					if (IS_SINGLE(op))	/* C.ULE.S */
						SET_FCC((op >> 8) & 7, (FSVALS_FR1 <= FTVALS_FR1));
					else				/* C.ULE.D */
						SET_FCC((op >> 8) & 7, (FSVALD_FR1 <= FTVALD_FR1));
					break;

				default:
					fprintf(stderr, "cop1 %X\n", op);
					break;
			}
			break;
	}
}



/***************************************************************************
    COP1X (FPU EXTRA) EXECUTION HANDLING
***************************************************************************/

INLINE void handle_cop1x_fr0(UINT32 op)
{
	UINT64 temp64;
	UINT32 temp;

	if (!(SR & SR_COP1))
	{
		mips3.badcop_value = 1;
		generate_exception(EXCEPTION_BADCOP, 1);
	}

	switch (op & 0x3f)
	{
		case 0x00:		/* LWXC1 */
			if (RWORD(RSVAL32 + RTVAL32, &temp)) FDVALW_FR0 = temp;
			break;

		case 0x01:		/* LDXC1 */
			if (RDOUBLE(RSVAL32 + RTVAL32, &temp64)) FDVALL_FR0 = temp64;
			break;

		case 0x08:		/* SWXC1 */
			WWORD(RSVAL32 + RTVAL32, get_cop1_reg32(FDREG));
			break;

		case 0x09:		/* SDXC1 */
			WDOUBLE(RSVAL32 + RTVAL32, get_cop1_reg64(FDREG));
			break;

		case 0x0f:		/* PREFX */
			break;

		case 0x20:		/* MADD.S */
			FDVALS_FR0 = FSVALS_FR0 * FTVALS_FR0 + FRVALS_FR0;
			break;

		case 0x21:		/* MADD.D */
			FDVALD_FR0 = FSVALD_FR0 * FTVALD_FR0 + FRVALD_FR0;
			break;

		case 0x28:		/* MSUB.S */
			FDVALS_FR0 = FSVALS_FR0 * FTVALS_FR0 - FRVALS_FR0;
			break;

		case 0x29:		/* MSUB.D */
			FDVALD_FR0 = FSVALD_FR0 * FTVALD_FR0 - FRVALD_FR0;
			break;

		case 0x30:		/* NMADD.S */
			FDVALS_FR0 = -(FSVALS_FR0 * FTVALS_FR0 + FRVALS_FR0);
			break;

		case 0x31:		/* NMADD.D */
			FDVALD_FR0 = -(FSVALD_FR0 * FTVALD_FR0 + FRVALD_FR0);
			break;

		case 0x38:		/* NMSUB.S */
			FDVALS_FR0 = -(FSVALS_FR0 * FTVALS_FR0 - FRVALS_FR0);
			break;

		case 0x39:		/* NMSUB.D */
			FDVALD_FR0 = -(FSVALD_FR0 * FTVALD_FR0 - FRVALD_FR0);
			break;

		case 0x24:		/* MADD.W */
		case 0x25:		/* MADD.L */
		case 0x2c:		/* MSUB.W */
		case 0x2d:		/* MSUB.L */
		case 0x34:		/* NMADD.W */
		case 0x35:		/* NMADD.L */
		case 0x3c:		/* NMSUB.W */
		case 0x3d:		/* NMSUB.L */
		default:
			fprintf(stderr, "cop1x %X\n", op);
			break;
	}
}


INLINE void handle_cop1x_fr1(UINT32 op)
{
	UINT64 temp64;
	UINT32 temp;

	if (!(SR & SR_COP1))
	{
		mips3.badcop_value = 1;
		generate_exception(EXCEPTION_BADCOP, 1);
	}

	switch (op & 0x3f)
	{
		case 0x00:		/* LWXC1 */
			if (RWORD(RSVAL32 + RTVAL32, &temp)) FDVALW_FR1 = temp;
			break;

		case 0x01:		/* LDXC1 */
			if (RDOUBLE(RSVAL32 + RTVAL32, &temp64)) FDVALL_FR1 = temp64;
			break;

		case 0x08:		/* SWXC1 */
			WWORD(RSVAL32 + RTVAL32, get_cop1_reg32(FDREG));
			break;

		case 0x09:		/* SDXC1 */
			WDOUBLE(RSVAL32 + RTVAL32, get_cop1_reg64(FDREG));
			break;

		case 0x0f:		/* PREFX */
			break;

		case 0x20:		/* MADD.S */
			FDVALS_FR1 = FSVALS_FR1 * FTVALS_FR1 + FRVALS_FR1;
			break;

		case 0x21:		/* MADD.D */
			FDVALD_FR1 = FSVALD_FR1 * FTVALD_FR1 + FRVALD_FR1;
			break;

		case 0x28:		/* MSUB.S */
			FDVALS_FR1 = FSVALS_FR1 * FTVALS_FR1 - FRVALS_FR1;
			break;

		case 0x29:		/* MSUB.D */
			FDVALD_FR1 = FSVALD_FR1 * FTVALD_FR1 - FRVALD_FR1;
			break;

		case 0x30:		/* NMADD.S */
			FDVALS_FR1 = -(FSVALS_FR1 * FTVALS_FR1 + FRVALS_FR1);
			break;

		case 0x31:		/* NMADD.D */
			FDVALD_FR1 = -(FSVALD_FR1 * FTVALD_FR1 + FRVALD_FR1);
			break;

		case 0x38:		/* NMSUB.S */
			FDVALS_FR1 = -(FSVALS_FR1 * FTVALS_FR1 - FRVALS_FR1);
			break;

		case 0x39:		/* NMSUB.D */
			FDVALD_FR1 = -(FSVALD_FR1 * FTVALD_FR1 - FRVALD_FR1);
			break;

		case 0x24:		/* MADD.W */
		case 0x25:		/* MADD.L */
		case 0x2c:		/* MSUB.W */
		case 0x2d:		/* MSUB.L */
		case 0x34:		/* NMADD.W */
		case 0x35:		/* NMADD.L */
		case 0x3c:		/* NMSUB.W */
		case 0x3d:		/* NMSUB.L */
		default:
			fprintf(stderr, "cop1x %X\n", op);
			break;
	}
}



/***************************************************************************
    COP2 (CUSTOM) EXECUTION HANDLING
***************************************************************************/

INLINE UINT64 get_cop2_reg(int idx)
{
	return mips3.core.cpr[2][idx];
}

INLINE void set_cop2_reg(int idx, UINT64 val)
{
	mips3.core.cpr[2][idx] = val;
}

INLINE UINT64 get_cop2_creg(int idx)
{
	return mips3.core.ccr[2][idx];
}

INLINE void set_cop2_creg(int idx, UINT64 val)
{
	mips3.core.ccr[2][idx] = val;
}

INLINE void handle_cop2(UINT32 op)
{
	if (!(SR & SR_COP2))
	{
		mips3.badcop_value = 2;
		generate_exception(EXCEPTION_BADCOP, 1);
	}

	switch (RSREG)
	{
		case 0x00:	/* MFCz */		if (RTREG) RTVAL64 = (INT32)get_cop2_reg(RDREG);		break;
		case 0x01:	/* DMFCz */		if (RTREG) RTVAL64 = get_cop2_reg(RDREG);				break;
		case 0x02:	/* CFCz */		if (RTREG) RTVAL64 = (INT32)get_cop2_creg(RDREG);		break;
		case 0x04:	/* MTCz */		set_cop2_reg(RDREG, RTVAL32);							break;
		case 0x05:	/* DMTCz */		set_cop2_reg(RDREG, RTVAL64);							break;
		case 0x06:	/* CTCz */		set_cop2_creg(RDREG, RTVAL32);							break;
		case 0x08:	/* BC */
			switch (RTREG)
			{
				case 0x00:	/* BCzF */	if (!mips3.cf[2]) ADDPC(SIMMVAL);				break;
				case 0x01:	/* BCzF */	if (mips3.cf[2]) ADDPC(SIMMVAL);				break;
				case 0x02:	/* BCzFL */	invalid_instruction(op);							break;
				case 0x03:	/* BCzTL */	invalid_instruction(op);							break;
				default:	invalid_instruction(op);										break;
			}
			break;
		case 0x10:
		case 0x11:
		case 0x12:
		case 0x13:
		case 0x14:
		case 0x15:
		case 0x16:
		case 0x17:
		case 0x18:
		case 0x19:
		case 0x1a:
		case 0x1b:
		case 0x1c:
		case 0x1d:
		case 0x1e:
		case 0x1f:	/* COP */		invalid_instruction(op);								break;
		default:	invalid_instruction(op);												break;
	}
}



/***************************************************************************
    CORE EXECUTION LOOP
***************************************************************************/

CPU_EXECUTE( mips3 )
{
	/* count cycles and interrupt cycles */
	mips3.core.icount -= mips3.interrupt_cycles;
	mips3.interrupt_cycles = 0;

	/* update timers & such */
	mips3com_update_cycle_counting(&mips3.core);

	/* check for IRQs */
	check_irqs();

	/* core execution loop */
	do
	{
		UINT32 op;
		UINT64 temp64 = 0;
		UINT32 temp;

		/* debugging */
		mips3.ppc = mips3.core.pc;
		debugger_instruction_hook(device, mips3.core.pc);

		/* instruction fetch */
		if(!RWORD(mips3.core.pc, &op))
		{
			continue;
		}

		/* adjust for next PC */
		if (mips3.nextpc != ~0)
		{
			mips3.core.pc = mips3.nextpc;
			mips3.nextpc = ~0;
		}
		else
			mips3.core.pc += 4;

		/* parse the instruction */
		switch (op >> 26)
		{
			case 0x00:	/* SPECIAL */
				switch (op & 63)
				{
					case 0x00:	/* SLL */		if (RDREG) RDVAL64 = (INT32)(RTVAL32 << SHIFT);					break;
					case 0x01:	/* MOVF - R5000*/if (RDREG && GET_FCC((op >> 18) & 7) == ((op >> 16) & 1)) RDVAL64 = RSVAL64;	break;
					case 0x02:	/* SRL */		if (RDREG) RDVAL64 = (INT32)(RTVAL32 >> SHIFT);					break;
					case 0x03:	/* SRA */		if (RDREG) RDVAL64 = (INT32)RTVAL32 >> SHIFT;					break;
					case 0x04:	/* SLLV */		if (RDREG) RDVAL64 = (INT32)(RTVAL32 << (RSVAL32 & 31));		break;
					case 0x06:	/* SRLV */		if (RDREG) RDVAL64 = (INT32)(RTVAL32 >> (RSVAL32 & 31));		break;
					case 0x07:	/* SRAV */		if (RDREG) RDVAL64 = (INT32)RTVAL32 >> (RSVAL32 & 31);			break;
					case 0x08:	/* JR */		SETPC(RSVAL32);													break;
					case 0x09:	/* JALR */		SETPCL(RSVAL32,RDREG);											break;
					case 0x0a:	/* MOVZ - R5000 */if (RTVAL64 == 0) { if (RDREG) RDVAL64 = RSVAL64; }			break;
					case 0x0b:	/* MOVN - R5000 */if (RTVAL64 != 0) { if (RDREG) RDVAL64 = RSVAL64; }			break;
					case 0x0c:	/* SYSCALL */	generate_exception(EXCEPTION_SYSCALL, 1);						break;
					case 0x0d:	/* BREAK */		generate_exception(EXCEPTION_BREAK, 1);							break;
					case 0x0f:	/* SYNC */		/* effective no-op */											break;
					case 0x10:	/* MFHI */		if (RDREG) RDVAL64 = HIVAL64;									break;
					case 0x11:	/* MTHI */		HIVAL64 = RSVAL64;												break;
					case 0x12:	/* MFLO */		if (RDREG) RDVAL64 = LOVAL64;									break;
					case 0x13:	/* MTLO */		LOVAL64 = RSVAL64;												break;
					case 0x14:	/* DSLLV */		if (RDREG) RDVAL64 = RTVAL64 << (RSVAL32 & 63);					break;
					case 0x16:	/* DSRLV */		if (RDREG) RDVAL64 = RTVAL64 >> (RSVAL32 & 63);					break;
					case 0x17:	/* DSRAV */		if (RDREG) RDVAL64 = (INT64)RTVAL64 >> (RSVAL32 & 63);			break;
					case 0x18:	/* MULT */
						temp64 = (INT64)(INT32)RSVAL32 * (INT64)(INT32)RTVAL32;
						LOVAL64 = (INT32)temp64;
						HIVAL64 = (INT32)(temp64 >> 32);
						mips3.core.icount -= 3;
						break;
					case 0x19:	/* MULTU */
						temp64 = (UINT64)RSVAL32 * (UINT64)RTVAL32;
						LOVAL64 = (INT32)temp64;
						HIVAL64 = (INT32)(temp64 >> 32);
						mips3.core.icount -= 3;
						break;
					case 0x1a:	/* DIV */
						if (RTVAL32)
						{
							LOVAL64 = (INT32)((INT32)RSVAL32 / (INT32)RTVAL32);
							HIVAL64 = (INT32)((INT32)RSVAL32 % (INT32)RTVAL32);
						}
						mips3.core.icount -= 35;
						break;
					case 0x1b:	/* DIVU */
						if (RTVAL32)
						{
							LOVAL64 = (INT32)(RSVAL32 / RTVAL32);
							HIVAL64 = (INT32)(RSVAL32 % RTVAL32);
						}
						mips3.core.icount -= 35;
						break;
					case 0x1c:	/* DMULT */
						temp64 = (INT64)RSVAL64 * (INT64)RTVAL64;
						LOVAL64 = temp64;
						HIVAL64 = (INT64)temp64 >> 63;
						mips3.core.icount -= 7;
						break;
					case 0x1d:	/* DMULTU */
						temp64 = (UINT64)RSVAL64 * (UINT64)RTVAL64;
						LOVAL64 = temp64;
						HIVAL64 = 0;
						mips3.core.icount -= 7;
						break;
					case 0x1e:	/* DDIV */
						if (RTVAL64)
						{
							LOVAL64 = (INT64)RSVAL64 / (INT64)RTVAL64;
							HIVAL64 = (INT64)RSVAL64 % (INT64)RTVAL64;
						}
						mips3.core.icount -= 67;
						break;
					case 0x1f:	/* DDIVU */
						if (RTVAL64)
						{
							LOVAL64 = RSVAL64 / RTVAL64;
							HIVAL64 = RSVAL64 % RTVAL64;
						}
						mips3.core.icount -= 67;
						break;
					case 0x20:	/* ADD */
						if (ENABLE_OVERFLOWS && RSVAL32 > ~RTVAL32) generate_exception(EXCEPTION_OVERFLOW, 1);
						else if (RDREG) RDVAL64 = (INT32)(RSVAL32 + RTVAL32);
						break;
					case 0x21:	/* ADDU */		if (RDREG) RDVAL64 = (INT32)(RSVAL32 + RTVAL32);				break;
					case 0x22:	/* SUB */
						if (ENABLE_OVERFLOWS && RSVAL32 < RTVAL32) generate_exception(EXCEPTION_OVERFLOW, 1);
						else if (RDREG) RDVAL64 = (INT32)(RSVAL32 - RTVAL32);
						break;
					case 0x23:	/* SUBU */		if (RDREG) RDVAL64 = (INT32)(RSVAL32 - RTVAL32);				break;
					case 0x24:	/* AND */		if (RDREG) RDVAL64 = RSVAL64 & RTVAL64;							break;
					case 0x25:	/* OR */		if (RDREG) RDVAL64 = RSVAL64 | RTVAL64;							break;
					case 0x26:	/* XOR */		if (RDREG) RDVAL64 = RSVAL64 ^ RTVAL64;							break;
					case 0x27:	/* NOR */		if (RDREG) RDVAL64 = ~(RSVAL64 | RTVAL64);						break;
					case 0x2a:	/* SLT */		if (RDREG) RDVAL64 = (INT64)RSVAL64 < (INT64)RTVAL64;			break;
					case 0x2b:	/* SLTU */		if (RDREG) RDVAL64 = (UINT64)RSVAL64 < (UINT64)RTVAL64;			break;
					case 0x2c:	/* DADD */
						if (ENABLE_OVERFLOWS && RSVAL64 > ~RTVAL64) generate_exception(EXCEPTION_OVERFLOW, 1);
						else if (RDREG) RDVAL64 = RSVAL64 + RTVAL64;
						break;
					case 0x2d:	/* DADDU */		if (RDREG) RDVAL64 = RSVAL64 + RTVAL64;							break;
					case 0x2e:	/* DSUB */
						if (ENABLE_OVERFLOWS && RSVAL64 < RTVAL64) generate_exception(EXCEPTION_OVERFLOW, 1);
						else if (RDREG) RDVAL64 = RSVAL64 - RTVAL64;
						break;
					case 0x2f:	/* DSUBU */		if (RDREG) RDVAL64 = RSVAL64 - RTVAL64;							break;
					case 0x30:	/* TGE */		if ((INT64)RSVAL64 >= (INT64)RTVAL64) generate_exception(EXCEPTION_TRAP, 1); break;
					case 0x31:	/* TGEU */		if (RSVAL64 >= RTVAL64) generate_exception(EXCEPTION_TRAP, 1);	break;
					case 0x32:	/* TLT */		if ((INT64)RSVAL64 < (INT64)RTVAL64) generate_exception(EXCEPTION_TRAP, 1); break;
					case 0x33:	/* TLTU */		if (RSVAL64 < RTVAL64) generate_exception(EXCEPTION_TRAP, 1);	break;
					case 0x34:	/* TEQ */		if (RSVAL64 == RTVAL64) generate_exception(EXCEPTION_TRAP, 1);	break;
					case 0x36:	/* TNE */		if (RSVAL64 != RTVAL64) generate_exception(EXCEPTION_TRAP, 1);	break;
					case 0x38:	/* DSLL */		if (RDREG) RDVAL64 = RTVAL64 << SHIFT;							break;
					case 0x3a:	/* DSRL */		if (RDREG) RDVAL64 = RTVAL64 >> SHIFT;							break;
					case 0x3b:	/* DSRA */		if (RDREG) RDVAL64 = (INT64)RTVAL64 >> SHIFT;					break;
					case 0x3c:	/* DSLL32 */	if (RDREG) RDVAL64 = RTVAL64 << (SHIFT + 32);					break;
					case 0x3e:	/* DSRL32 */	if (RDREG) RDVAL64 = RTVAL64 >> (SHIFT + 32);					break;
					case 0x3f:	/* DSRA32 */	if (RDREG) RDVAL64 = (INT64)RTVAL64 >> (SHIFT + 32);			break;
					default:	/* ??? */		invalid_instruction(op);										break;
				}
				break;

			case 0x01:	/* REGIMM */
				switch (RTREG)
				{
					case 0x00:	/* BLTZ */		if ((INT64)RSVAL64 < 0) ADDPC(SIMMVAL);							break;
					case 0x01:	/* BGEZ */		if ((INT64)RSVAL64 >= 0) ADDPC(SIMMVAL);						break;
					case 0x02:	/* BLTZL */		if ((INT64)RSVAL64 < 0) ADDPC(SIMMVAL);	else mips3.core.pc += 4;		break;
					case 0x03:	/* BGEZL */		if ((INT64)RSVAL64 >= 0) ADDPC(SIMMVAL); else mips3.core.pc += 4;	break;
					case 0x08:	/* TGEI */		if ((INT64)RSVAL64 >= SIMMVAL) generate_exception(EXCEPTION_TRAP, 1);	break;
					case 0x09:	/* TGEIU */		if (RSVAL64 >= SIMMVAL) generate_exception(EXCEPTION_TRAP, 1);	break;
					case 0x0a:	/* TLTI */		if ((INT64)RSVAL64 < SIMMVAL) generate_exception(EXCEPTION_TRAP, 1);	break;
					case 0x0b:	/* TLTIU */		if (RSVAL64 >= SIMMVAL) generate_exception(EXCEPTION_TRAP, 1);	break;
					case 0x0c:	/* TEQI */		if (RSVAL64 == SIMMVAL) generate_exception(EXCEPTION_TRAP, 1);	break;
					case 0x0e:	/* TNEI */		if (RSVAL64 != SIMMVAL) generate_exception(EXCEPTION_TRAP, 1);	break;
					case 0x10:	/* BLTZAL */	if ((INT64)RSVAL64 < 0) ADDPCL(SIMMVAL,31);						break;
					case 0x11:	/* BGEZAL */	if ((INT64)RSVAL64 >= 0) ADDPCL(SIMMVAL,31);					break;
					case 0x12:	/* BLTZALL */	if ((INT64)RSVAL64 < 0) ADDPCL(SIMMVAL,31) else mips3.core.pc += 4;	break;
					case 0x13:	/* BGEZALL */	if ((INT64)RSVAL64 >= 0) ADDPCL(SIMMVAL,31) else mips3.core.pc += 4;	break;
					default:	/* ??? */		invalid_instruction(op);										break;
				}
				break;

			case 0x02:	/* J */			ABSPC(LIMMVAL);															break;
			case 0x03:	/* JAL */		ABSPCL(LIMMVAL,31);														break;
			case 0x04:	/* BEQ */		if (RSVAL64 == RTVAL64) ADDPC(SIMMVAL);									break;
			case 0x05:	/* BNE */		if (RSVAL64 != RTVAL64) ADDPC(SIMMVAL);									break;
			case 0x06:	/* BLEZ */		if ((INT64)RSVAL64 <= 0) ADDPC(SIMMVAL);								break;
			case 0x07:	/* BGTZ */		if ((INT64)RSVAL64 > 0) ADDPC(SIMMVAL);									break;
			case 0x08:	/* ADDI */
				if (ENABLE_OVERFLOWS && RSVAL32 > ~SIMMVAL) generate_exception(EXCEPTION_OVERFLOW, 1);
				else if (RTREG) RTVAL64 = (INT32)(RSVAL32 + SIMMVAL);
				break;
			case 0x09:	/* ADDIU */		if (RTREG) RTVAL64 = (INT32)(RSVAL32 + SIMMVAL);						break;
			case 0x0a:	/* SLTI */		if (RTREG) RTVAL64 = (INT64)RSVAL64 < (INT64)SIMMVAL;					break;
			case 0x0b:	/* SLTIU */		if (RTREG) RTVAL64 = (UINT64)RSVAL64 < (UINT64)SIMMVAL;					break;
			case 0x0c:	/* ANDI */		if (RTREG) RTVAL64 = RSVAL64 & UIMMVAL;									break;
			case 0x0d:	/* ORI */		if (RTREG) RTVAL64 = RSVAL64 | UIMMVAL;									break;
			case 0x0e:	/* XORI */		if (RTREG) RTVAL64 = RSVAL64 ^ UIMMVAL;									break;
			case 0x0f:	/* LUI */		if (RTREG) RTVAL64 = (INT32)(UIMMVAL << 16);							break;
			case 0x10:	/* COP0 */		handle_cop0(op);														break;
			case 0x11:	/* COP1 */		if (IS_FR0) handle_cop1_fr0(op); else handle_cop1_fr1(op);				break;
			case 0x12:	/* COP2 */		handle_cop2(op);														break;
			case 0x13:	/* COP1X - R5000 */if (IS_FR0) handle_cop1x_fr0(op); else handle_cop1x_fr1(op);			break;
			case 0x14:	/* BEQL */		if (RSVAL64 == RTVAL64) ADDPC(SIMMVAL); else mips3.core.pc += 4;				break;
			case 0x15:	/* BNEL */		if (RSVAL64 != RTVAL64) ADDPC(SIMMVAL);	else mips3.core.pc += 4;				break;
			case 0x16:	/* BLEZL */		if ((INT64)RSVAL64 <= 0) ADDPC(SIMMVAL); else mips3.core.pc += 4;			break;
			case 0x17:	/* BGTZL */		if ((INT64)RSVAL64 > 0) ADDPC(SIMMVAL); else mips3.core.pc += 4;				break;
			case 0x18:	/* DADDI */
				if (ENABLE_OVERFLOWS && RSVAL64 > ~SIMMVAL) generate_exception(EXCEPTION_OVERFLOW, 1);
				else if (RTREG) RTVAL64 = RSVAL64 + (INT64)SIMMVAL;
				break;
			case 0x19:	/* DADDIU */	if (RTREG) RTVAL64 = RSVAL64 + (UINT64)SIMMVAL;							break;
			case 0x1a:	/* LDL */		(*mips3.ldl)(op);														break;
			case 0x1b:	/* LDR */		(*mips3.ldr)(op);														break;
			case 0x1c:	/* IDT-specific opcodes: mad/madu/mul on R4640/4650, msub on RC32364 */
				switch (op & 0x1f)
				{
					case 2: /* MUL */
						RDVAL64 = (INT32)((INT32)RSVAL32 * (INT32)RTVAL32);
						mips3.core.icount -= 3;
						break;
					default: invalid_instruction(op);
				}
				break;
			case 0x20:	/* LB */		if (RBYTE(SIMMVAL+RSVAL32, &temp) && RTREG) RTVAL64 = (INT8)temp;		break;
			case 0x21:	/* LH */		if (RHALF(SIMMVAL+RSVAL32, &temp) && RTREG) RTVAL64 = (INT16)temp;		break;
			case 0x22:	/* LWL */		(*mips3.lwl)(op);														break;
			case 0x23:	/* LW */		if (RWORD(SIMMVAL+RSVAL32, &temp) && RTREG) RTVAL64 = (INT32)temp;		break;
			case 0x24:	/* LBU */		if (RBYTE(SIMMVAL+RSVAL32, &temp) && RTREG) RTVAL64 = (UINT8)temp;		break;
			case 0x25:	/* LHU */		if (RHALF(SIMMVAL+RSVAL32, &temp) && RTREG) RTVAL64 = (UINT16)temp;		break;
			case 0x26:	/* LWR */		(*mips3.lwr)(op);														break;
			case 0x27:	/* LWU */		if (RWORD(SIMMVAL+RSVAL32, &temp) && RTREG) RTVAL64 = (UINT32)temp;		break;
			case 0x28:	/* SB */		WBYTE(SIMMVAL+RSVAL32, RTVAL32);										break;
			case 0x29:	/* SH */		WHALF(SIMMVAL+RSVAL32, RTVAL32);										break;
			case 0x2a:	/* SWL */		(*mips3.swl)(op);														break;
			case 0x2b:	/* SW */		WWORD(SIMMVAL+RSVAL32, RTVAL32);										break;
			case 0x2c:	/* SDL */		(*mips3.sdl)(op);														break;
			case 0x2d:	/* SDR */		(*mips3.sdr)(op);														break;
			case 0x2e:	/* SWR */		(*mips3.swr)(op);														break;
			case 0x2f:	/* CACHE */		/* effective no-op */													break;
			case 0x30:	/* LL */		if (RWORD(SIMMVAL+RSVAL32, &temp) && RTREG) RTVAL64 = (UINT32)temp; mips3.ll_value = RTVAL32;		break;
			case 0x31:	/* LWC1 */		if (RWORD(SIMMVAL+RSVAL32, &temp)) set_cop1_reg32(RTREG, temp);			break;
			case 0x32:	/* LWC2 */		if (RWORD(SIMMVAL+RSVAL32, &temp)) set_cop2_reg(RTREG, temp);			break;
			case 0x33:	/* PREF */		/* effective no-op */													break;
			case 0x34:	/* LLD */		if (RDOUBLE(SIMMVAL+RSVAL32, &temp64) && RTREG) RTVAL64 = temp64; mips3.lld_value = temp64;		break;
			case 0x35:	/* LDC1 */		if (RDOUBLE(SIMMVAL+RSVAL32, &temp64)) set_cop1_reg64(RTREG, temp64);		break;
			case 0x36:	/* LDC2 */		if (RDOUBLE(SIMMVAL+RSVAL32, &temp64)) set_cop2_reg(RTREG, temp64);		break;
			case 0x37:	/* LD */		if (RDOUBLE(SIMMVAL+RSVAL32, &temp64) && RTREG) RTVAL64 = temp64;		break;
			case 0x38:	/* SC */		if (RWORD(SIMMVAL+RSVAL32, &temp) && RTREG)
								{
									if (temp == mips3.ll_value)
									{
										WWORD(SIMMVAL+RSVAL32, RTVAL32);
										RTVAL64 = (UINT32)1;
									}
									else
									{
										RTVAL64 = (UINT32)0;
									}
								}
								break;
			case 0x39:	/* SWC1 */		WWORD(SIMMVAL+RSVAL32, get_cop1_reg32(RTREG));							break;
			case 0x3a:	/* SWC2 */		WWORD(SIMMVAL+RSVAL32, get_cop2_reg(RTREG));							break;
			case 0x3b:	/* SWC3 */		invalid_instruction(op);												break;
			case 0x3c:	/* SCD */		if (RDOUBLE(SIMMVAL+RSVAL32, &temp64) && RTREG)
								{
									if (temp64 == mips3.lld_value)
									{
										WDOUBLE(SIMMVAL+RSVAL32, RTVAL64);
										RTVAL64 = 1;
									}
									else
									{
										RTVAL64 = 0;
									}
								}
								break;
			case 0x3d:	/* SDC1 */		WDOUBLE(SIMMVAL+RSVAL32, get_cop1_reg64(RTREG));							break;
			case 0x3e:	/* SDC2 */		WDOUBLE(SIMMVAL+RSVAL32, get_cop2_reg(RTREG));							break;
			case 0x3f:	/* SD */		WDOUBLE(SIMMVAL+RSVAL32, RTVAL64);										break;
			default:	/* ??? */		invalid_instruction(op);												break;
		}
		mips3.core.icount--;

	} while (mips3.core.icount > 0 || mips3.nextpc != ~0);

	mips3.core.icount -= mips3.interrupt_cycles;
	mips3.interrupt_cycles = 0;
}



/***************************************************************************
    COMPLEX OPCODE IMPLEMENTATIONS
***************************************************************************/

static void lwl_be(UINT32 op)
{
	offs_t offs = SIMMVAL + RSVAL32;
	int shift = 8 * (offs & 3);
	UINT32 mask = 0xffffffffUL << shift;
	UINT32 temp;

	if (RWORD_MASKED(offs & ~3, &temp, mask >> shift) && RTREG)
		RTVAL64 = (INT32)((RTVAL32 & ~mask) | (temp << shift));
}

static void lwr_be(UINT32 op)
{
	offs_t offs = SIMMVAL + RSVAL32;
	int shift = 8 * (~offs & 3);
	UINT32 mask = 0xffffffffUL >> shift;
	UINT32 temp;

	if (RWORD_MASKED(offs & ~3, &temp, mask << shift) && RTREG)
		RTVAL64 = (INT32)((RTVAL32 & ~mask) | (temp >> shift));
}

static void ldl_be(UINT32 op)
{
	offs_t offs = SIMMVAL + RSVAL32;
	int shift = 8 * (offs & 7);
	UINT64 mask = U64(0xffffffffffffffff) << shift;
	UINT64 temp;

	if (RDOUBLE_MASKED(offs & ~7, &temp, mask >> shift) && RTREG)
		RTVAL64 = (RTVAL64 & ~mask) | (temp << shift);
}

static void ldr_be(UINT32 op)
{
	offs_t offs = SIMMVAL + RSVAL32;
	int shift = 8 * (~offs & 7);
	UINT64 mask = U64(0xffffffffffffffff) >> shift;
	UINT64 temp;

	if (RDOUBLE_MASKED(offs & ~7, &temp, mask << shift) && RTREG)
		RTVAL64 = (RTVAL64 & ~mask) | (temp >> shift);
}

static void swl_be(UINT32 op)
{
	offs_t offs = SIMMVAL + RSVAL32;
	int shift = 8 * (offs & 3);
	UINT32 mask = 0xffffffffUL >> shift;
	WWORD_MASKED(offs & ~3, RTVAL32 >> shift, mask);
}

static void swr_be(UINT32 op)
{
	offs_t offs = SIMMVAL + RSVAL32;
	int shift = 8 * (~offs & 3);
	UINT32 mask = 0xffffffffUL << shift;
	WWORD_MASKED(offs & ~3, RTVAL32 << shift, mask);
}

static void sdl_be(UINT32 op)
{
	offs_t offs = SIMMVAL + RSVAL32;
	int shift = 8 * (offs & 7);
	UINT64 mask = U64(0xffffffffffffffff) >> shift;
	WDOUBLE_MASKED(offs & ~7, RTVAL64 >> shift, mask);
}

static void sdr_be(UINT32 op)
{
	offs_t offs = SIMMVAL + RSVAL32;
	int shift = 8 * (~offs & 7);
	UINT64 mask = U64(0xffffffffffffffff) << shift;
	WDOUBLE_MASKED(offs & ~7, RTVAL64 << shift, mask);
}



static void lwl_le(UINT32 op)
{
	offs_t offs = SIMMVAL + RSVAL32;
	int shift = 8 * (~offs & 3);
	UINT32 mask = 0xffffffffUL << shift;
	UINT32 temp;

	if (RWORD_MASKED(offs & ~3, &temp, mask >> shift) && RTREG)
		RTVAL64 = (INT32)((RTVAL32 & ~mask) | (temp << shift));
}

static void lwr_le(UINT32 op)
{
	offs_t offs = SIMMVAL + RSVAL32;
	int shift = 8 * (offs & 3);
	UINT32 mask = 0xffffffffUL >> shift;
	UINT32 temp;

	if (RWORD_MASKED(offs & ~3, &temp, mask << shift) && RTREG)
		RTVAL64 = (INT32)((RTVAL32 & ~mask) | (temp >> shift));
}

static void ldl_le(UINT32 op)
{
	offs_t offs = SIMMVAL + RSVAL32;
	int shift = 8 * (~offs & 7);
	UINT64 mask = U64(0xffffffffffffffff) << shift;
	UINT64 temp;

	if (RDOUBLE_MASKED(offs & ~7, &temp, mask >> shift) && RTREG)
		RTVAL64 = (RTVAL64 & ~mask) | (temp << shift);
}

static void ldr_le(UINT32 op)
{
	offs_t offs = SIMMVAL + RSVAL32;
	int shift = 8 * (offs & 7);
	UINT64 mask = U64(0xffffffffffffffff) >> shift;
	UINT64 temp;

	if (RDOUBLE_MASKED(offs & ~7, &temp, mask << shift) && RTREG)
		RTVAL64 = (RTVAL64 & ~mask) | (temp >> shift);
}

static void swl_le(UINT32 op)
{
	offs_t offs = SIMMVAL + RSVAL32;
	int shift = 8 * (~offs & 3);
	UINT32 mask = 0xffffffffUL >> shift;
	WWORD_MASKED(offs & ~3, RTVAL32 >> shift, mask);
}

static void swr_le(UINT32 op)
{
	offs_t offs = SIMMVAL + RSVAL32;
	int shift = 8 * (offs & 3);
	UINT32 mask = 0xffffffffUL << shift;
	WWORD_MASKED(offs & ~3, RTVAL32 << shift, mask);
}

static void sdl_le(UINT32 op)
{
	offs_t offs = SIMMVAL + RSVAL32;
	int shift = 8 * (~offs & 7);
	UINT64 mask = U64(0xffffffffffffffff) >> shift;
	WDOUBLE_MASKED(offs & ~7, RTVAL64 >> shift, mask);
}

static void sdr_le(UINT32 op)
{
	offs_t offs = SIMMVAL + RSVAL32;
	int shift = 8 * (offs & 7);
	UINT64 mask = U64(0xffffffffffffffff) << shift;
	WDOUBLE_MASKED(offs & ~7, RTVAL64 << shift, mask);
}



/***************************************************************************
    GENERIC GET/SET INFO
***************************************************************************/

static CPU_SET_INFO( mips3 )
{
	/* everything is handled generically here */
	mips3com_set_info(&mips3.core, state, info);
}


static CPU_GET_INFO( mips3 )
{
	switch (state)
	{
		/* --- the following bits of info are returned as 64-bit signed integers --- */
		case CPUINFO_INT_CONTEXT_SIZE:					info->i = sizeof(mips3);				break;
		case CPUINFO_INT_PREVIOUSPC:					info->i = mips3.ppc;					break;

		/* --- the following bits of info are returned as pointers to data or functions --- */
		case CPUINFO_FCT_SET_INFO:						info->setinfo = CPU_SET_INFO_NAME(mips3);			break;
		case CPUINFO_FCT_INIT:							/* provided per-CPU */					break;
		case CPUINFO_FCT_RESET:							info->reset = CPU_RESET_NAME(mips3);				break;
		case CPUINFO_FCT_EXECUTE:						info->execute = CPU_EXECUTE_NAME(mips3);			break;
		case CPUINFO_FCT_DISASSEMBLE:					info->disassemble = CPU_DISASSEMBLE_NAME(mips3);			break;
		case CPUINFO_FCT_TRANSLATE:						info->translate = CPU_TRANSLATE_NAME(mips3);		break;

		/* --- the following bits of info are returned as NULL-terminated strings --- */
		case CPUINFO_STR_SOURCE_FILE:						strcpy(info->s, __FILE__);				break;

		/* --- everything else is handled generically --- */
		default:										mips3com_get_info(&mips3.core, state, info); break;
	}
}



/***************************************************************************
    NEC VR4300 VARIANTS
***************************************************************************/

// NEC VR4300 series is MIPS III with 32-bit address bus and slightly custom COP0/TLB
static CPU_INIT( vr4300be )
{
	mips3com_init(&mips3.core, MIPS3_TYPE_VR4300, TRUE, device, irqcallback);
	mips3.tlb_table = vtlb_table(mips3.core.vtlb);
}

static CPU_INIT( vr4300le )
{
	mips3com_init(&mips3.core, MIPS3_TYPE_VR4300, FALSE, device, irqcallback);
	mips3.tlb_table = vtlb_table(mips3.core.vtlb);
}

CPU_GET_INFO( vr4300be )
{
	switch (state)
	{
		/* --- the following bits of info are returned as 64-bit signed integers --- */
		case CPUINFO_INT_ENDIANNESS:					info->i = ENDIANNESS_BIG;					break;

		/* --- the following bits of info are returned as pointers to data or functions --- */
		case CPUINFO_FCT_INIT:							info->init = CPU_INIT_NAME(vr4300be);				break;

		/* --- the following bits of info are returned as NULL-terminated strings --- */
		case CPUINFO_STR_NAME:							strcpy(info->s, "VR4300 (big)");			break;

		/* --- everything else is handled generically --- */
		default:										CPU_GET_INFO_CALL(mips3);			break;
	}
}

CPU_GET_INFO( vr4300le )
{
	switch (state)
	{
		/* --- the following bits of info are returned as 64-bit signed integers --- */
		case CPUINFO_INT_ENDIANNESS:					info->i = ENDIANNESS_LITTLE;					break;

		/* --- the following bits of info are returned as pointers to data or functions --- */
		case CPUINFO_FCT_INIT:							info->init = CPU_INIT_NAME(vr4300le);				break;

		/* --- the following bits of info are returned as NULL-terminated strings --- */
		case CPUINFO_STR_NAME:							strcpy(info->s, "VR4300 (little)");		break;

		/* --- everything else is handled generically --- */
		default:										CPU_GET_INFO_CALL(mips3);			break;
	}
}

// VR4310 = VR4300 with different speed bin
CPU_GET_INFO( vr4310be )
{
	switch (state)
	{
		/* --- the following bits of info are returned as 64-bit signed integers --- */
		case CPUINFO_INT_ENDIANNESS:					info->i = ENDIANNESS_BIG;					break;

		/* --- the following bits of info are returned as pointers to data or functions --- */
		case CPUINFO_FCT_INIT:							info->init = CPU_INIT_NAME(vr4300be);				break;

		/* --- the following bits of info are returned as NULL-terminated strings --- */
		case CPUINFO_STR_NAME:							strcpy(info->s, "VR4310 (big)");			break;

		/* --- everything else is handled generically --- */
		default:										CPU_GET_INFO_CALL(mips3);			break;
	}
}

CPU_GET_INFO( vr4310le )
{
	switch (state)
	{
		/* --- the following bits of info are returned as 64-bit signed integers --- */
		case CPUINFO_INT_ENDIANNESS:					info->i = ENDIANNESS_LITTLE;					break;

		/* --- the following bits of info are returned as pointers to data or functions --- */
		case CPUINFO_FCT_INIT:							info->init = CPU_INIT_NAME(vr4300le);				break;

		/* --- the following bits of info are returned as NULL-terminated strings --- */
		case CPUINFO_STR_NAME:							strcpy(info->s, "VR4310 (little)");		break;

		/* --- everything else is handled generically --- */
		default:										CPU_GET_INFO_CALL(mips3);			break;
	}
}


/***************************************************************************
    R4600 VARIANTS
***************************************************************************/

static CPU_INIT( r4600be )
{
	mips3com_init(&mips3.core, MIPS3_TYPE_R4600, TRUE, device, irqcallback);
	mips3.tlb_table = vtlb_table(mips3.core.vtlb);
}

static CPU_INIT( r4600le )
{
	mips3com_init(&mips3.core, MIPS3_TYPE_R4600, FALSE, device, irqcallback);
	mips3.tlb_table = vtlb_table(mips3.core.vtlb);
}

CPU_GET_INFO( r4600be )
{
	switch (state)
	{
		/* --- the following bits of info are returned as 64-bit signed integers --- */
		case CPUINFO_INT_ENDIANNESS:					info->i = ENDIANNESS_BIG;					break;

		/* --- the following bits of info are returned as pointers to data or functions --- */
		case CPUINFO_FCT_INIT:							info->init = CPU_INIT_NAME(r4600be);				break;

		/* --- the following bits of info are returned as NULL-terminated strings --- */
		case CPUINFO_STR_NAME:							strcpy(info->s, "R4600 (big)");			break;

		/* --- everything else is handled generically --- */
		default:										CPU_GET_INFO_CALL(mips3);			break;
	}
}

CPU_GET_INFO( r4600le )
{
	switch (state)
	{
		/* --- the following bits of info are returned as 64-bit signed integers --- */
		case CPUINFO_INT_ENDIANNESS:					info->i = ENDIANNESS_LITTLE;					break;

		/* --- the following bits of info are returned as pointers to data or functions --- */
		case CPUINFO_FCT_INIT:							info->init = CPU_INIT_NAME(r4600le);				break;

		/* --- the following bits of info are returned as NULL-terminated strings --- */
		case CPUINFO_STR_NAME:							strcpy(info->s, "R4600 (little)");		break;

		/* --- everything else is handled generically --- */
		default:										CPU_GET_INFO_CALL(mips3);			break;
	}
}



/***************************************************************************
    R4650 VARIANTS
***************************************************************************/

static CPU_INIT( r4650be )
{
	mips3com_init(&mips3.core, MIPS3_TYPE_R4650, TRUE, device, irqcallback);
	mips3.tlb_table = vtlb_table(mips3.core.vtlb);
}

static CPU_INIT( r4650le )
{
	mips3com_init(&mips3.core, MIPS3_TYPE_R4650, FALSE, device, irqcallback);
	mips3.tlb_table = vtlb_table(mips3.core.vtlb);
}

CPU_GET_INFO( r4650be )
{
	switch (state)
	{
		/* --- the following bits of info are returned as 64-bit signed integers --- */
		case CPUINFO_INT_ENDIANNESS:					info->i = ENDIANNESS_BIG;					break;

		/* --- the following bits of info are returned as pointers to data or functions --- */
		case CPUINFO_FCT_INIT:							info->init = CPU_INIT_NAME(r4650be);				break;

		/* --- the following bits of info are returned as NULL-terminated strings --- */
		case CPUINFO_STR_NAME:							strcpy(info->s, "IDT R4650 (big)");		break;

		/* --- everything else is handled generically --- */
		default:										CPU_GET_INFO_CALL(mips3);			break;
	}
}

CPU_GET_INFO( r4650le )
{
	switch (state)
	{
		/* --- the following bits of info are returned as 64-bit signed integers --- */
		case CPUINFO_INT_ENDIANNESS:					info->i = ENDIANNESS_LITTLE;					break;

		/* --- the following bits of info are returned as pointers to data or functions --- */
		case CPUINFO_FCT_INIT:							info->init = CPU_INIT_NAME(r4650le);				break;

		/* --- the following bits of info are returned as NULL-terminated strings --- */
		case CPUINFO_STR_NAME:							strcpy(info->s, "IDT R4650 (little)");	break;

		/* --- everything else is handled generically --- */
		default:										CPU_GET_INFO_CALL(mips3);			break;
	}
}



/***************************************************************************
    R4700 VARIANTS
***************************************************************************/

static CPU_INIT( r4700be )
{
	mips3com_init(&mips3.core, MIPS3_TYPE_R4700, TRUE, device, irqcallback);
	mips3.tlb_table = vtlb_table(mips3.core.vtlb);
}

static CPU_INIT( r4700le )
{
	mips3com_init(&mips3.core, MIPS3_TYPE_R4700, FALSE, device, irqcallback);
	mips3.tlb_table = vtlb_table(mips3.core.vtlb);
}

CPU_GET_INFO( r4700be )
{
	switch (state)
	{
		/* --- the following bits of info are returned as 64-bit signed integers --- */
		case CPUINFO_INT_ENDIANNESS:					info->i = ENDIANNESS_BIG;					break;

		/* --- the following bits of info are returned as pointers to data or functions --- */
		case CPUINFO_FCT_INIT:							info->init = CPU_INIT_NAME(r4700be);				break;

		/* --- the following bits of info are returned as NULL-terminated strings --- */
		case CPUINFO_STR_NAME:							strcpy(info->s, "R4700 (big)");			break;

		/* --- everything else is handled generically --- */
		default:										CPU_GET_INFO_CALL(mips3);			break;
	}
}


CPU_GET_INFO( r4700le )
{
	switch (state)
	{
		/* --- the following bits of info are returned as 64-bit signed integers --- */
		case CPUINFO_INT_ENDIANNESS:					info->i = ENDIANNESS_LITTLE;					break;

		/* --- the following bits of info are returned as pointers to data or functions --- */
		case CPUINFO_FCT_INIT:							info->init = CPU_INIT_NAME(r4700le);				break;

		/* --- the following bits of info are returned as NULL-terminated strings --- */
		case CPUINFO_STR_NAME:							strcpy(info->s, "R4700 (little)");		break;

		/* --- everything else is handled generically --- */
		default:										CPU_GET_INFO_CALL(mips3);			break;
	}
}



/***************************************************************************
    R5000 VARIANTS
***************************************************************************/

static CPU_INIT( r5000be )
{
	mips3com_init(&mips3.core, MIPS3_TYPE_R5000, TRUE, device, irqcallback);
	mips3.tlb_table = vtlb_table(mips3.core.vtlb);
}

static CPU_INIT( r5000le )
{
	mips3com_init(&mips3.core, MIPS3_TYPE_R5000, FALSE, device, irqcallback);
	mips3.tlb_table = vtlb_table(mips3.core.vtlb);
}

CPU_GET_INFO( r5000be )
{
	switch (state)
	{
		/* --- the following bits of info are returned as 64-bit signed integers --- */
		case CPUINFO_INT_ENDIANNESS:					info->i = ENDIANNESS_BIG;					break;

		/* --- the following bits of info are returned as pointers to data or functions --- */
		case CPUINFO_FCT_INIT:							info->init = CPU_INIT_NAME(r5000be);				break;

		/* --- the following bits of info are returned as NULL-terminated strings --- */
		case CPUINFO_STR_NAME:							strcpy(info->s, "R5000 (big)");			break;

		/* --- everything else is handled generically --- */
		default:										CPU_GET_INFO_CALL(mips3);			break;
	}
}

CPU_GET_INFO( r5000le )
{
	switch (state)
	{
		/* --- the following bits of info are returned as 64-bit signed integers --- */
		case CPUINFO_INT_ENDIANNESS:					info->i = ENDIANNESS_LITTLE;					break;

		/* --- the following bits of info are returned as pointers to data or functions --- */
		case CPUINFO_FCT_INIT:							info->init = CPU_INIT_NAME(r5000le);				break;

		/* --- the following bits of info are returned as NULL-terminated strings --- */
		case CPUINFO_STR_NAME:							strcpy(info->s, "R5000 (little)");		break;

		/* --- everything else is handled generically --- */
		default:										CPU_GET_INFO_CALL(mips3);			break;
	}
}



/***************************************************************************
    QED5271 VARIANTS
***************************************************************************/

static CPU_INIT( qed5271be )
{
	mips3com_init(&mips3.core, MIPS3_TYPE_QED5271, TRUE, device, irqcallback);
	mips3.tlb_table = vtlb_table(mips3.core.vtlb);
}

static CPU_INIT( qed5271le )
{
	mips3com_init(&mips3.core, MIPS3_TYPE_QED5271, FALSE, device, irqcallback);
	mips3.tlb_table = vtlb_table(mips3.core.vtlb);
}

CPU_GET_INFO( qed5271be )
{
	switch (state)
	{
		/* --- the following bits of info are returned as 64-bit signed integers --- */
		case CPUINFO_INT_ENDIANNESS:					info->i = ENDIANNESS_BIG;					break;

		/* --- the following bits of info are returned as pointers to data or functions --- */
		case CPUINFO_FCT_INIT:							info->init = CPU_INIT_NAME(qed5271be);			break;

		/* --- the following bits of info are returned as NULL-terminated strings --- */
		case CPUINFO_STR_NAME:							strcpy(info->s, "QED5271 (big)");		break;

		/* --- everything else is handled generically --- */
		default:										CPU_GET_INFO_CALL(mips3);			break;
	}
}

CPU_GET_INFO( qed5271le )
{
	switch (state)
	{
		/* --- the following bits of info are returned as 64-bit signed integers --- */
		case CPUINFO_INT_ENDIANNESS:					info->i = ENDIANNESS_LITTLE;					break;

		/* --- the following bits of info are returned as pointers to data or functions --- */
		case CPUINFO_FCT_INIT:							info->init = CPU_INIT_NAME(qed5271le);			break;

		/* --- the following bits of info are returned as NULL-terminated strings --- */
		case CPUINFO_STR_NAME:							strcpy(info->s, "QED5271 (little)");	break;

		/* --- everything else is handled generically --- */
		default:										CPU_GET_INFO_CALL(mips3);			break;
	}
}



/***************************************************************************
    RM7000 VARIANTS
***************************************************************************/

static CPU_INIT( rm7000be )
{
	mips3com_init(&mips3.core, MIPS3_TYPE_RM7000, TRUE, device, irqcallback);
	mips3.tlb_table = vtlb_table(mips3.core.vtlb);
}

static CPU_INIT( rm7000le )
{
	mips3com_init(&mips3.core, MIPS3_TYPE_RM7000, FALSE, device, irqcallback);
	mips3.tlb_table = vtlb_table(mips3.core.vtlb);
}

CPU_GET_INFO( rm7000be )
{
	switch (state)
	{
		/* --- the following bits of info are returned as 64-bit signed integers --- */
		case CPUINFO_INT_ENDIANNESS:					info->i = ENDIANNESS_BIG;					break;

		/* --- the following bits of info are returned as pointers to data or functions --- */
		case CPUINFO_FCT_INIT:							info->init = CPU_INIT_NAME(rm7000be);				break;

		/* --- the following bits of info are returned as NULL-terminated strings --- */
		case CPUINFO_STR_NAME:							strcpy(info->s, "RM7000 (big)");		break;

		/* --- everything else is handled generically --- */
		default:										CPU_GET_INFO_CALL(mips3);			break;
	}
}

CPU_GET_INFO( rm7000le )
{
	switch (state)
	{
		/* --- the following bits of info are returned as 64-bit signed integers --- */
		case CPUINFO_INT_ENDIANNESS:					info->i = ENDIANNESS_LITTLE;					break;

		/* --- the following bits of info are returned as pointers to data or functions --- */
		case CPUINFO_FCT_INIT:							info->init = CPU_INIT_NAME(rm7000le);				break;

		/* --- the following bits of info are returned as NULL-terminated strings --- */
		case CPUINFO_STR_NAME:							strcpy(info->s, "RM7000 (little)");		break;

		/* --- everything else is handled generically --- */
		default:										CPU_GET_INFO_CALL(mips3);			break;
	}
}

DEFINE_LEGACY_CPU_DEVICE(VR4300BE, vr4300be);
DEFINE_LEGACY_CPU_DEVICE(VR4300LE, vr4300le);
DEFINE_LEGACY_CPU_DEVICE(VR4310BE, vr4310be);
DEFINE_LEGACY_CPU_DEVICE(VR4310LE, vr4310le);

DEFINE_LEGACY_CPU_DEVICE(R4600BE, r4600be);
DEFINE_LEGACY_CPU_DEVICE(R4600LE, r4600le);

DEFINE_LEGACY_CPU_DEVICE(R4650BE, r4650be);
DEFINE_LEGACY_CPU_DEVICE(R4650LE, r4650le);

DEFINE_LEGACY_CPU_DEVICE(R4700BE, r4700be);
DEFINE_LEGACY_CPU_DEVICE(R4700LE, r4700le);

DEFINE_LEGACY_CPU_DEVICE(R5000BE, r5000be);
DEFINE_LEGACY_CPU_DEVICE(R5000LE, r5000le);

DEFINE_LEGACY_CPU_DEVICE(QED5271BE, qed5271be);
DEFINE_LEGACY_CPU_DEVICE(QED5271LE, qed5271le);

DEFINE_LEGACY_CPU_DEVICE(RM7000BE, rm7000be);
DEFINE_LEGACY_CPU_DEVICE(RM7000LE, rm7000le);

#endif
