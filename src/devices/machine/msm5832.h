// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    OKI MSM5832 Real Time Clock/Calendar emulation

**********************************************************************
                            _____   _____
                   Vdd   1 |*    \_/     | 18  HOLD
                 WRITE   2 |             | 17  _XT
                  READ   3 |             | 16  XT
                    A0   4 |             | 15  +- 30 ADJ
                    A1   5 |   MSM5832   | 14  TEST
                    A2   6 |             | 13  GND
                    A3   7 |             | 12  D3
                    CS   8 |             | 11  D2
                    D0   9 |_____________| 10  D1

**********************************************************************/

#pragma once

#ifndef __MSM5832__
#define __MSM5832__

#include "emu.h"



//**************************************************************************
//  INTERFACE CONFIGURATION MACROS
//**************************************************************************

#define MCFG_MSM5832_ADD(_tag, _clock) \
	MCFG_DEVICE_ADD(_tag, MSM5832, _clock)



//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> msm5832_device

class msm5832_device :  public device_t,
						public device_rtc_interface
{
public:
	// construction/destruction
	msm5832_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	DECLARE_READ8_MEMBER( data_r );
	DECLARE_WRITE8_MEMBER( data_w );

	void address_w(UINT8 data);

	DECLARE_WRITE_LINE_MEMBER( adj_w );
	DECLARE_WRITE_LINE_MEMBER( test_w );
	DECLARE_WRITE_LINE_MEMBER( hold_w );

	DECLARE_WRITE_LINE_MEMBER( read_w );
	DECLARE_WRITE_LINE_MEMBER( write_w );
	DECLARE_WRITE_LINE_MEMBER( cs_w );

protected:
	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;
	virtual void device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr) override;

	// device_rtc_interface overrides
	virtual void rtc_clock_updated(int year, int month, int day, int day_of_week, int hour, int minute, int second) override;

private:
	static const device_timer_id TIMER_CLOCK = 0;

	inline int read_counter(int counter);
	inline void write_counter(int counter, int value);

	UINT8 m_reg[13];            // registers

	int m_hold;                 // counter hold
	int m_address;              // address

	int m_read;
	int m_write;
	int m_cs;

	// timers
	emu_timer *m_clock_timer;
};


// device type definition
extern const device_type MSM5832;



#endif
