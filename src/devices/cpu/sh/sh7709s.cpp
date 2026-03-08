// license:BSD-3-Clause
// copyright-holders:Jesus Ramos

// SH 7709S experimental cache/memory timing
// Some values/timing tables are hardcoded
// (such as no bank mode timing/sdram access logic, and some area mappings)
// for cv1k emulation specifically

// Unimplemented functionality:
// - Cache invalidation and write locking of cache lines
// - Uncached access penalty
// - Prefix instruction fetch timing + cache handling
// - Checks for certain area specifics : bank mode timing for sdram access, area check for sdram, area check for burst mode + burst size
// - TLB simulation + timing and lookups
// - Branch predicition/mispredict penalties - Needs a branch target buffer and some basic tracking, didn't make a huge difference for cv1k when I threw one together
// - Full pipeline simulation:
//   Some penalties should overlap with following instructions that don't involve bus access
//   Writeback timing for certain operations should also overlap in some cases
//   If the CPU supports critical word first the instruction that takes the miss should be able to start as soon as that address is read in from memory even if there are further words to be read

#include "sh7709s.h"
#include "sh3comn.h"
#include "sh4comn.h"
#include "cpu/drcumlsh.h"

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
}

void sh7709s_device::device_start()
{
	sh3_base_device::device_start();

	memset(m_cache, 0, sizeof(m_cache));
	m_wb_address = 0;
	m_last_area_accessed = 0;
	m_last_area_accessed_was_write = false;

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
}

static bool is_cacheable(uint32_t address)
{
	// 0x00000000 - 0x80000000 2GB cacheable virtual space
	// 0x80000000 - 0xA0000000 0.5GB fixed physical cacheable space
	// 0xC0000000 - 0xE0000000 0.5GB virtual cacheable space
	uint8_t region = address >> 29;
	return region != 0x5 && region != 0x7;
}

// Returns true on cache hit, false if there was a miss cache miss and updates the cache state to reflect the access
bool sh7709s_device::cache_access(uint32_t address, bool write)
{
	if (!is_cacheable(address))
		return false;

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
				entry->lru = SH7709S_CACHE_ASSOCIATIVITY - 1;
				for (int j = 0; j < SH7709S_CACHE_ASSOCIATIVITY; j++)
				{
					struct sh7709s_cache_entry* update_entry = &m_cache[cache_block][j];
					if (i != j && update_entry->lru != 0)
						update_entry->lru--;
				}
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
			{
				m_wb_address = entry->tag * SH7709S_CACHE_LINE_SIZE;
				m_sh2_state->icount--; // Takes 1 cycle to move the cache line to the wb buffer
			}
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

#define SH7709S_AREA_MASK (0x1FFFFFFF)

unsigned int get_area(uint32_t address)
{
	// Mask to 29 bit physical space
	uint32_t phys_mask = address & SH7709S_AREA_MASK;

	return phys_mask >> 26;
}

static bool is_sdram_region(uint32_t address)
{
	unsigned int area = get_area(address);

	// Hardcoded for now, assumes area 2 and 3 are both mapped to SDRAM
	return area == 2 || area == 3;
}

bool can_use_burst(uint32_t address)
{
	// Assumes only sdram can burst read/write
	return is_sdram_region(address);
}

#define CACHE_LINE_BURST_READ_PENALTY (6) // 2 base + 4 to burst read 16 bytes for a cache line
#define CACHE_LINE_BURST_WRITE_PENALTY (4) // Only the 4 data cycles for the cache line
#define CACHE_MISS_STALL_PENALTY (4) // 3 base (tag lookup, compare, etc...) + 1 for BSC sync
#define BUS_ACCESS_PENALTY (2)

static unsigned int get_wcr1_timing(uint32_t address, uint16_t wcr1)
{
	unsigned int area = get_area(address);

	if (area > 6 || area == 1)
		return 0;

	unsigned int area_val = (wcr1 >> (area * 2)) & 0x3;

	if (area_val == 0)
		return 1;

	return area_val;
}

static unsigned int get_wcr2_timing(uint32_t address, uint16_t wcr2)
{
	unsigned int area = get_area(address);
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

unsigned int mcr_tpc(uint16_t mcr)
{
	unsigned int tpc_val((mcr >> 14) & 0x3);

	// self refresh mode timings, should check RMODE in
	// MCR for non cv1k and use the other timings
	switch (tpc_val)
	{
	case 0: return 2;
	case 1: return 5;
	case 2: return 8;
	case 3: return 11;
	}

	return 2; // Unreachable
}

unsigned int mcr_rcd(uint16_t mcr)
{
	return ((mcr >> 12) & 0x3) + 1;
}

unsigned int mcr_trwl(uint16_t mcr)
{
	return ((mcr >> 10) & 0x3) + 1;
}

unsigned int mcr_tras(uint16_t mcr)
{
	return ((mcr >> 8) & 0x3) + 2;
}

unsigned int cache_line_fetch_count(uint32_t address, uint16_t bcr2)
{
	unsigned int area = get_area(address);

	// Hardcoded for area 0 for cv1k code rom, 16 bit bus size
	if (area == 0)
		return 8;

	// Not encoded in bcr2
	if (area == 1)
		return 0;

	unsigned int bcr2_val = (bcr2 >> (area * 2)) & 0x3;

	return SH7709S_CACHE_LINE_SIZE >> bcr2_val;
}

// Some of the values used here are bus cycles that might need conversion but it's close enough for now
// It's possible for some instructions to overlap in the pipeline with the fetches but that's a good bit
// of extra tracking required
unsigned int sh7709s_device::access_penalty(uint32_t address, bool write)
{
	// cv1k doesn't use TLB so we can just use raw physical address
	unsigned int penalty = 0;
	bool is_in_cache = cache_access(address, write);
	bool is_wb = false;
	bool burst_capable = false;
	unsigned int area = 0;

	if (is_in_cache)
		goto check_wb;

wb_handle:
	area = get_area(address);
	burst_capable = can_use_burst(address);
	// If we're swapping access area or read<->write pay the cost in WCR1 for the switch
	if (area != m_last_area_accessed || (m_last_area_accessed_was_write && !is_wb) || (!m_last_area_accessed_was_write && is_wb))
		penalty += get_wcr1_timing(address, m_wcr1);

	m_last_area_accessed = area;
	m_last_area_accessed_was_write = is_wb;

	// We don't really handle uncached access here but it doesn't seem to affect cv1k slowodwn
	if (burst_capable && !is_wb)
		penalty += CACHE_LINE_BURST_READ_PENALTY;
	if (burst_capable && is_wb)
		penalty += CACHE_LINE_BURST_WRITE_PENALTY - 1; // First 4 bytes of the transfer overlaps the column (write) command issue so we can remove 1 cycle

	// If we can't use burst timing it's gonna be pricey to access
	// Each access will have to initiate bus access and wait to read 4 words for the cache line fill
	if (!burst_capable)
		penalty += (BUS_ACCESS_PENALTY + get_wcr2_timing(address, m_wcr2)) * cache_line_fetch_count(address, m_bcr2);
	else if (!is_wb) // For cache line writeback we can ignore the stall as well as CAS latency
		penalty += CACHE_MISS_STALL_PENALTY + get_wcr2_timing(address, m_wcr2);

	// On cv1k boards the SDRAM is in self refresh mode so every access is a page miss due to the page close after each fetch
	if (is_sdram_region(address))
	{
		penalty += BUS_ACCESS_PENALTY +
			1 /* ACTV command issue */ +
			mcr_rcd(m_mcr) +
			1 /* Column command issue (read/write), for writes this overlaps the first word write */ +
			mcr_tpc(m_mcr) + mcr_tras(m_mcr);
		// If trcd cycles is 2 or more for sdram a NOP command to SDRAM is inserted between tr and tc1 which is trcd - 1 cycles long
		if (mcr_rcd(m_mcr) >= 2)
			penalty += mcr_rcd(m_mcr) - 1;
	}

check_wb:
	// The read fetch is always handled first before any wb buffer eviction
	// so we always handle those first
	// This would normally happen in the background but it's likely that
	// you'll hit a subsequent cache miss and take the hit anyway
	if (m_wb_address != 0)
	{
		address = m_wb_address;
		is_wb = true;
		m_wb_address = 0;
		penalty += mcr_trwl(m_mcr);
		goto wb_handle;
	}

	return penalty;
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
	// Assume the instruction prefetch is perfect for now
	// we'll still pay a bit of penalty for the access and
	// handle cache writeback when the icache fetch causes
	// a dirty line eviction
	uint32_t pc_addr = m_sh2_state->pc & SH34_AM;
	m_sh2_state->icount -= access_penalty(pc_addr, false);
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

	UML_AND(block, I0, I0, SH34_AM);     // and r0, r0, #AM (0x1fffffff)

	UML_MOV(block, mem(&m_sh2_state->arg0), I0);       // mov     [arg0],i0
	UML_MOV(block, mem(&m_sh2_state->arg1), size);     // mov     [arg1],size

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
}
