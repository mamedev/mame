// license:BSD-3-Clause
// copyright-holders:Curt Coder
/***************************************************************************

    MOS 7360/8360 Text Edit Device (TED) emulation

****************************************************************************
                            _____   _____
                   DB6   1 |*    \_/     | 40  Vcc
                   DB5   2 |             | 39  DB7
                   DB4   3 |             | 38  DB8
                   DB3   4 |             | 37  DB9
                   DB2   5 |             | 36  DB10
                   DB1   6 |             | 35  DB11
                   DB0   7 |             | 34  A13
                  _IRQ   8 |             | 33  A12
                    LP   9 |             | 32  A11
                   _CS  10 |   MOS7360   | 31  A10
                   R/W  11 |             | 30  A9
                    BA  12 |             | 29  A8
                   Vdd  13 |             | 28  A7
                 COLOR  14 |             | 27  A6
                 S/LUM  15 |             | 26  A5
                   AEC  16 |             | 25  A4
                   PH0  17 |             | 24  A3
                  PHIN  18 |             | 23  A2
                 PHCOL  19 |             | 22  A1
                   Vss  20 |_____________| 21  A0

***************************************************************************/

#pragma once

#ifndef __MOS7360__
#define __MOS7360__

#include "emu.h"



/***************************************************************************
    DEVICE CONFIGURATION MACROS
***************************************************************************/

#define MCFG_MOS7360_ADD(_tag, _screen_tag, _cpu_tag, _clock, _videoram_map, _irq, _k) \
	MCFG_SCREEN_ADD(_screen_tag, RASTER) \
	MCFG_SCREEN_REFRESH_RATE(TED7360PAL_VRETRACERATE) \
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(2500)) \
	MCFG_SCREEN_SIZE(336, 216) \
	MCFG_SCREEN_VISIBLE_AREA(0, 336 - 1, 0, 216 - 1) \
	MCFG_SCREEN_UPDATE_DEVICE(_tag, mos7360_device, screen_update) \
	MCFG_DEVICE_ADD(_tag, MOS7360, _clock) \
	MCFG_DEVICE_ADDRESS_MAP(AS_0, _videoram_map) \
	MCFG_VIDEO_SET_SCREEN(_screen_tag) \
	downcast<mos7360_device *>(device)->set_callbacks(_cpu_tag, DEVCB_##_irq, DEVCB_##_k);



//**************************************************************************
//  MACROS / CONSTANTS
//**************************************************************************

#define TED7360NTSC_VRETRACERATE 60
#define TED7360PAL_VRETRACERATE 50
#define TED7360_HRETRACERATE 15625

/* the following values depend on the VIC clock,
 * but to achieve TV-frequency the clock must have a fix frequency */
#define TED7360_HSIZE   320
#define TED7360_VSIZE   200

/* of course you clock select an other clock, but for accurate */
/* video timing (these are used in c16/c116/plus4) */
#define TED7360NTSC_CLOCK   (14318180/4)
#define TED7360PAL_CLOCK    (17734470/5)

/* pal 50 Hz vertical screen refresh, screen consists of 312 lines
 * ntsc 60 Hz vertical screen refresh, screen consists of 262 lines */
#define TED7360NTSC_LINES 261
#define TED7360PAL_LINES 312



/***************************************************************************
    TYPE DEFINITIONS
***************************************************************************/

// ======================> mos7360_device

class mos7360_device :  public device_t,
						public device_memory_interface,
						public device_sound_interface,
						public device_video_interface
{
public:
	// construction/destruction
	//mos7360_device(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, UINT32 clock);
	mos7360_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	template<class _irq, class _k> void set_callbacks(const char *cpu_tag, _irq irq, _k k) {
		m_cpu_tag = cpu_tag;
		m_write_irq.set_callback(irq);
		m_read_k.set_callback(k);
	}

	virtual const address_space_config *memory_space_config(address_spacenum spacenum = AS_0) const override;

	UINT8 read(address_space &space, offs_t offset, int &cs0, int &cs1);
	void write(address_space &space, offs_t offset, UINT8 data, int &cs0, int &cs1);

	UINT32 screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);

protected:
	enum
	{
		TYPE_7360
	};

	enum
	{
		TIMER_ID_1,
		TIMER_ID_2,
		TIMER_ID_3,
		TIMER_LINE,
		TIMER_FRAME
	};

	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;
	virtual void device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr) override;

	// device_sound_interface callbacks
	virtual void sound_stream_update(sound_stream &stream, stream_sample_t **inputs, stream_sample_t **outputs, int samples) override;

	inline void set_interrupt(int mask);
	inline void clear_interrupt(int mask);
	inline int rastercolumn();
	inline UINT8 read_ram(offs_t offset);
	inline UINT8 read_rom(offs_t offset);

	void draw_character(int ybegin, int yend, int ch, int yoff, int xoff, UINT16 *color);
	void draw_character_multi(int ybegin, int yend, int ch, int yoff, int xoff);
	void draw_bitmap(int ybegin, int yend, int ch, int yoff, int xoff);
	void draw_bitmap_multi(int ybegin, int yend, int ch, int yoff, int xoff);
	void draw_cursor(int ybegin, int yend, int yoff, int xoff, int color);
	void drawlines(int first, int last);
	void soundport_w(int offset, int data);
	void frame_interrupt_gen();
	void raster_interrupt_gen();
	int cs0_r(offs_t offset);
	int cs1_r(offs_t offset);

	const address_space_config      m_videoram_space_config;

	devcb_write_line   m_write_irq;
	devcb_read8        m_read_k;

	const char *m_cpu_tag;
	cpu_device *m_cpu;
	sound_stream *m_stream;

	UINT8 m_reg[0x20];
	UINT8 m_last_data;

	bitmap_rgb32 m_bitmap;

	int m_rom;

	int m_frame_count;

	int m_lines;
	int m_timer1_active, m_timer2_active, m_timer3_active;
	emu_timer *m_timer1, *m_timer2, *m_timer3;
	int m_cursor1;

	int m_chargenaddr, m_bitmapaddr, m_videoaddr;

	int m_x_begin, m_x_end;
	int m_y_begin, m_y_end;

	UINT16 m_c16_bitmap[2], m_bitmapmulti[4], m_mono[2], m_monoinversed[2], m_multi[4], m_ecmcolor[2], m_colors[5];

	int m_rasterline, m_lastline;
	double m_rastertime;

	/* sound part */
	std::unique_ptr<UINT8[]> m_noise;
	int m_tone1pos, m_tone2pos,
	m_tone1samples, m_tone2samples,
	m_noisesize,          /* number of samples */
	m_noisepos,         /* pos of tone */
	m_noisesamples;   /* count of samples to give out per tone */

	emu_timer *m_line_timer;
	emu_timer *m_frame_timer;
};


// device type definition
extern const device_type MOS7360;



#endif
