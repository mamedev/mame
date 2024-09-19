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

#ifndef MAME_VIDBRAIN_UV201_H
#define MAME_VIDBRAIN_UV201_H

#pragma once


#include "screen.h"


//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> uv201_device

class uv201_device :    public device_t,
						public device_video_interface
{
public:
	// construction/destruction
	uv201_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	auto ext_int_wr_callback() { return m_write_ext_int.bind(); }
	auto hblank_wr_callback() { return m_write_hblank.bind(); }
	auto db_rd_callback() { return m_read_db.bind(); }

	uint8_t read(offs_t offset);
	void write(offs_t offset, uint8_t data);

	void ext_int_w(int state);
	int kbd_r();

	uint32_t screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	TIMER_CALLBACK_MEMBER(y_update_tick);
	TIMER_CALLBACK_MEMBER(hblank_on);
	TIMER_CALLBACK_MEMBER(hblank_off);

private:
	void initialize_palette();
	int get_field_vpos();
	int get_field();
	void set_y_interrupt();
	void do_partial_update();

	devcb_write_line   m_write_ext_int;
	devcb_write_line   m_write_hblank;
	devcb_read8        m_read_db;

	rgb_t m_palette_val[32];
	uint8_t m_ram[0x90];
	uint8_t m_y_int;
	uint8_t m_fmod;
	uint8_t m_bg;
	uint8_t m_cmd;
	uint8_t m_freeze_x;
	uint16_t m_freeze_y;
	int m_field;

	// timers
	emu_timer *m_timer_y_odd;
	emu_timer *m_timer_y_even;
	emu_timer *m_timer_hblank_on;
	emu_timer *m_timer_hblank_off;
};


// device type definition
DECLARE_DEVICE_TYPE(UV201, uv201_device)

#endif // MAME_VIDBRAIN_UV201_H
