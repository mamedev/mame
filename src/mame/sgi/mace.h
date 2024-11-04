// license:BSD-3-Clause
// copyright-holders:Ryan Holtz
/**********************************************************************

    SGI MACE skeleton device

**********************************************************************/

#ifndef MAME_SGI_MACE_H
#define MAME_SGI_MACE_H

#pragma once

#include "cpu/mips/mips3.h"

class mace_device : public device_t
{
public:
	template <typename T>
	mace_device(const machine_config &mconfig, const char *tag, device_t *owner, T &&cpu_tag)
		: mace_device(mconfig, tag, owner, (uint32_t)0)
	{
		m_maincpu.set_tag(std::forward<T>(cpu_tag));
	}

	mace_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	auto rtc_read_callback() { return m_rtc_read_callback.bind(); }
	auto rtc_write_callback() { return m_rtc_write_callback.bind(); }

	void map(address_map &map) ATTR_COLD;

protected:
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;

	TIMER_CALLBACK_MEMBER(ust_tick);
	TIMER_CALLBACK_MEMBER(msc_tick);

	// UST/MSC Timer
	void check_ust_msc_compare();

	// Read/Write Handlers
	uint64_t pci_r(offs_t offset, uint64_t mem_mask = ~0);
	void pci_w(offs_t offset, uint64_t data, uint64_t mem_mask = ~0);
	uint64_t vin1_r(offs_t offset, uint64_t mem_mask = ~0);
	void vin1_w(offs_t offset, uint64_t data, uint64_t mem_mask = ~0);
	uint64_t vin2_r(offs_t offset, uint64_t mem_mask = ~0);
	void vin2_w(offs_t offset, uint64_t data, uint64_t mem_mask = ~0);
	uint64_t vout_r(offs_t offset, uint64_t mem_mask = ~0);
	void vout_w(offs_t offset, uint64_t data, uint64_t mem_mask = ~0);
	uint64_t enet_r(offs_t offset, uint64_t mem_mask = ~0);
	void enet_w(offs_t offset, uint64_t data, uint64_t mem_mask = ~0);
	uint64_t audio_r(offs_t offset, uint64_t mem_mask = ~0);
	void audio_w(offs_t offset, uint64_t data, uint64_t mem_mask = ~0);
	uint64_t isa_r(offs_t offset, uint64_t mem_mask = ~0);
	void isa_w(offs_t offset, uint64_t data, uint64_t mem_mask = ~0);
	uint64_t kbdms_r(offs_t offset, uint64_t mem_mask = ~0);
	void kbdms_w(offs_t offset, uint64_t data, uint64_t mem_mask = ~0);
	uint64_t i2c_r(offs_t offset, uint64_t mem_mask = ~0);
	void i2c_w(offs_t offset, uint64_t data, uint64_t mem_mask = ~0);
	uint64_t ust_msc_r(offs_t offset, uint64_t mem_mask = ~0);
	void ust_msc_w(offs_t offset, uint64_t data, uint64_t mem_mask = ~0);
	uint64_t isa_ext_r(offs_t offset, uint64_t mem_mask = ~0);
	void isa_ext_w(offs_t offset, uint64_t data, uint64_t mem_mask = ~0);
	uint64_t rtc_r(offs_t offset, uint64_t mem_mask = ~0);
	void rtc_w(offs_t offset, uint64_t data, uint64_t mem_mask = ~0);

	required_device<mips3_device> m_maincpu;

	devcb_read8 m_rtc_read_callback;
	devcb_write8 m_rtc_write_callback;

	enum
	{
		ISA_INT_COMPARE1    = 0x2000,
		ISA_INT_COMPARE2    = 0x4000,
		ISA_INT_COMPARE3    = 0x8000,
	};

	struct isa_t
	{
		uint32_t m_ringbase_reset;
		uint32_t m_flash_nic_ctrl;
		uint32_t m_int_status;
		uint32_t m_int_mask;
	};

	struct ust_msc_t
	{
		uint32_t m_msc;
		uint32_t m_ust;
		uint64_t m_ust_msc;
		uint64_t m_compare1;
		uint64_t m_compare2;
		uint64_t m_compare3;
		uint64_t m_ain_msc_ust;
		uint64_t m_aout1_msc_ust;
		uint64_t m_aout2_msc_ust;
		uint64_t m_vin1_msc_ust;
		uint64_t m_vin2_msc_ust;
		uint64_t m_vout_msc_ust;
	};

	isa_t m_isa;

	ust_msc_t m_ust_msc;
	emu_timer *m_timer_ust;
	emu_timer *m_timer_msc;
};

DECLARE_DEVICE_TYPE(SGI_MACE, mace_device)

#endif // MAME_SGI_MACE_H
