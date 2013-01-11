/**********************************************************************

    COM8116 Dual Baud Rate Generator (Programmable Divider) emulation

    Copyright MESS Team.
    Visit http://mamedev.org for licensing and usage restrictions.

**********************************************************************
                            _____   _____
             XTAL/EXT1   1 |*    \_/     | 18  XTAL/EXT2
                   +5V   2 |             | 17  fT
                    fR   3 |             | 16  Ta
                    Ra   4 |   COM8116   | 15  Tb
                    Rb   5 |   COM8116T  | 14  Tc
                    Rc   6 |   COM8136   | 13  Td
                    Rd   7 |   COM8136T  | 12  STT
                   STR   8 |             | 11  GND
                    NC   9 |_____________| 10  fX/4

**********************************************************************/


#pragma once

#ifndef __COM8116__
#define __COM8116__

#include "emu.h"



///*************************************************************************
//  INTERFACE CONFIGURATION MACROS
///*************************************************************************

#define MCFG_COM8116_ADD(_tag, _clock, _config) \
	MCFG_DEVICE_ADD(_tag, COM8116, _clock)  \
	MCFG_DEVICE_CONFIG(_config)


#define COM8116_INTERFACE(name) \
	const com8116_interface (name) =



///*************************************************************************
//  TYPE DEFINITIONS
///*************************************************************************

// ======================> com8116_interface

struct com8116_interface
{
	devcb_write_line        m_out_fx4_cb;
	devcb_write_line        m_out_fr_cb;
	devcb_write_line        m_out_ft_cb;

	// receiver divisor ROM (19-bit)
	UINT32 m_fr_divisors[16];

	// transmitter divisor ROM (19-bit)
	UINT32 m_ft_divisors[16];
};


// ======================> com8116_device

class com8116_device :  public device_t,
						public com8116_interface
{
public:
	// construction/destruction
	com8116_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	DECLARE_WRITE8_MEMBER( str_w );
	DECLARE_WRITE8_MEMBER( stt_w );

protected:
	// device-level overrides
	virtual void device_config_complete();
	virtual void device_start();
	virtual void device_timer(emu_timer &timer, device_timer_id id, int m_param, void *ptr);

private:
	static const device_timer_id TIMER_FX4 = 0;
	static const device_timer_id TIMER_FR = 1;
	static const device_timer_id TIMER_FT = 2;

	devcb_resolved_write_line   m_out_fx4_func;
	devcb_resolved_write_line   m_out_fr_func;
	devcb_resolved_write_line   m_out_ft_func;

	int m_fr;                       // receiver frequency
	int m_ft;                       // transmitter frequency

	// timers
	emu_timer *m_fx4_timer;
	emu_timer *m_fr_timer;
	emu_timer *m_ft_timer;
};


// device type definition
extern const device_type COM8116;



#endif
