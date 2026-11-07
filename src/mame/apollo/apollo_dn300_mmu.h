// license:BSD-3-Clause
// copyright-holders:Chris Toshok
/*
 * apollo_dn300_mmu.h - the Apollo DN300/DN320 memory management unit
 */

#ifndef MAME_APOLLO_APOLLO_DN300_MMU_H
#define MAME_APOLLO_APOLLO_DN300_MMU_H

#pragma once

#include "cpu/m68000/m68010.h"

// The DN300 MMU sits between the 68010 and the bus: the CPU's program space
// is the virtual view (virtual_map, every cycle goes through virtual_r /
// virtual_w), and this device owns the physical bus (space 0, "physical":
// RAM, PROM, I/O, display memory and the MMU's own registers/PFT/PTT, laid
// out by the driver with set_addrmap(0, ...)).
class apollo_dn300_mmu_device :
	public device_t,
	public device_memory_interface
{
public:
	//construction/destruction
	apollo_dn300_mmu_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// configuration
	template <typename T> void set_cpu(T &&cputag)
	{
		m_cpu.set_tag(std::forward<T>(cputag));
	}

	bool is_enabled() { return m_enabled; }

	// the CPU's (virtual) program space
	void virtual_map(address_map &map) ATTR_COLD;
	uint16_t virtual_r(offs_t offset, uint16_t mem_mask = ~0);
	void virtual_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);

	void pft_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	uint16_t pft_r(offs_t offset, uint16_t mem_mask = ~0);

	void ptt_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	uint16_t ptt_r(offs_t offset, uint16_t mem_mask = ~0);

	void unk_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	uint16_t unk_r(offs_t offset, uint16_t mem_mask = ~0);

	void pid_priv_power_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	uint16_t pid_priv_power_r(offs_t offset, uint16_t mem_mask = ~0);

	void status_w(offs_t offset, uint8_t data, uint8_t mem_mask = ~0);
	uint8_t status_r(offs_t offset, uint8_t mem_mask = ~0);

	void fpu_owner_w(offs_t offset, uint8_t data, uint8_t mem_mask = ~0);
	uint8_t fpu_owner_r(offs_t offset, uint8_t mem_mask = ~0);

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	// device_memory_interface implementation
	virtual space_config_vector memory_space_config() const override;

private:
	offs_t translate(offs_t offset, int intention);
	bool write_suppressed();

	address_space_config m_physical_config;
	memory_access<24, 1, 0, ENDIANNESS_BIG>::specific m_physical;

	// a rerun bus error raised for the current instruction (see write_suppressed)
	bool m_fault_pending;
	uint64_t m_fault_cycles;

	required_device<m68000_musashi_device> m_cpu;

	std::unique_ptr<uint16_t[]> m_pft;
	std::unique_ptr<uint16_t[]> m_ptt;
	uint16_t m_pid_priv_power;
	uint8_t m_status;
	uint8_t m_fpu_owner;

	uint8_t m_enabled;
	uint8_t m_asid;
	uint8_t m_domain;
};

// device type definition
DECLARE_DEVICE_TYPE(APOLLO_DN300_MMU, apollo_dn300_mmu_device)

#endif // MAME_APOLLO_APOLLO_DN300_MMU_H
