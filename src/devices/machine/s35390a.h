// license:BSD-3-Clause
// copyright-holders:Peter Wilhelmsen

#ifndef MAME_MACHINE_S35390A_H
#define MAME_MACHINE_S35390A_H

#pragma once

#include "dirtc.h"

class s35390a_device : public device_t, public device_rtc_interface, public device_nvram_interface
{
public:
	s35390a_device(machine_config const &mconfig, char const *tag, device_t *owner, u32 clock);

	// Byte-level I2C interface. Bit 8 marks an address byte.
	u8 transmit(offs_t data);
	u8 receive();
	void stop(int state) { m_selected = false; }
	void acknowledge(int state);

protected:
	// device_t implementation
	virtual void device_start() override ATTR_COLD;

	// device_rtc_interface implementation
	virtual bool rtc_feature_leap_year() const override;
	virtual void rtc_clock_updated(int year, int month, int day, int day_of_week, int hour, int minute, int second) override;

	// device_nvram_interface implementation
	virtual void nvram_default() override ATTR_COLD;
	virtual bool nvram_read(util::read_stream &file) override ATTR_COLD;
	virtual bool nvram_write(util::write_stream &file) override ATTR_COLD;

private:
	static u8 reverse(u8 data) { return bitswap<8>(data, 0, 1, 2, 3, 4, 5, 6, 7); }
	void apply_time();
	void reset_registers();
	TIMER_CALLBACK_MEMBER(tick);

	u8 m_regs[8][7];
	u8 m_time[7];
	u8 m_transfer[7];
	u8 m_command;
	u8 m_pos;
	u8 m_length;
	bool m_selected;
	bool m_read;
	emu_timer *m_timer;
};

DECLARE_DEVICE_TYPE(S35390A, s35390a_device)

#endif // MAME_MACHINE_S35390A_H
