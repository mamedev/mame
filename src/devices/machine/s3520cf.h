// license:BSD-3-Clause
// copyright-holders:Angelo Salese
/***************************************************************************

Seiko/Epson S-3520CF

***************************************************************************/

#ifndef MAME_MACHINE_S3520CF_H
#define MAME_MACHINE_S3520CF_H

#pragma once

#include "dirtc.h"


//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> s3520cf_device

class s3520cf_device :  public device_t,
						public device_nvram_interface,
						public device_rtc_interface
{
public:
	// construction/destruction
	s3520cf_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock = 32'768);

	// I/O operations
	int read_bit();
	void set_dir_line(int state);
	void set_cs_line(int state);
	void set_clock_line(int state);
	void write_bit(int state);
	TIMER_CALLBACK_MEMBER(timer_callback);

protected:
	s3520cf_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock);

	struct rtc_regs_t
	{
		u8 sec, min, hour, day, wday, month, year;
	};

	// device-level overrides
	virtual void device_validity_check(validity_checker &valid) const override;
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	// device_nvram_interface overrides
	virtual void nvram_default() override;
	virtual bool nvram_read(util::read_stream &file) override;
	virtual bool nvram_write(util::write_stream &file) override;

	// device_rtc_interface overrides
	virtual bool rtc_feature_y2k() const override { return false; }
	virtual bool rtc_feature_leap_year() const override { return true; }
	virtual void rtc_clock_updated(int year, int month, int day, int day_of_week, int hour, int minute, int second) override;

	optional_memory_region m_region;

	inline u8 rtc_read(u8 offset);
	inline void rtc_write(u8 offset, u8 data);
	void check_overflow();

	int m_dir;
	int m_latch;
	int m_reset_line;
	int m_read_latch;
	u8 m_bitstream, m_stream_pos;
	u8 m_mode, m_sysr, m_cntrl1, m_cntrl2;

	rtc_regs_t m_rtc;
	u8 m_nvdata[15];

	emu_timer *m_timer;
};

/***************************************************************************

Epson RTC-4553

***************************************************************************/

class rtc4553_device : public s3520cf_device
{
public:
	rtc4553_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock = 32'768);
};

// device type definition
DECLARE_DEVICE_TYPE(S3520CF, s3520cf_device)
DECLARE_DEVICE_TYPE(RTC4553, rtc4553_device)

#endif // MAME_MACHINE_S3520CF_H
