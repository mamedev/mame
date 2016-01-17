// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    VideoBrain UV201/UV202 video chip emulation

**********************************************************************
                            _____   _____
                   GND   1 |*    \_/     | 40  DMAREQ
                     G   2 |             | 39  /CE
                     I   3 |             | 38  RESET
                     B   4 |             | 37  BISTROBE
                     R   5 |             | 36  UMIREQ0
                   BA0   6 |             | 35  BRCLK
                   BA1   7 |             | 34  HBLANK
                   BA2   8 |             | 33  VBLANK
                   BA3   9 |             | 32  R/W
                   BA4  10 |    UV201    | 31  keypad column 8
                   BA5  11 |             | 30  EXT INT
                   BA6  12 |             | 29  FIELD
                   BA7  13 |             | 28  BD7
                   BA8  14 |             | 27  BD6
                   BA9  15 |             | 26  BD5
                  BA10  16 |             | 25  BD4
                  BA11  17 |             | 24  BD3
                  BA12  18 |             | 23  BD2
                   +5V  19 |             | 22  BD1
                  +12V  20 |_____________| 21  BD0

                            _____   _____
               UMIREQ1   1 |*    \_/     | 40  BISTROBE
               UMIREQ0   2 |             | 39  CPUREQ1
               CPUREQ0   3 |             | 38  GND
                   XIN   4 |             | 37  /800-BFF
                  XOUT   5 |             | 36  RST
               DMAREQ0   6 |             | 35  DMAREQ1
               CPU CLK   7 |             | 34  COLCLK
                  WACK   8 |             | 33  BRCLK
                    D0   9 |             | 32  D7
                   BD0  10 |    UV202    | 31  BD7
                    D1  11 |             | 30  D6
                   BD1  12 |             | 29  BD6
                    D2  13 |             | 28  D5
                   BD2  14 |             | 27  BD5
                    D3  15 |             | 26  D4
                   BD3  16 |             | 25  BD4
                HBLANK  17 |             | 24  FIELD
                VBLANK  18 |             | 23  SCANLINE
                 BURST  19 |             | 22  +12V
                CSYNCH  20 |_____________| 21  +5V

**********************************************************************/

#pragma once

#ifndef __UV201__
#define __UV201__

#include "emu.h"



//**************************************************************************
//  INTERFACE CONFIGURATION MACROS
//**************************************************************************

#define MCFG_UV201_ADD(_tag, _screen_tag, _clock, _config) \
	MCFG_SCREEN_ADD(_screen_tag, RASTER) \
	MCFG_SCREEN_UPDATE_DEVICE(_tag, uv201_device, screen_update) \
	MCFG_SCREEN_RAW_PARAMS(_clock, 232, 18, 232, 262, 21, 262) \
	MCFG_DEVICE_ADD(_tag, UV201, _clock) \
	MCFG_VIDEO_SET_SCREEN(_screen_tag)


#define MCFG_UV201_EXT_INT_CALLBACK(_write) \
	devcb = &uv201_device::set_ext_int_wr_callback(*device, DEVCB_##_write);

#define MCFG_UV201_HBLANK_CALLBACK(_write) \
	devcb = &uv201_device::set_hblank_wr_callback(*device, DEVCB_##_write);

#define MCFG_UV201_DB_CALLBACK(_read) \
	devcb = &uv201_device::set_db_rd_callback(*device, DEVCB_##_read);



//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> uv201_device

class uv201_device :    public device_t,
						public device_video_interface
{
public:
	// construction/destruction
	uv201_device(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock);

	template<class _Object> static devcb_base &set_ext_int_wr_callback(device_t &device, _Object object) { return downcast<uv201_device &>(device).m_write_ext_int.set_callback(object); }
	template<class _Object> static devcb_base &set_hblank_wr_callback(device_t &device, _Object object) { return downcast<uv201_device &>(device).m_write_hblank.set_callback(object); }
	template<class _Object> static devcb_base &set_db_rd_callback(device_t &device, _Object object) { return downcast<uv201_device &>(device).m_read_db.set_callback(object); }

	DECLARE_READ8_MEMBER( read );
	DECLARE_WRITE8_MEMBER( write );

	DECLARE_WRITE_LINE_MEMBER( ext_int_w );
	DECLARE_READ_LINE_MEMBER( kbd_r );

	UINT32 screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);

protected:
	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;
	virtual void device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr) override;

private:
	enum
	{
		TIMER_Y_ODD,
		TIMER_Y_EVEN,
		TIMER_HBLANK_ON,
		TIMER_HBLANK_OFF
	};

	void initialize_palette();
	int get_field_vpos();
	int get_field();
	void set_y_interrupt();
	void do_partial_update();

	devcb_write_line   m_write_ext_int;
	devcb_write_line   m_write_hblank;
	devcb_read8        m_read_db;

	rgb_t m_palette_val[32];
	UINT8 m_ram[0x90];
	UINT8 m_y_int;
	UINT8 m_fmod;
	UINT8 m_bg;
	UINT8 m_cmd;
	UINT8 m_freeze_x;
	UINT16 m_freeze_y;
	int m_field;

	// timers
	emu_timer *m_timer_y_odd;
	emu_timer *m_timer_y_even;
	emu_timer *m_timer_hblank_on;
	emu_timer *m_timer_hblank_off;
};


// device type definition
extern const device_type UV201;



#endif
