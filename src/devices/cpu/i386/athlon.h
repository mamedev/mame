// license:BSD-3-Clause
// copyright-holders:Ville Linde, Barry Rodewald, Carl, Philip Bennett

#ifndef MAME_CPU_I386_ATHLON_H
#define MAME_CPU_I386_ATHLON_H

#pragma once

#include "i386.h"
#include "cache.h"

class athlonxp_device : public pentium_device
{
public:
	// construction/destruction
	athlonxp_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	virtual void opcode_cpuid() override;
	virtual uint64_t opcode_rdmsr(bool &valid_msr) override;
	virtual void opcode_wrmsr(uint64_t data, bool &valid_msr) override;
	virtual void cache_writeback() override;
	virtual void cache_invalidate() override;
	virtual void cache_clean() override;
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;
	virtual void enter_smm() override;
	virtual void leave_smm() override;

	virtual u8 mem_pr8(offs_t address) override { return opcode_read_cache<u8, NATIVE_ENDIAN_VALUE_LE_BE(0, 3)>(address);   }
	virtual u16 mem_pr16(offs_t address) override { return opcode_read_cache<u16, NATIVE_ENDIAN_VALUE_LE_BE(0, 2)>(address); }
	virtual u32 mem_pr32(offs_t address) override { return opcode_read_cache<u32, 0>(address); }

	virtual uint8_t READ8PL(uint32_t ea, uint8_t privilege) override;
	virtual uint16_t READ16PL(uint32_t ea, uint8_t privilege) override;
	virtual uint32_t READ32PL(uint32_t ea, uint8_t privilege) override;
	virtual uint64_t READ64PL(uint32_t ea, uint8_t privilege) override;
	virtual void WRITE8PL(uint32_t ea, uint8_t privilege, uint8_t value) override;
	virtual void WRITE16PL(uint32_t ea, uint8_t privilege, uint16_t value) override;
	virtual void WRITE32PL(uint32_t ea, uint8_t privilege, uint32_t value) override;
	virtual void WRITE64PL(uint32_t ea, uint8_t privilege, uint64_t value) override;

	// device_memory_interface override
	virtual space_config_vector memory_space_config() const override;

private:
	void parse_mtrrfix(u64 mtrr, offs_t base, int kblock);
	inline int check_cacheable(offs_t address);
	template <int wr> int address_mode(offs_t address);

	template <class dt, offs_t xorle> dt opcode_read_cache(offs_t address);
	uint32_t program_read_cache(offs_t address, uint32_t mask);
	void program_write_cache(offs_t address, uint32_t data, uint32_t mask);

	u32 debug_read_memory(offs_t offset, u32 mask);
	void debug_write_memory(offs_t offset, uint32_t value, uint32_t mask);

	address_space_config m_data_config;
	address_space_config m_opcodes_config;
	memory_access<32, 2, 0, ENDIANNESS_LITTLE>::cache mmacache32;
	memory_access<32, 2, 0, ENDIANNESS_LITTLE>::specific m_opcodes;
	memory_access<32, 2, 0, ENDIANNESS_LITTLE>::specific m_data;
	uint8_t m_processor_name_string[48];
	offs_t m_msr_top_mem;
	uint64_t m_msr_sys_cfg;
	offs_t m_msr_smm_base;
	uint64_t m_msr_smm_mask;
	uint64_t m_msr_mtrrfix[11];
	uint8_t m_memory_ranges_1m[1024 / 4];
	cpucache<17, 9, Cache2Way, CacheLineBytes64> cache; // 512 sets, 2 ways (cachelines per set), 64 bytes per cacheline
};


DECLARE_DEVICE_TYPE(ATHLONXP,    athlonxp_device)

#endif // MAME_CPU_I386_ATHLON_H
