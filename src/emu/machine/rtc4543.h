/**********************************************************************

    rtc4543.h - Epson R4543 real-time clock emulation
    by R. Belmont

**********************************************************************/

#pragma once

#ifndef __RTC4543_H__
#define __RTC4543_H__

#include "emu.h"



//**************************************************************************
//  INTERFACE CONFIGURATION MACROS
//**************************************************************************

#define MCFG_RTC4543_ADD(_tag, _clock) \
	MCFG_DEVICE_ADD(_tag, RTC4543, _clock)



//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> rtc4543_device

class rtc4543_device :  public device_t,
						public device_rtc_interface
{
public:
	// construction/destruction
	rtc4543_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	DECLARE_WRITE_LINE_MEMBER( ce_w );
	DECLARE_WRITE_LINE_MEMBER( wr_w );
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

private:
	int m_ce;
	int m_clk;
	int m_wr;
	int m_data;
	int m_regs[7];
	int m_curreg;
	int m_bit;

	// timers
	emu_timer *m_clock_timer;
};


// device type definition
extern const device_type RTC4543;

#endif
