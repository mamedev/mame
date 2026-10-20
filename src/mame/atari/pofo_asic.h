// license:BSD-3-Clause
// copyright-holders:Curt Coder
/*

    Atari Portfolio system ASIC

    Handles the interrupt controller, memory/IO bus decoding, system
    counter, power management and miscellaneous glue logic of the
    Atari Portfolio.

*/

#ifndef MAME_ATARI_POFO_ASIC_H
#define MAME_ATARI_POFO_ASIC_H

#pragma once

#include "bus/pofo/ccm.h"
#include "bus/pofo/exp.h"
#include "machine/ram.h"
#include "machine/timer.h"

class portfolio_asic_device : public device_t
{
public:
	portfolio_asic_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock = 0);

	void set_rom_tag(const char *tag) { m_rom_tag = tag; }

	auto pint_wr_cb() { return m_write_pint.bind(); }
	auto nmio_wr_cb() { return m_write_nmio.bind(); }
	auto nmd1_rd_cb() { return m_read_nmd1.bind(); }
	auto pdet_rd_cb() { return m_read_pdet.bind(); }
	auto ncc1_wr_cb() { return m_write_ncc1.bind(); }
	auto ncc2_wr_cb() { return m_write_ncc2.bind(); }
	auto dtmf_wr_cb() { return m_write_dtmf.bind(); }
	auto contrast_wr_cb() { return m_write_contrast.bind(); }
	auto lcdc_rd_cb() { return m_read_lcdc.bind(); }
	auto lcdc_wr_cb() { return m_write_lcdc.bind(); }
	auto kop0_rd_cb() { return m_read_kop[0].bind(); }
	auto kop1_rd_cb() { return m_read_kop[1].bind(); }
	auto kop2_rd_cb() { return m_read_kop[2].bind(); }
	auto kop3_rd_cb() { return m_read_kop[3].bind(); }
	auto kop4_rd_cb() { return m_read_kop[4].bind(); }
	auto kop5_rd_cb() { return m_read_kop[5].bind(); }
	auto kop6_rd_cb() { return m_read_kop[6].bind(); }
	auto kop7_rd_cb() { return m_read_kop[7].bind(); }
	auto battery_rd_cb() { return m_read_battery.bind(); }

	void mem_map(address_map &map) ATTR_COLD;
	void io_map(address_map &map) ATTR_COLD;

	uint8_t mack_r();

protected:
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;

private:
	static constexpr uint8_t INTERRUPT_VECTOR[] = { 0x08, 0x09, 0x00 };

	static constexpr int INT_TICK     = 0;
	static constexpr int INT_KEYBOARD = 1;
	static constexpr int INT_ERROR    = 2;

	static constexpr int ROM_APP = 0b000; // 0
	static constexpr int CCM_A   = 0b011; // 3
	static constexpr int CCM_B   = 0b111; // 7
	static constexpr int ROM_EXT = 0b010; // 2

	void check_interrupt();
	void trigger_interrupt(int level);
	uint8_t irq_status_r();
	void irq_mask_w(uint8_t data);

	uint8_t keyboard_r() { return m_kbd_data; }
	void key_make(u8 row, u8 column);
	void key_break(u8 row, u8 column);
	TIMER_DEVICE_CALLBACK_MEMBER(keyboard_tick);

	void update_ccm_select();
	void select_w(uint8_t data);

	void power_w(uint8_t data);
	uint8_t battery_r();

	uint8_t counter_r(offs_t offset);
	void counter_w(offs_t offset, uint8_t data);
	TIMER_DEVICE_CALLBACK_MEMBER(counter_tick);
	TIMER_DEVICE_CALLBACK_MEMBER(nmi_tick);

	uint8_t contrast_r() { return m_contrast; }
	void contrast_w(uint8_t data) { m_contrast = data; m_write_contrast(data); }

	devcb_write_line m_write_pint;
	devcb_write_line m_write_nmio;
	devcb_read_line m_read_nmd1;
	devcb_read_line m_read_pdet;
	devcb_write_line m_write_ncc1;
	devcb_write_line m_write_ncc2;
	devcb_write8 m_write_dtmf;
	devcb_write8 m_write_contrast;
	devcb_read8 m_read_lcdc;
	devcb_write8 m_write_lcdc;
	devcb_read8::array<8> m_read_kop;
	devcb_read8 m_read_battery;

	required_device<timer_device> m_nmi_timer;
	memory_view m_rom_bank_view;

	const char *m_rom_tag = nullptr;

	bool m_sleep = false;
	u8 m_ip = 0;
	u8 m_ie = 0;
	u16 m_counter = 0;
	u8 m_contrast = 0x80;
	int m_rom_b = 0;
	u8 m_kbd_data = 0xff;
	u8 m_kop_state[8] = { 0 };
	u8 m_kop_row = 0;
};

DECLARE_DEVICE_TYPE(PORTFOLIO_ASIC, portfolio_asic_device)

#endif // MAME_ATARI_POFO_ASIC_H
