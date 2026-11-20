// license:BSD-3-Clause
// copyright-holders:Ville Linde
#ifndef MAME_BUS_CENTRONICS_ATARI_JAMMIT_H
#define MAME_BUS_CENTRONICS_ATARI_JAMMIT_H

#pragma once

#include "ctronics.h"

// Atari 5772-15642-02 JAMMIT cabinet interface, connected to the PC's DB25 LPT.
class atari_jammit_device : public device_t, public device_centronics_peripheral_interface
{
public:
	atari_jammit_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	auto system_callback() { return m_system_cb.bind(); }
	auto starts_callback() { return m_starts_cb.bind(); }
	auto output_callback() { return m_output_cb.bind(); }
	// Per player: bits 0-11 = vertical counter, 12-21 = horizontal counter,
	// bit 24 = trigger (including offscreen shot), bit 25 = offscreen indication.
	auto gun_callback() { return m_gun_cb.bind(); }

	virtual void input_data0(int state) override { data_w(0, state); }
	virtual void input_data1(int state) override { data_w(1, state); }
	virtual void input_data2(int state) override { data_w(2, state); }
	virtual void input_data3(int state) override { data_w(3, state); }
	virtual void input_data4(int state) override { data_w(4, state); }
	virtual void input_data5(int state) override { data_w(5, state); }
	virtual void input_data6(int state) override { data_w(6, state); }
	virtual void input_data7(int state) override { data_w(7, state); }
	virtual void input_strobe(int state) override;

protected:
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

private:
	void data_w(unsigned bit, int state);
	void clock_command();
	uint8_t input_r();
	void update_outputs();
	void update_status();
	void postload();

	devcb_read16 m_system_cb;
	devcb_read8 m_starts_cb;
	devcb_read32 m_gun_cb;
	devcb_write16 m_output_cb;
	uint8_t m_data = 0xff;
	bool m_strobe = true;
	uint8_t m_pointer = 0;
	uint8_t m_reply = 0;
	uint16_t m_lfsr = 0;
	uint16_t m_previous = 0;
	uint16_t m_output_latch = 0;
	uint16_t m_gun_latch[4]{};
	uint8_t m_gun_flags = 0;
};

DECLARE_DEVICE_TYPE(ATARI_JAMMIT, atari_jammit_device)

#endif // MAME_BUS_CENTRONICS_ATARI_JAMMIT_H
