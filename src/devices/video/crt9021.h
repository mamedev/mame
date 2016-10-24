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

#define CRT9021_DRAW_CHARACTER_MEMBER(_name) void _name(bitmap_rgb32 &bitmap, int y, int x, uint8_t video, int intout)


#define MCFG_CRT9021_DRAW_CHARACTER_CALLBACK_OWNER(_class, _method) \
	crt9021_t::static_set_display_callback(*device, crt9021_draw_character_delegate(&_class::_method, #_class "::" #_method, downcast<_class *>(owner)));


//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

typedef device_delegate<void (bitmap_rgb32 &bitmap, int y, int x, uint8_t video, int intout)> crt9021_draw_character_delegate;


// ======================> crt9021_t

class crt9021_t :  public device_t,
					public device_video_interface
{
public:
	// construction/destruction
	crt9021_t(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	static void static_set_display_callback(device_t &device, crt9021_draw_character_delegate callback) { downcast<crt9021_t &>(device).m_display_cb = callback; }

	void write(uint8_t data) { m_data = data; }
	void write(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff) { write(data); }
	void ms0_w(int state) { m_ms0 = state; }
	void ms1_w(int state) { m_ms1 = state; }
	void revid_w(int state) { m_revid = state; }
	void chabl_w(int state) { m_chabl = state; }
	void blink_w(int state) { m_blink = state; }
	void intin_w(int state) { m_intin = state; }
	void atten_w(int state) { m_atten = state; }
	void cursor_w(int state) { m_cursor = state; }
	void retbl_w(int state) { m_retbl = state; }
	void ld_sh_w(int state);
	void sld_w(int state) { m_sld = state; }
	void slg_w(int state) { m_slg = state; }
	void blc_w(int state) { m_blc = state; }
	void bkc_w(int state) { m_bkc = state; }
	void sl0_w(int state) { m_sl0 = state; }
	void sl1_w(int state) { m_sl1 = state; }
	void sl2_w(int state) { m_sl2 = state; }
	void sl3_w(int state) { m_sl3 = state; }
	void vsync_w(int state);

	uint32_t screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);

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
	uint8_t m_data;
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
	uint8_t m_sr;
	int m_intout;
	int m_sl;
};


// device type definition
extern const device_type CRT9021;



#endif
