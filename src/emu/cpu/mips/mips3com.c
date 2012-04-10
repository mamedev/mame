/***************************************************************************

    mips3com.c

    Common MIPS III/IV definitions and functions

***************************************************************************/

#include "emu.h"
#include "mips3com.h"


/***************************************************************************
    DEBUGGING
***************************************************************************/

#define PRINTF_TLB				(0)
#define USE_ABI_REG_NAMES		(1)


/***************************************************************************
    FUNCTION PROTOTYPES
***************************************************************************/

static TIMER_CALLBACK( compare_int_callback );

static UINT32 compute_config_register(const mips3_state *mips);
static UINT32 compute_prid_register(const mips3_state *mips);

static void tlb_map_entry(mips3_state *mips, int tlbindex);
static void tlb_write_common(mips3_state *mips, int tlbindex);
static void tlb_entry_log_half(mips3_tlb_entry *entry, int tlbindex, int which);



/***************************************************************************
    INLINE FUNCTIONS
***************************************************************************/

/*-------------------------------------------------
    tlb_entry_matches_asid - TRUE if the given
    TLB entry matches the provided ASID
-------------------------------------------------*/

INLINE int tlb_entry_matches_asid(const mips3_tlb_entry *entry, UINT8 asid)
{
	return (entry->entry_hi & 0xff) == asid;
}


/*-------------------------------------------------
    tlb_entry_is_global - TRUE if the given
    TLB entry is global
-------------------------------------------------*/

INLINE int tlb_entry_is_global(const mips3_tlb_entry *entry)
{
	return (entry->entry_lo[0] & entry->entry_lo[1] & TLB_GLOBAL);
}



/***************************************************************************
    INITIALIZATION AND SHUTDOWN
***************************************************************************/

/*-------------------------------------------------
    mips3com_init - initialize the mips3_state
    structure based on the configured type
-------------------------------------------------*/

void mips3com_init(mips3_state *mips, mips3_flavor flavor, int bigendian, legacy_cpu_device *device, device_irq_acknowledge_callback irqcallback)
{
	const mips3_config *config = (const mips3_config *)device->static_config();
	int tlbindex;

	/* initialize based on the config */
	memset(mips, 0, sizeof(*mips));
	mips->flavor = flavor;
	mips->bigendian = bigendian;
	mips->cpu_clock = device->clock();
	mips->irq_callback = irqcallback;
	mips->device = device;
	mips->program = device->space(AS_PROGRAM);
	mips->direct = &mips->program->direct();
	mips->icache_size = config->icache;
	mips->dcache_size = config->dcache;
	mips->system_clock = config->system_clock;

	/* configure flavor-specific parameters */
	mips->pfnmask = 0x00ffffff;
	mips->tlbentries = MIPS3_MAX_TLB_ENTRIES;

	/* VR4300 and VR5432 have 4 fewer PFN bits, and only 32 TLB entries */
	if (flavor == MIPS3_TYPE_VR4300)
	{
		mips->pfnmask = 0x000fffff;
		mips->tlbentries = 32;
	}

	/* set up the endianness */
	mips->program->accessors(mips->memory);

	/* allocate the virtual TLB */
	mips->vtlb = vtlb_alloc(device, AS_PROGRAM, 2 * mips->tlbentries + 2, 0);

	/* allocate a timer for the compare interrupt */
	mips->compare_int_timer = device->machine().scheduler().timer_alloc(FUNC(compare_int_callback), (void *)device);

	/* reset the state */
	mips3com_reset(mips);

	/* register for save states */
	device->save_item(NAME(mips->pc));
	device->save_item(NAME(mips->r));
	device->save_item(NAME(mips->cpr));
	device->save_item(NAME(mips->ccr));
	device->save_item(NAME(mips->llbit));
	device->save_item(NAME(mips->count_zero_time));
	for (tlbindex = 0; tlbindex < mips->tlbentries; tlbindex++)
	{
		device->save_item(NAME(mips->tlb[tlbindex].page_mask), tlbindex);
		device->save_item(NAME(mips->tlb[tlbindex].entry_hi), tlbindex);
		device->save_item(NAME(mips->tlb[tlbindex].entry_lo), tlbindex);
	}
}


/*-------------------------------------------------
    mips3com_exit - common cleanup/exit
-------------------------------------------------*/

void mips3com_exit(mips3_state *mips)
{
	if (mips->vtlb != NULL)
		vtlb_free(mips->vtlb);
}


/*-------------------------------------------------
    mips3com_reset - reset the state of all the
    registers
-------------------------------------------------*/

void mips3com_reset(mips3_state *mips)
{
	int tlbindex;

	/* initialize the state */
	mips->pc = 0xbfc00000;
	mips->cpr[0][COP0_Status] = SR_BEV | SR_ERL;
	mips->cpr[0][COP0_Wired] = 0;
	mips->cpr[0][COP0_Compare] = 0xffffffff;
	mips->cpr[0][COP0_Count] = 0;
	mips->cpr[0][COP0_Config] = compute_config_register(mips);
	mips->cpr[0][COP0_PRId] = compute_prid_register(mips);
	mips->count_zero_time = mips->device->total_cycles();

	/* initialize the TLB state */
	for (tlbindex = 0; tlbindex < mips->tlbentries; tlbindex++)
	{
		mips3_tlb_entry *entry = &mips->tlb[tlbindex];
		entry->page_mask = 0;
		entry->entry_hi = 0xffffffff;
		entry->entry_lo[0] = 0xfffffff8;
		entry->entry_lo[1] = 0xfffffff8;
		vtlb_load(mips->vtlb, 2 * tlbindex + 0, 0, 0, 0);
		vtlb_load(mips->vtlb, 2 * tlbindex + 1, 0, 0, 0);
	}

	/* load the fixed TLB range */
	vtlb_load(mips->vtlb, 2 * mips->tlbentries + 0, (0xa0000000 - 0x80000000) >> MIPS3_MIN_PAGE_SHIFT, 0x80000000, 0x00000000 | VTLB_READ_ALLOWED | VTLB_WRITE_ALLOWED | VTLB_FETCH_ALLOWED | VTLB_FLAG_VALID);
	vtlb_load(mips->vtlb, 2 * mips->tlbentries + 1, (0xc0000000 - 0xa0000000) >> MIPS3_MIN_PAGE_SHIFT, 0xa0000000, 0x00000000 | VTLB_READ_ALLOWED | VTLB_WRITE_ALLOWED | VTLB_FETCH_ALLOWED | VTLB_FLAG_VALID);
}


/*-------------------------------------------------
    mips3com_dasm - handle disassembly for a
    CPU
-------------------------------------------------*/

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


/*-------------------------------------------------
    mips3com_update_cycle_counting - update cycle
    counts and the timers
-------------------------------------------------*/

void mips3com_update_cycle_counting(mips3_state *mips)
{
	/* modify the timer to go off */
	if (mips->compare_armed && (mips->cpr[0][COP0_Status] & SR_IMEX5))
	{
		UINT32 count = (mips->device->total_cycles() - mips->count_zero_time) / 2;
		UINT32 compare = mips->cpr[0][COP0_Compare];
		UINT32 delta = compare - count;
		attotime newtime = mips->device->cycles_to_attotime((UINT64)delta * 2);
		mips->compare_int_timer->adjust(newtime);
		return;
	}
	mips->compare_int_timer->adjust(attotime::never);
}



/***************************************************************************
    TLB HANDLING
***************************************************************************/

/*-------------------------------------------------
    mips3com_asid_changed - remap all non-global
    TLB entries
-------------------------------------------------*/

void mips3com_asid_changed(mips3_state *mips)
{
	int tlbindex;

	/* iterate over all non-global TLB entries and remap them */
	for (tlbindex = 0; tlbindex < mips->tlbentries; tlbindex++)
		if (!tlb_entry_is_global(&mips->tlb[tlbindex]))
			tlb_map_entry(mips, tlbindex);
}


/*-------------------------------------------------
    mips3com_translate_address - translate an address
    from logical to physical
-------------------------------------------------*/

int mips3com_translate_address(mips3_state *mips, address_spacenum space, int intention, offs_t *address)
{
	/* only applies to the program address space */
	if (space == AS_PROGRAM)
	{
		const vtlb_entry *table = vtlb_table(mips->vtlb);
		vtlb_entry entry = table[*address >> MIPS3_MIN_PAGE_SHIFT];
		if ((entry & (1 << (intention & (TRANSLATE_TYPE_MASK | TRANSLATE_USER_MASK)))) == 0)
			return FALSE;
		*address = (entry & ~MIPS3_MIN_PAGE_MASK) | (*address & MIPS3_MIN_PAGE_MASK);
	}
	return TRUE;
}


/*-------------------------------------------------
    mips3com_tlbr - execute the tlbr instruction
-------------------------------------------------*/

void mips3com_tlbr(mips3_state *mips)
{
	UINT32 tlbindex = mips->cpr[0][COP0_Index] & 0x3f;

	/* only handle entries within the TLB */
	if (tlbindex < mips->tlbentries)
	{
		mips3_tlb_entry *entry = &mips->tlb[tlbindex];

		/* copy data from the TLB entry into the COP0 registers */
		mips->cpr[0][COP0_PageMask] = entry->page_mask;
		mips->cpr[0][COP0_EntryHi] = entry->entry_hi;
		mips->cpr[0][COP0_EntryLo0] = entry->entry_lo[0];
		mips->cpr[0][COP0_EntryLo1] = entry->entry_lo[1];
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
	UINT32 unwired = mips->tlbentries - wired;
	UINT32 tlbindex = mips->tlbentries - 1;

	/* "random" is based off of the current cycle counting through the non-wired pages */
	if (unwired > 0)
		tlbindex = ((mips->device->total_cycles() - mips->count_zero_time) % unwired + wired) & 0x3f;

	/* use the common handler to write to this tlbindex */
	tlb_write_common(mips, tlbindex);
}


/*-------------------------------------------------
    mips3com_tlbp - execute the tlbp instruction
-------------------------------------------------*/

void mips3com_tlbp(mips3_state *mips)
{
	UINT32 tlbindex;
//  UINT64 vpn;

	/* iterate over TLB entries */
	for (tlbindex = 0; tlbindex < mips->tlbentries; tlbindex++)
	{
		mips3_tlb_entry *entry = &mips->tlb[tlbindex];
		UINT64 mask = ~((entry->page_mask >> 13) & 0xfff) << 13;

		/* if the relevant bits of EntryHi match the relevant bits of the TLB */
		if ((entry->entry_hi & mask) == (mips->cpr[0][COP0_EntryHi] & mask))

			/* and if we are either global or matching the current ASID, then stop */
			if ((entry->entry_hi & 0xff) == (mips->cpr[0][COP0_EntryHi] & 0xff) || ((entry->entry_lo[0] & entry->entry_lo[1]) & TLB_GLOBAL))
				break;
	}

	/* validate that our tlb_table was in sync */
//  vpn = ((mips->cpr[0][COP0_EntryHi] >> 13) & 0x07ffffff) << 1;
	if (tlbindex != mips->tlbentries)
		mips->cpr[0][COP0_Index] = tlbindex;
	else
		mips->cpr[0][COP0_Index] = 0x80000000;
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
		case CPUINFO_INT_REGISTER + MIPS3_SR:			mips->cpr[0][COP0_Status] = info->i;	break;
		case CPUINFO_INT_REGISTER + MIPS3_EPC:			mips->cpr[0][COP0_EPC] = info->i;		break;
		case CPUINFO_INT_REGISTER + MIPS3_CAUSE:		mips->cpr[0][COP0_Cause] = info->i;		break;
		case CPUINFO_INT_REGISTER + MIPS3_COUNT:		mips->cpr[0][COP0_Count] = info->i; 	break;
		case CPUINFO_INT_REGISTER + MIPS3_COMPARE:		mips->cpr[0][COP0_Compare] = info->i;	break;
		case CPUINFO_INT_REGISTER + MIPS3_INDEX:		mips->cpr[0][COP0_Index] = info->i; 	break;
		case CPUINFO_INT_REGISTER + MIPS3_RANDOM:		mips->cpr[0][COP0_Random] = info->i;	break;
		case CPUINFO_INT_REGISTER + MIPS3_ENTRYHI:		mips->cpr[0][COP0_EntryHi] = info->i;	break;
		case CPUINFO_INT_REGISTER + MIPS3_ENTRYLO0:		mips->cpr[0][COP0_EntryLo0] = info->i;	break;
		case CPUINFO_INT_REGISTER + MIPS3_ENTRYLO1:		mips->cpr[0][COP0_EntryLo1] = info->i;	break;
		case CPUINFO_INT_REGISTER + MIPS3_PAGEMASK:		mips->cpr[0][COP0_PageMask] = info->i;	break;
		case CPUINFO_INT_REGISTER + MIPS3_WIRED:		mips->cpr[0][COP0_Wired] = info->i; 	break;
		case CPUINFO_INT_REGISTER + MIPS3_BADVADDR:		mips->cpr[0][COP0_BadVAddr] = info->i;	break;

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
		case DEVINFO_INT_ENDIANNESS:					info->i = mips->bigendian ? ENDIANNESS_BIG : ENDIANNESS_LITTLE; break;
		case CPUINFO_INT_CLOCK_MULTIPLIER:				info->i = 1;							break;
		case CPUINFO_INT_CLOCK_DIVIDER:					info->i = 1;							break;
		case CPUINFO_INT_MIN_INSTRUCTION_BYTES:			info->i = 4;							break;
		case CPUINFO_INT_MAX_INSTRUCTION_BYTES:			info->i = 4;							break;
		case CPUINFO_INT_MIN_CYCLES:					info->i = 1;							break;
		case CPUINFO_INT_MAX_CYCLES:					info->i = 40;							break;

		case DEVINFO_INT_DATABUS_WIDTH + AS_PROGRAM:	info->i = MIPS3_MAX_PADDR_SHIFT;break;
		case DEVINFO_INT_ADDRBUS_WIDTH + AS_PROGRAM: info->i = 32;					break;
		case DEVINFO_INT_ADDRBUS_SHIFT + AS_PROGRAM: info->i = 0;					break;
		case CPUINFO_INT_LOGADDR_WIDTH_PROGRAM: info->i = 32;					break;
		case CPUINFO_INT_PAGE_SHIFT_PROGRAM:	info->i = MIPS3_MIN_PAGE_SHIFT;	break;

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
		case CPUINFO_INT_REGISTER + MIPS3_COUNT:		info->i = ((mips->device->total_cycles() - mips->count_zero_time) / 2); break;
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
		case CPUINFO_FCT_SET_INFO:						/* provided by core */					break;
		case CPUINFO_FCT_INIT:							/* provided by core */					break;
		case CPUINFO_FCT_RESET:							/* provided by core */					break;
		case CPUINFO_FCT_EXIT:							/* provided by core */					break;
		case CPUINFO_FCT_EXECUTE:						/* provided by core */					break;
		case CPUINFO_FCT_TRANSLATE:						/* provided by core */					break;
		case CPUINFO_FCT_DISASSEMBLE:					/* provided by core */					break;
		case CPUINFO_PTR_INSTRUCTION_COUNTER:			info->icount = &mips->icount;			break;

		/* --- the following bits of info are returned as NULL-terminated strings --- */
		case DEVINFO_STR_NAME:							strcpy(info->s, "MIPS III");			break;
		case DEVINFO_STR_FAMILY:					strcpy(info->s, "MIPS III");			break;
		case DEVINFO_STR_VERSION:					strcpy(info->s, "3.0");					break;
		case DEVINFO_STR_SOURCE_FILE:						/* provided by core */					break;
		case DEVINFO_STR_CREDITS:					strcpy(info->s, "Copyright Aaron Giles"); break;

		case CPUINFO_STR_FLAGS:							strcpy(info->s, " ");					break;

		case CPUINFO_STR_REGISTER + MIPS3_PC:			sprintf(info->s, "PC: %08X", mips->pc); break;
		case CPUINFO_STR_REGISTER + MIPS3_SR:			sprintf(info->s, "SR: %08X", (UINT32)mips->cpr[0][COP0_Status]); break;
		case CPUINFO_STR_REGISTER + MIPS3_EPC:			sprintf(info->s, "EPC:%08X", (UINT32)mips->cpr[0][COP0_EPC]); break;
		case CPUINFO_STR_REGISTER + MIPS3_CAUSE:		sprintf(info->s, "Cause:%08X", (UINT32)mips->cpr[0][COP0_Cause]); break;
		case CPUINFO_STR_REGISTER + MIPS3_COUNT:		sprintf(info->s, "Count:%08X", (UINT32)((mips->device->total_cycles() - mips->count_zero_time) / 2)); break;
		case CPUINFO_STR_REGISTER + MIPS3_COMPARE:		sprintf(info->s, "Compare:%08X", (UINT32)mips->cpr[0][COP0_Compare]); break;
		case CPUINFO_STR_REGISTER + MIPS3_INDEX:		sprintf(info->s, "Index:%08X", (UINT32)mips->cpr[0][COP0_Index]); break;
		case CPUINFO_STR_REGISTER + MIPS3_RANDOM:		sprintf(info->s, "Random:%08X", (UINT32)mips->cpr[0][COP0_Random]); break;
		case CPUINFO_STR_REGISTER + MIPS3_ENTRYHI:		sprintf(info->s, "EntryHi:%08X%08X", (UINT32)(mips->cpr[0][COP0_EntryHi] >> 32), (UINT32)mips->cpr[0][COP0_EntryHi]); break;
		case CPUINFO_STR_REGISTER + MIPS3_ENTRYLO0:		sprintf(info->s, "EntryLo0:%08X%08X", (UINT32)(mips->cpr[0][COP0_EntryLo0] >> 32), (UINT32)mips->cpr[0][COP0_EntryLo0]); break;
		case CPUINFO_STR_REGISTER + MIPS3_ENTRYLO1:		sprintf(info->s, "EntryLo1:%08X%08X", (UINT32)(mips->cpr[0][COP0_EntryLo1] >> 32), (UINT32)mips->cpr[0][COP0_EntryLo1]); break;
		case CPUINFO_STR_REGISTER + MIPS3_PAGEMASK:		sprintf(info->s, "PageMask:%08X%08X", (UINT32)(mips->cpr[0][COP0_PageMask] >> 32), (UINT32)mips->cpr[0][COP0_PageMask]); break;
		case CPUINFO_STR_REGISTER + MIPS3_WIRED:		sprintf(info->s, "Wired:%08X", (UINT32)mips->cpr[0][COP0_Wired]); break;
		case CPUINFO_STR_REGISTER + MIPS3_BADVADDR:		sprintf(info->s, "BadVAddr:%08X", (UINT32)mips->cpr[0][COP0_BadVAddr]); break;

#if USE_ABI_REG_NAMES
		case CPUINFO_STR_REGISTER + MIPS3_R0:			sprintf(info->s, "$zero: %08X%08X", (UINT32)(mips->r[0] >> 32), (UINT32)mips->r[0]); break;
		case CPUINFO_STR_REGISTER + MIPS3_R1:			sprintf(info->s, "  $at: %08X%08X", (UINT32)(mips->r[1] >> 32), (UINT32)mips->r[1]); break;
		case CPUINFO_STR_REGISTER + MIPS3_R2:			sprintf(info->s, "  $v0: %08X%08X", (UINT32)(mips->r[2] >> 32), (UINT32)mips->r[2]); break;
		case CPUINFO_STR_REGISTER + MIPS3_R3:			sprintf(info->s, "  $v1: %08X%08X", (UINT32)(mips->r[3] >> 32), (UINT32)mips->r[3]); break;
		case CPUINFO_STR_REGISTER + MIPS3_R4:			sprintf(info->s, "  $a0: %08X%08X", (UINT32)(mips->r[4] >> 32), (UINT32)mips->r[4]); break;
		case CPUINFO_STR_REGISTER + MIPS3_R5:			sprintf(info->s, "  $a1: %08X%08X", (UINT32)(mips->r[5] >> 32), (UINT32)mips->r[5]); break;
		case CPUINFO_STR_REGISTER + MIPS3_R6:			sprintf(info->s, "  $a2: %08X%08X", (UINT32)(mips->r[6] >> 32), (UINT32)mips->r[6]); break;
		case CPUINFO_STR_REGISTER + MIPS3_R7:			sprintf(info->s, "  $a3: %08X%08X", (UINT32)(mips->r[7] >> 32), (UINT32)mips->r[7]); break;
		case CPUINFO_STR_REGISTER + MIPS3_R8:			sprintf(info->s, "  $t0: %08X%08X", (UINT32)(mips->r[8] >> 32), (UINT32)mips->r[8]); break;
		case CPUINFO_STR_REGISTER + MIPS3_R9:			sprintf(info->s, "  $t1: %08X%08X", (UINT32)(mips->r[9] >> 32), (UINT32)mips->r[9]); break;
		case CPUINFO_STR_REGISTER + MIPS3_R10:			sprintf(info->s, "  $t2:%08X%08X", (UINT32)(mips->r[10] >> 32), (UINT32)mips->r[10]); break;
		case CPUINFO_STR_REGISTER + MIPS3_R11:			sprintf(info->s, "  $t3:%08X%08X", (UINT32)(mips->r[11] >> 32), (UINT32)mips->r[11]); break;
		case CPUINFO_STR_REGISTER + MIPS3_R12:			sprintf(info->s, "  $t4:%08X%08X", (UINT32)(mips->r[12] >> 32), (UINT32)mips->r[12]); break;
		case CPUINFO_STR_REGISTER + MIPS3_R13:			sprintf(info->s, "  $t5:%08X%08X", (UINT32)(mips->r[13] >> 32), (UINT32)mips->r[13]); break;
		case CPUINFO_STR_REGISTER + MIPS3_R14:			sprintf(info->s, "  $t6:%08X%08X", (UINT32)(mips->r[14] >> 32), (UINT32)mips->r[14]); break;
		case CPUINFO_STR_REGISTER + MIPS3_R15:			sprintf(info->s, "  $t7:%08X%08X", (UINT32)(mips->r[15] >> 32), (UINT32)mips->r[15]); break;
		case CPUINFO_STR_REGISTER + MIPS3_R16:			sprintf(info->s, "  $s0:%08X%08X", (UINT32)(mips->r[16] >> 32), (UINT32)mips->r[16]); break;
		case CPUINFO_STR_REGISTER + MIPS3_R17:			sprintf(info->s, "  $s1:%08X%08X", (UINT32)(mips->r[17] >> 32), (UINT32)mips->r[17]); break;
		case CPUINFO_STR_REGISTER + MIPS3_R18:			sprintf(info->s, "  $s2:%08X%08X", (UINT32)(mips->r[18] >> 32), (UINT32)mips->r[18]); break;
		case CPUINFO_STR_REGISTER + MIPS3_R19:			sprintf(info->s, "  $s3:%08X%08X", (UINT32)(mips->r[19] >> 32), (UINT32)mips->r[19]); break;
		case CPUINFO_STR_REGISTER + MIPS3_R20:			sprintf(info->s, "  $s4:%08X%08X", (UINT32)(mips->r[20] >> 32), (UINT32)mips->r[20]); break;
		case CPUINFO_STR_REGISTER + MIPS3_R21:			sprintf(info->s, "  $s5:%08X%08X", (UINT32)(mips->r[21] >> 32), (UINT32)mips->r[21]); break;
		case CPUINFO_STR_REGISTER + MIPS3_R22:			sprintf(info->s, "  $s6:%08X%08X", (UINT32)(mips->r[22] >> 32), (UINT32)mips->r[22]); break;
		case CPUINFO_STR_REGISTER + MIPS3_R23:			sprintf(info->s, "  $s7:%08X%08X", (UINT32)(mips->r[23] >> 32), (UINT32)mips->r[23]); break;
		case CPUINFO_STR_REGISTER + MIPS3_R24:			sprintf(info->s, "  $t8:%08X%08X", (UINT32)(mips->r[24] >> 32), (UINT32)mips->r[24]); break;
		case CPUINFO_STR_REGISTER + MIPS3_R25:			sprintf(info->s, "  $t9:%08X%08X", (UINT32)(mips->r[25] >> 32), (UINT32)mips->r[25]); break;
		case CPUINFO_STR_REGISTER + MIPS3_R26:			sprintf(info->s, "  $k0:%08X%08X", (UINT32)(mips->r[26] >> 32), (UINT32)mips->r[26]); break;
		case CPUINFO_STR_REGISTER + MIPS3_R27:			sprintf(info->s, "  $k1:%08X%08X", (UINT32)(mips->r[27] >> 32), (UINT32)mips->r[27]); break;
		case CPUINFO_STR_REGISTER + MIPS3_R28:			sprintf(info->s, "  $gp:%08X%08X", (UINT32)(mips->r[28] >> 32), (UINT32)mips->r[28]); break;
		case CPUINFO_STR_REGISTER + MIPS3_R29:			sprintf(info->s, "  $sp:%08X%08X", (UINT32)(mips->r[29] >> 32), (UINT32)mips->r[29]); break;
		case CPUINFO_STR_REGISTER + MIPS3_R30:			sprintf(info->s, "  $fp:%08X%08X", (UINT32)(mips->r[30] >> 32), (UINT32)mips->r[30]); break;
		case CPUINFO_STR_REGISTER + MIPS3_R31:			sprintf(info->s, "  $ra:%08X%08X", (UINT32)(mips->r[31] >> 32), (UINT32)mips->r[31]); break;
#else
		case CPUINFO_STR_REGISTER + MIPS3_R0:			sprintf(info->s, " R0: %08X%08X", (UINT32)(mips->r[0] >> 32), (UINT32)mips->r[0]); break;
		case CPUINFO_STR_REGISTER + MIPS3_R1:			sprintf(info->s, " R1: %08X%08X", (UINT32)(mips->r[1] >> 32), (UINT32)mips->r[1]); break;
		case CPUINFO_STR_REGISTER + MIPS3_R2:			sprintf(info->s, " R2: %08X%08X", (UINT32)(mips->r[2] >> 32), (UINT32)mips->r[2]); break;
		case CPUINFO_STR_REGISTER + MIPS3_R3:			sprintf(info->s, " R3: %08X%08X", (UINT32)(mips->r[3] >> 32), (UINT32)mips->r[3]); break;
		case CPUINFO_STR_REGISTER + MIPS3_R4:			sprintf(info->s, " R4: %08X%08X", (UINT32)(mips->r[4] >> 32), (UINT32)mips->r[4]); break;
		case CPUINFO_STR_REGISTER + MIPS3_R5:			sprintf(info->s, " R5: %08X%08X", (UINT32)(mips->r[5] >> 32), (UINT32)mips->r[5]); break;
		case CPUINFO_STR_REGISTER + MIPS3_R6:			sprintf(info->s, " R6: %08X%08X", (UINT32)(mips->r[6] >> 32), (UINT32)mips->r[6]); break;
		case CPUINFO_STR_REGISTER + MIPS3_R7:			sprintf(info->s, " R7: %08X%08X", (UINT32)(mips->r[7] >> 32), (UINT32)mips->r[7]); break;
		case CPUINFO_STR_REGISTER + MIPS3_R8:			sprintf(info->s, " R8: %08X%08X", (UINT32)(mips->r[8] >> 32), (UINT32)mips->r[8]); break;
		case CPUINFO_STR_REGISTER + MIPS3_R9:			sprintf(info->s, " R9: %08X%08X", (UINT32)(mips->r[9] >> 32), (UINT32)mips->r[9]); break;
		case CPUINFO_STR_REGISTER + MIPS3_R10:			sprintf(info->s, "R10: %08X%08X", (UINT32)(mips->r[10] >> 32), (UINT32)mips->r[10]); break;
		case CPUINFO_STR_REGISTER + MIPS3_R11:			sprintf(info->s, "R11: %08X%08X", (UINT32)(mips->r[11] >> 32), (UINT32)mips->r[11]); break;
		case CPUINFO_STR_REGISTER + MIPS3_R12:			sprintf(info->s, "R12: %08X%08X", (UINT32)(mips->r[12] >> 32), (UINT32)mips->r[12]); break;
		case CPUINFO_STR_REGISTER + MIPS3_R13:			sprintf(info->s, "R13: %08X%08X", (UINT32)(mips->r[13] >> 32), (UINT32)mips->r[13]); break;
		case CPUINFO_STR_REGISTER + MIPS3_R14:			sprintf(info->s, "R14: %08X%08X", (UINT32)(mips->r[14] >> 32), (UINT32)mips->r[14]); break;
		case CPUINFO_STR_REGISTER + MIPS3_R15:			sprintf(info->s, "R15: %08X%08X", (UINT32)(mips->r[15] >> 32), (UINT32)mips->r[15]); break;
		case CPUINFO_STR_REGISTER + MIPS3_R16:			sprintf(info->s, "R16: %08X%08X", (UINT32)(mips->r[16] >> 32), (UINT32)mips->r[16]); break;
		case CPUINFO_STR_REGISTER + MIPS3_R17:			sprintf(info->s, "R17: %08X%08X", (UINT32)(mips->r[17] >> 32), (UINT32)mips->r[17]); break;
		case CPUINFO_STR_REGISTER + MIPS3_R18:			sprintf(info->s, "R18: %08X%08X", (UINT32)(mips->r[18] >> 32), (UINT32)mips->r[18]); break;
		case CPUINFO_STR_REGISTER + MIPS3_R19:			sprintf(info->s, "R19: %08X%08X", (UINT32)(mips->r[19] >> 32), (UINT32)mips->r[19]); break;
		case CPUINFO_STR_REGISTER + MIPS3_R20:			sprintf(info->s, "R20: %08X%08X", (UINT32)(mips->r[20] >> 32), (UINT32)mips->r[20]); break;
		case CPUINFO_STR_REGISTER + MIPS3_R21:			sprintf(info->s, "R21: %08X%08X", (UINT32)(mips->r[21] >> 32), (UINT32)mips->r[21]); break;
		case CPUINFO_STR_REGISTER + MIPS3_R22:			sprintf(info->s, "R22: %08X%08X", (UINT32)(mips->r[22] >> 32), (UINT32)mips->r[22]); break;
		case CPUINFO_STR_REGISTER + MIPS3_R23:			sprintf(info->s, "R23: %08X%08X", (UINT32)(mips->r[23] >> 32), (UINT32)mips->r[23]); break;
		case CPUINFO_STR_REGISTER + MIPS3_R24:			sprintf(info->s, "R24: %08X%08X", (UINT32)(mips->r[24] >> 32), (UINT32)mips->r[24]); break;
		case CPUINFO_STR_REGISTER + MIPS3_R25:			sprintf(info->s, "R25: %08X%08X", (UINT32)(mips->r[25] >> 32), (UINT32)mips->r[25]); break;
		case CPUINFO_STR_REGISTER + MIPS3_R26:			sprintf(info->s, "R26: %08X%08X", (UINT32)(mips->r[26] >> 32), (UINT32)mips->r[26]); break;
		case CPUINFO_STR_REGISTER + MIPS3_R27:			sprintf(info->s, "R27: %08X%08X", (UINT32)(mips->r[27] >> 32), (UINT32)mips->r[27]); break;
		case CPUINFO_STR_REGISTER + MIPS3_R28:			sprintf(info->s, "R28: %08X%08X", (UINT32)(mips->r[28] >> 32), (UINT32)mips->r[28]); break;
		case CPUINFO_STR_REGISTER + MIPS3_R29:			sprintf(info->s, "R29: %08X%08X", (UINT32)(mips->r[29] >> 32), (UINT32)mips->r[29]); break;
		case CPUINFO_STR_REGISTER + MIPS3_R30:			sprintf(info->s, "R30: %08X%08X", (UINT32)(mips->r[30] >> 32), (UINT32)mips->r[30]); break;
		case CPUINFO_STR_REGISTER + MIPS3_R31:			sprintf(info->s, "R31: %08X%08X", (UINT32)(mips->r[31] >> 32), (UINT32)mips->r[31]); break;
#endif
		case CPUINFO_STR_REGISTER + MIPS3_HI:			sprintf(info->s, "HI: %08X%08X", (UINT32)(mips->r[REG_HI] >> 32), (UINT32)mips->r[REG_HI]); break;
		case CPUINFO_STR_REGISTER + MIPS3_LO:			sprintf(info->s, "LO: %08X%08X", (UINT32)(mips->r[REG_LO] >> 32), (UINT32)mips->r[REG_LO]); break;

		case CPUINFO_STR_REGISTER + MIPS3_CCR1_31:		sprintf(info->s, "CCR31:%08X", (UINT32)mips->ccr[1][31]); break;

		case CPUINFO_STR_REGISTER + MIPS3_FPR0:			sprintf(info->s, "FPR0: %08X%08X", (UINT32)(mips->cpr[1][0] >> 32), (UINT32)mips->cpr[1][0]); break;
		case CPUINFO_STR_REGISTER + MIPS3_FPS0:			sprintf(info->s, "FPS0: !%16g", *(float *)&mips->cpr[1][0]); break;
		case CPUINFO_STR_REGISTER + MIPS3_FPD0:			sprintf(info->s, "FPD0: !%16g", *(double *)&mips->cpr[1][0]); break;
		case CPUINFO_STR_REGISTER + MIPS3_FPR1:			sprintf(info->s, "FPR1: %08X%08X", (UINT32)(mips->cpr[1][1] >> 32), (UINT32)mips->cpr[1][1]); break;
		case CPUINFO_STR_REGISTER + MIPS3_FPS1:			sprintf(info->s, "FPS1: !%16g", *(float *)&mips->cpr[1][1]); break;
		case CPUINFO_STR_REGISTER + MIPS3_FPD1:			sprintf(info->s, "FPD1: !%16g", *(double *)&mips->cpr[1][1]); break;
		case CPUINFO_STR_REGISTER + MIPS3_FPR2:			sprintf(info->s, "FPR2: %08X%08X", (UINT32)(mips->cpr[1][2] >> 32), (UINT32)mips->cpr[1][2]); break;
		case CPUINFO_STR_REGISTER + MIPS3_FPS2:			sprintf(info->s, "FPS2: !%16g", *(float *)&mips->cpr[1][2]); break;
		case CPUINFO_STR_REGISTER + MIPS3_FPD2:			sprintf(info->s, "FPD2: !%16g", *(double *)&mips->cpr[1][2]); break;
		case CPUINFO_STR_REGISTER + MIPS3_FPR3:			sprintf(info->s, "FPR3: %08X%08X", (UINT32)(mips->cpr[1][3] >> 32), (UINT32)mips->cpr[1][3]); break;
		case CPUINFO_STR_REGISTER + MIPS3_FPS3:			sprintf(info->s, "FPS3: !%16g", *(float *)&mips->cpr[1][3]); break;
		case CPUINFO_STR_REGISTER + MIPS3_FPD3:			sprintf(info->s, "FPD3: !%16g", *(double *)&mips->cpr[1][3]); break;
		case CPUINFO_STR_REGISTER + MIPS3_FPR4:			sprintf(info->s, "FPR4: %08X%08X", (UINT32)(mips->cpr[1][4] >> 32), (UINT32)mips->cpr[1][4]); break;
		case CPUINFO_STR_REGISTER + MIPS3_FPS4:			sprintf(info->s, "FPS4: !%16g", *(float *)&mips->cpr[1][4]); break;
		case CPUINFO_STR_REGISTER + MIPS3_FPD4:			sprintf(info->s, "FPD4: !%16g", *(double *)&mips->cpr[1][4]); break;
		case CPUINFO_STR_REGISTER + MIPS3_FPR5:			sprintf(info->s, "FPR5: %08X%08X", (UINT32)(mips->cpr[1][5] >> 32), (UINT32)mips->cpr[1][5]); break;
		case CPUINFO_STR_REGISTER + MIPS3_FPS5:			sprintf(info->s, "FPS5: !%16g", *(float *)&mips->cpr[1][5]); break;
		case CPUINFO_STR_REGISTER + MIPS3_FPD5:			sprintf(info->s, "FPD5: !%16g", *(double *)&mips->cpr[1][5]); break;
		case CPUINFO_STR_REGISTER + MIPS3_FPR6:			sprintf(info->s, "FPR6: %08X%08X", (UINT32)(mips->cpr[1][6] >> 32), (UINT32)mips->cpr[1][6]); break;
		case CPUINFO_STR_REGISTER + MIPS3_FPS6:			sprintf(info->s, "FPS6: !%16g", *(float *)&mips->cpr[1][6]); break;
		case CPUINFO_STR_REGISTER + MIPS3_FPD6:			sprintf(info->s, "FPD6: !%16g", *(double *)&mips->cpr[1][6]); break;
		case CPUINFO_STR_REGISTER + MIPS3_FPR7:			sprintf(info->s, "FPR7: %08X%08X", (UINT32)(mips->cpr[1][7] >> 32), (UINT32)mips->cpr[1][7]); break;
		case CPUINFO_STR_REGISTER + MIPS3_FPS7:			sprintf(info->s, "FPS7: !%16g", *(float *)&mips->cpr[1][7]); break;
		case CPUINFO_STR_REGISTER + MIPS3_FPD7:			sprintf(info->s, "FPD7: !%16g", *(double *)&mips->cpr[1][7]); break;
		case CPUINFO_STR_REGISTER + MIPS3_FPR8:			sprintf(info->s, "FPR8: %08X%08X", (UINT32)(mips->cpr[1][8] >> 32), (UINT32)mips->cpr[1][8]); break;
		case CPUINFO_STR_REGISTER + MIPS3_FPS8:			sprintf(info->s, "FPS8: !%16g", *(float *)&mips->cpr[1][8]); break;
		case CPUINFO_STR_REGISTER + MIPS3_FPD8:			sprintf(info->s, "FPD8: !%16g", *(double *)&mips->cpr[1][8]); break;
		case CPUINFO_STR_REGISTER + MIPS3_FPR9:			sprintf(info->s, "FPR9: %08X%08X", (UINT32)(mips->cpr[1][9] >> 32), (UINT32)mips->cpr[1][9]); break;
		case CPUINFO_STR_REGISTER + MIPS3_FPS9:			sprintf(info->s, "FPS9: !%16g", *(float *)&mips->cpr[1][9]); break;
		case CPUINFO_STR_REGISTER + MIPS3_FPD9:			sprintf(info->s, "FPD9: !%16g", *(double *)&mips->cpr[1][9]); break;
		case CPUINFO_STR_REGISTER + MIPS3_FPR10:		sprintf(info->s, "FPR10:%08X%08X", (UINT32)(mips->cpr[1][10] >> 32), (UINT32)mips->cpr[1][10]); break;
		case CPUINFO_STR_REGISTER + MIPS3_FPS10:		sprintf(info->s, "FPS10:!%16g", *(float *)&mips->cpr[1][10]); break;
		case CPUINFO_STR_REGISTER + MIPS3_FPD10:		sprintf(info->s, "FPD10:!%16g", *(double *)&mips->cpr[1][10]); break;
		case CPUINFO_STR_REGISTER + MIPS3_FPR11:		sprintf(info->s, "FPR11:%08X%08X", (UINT32)(mips->cpr[1][11] >> 32), (UINT32)mips->cpr[1][11]); break;
		case CPUINFO_STR_REGISTER + MIPS3_FPS11:		sprintf(info->s, "FPS11:!%16g", *(float *)&mips->cpr[1][11]); break;
		case CPUINFO_STR_REGISTER + MIPS3_FPD11:		sprintf(info->s, "FPD11:!%16g", *(double *)&mips->cpr[1][11]); break;
		case CPUINFO_STR_REGISTER + MIPS3_FPR12:		sprintf(info->s, "FPR12:%08X%08X", (UINT32)(mips->cpr[1][12] >> 32), (UINT32)mips->cpr[1][12]); break;
		case CPUINFO_STR_REGISTER + MIPS3_FPS12:		sprintf(info->s, "FPS12:!%16g", *(float *)&mips->cpr[1][12]); break;
		case CPUINFO_STR_REGISTER + MIPS3_FPD12:		sprintf(info->s, "FPD12:!%16g", *(double *)&mips->cpr[1][12]); break;
		case CPUINFO_STR_REGISTER + MIPS3_FPR13:		sprintf(info->s, "FPR13:%08X%08X", (UINT32)(mips->cpr[1][13] >> 32), (UINT32)mips->cpr[1][13]); break;
		case CPUINFO_STR_REGISTER + MIPS3_FPS13:		sprintf(info->s, "FPS13:!%16g", *(float *)&mips->cpr[1][13]); break;
		case CPUINFO_STR_REGISTER + MIPS3_FPD13:		sprintf(info->s, "FPD13:!%16g", *(double *)&mips->cpr[1][13]); break;
		case CPUINFO_STR_REGISTER + MIPS3_FPR14:		sprintf(info->s, "FPR14:%08X%08X", (UINT32)(mips->cpr[1][14] >> 32), (UINT32)mips->cpr[1][14]); break;
		case CPUINFO_STR_REGISTER + MIPS3_FPS14:		sprintf(info->s, "FPS14:!%16g", *(float *)&mips->cpr[1][14]); break;
		case CPUINFO_STR_REGISTER + MIPS3_FPD14:		sprintf(info->s, "FPD14:!%16g", *(double *)&mips->cpr[1][14]); break;
		case CPUINFO_STR_REGISTER + MIPS3_FPR15:		sprintf(info->s, "FPR15:%08X%08X", (UINT32)(mips->cpr[1][15] >> 32), (UINT32)mips->cpr[1][15]); break;
		case CPUINFO_STR_REGISTER + MIPS3_FPS15:		sprintf(info->s, "FPS15:!%16g", *(float *)&mips->cpr[1][15]); break;
		case CPUINFO_STR_REGISTER + MIPS3_FPD15:		sprintf(info->s, "FPD15:!%16g", *(double *)&mips->cpr[1][15]); break;
		case CPUINFO_STR_REGISTER + MIPS3_FPR16:		sprintf(info->s, "FPR16:%08X%08X", (UINT32)(mips->cpr[1][16] >> 32), (UINT32)mips->cpr[1][16]); break;
		case CPUINFO_STR_REGISTER + MIPS3_FPS16:		sprintf(info->s, "FPS16:!%16g", *(float *)&mips->cpr[1][16]); break;
		case CPUINFO_STR_REGISTER + MIPS3_FPD16:		sprintf(info->s, "FPD16:!%16g", *(double *)&mips->cpr[1][16]); break;
		case CPUINFO_STR_REGISTER + MIPS3_FPR17:		sprintf(info->s, "FPR17:%08X%08X", (UINT32)(mips->cpr[1][17] >> 32), (UINT32)mips->cpr[1][17]); break;
		case CPUINFO_STR_REGISTER + MIPS3_FPS17:		sprintf(info->s, "FPS17:!%16g", *(float *)&mips->cpr[1][17]); break;
		case CPUINFO_STR_REGISTER + MIPS3_FPD17:		sprintf(info->s, "FPD17:!%16g", *(double *)&mips->cpr[1][17]); break;
		case CPUINFO_STR_REGISTER + MIPS3_FPR18:		sprintf(info->s, "FPR18:%08X%08X", (UINT32)(mips->cpr[1][18] >> 32), (UINT32)mips->cpr[1][18]); break;
		case CPUINFO_STR_REGISTER + MIPS3_FPS18:		sprintf(info->s, "FPS18:!%16g", *(float *)&mips->cpr[1][18]); break;
		case CPUINFO_STR_REGISTER + MIPS3_FPD18:		sprintf(info->s, "FPD18:!%16g", *(double *)&mips->cpr[1][18]); break;
		case CPUINFO_STR_REGISTER + MIPS3_FPR19:		sprintf(info->s, "FPR19:%08X%08X", (UINT32)(mips->cpr[1][19] >> 32), (UINT32)mips->cpr[1][19]); break;
		case CPUINFO_STR_REGISTER + MIPS3_FPS19:		sprintf(info->s, "FPS19:!%16g", *(float *)&mips->cpr[1][19]); break;
		case CPUINFO_STR_REGISTER + MIPS3_FPD19:		sprintf(info->s, "FPD19:!%16g", *(double *)&mips->cpr[1][19]); break;
		case CPUINFO_STR_REGISTER + MIPS3_FPR20:		sprintf(info->s, "FPR20:%08X%08X", (UINT32)(mips->cpr[1][20] >> 32), (UINT32)mips->cpr[1][20]); break;
		case CPUINFO_STR_REGISTER + MIPS3_FPS20:		sprintf(info->s, "FPS20:!%16g", *(float *)&mips->cpr[1][20]); break;
		case CPUINFO_STR_REGISTER + MIPS3_FPD20:		sprintf(info->s, "FPD20:!%16g", *(double *)&mips->cpr[1][20]); break;
		case CPUINFO_STR_REGISTER + MIPS3_FPR21:		sprintf(info->s, "FPR21:%08X%08X", (UINT32)(mips->cpr[1][21] >> 32), (UINT32)mips->cpr[1][21]); break;
		case CPUINFO_STR_REGISTER + MIPS3_FPS21:		sprintf(info->s, "FPS21:!%16g", *(float *)&mips->cpr[1][21]); break;
		case CPUINFO_STR_REGISTER + MIPS3_FPD21:		sprintf(info->s, "FPD21:!%16g", *(double *)&mips->cpr[1][21]); break;
		case CPUINFO_STR_REGISTER + MIPS3_FPR22:		sprintf(info->s, "FPR22:%08X%08X", (UINT32)(mips->cpr[1][22] >> 32), (UINT32)mips->cpr[1][22]); break;
		case CPUINFO_STR_REGISTER + MIPS3_FPS22:		sprintf(info->s, "FPS22:!%16g", *(float *)&mips->cpr[1][22]); break;
		case CPUINFO_STR_REGISTER + MIPS3_FPD22:		sprintf(info->s, "FPD22:!%16g", *(double *)&mips->cpr[1][22]); break;
		case CPUINFO_STR_REGISTER + MIPS3_FPR23:		sprintf(info->s, "FPR23:%08X%08X", (UINT32)(mips->cpr[1][23] >> 32), (UINT32)mips->cpr[1][23]); break;
		case CPUINFO_STR_REGISTER + MIPS3_FPS23:		sprintf(info->s, "FPS23:!%16g", *(float *)&mips->cpr[1][23]); break;
		case CPUINFO_STR_REGISTER + MIPS3_FPD23:		sprintf(info->s, "FPD23:!%16g", *(double *)&mips->cpr[1][23]); break;
		case CPUINFO_STR_REGISTER + MIPS3_FPR24:		sprintf(info->s, "FPR24:%08X%08X", (UINT32)(mips->cpr[1][24] >> 32), (UINT32)mips->cpr[1][24]); break;
		case CPUINFO_STR_REGISTER + MIPS3_FPS24:		sprintf(info->s, "FPS24:!%16g", *(float *)&mips->cpr[1][24]); break;
		case CPUINFO_STR_REGISTER + MIPS3_FPD24:		sprintf(info->s, "FPD24:!%16g", *(double *)&mips->cpr[1][24]); break;
		case CPUINFO_STR_REGISTER + MIPS3_FPR25:		sprintf(info->s, "FPR25:%08X%08X", (UINT32)(mips->cpr[1][25] >> 32), (UINT32)mips->cpr[1][25]); break;
		case CPUINFO_STR_REGISTER + MIPS3_FPS25:		sprintf(info->s, "FPS25:!%16g", *(float *)&mips->cpr[1][25]); break;
		case CPUINFO_STR_REGISTER + MIPS3_FPD25:		sprintf(info->s, "FPD25:!%16g", *(double *)&mips->cpr[1][25]); break;
		case CPUINFO_STR_REGISTER + MIPS3_FPR26:		sprintf(info->s, "FPR26:%08X%08X", (UINT32)(mips->cpr[1][26] >> 32), (UINT32)mips->cpr[1][26]); break;
		case CPUINFO_STR_REGISTER + MIPS3_FPS26:		sprintf(info->s, "FPS26:!%16g", *(float *)&mips->cpr[1][26]); break;
		case CPUINFO_STR_REGISTER + MIPS3_FPD26:		sprintf(info->s, "FPD26:!%16g", *(double *)&mips->cpr[1][26]); break;
		case CPUINFO_STR_REGISTER + MIPS3_FPR27:		sprintf(info->s, "FPR27:%08X%08X", (UINT32)(mips->cpr[1][27] >> 32), (UINT32)mips->cpr[1][27]); break;
		case CPUINFO_STR_REGISTER + MIPS3_FPS27:		sprintf(info->s, "FPS27:!%16g", *(float *)&mips->cpr[1][27]); break;
		case CPUINFO_STR_REGISTER + MIPS3_FPD27:		sprintf(info->s, "FPD27:!%16g", *(double *)&mips->cpr[1][27]); break;
		case CPUINFO_STR_REGISTER + MIPS3_FPR28:		sprintf(info->s, "FPR28:%08X%08X", (UINT32)(mips->cpr[1][28] >> 32), (UINT32)mips->cpr[1][28]); break;
		case CPUINFO_STR_REGISTER + MIPS3_FPS28:		sprintf(info->s, "FPS28:!%16g", *(float *)&mips->cpr[1][28]); break;
		case CPUINFO_STR_REGISTER + MIPS3_FPD28:		sprintf(info->s, "FPD28:!%16g", *(double *)&mips->cpr[1][28]); break;
		case CPUINFO_STR_REGISTER + MIPS3_FPR29:		sprintf(info->s, "FPR29:%08X%08X", (UINT32)(mips->cpr[1][29] >> 32), (UINT32)mips->cpr[1][29]); break;
		case CPUINFO_STR_REGISTER + MIPS3_FPS29:		sprintf(info->s, "FPS29:!%16g", *(float *)&mips->cpr[1][29]); break;
		case CPUINFO_STR_REGISTER + MIPS3_FPD29:		sprintf(info->s, "FPD29:!%16g", *(double *)&mips->cpr[1][29]); break;
		case CPUINFO_STR_REGISTER + MIPS3_FPR30:		sprintf(info->s, "FPR30:%08X%08X", (UINT32)(mips->cpr[1][30] >> 32), (UINT32)mips->cpr[1][30]); break;
		case CPUINFO_STR_REGISTER + MIPS3_FPS30:		sprintf(info->s, "FPS30:!%16g", *(float *)&mips->cpr[1][30]); break;
		case CPUINFO_STR_REGISTER + MIPS3_FPD30:		sprintf(info->s, "FPD30:!%16g", *(double *)&mips->cpr[1][30]); break;
		case CPUINFO_STR_REGISTER + MIPS3_FPR31:		sprintf(info->s, "FPR31:%08X%08X", (UINT32)(mips->cpr[1][31] >> 32), (UINT32)mips->cpr[1][31]); break;
		case CPUINFO_STR_REGISTER + MIPS3_FPS31:		sprintf(info->s, "FPS31:!%16g", *(float *)&mips->cpr[1][31]); break;
		case CPUINFO_STR_REGISTER + MIPS3_FPD31:		sprintf(info->s, "FPD31:!%16g", *(double *)&mips->cpr[1][31]); break;
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
	legacy_cpu_device *device = (legacy_cpu_device *)ptr;
	device_set_input_line(device, MIPS3_IRQ5, ASSERT_LINE);
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

	// NEC VR series does not use a 100% compatible COP0/TLB implementation
	if (mips->flavor == MIPS3_TYPE_VR4300)
	{
		/*
            For VR43xx, Config is as follows:
            bit 31 = always 0
            bits 28-30 = EC
            bits 24-27 = EP
            bits 16-23 = always b0000010
            bit 15 = endian indicator as standard MIPS III
            bits 4-14 = always b11001000110
            bit 3 = CU
            bits 0-2 = K0 ("Coherency algorithm of kseg0")
        */

		configreg = 0x6460;
	}
	else
	{
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
	}

	/* set the endianness bit */
	if (mips->bigendian)
		configreg |= 0x00008000;

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
		case MIPS3_TYPE_VR4300:
			return 0x0b00;

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
    tlb_map_entry - map a single TLB
    entry
-------------------------------------------------*/

static void tlb_map_entry(mips3_state *mips, int tlbindex)
{
	int current_asid = mips->cpr[0][COP0_EntryHi] & 0xff;
	mips3_tlb_entry *entry = &mips->tlb[tlbindex];
	UINT32 count, vpn;
	int which;

	/* the ASID doesn't match the current ASID, and if the page isn't global, unmap it from the TLB */
	if (!tlb_entry_matches_asid(entry, current_asid) && !tlb_entry_is_global(entry))
	{
		vtlb_load(mips->vtlb, 2 * tlbindex + 0, 0, 0, 0);
		vtlb_load(mips->vtlb, 2 * tlbindex + 1, 0, 0, 0);
		return;
	}

	/* extract the VPN index; ignore if the virtual address is beyond 32 bits */
	vpn = ((entry->entry_hi >> 13) & 0x07ffffff) << 1;
	if (vpn >= (1 << (MIPS3_MAX_PADDR_SHIFT - MIPS3_MIN_PAGE_SHIFT)))
	{
		vtlb_load(mips->vtlb, 2 * tlbindex + 0, 0, 0, 0);
		vtlb_load(mips->vtlb, 2 * tlbindex + 1, 0, 0, 0);
		return;
	}

	/* get the number of pages from the page mask */
	count = ((entry->page_mask >> 13) & 0x00fff) + 1;

	/* loop over both the even and odd pages */
	for (which = 0; which < 2; which++)
	{
		UINT32 effvpn = vpn + count * which;
		UINT64 lo = entry->entry_lo[which];
		UINT32 pfn;
		UINT32 flags = 0;

		/* compute physical page index */
		pfn = (lo >> 6) & mips->pfnmask;

		/* valid? */
		if ((lo & 2) != 0)
		{
			flags |= VTLB_FLAG_VALID | VTLB_READ_ALLOWED | VTLB_FETCH_ALLOWED;

			/* writable? */
			if ((lo & 4) != 0)
				flags |= VTLB_WRITE_ALLOWED;

			/* mirror the flags for user mode if the VPN is in user space */
			if (effvpn < (0x80000000 >> MIPS3_MIN_PAGE_SHIFT))
				flags |= (flags << 4) & (VTLB_USER_READ_ALLOWED | VTLB_USER_WRITE_ALLOWED | VTLB_USER_FETCH_ALLOWED);
		}

		/* load the virtual TLB with the corresponding entries */
		if ((effvpn + count) <= (0x80000000 >> MIPS3_MIN_PAGE_SHIFT) || effvpn >= (0xc0000000 >> MIPS3_MIN_PAGE_SHIFT))
			vtlb_load(mips->vtlb, 2 * tlbindex + which, count, effvpn << MIPS3_MIN_PAGE_SHIFT, (pfn << MIPS3_MIN_PAGE_SHIFT) | flags);
		else
			vtlb_load(mips->vtlb, 2 * tlbindex + which, 0, 0, 0);
	}
}


/*-------------------------------------------------
    tlb_write_common - common routine for writing
    a TLB entry
-------------------------------------------------*/

static void tlb_write_common(mips3_state *mips, int tlbindex)
{
	/* only handle entries within the TLB */
	if (tlbindex < mips->tlbentries)
	{
		mips3_tlb_entry *entry = &mips->tlb[tlbindex];

		/* fill in the new TLB entry from the COP0 registers */
		entry->page_mask = mips->cpr[0][COP0_PageMask];
		entry->entry_hi = mips->cpr[0][COP0_EntryHi] & ~(entry->page_mask & U64(0x0000000001ffe000));
		entry->entry_lo[0] = mips->cpr[0][COP0_EntryLo0];
		entry->entry_lo[1] = mips->cpr[0][COP0_EntryLo1];

		/* remap this TLB entry */
		tlb_map_entry(mips, tlbindex);

		/* log the two halves once they are in */
		tlb_entry_log_half(entry, tlbindex, 0);
		tlb_entry_log_half(entry, tlbindex, 1);
	}
}


/*-------------------------------------------------
    tlb_entry_log_half - log half of a single TLB
    entry
-------------------------------------------------*/

static void tlb_entry_log_half(mips3_tlb_entry *entry, int tlbindex, int which)
{
if (PRINTF_TLB)
{
	UINT64 hi = entry->entry_hi;
	UINT64 lo = entry->entry_lo[which];
	UINT32 vpn = (((hi >> 13) & 0x07ffffff) << 1);
	UINT32 asid = hi & 0xff;
	UINT32 r = (hi >> 62) & 3;
	UINT32 pfn = (lo >> 6) & 0x00ffffff;
	UINT32 c = (lo >> 3) & 7;
	UINT32 pagesize = (((entry->page_mask >> 13) & 0xfff) + 1) << MIPS3_MIN_PAGE_SHIFT;
	UINT64 vaddr = (UINT64)vpn * MIPS3_MIN_PAGE_SIZE;
	UINT64 paddr = (UINT64)pfn * MIPS3_MIN_PAGE_SIZE;

	vaddr += pagesize * which;

	printf("index=%08X  pagesize=%08X  vaddr=%08X%08X  paddr=%08X%08X  asid=%02X  r=%X  c=%X  dvg=%c%c%c\n",
			tlbindex, pagesize, (UINT32)(vaddr >> 32), (UINT32)vaddr, (UINT32)(paddr >> 32), (UINT32)paddr,
			asid, r, c, (lo & 4) ? 'd' : '.', (lo & 2) ? 'v' : '.', (lo & 1) ? 'g' : '.');
}
}
