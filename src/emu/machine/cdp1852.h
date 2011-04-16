/**********************************************************************

    RCA CDP1852 Byte-Wide Input/Output Port emulation

    Copyright MESS Team.
    Visit http://mamedev.org for licensing and usage restrictions.

**********************************************************************
                            _____   _____
              CSI/_CSI   1 |*    \_/     | 24  Vdd
                  MODE   2 |             | 23  _SR/SR
                   DI0   3 |             | 22  DI7
                   DO0   4 |             | 21  DO7
                   DI1   5 |             | 20  DI6
                   DO1   6 |   CDP1852   | 19  DO6
                   DI2   7 |             | 18  DI5
                   DO2   8 |             | 17  DO5
                   DI3   9 |             | 16  DI4
                   DO3  10 |             | 15  DO4
                 CLOCK  11 |             | 14  _CLEAR
                   Vss  12 |_____________| 13  CS2

**********************************************************************/

#pragma once

#ifndef __CDP1852__
#define __CDP1852__

#include "emu.h"



//**************************************************************************
//  MACROS / CONSTANTS
//**************************************************************************

#define CDP1852_CLOCK_HIGH	0

#define CDP1852_MODE_INPUT \
	DEVCB_LINE_GND

#define CDP1852_MODE_OUTPUT \
	DEVCB_LINE_VCC



//**************************************************************************
//  INTERFACE CONFIGURATION MACROS
//**************************************************************************

#define MCFG_CDP1852_ADD(_tag, _clock, _config) \
	MCFG_DEVICE_ADD(_tag, CDP1852, _clock) \
	MCFG_DEVICE_CONFIG(_config)


#define CDP1852_INTERFACE(name) \
	const cdp1852_interface (name)=



//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> cdp1852_interface

struct cdp1852_interface
{
	devcb_read_line			m_in_mode_func;

	devcb_read8				m_in_data_func;
	devcb_write8			m_out_data_func;

	devcb_write_line		m_out_sr_func;
};


// ======================> cdp1852_device_config

class cdp1852_device_config :   public device_config,
                                public cdp1852_interface
{
    friend class cdp1852_device;

    // construction/destruction
    cdp1852_device_config(const machine_config &mconfig, const char *tag, const device_config *owner, UINT32 clock);

public:
    // allocators
    static device_config *static_alloc_device_config(const machine_config &mconfig, const char *tag, const device_config *owner, UINT32 clock);
    virtual device_t *alloc_device(running_machine &machine) const;

protected:
    // device_config overrides
    virtual void device_config_complete();
};


// ======================> cdp1852_device

class cdp1852_device :  public device_t
{
    friend class cdp1852_device_config;

    // construction/destruction
    cdp1852_device(running_machine &_machine, const cdp1852_device_config &_config);

public:
    DECLARE_READ8_MEMBER( read );
    DECLARE_WRITE8_MEMBER( write );

protected:
    // device-level overrides
    virtual void device_start();
    virtual void device_reset();
	virtual void device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr);

private:
	int get_mode();
	inline void set_sr_line(int state);

	devcb_resolved_read_line	m_in_mode_func;
	devcb_resolved_write_line	m_out_sr_func;
	devcb_resolved_read8		m_in_data_func;
	devcb_resolved_write8		m_out_data_func;

	int m_new_data;				// new data written
	UINT8 m_data;				// data latch
	UINT8 m_next_data;			// next data

	int m_sr;					// service request flag
	int m_next_sr;				// next value of service request flag

	// timers
	emu_timer *m_scan_timer;

	const cdp1852_device_config &m_config;
};


// device type definition
extern const device_type CDP1852;



#endif
