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

#ifndef MAME_SOUND_MOS7360_H
#define MAME_SOUND_MOS7360_H

#pragma once




//**************************************************************************
//  MACROS / CONSTANTS
//**************************************************************************

/* of course you clock select an other clock, but for accurate */
/* video timing (these are used in c16/c116/plus4) */
#define TED7360NTSC_CLOCK   (14318180/4)
#define TED7360PAL_CLOCK    (17734470/5)



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
	static constexpr unsigned NTSC_VRETRACERATE = 60;
	static constexpr unsigned PAL_VRETRACERATE = 50;
	static constexpr unsigned HRETRACERATE = 15625;

	/* the following values depend on the VIC clock,
	 * but to achieve TV-frequency the clock must have a fix frequency */
	static constexpr unsigned HSIZE   = 320;
	static constexpr unsigned VSIZE   = 200;

	/* pal 50 Hz vertical screen refresh, screen consists of 312 lines
	 * ntsc 60 Hz vertical screen refresh, screen consists of 262 lines */
	static constexpr unsigned NTSC_LINES = 261;
	static constexpr unsigned PAL_LINES = 312;

	// construction/destruction
	mos7360_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// callbacks
	auto write_irq_callback() { return m_write_irq.bind(); }
	auto read_k_callback() { return m_read_k.bind(); }

	virtual space_config_vector memory_space_config() const override;

	uint8_t read(offs_t offset, int &cs0, int &cs1);
	void write(offs_t offset, uint8_t data, int &cs0, int &cs1);

	uint32_t screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);

	void mos7360_videoram_map(address_map &map) ATTR_COLD;
protected:
	enum
	{
		TYPE_7360
	};

	enum
	{
		TIMER_ID_1,
		TIMER_ID_2,
		TIMER_ID_3
	};

	// device-level overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	// device_sound_interface callbacks
	virtual void sound_stream_update(sound_stream &stream, std::vector<read_stream_view> const &inputs, std::vector<write_stream_view> &outputs) override;

	inline void set_interrupt(int mask);
	inline void clear_interrupt(int mask);
	inline int rastercolumn();
	inline uint8_t read_ram(offs_t offset);
	inline uint8_t read_rom(offs_t offset);

	void draw_character(int ybegin, int yend, int ch, int yoff, int xoff, uint16_t *color);
	void draw_character_multi(int ybegin, int yend, int ch, int yoff, int xoff);
	void draw_bitmap(int ybegin, int yend, int ch, int yoff, int xoff);
	void draw_bitmap_multi(int ybegin, int yend, int ch, int yoff, int xoff);
	void draw_cursor(int ybegin, int yend, int yoff, int xoff, int color);
	void drawlines(int first, int last);
	void soundport_w(int offset, int data);
	TIMER_CALLBACK_MEMBER(timer_expired);
	TIMER_CALLBACK_MEMBER(frame_interrupt_gen);
	TIMER_CALLBACK_MEMBER(raster_interrupt_gen);
	int cs0_r(offs_t offset);
	int cs1_r(offs_t offset);

	const address_space_config      m_videoram_space_config;

	devcb_write_line   m_write_irq;
	devcb_read8        m_read_k;

	sound_stream *m_stream;

	uint8_t m_reg[0x20];
	uint8_t m_last_data;

	bitmap_rgb32 m_bitmap;

	int m_rom;

	int m_frame_count;

	int m_lines;
	bool m_timer_active[3];
	emu_timer *m_timer[3];
	int m_cursor1;

	int m_chargenaddr;
	int m_bitmapaddr;
	int m_videoaddr;

	int m_x_begin, m_x_end;
	int m_y_begin, m_y_end;

	uint16_t m_c16_bitmap[2];
	uint16_t m_bitmapmulti[4];
	uint16_t m_mono[2];
	uint16_t m_monoinversed[2];
	uint16_t m_multi[4];
	uint16_t m_ecmcolor[2];
	uint16_t m_colors[5];

	int m_rasterline, m_lastline;
	double m_rastertime;

	/* sound part */
	std::unique_ptr<uint8_t[]> m_noise;
	int m_tone1pos, m_tone2pos,
	m_tone1samples, m_tone2samples,
	m_noisesize,          /* number of samples */
	m_noisepos,         /* pos of tone */
	m_noisesamples;   /* count of samples to give out per tone */

	emu_timer *m_line_timer;
	emu_timer *m_frame_timer;
};


// device type definition
DECLARE_DEVICE_TYPE(MOS7360, mos7360_device)

#endif // MAME_SOUND_MOS7360_H
