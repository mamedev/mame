// license:BSD-3-Clause
// copyright-holders:Angelo Salese
/***************************************************************************

Seiko/Epson S-3520CF

***************************************************************************/

#ifndef MAME_MACHINE_S3520CF_H
#define MAME_MACHINE_S3520CF_H

#pragma once


//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> s3520cf_device

class s3520cf_device :  public device_t
{
public:
	// construction/destruction
	s3520cf_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock = 32'768);

	// I/O operations
	DECLARE_READ_LINE_MEMBER( read_bit );
	DECLARE_WRITE_LINE_MEMBER( set_dir_line );
	DECLARE_WRITE_LINE_MEMBER( set_cs_line );
	DECLARE_WRITE_LINE_MEMBER( set_clock_line );
	DECLARE_WRITE_LINE_MEMBER( write_bit );
	TIMER_CALLBACK_MEMBER(timer_callback);

protected:
	enum state_t
	{
		RTC_SET_ADDRESS = 0,
		RTC_SET_DATA
	};

	struct rtc_regs_t
	{
		uint8_t sec, min, hour, day, wday, month, year;
	};

	// device-level overrides
	virtual void device_validity_check(validity_checker &valid) const override;
	virtual void device_start() override;
	virtual void device_reset() override;
	inline uint8_t rtc_read(uint8_t offset);
	inline void rtc_write(uint8_t offset,uint8_t data);

	int m_dir;
	int m_latch;
	int m_reset_line;
	int m_read_latch;
	uint8_t m_current_cmd;
	uint8_t m_cmd_stream_pos;
	uint8_t m_rtc_addr;
	uint8_t m_mode, m_sysr;

	state_t m_rtc_state;
	rtc_regs_t m_rtc;

	emu_timer *m_timer;
};


// device type definition
DECLARE_DEVICE_TYPE(S3520CF, s3520cf_device)

#endif // MAME_MACHINE_S3520CF_H
