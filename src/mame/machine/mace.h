// license:BSD-3-Clause
// copyright-holders:Ryan Holtz
/**********************************************************************

    SGI MACE skeleton device

**********************************************************************/

#ifndef MAME_MACHINE_MACE_H
#define MAME_MACHINE_MACE_H

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

	void map(address_map &map);

protected:
	virtual void device_resolve_objects() override;
	virtual void device_start() override;
	virtual void device_reset() override;
	virtual void device_add_mconfig(machine_config &config) override;
	virtual void device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr) override;

	static const device_timer_id TIMER_MSC = 0;
	static const device_timer_id TIMER_UST = 1;

	// UST/MSC Timer
	void check_ust_msc_compare();

	// Read/Write Handlers
	DECLARE_READ64_MEMBER(pci_r);
	DECLARE_WRITE64_MEMBER(pci_w);
	DECLARE_READ64_MEMBER(vin1_r);
	DECLARE_WRITE64_MEMBER(vin1_w);
	DECLARE_READ64_MEMBER(vin2_r);
	DECLARE_WRITE64_MEMBER(vin2_w);
	DECLARE_READ64_MEMBER(vout_r);
	DECLARE_WRITE64_MEMBER(vout_w);
	DECLARE_READ64_MEMBER(enet_r);
	DECLARE_WRITE64_MEMBER(enet_w);
	DECLARE_READ64_MEMBER(audio_r);
	DECLARE_WRITE64_MEMBER(audio_w);
	DECLARE_READ64_MEMBER(isa_r);
	DECLARE_WRITE64_MEMBER(isa_w);
	DECLARE_READ64_MEMBER(kbdms_r);
	DECLARE_WRITE64_MEMBER(kbdms_w);
	DECLARE_READ64_MEMBER(i2c_r);
	DECLARE_WRITE64_MEMBER(i2c_w);
	DECLARE_READ64_MEMBER(ust_msc_r);
	DECLARE_WRITE64_MEMBER(ust_msc_w);
	DECLARE_READ64_MEMBER(isa_ext_r);
	DECLARE_WRITE64_MEMBER(isa_ext_w);
	DECLARE_READ64_MEMBER(rtc_r);
	DECLARE_WRITE64_MEMBER(rtc_w);

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

#endif // MAME_MACHINE_MACE_H
