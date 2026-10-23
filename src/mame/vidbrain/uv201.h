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
	void reset_w(int state);
	int bus_request(bool registers);
	int kbd_r();

	uint32_t screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);

protected:
	// device_t implementation
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

private:
	TIMER_CALLBACK_MEMBER(y_update_tick);
	TIMER_CALLBACK_MEMBER(hblank_on);
	TIMER_CALLBACK_MEMBER(hblank_off);

	void initialize_palette();
	int get_field_vpos();
	int get_field();
	void set_y_interrupt();
	void reset_state();

	int sprite_bank(int i) const;
	int sprite_y(int i) const;
	int sprite_width(int i) const;
	int next_sprite(int i) const;

	void fifo_clear();
	void fifo_push(uint16_t count, uint8_t data, uint8_t color);
	bool fifo_pop();

	void fetch_start();
	void fetch_next(int width);
	void fetch_end(int y);
	void fetch_clock(int y);
	uint8_t render_clock();
	void update();

	static constexpr int FIFO_DEPTH = 10;

	enum : uint8_t
	{
		FETCH_IDLE = 0,
		FETCH_DELAY,
		FETCH_DMA
	};

	devcb_write_line   m_write_ext_int;
	devcb_write_line   m_write_hblank;
	devcb_read8        m_read_db;

	bitmap_rgb32 m_bitmap[2];
	rgb_t m_palette_val[16];
	uint8_t m_ram[0x90];
	uint8_t m_y_int;
	uint8_t m_fmod;
	uint8_t m_bg;
	uint8_t m_cmd;
	uint8_t m_freeze_x;
	uint16_t m_freeze_y;
	uint8_t m_ext_int;

	// beam position
	int32_t m_vpos;
	int32_t m_hpos;
	uint64_t m_cycle;

	// sprite list state
	uint16_t m_in_range;
	uint16_t m_height;

	// fetcher
	uint8_t m_fetch_state;
	int32_t m_fetch_sprite;
	int32_t m_fetch_byte;
	uint16_t m_fetch_ptr;
	int32_t m_fetch_delay;
	int32_t m_fetch_total;
	uint64_t m_fetch_start;
	int32_t m_fetch_ws;
	int32_t m_fetch_pending;
	int32_t m_fetch_cpu_at;
	int32_t m_dma_count;
	uint64_t m_bus_free;
	int32_t m_xdelta;

	// FIFO
	uint16_t m_fifo_count[FIFO_DEPTH];
	uint8_t m_fifo_data[FIFO_DEPTH];
	uint8_t m_fifo_color[FIFO_DEPTH];
	int32_t m_fifo_head;
	int32_t m_fifo_level;
	bool m_fifo_full;

	// renderer
	int32_t m_render_left;
	int32_t m_render_pos;
	uint16_t m_render_count;
	uint8_t m_render_data;
	uint8_t m_render_color;

	// timers
	emu_timer *m_timer_y_odd;
	emu_timer *m_timer_y_even;
	emu_timer *m_timer_hblank_on;
	emu_timer *m_timer_hblank_off;
};


// device type definition
DECLARE_DEVICE_TYPE(UV201, uv201_device)

#endif // MAME_VIDBRAIN_UV201_H
