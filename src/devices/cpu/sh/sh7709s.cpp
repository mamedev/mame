// license:BSD-3-Clause
// copyright-holders:Jesus Ramos

// SH 7709S experimental cache/memory timing
// Some values/timing tables are hardcoded
// (such as no bank mode timing/sdram access logic, and some area mappings)
// for cv1k emulation specifically

// Unimplemented functionality:
// - Write locking of cache lines
// - Prefix instruction fetch timing + cache handling
// - Checks for certain area specifics : bank mode timing for sdram access, area check for sdram, area check for burst mode + burst size
// - TLB simulation + timing and lookups - Unused by cv1k
// - Branch predicition/mispredict penalties

/* cv1k notes
*
* - Most slowdown is caused from the working size of the current frame exceeding the cache size
*   causing a lot of acceses to memory leaving the cpu idling. Cave doesn't seem to use prefetch
*   to alleviate this anywhere so you're just cache trashing at a certain point. This is what leads
*   to the iconic slingshotting in early titles such as Mushihimesama + Futari where you go from
*   sections of slowdown right back to regular speed as bullets leave the screen.
*
* - There seems to be a bug in the ROM read code in cv1k titles that is present in all of them.
*   This code when ROM reads are queued for whatever reason doesn't check if a read or write
*   is done and always ends up doing an expensive cache flush + invalidate that isn't required.
*   The processor ends up churning a bunch of cycles on uncached fetches and then also has to repopulate
*   the cache.
*
*   This requires looping 1024 times over every cache entry from an uncached aliased mapping in SDRAM
*   which comes with hefty access penalties due to all the instruction fetches + some data fetches.
*   While the code is present in every cv1k title the performance problems start manifesting more and
*   more in the later titles, the final two cv1k titles being the worst offenders with regards to
*   slowdown sections that at a glance don't look like they should be slowing down or causing extra
*   slowdown.
*
* - The code that reads irr0 to check for irq2 also seems to have a ready modify write timing bug where
*   the register value is read, IRQ2 is masked out of that value, then that masked value is written back.
*   Documentation does mention in an addendum that this behavior can lead to lost IRQ's but luckily these
*   games don't seem to actually use them in the release versions.
*
* - A quirk according to the docs for the sh7709s is that all areas must have the same bus width to use
*   bank active mode. The cv1k board uses a 16 bit rom in area 0 which means the SDRAM is now stuck
*   using auto precharge mode that forces a row close after every access which leads to some extra
*   delays when cache misses collide on the same bank back to back.
*/

#include "emu.h"
#include "sh7709s.h"
#include "sh3comn.h"
#include "sh4comn.h"
#include "cpu/drcumlsh.h"

#include "endianness.h"

DEFINE_DEVICE_TYPE(SH7709S, sh7709s_device, "sh7709s", "Hitachi SH7709S")

sh7709s_device::sh7709s_device(const machine_config& mconfig, const char* tag, device_t* owner, uint32_t clock, endianness_t endianness)
	: sh3_base_device(mconfig, SH7709S, tag, owner, clock, endianness)
{
}

void sh7709s_device::device_reset()
{
	sh3_base_device::device_reset();

	memset(m_cache, 0, sizeof(m_cache));
	m_wb_address = 0;
	m_last_area_accessed = 0;
	m_last_area_accessed_was_write = false;
	m_wb_active_cycles = 0;
	m_last_sdram_bank = 0;
	m_precharge_remaining_cycles = 0;
	m_last_op_cycle_count = 0;
}

void sh7709s_device::device_start()
{
	sh3_base_device::device_start();

	memset(m_cache, 0, sizeof(m_cache));
	m_wb_address = 0;
	m_last_area_accessed = 0;
	m_last_area_accessed_was_write = false;
	m_wb_active_cycles = 0;
	m_last_sdram_bank = 0;
	m_precharge_remaining_cycles = 0;
	m_last_op_cycle_count = 0;

	for (int i = 0; i < SH7709S_CACHE_BLOCKS; i++)
		for (int j = 0; j < SH7709S_CACHE_ASSOCIATIVITY; j++)
		{
			save_item(NAME(m_cache[i][j].tag), j + (i * SH7709S_CACHE_BLOCKS));
			save_item(NAME(m_cache[i][j].lru), j + (i * SH7709S_CACHE_BLOCKS));
			save_item(NAME(m_cache[i][j].dirty), j + (i * SH7709S_CACHE_BLOCKS));
		}

	save_item(NAME(m_wb_address));
	save_item(NAME(m_last_area_accessed));
	save_item(NAME(m_last_area_accessed_was_write));
	save_item(NAME(m_wb_active_cycles));
	save_item(NAME(m_last_sdram_bank));
	save_item(NAME(m_precharge_remaining_cycles));
	save_item(NAME(m_last_op_cycle_count));
}

// Top 3 region bits allow for aliasing cached/uncached pointers to the same physical address
bool is_cacheable(uint32_t address)
{
	// 0x00000000 - 0x80000000 2GB cacheable virtual space
	// 0x80000000 - 0xA0000000 0.5GB fixed physical cacheable space
	// 0xC0000000 - 0xE0000000 0.5GB virtual cacheable space
	uint8_t region = address >> 29;
	return region != 0x5 && region != 0x7;
}

uint32_t get_area(uint32_t address)
{
	// Mask to 29 bit physical space
	uint32_t phys_mask = address & SH34_AM;

	return phys_mask >> 26;
}

static bool is_sdram_region(uint32_t address)
{
	unsigned int area = get_area(address);

	// Hardcoded for cv1k, assumes 3 mapped to SDRAM
	return area == 3;
}

// Returns true on cache hit, false if there was a miss cache miss and updates the cache state to reflect the access
bool sh7709s_device::cache_access(uint32_t address, bool write)
{
	if (!is_cacheable(address))
		return false;

	// Programs should ensure that 2 pointers cannot alias to the same physical address
	// as the cache discards the top 3 region bits
	address &= SH34_AM;

	uint32_t cache_address = address / SH7709S_CACHE_LINE_SIZE;
	uint32_t cache_block = cache_address % SH7709S_CACHE_BLOCKS;

	for (int i = 0; i < SH7709S_CACHE_ASSOCIATIVITY; i++)
	{
		struct sh7709s_cache_entry *entry = &m_cache[cache_block][i];
		if (entry->tag == cache_address)
		{
			if (write)
				entry->dirty = 1;
			// If the entry is already the latest in the LRU no need to update anything else
			if (entry->lru != SH7709S_CACHE_ASSOCIATIVITY - 1)
			{
				for (int j = 0; j < SH7709S_CACHE_ASSOCIATIVITY; j++)
				{
					struct sh7709s_cache_entry* update_entry = &m_cache[cache_block][j];
					if (update_entry->lru > entry->lru)
						update_entry->lru--;
				}
				entry->lru = SH7709S_CACHE_ASSOCIATIVITY - 1;
			}
			return true;
		}
	}

	// We didn't find the entry in the cache so it's time to evict a cache line
	for (int i = 0; i < SH7709S_CACHE_ASSOCIATIVITY; i++)
	{
		struct sh7709s_cache_entry* entry = &m_cache[cache_block][i];
		if (entry->lru == 0)
		{
			if (entry->dirty)
				m_wb_address = entry->tag * SH7709S_CACHE_LINE_SIZE;
			entry->tag = cache_address;
			entry->lru = SH7709S_CACHE_ASSOCIATIVITY - 1;
			entry->dirty = write;
			for (int j = 0; j < SH7709S_CACHE_ASSOCIATIVITY; j++)
			{
				struct sh7709s_cache_entry* update_entry = &m_cache[cache_block][j];
				if (i != j && update_entry->lru != 0)
					update_entry->lru--;
			}
			break;
		}
	}

	return false;
}

bool can_use_burst(uint32_t address)
{
	// Assumes only sdram can burst read/write
	return is_sdram_region(address);
}

uint32_t sh7709s_device::get_wcr1_timing(uint32_t address)
{
	uint32_t area = get_area(address);

	if (area > 6 || area == 1)
		return 0;

	unsigned int area_val = (m_wcr1 >> (area * 2)) & 0x3;

	if (area_val == 0)
		return 1;

	return area_val;
}

uint32_t sh7709s_device::get_wcr2_timing(uint32_t address)
{
	uint32_t area = get_area(address);
	uint16_t wcr2 = m_wcr2;

	if (area > 6 || area == 1)
		return 0;

	unsigned int area_val = 0;

	if (area == 0)
	{
		area_val = wcr2 & 0x7;
	}
	else if (area == 2 || area == 3) // These areas have their own timings
	{
		wcr2 >>= 3;
		if (area == 3)
			wcr2 >>= 2;
		area_val = wcr2 & 0x3;
		if (area_val == 0)
			return 1;
		return area_val;
	}
	else
	{
		wcr2 >>= 7 + ((area - 4) * 3);
		area_val = wcr2 & 0x7;
	}

	switch (area_val)
	{
	case 0: return 0;
	case 1: return 1;
	case 2: return 2;
	case 3: return 3;
	case 4: return 4;
	case 5: return 6;
	case 6: return 8;
	case 7: return 10;
	}

	return 2; // Unreachable
}

uint32_t sh7709s_device::sdram_bank(uint32_t address)
{
	uint32_t amx = (m_mcr >> 3) & 0xf;

	switch (amx)
	{
	case 0x4: // row begins with A9: 1M x 16-bit x 4-bank
	case 0x7: // row begins with A9: 512k x 32-bit x 4-bank
		return (address >> 7) & 0x3;   // A8:A7

	case 0x5: // row begins with A10: 2M x 8/16-bit x 4-bank
	case 0xd: // row begins with A10: 4M x 16-bit x 4-bank
		return (address >> 8) & 0x3;   // A9:A8

	case 0xa: // row begins with A11: 8M x 16-bit x 4-bank
		return (address >> 9) & 0x3;   // A10:A9
	}

	return 0;
}

uint32_t sh7709s_device::mcr_tpc()
{
	return ((m_mcr >> 14) & 0x3) + 1;
}

uint32_t sh7709s_device::mcr_rcd()
{
	return ((m_mcr >> 12) & 0x3) + 1;
}

uint32_t sh7709s_device::mcr_trwl()
{
	return ((m_mcr >> 10) & 0x3) + 1;
}

uint32_t sh7709s_device::mcr_tras()
{
	return ((m_mcr >> 8) & 0x3) + 2;
}

uint32_t sh7709s_device::cache_line_fetch_count(uint32_t address)
{
	uint32_t area = get_area(address);

	// Hardcoded for area 0 for cv1k code rom, 16 bit bus size
	if (area == 0)
		return 8;

	// Not encoded in bcr2
	if (area == 1)
		return 0;

	uint32_t bcr2_val = (m_bcr2 >> (area * 2)) & 0x3;

	return SH7709S_CACHE_LINE_SIZE >> bcr2_val;
}

#define CACHE_MISS_STALL (1) // Miss detection in 1 cpu cycle, the rest of the ops (wb buffer movement, etc..) happen in the background

static uint64_t remaining_cycles(uint64_t elapsed, uint64_t cycles)
{
	return (elapsed < cycles) ? (cycles - elapsed) : 0;
}

// cpu->bus cycle conversion hardcoded to 2x as cv1k sh3 runs the bus at 50mhz
unsigned int sh7709s_device::access_penalty(uint32_t address, bool write)
{
	bool is_in_cache = cache_access(address, write);

	if (is_in_cache)
		return 0;

	uint32_t area = get_area(address);
	uint64_t elapsed_cycles = total_cycles() - m_last_op_cycle_count;
	uint32_t cpu_penalty = is_cacheable(address) ? CACHE_MISS_STALL : 0;
	uint32_t bank_read = sdram_bank(address);
	uint32_t bus_penalty = 0;

	// SDRAM timing based on SH7709S documentation
	// These are copied from the timing charts. These are all in bus cycles
	// CAS for SDRAM is stored in WCR2
	//
	// Burst read  (cache fill, 4 longwords): Tr(ACTV) + Trw(RCD-1) + Tc1-Tc4 + CAS + Td2-Td4
	// 1 + RCD + CAS + 3
	//
	// CPU does critical word first so it should only actually stall until:
	// 1 + RCD + CAS
	//
	// Single read (uncached): Tr + Tc1(READA) + Td1
	// 1 + RCD + CAS
	//
	// Single write(uncached): Tr + Tc1(WRITA) + Trwl
	// 1 + RCD + TRWL
	//
	// Burst write (cache eviction writeback, 4 longwords): Tr + Tc1(WRITA) + Td2-Td4 + Trwl
	// 1 + RCD + TRWL + 3
	//
	// Non-SDRAM areas : T1(address) + T2(data) + WCR2 wait states
	// 2 + WCR2

	// Critical word first handling, only stall until the critical word
	// Right now we just stall until the first word lands which isn't quite right but is probably fine

	if (is_sdram_region(address))
		bus_penalty = 1 + mcr_rcd() + get_wcr2_timing(address);
	else
	{
		if (is_cacheable(address))
			bus_penalty = (2 + get_wcr2_timing(address)) * cache_line_fetch_count(address);
		else
			bus_penalty = 2 + get_wcr2_timing(address);
	}

	if (area != m_last_area_accessed)
		bus_penalty += get_wcr1_timing(address);

	m_last_area_accessed = area;

	// CPU is in auto precharge mode, check how many cycles to stall if not enough time has elapsed on a bank conflict
	if (is_sdram_region(address) && bank_read == m_last_sdram_bank && m_precharge_remaining_cycles > 0)
		cpu_penalty += remaining_cycles(elapsed_cycles, m_precharge_remaining_cycles);

	m_precharge_remaining_cycles = 0;

	// We hit another bus operation before a writeback eviction finished, calculate remaining stall time
	if (m_wb_active_cycles > 0)
	{
		uint64_t wb_cycles_left = remaining_cycles(elapsed_cycles, m_wb_active_cycles);
		cpu_penalty += wb_cycles_left;
		// If there was a bank conflict also calculate remaining cycles for the precharge
		if (m_last_sdram_bank == bank_read)
		{
			uint64_t tpc_cycles_left = (wb_cycles_left == 0) ? elapsed_cycles - m_wb_active_cycles : 0;
			cpu_penalty += remaining_cycles(tpc_cycles_left, mcr_tpc() * 2);
		}
		m_wb_active_cycles = 0;
	}

	if (is_sdram_region(address))
		m_last_sdram_bank = bank_read;

	// We had a dirty writeback eviction, total up the background cost penalty we'll pay on subsequent cycles
	if (m_wb_address != 0)
	{
		if (is_sdram_region(m_wb_address))
		{
			uint32_t bank_write = sdram_bank(m_wb_address);
			if (bank_read == bank_write)
			{
				if (is_cacheable(address))
					m_wb_active_cycles += (3 + mcr_tpc()) * 2;
				else
					m_wb_active_cycles += mcr_tpc() * 2;
			}
			m_wb_active_cycles += (1 + mcr_rcd() + 3 + mcr_trwl() + mcr_tpc() + get_wcr1_timing(m_wb_address)) * 2;
			m_last_sdram_bank = bank_write;
		}
		else
		{
			m_wb_active_cycles += get_wcr1_timing(m_wb_address) * 2;
			m_wb_active_cycles += ((2 + get_wcr2_timing(m_wb_address)) * cache_line_fetch_count(m_wb_address)) * 2;
		}

		m_wb_address = 0;
	}
	else if (is_sdram_region(address)) // Account for the read close after burst read if we hit the same bank, writebacks include the cost in the background cycles if there is a conflict
	{
		if (is_cacheable(address))
			m_precharge_remaining_cycles = (3 + mcr_tpc()) * 2;
		else
			m_precharge_remaining_cycles = mcr_tpc() * 2;
	}

	return cpu_penalty + (bus_penalty * 2);
}

void sh7709s_device::update_access_cycles(uint32_t address, bool write)
{
	// For now only handle cacheable instruction fetch sampling when we do
	// data accesses. The majority of uncached instruction fetches is
	// handled in the cache flush timing
	if (is_cacheable(m_sh2_state->pc)) {
		m_sh2_state->icount -= access_penalty(m_sh2_state->pc, false);
		m_last_op_cycle_count = total_cycles();
	}
	m_sh2_state->icount -= access_penalty(address, write);
	m_last_op_cycle_count = total_cycles();
}

void sh7709s_device::drc_memory_access_read()
{
	uint32_t address = m_sh2_state->arg0;
	update_access_cycles(address, false);
}

void sh7709s_device::drc_memory_access_write()
{
	uint32_t address = m_sh2_state->arg0;
	update_access_cycles(address, true);
}

static void cfunc_drc_memory_access_read(void *param)
{
	((sh7709s_device*)param)->drc_memory_access_read();
}

static void cfunc_drc_memory_access_write(void* param)
{
	((sh7709s_device*)param)->drc_memory_access_write();
}

// Same as static_generate_memory_accessor from sh4.cpp but with added read/write penalty tracking
// Address of access stored in arg0, size in arg1 (unused for now)
void sh7709s_device::static_generate_memory_accessor(int size, int iswrite, const char* name, uml::code_handle*& handleptr)
{
	/* on entry, address is in I0; data for writes is in I1 */
	/* on exit, read result is in I0 */
	/* routine trashes I0 */
	int label = 1;

	/* begin generating */
	drcuml_block& block(m_drcuml->begin_block(1024));

	/* add a global entry for this */
	alloc_handle(handleptr, name);
	UML_HANDLE(block, *handleptr);                         // handle  *handleptr

	UML_CMP(block, I0, 0xe0000000);
	UML_JMPc(block, COND_AE, label);

	UML_MOV(block, mem(&m_sh2_state->arg0), I0);       // mov     [arg0],i0
	UML_MOV(block, mem(&m_sh2_state->arg1), size);     // mov     [arg1],size

	UML_AND(block, I0, I0, SH34_AM);     // and r0, r0, #AM (0x1fffffff)

	UML_LABEL(block, label++);              // label:

	if (!debugger_enabled())
	{
		for (auto& elem : m_fastram)
		{
			if (elem.base != nullptr && (!iswrite || !elem.readonly))
			{
				void* fastbase = (uint8_t*)elem.base - elem.start;
				uint32_t skip = label++;
				if (elem.end != 0xffffffff)
				{
					UML_CMP(block, I0, elem.end);   // cmp     i0,end
					UML_JMPc(block, COND_A, skip);                                      // ja      skip
				}
				if (elem.start != 0x00000000)
				{
					UML_CMP(block, I0, elem.start);// cmp     i0,fastram_start
					UML_JMPc(block, COND_B, skip);                                      // jb      skip
				}

				if (!iswrite)
				{
					if (size == 1)
					{
						UML_XOR(block, I0, I0, m_bigendian ? BYTE8_XOR_BE(0) : BYTE8_XOR_LE(0));
						UML_LOAD(block, I0, fastbase, I0, SIZE_BYTE, SCALE_x1);             // load    i0,fastbase,i0,byte
					}
					else if (size == 2)
					{
						UML_XOR(block, I0, I0, m_bigendian ? WORD2_XOR_BE(0) : WORD2_XOR_LE(0));
						UML_LOAD(block, I0, fastbase, I0, SIZE_WORD, SCALE_x1);         // load    i0,fastbase,i0,word_x1
					}
					else if (size == 4)
					{

						UML_XOR(block, I0, I0, m_bigendian ? DWORD_XOR_BE(0) : DWORD_XOR_LE(0));
						UML_LOAD(block, I0, fastbase, I0, SIZE_DWORD, SCALE_x1);            // load    i0,fastbase,i0,dword_x1
					}

					UML_CALLC(block, cfunc_drc_memory_access_read, this); // callc  cfunc_drc_memory_access_read,this

					UML_RET(block);                                                     // ret
				}
				else
				{
					if (size == 1)
					{
						UML_XOR(block, I0, I0, m_bigendian ? BYTE8_XOR_BE(0) : BYTE8_XOR_LE(0));
						UML_STORE(block, fastbase, I0, I1, SIZE_BYTE, SCALE_x1);// store   fastbase,i0,i1,byte
					}
					else if (size == 2)
					{
						UML_XOR(block, I0, I0, m_bigendian ? WORD2_XOR_BE(0) : WORD2_XOR_LE(0));
						UML_STORE(block, fastbase, I0, I1, SIZE_WORD, SCALE_x1);// store   fastbase,i0,i1,word_x1
					}
					else if (size == 4)
					{
						UML_XOR(block, I0, I0, m_bigendian ? DWORD_XOR_BE(0) : DWORD_XOR_LE(0));
						UML_STORE(block, fastbase, I0, I1, SIZE_DWORD, SCALE_x1);       // store   fastbase,i0,i1,dword_x1
					}

					UML_CALLC(block, cfunc_drc_memory_access_write, this); // callc  cfunc_drc_memory_access_write,this

					UML_RET(block);                                                     // ret
				}

				UML_LABEL(block, skip);                                             // skip:
			}
		}
	}

	if (iswrite)
	{
		switch (size)
		{
		case 1:
			UML_WRITE(block, I0, I1, SIZE_BYTE, SPACE_PROGRAM); // write r0, r1, program_byte
			break;

		case 2:
			UML_WRITE(block, I0, I1, SIZE_WORD, SPACE_PROGRAM); // write r0, r1, program_word
			break;

		case 4:
			UML_WRITE(block, I0, I1, SIZE_DWORD, SPACE_PROGRAM);    // write r0, r1, program_dword
			break;
		}

		UML_CALLC(block, cfunc_drc_memory_access_write, this); // callc  cfunc_drc_memory_access_write,this
	}
	else
	{
		switch (size)
		{
		case 1:
			UML_READ(block, I0, I0, SIZE_BYTE, SPACE_PROGRAM);  // read r0, program_byte
			break;

		case 2:
			UML_READ(block, I0, I0, SIZE_WORD, SPACE_PROGRAM);  // read r0, program_word
			break;

		case 4:
			UML_READ(block, I0, I0, SIZE_DWORD, SPACE_PROGRAM); // read r0, program_dword
			break;
		}

		UML_CALLC(block, cfunc_drc_memory_access_read, this); // callc  cfunc_drc_memory_access_read,this
	}

	UML_RET(block);                         // ret

	block.end();
}

uint32_t sh7709s_device::ccr_r(offs_t offset, uint32_t mem_mask)
{
	logerror("'%s' (%08x): CCN unmapped internal read mask %08x (CCR) %08x\n", tag(), m_sh2_state->pc, mem_mask, m_ccr);
	return m_ccr;
}

void sh7709s_device::ccr_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	// Don't write the CF bit into ccr as that's only used for cache invalidate
	mem_mask &= ~0b1000;
	if (data & 0b1000)
	{
		// We're going to invalidate the whole cache but we don't really handle throwing out any dirty data
		// this just assumes that the game is correct in flushing all the data it needs or doesn't rely on
		// throwing out some data
		// After cache invalidation the cpu requires 4 NOP's for the cache state to settle, these are software
		// enforced and not handled by the hardware
		memset(m_cache, 0, sizeof(m_cache));
		LOG("SH7709S cache invalidate\n");
		// We want to account for some of the uncached instruction
		// fetches that happen here but instrumenting every instruction
		// is too cpu intensive. Since this causes a decent amount of
		// fetches in the cache flush loop just total an approximation
		// up here

		// Main loop body (2 instruction fetches per loop due to 32 bit reads):
		// MOV.L R0, @R1
		// DT R2
		// BFS $AC002A66
		// ADD #$10, R1
		m_sh2_state->icount -= (2 /*fetches*/ * 2 /*bus cycle convert*/ * 1024 /*loop iterations*/) *
			(1 + mcr_rcd() + get_wcr2_timing(m_sh2_state->pc) + mcr_tpc());
	}
	COMBINE_DATA(&m_ccr);
	logerror("'%s' (%08x): CCN unmapped internal write %08x & %08x (CCR)\n", tag(), m_sh2_state->pc, data, mem_mask);
}

constexpr uint32_t CACHE_MAPPING_BASE = 0xF0000000;
constexpr uint32_t CACHE_MAPPING_END = 0xF0FFFFFF;

uint32_t sh7709s_device::cache_address_array_r(offs_t offset, uint32_t mem_mask)
{
	// TODO : LRU bits
	// This is unused by cv1k but just added for some extra info
	uint32_t cache_entry_index = offset / 4;
	unsigned int entry_block = cache_entry_index % SH7709S_CACHE_BLOCKS;
	unsigned int way = cache_entry_index / SH7709S_CACHE_BLOCKS;
	struct sh7709s_cache_entry* entry = &m_cache[entry_block][way];
	int v = entry->tag != 0;
	int u = entry->dirty;
	uint32_t ret = (entry->tag << 10) | (u << 1) | v;
	logerror("Cache address array unmapped access read: index %u data %u\n", cache_entry_index, ret);
	return ret;
}

void sh7709s_device::cache_address_array_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	// Only handle U bit writes to flush the entry for now
	uint32_t cache_entry_index = offset / 4;
	// cv1k doesn't even bother to read the entry values, it just writes 0 to every entry to flush them
	bool is_flush = (data & 0x2) == 0;
	if (is_flush)
	{
		unsigned int entry_block = cache_entry_index % SH7709S_CACHE_BLOCKS;
		unsigned int way = cache_entry_index / SH7709S_CACHE_BLOCKS;
		if (m_cache[entry_block][way].dirty)
		{
			uint32_t wb_address = m_cache[entry_block][way].tag * SH7709S_CACHE_LINE_SIZE;
			m_cache[entry_block][way].dirty = 0;
			// Always add wcr1 timing here, accesses to the cache address mappings have to be done via instruction from an uncached region
			m_sh2_state->icount -= (1 + mcr_rcd() + 3 + mcr_trwl() + mcr_tpc() + get_wcr1_timing(wb_address)) * 2;
			// Followup access is likely to area swap as well so those will also need to pay the wait state cost in wcr1 on the instruction fetch
			m_last_area_accessed = get_area(wb_address);
			LOG("Flushing dirty cache entry idx: %u\n", cache_entry_index);
		}
	}
	logerror("Cache address array unmapped access write: %u data: %u\n", cache_entry_index, data);
}

// SH7709S memory mapped cache area
void sh7709s_device::cache_7709s_map(address_map& map)
{
	// TODO : LRU bits should also be tracked/mapped for titles that change the LRU way replacement bits
	// Not sure if other titles attempt to access the data array mapped section
	map(CACHE_MAPPING_BASE, CACHE_MAPPING_END).rw(FUNC(sh7709s_device::cache_address_array_r), FUNC(sh7709s_device::cache_address_array_w));
}

void sh7709s_device::sh3_register_map(address_map& map)
{
	ccn_7709s_map(map);
	ubc_7709s_map(map);
	cpg_7709_map(map);
	bsc_7709s_map(map);
	rtc_map(map);
	intc_7709_map(map);
	dmac_7709_map(map);
	tmu_map(map);
	sci_7709_map(map);
	cmt_7709_map(map);
	ad_7709_map(map);
	da_7709_map(map);
	port_7709_map(map);
	irda_7709_map(map);
	scif_7709_map(map);
	udi_7709s_map(map);
	cache_7709s_map(map);
}
