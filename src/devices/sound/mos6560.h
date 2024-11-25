// license:BSD-3-Clause
// copyright-holders:Peter Trauner
/***************************************************************************

    MOS 6560/6561 Video Interface Chip (VIC) emulation

****************************************************************************
                            _____   _____
                   N/C   1 |*    \_/     | 40  Vdd
                CHROMA   2 |             | 39  phi1
             LUMA/SYNC   3 |             | 38  phi2
                   R/W   4 |             | 37  OPTION
                   D11   5 |             | 36  Pphi2
                   D10   6 |             | 35  Pphi1
                    D9   7 |             | 34  A13
                    D8   8 |             | 33  A12
                    D7   9 |             | 32  A11
                    D6  10 |   MOS6560   | 31  A10
                    D5  11 |   MOS6561   | 30  A9
                    D4  12 |             | 29  A8
                    D3  13 |             | 28  A7
                    D2  14 |             | 27  A6
                    D1  15 |             | 26  A5
                    D0  16 |             | 25  A4
                 POT X  17 |             | 24  A3
                 POT Y  18 |             | 23  A2
                 AUDIO  19 |             | 22  A1
                   Vss  20 |_____________| 21  A0

***************************************************************************/

#ifndef MAME_SOUND_MOS6560_H
#define MAME_SOUND_MOS6560_H

#pragma once

#include "screen.h"


//**************************************************************************
//  MACROS / CONSTANTS
//**************************************************************************

#define MOS6560_VRETRACERATE 60
#define MOS6561_VRETRACERATE 50

#define MOS6560_MAME_XPOS  4           /* xleft not displayed */
#define MOS6560_MAME_YPOS  10          /* y up not displayed */
#define MOS6561_MAME_XPOS  20
#define MOS6561_MAME_YPOS  10
#define MOS6560_MAME_XSIZE  200
#define MOS6560_MAME_YSIZE  248
#define MOS6561_MAME_XSIZE  224
#define MOS6561_MAME_YSIZE  296
/* real values */

#define MOS6560_LINES 261
#define MOS6561_LINES 312

#define MOS6560_XSIZE   (4+201)        /* 4 left not visible */
#define MOS6560_YSIZE   (10+251)       /* 10 not visible */
/* cycles 65 */

#define MOS6561_XSIZE   (20+229)       /* 20 left not visible */
#define MOS6561_YSIZE   (10+302)       /* 10 not visible */
/* cycles 71 */


/* the following values depend on the VIC clock,
 * but to achieve TV-frequency the clock must have a fix frequency */
#define MOS6560_CLOCK   (14318181/14)
#define MOS6561_CLOCK   (4433618/4)



//***************************************************************************
//  TYPE DEFINITIONS
//***************************************************************************

// ======================> mos6560_device

class mos6560_device : public device_t,
						public device_memory_interface,
						public device_sound_interface,
						public device_video_interface
{
public:
	mos6560_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	auto potx_rd_callback() { return m_read_potx.bind(); }
	auto poty_rd_callback() { return m_read_poty.bind(); }

	virtual space_config_vector memory_space_config() const override;

	uint8_t read(offs_t offset);
	void write(offs_t offset, uint8_t data);

	uint8_t bus_r();

	void lp_w(int state);

	uint32_t screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);

	void mos6560_colorram_map(address_map &map) ATTR_COLD;
	void mos6560_videoram_map(address_map &map) ATTR_COLD;
protected:
	enum
	{
		TYPE_6560,          // NTSC-M
		TYPE_6561,          // PAL-B
		TYPE_ATTACK_UFO     // NTSC-M, less features
	};

	mos6560_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock, uint32_t variant);

	// device-level overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	// sound stream update overrides
	virtual void sound_stream_update(sound_stream &stream, std::vector<read_stream_view> const &inputs, std::vector<write_stream_view> &outputs) override;

	inline uint8_t read_videoram(offs_t offset);
	inline uint8_t read_colorram(offs_t offset);

	void draw_character( int ybegin, int yend, int ch, int yoff, int xoff, uint16_t *color );
	void draw_character_multi( int ybegin, int yend, int ch, int yoff, int xoff, uint16_t *color );
	void drawlines( int first, int last );
	void soundport_w( int offset, int data );
	void sound_start();
	TIMER_CALLBACK_MEMBER(raster_interrupt_gen);

	const int  m_variant;

	const address_space_config      m_videoram_space_config;
	const address_space_config      m_colorram_space_config;

	devcb_read8    m_read_potx;
	devcb_read8    m_read_poty;

	uint8_t m_reg[16];

	bitmap_rgb32 m_bitmap;

	int m_rasterline, m_lastline;
	double m_lightpenreadtime;

	int m_charheight, m_matrix8x16, m_inverted;
	int m_chars_x, m_chars_y;
	int m_xsize, m_ysize, m_xpos, m_ypos;
	int m_chargenaddr, m_videoaddr;

	/* values in videoformat */
	uint16_t m_backgroundcolor, m_framecolor, m_helpercolor;

	/* arrays for bit to color conversion without condition checking */
	uint16_t m_mono[2], m_monoinverted[2], m_multi[4], m_multiinverted[4];

	/* video chip settings */
	int m_total_xsize, m_total_ysize, m_total_lines, m_total_vretracerate;

	/* DMA */
	uint8_t m_last_data;

	/* sound part */
	int m_tone1pos, m_tone2pos, m_tone3pos,
	m_tonesize, m_tone1samples, m_tone2samples, m_tone3samples,
	m_noisesize,          /* number of samples */
	m_noisepos,         /* pos of tone */
	m_noisesamples;   /* count of samples to give out per tone */

	sound_stream *m_channel;
	std::unique_ptr<int16_t []> m_tone;
	std::unique_ptr<int8_t []> m_noise;

	emu_timer *m_line_timer;
};


// ======================> mos6561_device

class mos6561_device : public mos6560_device
{
public:
	// construction/destruction
	mos6561_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
};


// ======================> mos656x_attack_ufo_device

class mos656x_attack_ufo_device : public mos6560_device
{
public:
	// construction/destruction
	mos656x_attack_ufo_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
};


// device type definitions
DECLARE_DEVICE_TYPE(MOS6560,            mos6560_device)
DECLARE_DEVICE_TYPE(MOS6561,            mos6561_device)
DECLARE_DEVICE_TYPE(MOS656X_ATTACK_UFO, mos656x_attack_ufo_device)

#endif // MAME_SOUND_MOS6560_H
