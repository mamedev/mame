/**********************************************************************

    OKI MSM58321RS Real Time Clock/Calendar emulation

    Copyright MESS Team.
    Visit http://mamedev.org for licensing and usage restrictions.

**********************************************************************
                            _____   _____
                   CS2   1 |*    \_/     | 16  Vdd
                 WRITE   2 |             | 15  XT
                  READ   3 |             | 14  _XT
                    D0   4 |   MSM58321  | 13  CS1
                    D1   5 |   RTC58321  | 12  TEST
                    D2   6 |             | 11  STOP
                    D3   7 |             | 10  _BUSY
                   GND   8 |_____________| 9   ADDRESS WRITE

**********************************************************************/

#pragma once

#ifndef __MSM58321__
#define __MSM58321__

#include "emu.h"



//**************************************************************************
//  INTERFACE CONFIGURATION MACROS
//**************************************************************************

#define MCFG_MSM58321_ADD(_tag, _clock, _config) \
	MCFG_DEVICE_ADD(_tag, MSM58321, _clock) \
	MCFG_DEVICE_CONFIG(_config)

#define MSM58321_INTERFACE(name) \
	const msm58321_interface (name) =



//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> msm58321_interface

struct msm58321_interface
{
	devcb_write_line    m_out_busy_cb;
};



// ======================> msm58321_device

class msm58321_device : public device_t,
						public device_rtc_interface,
						public msm58321_interface
{
public:
	// construction/destruction
	msm58321_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	DECLARE_READ8_MEMBER( read );
	DECLARE_WRITE8_MEMBER( write );

	DECLARE_WRITE_LINE_MEMBER( cs1_w );
	DECLARE_WRITE_LINE_MEMBER( cs2_w );
	DECLARE_WRITE_LINE_MEMBER( read_w );
	DECLARE_WRITE_LINE_MEMBER( write_w );
	DECLARE_WRITE_LINE_MEMBER( address_write_w );
	DECLARE_WRITE_LINE_MEMBER( stop_w );
	DECLARE_WRITE_LINE_MEMBER( test_w );
	DECLARE_READ_LINE_MEMBER( busy_r );

protected:
	// device-level overrides
	virtual void device_config_complete();
	virtual void device_start();
	virtual void device_reset();
	virtual void device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr);

	// device_rtc_interface overrides
	virtual void rtc_clock_updated(int year, int month, int day, int day_of_week, int hour, int minute, int second);

private:
	static const device_timer_id TIMER_CLOCK = 0;
	static const device_timer_id TIMER_BUSY = 1;

	inline int read_counter(int counter);
	inline void write_counter(int counter, int value);

	devcb_resolved_write_line   m_out_busy_func;

	int m_cs1;                  // chip select 1
	int m_cs2;                  // chip select 2
	int m_busy;                 // busy flag
	int m_read;                 // read data
	int m_write;                // write data
	int m_address_write;        // write address

	UINT8 m_reg[13];            // registers
	UINT8 m_latch;              // data latch (not present in real chip)
	UINT8 m_address;            // address latch

	// timers
	emu_timer *m_clock_timer;
	emu_timer *m_busy_timer;
};


// device type definition
extern const device_type MSM58321;



#endif
