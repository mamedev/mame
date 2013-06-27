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

#define MCFG_COM8116_ADD(_tag, _clock, _fx4, _fr, _ft) \
	MCFG_DEVICE_ADD(_tag, COM8116, _clock) \
	downcast<com8116_device *>(device)->set_fx4_callback(DEVCB2_##_fx4); \
	downcast<com8116_device *>(device)->set_fr_callback(DEVCB2_##_fr); \
	downcast<com8116_device *>(device)->set_ft_callback(DEVCB2_##_ft);



///*************************************************************************
//  TYPE DEFINITIONS
///*************************************************************************

// ======================> com8116_device

class com8116_device :  public device_t
{
public:
	// construction/destruction
	com8116_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	template<class _fx4> void set_fx4_callback(_fx4 fx4) { m_write_fx4.set_callback(fx4); }
	template<class _fr> void set_fr_callback(_fr fr) { m_write_fr.set_callback(fr); }
	template<class _ft> void set_ft_callback(_ft ft) { m_write_ft.set_callback(ft); }

	void str_w(UINT8 data);
	DECLARE_WRITE8_MEMBER( str_w );
	void stt_w(UINT8 data);
	DECLARE_WRITE8_MEMBER( stt_w );

	static const int divisors_16X_5_0688MHz[];
	static const int divisors_16X_4_9152MHz[];
	static const int divisors_32X_5_0688MHz[];

protected:
	// device-level overrides
	virtual void device_start();
	virtual void device_reset();
	virtual void device_timer(emu_timer &timer, device_timer_id id, int m_param, void *ptr);

private:
	enum
	{
		TIMER_FX4,
		TIMER_FR,
		TIMER_FT
	};

	devcb2_write_line   m_write_fx4;
	devcb2_write_line   m_write_fr;
	devcb2_write_line   m_write_ft;

	int m_fr;                       // receiver frequency
	int m_ft;                       // transmitter frequency

	const int *m_fr_divisors;
	const int *m_ft_divisors;

	// timers
	emu_timer *m_fx4_timer;
	emu_timer *m_fr_timer;
	emu_timer *m_ft_timer;
};


// device type definition
extern const device_type COM8116;



#endif
