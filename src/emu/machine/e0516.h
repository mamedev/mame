/**********************************************************************

    E05-16 Real Time Clock emulation

    Copyright MESS Team.
    Visit http://mamedev.org for licensing and usage restrictions.

**********************************************************************
                            _____   _____
                  Vdd1   1 |*    \_/     | 16  Vdd2
                OSC IN   2 |             | 15  Clk
               OSC OUT   3 |             | 14  XOUT
                 _STOP   4 |   E05-16    | 13  DI/O
                _RESET   5 |   E050-16   | 12  _SEC
               _OUTSEL   6 |             | 11  _MIN
                  _DAY   7 |             | 10  _HRS
                   Vss   8 |_____________| 9   _CS

**********************************************************************/

#pragma once

#ifndef __E0516__
#define __E0516__

#include "emu.h"



//**************************************************************************
//  INTERFACE CONFIGURATION MACROS
//**************************************************************************

#define MCFG_E0516_ADD(_tag, _clock) \
	MCFG_DEVICE_ADD(_tag, E0516, _clock)



//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************


// ======================> e0516_device_config

class e0516_device_config :   public device_config,
							  public device_config_rtc_interface
{
    friend class e0516_device;

    // construction/destruction
    e0516_device_config(const machine_config &mconfig, const char *tag, const device_config *owner, UINT32 clock);

public:
    // allocators
    static device_config *static_alloc_device_config(const machine_config &mconfig, const char *tag, const device_config *owner, UINT32 clock);
    virtual device_t *alloc_device(running_machine &machine) const;

protected:
};


// ======================> e0516_device

class e0516_device :  public device_t,
					  public device_rtc_interface
{
    friend class e0516_device_config;

    // construction/destruction
    e0516_device(running_machine &_machine, const e0516_device_config &_config);

public:
	DECLARE_WRITE_LINE_MEMBER( cs_w );
	DECLARE_WRITE_LINE_MEMBER( clk_w );
	DECLARE_WRITE_LINE_MEMBER( dio_w );
	DECLARE_READ_LINE_MEMBER( dio_r );

protected:
    // device-level overrides
    virtual void device_start();
    virtual void device_reset();
	virtual void device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr);

	// device_rtc_interface overrides
	virtual void rtc_set_time(int year, int month, int day, int day_of_week, int hour, int minute, int second);
	virtual bool rtc_is_year_2000_compliant() { return false; }

private:
	inline void advance_seconds();

	int m_cs;						// chip select
	int m_clk;						// clock
	int m_data_latch;				// data latch
	int m_reg_latch;				// register latch
	int m_read_write;				// read/write data
	int m_state;					// state
	int m_bits;						// number of bits transferred
	int m_dio;						// data pin

	UINT8 m_register[8];			// registers

	// timers
	emu_timer *m_timer;

	const e0516_device_config &m_config;
};


// device type definition
extern const device_type E0516;



#endif
