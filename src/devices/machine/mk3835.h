// license:BSD-3-Clause
// copyright-holders:Curt Coder
/***************************************************************************

    Mostek MK3835N/MK3831N CMOS Microcomputer Clock/RAM emulation

****************************************************************************/

#ifndef MAME_MACHINE_MK3835_H
#define MAME_MACHINE_MK3835_H

#pragma once

#include "dirtc.h"


class mk3835_device : public device_t, public device_rtc_interface, public device_nvram_interface
{
public:
	mk3835_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock = 0);

	void ce_w(int state);
	void sclk_w(int state);
	void io_w(int state);
	int io_r() const { return m_io_out; }

protected:
	virtual void device_start() override ATTR_COLD;
	virtual void nvram_default() override;
	virtual bool nvram_read(util::read_stream &file) override;
	virtual bool nvram_write(util::write_stream &file) override;
	virtual void rtc_clock_updated(int year, int month, int day, int day_of_week, int hour, int minute, int second) override;

private:
	TIMER_CALLBACK_MEMBER(clock_tick);

	void receive_bit();
	void begin_command();
	void advance_output();
	void write_byte(u8 data);
	u8 read_byte() const;
	void restore_clock();

	optional_memory_region m_region;
	emu_timer *m_clock_timer;
	std::array<u8, 8> m_rtc;
	std::array<u8, 24> m_ram;
	u8 m_ce;
	u8 m_sclk;
	u8 m_io_in;
	u8 m_io_out;
	u8 m_shift;
	u8 m_bits;
	u8 m_address;
	bool m_ram_selected;
	bool m_reading;
	bool m_command_received;
	bool m_burst;
	bool m_transfer_done;
	bool m_time_initialized;
};

DECLARE_DEVICE_TYPE(MK3835, mk3835_device)

#endif // MAME_MACHINE_MK3835_H
