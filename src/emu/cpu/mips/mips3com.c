/***************************************************************************

    mips3com.c

    Common MIPS III/IV definitions and functions

***************************************************************************/

#include "mips3com.h"


/***************************************************************************
    DEBUGGING
***************************************************************************/

#define PRINTF_TLB				(1)



/***************************************************************************
    FUNCTION PROTOTYPES
***************************************************************************/

static TIMER_CALLBACK( compare_int_callback );

static UINT32 compute_config_register(const mips3_state *mips);
static UINT32 compute_prid_register(const mips3_state *mips);

static void tlb_write_common(mips3_state *mips, int index);
static void tlb_entry_log_half(mips3_tlb_entry *tlbent, int index, int which);

static UINT64 program_read_qword_32be(offs_t offset);
static UINT64 program_read_qword_masked_32be(offs_t offset, UINT64 mem_mask);

static void program_write_qword_32be(offs_t offset, UINT64 data);
static void program_write_qword_masked_32be(offs_t offset, UINT64 data, UINT64 mem_mask);

static UINT64 program_read_qword_32le(offs_t offset);
static UINT64 program_read_qword_masked_32le(offs_t offset, UINT64 mem_mask);

static void program_write_qword_32le(offs_t offset, UINT64 data);
static void program_write_qword_masked_32le(offs_t offset, UINT64 data, UINT64 mem_mask);



/***************************************************************************
    PRIVATE GLOBAL VARIABLES
***************************************************************************/

static const memory_handlers be_memory =
{
	program_read_byte_32be,
	program_read_word_32be,
	program_read_dword_32be,
	program_read_masked_32be,
	program_read_qword_32be,
	program_read_qword_masked_32be,

	program_write_byte_32be,
	program_write_word_32be,
	program_write_dword_32be,
	program_write_masked_32be,
	program_write_qword_32be,
	program_write_qword_masked_32be
};

static const memory_handlers le_memory =
{
	program_read_byte_32le,
	program_read_word_32le,
	program_read_dword_32le,
	program_read_masked_32le,
	program_read_qword_32le,
	program_read_qword_masked_32le,

	program_write_byte_32le,
	program_write_word_32le,
	program_write_dword_32le,
	program_write_masked_32le,
	program_write_qword_32le,
	program_write_qword_masked_32le
};



/***************************************************************************
    INITIALIZATION AND SHUTDOWN
***************************************************************************/

/*-------------------------------------------------
    mips3com_init - initialize the mips3_state
    structure based on the configured type
-------------------------------------------------*/

void mips3com_init(mips3_state *mips, mips3_flavor flavor, int bigendian, int index, int clock, const struct mips3_config *config, int (*irqcallback)(int))
{
	/* initialize the state */
	memset(mips, 0, sizeof(*mips));

	/* initialize based on the config */
	mips->flavor = flavor;
	mips->bigendian = bigendian;
	mips->cpu_clock = clock;
	mips->irq_callback = irqcallback;
	mips->icache_size = config->icache;
	mips->dcache_size = config->dcache;
	mips->system_clock = config->system_clock;

	/* set up the endianness */
	if (mips->bigendian)
		mips->memory = be_memory;
	else
		mips->memory = le_memory;

	/* allocate memory */
	mips->icache = auto_malloc(config->icache);
	mips->dcache = auto_malloc(config->dcache);
	mips->tlb_table = auto_malloc(sizeof(mips->tlb_table[0]) * (1 << (MIPS3_MAX_PADDR_SHIFT - MIPS3_MIN_PAGE_SHIFT)));

	/* allocate a timer for the compare interrupt */
	mips->compare_int_timer = timer_alloc(compare_int_callback, NULL);

	/* reset the state */
	mips3com_reset(mips);
}


/*-------------------------------------------------
    mips3com_reset - reset the state of all the
    registers
-------------------------------------------------*/

void mips3com_reset(mips3_state *mips)
{
	/* initialize the state */
	mips->pc = 0xbfc00000;
	mips->cpr[0][COP0_Status] = SR_BEV | SR_ERL;
	mips->cpr[0][COP0_Compare] = 0xffffffff;
	mips->cpr[0][COP0_Count] = 0;
	mips->cpr[0][COP0_Config] = compute_config_register(mips);
	mips->cpr[0][COP0_PRId] = compute_prid_register(mips);
	mips->count_zero_time = activecpu_gettotalcycles64();

	/* recompute the TLB table */
	mips3com_recompute_tlb_table(mips);
}


/*-------------------------------------------------
    mips3com_dasm - handle disassembly for a
    CPU
-------------------------------------------------*/

#ifdef MAME_DEBUG
offs_t mips3com_dasm(mips3_state *mips, char *buffer, offs_t pc, const UINT8 *oprom, const UINT8 *opram)
{
	extern unsigned dasmmips3(char *, unsigned, UINT32);
	UINT32 op = *(UINT32 *)oprom;
	if (mips->bigendian)
		op = BIG_ENDIANIZE_INT32(op);
	else
		op = LITTLE_ENDIANIZE_INT32(op);
	return dasmmips3(buffer, pc, op);
}
#endif /* MAME_DEBUG */



/*-------------------------------------------------
    mips3com_update_cycle_counting - update cycle
    counts and the timers
-------------------------------------------------*/

void mips3com_update_cycle_counting(mips3_state *mips)
{
	/* modify the timer to go off */
	if ((mips->cpr[0][COP0_Status] & SR_IMEX5) && mips->cpr[0][COP0_Compare] != 0xffffffff)
	{
		UINT32 count = (activecpu_gettotalcycles64() - mips->count_zero_time) / 2;
		UINT32 compare = mips->cpr[0][COP0_Compare];
		UINT32 cyclesleft = compare - count;
		attotime newtime = ATTOTIME_IN_CYCLES(((INT64)cyclesleft * 2), cpu_getactivecpu());
		timer_adjust(mips->compare_int_timer, newtime, cpu_getactivecpu(), attotime_zero);
	}
	else
		timer_adjust(mips->compare_int_timer, attotime_never, cpu_getactivecpu(), attotime_zero);
}



/***************************************************************************
    TLB HANDLING
***************************************************************************/

/*-------------------------------------------------
    mips3com_map_tlb_entries - map entries from the
    TLB into the tlb_table
-------------------------------------------------*/

void mips3com_map_tlb_entries(mips3_state *mips)
{
	int valid_asid = mips->cpr[0][COP0_EntryHi] & 0xff;
	int index;

	/* iterate over all TLB entries */
	for (index = 0; index < ARRAY_LENGTH(mips->tlb); index++)
	{
		mips3_tlb_entry *tlbent = &mips->tlb[index];

		/* only process if global or if the ASID matches */
		if ((tlbent->entry_hi & 0x1000) || valid_asid == (tlbent->entry_hi & 0xff))
		{
			UINT32 count = (tlbent->page_mask >> 13) & 0x00fff;
			UINT32 vpn = ((tlbent->entry_hi >> 13) & 0x07ffffff) << 1;
			int which, i;

			/* ignore if the virtual address is beyond 32 bits */
			if (vpn < (1 << (MIPS3_MAX_PADDR_SHIFT - MIPS3_MIN_PAGE_SHIFT)))

				/* loop over both the even and odd pages */
				for (which = 0; which < 2; which++)
				{
					UINT64 lo = tlbent->entry_lo[which];

					/* only map if the TLB entry is valid */
					if (lo & 2)
					{
						UINT32 pfn = (lo >> 6) & 0x00ffffff;
						UINT32 wp = (~lo >> 2) & 1;

						for (i = 0; i <= count; i++, vpn++, pfn++)
							if (vpn < 0x80000 || vpn >= 0xc0000)
								mips->tlb_table[vpn] = (pfn << MIPS3_MIN_PAGE_SHIFT) | wp;
					}

					/* otherwise, advance by the number of pages we would have mapped */
					else
						vpn += count + 1;
				}
		}
	}
}


/*-------------------------------------------------
    mips3com_unmap_tlb_entries - unmap entries from
    the TLB into the tlb_table
-------------------------------------------------*/

void mips3com_unmap_tlb_entries(mips3_state *mips)
{
	int index;

	/* iterate over all TLB entries */
	for (index = 0; index < ARRAY_LENGTH(mips->tlb); index++)
	{
		mips3_tlb_entry *tlbent = &mips->tlb[index];
		UINT32 count = (tlbent->page_mask >> 13) & 0x00fff;
		UINT32 vpn = ((tlbent->entry_hi >> 13) & 0x07ffffff) << 1;
		int which, i;

		/* ignore if the virtual address is beyond 32 bits */
		if (vpn < (1 << (MIPS3_MAX_PADDR_SHIFT - MIPS3_MIN_PAGE_SHIFT)))

			/* loop over both the even and odd pages */
			for (which = 0; which < 2; which++)
				for (i = 0; i <= count; i++, vpn++)
					if (vpn < 0x80000 || vpn >= 0xc0000)
						mips->tlb_table[vpn] = 0xffffffff;
	}
}


/*-------------------------------------------------
    mips3com_recompute_tlb_table - recompute the TLB
    table from scratch
-------------------------------------------------*/

void mips3com_recompute_tlb_table(mips3_state *mips)
{
	UINT32 addr;

	/* map in the hard-coded spaces */
	for (addr = 0x80000000; addr < 0xc0000000; addr += MIPS3_MIN_PAGE_SIZE)
		mips->tlb_table[addr >> MIPS3_MIN_PAGE_SHIFT] = addr & 0x1ffff000;

	/* reset everything else to unmapped */
	memset(&mips->tlb_table[0x00000000 >> MIPS3_MIN_PAGE_SHIFT], 0xff, sizeof(mips->tlb_table[0]) * (0x80000000 >> MIPS3_MIN_PAGE_SHIFT));
	memset(&mips->tlb_table[0xc0000000 >> MIPS3_MIN_PAGE_SHIFT], 0xff, sizeof(mips->tlb_table[0]) * (0x40000000 >> MIPS3_MIN_PAGE_SHIFT));

	/* remap all the entries in the TLB */
	mips3com_map_tlb_entries(mips);
}


/*-------------------------------------------------
    mips3com_translate_address - translate an address
    from logical to physical
-------------------------------------------------*/

int mips3com_translate_address(mips3_state *mips, int space, offs_t *address)
{
	/* only applies to the program address space */
	if (space == ADDRESS_SPACE_PROGRAM)
	{
		UINT32 result = mips->tlb_table[*address >> MIPS3_MIN_PAGE_SHIFT];
		if (result == 0xffffffff)
			return FALSE;
		*address = (result & ~MIPS3_MIN_PAGE_MASK) | (*address & MIPS3_MIN_PAGE_MASK);
	}
	return TRUE;
}


/*-------------------------------------------------
    mips3com_tlbr - execute the tlbr instruction
-------------------------------------------------*/

void mips3com_tlbr(mips3_state *mips)
{
	UINT32 index = mips->cpr[0][COP0_Index] & 0x3f;

	/* only handle entries within the TLB */
	if (index < ARRAY_LENGTH(mips->tlb))
	{
		mips3_tlb_entry *tlbent = &mips->tlb[index];

		/* copy data from the TLB entry into the COP0 registers */
		mips->cpr[0][COP0_PageMask] = tlbent->page_mask;
		mips->cpr[0][COP0_EntryHi] = tlbent->entry_hi;
		mips->cpr[0][COP0_EntryLo0] = tlbent->entry_lo[0];
		mips->cpr[0][COP0_EntryLo1] = tlbent->entry_lo[1];
	}
}


/*-------------------------------------------------
    mips3com_tlbwi - execute the tlbwi instruction
-------------------------------------------------*/

void mips3com_tlbwi(mips3_state *mips)
{
	/* use the common handler and write based off the COP0 Index register */
	tlb_write_common(mips, mips->cpr[0][COP0_Index] & 0x3f);
}


/*-------------------------------------------------
    mips3com_tlbwr - execute the tlbwr instruction
-------------------------------------------------*/

void mips3com_tlbwr(mips3_state *mips)
{
	UINT32 wired = mips->cpr[0][COP0_Wired] & 0x3f;
	UINT32 unwired = ARRAY_LENGTH(mips->tlb) - wired;
	UINT32 index = ARRAY_LENGTH(mips->tlb) - 1;

	/* "random" is based off of the current cycle counting through the non-wired pages */
	if (unwired > 0)
		index = ((activecpu_gettotalcycles64() - mips->count_zero_time) % unwired + wired) & 0x3f;

	/* use the common handler to write to this index */
	tlb_write_common(mips, index);
}


/*-------------------------------------------------
    mips3com_tlbp - execute the tlbp instruction
-------------------------------------------------*/

void mips3com_tlbp(mips3_state *mips)
{
	UINT32 index;
	UINT64 vpn;

	/* iterate over TLB entries */
	for (index = 0; index < ARRAY_LENGTH(mips->tlb); index++)
	{
		mips3_tlb_entry *tlbent = &mips->tlb[index];
		UINT64 mask = ~((tlbent->page_mask >> 13) & 0xfff) << 13;

		/* if the relevant bits of EntryHi match the relevant bits of the TLB */
		if ((tlbent->entry_hi & mask) == (mips->cpr[0][COP0_EntryHi] & mask))

			/* and if we are either global or matching the current ASID, then stop */
			if ((tlbent->entry_hi & 0x1000) || (tlbent->entry_hi & 0xff) == (mips->cpr[0][COP0_EntryHi] & 0xff))
				break;
	}

	/* validate that our tlb_table was in sync */
	vpn = ((mips->cpr[0][COP0_EntryHi] >> 13) & 0x07ffffff) << 1;
	if (index != ARRAY_LENGTH(mips->tlb))
	{
		/* we can't assert this because the TLB entry may not be valid */
		/* assert(mips->tlb_table[vpn & 0xfffff] != 0xffffffff); */
		mips->cpr[0][COP0_Index] = index;
	}
	else
	{
		assert(mips->tlb_table[vpn & 0xfffff] == 0xffffffff);
		mips->cpr[0][COP0_Index] = 0x80000000;
	}
}



/***************************************************************************
    COMMON GET/SET INFO
***************************************************************************/

/*-------------------------------------------------
    mips3com_set_info - set information about
    a MIPS 3 CPU
-------------------------------------------------*/

void mips3com_set_info(mips3_state *mips, UINT32 state, cpuinfo *info)
{
	switch (state)
	{
		/* --- the following bits of info are set as 64-bit signed integers --- */
		case CPUINFO_INT_INPUT_STATE + MIPS3_IRQ0:		mips3com_set_irq_line(mips, MIPS3_IRQ0, info->i);	break;
		case CPUINFO_INT_INPUT_STATE + MIPS3_IRQ1:		mips3com_set_irq_line(mips, MIPS3_IRQ1, info->i);	break;
		case CPUINFO_INT_INPUT_STATE + MIPS3_IRQ2:		mips3com_set_irq_line(mips, MIPS3_IRQ2, info->i);	break;
		case CPUINFO_INT_INPUT_STATE + MIPS3_IRQ3:		mips3com_set_irq_line(mips, MIPS3_IRQ3, info->i);	break;
		case CPUINFO_INT_INPUT_STATE + MIPS3_IRQ4:		mips3com_set_irq_line(mips, MIPS3_IRQ4, info->i);	break;
		case CPUINFO_INT_INPUT_STATE + MIPS3_IRQ5:		mips3com_set_irq_line(mips, MIPS3_IRQ5, info->i);	break;

		case CPUINFO_INT_PC:
		case CPUINFO_INT_REGISTER + MIPS3_PC:			mips->pc = info->i;						break;
		case CPUINFO_INT_REGISTER + MIPS3_SR:			mips->cpr[0][COP0_Status] = info->i; 	break;
		case CPUINFO_INT_REGISTER + MIPS3_EPC:			mips->cpr[0][COP0_EPC] = info->i; 		break;
		case CPUINFO_INT_REGISTER + MIPS3_CAUSE:		mips->cpr[0][COP0_Cause] = info->i;		break;
		case CPUINFO_INT_REGISTER + MIPS3_COUNT:		mips->cpr[0][COP0_Count] = info->i; 	break;
		case CPUINFO_INT_REGISTER + MIPS3_COMPARE:		mips->cpr[0][COP0_Compare] = info->i; 	break;
		case CPUINFO_INT_REGISTER + MIPS3_INDEX:		mips->cpr[0][COP0_Index] = info->i; 	break;
		case CPUINFO_INT_REGISTER + MIPS3_RANDOM:		mips->cpr[0][COP0_Random] = info->i; 	break;
		case CPUINFO_INT_REGISTER + MIPS3_ENTRYHI:		mips->cpr[0][COP0_EntryHi] = info->i; 	break;
		case CPUINFO_INT_REGISTER + MIPS3_ENTRYLO0:		mips->cpr[0][COP0_EntryLo0] = info->i; 	break;
		case CPUINFO_INT_REGISTER + MIPS3_ENTRYLO1:		mips->cpr[0][COP0_EntryLo1] = info->i; 	break;
		case CPUINFO_INT_REGISTER + MIPS3_PAGEMASK:		mips->cpr[0][COP0_PageMask] = info->i; 	break;
		case CPUINFO_INT_REGISTER + MIPS3_WIRED:		mips->cpr[0][COP0_Wired] = info->i; 	break;
		case CPUINFO_INT_REGISTER + MIPS3_BADVADDR:		mips->cpr[0][COP0_BadVAddr] = info->i; 	break;

		case CPUINFO_INT_REGISTER + MIPS3_R0:			/* can't change R0 */					break;
		case CPUINFO_INT_REGISTER + MIPS3_R1:			mips->r[1] = info->i;					break;
		case CPUINFO_INT_REGISTER + MIPS3_R2:			mips->r[2] = info->i;					break;
		case CPUINFO_INT_REGISTER + MIPS3_R3:			mips->r[3] = info->i;					break;
		case CPUINFO_INT_REGISTER + MIPS3_R4:			mips->r[4] = info->i;					break;
		case CPUINFO_INT_REGISTER + MIPS3_R5:			mips->r[5] = info->i;					break;
		case CPUINFO_INT_REGISTER + MIPS3_R6:			mips->r[6] = info->i;					break;
		case CPUINFO_INT_REGISTER + MIPS3_R7:			mips->r[7] = info->i;					break;
		case CPUINFO_INT_REGISTER + MIPS3_R8:			mips->r[8] = info->i;					break;
		case CPUINFO_INT_REGISTER + MIPS3_R9:			mips->r[9] = info->i;					break;
		case CPUINFO_INT_REGISTER + MIPS3_R10:			mips->r[10] = info->i;					break;
		case CPUINFO_INT_REGISTER + MIPS3_R11:			mips->r[11] = info->i;					break;
		case CPUINFO_INT_REGISTER + MIPS3_R12:			mips->r[12] = info->i;					break;
		case CPUINFO_INT_REGISTER + MIPS3_R13:			mips->r[13] = info->i;					break;
		case CPUINFO_INT_REGISTER + MIPS3_R14:			mips->r[14] = info->i;					break;
		case CPUINFO_INT_REGISTER + MIPS3_R15:			mips->r[15] = info->i;					break;
		case CPUINFO_INT_REGISTER + MIPS3_R16:			mips->r[16] = info->i;					break;
		case CPUINFO_INT_REGISTER + MIPS3_R17:			mips->r[17] = info->i;					break;
		case CPUINFO_INT_REGISTER + MIPS3_R18:			mips->r[18] = info->i;					break;
		case CPUINFO_INT_REGISTER + MIPS3_R19:			mips->r[19] = info->i;					break;
		case CPUINFO_INT_REGISTER + MIPS3_R20:			mips->r[20] = info->i;					break;
		case CPUINFO_INT_REGISTER + MIPS3_R21:			mips->r[21] = info->i;					break;
		case CPUINFO_INT_REGISTER + MIPS3_R22:			mips->r[22] = info->i;					break;
		case CPUINFO_INT_REGISTER + MIPS3_R23:			mips->r[23] = info->i;					break;
		case CPUINFO_INT_REGISTER + MIPS3_R24:			mips->r[24] = info->i;					break;
		case CPUINFO_INT_REGISTER + MIPS3_R25:			mips->r[25] = info->i;					break;
		case CPUINFO_INT_REGISTER + MIPS3_R26:			mips->r[26] = info->i;					break;
		case CPUINFO_INT_REGISTER + MIPS3_R27:			mips->r[27] = info->i;					break;
		case CPUINFO_INT_REGISTER + MIPS3_R28:			mips->r[28] = info->i;					break;
		case CPUINFO_INT_REGISTER + MIPS3_R29:			mips->r[29] = info->i;					break;
		case CPUINFO_INT_REGISTER + MIPS3_R30:			mips->r[30] = info->i;					break;
		case CPUINFO_INT_SP:
		case CPUINFO_INT_REGISTER + MIPS3_R31:			mips->r[31] = info->i;					break;
		case CPUINFO_INT_REGISTER + MIPS3_HI:			mips->r[REG_HI] = info->i;				break;
		case CPUINFO_INT_REGISTER + MIPS3_LO:			mips->r[REG_LO] = info->i;				break;

		case CPUINFO_INT_REGISTER + MIPS3_FPR0:			mips->cpr[1][0] = info->i;				break;
		case CPUINFO_INT_REGISTER + MIPS3_FPR1:			mips->cpr[1][1] = info->i;				break;
		case CPUINFO_INT_REGISTER + MIPS3_FPR2:			mips->cpr[1][2] = info->i;				break;
		case CPUINFO_INT_REGISTER + MIPS3_FPR3:			mips->cpr[1][3] = info->i;				break;
		case CPUINFO_INT_REGISTER + MIPS3_FPR4:			mips->cpr[1][4] = info->i;				break;
		case CPUINFO_INT_REGISTER + MIPS3_FPR5:			mips->cpr[1][5] = info->i;				break;
		case CPUINFO_INT_REGISTER + MIPS3_FPR6:			mips->cpr[1][6] = info->i;				break;
		case CPUINFO_INT_REGISTER + MIPS3_FPR7:			mips->cpr[1][7] = info->i;				break;
		case CPUINFO_INT_REGISTER + MIPS3_FPR8:			mips->cpr[1][8] = info->i;				break;
		case CPUINFO_INT_REGISTER + MIPS3_FPR9:			mips->cpr[1][9] = info->i;				break;
		case CPUINFO_INT_REGISTER + MIPS3_FPR10:		mips->cpr[1][10] = info->i;				break;
		case CPUINFO_INT_REGISTER + MIPS3_FPR11:		mips->cpr[1][11] = info->i;				break;
		case CPUINFO_INT_REGISTER + MIPS3_FPR12:		mips->cpr[1][12] = info->i;				break;
		case CPUINFO_INT_REGISTER + MIPS3_FPR13:		mips->cpr[1][13] = info->i;				break;
		case CPUINFO_INT_REGISTER + MIPS3_FPR14:		mips->cpr[1][14] = info->i;				break;
		case CPUINFO_INT_REGISTER + MIPS3_FPR15:		mips->cpr[1][15] = info->i;				break;
		case CPUINFO_INT_REGISTER + MIPS3_FPR16:		mips->cpr[1][16] = info->i;				break;
		case CPUINFO_INT_REGISTER + MIPS3_FPR17:		mips->cpr[1][17] = info->i;				break;
		case CPUINFO_INT_REGISTER + MIPS3_FPR18:		mips->cpr[1][18] = info->i;				break;
		case CPUINFO_INT_REGISTER + MIPS3_FPR19:		mips->cpr[1][19] = info->i;				break;
		case CPUINFO_INT_REGISTER + MIPS3_FPR20:		mips->cpr[1][20] = info->i;				break;
		case CPUINFO_INT_REGISTER + MIPS3_FPR21:		mips->cpr[1][21] = info->i;				break;
		case CPUINFO_INT_REGISTER + MIPS3_FPR22:		mips->cpr[1][22] = info->i;				break;
		case CPUINFO_INT_REGISTER + MIPS3_FPR23:		mips->cpr[1][23] = info->i;				break;
		case CPUINFO_INT_REGISTER + MIPS3_FPR24:		mips->cpr[1][24] = info->i;				break;
		case CPUINFO_INT_REGISTER + MIPS3_FPR25:		mips->cpr[1][25] = info->i;				break;
		case CPUINFO_INT_REGISTER + MIPS3_FPR26:		mips->cpr[1][26] = info->i;				break;
		case CPUINFO_INT_REGISTER + MIPS3_FPR27:		mips->cpr[1][27] = info->i;				break;
		case CPUINFO_INT_REGISTER + MIPS3_FPR28:		mips->cpr[1][28] = info->i;				break;
		case CPUINFO_INT_REGISTER + MIPS3_FPR29:		mips->cpr[1][29] = info->i;				break;
		case CPUINFO_INT_REGISTER + MIPS3_FPR30:		mips->cpr[1][30] = info->i;				break;
		case CPUINFO_INT_REGISTER + MIPS3_FPR31:		mips->cpr[1][31] = info->i;				break;
	}
}


/*-------------------------------------------------
    mips3com_get_info - get information about
    a MIPS 3 CPU
-------------------------------------------------*/

void mips3com_get_info(mips3_state *mips, UINT32 state, cpuinfo *info)
{
	switch (state)
	{
		/* --- the following bits of info are returned as 64-bit signed integers --- */
		case CPUINFO_INT_CONTEXT_SIZE:					/* provided by core */					break;
		case CPUINFO_INT_INPUT_LINES:					info->i = 6;							break;
		case CPUINFO_INT_DEFAULT_IRQ_VECTOR:			info->i = 0;							break;
		case CPUINFO_INT_ENDIANNESS:					info->i = mips->bigendian ? CPU_IS_BE : CPU_IS_LE; break;
		case CPUINFO_INT_CLOCK_DIVIDER:					info->i = 1;							break;
		case CPUINFO_INT_MIN_INSTRUCTION_BYTES:			info->i = 4;							break;
		case CPUINFO_INT_MAX_INSTRUCTION_BYTES:			info->i = 4;							break;
		case CPUINFO_INT_MIN_CYCLES:					info->i = 1;							break;
		case CPUINFO_INT_MAX_CYCLES:					info->i = 40;							break;

		case CPUINFO_INT_DATABUS_WIDTH + ADDRESS_SPACE_PROGRAM:	info->i = MIPS3_MAX_PADDR_SHIFT;break;
		case CPUINFO_INT_ADDRBUS_WIDTH + ADDRESS_SPACE_PROGRAM: info->i = 32;					break;
		case CPUINFO_INT_ADDRBUS_SHIFT + ADDRESS_SPACE_PROGRAM: info->i = 0;					break;
		case CPUINFO_INT_LOGADDR_WIDTH + ADDRESS_SPACE_PROGRAM: info->i = 32;					break;
		case CPUINFO_INT_PAGE_SHIFT + ADDRESS_SPACE_PROGRAM: 	info->i = MIPS3_MIN_PAGE_SHIFT;	break;

		case CPUINFO_INT_INPUT_STATE + MIPS3_IRQ0:		info->i = (mips->cpr[0][COP0_Cause] & 0x400) ? ASSERT_LINE : CLEAR_LINE;	break;
		case CPUINFO_INT_INPUT_STATE + MIPS3_IRQ1:		info->i = (mips->cpr[0][COP0_Cause] & 0x800) ? ASSERT_LINE : CLEAR_LINE;	break;
		case CPUINFO_INT_INPUT_STATE + MIPS3_IRQ2:		info->i = (mips->cpr[0][COP0_Cause] & 0x1000) ? ASSERT_LINE : CLEAR_LINE;	break;
		case CPUINFO_INT_INPUT_STATE + MIPS3_IRQ3:		info->i = (mips->cpr[0][COP0_Cause] & 0x2000) ? ASSERT_LINE : CLEAR_LINE;	break;
		case CPUINFO_INT_INPUT_STATE + MIPS3_IRQ4:		info->i = (mips->cpr[0][COP0_Cause] & 0x4000) ? ASSERT_LINE : CLEAR_LINE;	break;
		case CPUINFO_INT_INPUT_STATE + MIPS3_IRQ5:		info->i = (mips->cpr[0][COP0_Cause] & 0x8000) ? ASSERT_LINE : CLEAR_LINE;	break;

		case CPUINFO_INT_PREVIOUSPC:					/* optionally implemented */			break;

		case CPUINFO_INT_PC:
		case CPUINFO_INT_REGISTER + MIPS3_PC:			info->i = mips->pc;						break;
		case CPUINFO_INT_REGISTER + MIPS3_SR:			info->i = mips->cpr[0][COP0_Status];	break;
		case CPUINFO_INT_REGISTER + MIPS3_EPC:			info->i = mips->cpr[0][COP0_EPC];		break;
		case CPUINFO_INT_REGISTER + MIPS3_CAUSE:		info->i = mips->cpr[0][COP0_Cause];		break;
		case CPUINFO_INT_REGISTER + MIPS3_COUNT:		info->i = ((activecpu_gettotalcycles64() - mips->count_zero_time) / 2); break;
		case CPUINFO_INT_REGISTER + MIPS3_COMPARE:		info->i = mips->cpr[0][COP0_Compare];	break;
		case CPUINFO_INT_REGISTER + MIPS3_INDEX:		info->i = mips->cpr[0][COP0_Index];		break;
		case CPUINFO_INT_REGISTER + MIPS3_RANDOM:		info->i = mips->cpr[0][COP0_Random];	break;
		case CPUINFO_INT_REGISTER + MIPS3_ENTRYHI:		info->i = mips->cpr[0][COP0_EntryHi];	break;
		case CPUINFO_INT_REGISTER + MIPS3_ENTRYLO0:		info->i = mips->cpr[0][COP0_EntryLo0];	break;
		case CPUINFO_INT_REGISTER + MIPS3_ENTRYLO1:		info->i = mips->cpr[0][COP0_EntryLo1];	break;
		case CPUINFO_INT_REGISTER + MIPS3_PAGEMASK:		info->i = mips->cpr[0][COP0_PageMask];	break;
		case CPUINFO_INT_REGISTER + MIPS3_WIRED:		info->i = mips->cpr[0][COP0_Wired];		break;
		case CPUINFO_INT_REGISTER + MIPS3_BADVADDR:		info->i = mips->cpr[0][COP0_BadVAddr];	break;

		case CPUINFO_INT_REGISTER + MIPS3_R0:			info->i = mips->r[0];					break;
		case CPUINFO_INT_REGISTER + MIPS3_R1:			info->i = mips->r[1];					break;
		case CPUINFO_INT_REGISTER + MIPS3_R2:			info->i = mips->r[2];					break;
		case CPUINFO_INT_REGISTER + MIPS3_R3:			info->i = mips->r[3];					break;
		case CPUINFO_INT_REGISTER + MIPS3_R4:			info->i = mips->r[4];					break;
		case CPUINFO_INT_REGISTER + MIPS3_R5:			info->i = mips->r[5];					break;
		case CPUINFO_INT_REGISTER + MIPS3_R6:			info->i = mips->r[6];					break;
		case CPUINFO_INT_REGISTER + MIPS3_R7:			info->i = mips->r[7];					break;
		case CPUINFO_INT_REGISTER + MIPS3_R8:			info->i = mips->r[8];					break;
		case CPUINFO_INT_REGISTER + MIPS3_R9:			info->i = mips->r[9];					break;
		case CPUINFO_INT_REGISTER + MIPS3_R10:			info->i = mips->r[10];					break;
		case CPUINFO_INT_REGISTER + MIPS3_R11:			info->i = mips->r[11];					break;
		case CPUINFO_INT_REGISTER + MIPS3_R12:			info->i = mips->r[12];					break;
		case CPUINFO_INT_REGISTER + MIPS3_R13:			info->i = mips->r[13];					break;
		case CPUINFO_INT_REGISTER + MIPS3_R14:			info->i = mips->r[14];					break;
		case CPUINFO_INT_REGISTER + MIPS3_R15:			info->i = mips->r[15];					break;
		case CPUINFO_INT_REGISTER + MIPS3_R16:			info->i = mips->r[16];					break;
		case CPUINFO_INT_REGISTER + MIPS3_R17:			info->i = mips->r[17];					break;
		case CPUINFO_INT_REGISTER + MIPS3_R18:			info->i = mips->r[18];					break;
		case CPUINFO_INT_REGISTER + MIPS3_R19:			info->i = mips->r[19];					break;
		case CPUINFO_INT_REGISTER + MIPS3_R20:			info->i = mips->r[20];					break;
		case CPUINFO_INT_REGISTER + MIPS3_R21:			info->i = mips->r[21];					break;
		case CPUINFO_INT_REGISTER + MIPS3_R22:			info->i = mips->r[22];					break;
		case CPUINFO_INT_REGISTER + MIPS3_R23:			info->i = mips->r[23];					break;
		case CPUINFO_INT_REGISTER + MIPS3_R24:			info->i = mips->r[24];					break;
		case CPUINFO_INT_REGISTER + MIPS3_R25:			info->i = mips->r[25];					break;
		case CPUINFO_INT_REGISTER + MIPS3_R26:			info->i = mips->r[26];					break;
		case CPUINFO_INT_REGISTER + MIPS3_R27:			info->i = mips->r[27];					break;
		case CPUINFO_INT_REGISTER + MIPS3_R28:			info->i = mips->r[28];					break;
		case CPUINFO_INT_REGISTER + MIPS3_R29:			info->i = mips->r[29];					break;
		case CPUINFO_INT_REGISTER + MIPS3_R30:			info->i = mips->r[30];					break;
		case CPUINFO_INT_SP:
		case CPUINFO_INT_REGISTER + MIPS3_R31:			info->i = mips->r[31];					break;
		case CPUINFO_INT_REGISTER + MIPS3_HI:			info->i = mips->r[REG_HI];				break;
		case CPUINFO_INT_REGISTER + MIPS3_LO:			info->i = mips->r[REG_LO];				break;

		/* --- the following bits of info are returned as pointers to data or functions --- */
		case CPUINFO_PTR_SET_INFO:						/* provided by core */					break;
		case CPUINFO_PTR_GET_CONTEXT:					/* provided by core */					break;
		case CPUINFO_PTR_SET_CONTEXT:					/* provided by core */					break;
		case CPUINFO_PTR_INIT:							/* provided by core */					break;
		case CPUINFO_PTR_RESET:							/* provided by core */					break;
		case CPUINFO_PTR_EXIT:							/* provided by core */					break;
		case CPUINFO_PTR_EXECUTE:						/* provided by core */					break;
		case CPUINFO_PTR_TRANSLATE:						/* provided by core */					break;
		case CPUINFO_PTR_DISASSEMBLE:					/* provided by core */					break;
		case CPUINFO_PTR_INSTRUCTION_COUNTER:			info->icount = &mips->icount;			break;

		/* --- the following bits of info are returned as NULL-terminated strings --- */
		case CPUINFO_STR_NAME:							strcpy(info->s, "MIPS III");			break;
		case CPUINFO_STR_CORE_FAMILY:					strcpy(info->s, "MIPS III");			break;
		case CPUINFO_STR_CORE_VERSION:					strcpy(info->s, "3.0");					break;
		case CPUINFO_STR_CORE_FILE:						/* provided by core */					break;
		case CPUINFO_STR_CORE_CREDITS:					strcpy(info->s, "Copyright Aaron Giles"); break;

		case CPUINFO_STR_FLAGS:							strcpy(info->s, " ");					break;

		case CPUINFO_STR_REGISTER + MIPS3_PC:			sprintf(info->s, "PC: %08X", mips->pc); break;
		case CPUINFO_STR_REGISTER + MIPS3_SR:			sprintf(info->s, "SR: %08X", (UINT32)mips->cpr[0][COP0_Status]); break;
		case CPUINFO_STR_REGISTER + MIPS3_EPC:			sprintf(info->s, "EPC:%08X", (UINT32)mips->cpr[0][COP0_EPC]); break;
		case CPUINFO_STR_REGISTER + MIPS3_CAUSE: 		sprintf(info->s, "Cause:%08X", (UINT32)mips->cpr[0][COP0_Cause]); break;
		case CPUINFO_STR_REGISTER + MIPS3_COUNT: 		sprintf(info->s, "Count:%08X", (UINT32)((activecpu_gettotalcycles64() - mips->count_zero_time) / 2)); break;
		case CPUINFO_STR_REGISTER + MIPS3_COMPARE:		sprintf(info->s, "Compare:%08X", (UINT32)mips->cpr[0][COP0_Compare]); break;
		case CPUINFO_STR_REGISTER + MIPS3_INDEX:		sprintf(info->s, "Index:%08X", (UINT32)mips->cpr[0][COP0_Index]); break;
		case CPUINFO_STR_REGISTER + MIPS3_RANDOM:		sprintf(info->s, "Random:%08X", (UINT32)mips->cpr[0][COP0_Random]); break;
		case CPUINFO_STR_REGISTER + MIPS3_ENTRYHI:		sprintf(info->s, "EntryHi:%08X%08X", (UINT32)(mips->cpr[0][COP0_EntryHi] >> 32), (UINT32)mips->cpr[0][COP0_EntryHi]); break;
		case CPUINFO_STR_REGISTER + MIPS3_ENTRYLO0:		sprintf(info->s, "EntryLo0:%08X%08X", (UINT32)(mips->cpr[0][COP0_EntryLo0] >> 32), (UINT32)mips->cpr[0][COP0_EntryLo0]); break;
		case CPUINFO_STR_REGISTER + MIPS3_ENTRYLO1:		sprintf(info->s, "EntryLo1:%08X%08X", (UINT32)(mips->cpr[0][COP0_EntryLo1] >> 32), (UINT32)mips->cpr[0][COP0_EntryLo1]); break;
		case CPUINFO_STR_REGISTER + MIPS3_PAGEMASK:		sprintf(info->s, "PageMask:%08X%08X", (UINT32)(mips->cpr[0][COP0_PageMask] >> 32), (UINT32)mips->cpr[0][COP0_PageMask]); break;
		case CPUINFO_STR_REGISTER + MIPS3_WIRED:		sprintf(info->s, "Wired:%08X", (UINT32)mips->cpr[0][COP0_Wired]); break;
		case CPUINFO_STR_REGISTER + MIPS3_BADVADDR:		sprintf(info->s, "BadVAddr:%08X", (UINT32)mips->cpr[0][COP0_BadVAddr]); break;

		case CPUINFO_STR_REGISTER + MIPS3_R0:			sprintf(info->s, "R0: %08X%08X", (UINT32)(mips->r[0] >> 32), (UINT32)mips->r[0]); break;
		case CPUINFO_STR_REGISTER + MIPS3_R1:			sprintf(info->s, "R1: %08X%08X", (UINT32)(mips->r[1] >> 32), (UINT32)mips->r[1]); break;
		case CPUINFO_STR_REGISTER + MIPS3_R2:			sprintf(info->s, "R2: %08X%08X", (UINT32)(mips->r[2] >> 32), (UINT32)mips->r[2]); break;
		case CPUINFO_STR_REGISTER + MIPS3_R3:			sprintf(info->s, "R3: %08X%08X", (UINT32)(mips->r[3] >> 32), (UINT32)mips->r[3]); break;
		case CPUINFO_STR_REGISTER + MIPS3_R4:			sprintf(info->s, "R4: %08X%08X", (UINT32)(mips->r[4] >> 32), (UINT32)mips->r[4]); break;
		case CPUINFO_STR_REGISTER + MIPS3_R5:			sprintf(info->s, "R5: %08X%08X", (UINT32)(mips->r[5] >> 32), (UINT32)mips->r[5]); break;
		case CPUINFO_STR_REGISTER + MIPS3_R6:			sprintf(info->s, "R6: %08X%08X", (UINT32)(mips->r[6] >> 32), (UINT32)mips->r[6]); break;
		case CPUINFO_STR_REGISTER + MIPS3_R7:			sprintf(info->s, "R7: %08X%08X", (UINT32)(mips->r[7] >> 32), (UINT32)mips->r[7]); break;
		case CPUINFO_STR_REGISTER + MIPS3_R8:			sprintf(info->s, "R8: %08X%08X", (UINT32)(mips->r[8] >> 32), (UINT32)mips->r[8]); break;
		case CPUINFO_STR_REGISTER + MIPS3_R9:			sprintf(info->s, "R9: %08X%08X", (UINT32)(mips->r[9] >> 32), (UINT32)mips->r[9]); break;
		case CPUINFO_STR_REGISTER + MIPS3_R10:			sprintf(info->s, "R10:%08X%08X", (UINT32)(mips->r[10] >> 32), (UINT32)mips->r[10]); break;
		case CPUINFO_STR_REGISTER + MIPS3_R11:			sprintf(info->s, "R11:%08X%08X", (UINT32)(mips->r[11] >> 32), (UINT32)mips->r[11]); break;
		case CPUINFO_STR_REGISTER + MIPS3_R12:			sprintf(info->s, "R12:%08X%08X", (UINT32)(mips->r[12] >> 32), (UINT32)mips->r[12]); break;
		case CPUINFO_STR_REGISTER + MIPS3_R13:			sprintf(info->s, "R13:%08X%08X", (UINT32)(mips->r[13] >> 32), (UINT32)mips->r[13]); break;
		case CPUINFO_STR_REGISTER + MIPS3_R14:			sprintf(info->s, "R14:%08X%08X", (UINT32)(mips->r[14] >> 32), (UINT32)mips->r[14]); break;
		case CPUINFO_STR_REGISTER + MIPS3_R15:			sprintf(info->s, "R15:%08X%08X", (UINT32)(mips->r[15] >> 32), (UINT32)mips->r[15]); break;
		case CPUINFO_STR_REGISTER + MIPS3_R16:			sprintf(info->s, "R16:%08X%08X", (UINT32)(mips->r[16] >> 32), (UINT32)mips->r[16]); break;
		case CPUINFO_STR_REGISTER + MIPS3_R17:			sprintf(info->s, "R17:%08X%08X", (UINT32)(mips->r[17] >> 32), (UINT32)mips->r[17]); break;
		case CPUINFO_STR_REGISTER + MIPS3_R18:			sprintf(info->s, "R18:%08X%08X", (UINT32)(mips->r[18] >> 32), (UINT32)mips->r[18]); break;
		case CPUINFO_STR_REGISTER + MIPS3_R19:			sprintf(info->s, "R19:%08X%08X", (UINT32)(mips->r[19] >> 32), (UINT32)mips->r[19]); break;
		case CPUINFO_STR_REGISTER + MIPS3_R20:			sprintf(info->s, "R20:%08X%08X", (UINT32)(mips->r[20] >> 32), (UINT32)mips->r[20]); break;
		case CPUINFO_STR_REGISTER + MIPS3_R21:			sprintf(info->s, "R21:%08X%08X", (UINT32)(mips->r[21] >> 32), (UINT32)mips->r[21]); break;
		case CPUINFO_STR_REGISTER + MIPS3_R22:			sprintf(info->s, "R22:%08X%08X", (UINT32)(mips->r[22] >> 32), (UINT32)mips->r[22]); break;
		case CPUINFO_STR_REGISTER + MIPS3_R23:			sprintf(info->s, "R23:%08X%08X", (UINT32)(mips->r[23] >> 32), (UINT32)mips->r[23]); break;
		case CPUINFO_STR_REGISTER + MIPS3_R24:			sprintf(info->s, "R24:%08X%08X", (UINT32)(mips->r[24] >> 32), (UINT32)mips->r[24]); break;
		case CPUINFO_STR_REGISTER + MIPS3_R25:			sprintf(info->s, "R25:%08X%08X", (UINT32)(mips->r[25] >> 32), (UINT32)mips->r[25]); break;
		case CPUINFO_STR_REGISTER + MIPS3_R26:			sprintf(info->s, "R26:%08X%08X", (UINT32)(mips->r[26] >> 32), (UINT32)mips->r[26]); break;
		case CPUINFO_STR_REGISTER + MIPS3_R27:			sprintf(info->s, "R27:%08X%08X", (UINT32)(mips->r[27] >> 32), (UINT32)mips->r[27]); break;
		case CPUINFO_STR_REGISTER + MIPS3_R28:			sprintf(info->s, "R28:%08X%08X", (UINT32)(mips->r[28] >> 32), (UINT32)mips->r[28]); break;
		case CPUINFO_STR_REGISTER + MIPS3_R29:			sprintf(info->s, "R29:%08X%08X", (UINT32)(mips->r[29] >> 32), (UINT32)mips->r[29]); break;
		case CPUINFO_STR_REGISTER + MIPS3_R30:			sprintf(info->s, "R30:%08X%08X", (UINT32)(mips->r[30] >> 32), (UINT32)mips->r[30]); break;
		case CPUINFO_STR_REGISTER + MIPS3_R31:			sprintf(info->s, "R31:%08X%08X", (UINT32)(mips->r[31] >> 32), (UINT32)mips->r[31]); break;
		case CPUINFO_STR_REGISTER + MIPS3_HI:			sprintf(info->s, "HI: %08X%08X", (UINT32)(mips->r[REG_HI] >> 32), (UINT32)mips->r[REG_HI]); break;
		case CPUINFO_STR_REGISTER + MIPS3_LO:			sprintf(info->s, "LO: %08X%08X", (UINT32)(mips->r[REG_LO] >> 32), (UINT32)mips->r[REG_LO]); break;

		case CPUINFO_STR_REGISTER + MIPS3_FPR0:			sprintf(info->s, "FPR0: %08X%08X", (UINT32)(mips->cpr[1][0] >> 32), (UINT32)mips->cpr[1][0]); break;
		case CPUINFO_STR_REGISTER + MIPS3_FPR1:			sprintf(info->s, "FPR1: %08X%08X", (UINT32)(mips->cpr[1][1] >> 32), (UINT32)mips->cpr[1][1]); break;
		case CPUINFO_STR_REGISTER + MIPS3_FPR2:			sprintf(info->s, "FPR2: %08X%08X", (UINT32)(mips->cpr[1][2] >> 32), (UINT32)mips->cpr[1][2]); break;
		case CPUINFO_STR_REGISTER + MIPS3_FPR3:			sprintf(info->s, "FPR3: %08X%08X", (UINT32)(mips->cpr[1][3] >> 32), (UINT32)mips->cpr[1][3]); break;
		case CPUINFO_STR_REGISTER + MIPS3_FPR4:			sprintf(info->s, "FPR4: %08X%08X", (UINT32)(mips->cpr[1][4] >> 32), (UINT32)mips->cpr[1][4]); break;
		case CPUINFO_STR_REGISTER + MIPS3_FPR5:			sprintf(info->s, "FPR5: %08X%08X", (UINT32)(mips->cpr[1][5] >> 32), (UINT32)mips->cpr[1][5]); break;
		case CPUINFO_STR_REGISTER + MIPS3_FPR6:			sprintf(info->s, "FPR6: %08X%08X", (UINT32)(mips->cpr[1][6] >> 32), (UINT32)mips->cpr[1][6]); break;
		case CPUINFO_STR_REGISTER + MIPS3_FPR7:			sprintf(info->s, "FPR7: %08X%08X", (UINT32)(mips->cpr[1][7] >> 32), (UINT32)mips->cpr[1][7]); break;
		case CPUINFO_STR_REGISTER + MIPS3_FPR8:			sprintf(info->s, "FPR8: %08X%08X", (UINT32)(mips->cpr[1][8] >> 32), (UINT32)mips->cpr[1][8]); break;
		case CPUINFO_STR_REGISTER + MIPS3_FPR9:			sprintf(info->s, "FPR9: %08X%08X", (UINT32)(mips->cpr[1][9] >> 32), (UINT32)mips->cpr[1][9]); break;
		case CPUINFO_STR_REGISTER + MIPS3_FPR10:		sprintf(info->s, "FPR10:%08X%08X", (UINT32)(mips->cpr[1][10] >> 32), (UINT32)mips->cpr[1][10]); break;
		case CPUINFO_STR_REGISTER + MIPS3_FPR11:		sprintf(info->s, "FPR11:%08X%08X", (UINT32)(mips->cpr[1][11] >> 32), (UINT32)mips->cpr[1][11]); break;
		case CPUINFO_STR_REGISTER + MIPS3_FPR12:		sprintf(info->s, "FPR12:%08X%08X", (UINT32)(mips->cpr[1][12] >> 32), (UINT32)mips->cpr[1][12]); break;
		case CPUINFO_STR_REGISTER + MIPS3_FPR13:		sprintf(info->s, "FPR13:%08X%08X", (UINT32)(mips->cpr[1][13] >> 32), (UINT32)mips->cpr[1][13]); break;
		case CPUINFO_STR_REGISTER + MIPS3_FPR14:		sprintf(info->s, "FPR14:%08X%08X", (UINT32)(mips->cpr[1][14] >> 32), (UINT32)mips->cpr[1][14]); break;
		case CPUINFO_STR_REGISTER + MIPS3_FPR15:		sprintf(info->s, "FPR15:%08X%08X", (UINT32)(mips->cpr[1][15] >> 32), (UINT32)mips->cpr[1][15]); break;
		case CPUINFO_STR_REGISTER + MIPS3_FPR16:		sprintf(info->s, "FPR16:%08X%08X", (UINT32)(mips->cpr[1][16] >> 32), (UINT32)mips->cpr[1][16]); break;
		case CPUINFO_STR_REGISTER + MIPS3_FPR17:		sprintf(info->s, "FPR17:%08X%08X", (UINT32)(mips->cpr[1][17] >> 32), (UINT32)mips->cpr[1][17]); break;
		case CPUINFO_STR_REGISTER + MIPS3_FPR18:		sprintf(info->s, "FPR18:%08X%08X", (UINT32)(mips->cpr[1][18] >> 32), (UINT32)mips->cpr[1][18]); break;
		case CPUINFO_STR_REGISTER + MIPS3_FPR19:		sprintf(info->s, "FPR19:%08X%08X", (UINT32)(mips->cpr[1][19] >> 32), (UINT32)mips->cpr[1][19]); break;
		case CPUINFO_STR_REGISTER + MIPS3_FPR20:		sprintf(info->s, "FPR20:%08X%08X", (UINT32)(mips->cpr[1][20] >> 32), (UINT32)mips->cpr[1][20]); break;
		case CPUINFO_STR_REGISTER + MIPS3_FPR21:		sprintf(info->s, "FPR21:%08X%08X", (UINT32)(mips->cpr[1][21] >> 32), (UINT32)mips->cpr[1][21]); break;
		case CPUINFO_STR_REGISTER + MIPS3_FPR22:		sprintf(info->s, "FPR22:%08X%08X", (UINT32)(mips->cpr[1][22] >> 32), (UINT32)mips->cpr[1][22]); break;
		case CPUINFO_STR_REGISTER + MIPS3_FPR23:		sprintf(info->s, "FPR23:%08X%08X", (UINT32)(mips->cpr[1][23] >> 32), (UINT32)mips->cpr[1][23]); break;
		case CPUINFO_STR_REGISTER + MIPS3_FPR24:		sprintf(info->s, "FPR24:%08X%08X", (UINT32)(mips->cpr[1][24] >> 32), (UINT32)mips->cpr[1][24]); break;
		case CPUINFO_STR_REGISTER + MIPS3_FPR25:		sprintf(info->s, "FPR25:%08X%08X", (UINT32)(mips->cpr[1][25] >> 32), (UINT32)mips->cpr[1][25]); break;
		case CPUINFO_STR_REGISTER + MIPS3_FPR26:		sprintf(info->s, "FPR26:%08X%08X", (UINT32)(mips->cpr[1][26] >> 32), (UINT32)mips->cpr[1][26]); break;
		case CPUINFO_STR_REGISTER + MIPS3_FPR27:		sprintf(info->s, "FPR27:%08X%08X", (UINT32)(mips->cpr[1][27] >> 32), (UINT32)mips->cpr[1][27]); break;
		case CPUINFO_STR_REGISTER + MIPS3_FPR28:		sprintf(info->s, "FPR28:%08X%08X", (UINT32)(mips->cpr[1][28] >> 32), (UINT32)mips->cpr[1][28]); break;
		case CPUINFO_STR_REGISTER + MIPS3_FPR29:		sprintf(info->s, "FPR29:%08X%08X", (UINT32)(mips->cpr[1][29] >> 32), (UINT32)mips->cpr[1][29]); break;
		case CPUINFO_STR_REGISTER + MIPS3_FPR30:		sprintf(info->s, "FPR30:%08X%08X", (UINT32)(mips->cpr[1][30] >> 32), (UINT32)mips->cpr[1][30]); break;
		case CPUINFO_STR_REGISTER + MIPS3_FPR31:		sprintf(info->s, "FPR31:%08X%08X", (UINT32)(mips->cpr[1][31] >> 32), (UINT32)mips->cpr[1][31]); break;
	}
}


/***************************************************************************
    INTERNAL HELPERS
***************************************************************************/

/*-------------------------------------------------
    compare_int_callback - callback that fires
    whenever a compare interrupt is generated
-------------------------------------------------*/

static TIMER_CALLBACK( compare_int_callback )
{
	cpunum_set_input_line(param, MIPS3_IRQ5, ASSERT_LINE);
}


/*-------------------------------------------------
    compute_config_register - compute the value
    of the config register
-------------------------------------------------*/

static UINT32 compute_config_register(const mips3_state *mips)
{
	/* set the cache line size to 32 bytes */
	UINT32 configreg = 0x00026030;
	int divisor;

	/* set the data cache size */
	     if (mips->icache_size <= 0x01000) configreg |= 0 << 6;
	else if (mips->icache_size <= 0x02000) configreg |= 1 << 6;
	else if (mips->icache_size <= 0x04000) configreg |= 2 << 6;
	else if (mips->icache_size <= 0x08000) configreg |= 3 << 6;
	else if (mips->icache_size <= 0x10000) configreg |= 4 << 6;
	else if (mips->icache_size <= 0x20000) configreg |= 5 << 6;
	else if (mips->icache_size <= 0x40000) configreg |= 6 << 6;
	else                                   configreg |= 7 << 6;

	/* set the instruction cache size */
	     if (mips->icache_size <= 0x01000) configreg |= 0 << 9;
	else if (mips->icache_size <= 0x02000) configreg |= 1 << 9;
	else if (mips->icache_size <= 0x04000) configreg |= 2 << 9;
	else if (mips->icache_size <= 0x08000) configreg |= 3 << 9;
	else if (mips->icache_size <= 0x10000) configreg |= 4 << 9;
	else if (mips->icache_size <= 0x20000) configreg |= 5 << 9;
	else if (mips->icache_size <= 0x40000) configreg |= 6 << 9;
	else                                   configreg |= 7 << 9;

	/* set the endianness bit */
	if (mips->bigendian)
		configreg |= 0x00008000;

	/* set the system clock divider */
	divisor = 2;
	if (mips->system_clock != 0)
	{
		divisor = mips->cpu_clock / mips->system_clock;
		if (mips->system_clock * divisor != mips->cpu_clock)
		{
			configreg |= 0x80000000;
			divisor = mips->cpu_clock * 2 / mips->system_clock;
		}
	}
	configreg |= (((divisor < 2) ? 2 : (divisor > 8) ? 8 : divisor) - 2) << 28;

	return configreg;
}


/*-------------------------------------------------
    compute_prid_register - compute the value
    of the PRId register
-------------------------------------------------*/

static UINT32 compute_prid_register(const mips3_state *mips)
{
	switch (mips->flavor)
	{
		case MIPS3_TYPE_R4600:
		case MIPS3_TYPE_R4650:
			return 0x2000;

		case MIPS3_TYPE_R4700:
			return 0x2100;

		case MIPS3_TYPE_R5000:
		case MIPS3_TYPE_QED5271:
			return 0x2300;

		case MIPS3_TYPE_RM7000:
			return 0x2700;

		default:
			fatalerror("Unknown MIPS flavor specified");
	}
	return 0x2000;
}


/*-------------------------------------------------
    tlb_write_common - common routine for writing
    a TLB entry
-------------------------------------------------*/

static void tlb_write_common(mips3_state *mips, int index)
{
	/* only handle entries within the TLB */
	if (index < ARRAY_LENGTH(mips->tlb))
	{
		mips3_tlb_entry *tlbent = &mips->tlb[index];

		/* unmap what we have */
		mips3com_unmap_tlb_entries(mips);

		/* fill in the new TLB entry from the COP0 registers */
		tlbent->page_mask = mips->cpr[0][COP0_PageMask];
		tlbent->entry_hi = mips->cpr[0][COP0_EntryHi] & ~(tlbent->page_mask & U64(0x0000000001ffe000));
		tlbent->entry_lo[0] = mips->cpr[0][COP0_EntryLo0];
		tlbent->entry_lo[1] = mips->cpr[0][COP0_EntryLo1];

		/* remap the TLB */
		mips3com_map_tlb_entries(mips);

		/* log the two halves once they are in */
		tlb_entry_log_half(tlbent, index, 0);
		tlb_entry_log_half(tlbent, index, 1);
	}
}


/*-------------------------------------------------
    tlb_entry_log_half - log half of a single TLB
    entry
-------------------------------------------------*/

static void tlb_entry_log_half(mips3_tlb_entry *tlbent, int index, int which)
{
#if PRINTF_TLB
	UINT64 hi = tlbent->entry_hi;
	UINT64 lo = tlbent->entry_lo[which];
	UINT32 vpn = (((hi >> 13) & 0x07ffffff) << 1);
	UINT32 asid = hi & 0xff;
	UINT32 r = (hi >> 62) & 3;
	UINT32 pfn = (lo >> 6) & 0x00ffffff;
	UINT32 c = (lo >> 3) & 7;
	UINT32 pagesize = (((tlbent->page_mask >> 13) & 0xfff) + 1) << MIPS3_MIN_PAGE_SHIFT;
	UINT64 vaddr = (UINT64)vpn * MIPS3_MIN_PAGE_SIZE;
	UINT64 paddr = (UINT64)pfn * MIPS3_MIN_PAGE_SIZE;

	vaddr += pagesize * which;

	mame_printf_debug("index=%08X  pagesize=%08X  vaddr=%08X%08X  paddr=%08X%08X  asid=%02X  r=%X  c=%X  dvg=%c%c%c\n",
			index, pagesize, (UINT32)(vaddr >> 32), (UINT32)vaddr, (UINT32)(paddr >> 32), (UINT32)paddr,
			asid, r, c, (lo & 4) ? 'd' : '.', (lo & 2) ? 'v' : '.', (hi & 0x1000) ? 'g' : '.');
#endif
}



/***************************************************************************
    DOUBLEWORD READS/WRITES
***************************************************************************/

/*-------------------------------------------------
    program_read_qword_32be - read a 64-bit
    big-endian value
-------------------------------------------------*/

static UINT64 program_read_qword_32be(offs_t offset)
{
	UINT64 result = (UINT64)program_read_dword_32be(offset) << 32;
	return result | program_read_dword_32be(offset + 4);
}


/*-------------------------------------------------
    program_read_qword_masked_32be - read a 64-bit
    big-endian value with explicit masking
-------------------------------------------------*/

static UINT64 program_read_qword_masked_32be(offs_t offset, UINT64 mem_mask)
{
	UINT64 result = 0;
	if ((mem_mask & U64(0xffffffff00000000)) != U64(0xffffffff00000000))
		result |= (UINT64)program_read_masked_32be(offset, mem_mask >> 32) << 32;
	if ((mem_mask & U64(0x00000000ffffffff)) != U64(0x00000000ffffffff))
		result |= program_read_masked_32be(offset + 4, mem_mask);
	return result;
}


/*-------------------------------------------------
    program_read_qword_32le - read a 64-bit
    little-endian value
-------------------------------------------------*/

static UINT64 program_read_qword_32le(offs_t offset)
{
	UINT64 result = program_read_dword_32le(offset);
	return result | ((UINT64)program_read_dword_32le(offset + 4) << 32);
}


/*-------------------------------------------------
    program_read_qword_masked_32le - read a 64-bit
    little-endian value with explicit masking
-------------------------------------------------*/

static UINT64 program_read_qword_masked_32le(offs_t offset, UINT64 mem_mask)
{
	UINT64 result = 0;
	if ((mem_mask & U64(0x00000000ffffffff)) != U64(0x00000000ffffffff))
		result |= program_read_masked_32le(offset, mem_mask);
	if ((mem_mask & U64(0xffffffff00000000)) != U64(0xffffffff00000000))
		result |= (UINT64)program_read_masked_32le(offset + 4, mem_mask >> 32) << 32;
	return result;
}


/*-------------------------------------------------
    program_write_qword_32be - write a 64-bit
    big-endian value
-------------------------------------------------*/

static void program_write_qword_32be(offs_t offset, UINT64 data)
{
	program_write_dword_32be(offset, data >> 32);
	program_write_dword_32be(offset + 4, data);
}


/*-------------------------------------------------
    program_write_qword_masked_32be - write a
    64-bit big-endian value with explicit masking
-------------------------------------------------*/

static void program_write_qword_masked_32be(offs_t offset, UINT64 data, UINT64 mem_mask)
{
	if ((mem_mask & U64(0xffffffff00000000)) != U64(0xffffffff00000000))
		program_write_masked_32be(offset, data >> 32, mem_mask >> 32);
	if ((mem_mask & U64(0x00000000ffffffff)) != U64(0x00000000ffffffff))
		program_write_masked_32be(offset + 4, data, mem_mask);
}


/*-------------------------------------------------
    program_write_qword_32le - write a 64-bit
    little-endian value
-------------------------------------------------*/

static void program_write_qword_32le(offs_t offset, UINT64 data)
{
	program_write_dword_32le(offset, data);
	program_write_dword_32le(offset + 4, data >> 32);
}


/*-------------------------------------------------
    program_write_qword_masked_32le - write a
    64-bit little-endian value with explicit masking
-------------------------------------------------*/

static void program_write_qword_masked_32le(offs_t offset, UINT64 data, UINT64 mem_mask)
{
	if ((mem_mask & U64(0x00000000ffffffff)) != U64(0x00000000ffffffff))
		program_write_masked_32le(offset, data, mem_mask);
	if ((mem_mask & U64(0xffffffff00000000)) != U64(0xffffffff00000000))
		program_write_masked_32le(offset + 4, data >> 32, mem_mask >> 32);
}
