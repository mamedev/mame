// license:BSD-3-Clause
// copyright-holders:Aaron Giles
/***************************************************************************

    Bally/Sente 6VB audio board emulation

***************************************************************************/

#ifndef MAME_MIDWAY_SENTE6VB_H
#define MAME_MIDWAY_SENTE6VB_H

#pragma once

#include "machine/6850acia.h"
#include "machine/pit8253.h"
#include "machine/timer.h"
#include "sound/cem3394.h"


class sente6vb_device : public device_t
{
	static constexpr unsigned POLY17_BITS = 17;
	static constexpr size_t POLY17_SIZE = (1 << POLY17_BITS) - 1;
	static constexpr unsigned POLY17_SHL = 7;
	static constexpr unsigned POLY17_SHR = 10;
	static constexpr uint32_t POLY17_ADD = 0x18000;

public:
	sente6vb_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock = 0);

	auto send_cb() { return m_send_cb.bind(); }
	auto clock_out_cb() { return m_clock_out_cb.bind(); }

	void rec_w(int state);

protected:
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual const tiny_rom_entry *device_rom_region() const override ATTR_COLD;
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

private:
	uint8_t counter_state_r();
	void counter_control_w(uint8_t data);
	void chip_select_w(uint8_t data);
	void dac_data_w(offs_t offset, uint8_t data);
	void register_addr_w(uint8_t data);
	void uart_clock_w(int state);
	void counter_0_set_out(int state);

	void update_counter_0_timer();
	TIMER_DEVICE_CALLBACK_MEMBER(clock_counter_0_ff);
	void set_counter_0_ff(int state);

	void mem_map(address_map &map) ATTR_COLD;
	void io_map(address_map &map) ATTR_COLD;

	required_device<pit8253_device> m_pit;
	required_device<timer_device> m_counter_0_timer;
	required_device_array<cem3394_device, 6> m_cem_device;
	required_device<cpu_device> m_audiocpu;
	required_device<acia6850_device> m_uart;

	devcb_write_line m_send_cb;
	devcb_write_line m_clock_out_cb;

	// manually clocked counter 0 states
	uint8_t m_counter_control;
	bool m_counter_0_ff;
	bool m_counter_0_out;
	bool m_counter_0_timer_active;

	// CEM3394 DAC control states
	uint16_t m_dac_value;
	uint8_t m_dac_register;
	uint8_t m_chip_select;

	// sound CPU 6850 states
	bool m_uint;
};

DECLARE_DEVICE_TYPE(SENTE6VB, sente6vb_device)

#endif // MAME_MIDWAY_SENTE6VB_H
