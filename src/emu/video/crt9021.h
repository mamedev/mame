// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    SMC CRT9021 Video Attributes Controller (VAC) emulation

    Copyright MESS Team.
    Visit http://mamedev.org for licensing and usage restrictions.

**********************************************************************
                            _____   _____
                    D0   1 |*    \_/     | 28  D1
                   MS0   2 |             | 27  D2
                   MS1   3 |             | 26  D3
                 REVID   4 |             | 25  D4
                 CHABL   5 |             | 24  D5
                 BLINK   6 |             | 23  D6
                 INTIN   7 |   CRT9021   | 22  D7
                   +5V   8 |             | 21  _VSYNC
                 ATTEN   9 |             | 20  GND
                INTOUT  10 |             | 19  SL0/SLD
                CURSOR  11 |             | 18  SL1/_SLG
                 RETBL  12 |             | 17  SL2/BLC
                _LD/SH  13 |             | 16  SL3/BKC
                 VIDEO  14 |_____________| 15  VDC

**********************************************************************/

#pragma once

#ifndef __CRT9021__
#define __CRT9021__

#include "emu.h"



//**************************************************************************
//  MACROS / CONSTANTS
//**************************************************************************




//**************************************************************************
//  INTERFACE CONFIGURATION MACROS
//**************************************************************************

#define MCFG_CRT9021_IN_DATA_CB(_devcb) \
	devcb = &crt9021_device::set_in_data_callback(*device, DEVCB2_##_devcb);

#define MCFG_CRT9021_IN_ATTR_CB(_devcb) \
	devcb = &crt9021_device::set_in_attr_callback(*device, DEVCB2_##_devcb);

#define MCFG_CRT9021_IN_ATTEN_CB(_devcb) \
	devcb = &crt9021_device::set_in_atten_callback(*device, DEVCB2_##_devcb);	

//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> crt9021_device

class crt9021_device :  public device_t,
						public device_video_interface
{
public:
	// construction/destruction
	crt9021_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
	
	template<class _Object> static devcb2_base &set_in_data_callback(device_t &device, _Object object) { return downcast<crt9021_device &>(device).m_in_data_cb.set_callback(object); }
	template<class _Object> static devcb2_base &set_in_attr_callback(device_t &device, _Object object) { return downcast<crt9021_device &>(device).m_in_attr_cb.set_callback(object); }
	template<class _Object> static devcb2_base &set_in_atten_callback(device_t &device, _Object object) { return downcast<crt9021_device &>(device).m_in_atten_cb.set_callback(object); }

	DECLARE_WRITE_LINE_MEMBER( slg_w );
	DECLARE_WRITE_LINE_MEMBER( sld_w );
	DECLARE_WRITE_LINE_MEMBER( cursor_w );
	DECLARE_WRITE_LINE_MEMBER( retbl_w );
	DECLARE_WRITE_LINE_MEMBER( vsync_w );

	UINT32 screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);

protected:
	// device-level overrides
	virtual void device_start();
	virtual void device_clock_changed();
	virtual void device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr);

private:
	devcb2_read8             m_in_data_cb;
	devcb2_read8             m_in_attr_cb;

	devcb2_read_line         m_in_atten_cb;

	int m_slg;
	int m_sld;
	int m_cursor;
	int m_retbl;
	int m_vsync;
};


// device type definition
extern const device_type CRT9021;



#endif
