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

class s3520cf_device :  public device_t,
						public device_nvram_interface
{
public:
	// construction/destruction
	s3520cf_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock = 32'768);

	// I/O operations
	DECLARE_READ_LINE_MEMBER( read_bit );
	DECLARE_WRITE_LINE_MEMBER( set_dir_line );
	DECLARE_WRITE_LINE_MEMBER( set_cs_line );
	DECLARE_WRITE_LINE_MEMBER( set_clock_line );
	DECLARE_WRITE_LINE_MEMBER( write_bit );
	TIMER_CALLBACK_MEMBER(timer_callback);

protected:
	s3520cf_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock);

	struct rtc_regs_t
	{
		u8 sec, min, hour, day, wday, month, year;
	};

	// device-level overrides
	virtual void device_validity_check(validity_checker &valid) const override;
	virtual void device_start() override;
	virtual void device_reset() override;

	// device_nvram_interface overrides
	virtual void nvram_default() override;
	virtual bool nvram_read(util::read_stream &file) override;
	virtual bool nvram_write(util::write_stream &file) override;

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
