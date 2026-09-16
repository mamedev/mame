// license:BSD-3-Clause
// copyright-holders:Angelo Salese, David Haywood
/***************************************************************************

    rtc9701.h

    Serial rtc9701s.

***************************************************************************/

#ifndef MAME_MACHINE_RTC9701_H
#define MAME_MACHINE_RTC9701_H

#pragma once

#include "dirtc.h"


//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************


// ======================> rtc9701_device

class rtc9701_device :  public device_t,
						public device_nvram_interface,
						public device_rtc_interface
{
public:
	// construction/destruction
	rtc9701_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock = 32'768);


	// I/O operations
	void write_bit(int state);
	int read_bit();
	void set_cs_line(int state);
	void set_clock_line(int state);
	TIMER_CALLBACK_MEMBER(timer_callback);

protected:
	enum class state_t : u8
	{
		CMD_WAIT = 0,
		RTC_READ,
		RTC_WRITE,
		EEPROM_READ,
		EEPROM_WRITE,
		AFTER_WRITE_ENABLE

	};

	struct regs_t
	{
		uint8_t sec, min, hour, day, wday, month, year;
	};

	// device-level overrides
	virtual void device_validity_check(validity_checker &valid) const override;
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	// device_nvram_interface overrides
	virtual void nvram_default() override;
	virtual bool nvram_read(util::read_stream &file) override;
	virtual bool nvram_write(util::write_stream &file) override;
	inline uint8_t rtc_read(uint8_t offset);
	inline void rtc_write(uint8_t offset,uint8_t data);

	// device_rtc_interface overrides
	virtual bool rtc_feature_y2k() const override { return false; }
	virtual bool rtc_feature_leap_year() const override { return true; }
	virtual void rtc_clock_updated(int year, int month, int day, int day_of_week, int hour, int minute, int second) override;

	int                     m_latch;
	int                     m_reset_line;
	int                     m_clock_line;


	state_t rtc_state;
	int cmd_stream_pos;
	int current_cmd;

	int rtc9701_address_pos;
	int rtc9701_current_address;

	uint16_t rtc9701_current_data;
	int rtc9701_data_pos;

	uint16_t rtc9701_data[0x100];

	regs_t m_rtc;

	emu_timer *m_timer;
};


// device type definition
DECLARE_DEVICE_TYPE(RTC9701, rtc9701_device)

#endif // MAME_MACHINE_RTC9701_H
