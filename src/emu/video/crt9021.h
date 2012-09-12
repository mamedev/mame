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

#define MCFG_CRT9021_ADD(_tag, _clock, _config) \
	MCFG_DEVICE_ADD(_tag, CRT9021, _clock) \
	MCFG_DEVICE_CONFIG(_config)


#define CRT9021_INTERFACE(name) \
	const crt9021_interface (name) =



//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************


// ======================> crt9021_interface

struct crt9021_interface
{
	const char *screen_tag;		/* screen we are acting on */

	devcb_read8				in_data_cb;
	devcb_read8				in_attr_cb;

	devcb_read_line			in_atten_cb;
};



// ======================> crt9021_device

class crt9021_device :	public device_t,
                        public crt9021_interface
{
public:
    // construction/destruction
    crt9021_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	DECLARE_WRITE_LINE_MEMBER( slg_w );
	DECLARE_WRITE_LINE_MEMBER( sld_w );
	DECLARE_WRITE_LINE_MEMBER( cursor_w );
	DECLARE_WRITE_LINE_MEMBER( retbl_w );
	DECLARE_WRITE_LINE_MEMBER( vsync_w );

	UINT32 screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);

protected:
    // device-level overrides
	virtual void device_config_complete();
    virtual void device_start();
	virtual void device_clock_changed();
	virtual void device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr);

private:
	devcb_resolved_read8			m_in_data_func;
	devcb_resolved_read8			m_in_attr_func;
	devcb_resolved_read_line		m_in_atten_func;

	screen_device *m_screen;

	int m_slg;
	int m_sld;
	int m_cursor;
	int m_retbl;
	int m_vsync;
};


// device type definition
extern const device_type CRT9021;



#endif
