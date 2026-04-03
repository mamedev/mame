// license:BSD-3-Clause
// copyright-holders:Jesus Ramos

#ifndef MAME_CPU_SH_SH7709S_H
#define MAME_CPU_SH_SH7709S_H

#include "sh4.h"

#define SH7709S_CACHE_SIZE (16384)
#define SH7709S_CACHE_LINE_SIZE (16)
#define SH7709S_CACHE_ENTRY_COUNT (SH7709S_CACHE_SIZE / SH7709S_CACHE_LINE_SIZE)
#define SH7709S_CACHE_ASSOCIATIVITY (4)
#define SH7709S_CACHE_BLOCKS (SH7709S_CACHE_ENTRY_COUNT / SH7709S_CACHE_ASSOCIATIVITY)

// U bit tracked in the dirty field, V bit currently untracked
// Assumes all cache lines are valid as cache line invalidation is unimplemented
struct sh7709s_cache_entry
{
	uint32_t tag; // Address tag for entry
	uint8_t lru; // LRU value [0, SH7709S_CACHE_ASSOCIATIVITY)
	uint8_t dirty; // Dirty bit for write eviction
};

class sh7709s_device : public sh3_base_device
{
public:
	sh7709s_device(const machine_config& mconfig, const char* tag, device_t* owner, uint32_t clock, endianness_t endianness = ENDIANNESS_LITTLE);

	// DRC functions used to update the cache state
	void drc_memory_access_read();
	void drc_memory_access_write();
	void drc_update_icache();

protected:
	virtual void sh3_register_map(address_map& map) override ATTR_COLD;
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;
	virtual void static_generate_memory_accessor(int size, int iswrite, const char* name, uml::code_handle*& handleptr) override;
	virtual bool generate_group_0(drcuml_block& block, compiler_state& compiler, const opcode_desc* desc, uint16_t opcode, int in_delay_slot, uint32_t ovrpc) override;
	virtual bool generate_group_4(drcuml_block& block, compiler_state& compiler, const opcode_desc* desc, uint16_t opcode, int in_delay_slot, uint32_t ovrpc) override;
	virtual bool generate_group_15(drcuml_block& block, compiler_state& compiler, const opcode_desc* desc, uint16_t opcode, int in_delay_slot, uint32_t ovrpc) override;

private:
	// Cache state tracking
	struct sh7709s_cache_entry m_cache[SH7709S_CACHE_BLOCKS][SH7709S_CACHE_ASSOCIATIVITY];
	uint32_t m_wb_address; // writeback buffer address if there's a dirty cache line to evict
	uint8_t m_last_area_accessed; // last memory area accessed for WCR1 timing purposes
	bool m_last_area_accessed_was_write; // last memory area accessed operation also for WCR1 timing purposes
	unsigned int m_wb_active_cycles; // Track any background cycles for writeback and precharge waits on the same bank
	unsigned int m_last_sdram_page; // Last accessed sdram page, used to track when to have to pay tpc(precharge) cost
	unsigned int m_precharge_remaining_cycles;

	bool cache_access(uint32_t address, bool write);
	unsigned int access_penalty(uint32_t address, bool write);
};

DECLARE_DEVICE_TYPE(SH7709S, sh7709s_device)

#endif /* MAME_CPU_SH7709S_H */
