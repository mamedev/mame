// license:BSD-3-Clause
// copyright-holders:Aaron Giles
/***************************************************************************

    mips3com.c

    Common MIPS III/IV definitions and functions

***************************************************************************/

#include "emu.h"
#include "mips3com.h"
#include "ps2vu.h"


/***************************************************************************
    FUNCTION PROTOTYPES
***************************************************************************/

static void tlb_entry_log_half(mips3_tlb_entry *entry, int tlbindex, int which);



/***************************************************************************
    INLINE FUNCTIONS
***************************************************************************/

/*-------------------------------------------------
    tlb_entry_matches_asid - true if the given
    TLB entry matches the provided ASID
-------------------------------------------------*/

static inline bool tlb_entry_matches_asid(const mips3_tlb_entry *entry, uint8_t asid)
{
	return (entry->entry_hi & 0xff) == asid;
}


/*-------------------------------------------------
    tlb_entry_is_global - true if the given
    TLB entry is global
-------------------------------------------------*/

static inline bool tlb_entry_is_global(const mips3_tlb_entry *entry)
{
	return (entry->entry_lo[0] & entry->entry_lo[1] & TLB_GLOBAL);
}


void mips3_device::execute_set_input(int inputnum, int state)
{
	if (inputnum >= MIPS3_IRQ0 && inputnum <= MIPS3_IRQ5)
	{
		if (state != CLEAR_LINE)
			m_core->cpr[0][COP0_Cause] |= 0x400 << inputnum;
		else
			m_core->cpr[0][COP0_Cause] &= ~(0x400 << inputnum);
	}
}


/***************************************************************************
    INITIALIZATION AND SHUTDOWN
***************************************************************************/

/*-------------------------------------------------
    mips3com_update_cycle_counting - update cycle
    counts and the timers
-------------------------------------------------*/

void mips3_device::mips3com_update_cycle_counting()
{
	/* modify the timer to go off */
	if (m_core->compare_armed)
	{
		uint32_t count = (total_cycles() - m_core->count_zero_time) / 2;
		uint32_t compare = m_core->cpr[0][COP0_Compare];
		uint32_t delta = compare - count;
		m_core->compare_armed = 0;
		attotime newtime = cycles_to_attotime((uint64_t)delta * 2);
		m_compare_int_timer->adjust(newtime);
		return;
	}
}

/***************************************************************************
    TLB HANDLING
***************************************************************************/

/*-------------------------------------------------
    mips3com_asid_changed - remap all non-global
    TLB entries
-------------------------------------------------*/

void mips3_device::mips3com_asid_changed()
{
	int tlbindex;

	/* iterate over all non-global TLB entries and remap them */
	for (tlbindex = 0; tlbindex < m_tlbentries; tlbindex++)
		if (!tlb_entry_is_global(&m_tlb[tlbindex]))
			tlb_map_entry(tlbindex);
}


/*-------------------------------------------------
    mips3com_tlbr - execute the tlbr instruction
-------------------------------------------------*/

void mips3_device::mips3com_tlbr()
{
	uint32_t tlbindex = m_core->cpr[0][COP0_Index] & 0x3f;

	/* only handle entries within the TLB */
	if (tlbindex < m_tlbentries)
	{
		mips3_tlb_entry *entry = &m_tlb[tlbindex];

		/* copy data from the TLB entry into the COP0 registers */
		m_core->cpr[0][COP0_PageMask] = entry->page_mask;
		m_core->cpr[0][COP0_EntryHi] = entry->entry_hi;
		m_core->cpr[0][COP0_EntryLo0] = entry->entry_lo[0];
		m_core->cpr[0][COP0_EntryLo1] = entry->entry_lo[1];
	}
}


/*-------------------------------------------------
    mips3com_tlbwi - execute the tlbwi instruction
-------------------------------------------------*/

void mips3_device::mips3com_tlbwi()
{
	/* use the common handler and write based off the COP0 Index register */
	tlb_write_common(m_core->cpr[0][COP0_Index] & 0x3f);
}


/*-------------------------------------------------
generate_tlb_index - generate a random tlb index
-------------------------------------------------*/

uint32_t mips3_device::generate_tlb_index()
{
	// Actual hardware uses a free running counter to generate the index.
	// This impementation uses a linear congruential generator so that DRC and non-DRC code sequences match.
	m_tlb_seed = 214013 * m_tlb_seed + 2531011;
	return (m_tlb_seed >> 16) & 0x3f;
}

/*-------------------------------------------------
    mips3com_tlbwr - execute the tlbwr instruction
-------------------------------------------------*/

void mips3_device::mips3com_tlbwr()
{
	uint32_t wired = m_core->cpr[0][COP0_Wired] & 0x3f;
	uint32_t unwired = m_tlbentries - wired;
	uint32_t tlbindex = m_tlbentries - 1;

	/* "random" is based off of linear congruential sequence through the non-wired pages */
	if (unwired > 0)
		tlbindex = (generate_tlb_index() % unwired) + wired;

	/* use the common handler to write to this tlbindex */
	tlb_write_common(tlbindex);
}


/*-------------------------------------------------
    mips3com_tlbp - execute the tlbp instruction
-------------------------------------------------*/

void mips3_device::mips3com_tlbp()
{
	uint32_t tlbindex;

	/* iterate over TLB entries */
	for (tlbindex = 0; tlbindex < m_tlbentries; tlbindex++)
	{
		mips3_tlb_entry *entry = &m_tlb[tlbindex];
		uint64_t mask = ~((entry->page_mask >> 13) & 0xfff) << 13;

		/* if the relevant bits of EntryHi match the relevant bits of the TLB */
		if ((entry->entry_hi & mask) == (m_core->cpr[0][COP0_EntryHi] & mask))

			/* and if we are either global or matching the current ASID, then stop */
			if ((entry->entry_hi & 0xff) == (m_core->cpr[0][COP0_EntryHi] & 0xff) || ((entry->entry_lo[0] & entry->entry_lo[1]) & TLB_GLOBAL))
				break;
	}

	/* validate that our tlb_table was in sync */
//  vpn = ((m_cores->cpr[0][COP0_EntryHi] >> 13) & 0x07ffffff) << 1;
	if (tlbindex != m_tlbentries)
		m_core->cpr[0][COP0_Index] = tlbindex;
	else
		m_core->cpr[0][COP0_Index] = 0x80000000;
}



/***************************************************************************
    INTERNAL HELPERS
***************************************************************************/

/*-------------------------------------------------
    compare_int_callback - callback that fires
    whenever a compare interrupt is generated
-------------------------------------------------*/

TIMER_CALLBACK_MEMBER( mips3_device::compare_int_callback )
{
	m_compare_int_timer->adjust(attotime::never);
	set_input_line(MIPS3_IRQ5, ASSERT_LINE);
}


/*-------------------------------------------------
    compute_config_register - compute the value
    of the config register
-------------------------------------------------*/

uint32_t mips3_device::compute_config_register()
{
	/* set the cache line size to 32 bytes */
	uint32_t configreg = 0x00026030;

	m_dcache = nullptr;
	m_icache = nullptr;

	// NEC VR series does not use a 100% compatible COP0/TLB implementation
	if (m_flavor == MIPS3_TYPE_VR4300)
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
	else if (m_flavor == MIPS3_TYPE_VR5500)
	{
		/*
		    For VR55xx, Config is as follows:
		    bit 31 = always 0
		    bits 28-30 = EC
		    bits 24-27 = EP
		    bits 23-22 = EM
		    bits 21-20 = always b11
		    bits 19-18 = EW
		    bits 17-16 = always b10
		    bit 15 = endian indicator as standard MIPS III
		    bits 3-14 = always b110011011110
		    bits 0-2 = K0 ("Coherency algorithm of kseg0")
		*/

		configreg = 0x6460;
	}
	else
	{
		/* set the data cache size */
				if (c_dcache_size <= 0x01000) configreg |= 0 << 6;
		else if (c_dcache_size <= 0x02000) configreg |= 1 << 6;
		else if (c_dcache_size <= 0x04000) configreg |= 2 << 6;
		else if (c_dcache_size <= 0x08000) configreg |= 3 << 6;
		else if (c_dcache_size <= 0x10000) configreg |= 4 << 6;
		else if (c_dcache_size <= 0x20000) configreg |= 5 << 6;
		else if (c_dcache_size <= 0x40000) configreg |= 6 << 6;
		else                                   configreg |= 7 << 6;

		/* set the instruction cache size */
				if (c_icache_size <= 0x01000) configreg |= 0 << 9;
		else if (c_icache_size <= 0x02000) configreg |= 1 << 9;
		else if (c_icache_size <= 0x04000) configreg |= 2 << 9;
		else if (c_icache_size <= 0x08000) configreg |= 3 << 9;
		else if (c_icache_size <= 0x10000) configreg |= 4 << 9;
		else if (c_icache_size <= 0x20000) configreg |= 5 << 9;
		else if (c_icache_size <= 0x40000) configreg |= 6 << 9;
		else                                   configreg |= 7 << 9;

		if (c_secondary_cache_line_size != 0) {
			configreg &= ~((0xf << 20) | (1 << 17));
					if (c_secondary_cache_line_size <= 0x10) configreg |= 0 << 22;
			else if (c_secondary_cache_line_size <= 0x20) configreg |= 1 << 22;
			else if (c_secondary_cache_line_size <= 0x40) configreg |= 2 << 22;
			else                                          configreg |= 3 << 22;
		}
		/* set the system clock divider */
		int divisor = 2;
		if (c_system_clock != 0)
		{
			divisor = m_cpu_clock / c_system_clock;
			if (c_system_clock * divisor != m_cpu_clock)
			{
				configreg |= 0x80000000;
				divisor = m_cpu_clock * 2 / c_system_clock;
			}
		}
		configreg |= (((divisor < 2) ? 2 : (divisor > 8) ? 8 : divisor) - 2) << 28;
	}

	/* set the endianness bit */
	if (m_bigendian)
		configreg |= 0x00008000;

	return configreg;
}


/*-------------------------------------------------
    compute_prid_register - compute the value
    of the PRId register
-------------------------------------------------*/

uint32_t mips3_device::compute_prid_register()
{
	switch (m_flavor)
	{
		case MIPS3_TYPE_R4000:
			return 0x0400;

		case MIPS3_TYPE_R4400:
			return 0x0440;

		case MIPS3_TYPE_VR4300:
			return 0x0b00;

		case MIPS3_TYPE_VR5500:
			return 0x5500;

		case MIPS3_TYPE_R4600:
		case MIPS3_TYPE_R4650:
			return 0x2000;

		case MIPS3_TYPE_R4700:
			return 0x2100;

		case MIPS3_TYPE_TX4925:
			return 0x2d23;

		case MIPS3_TYPE_R5000:
			return 0x2300;

		case MIPS3_TYPE_QED5271:
			return 0x2800;

		case MIPS3_TYPE_RM7000:
			return 0x2700;

		case MIPS3_TYPE_R5900:
			return 0x2e14;

		default:
			fatalerror("Unknown MIPS flavor specified\n");
	}
	// never executed
	//return 0x2000;
}

/*-------------------------------------------------
    tlb_map_entry - map a single TLB
    entry
-------------------------------------------------*/

void mips3_device::tlb_map_entry(int tlbindex)
{
	int current_asid = m_core->cpr[0][COP0_EntryHi] & 0xff;
	mips3_tlb_entry *entry = &m_tlb[tlbindex];
	uint32_t count, vpn;
	int which;

	/* the ASID doesn't match the current ASID, and if the page isn't global, unmap it from the TLB */
	if (!tlb_entry_matches_asid(entry, current_asid) && !tlb_entry_is_global(entry))
	{
		vtlb_load(2 * tlbindex + 0, 0, 0, 0);
		vtlb_load(2 * tlbindex + 1, 0, 0, 0);
		return;
	}

	/* extract the VPN index; ignore if the virtual address is beyond 32 bits */
	vpn = ((entry->entry_hi >> 13) & 0x07ffffff) << 1;
	if (vpn >= (1 << (MIPS3_MAX_PADDR_SHIFT - MIPS3_MIN_PAGE_SHIFT)))
	{
		vtlb_load(2 * tlbindex + 0, 0, 0, 0);
		vtlb_load(2 * tlbindex + 1, 0, 0, 0);
		return;
	}

	/* get the number of pages from the page mask */
	/* R5900: if the S bit is set in EntryLo, it is the scratchpad, and is always 4 pages. */
	if ((entry->entry_lo[0] & 0x80000000) && m_flavor == MIPS3_TYPE_R5900)
		count = 4;
	else
		count = ((entry->page_mask >> 13) & 0x00fff) + 1;

	/* loop over both the even and odd pages */
	for (which = 0; which < 2; which++)
	{
		uint32_t effvpn = vpn + count * which;
		uint64_t lo = entry->entry_lo[which];
		uint32_t pfn;
		uint32_t flags = 0;

		/* compute physical page index */
		pfn = (lo >> 6) & m_pfnmask;

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
			vtlb_load(2 * tlbindex + which, count, effvpn << MIPS3_MIN_PAGE_SHIFT, (pfn << MIPS3_MIN_PAGE_SHIFT) | flags);
		else
			vtlb_load(2 * tlbindex + which, 0, 0, 0);
	}
}


/*-------------------------------------------------
    tlb_write_common - common routine for writing
    a TLB entry
-------------------------------------------------*/

void mips3_device::tlb_write_common(int tlbindex)
{
	/* only handle entries within the TLB */
	if (tlbindex < m_tlbentries)
	{
		mips3_tlb_entry *entry = &m_tlb[tlbindex];

		/* fill in the new TLB entry from the COP0 registers */
		entry->page_mask = m_core->cpr[0][COP0_PageMask];
		entry->entry_hi = m_core->cpr[0][COP0_EntryHi] & ~(entry->page_mask & u64(0x0000000001ffe000U));
		entry->entry_lo[0] = m_core->cpr[0][COP0_EntryLo0];
		entry->entry_lo[1] = m_core->cpr[0][COP0_EntryLo1];

		/* remap this TLB entry */
		tlb_map_entry(tlbindex);
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
	uint64_t hi = entry->entry_hi;
	uint64_t lo = entry->entry_lo[which];
	uint32_t vpn = (((hi >> 13) & 0x07ffffff) << 1);
	uint32_t asid = hi & 0xff;
	uint32_t r = (hi >> 62) & 3;
	uint32_t pfn = (lo >> 6) & 0x00ffffff;
	uint32_t c = (lo >> 3) & 7;
	uint32_t pagesize = (((entry->page_mask >> 13) & 0xfff) + 1) << MIPS3_MIN_PAGE_SHIFT;
	uint64_t vaddr = (uint64_t)vpn * MIPS3_MIN_PAGE_SIZE;
	uint64_t paddr = (uint64_t)pfn * MIPS3_MIN_PAGE_SIZE;

	vaddr += pagesize * which;

	printf("index=%08X  pagesize=%08X  vaddr=%08X%08X  paddr=%08X%08X  asid=%02X  r=%X  c=%X  dvg=%c%c%c\n",
			tlbindex, pagesize, (uint32_t)(vaddr >> 32), (uint32_t)vaddr, (uint32_t)(paddr >> 32), (uint32_t)paddr,
			asid, r, c, (lo & 4) ? 'd' : '.', (lo & 2) ? 'v' : '.', (lo & 1) ? 'g' : '.');
}
}
