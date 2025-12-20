// license:BSD-3-Clause
// copyright-holders:Patrick Mackinlay

#ifndef MAME_SGI_PM2_MMU_H
#define MAME_SGI_PM2_MMU_H

#pragma once

#include "cpu/m68000/m68000.h"

class pm2_mmu_device
	: public device_t
	, public m68000_device::mmu
{
public:
	template <unsigned N, typename T> void set_space(T &&tag, int spacenum) { m_space[N].set_tag(std::forward<T>(tag), spacenum); }

	enum error_type : unsigned
	{
		MMU_DEFER = 0, // invalid multibus access
		MMU_ERROR = 1, // bus error (system space, segment protection, nonexistent page)
	};
	auto error() { return m_error.bind(); }

	pm2_mmu_device(machine_config const &mconfig, char const *tag, device_t *owner, u32 clock = 0);

	void boot_w(int state);

	// register access
	u16 context_r();
	void context_w(u16 data);
	u16 page_r(offs_t offset);
	void page_w(offs_t offset, u16 data, u16 mem_mask);
	u16 prot_r(offs_t offset);
	void prot_w(offs_t offset, u16 data, u16 mem_mask);

protected:
	// device_t implementation
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	// m68000_device::mmu implementation
	virtual u16 read_program(offs_t addr, u16 mem_mask) override;
	virtual void write_program(offs_t addr, u16 data, u16 mem_mask) override;
	virtual u16 read_data(offs_t addr, u16 mem_mask) override;
	virtual void write_data(offs_t addr, u16 data, u16 mem_mask) override;
	virtual u16 read_cpu(offs_t addr, u16 mem_mask) override;
	virtual void set_super(bool super) override;

	std::optional<std::pair<unsigned, offs_t>> translate(offs_t const address, unsigned const mode);
	template <bool Execute> u16 mmu_read(offs_t logical, u16 mem_mask);
	void mmu_write(offs_t logical, u16 data, u16 mem_mask);

private:
	required_address_space m_space[4];
	devcb_write_line m_error;

	memory_access<24, 1, 0, ENDIANNESS_BIG>::specific m_cpu_mem;
	memory_access<24, 1, 0, ENDIANNESS_BIG>::specific m_cpu_spc;

	memory_access<24, 1, 0, ENDIANNESS_LITTLE>::specific m_bus_mem;
	memory_access<16, 1, 0, ENDIANNESS_LITTLE>::specific m_bus_pio;

	std::unique_ptr<u16[]> m_page;
	std::unique_ptr<u16[]> m_prot;
	u8 m_context;

	bool m_super;
	bool m_boot;
};

DECLARE_DEVICE_TYPE(SGI_PM2_MMU, pm2_mmu_device)

#endif // MAME_SGI_PM2_MMU_H
