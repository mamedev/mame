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
// - TLB simulation + timing and lookups
// - Branch predicition/mispredict penalties - Needs a branch target buffer and some basic tracking, didn't make a huge difference for cv1k when I threw one together

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
	m_last_sdram_page = 0;
	m_precharge_remaining_cycles = 0;
}

void sh7709s_device::device_start()
{
	sh3_base_device::device_start();

	memset(m_cache, 0, sizeof(m_cache));
	m_wb_address = 0;
	m_last_area_accessed = 0;
	m_last_area_accessed_was_write = false;
	m_wb_active_cycles = 0;
	m_last_sdram_page = 0;
	m_precharge_remaining_cycles = 0;

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
	save_item(NAME(m_last_sdram_page));
	save_item(NAME(m_precharge_remaining_cycles));
}

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
	bool burst_capable = can_use_burst(address);

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

	if (burst_capable)
	{
		switch (area_val)
		{
		case 0: return 2;
		case 1: return 2;
		case 2: return 3;
		case 3: return 4;
		case 4: return 4;
		case 5: return 6;
		case 6: return 8;
		case 7: return 10;
		}
	}
	else
	{
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
	}

	return 2; // Unreachable
}

// TODO : Update tpc handling to use the address multiplexer bits in mcr
// instead of the sdram page as the precharge command holds up the whole bank
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

#define SDRAM_PAGE_SIZE (1024) // Hardcoded for cv1k

// CPU cycles
#define CACHE_MISS_STALL (1) // Miss detection in 1 cycle, the rest of the ops (wb buffer movement, etc..) happen in the background
// Bus cycles
// TODO : Verify the bus stall here. These Renesas docs (https://resource.renesas.com/lib/eng/e_learnig/superh_e_learning/29/contents.html)
// refer to a 3 bus cycle penalty for a cache line fill but the other documents don't seem to mention it.
// For now this can cover the bsc clock sync + actv + read/write command cycles
#define BUS_ACCESS_PENALTY (3)
// TODO : CPU does critical word first handling here, so the first access causing the fetch
// can amortize a couple of the remaining cyles of the read. These don't amount to too much for
// now though
#define BURST_READ_WORD_PENALTY (3)
#define BURST_WRITE_WORD_PENALTY (4)

// cpu->bus cycle conversion hardcoded to 2x as cv1k sh3 runs the bus at 50mhz
unsigned int sh7709s_device::access_penalty(uint32_t address, bool write)
{
	uint32_t area = get_area(address);
	bool is_in_cache = cache_access(address, write);

	if (is_in_cache)
		return 0;

	// Penalty calculations for SDRAM access
	unsigned int bus_penalty = BUS_ACCESS_PENALTY + BURST_READ_WORD_PENALTY + mcr_rcd() + get_wcr2_timing(address);
	unsigned int cpu_penalty = is_cacheable(address) ? CACHE_MISS_STALL : 0;
	unsigned int page_read = address / SDRAM_PAGE_SIZE;

	// Penalty calculations for non-sdram access
	if (!is_sdram_region(address))
	{
		// If it's not cacheable we're going to just do the single access
		if (!is_cacheable(address))
			bus_penalty = BUS_ACCESS_PENALTY + get_wcr2_timing(address);
		else // Cacheable access, fetch the whole cache line
			bus_penalty = (BUS_ACCESS_PENALTY + get_wcr2_timing(address)) * cache_line_fetch_count(address);
	}
	else if (!is_cacheable(address)) // non-cacheable sdram access
	{
		if (!write)
			bus_penalty = BUS_ACCESS_PENALTY + mcr_rcd() + get_wcr2_timing(address);
		else
			bus_penalty = BUS_ACCESS_PENALTY + mcr_rcd() + mcr_trwl();
	}

	if (area != m_last_area_accessed)
		bus_penalty += get_wcr1_timing(address);

	m_last_area_accessed = area;

	// CPU is in auto precharge mode, since we hit a bank conflict and the last precharge isn't done we stall
	if (is_sdram_region(address) && page_read == m_last_sdram_page && m_precharge_remaining_cycles > 0)
		cpu_penalty += m_precharge_remaining_cycles;

	m_precharge_remaining_cycles = 0;

	// We hit another bus operation before a writeback eviction finished, stall until that's done
	// We already account for the precharge bank conflict above
	if (m_wb_active_cycles > 0)
	{
		cpu_penalty += m_wb_active_cycles;
		m_wb_active_cycles = 0;
	}

	if (is_sdram_region(address))
		m_last_sdram_page = page_read;

	// We had a dirty writeback eviction, total up the background cost penalty we'll pay on subsequent cycles
	if (m_wb_address != 0)
	{
		if (is_sdram_region(address))
		{
			unsigned int page_write = m_wb_address / SDRAM_PAGE_SIZE;
			// Bank collision in auto precharge mode, we have to wait for precharge to finish before starting
			if (page_read == page_write)
				m_wb_active_cycles += mcr_tpc() * 2;
			m_wb_active_cycles += (BUS_ACCESS_PENALTY + get_wcr1_timing(address) + BURST_WRITE_WORD_PENALTY + mcr_rcd() + mcr_trwl()) * 2;
			m_last_sdram_page = page_write;
		}
		else
		{
			m_wb_active_cycles += get_wcr1_timing(address) * 2;
			m_wb_active_cycles += ((BUS_ACCESS_PENALTY + get_wcr2_timing(address)) * cache_line_fetch_count(address)) * 2;
		}

		m_wb_address = 0;
	}
	else if (is_sdram_region(address)) // Account for the read close after burst read if we hit the same bank, writebacks include the cost in the background cycles if there is a conflict
		m_precharge_remaining_cycles = mcr_tpc() * 2;

	return cpu_penalty + (bus_penalty * 2);
}

void sh7709s_device::drc_memory_access_read()
{
	uint32_t address = m_sh2_state->arg0;
	m_sh2_state->icount -= access_penalty(address, false);
}

void sh7709s_device::drc_memory_access_write()
{
	uint32_t address = m_sh2_state->arg0;
	m_sh2_state->icount -= access_penalty(address, true);
}

void sh7709s_device::drc_update_icache()
{
	if (m_wb_active_cycles > 0)
	{
		m_wb_active_cycles--;
		// If we had an active wb complete set the remaining precharge cycles in case of bank conflict
		if (m_wb_active_cycles == 0)
			m_precharge_remaining_cycles = mcr_tpc() * 2;
	}
	else if (m_precharge_remaining_cycles > 0)
		m_precharge_remaining_cycles--;
	// Assume the instruction prefetch is perfect for now
	// we'll still pay a bit of penalty for the access and
	// handle cache writeback when the icache fetch causes
	// a dirty line eviction
	m_sh2_state->icount -= access_penalty(m_sh2_state->pc, false);
}

static void cfunc_drc_memory_access_read(void *param)
{
	((sh7709s_device*)param)->drc_memory_access_read();
}

static void cfunc_drc_memory_access_write(void* param)
{
	((sh7709s_device*)param)->drc_memory_access_write();
}

static void cfunc_drc_update_icache(void* param)
{
	((sh7709s_device*)param)->drc_update_icache();
}

// Have each instruction update the icache when executed since it's a shared icache+dcache
// TODO : Ideally this uses generate_opcode but that seems to be quite cpu heavy
// Check if there's another way of handling it that isn't as heavy. The CPU accesses
// for instructions aren't a huge cause of slowdown related access other than the loop on cv1k
// that handles cache invalidation that executes 1024 times which can add up.
// Also need to double check how the uncached instruction fetch is handled if 32 bit reads are done
bool sh7709s_device::generate_group_0(drcuml_block& block, compiler_state& compiler, const opcode_desc* desc, uint16_t opcode, int in_delay_slot, uint32_t ovrpc)
{
	UML_CALLC(block, cfunc_drc_update_icache, this);

	return sh3_base_device::generate_group_0(block, compiler, desc, opcode, in_delay_slot, ovrpc);
}

bool sh7709s_device::generate_group_4(drcuml_block& block, compiler_state& compiler, const opcode_desc* desc, uint16_t opcode, int in_delay_slot, uint32_t ovrpc)
{
	UML_CALLC(block, cfunc_drc_update_icache, this);

	return sh3_base_device::generate_group_4(block, compiler, desc, opcode, in_delay_slot, ovrpc);
}

bool sh7709s_device::generate_group_15(drcuml_block& block, compiler_state& compiler, const opcode_desc* desc, uint16_t opcode, int in_delay_slot, uint32_t ovrpc)
{
	UML_CALLC(block, cfunc_drc_update_icache, this);

	return sh3_base_device::generate_group_15(block, compiler, desc, opcode, in_delay_slot, ovrpc);
}

// Same as static_generate_memory_accessor from sh4.cpp but with added read/write penalty tracking
// Address of access stored in arg0, size in arg1 (unused for now)
// TODO : Some accesses go through read/write byte,word,etc... might need to handle those
// via overrides and only handle the fastram accesses here
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
			m_sh2_state->icount -= (BUS_ACCESS_PENALTY + BURST_WRITE_WORD_PENALTY + get_wcr1_timing(wb_address) + mcr_rcd() + mcr_trwl()) * 2;
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
