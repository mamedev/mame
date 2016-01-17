// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    SMC CRT9021 Video Attributes Controller (VAC) emulation

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
//  INTERFACE CONFIGURATION MACROS
//**************************************************************************

#define CRT9021_DRAW_CHARACTER_MEMBER(_name) void _name(bitmap_rgb32 &bitmap, int y, int x, UINT8 video, int intout)


#define MCFG_CRT9021_DRAW_CHARACTER_CALLBACK_OWNER(_class, _method) \
	crt9021_t::static_set_display_callback(*device, crt9021_draw_character_delegate(&_class::_method, #_class "::" #_method, downcast<_class *>(owner)));


//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

typedef device_delegate<void (bitmap_rgb32 &bitmap, int y, int x, UINT8 video, int intout)> crt9021_draw_character_delegate;


// ======================> crt9021_t

class crt9021_t :  public device_t,
					public device_video_interface
{
public:
	// construction/destruction
	crt9021_t(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock);

	static void static_set_display_callback(device_t &device, crt9021_draw_character_delegate callback) { downcast<crt9021_t &>(device).m_display_cb = callback; }

	void write(UINT8 data) { m_data = data; }
	DECLARE_WRITE8_MEMBER( write ) { write(data); }
	DECLARE_WRITE_LINE_MEMBER( ms0_w ) { m_ms0 = state; }
	DECLARE_WRITE_LINE_MEMBER( ms1_w ) { m_ms1 = state; }
	DECLARE_WRITE_LINE_MEMBER( revid_w ) { m_revid = state; }
	DECLARE_WRITE_LINE_MEMBER( chabl_w ) { m_chabl = state; }
	DECLARE_WRITE_LINE_MEMBER( blink_w ) { m_blink = state; }
	DECLARE_WRITE_LINE_MEMBER( intin_w ) { m_intin = state; }
	DECLARE_WRITE_LINE_MEMBER( atten_w ) { m_atten = state; }
	DECLARE_WRITE_LINE_MEMBER( cursor_w ) { m_cursor = state; }
	DECLARE_WRITE_LINE_MEMBER( retbl_w ) { m_retbl = state; }
	DECLARE_WRITE_LINE_MEMBER( ld_sh_w );
	DECLARE_WRITE_LINE_MEMBER( sld_w ) { m_sld = state; }
	DECLARE_WRITE_LINE_MEMBER( slg_w ) { m_slg = state; }
	DECLARE_WRITE_LINE_MEMBER( blc_w ) { m_blc = state; }
	DECLARE_WRITE_LINE_MEMBER( bkc_w ) { m_bkc = state; }
	DECLARE_WRITE_LINE_MEMBER( sl0_w ) { m_sl0 = state; }
	DECLARE_WRITE_LINE_MEMBER( sl1_w ) { m_sl1 = state; }
	DECLARE_WRITE_LINE_MEMBER( sl2_w ) { m_sl2 = state; }
	DECLARE_WRITE_LINE_MEMBER( sl3_w ) { m_sl3 = state; }
	DECLARE_WRITE_LINE_MEMBER( vsync_w );

	UINT32 screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);

protected:
	// device-level overrides
	virtual void device_start() override;

private:
	enum
	{
		MS_WIDE_GRAPHICS,
		MS_CHARACTER,
		MS_THIN_GRAPHICS,
		MS_UNDERLINE
	};

	crt9021_draw_character_delegate m_display_cb;

	bitmap_rgb32 m_bitmap;

	// inputs
	UINT8 m_data;
	int m_ms0;
	int m_ms1;
	int m_revid;
	int m_chabl;
	int m_blink;
	int m_intin;
	int m_atten;
	int m_cursor;
	int m_retbl;
	int m_ld_sh;
	int m_sld;
	int m_slg;
	int m_blc;
	int m_bkc;
	int m_sl0;
	int m_sl1;
	int m_sl2;
	int m_sl3;
	int m_vsync;

	// outputs
	UINT8 m_sr;
	int m_intout;
	int m_sl;
};


// device type definition
extern const device_type CRT9021;



#endif
