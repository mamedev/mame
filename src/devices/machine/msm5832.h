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

#ifndef MAME_MACHINE_MSM5832_H
#define MAME_MACHINE_MSM5832_H

#pragma once

#include "dirtc.h"



//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> msm5832_device

class msm5832_device :  public device_t,
						public device_rtc_interface
{
public:
	// construction/destruction
	msm5832_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	uint8_t data_r();
	void data_w(uint8_t data);

	void address_w(uint8_t data);

	DECLARE_WRITE_LINE_MEMBER( adj_w );
	DECLARE_WRITE_LINE_MEMBER( test_w );
	DECLARE_WRITE_LINE_MEMBER( hold_w );

	DECLARE_WRITE_LINE_MEMBER( read_w );
	DECLARE_WRITE_LINE_MEMBER( write_w );
	DECLARE_WRITE_LINE_MEMBER( cs_w );

protected:
	// device-level overrides
	virtual void device_start() override;
	virtual void device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr) override;

	// device_rtc_interface overrides
	virtual void rtc_clock_updated(int year, int month, int day, int day_of_week, int hour, int minute, int second) override;

private:
	static constexpr device_timer_id TIMER_CLOCK = 0;

	inline int read_counter(int counter);
	inline void write_counter(int counter, int value);

	uint8_t m_reg[13];            // registers

	int m_hold;                 // counter hold

	uint8_t m_address;              // address
	uint8_t m_data;                 // latched data

	int m_read;
	int m_write;
	int m_cs;

	// timers
	emu_timer *m_clock_timer;
};


// device type definition
DECLARE_DEVICE_TYPE(MSM5832, msm5832_device)

#endif // MAME_MACHINE_MSM5832_H
