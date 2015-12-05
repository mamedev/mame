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

#pragma once

#ifndef __MOS6560__
#define __MOS6560__

#include "emu.h"



//***************************************************************************
// DEVICE CONFIGURATION MACROS
//***************************************************************************

#define MCFG_MOS6560_ADD(_tag, _screen_tag, _clock, _videoram_map, _colorram_map) \
	MCFG_SCREEN_ADD(_screen_tag, RASTER) \
	MCFG_SCREEN_REFRESH_RATE(MOS6560_VRETRACERATE) \
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(2500)) \
	MCFG_SCREEN_SIZE((MOS6560_XSIZE + 7) & ~7, MOS6560_YSIZE) \
	MCFG_SCREEN_VISIBLE_AREA(MOS6560_MAME_XPOS, MOS6560_MAME_XPOS + MOS6560_MAME_XSIZE - 1, MOS6560_MAME_YPOS, MOS6560_MAME_YPOS + MOS6560_MAME_YSIZE - 1) \
	MCFG_SCREEN_UPDATE_DEVICE(_tag, mos6560_device, screen_update) \
	MCFG_SOUND_ADD(_tag, MOS6560, _clock) \
	MCFG_VIDEO_SET_SCREEN(_screen_tag) \
	MCFG_DEVICE_ADDRESS_MAP(AS_0, _videoram_map) \
	MCFG_DEVICE_ADDRESS_MAP(AS_1, _colorram_map)

#define MCFG_MOS6561_ADD(_tag, _screen_tag, _clock, _videoram_map, _colorram_map) \
	MCFG_SCREEN_ADD(_screen_tag, RASTER) \
	MCFG_SCREEN_REFRESH_RATE(MOS6561_VRETRACERATE) \
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(2500)) \
	MCFG_SCREEN_SIZE((MOS6561_XSIZE + 7) & ~7, MOS6561_YSIZE) \
	MCFG_SCREEN_VISIBLE_AREA(MOS6561_MAME_XPOS, MOS6561_MAME_XPOS + MOS6561_MAME_XSIZE - 1, MOS6561_MAME_YPOS, MOS6561_MAME_YPOS + MOS6561_MAME_YSIZE - 1) \
	MCFG_SCREEN_UPDATE_DEVICE(_tag, mos6560_device, screen_update) \
	MCFG_SOUND_ADD(_tag, MOS6561, _clock) \
	MCFG_VIDEO_SET_SCREEN(_screen_tag) \
	MCFG_DEVICE_ADDRESS_MAP(AS_0, _videoram_map) \
	MCFG_DEVICE_ADDRESS_MAP(AS_1, _colorram_map)

#define MCFG_MOS656X_ATTACK_UFO_ADD(_tag, _screen_tag, _clock, _videoram_map, _colorram_map) \
	MCFG_SCREEN_ADD(_screen_tag, RASTER) \
	MCFG_SCREEN_REFRESH_RATE(MOS6560_VRETRACERATE) \
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(2500)) \
	MCFG_SCREEN_SIZE((MOS6560_XSIZE + 7) & ~7, MOS6560_YSIZE) \
	MCFG_SCREEN_VISIBLE_AREA(0, 23*8 - 1, 0, 22*8 - 1) \
	MCFG_SCREEN_UPDATE_DEVICE(_tag, mos6560_device, screen_update) \
	MCFG_SOUND_ADD(_tag, MOS656X_ATTACK_UFO, _clock) \
	MCFG_VIDEO_SET_SCREEN(_screen_tag) \
	MCFG_DEVICE_ADDRESS_MAP(AS_0, _videoram_map) \
	MCFG_DEVICE_ADDRESS_MAP(AS_1, _colorram_map)


#define MCFG_MOS6560_POTX_CALLBACK(_read) \
	devcb = &mos6560_device::set_potx_rd_callback(*device, DEVCB_##_read);

#define MCFG_MOS6560_POTY_CALLBACK(_read) \
	devcb = &mos6560_device::set_poty_rd_callback(*device, DEVCB_##_read);



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
	mos6560_device(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, UINT32 clock, UINT32 variant, const char *shortname, const char *source);
	mos6560_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	template<class _Object> static devcb_base &set_potx_rd_callback(device_t &device, _Object object) { return downcast<mos6560_device &>(device).m_read_potx.set_callback(object); }
	template<class _Object> static devcb_base &set_poty_rd_callback(device_t &device, _Object object) { return downcast<mos6560_device &>(device).m_read_poty.set_callback(object); }

	virtual const address_space_config *memory_space_config(address_spacenum spacenum = AS_0) const override;

	DECLARE_READ8_MEMBER( read );
	DECLARE_WRITE8_MEMBER( write );

	UINT8 bus_r();

	DECLARE_WRITE_LINE_MEMBER( lp_w );

	UINT32 screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);

protected:
	enum
	{
		TYPE_6560,          // NTSC-M
		TYPE_6561,          // PAL-B
		TYPE_ATTACK_UFO     // NTSC-M, less features
	};

	enum
	{
		TIMER_LINE
	};

	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;
	virtual void device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr) override;

	// sound stream update overrides
	virtual void sound_stream_update(sound_stream &stream, stream_sample_t **inputs, stream_sample_t **outputs, int samples) override;

	inline UINT8 read_videoram(offs_t offset);
	inline UINT8 read_colorram(offs_t offset);

	void draw_character( int ybegin, int yend, int ch, int yoff, int xoff, UINT16 *color );
	void draw_character_multi( int ybegin, int yend, int ch, int yoff, int xoff, UINT16 *color );
	void drawlines( int first, int last );
	void soundport_w( int offset, int data );
	void sound_start();
	void raster_interrupt_gen();

	int  m_variant;

	const address_space_config      m_videoram_space_config;
	const address_space_config      m_colorram_space_config;

	devcb_read8    m_read_potx;
	devcb_read8    m_read_poty;

	UINT8 m_reg[16];

	bitmap_rgb32 m_bitmap;

	int m_rasterline, m_lastline;
	double m_lightpenreadtime;

	int m_charheight, m_matrix8x16, m_inverted;
	int m_chars_x, m_chars_y;
	int m_xsize, m_ysize, m_xpos, m_ypos;
	int m_chargenaddr, m_videoaddr;

	/* values in videoformat */
	UINT16 m_backgroundcolor, m_framecolor, m_helpercolor;

	/* arrays for bit to color conversion without condition checking */
	UINT16 m_mono[2], m_monoinverted[2], m_multi[4], m_multiinverted[4];

	/* video chip settings */
	int m_total_xsize, m_total_ysize, m_total_lines, m_total_vretracerate;

	/* DMA */
	UINT8 m_last_data;

	/* sound part */
	int m_tone1pos, m_tone2pos, m_tone3pos,
	m_tonesize, m_tone1samples, m_tone2samples, m_tone3samples,
	m_noisesize,          /* number of samples */
	m_noisepos,         /* pos of tone */
	m_noisesamples;   /* count of samples to give out per tone */

	sound_stream *m_channel;
	INT16 *m_tone;
	INT8 *m_noise;

	emu_timer *m_line_timer;
};


// ======================> mos6561_device

class mos6561_device :  public mos6560_device
{
public:
	// construction/destruction
	mos6561_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
};


// ======================> mos656x_attack_ufo_device

class mos656x_attack_ufo_device :  public mos6560_device
{
public:
	// construction/destruction
	mos656x_attack_ufo_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
};


// device type definitions
extern const device_type MOS6560;
extern const device_type MOS6561;
extern const device_type MOS656X_ATTACK_UFO;



#endif
