// license:BSD-3-Clause
// copyright-holders:R. Belmont
/**********************************************************************

    macrtc.h - Apple 343-0042 real time clock and battery RAM
    by R. Belmont

**********************************************************************/

#pragma once

#ifndef __RTC3430042_H__
#define __RTC3430042_H__

#include "emu.h"


//**************************************************************************
//  INTERFACE CONFIGURATION MACROS
//**************************************************************************

#define MCFG_RTC3430042_ADD(_tag, _clock) \
	MCFG_DEVICE_ADD(_tag, RTC3430042, _clock)



//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> rtc3430042_device

class rtc3430042_device :  public device_t,
						public device_rtc_interface,
						public device_nvram_interface
{
public:
	// construction/destruction
	rtc3430042_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	DECLARE_WRITE_LINE_MEMBER( ce_w );
	DECLARE_WRITE_LINE_MEMBER( clk_w );
	DECLARE_READ_LINE_MEMBER( data_r );
	DECLARE_WRITE_LINE_MEMBER( data_w );

protected:
	// device-level overrides
	virtual void device_start();
	virtual void device_reset();
	virtual void device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr);

	// device_rtc_interface overrides
	virtual void rtc_clock_updated(int year, int month, int day, int day_of_week, int hour, int minute, int second);
	virtual bool rtc_feature_leap_year() { return true; }

	// device_nvram_interface overrides
	virtual void nvram_default();
	virtual void nvram_read(emu_file &file);
	virtual void nvram_write(emu_file &file);

private:
	/* state of rTCEnb and rTCClk lines */
	UINT8 m_rtc_rTCEnb;
	UINT8 m_rtc_rTCClk;

	/* serial transmit/receive register : bits are shifted in/out of this byte */
	UINT8 m_rtc_data_byte;
	/* serial transmitted/received bit count */
	UINT8 m_rtc_bit_count;
	/* direction of the current transfer (0 : VIA->RTC, 1 : RTC->VIA) */
	UINT8 m_rtc_data_dir;
	/* when rtc_data_dir == 1, state of rTCData as set by RTC (-> data bit seen by VIA) */
	UINT8 m_rtc_data_out;

	/* set to 1 when command in progress */
	UINT8 m_rtc_cmd;

	/* write protect flag */
	UINT8 m_rtc_write_protect;

	/* internal seconds register */
	UINT8 m_rtc_seconds[/*8*/4];
	/* 20-byte long PRAM, or 256-byte long XPRAM */
	UINT8 m_pram[256];
	/* current extended address and RTC state */
	UINT8 m_rtc_xpaddr;
	UINT8 m_rtc_state;
	UINT8 m_data_latch;

	// timers
	emu_timer *m_clock_timer;

	void rtc_shift_data(int data);
	void rtc_execute_cmd(int data);
};


// device type definition
extern const device_type RTC3430042;

#endif
