// license:BSD-3-Clause
// copyright-holders:Aaron Giles
/***************************************************************************

    Bally/Sente 6VB audio board emulation

***************************************************************************/

#ifndef MAME_AUDIO_SENTE6VB_H
#define MAME_AUDIO_SENTE6VB_H

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

	DECLARE_WRITE_LINE_MEMBER(rec_w);

protected:
	virtual void device_add_mconfig(machine_config &config) override;
	virtual const tiny_rom_entry *device_rom_region() const override;
	virtual void device_start() override;
	virtual void device_reset() override;

private:
	DECLARE_READ8_MEMBER(counter_state_r);
	DECLARE_WRITE8_MEMBER(counter_control_w);
	DECLARE_WRITE8_MEMBER(chip_select_w);
	DECLARE_WRITE8_MEMBER(dac_data_w);
	DECLARE_WRITE8_MEMBER(register_addr_w);
	DECLARE_WRITE_LINE_MEMBER(uart_clock_w);
	DECLARE_WRITE_LINE_MEMBER(counter_0_set_out);

	void update_counter_0_timer();
	TIMER_DEVICE_CALLBACK_MEMBER(clock_counter_0_ff);
	void poly17_init();
	DECLARE_WRITE_LINE_MEMBER(set_counter_0_ff);
	inline void noise_gen_chip(int chip, int count, short *buffer);
	CEM3394_EXT_INPUT(noise_gen_0);
	CEM3394_EXT_INPUT(noise_gen_1);
	CEM3394_EXT_INPUT(noise_gen_2);
	CEM3394_EXT_INPUT(noise_gen_3);
	CEM3394_EXT_INPUT(noise_gen_4);
	CEM3394_EXT_INPUT(noise_gen_5);

	void mem_map(address_map &map);
	void io_map(address_map &map);

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

	// random number generator states
	uint8_t m_poly17[POLY17_SIZE + 1];

	// CEM3394 DAC control states
	uint16_t m_dac_value;
	uint8_t m_dac_register;
	uint8_t m_chip_select;

	// sound CPU 6850 states
	bool m_uint;

	// noise generator states
	uint32_t m_noise_position[6];
};

DECLARE_DEVICE_TYPE(SENTE6VB, sente6vb_device)

#endif // MAME_AUDIO_SENTE6VB_H
