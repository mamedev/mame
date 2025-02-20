// license:BSD-3-Clause
// copyright-holders:R. Belmont

#ifndef MAME_APPLE_MSC_H
#define MAME_APPLE_MSC_H

#pragma once

#include "pseudovia.h"

#include "machine/6522via.h"
#include "sound/asc.h"
#include "speaker.h"

class mscvia_device : public via6522_device
{
public:
	mscvia_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// Something is definitely customized with how PB1 interrupts work in the MSC's internal VIA1.
	// This makes CPU/PMU comms work properly and fits with what we see in the leaked System 7.1
	// source tree, but I'm not sure it's the exact behavior of the real MSC.
	void cb1_int_hack(int state)
	{
		if (state == ASSERT_LINE)
		{
			set_int(0x10);              // INT_CB1
		}
	}
};

class msc_device :  public device_t, public device_sound_interface
{
public:
	msc_device(const machine_config &mconfig, const char *tag, device_t *owner)
		: msc_device(mconfig, tag, owner, (uint32_t)0)
	{
	}

	msc_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	auto pb4_callback() { return write_pb4.bind(); }
	auto pb5_callback() { return write_pb5.bind(); }
	auto cb2_callback() { return write_cb2.bind(); }
	auto vbl_callback() { return write_vbl.bind(); }
	auto pb3_callback() { return read_pb3.bind(); }

	void map(address_map &map) ATTR_COLD;

	template <typename... T> void set_maincpu_tag(T &&... args) { m_maincpu.set_tag(std::forward<T>(args)...); }
	template <typename... T> void set_pmu_tag(T &&... args) { m_pmu.set_tag(std::forward<T>(args)...); }
	template <typename... T> void set_rom_tag(T &&... args) { m_rom.set_tag(std::forward<T>(args)...); }
	void set_ram_info(u32 *ram, u32 size);
	void set_cpu_clock(XTAL clock);

	void cb1_w(int state);
	void cb2_w(int state);
	void scc_irq_w(int state);
	void scsi_irq_w(int state);
	void slot0_irq_w(int state);
	void slot1_irq_w(int state);
	void slot2_irq_w(int state);
	void lcd_irq_w(int state);
	void pmu_reset_w(int state);

	void via_sync();

	int get_pmu_req();
	void pmu_ack_w(int state);
	void cb1_int_hack(int state);

protected:
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;

	virtual void sound_stream_update(sound_stream &stream, std::vector<read_stream_view> const &inputs, std::vector<write_stream_view> &outputs) override;

private:
	devcb_write_line write_pb4, write_pb5, write_cb2, write_vbl;
	devcb_read_line read_pb3;

	required_device<cpu_device> m_maincpu, m_pmu;
	required_device<mscvia_device> m_via1;
	required_device<pseudovia_device> m_pseudovia;
	required_device<asc_device> m_asc;
	required_region_ptr<u32> m_rom;

	sound_stream *m_stream;
	emu_timer *m_6015_timer;
	u32 m_cpu_clock;
	int m_via_interrupt, m_pmu_interrupt, m_via2_interrupt, m_scc_interrupt, m_last_taken_interrupt;
	bool m_overlay;
	u32 *m_ram_ptr, *m_rom_ptr;
	u32 m_ram_size, m_rom_size;
	u8 m_pmu_req, m_pmu_ack;
	u8 m_msc_config, m_msc_clock_ctrl, m_msc_sound_ctrl;

	u32 rom_switch_r(offs_t offset);
	void power_cycle_w(u32 data);
	u8 msc_config_r();
	void msc_config_w(u8 data);
	u8 msc_pseudovia_r(offs_t offset);
	void msc_pseudovia_w(offs_t offset, u8 data);

	u16 via_r(offs_t offset);
	void via_w(offs_t offset, u16 data, u16 mem_mask);
	u8 via2_r(offs_t offset);
	void via2_w(offs_t offset, u8 data);

	u8 via_in_a();
	u8 via_in_b();
	void via_out_a(u8 data);
	void via_out_b(u8 data);
	u8 via2_in_a();
	u8 via2_in_b();
	void via2_out_a(u8 data);
	void via2_out_b(u8 data);
	void field_interrupts();
	void via_out_cb2(int state);
	void via1_irq(int state);
	void via2_irq(int state);
	TIMER_CALLBACK_MEMBER(msc_6015_tick);
};

DECLARE_DEVICE_TYPE(MSC, msc_device)

#endif // MAME_APPLE_MSC_H
