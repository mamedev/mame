// license:BSD-3-Clause
// copyright-holders:David Haywood

/*******************************************************************************
 Shadow Force (c)1993 Technos
 Preliminary Driver by David Haywood
 Based on the Various Other Technos Games
********************************************************************************

Stephh's notes :

  - As for some other M68000 Technos games (or games running on similar hardware
    such as 'mugsmash'), the Inputs and the Dip Switches are mangled, so you need
    a specific read handler so end-users can see them in a "standard" order.

 01-Sept-2008 - Pierpaolo Prazzoli
 - Added irqs ack
 - Implemented raster irq
 - Fixed coin2 and service input not working during the game
 - Added watchdog
 - Fixed visible area
 - Added video enable and irqs enable flags


*******************************************************************************

Shadow Force
Technos 1993

PCB Layout
----------

TA-0032-P1-24
MADE IN JAPAN
|--------------------------------------------------------|
|LA4460 MB3615 YM3012  3.579545MHz 32J8-0.32             |
|VOL    MB3615 YM2151  Z80            32J7-0.25          |
|              M6295                     32J6-0.24       |
|   32J9-0.76         32J10.42              32J5-0.13    |
|                     6116                     32J4-0.12 |
|                                           |-------|    |
|                                           |TECHNOS|    |
|J                            62256         |TJ-005 |    |
|A          62256             13.4952MHz    |       |    |
|M          62256             28MHz         |-------|    |
|M          32J11-0.55                       6116        |
|A                 |-------|    |-------|    6116        |
|                  |TECHNOS|    |TECHNOS|                |
|                  |TJ-004 |    |TJ-002 |   32J3-0.6     |
|DSW3              |       |    |       |   32J2-0.5     |
|DSW2              |-------|    |-------|   32J1-0.4     |
|DSW1           32A12-0.34                               |
|CN2                32A14-0.33                           |
|                62256  32A13-0.26     62256   A1010     |
|        68000   62256      32J15-0.14 62256             |
|--------------------------------------------------------|
Notes:
      68000    - Clock 14.000MHz [28/2]
      Z80      - Clock 3.579545MHz
      YM2151   - Clock 3.579545MHz
      YM3012   - Clock 1.7897725MHz [3.579545/2]
      M6295    - Clock 1.6869MHz [13.4952/8]. Pin 7 HIGH (5.0V)
      A1010    - Actel A1010A-1 FPGA labelled 'TJ32A 92.11.30' (PLCC68)
      LA4460   - Audio Power Amp
      CN2      - 9 pin connector for extra buttons
      MB3615   - Fujitsu MB3615 Quad Operational Amplifier (like TL074 except pin 11 tied to GND, not -5V)
      6116     - 2k x8 SRAM (DIP24)
      62256    - 32k x8 SRAM (DIP28)
      DSW1/2/3 - 8-position dip switches

      ROMs
      ----
      32J1 to 32J8    - 16M mask ROM (DIP42)
      32A12/13/14/J15 - 27C2001 EPROM (DIP32)
      32J9            - 4M mask ROM (DIP32)
      32J10           - 27C512 EPROM (DIP28)
      32J11           - 27C010 EPROM (DIP32)

      Measurements
      ------------
      X1     - 27.99987MHz
      X2     - 3.57899MHz
      X4     - 13.4894MHz
      VSync  - 57.4446Hz
      HSync  - 15.6248kHz

-- Read Me --

Shadow Force (c)1993 Technos
TA-0032-P1-23 (Japan version is TA-0032-P1-24)

CPU: TMP68HC000P-16
Sound: Z80, YM2151, YM3012, OKI6295, MB3615 (x2)
Custom: Actel A1010A-1 (TJ32A 92.11.30), TJ-002, TJ-004, TJ-005

X1: 28 MHz
X2: 3.579545 MHz
X4: 13.4952 MHz

ROMs:

Program:
32A12-01.34 = 27C2001
32A14-0.33  = 27C2001
32A13-01.26 = 27C2001
32A15-0.14  = 27C2001

Char:
32A11-0.55 = 27C010

Backgrounds:
32J1-0.4 = 8meg mask
32J2-0.5 = 8meg mask
32J3-0.6 = 8meg mask

Sprites:
32J4-0.12 = 16meg mask
32J5-0.13 = 16meg mask
32J6-0.24 = 16meg mask
32J7-0.25 = 16meg mask
32J8-0.32 = 16meg mask

Sound CPU:
32J10.42 = 27C512

Samples:
32J9-0.76 = 27C040

*******************************************************************************/


/*
68k interrupts
lev 1 : 0x64 : 0004 fd00 - raster irq
lev 2 : 0x68 : 0000 1f32 - inputs irq (reads inputs, ...)
lev 3 : 0x6c : 0000 10f4 - vblank irq
lev 4 : 0x70 : 0000 11d0 - just rte
lev 5 : 0x74 : 0000 11d0 - just rte
lev 6 : 0x78 : 0000 11d0 - just rte
lev 7 : 0x7c : 0000 11d0 - just rte
*/

#include "emu.h"

#include "cpu/m68000/m68000.h"
#include "cpu/z80/z80.h"
#include "machine/gen_latch.h"
#include "machine/timer.h"
#include "machine/watchdog.h"
#include "sound/okim6295.h"
#include "sound/ymopm.h"
#include "video/bufsprite.h"

#include "emupal.h"
#include "screen.h"
#include "speaker.h"
#include "tilemap.h"


namespace {

class shadfrce_state : public driver_device
{
public:
	shadfrce_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_audiocpu(*this, "audiocpu"),
		m_oki(*this, "oki"),
		m_gfxdecode(*this, "gfxdecode"),
		m_screen(*this, "screen"),
		m_palette(*this, "palette"),
		m_soundlatch(*this, "soundlatch"),
		m_io_p(*this, "P%u", 1U),
		m_io_dsw(*this, "DSW%u", 1U),
		m_io_other(*this, "OTHER"),
		m_io_extra(*this, "EXTRA"),
		m_io_misc(*this, "MISC"),
		m_io_system(*this, "SYSTEM"),
		m_fgvideoram(*this, "fgvideoram"),
		m_bgvideoram(*this, "bgvideoram%u", 0U),
		m_spvideoram(*this, "spvideoram")
	{ }

	void shadfrce(machine_config &config);

protected:
	virtual void video_start() override ATTR_COLD;

private:
	required_device<cpu_device> m_maincpu;
	required_device<cpu_device> m_audiocpu;
	required_device<okim6295_device> m_oki;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<screen_device> m_screen;
	required_device<palette_device> m_palette;
	required_device<generic_latch_8_device> m_soundlatch;

	required_ioport_array<2> m_io_p;
	required_ioport_array<2> m_io_dsw;
	required_ioport m_io_other;
	required_ioport m_io_extra;
	required_ioport m_io_misc;
	required_ioport m_io_system;

	required_shared_ptr<uint16_t> m_fgvideoram;
	required_shared_ptr_array<uint16_t, 2> m_bgvideoram;
	required_device<buffered_spriteram16_device> m_spvideoram;

	tilemap_t *m_fgtilemap = nullptr;
	tilemap_t *m_bgtilemap[2]{};
	uint8_t m_video_enable = 0U;
	uint8_t m_irqs_enable = 0U;
	uint16_t m_raster_scanline = 0U;
	uint8_t m_raster_irq_enable = 0U;
	uint8_t m_vblank = 0U;
	uint16_t m_prev_value = 0U;

	void flip_screen(uint16_t data);
	uint16_t input_ports_r(offs_t offset);
	void screen_brt_w(uint8_t data);
	void irq_ack_w(offs_t offset, uint16_t data);
	void irq_w(uint16_t data);
	void scanline_w(uint16_t data);
	void fgvideoram_w(offs_t offset, uint16_t data);
	template <uint8_t Which> void bgvideoram_w(offs_t offset, uint16_t data);
	template <uint8_t Which> void bgscrollx_w(uint16_t data);
	template <uint8_t Which> void bgscrolly_w(uint16_t data);
	void oki_bankswitch_w(uint8_t data);

	TILE_GET_INFO_MEMBER(get_fgtile_info);
	TILE_GET_INFO_MEMBER(get_bg0tile_info);
	TILE_GET_INFO_MEMBER(get_bg1tile_info);

	TIMER_DEVICE_CALLBACK_MEMBER(scanline);

	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void draw_sprites(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

	void main_map(address_map &map) ATTR_COLD;
	void sound_map(address_map &map) ATTR_COLD;
};


TILE_GET_INFO_MEMBER(shadfrce_state::get_fgtile_info)
{
	// ---- ----  tttt tttt  ---- ----  pppp TTTT
	int tileno = (m_fgvideoram[tile_index *2] & 0x00ff) | ((m_fgvideoram[tile_index * 2 + 1] & 0x000f) << 8);
	int colour = (m_fgvideoram[tile_index *2 + 1] & 0x00f0) >> 4;

	tileinfo.set(0, tileno, colour * 4, 0);
}

void shadfrce_state::fgvideoram_w(offs_t offset, uint16_t data)
{
	m_fgvideoram[offset] = data;
	m_fgtilemap->mark_tile_dirty(offset / 2);
}

TILE_GET_INFO_MEMBER(shadfrce_state::get_bg0tile_info)
{
	// ---- ----  ---- cccc  --TT TTTT TTTT TTTT
	int tileno = (m_bgvideoram[0][tile_index * 2 + 1] & 0x3fff);
	int colour = m_bgvideoram[0][tile_index * 2] & 0x001f;
	if (colour & 0x10) colour ^= 0x30;  // skip hole
	int fyx = (m_bgvideoram[0][tile_index * 2] & 0x00c0) >> 6;

	tileinfo.set(2, tileno, colour, TILE_FLIPYX(fyx));
}

template <uint8_t Which>
void shadfrce_state::bgvideoram_w(offs_t offset, uint16_t data)
{
	m_bgvideoram[Which][offset] = data;
	m_bgtilemap[Which]->mark_tile_dirty(Which ? offset : offset / 2);
}

TILE_GET_INFO_MEMBER(shadfrce_state::get_bg1tile_info)
{
	int tileno = (m_bgvideoram[1][tile_index] & 0x0fff);
	int colour = (m_bgvideoram[1][tile_index] & 0xf000) >> 12;

	tileinfo.set(2, tileno, colour + 64, 0);
}

void shadfrce_state::video_start()
{
	m_fgtilemap  = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(shadfrce_state::get_fgtile_info)),  TILEMAP_SCAN_ROWS,  8,  8, 64, 32);
	m_fgtilemap->set_transparent_pen(0);

	m_bgtilemap[0] = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(shadfrce_state::get_bg0tile_info)), TILEMAP_SCAN_ROWS, 16, 16, 32, 32);
	m_bgtilemap[0]->set_transparent_pen(0);

	m_bgtilemap[1] = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(shadfrce_state::get_bg1tile_info)), TILEMAP_SCAN_ROWS, 16, 16, 32, 32);

	save_item(NAME(m_video_enable));
	save_item(NAME(m_irqs_enable));
	save_item(NAME(m_raster_scanline));
	save_item(NAME(m_raster_irq_enable));
	save_item(NAME(m_vblank));
	save_item(NAME(m_prev_value));
}

template <uint8_t Which>
void shadfrce_state::bgscrollx_w(uint16_t data)
{
	m_bgtilemap[Which]->set_scrollx(0, data & 0x1ff);
}

template <uint8_t Which>
void shadfrce_state::bgscrolly_w(uint16_t data)
{
	m_bgtilemap[Which]->set_scrolly(0, data  & 0x1ff);
}


void shadfrce_state::draw_sprites(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	/* | ---- ---- hhhf Fe-Y | ---- ---- yyyy yyyy | ---- ---- TTTT TTTT | ---- ---- tttt tttt |
	   | ---- ---- -pCc cccX | ---- ---- xxxx xxxx | ---- ---- ---- ---- | ---- ---- ---- ---- | */

	/* h  = height
	   f  = flipx
	   F  = flipy
	   e  = enable
	   Yy = Y Position
	   Tt = Tile No.
	   Xx = X Position
	   Cc = color
	   P = priority
	*/

	gfx_element *gfx = m_gfxdecode->gfx(1);
	uint16_t *finish = m_spvideoram->buffer();
	uint16_t *source = finish + 0x2000 / 2 - 8;
	while (source >= finish)
	{
		int const ypos = 0x100 - (((source[0] & 0x0003) << 8) | (source[1] & 0x00ff));
		int const xpos = (((source[4] & 0x0001) << 8) | (source[5] & 0x00ff)) + 1;
		int const tile = ((source[2] & 0x00ff) << 8) | (source[3] & 0x00ff);
		int height = (source[0] & 0x00e0) >> 5;
		int const enable = ((source[0] & 0x0004));
		int const flipx = ((source[0] & 0x0010) >> 4);
		int const flipy = ((source[0] & 0x0008) >> 3);
		int pal = ((source[4] & 0x003e));
		int const pri_mask = (source[4] & 0x0040) ? 0x02 : 0x00;

		if (pal & 0x20) pal ^= 0x60;    // skip hole

		height++;
		if (enable)
		{
			for (int hcount = 0; hcount < height; hcount++)
			{
				gfx->prio_transpen(bitmap, cliprect, tile + hcount, pal, flipx, flipy, xpos, ypos - hcount * 16 - 16, screen.priority(), pri_mask, 0);
				gfx->prio_transpen(bitmap, cliprect, tile + hcount, pal, flipx, flipy, xpos - 0x200, ypos - hcount * 16 - 16, screen.priority(), pri_mask, 0);
				gfx->prio_transpen(bitmap, cliprect, tile + hcount, pal, flipx, flipy, xpos, ypos - hcount * 16 - 16 + 0x200, screen.priority(), pri_mask, 0);
				gfx->prio_transpen(bitmap, cliprect, tile + hcount, pal, flipx, flipy, xpos - 0x200, ypos - hcount * 16 - 16 + 0x200, screen.priority(), pri_mask, 0);
			}
		}
		source -= 8;
	}
}

uint32_t shadfrce_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	screen.priority().fill(0, cliprect);

	if (m_video_enable)
	{
		m_bgtilemap[1]->draw(screen, bitmap, cliprect, 0, 0);
		m_bgtilemap[0]->draw(screen, bitmap, cliprect, 0, 1);
		draw_sprites(screen, bitmap, cliprect);
		m_fgtilemap->draw(screen, bitmap, cliprect, 0, 0);
	}
	else
	{
		bitmap.fill(m_palette->black_pen(), cliprect);
	}

	return 0;
}


// machine/gen_latch

void shadfrce_state::flip_screen(uint16_t data)
{
	flip_screen_set(data & 0x01);
}


/* Ports mapping :

    $1d0020.w : 0123456789ABCDEF
                x---------------    right     (player 1)
                -x--------------    left      (player 1)
                --x-------------    up        (player 1)
                ---x------------    down      (player 1)
                ----x-----------    button 1  (player 1)
                -----x----------    button 2  (player 1)
                ------x---------    button 3  (player 1)
                -------x--------    start     (player 1)
                --------x-------    coin 1
                ---------x------    coin 2                       *
                ----------x-----    service 1                    *
                -----------x----    unused
                ------------x---    DIP2-7
                -------------x--    DIP2-8
                --------------x-    unused
                ---------------x    unused

    $1d0022.w : 0123456789ABCDEF
                x---------------    right     (player 2)
                -x--------------    left      (player 2)
                --x-------------    up        (player 2)
                ---x------------    down      (player 2)
                ----x-----------    button 1  (player 2)
                -----x----------    button 2  (player 2)
                ------x---------    button 3  (player 2)
                -------x--------    start     (player 2)
                --------x-------    DIP2-1    ("Difficulty")
                ---------x------    DIP2-2    ("Difficulty")
                ----------x-----    DIP2-3    ("Stage Clear Energy Regain")
                -----------x----    DIP2-4    ("Stage Clear Energy Regain")
                ------------x---    DIP2-5
                -------------x--    DIP2-6
                --------------x-    unused
                ---------------x    unused

    $1d0024.w : 0123456789ABCDEF
                x---------------    button 4  (player 1)
                -x--------------    button 5  (player 1)
                --x-------------    button 6  (player 1)
                ---x------------    button 4  (player 2)
                ----x-----------    button 5  (player 2)
                -----x----------    button 6  (player 2)
                ------x---------    unused
                -------x--------    unused
                --------x-------    DIP1-1
                ---------x------    DIP1-2    ("Coin(s) for Credit(s)")
                ----------x-----    DIP1-3    ("Coin(s) for Credit(s)")
                -----------x----    DIP1-4    (DEF_STR( Continue_Price ))
                ------------x---    DIP1-5    ("Free Play")
                -------------x--    DIP1-6    ("Flip Screen")
                --------------x-    unused
                ---------------x    unused

    $1d0026.w : 0123456789ABCDEF
                x---------------    unused
                -x--------------    unused
                --x-------------    unused
                ---x------------    unused
                ----x-----------    unused
                -----x----------    unused
                ------x---------    unused
                -------x--------    unused
                --------x-------    DIP 1-7   ("Demo Sound")
                ---------x------    DIP 1-8   ("Test Mode")
                ----------x-----    vblank?
                -----------x----    unused
                ------------x---    unknown
                -------------x--    unused
                --------------x-    unused
                ---------------x    unused

    * only available when you are in "test mode"

*/


uint16_t shadfrce_state::input_ports_r(offs_t offset)
{
	uint16_t data = 0xffff;

	switch (offset)
	{
		case 0 :
			data = (m_io_p[0]->read() & 0xff) | ((m_io_dsw[1]->read() & 0xc0) << 6) | ((m_io_system->read() & 0x0f) << 8);
			break;
		case 1 :
			data = (m_io_p[1]->read() & 0xff) | ((m_io_dsw[1]->read() & 0x3f) << 8);
			break;
		case 2 :
			data = (m_io_extra->read() & 0xff) | ((m_io_dsw[0]->read() & 0x3f) << 8);
			break;
		case 3 :
			data = (m_io_other->read() & 0xff) | ((m_io_dsw[0]->read() & 0xc0) << 2) | ((m_io_misc->read() & 0x38) << 8) | (m_vblank << 8);
			break;
	}

	return (data);
}


void shadfrce_state::screen_brt_w(uint8_t data)
{
	double const brt = (data & 0xff) / 255.0;

	for (int i = 0; i < 0x4000; i++)
		m_palette->set_pen_contrast(i, brt);
}

void shadfrce_state::irq_ack_w(offs_t offset, uint16_t data)
{
	m_maincpu->set_input_line(offset ^ 3, CLEAR_LINE);
}

void shadfrce_state::irq_w(uint16_t data)
{
	m_irqs_enable = data & 1;   // maybe, it's set/unset inside every trap instruction which is executed
	m_video_enable = data & 8;  // probably

	// check if there's a high transition to enable the raster IRQ
	if ((~m_prev_value & 4) && (data & 4))
	{
		m_raster_irq_enable = 1;
	}

	// check if there's a low transition to disable the raster IRQ
	if ((m_prev_value & 4) && (~data & 4))
	{
		m_raster_irq_enable = 0;
	}

	m_prev_value = data;
}

void shadfrce_state::scanline_w(uint16_t data)
{
	m_raster_scanline = data;   // guess, 0 is always written
}

TIMER_DEVICE_CALLBACK_MEMBER(shadfrce_state::scanline)
{
	int scanline = param;

	// Vblank is lowered on scanline 0
	if (scanline == 0)
	{
		m_vblank = 0;
	}
	// Hack
	else if (scanline == (248 - 1))       // -1 is an hack needed to avoid deadlocks
	{
		m_vblank = 4;
	}

	// Raster interrupt - Perform raster effect on given scanline
	if (m_raster_irq_enable)
	{
		if (scanline == m_raster_scanline)
		{
			m_raster_scanline = (m_raster_scanline + 1) % 240;
			if (m_raster_scanline > 0)
				m_screen->update_partial(m_raster_scanline - 1);
			m_maincpu->set_input_line(1, ASSERT_LINE);
		}
	}

	// An interrupt is generated every 16 scanlines
	if (m_irqs_enable)
	{
		if (scanline % 16 == 0)
		{
			if (scanline > 0)
				m_screen->update_partial(scanline - 1);
			m_maincpu->set_input_line(2, ASSERT_LINE);
		}
	}

	// Vblank is raised on scanline 248
	if (m_irqs_enable)
	{
		if (scanline == 248)
		{
			m_screen->update_partial(scanline - 1);
			m_maincpu->set_input_line(3, ASSERT_LINE);
		}
	}
}



// Memory Maps

void shadfrce_state::main_map(address_map &map)
{
	map(0x000000, 0x0fffff).rom();
	map(0x100000, 0x100fff).ram().w(FUNC(shadfrce_state::bgvideoram_w<0>)).share(m_bgvideoram[0]);
	map(0x101000, 0x101fff).ram();
	map(0x102000, 0x1027ff).ram().w(FUNC(shadfrce_state::bgvideoram_w<1>)).share(m_bgvideoram[1]);
	map(0x102800, 0x103fff).ram();
	map(0x140000, 0x141fff).ram().w(FUNC(shadfrce_state::fgvideoram_w)).share(m_fgvideoram);
	map(0x142000, 0x143fff).ram().share("spvideoram");
	map(0x180000, 0x187fff).ram().w(m_palette, FUNC(palette_device::write16)).share("palette");
	map(0x1c0000, 0x1c0001).w(FUNC(shadfrce_state::bgscrollx_w<0>));
	map(0x1c0002, 0x1c0003).w(FUNC(shadfrce_state::bgscrolly_w<0>));
	map(0x1c0004, 0x1c0005).w(FUNC(shadfrce_state::bgscrollx_w<1>));
	map(0x1c0006, 0x1c0007).w(FUNC(shadfrce_state::bgscrolly_w<1>));
	map(0x1c0008, 0x1c0009).nopw(); // ??
	map(0x1c000a, 0x1c000b).nopr().w(FUNC(shadfrce_state::flip_screen));
	map(0x1c000c, 0x1c000d).nopw(); // ??
	map(0x1d0000, 0x1d0005).w(FUNC(shadfrce_state::irq_ack_w));
	map(0x1d0006, 0x1d0007).w(FUNC(shadfrce_state::irq_w));
	map(0x1d0008, 0x1d0009).w(FUNC(shadfrce_state::scanline_w));
	map(0x1d000c, 0x1d000d).nopr();
	map(0x1d000c, 0x1d000c).w(m_soundlatch, FUNC(generic_latch_8_device::write));
	map(0x1d000d, 0x1d000d).w(FUNC(shadfrce_state::screen_brt_w));
	map(0x1d0010, 0x1d0011).nopw(); // ??
	map(0x1d0012, 0x1d0013).nopw(); // ??
	map(0x1d0014, 0x1d0015).nopw(); // ??
	map(0x1d0016, 0x1d0017).w("watchdog", FUNC(watchdog_timer_device::reset16_w));
	map(0x1d0020, 0x1d0027).r(FUNC(shadfrce_state::input_ports_r));
	map(0x1f0000, 0x1fffff).ram();
}

// and the sound CPU

void shadfrce_state::oki_bankswitch_w(uint8_t data)
{
	m_oki->set_rom_bank(data & 1);
}

void shadfrce_state::sound_map(address_map &map)
{
	map(0x0000, 0xbfff).rom();
	map(0xc000, 0xc7ff).ram();
	map(0xc800, 0xc801).rw("ymsnd", FUNC(ym2151_device::read), FUNC(ym2151_device::write));
	map(0xd800, 0xd800).rw(m_oki, FUNC(okim6295_device::read), FUNC(okim6295_device::write));
	map(0xe000, 0xe000).r(m_soundlatch, FUNC(generic_latch_8_device::read));
	map(0xe800, 0xe800).w(FUNC(shadfrce_state::oki_bankswitch_w));
	map(0xf000, 0xffff).ram();
}


// Input Ports

// Similar to MUGSMASH_PLAYER_INPUT in edevices/mugsmash.cpp
#define SHADFRCE_PLAYER_INPUT( player, start ) \
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(player) PORT_8WAY \
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_PLAYER(player) PORT_8WAY \
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_PLAYER(player) PORT_8WAY \
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_PLAYER(player) PORT_8WAY \
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(player) \
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(player) \
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(player) \
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, start )


static INPUT_PORTS_START( shadfrce )
	PORT_START("P1")        // Fake IN0 (player 1 inputs)
	SHADFRCE_PLAYER_INPUT( 1, IPT_START1 )

	PORT_START("P2")        // Fake IN1 (player 2 inputs)
	SHADFRCE_PLAYER_INPUT( 2, IPT_START2 )

	PORT_START("EXTRA") // Fake IN2 (players 1 & 2 extra inputs
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_PLAYER(1)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON5 ) PORT_PLAYER(1)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON6 ) PORT_PLAYER(1)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_PLAYER(2)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON5 ) PORT_PLAYER(2)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON6 ) PORT_PLAYER(2)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("OTHER") // Fake IN3 (other extra inputs ?)
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("SYSTEM")    // Fake IN4 (system inputs)
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_COIN2 )          // only in "test mode" ?
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_SERVICE1 )       // only in "test mode" ?
	PORT_BIT( 0xf8, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("MISC")  // Fake IN5 (misc)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_CUSTOM )            // guess
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )           // must be ACTIVE_LOW or 'shadfrcj' jumps to the end (code at 0x04902e) */
	PORT_BIT( 0xeb, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("DSW1")  // not mapped directly
	PORT_DIPNAME( 0x01, 0x00, DEF_STR( Unused ) )            PORT_DIPLOCATION("SW1:1")
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x06, 0x06, DEF_STR( Coinage ) )           PORT_DIPLOCATION("SW1:2,3")
	PORT_DIPSETTING(    0x00, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x02, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x06, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x04, DEF_STR( 1C_2C ) )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Continue_Price ) )    PORT_DIPLOCATION("SW1:4")
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Free_Play ) )         PORT_DIPLOCATION("SW1:5")
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Flip_Screen ) )       PORT_DIPLOCATION("SW1:6")
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Demo_Sounds ) )       PORT_DIPLOCATION("SW1:7")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x40, DEF_STR( On ) )
	PORT_SERVICE( 0x80, IP_ACTIVE_LOW )                      PORT_DIPLOCATION("SW1:8")

	PORT_START("DSW2")  // not mapped directly
	PORT_DIPNAME( 0x03, 0x03, DEF_STR( Difficulty ) )        PORT_DIPLOCATION("SW2:1,2")
	PORT_DIPSETTING(    0x01, DEF_STR( Easy ) )
	PORT_DIPSETTING(    0x03, DEF_STR( Normal ) )
	PORT_DIPSETTING(    0x02, DEF_STR( Hard ) )                  // "Advanced" in manual
	PORT_DIPSETTING(    0x00, DEF_STR( Hardest ) )               // "Expert" in manual
	PORT_DIPNAME( 0x0c, 0x0c, "Stage Clear Energy Regain" )  PORT_DIPLOCATION("SW2:3,4")
	PORT_DIPSETTING(    0x04, "50%" )
	PORT_DIPSETTING(    0x0c, "25%" )
	PORT_DIPSETTING(    0x08, "10%" )
	PORT_DIPSETTING(    0x00, "0%" )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Unused ) )            PORT_DIPLOCATION("SW2:5")
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Unused ) )            PORT_DIPLOCATION("SW2:6")
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Unused ) )            PORT_DIPLOCATION("SW2:7")
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unused ) )            PORT_DIPLOCATION("SW2:8")
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	// PCB has 3rd DIP switch "SW3" listed in manual as 1:OFF & 2:OFF & 3:OFF (ALL "NOT USED"), 4:ON & 5:ON (ALL "DON'T TOUCH"), 6:OFF, 7:OFF, 8:OFF (ALL "NOT USED")
INPUT_PORTS_END

// Graphic Decoding

static const gfx_layout fg8x8x4_layout =
{
	8,8,
	RGN_FRAC(1,1),
	4,
	{ 0, 2, 4, 6 },
	{ 1, 0, 8*8+1, 8*8+0, 16*8+1, 16*8+0, 24*8+1, 24*8+0 },
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8 },
	4*8*8
};

static const gfx_layout sp16x16x5_layout =
{
	16,16,
	RGN_FRAC(1,5),
	5,
	{ RGN_FRAC(4,5), RGN_FRAC(3,5), RGN_FRAC(2,5), RGN_FRAC(1,5), RGN_FRAC(0,5) },
	{ STEP8(0,1), STEP8(16*8,1) },
	{ STEP16(0,8) },
	16*16
};

static const gfx_layout bg16x16x6_layout =
{
	16,16,
	RGN_FRAC(1,3),
	6,
	{ RGN_FRAC(0,3)+8, RGN_FRAC(0,3), RGN_FRAC(1,3)+8, RGN_FRAC(1,3), RGN_FRAC(2,3)+8, RGN_FRAC(2,3) },
	{ STEP8(0,1), STEP8(16*16,1) },
	{ STEP16(0,16) },
	2*16*16
};

static GFXDECODE_START( gfx_shadfrce )
	GFXDECODE_ENTRY( "chars",   0, fg8x8x4_layout,   0x0000, 256 )
	GFXDECODE_ENTRY( "sprites", 0, sp16x16x5_layout, 0x1000, 128 )
	GFXDECODE_ENTRY( "tiles",   0, bg16x16x6_layout, 0x2000, 128 )
GFXDECODE_END

// Machine Driver Bits

void shadfrce_state::shadfrce(machine_config &config)
{
	M68000(config, m_maincpu, XTAL(28'000'000) / 2);          // verified on PCB
	m_maincpu->set_addrmap(AS_PROGRAM, &shadfrce_state::main_map);
	TIMER(config, "scantimer").configure_scanline(FUNC(shadfrce_state::scanline), "screen", 0, 1);

	Z80(config, m_audiocpu, XTAL(3'579'545));         // verified on PCB
	m_audiocpu->set_addrmap(AS_PROGRAM, &shadfrce_state::sound_map);

	WATCHDOG_TIMER(config, "watchdog");

	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	m_screen->set_raw(XTAL(28'000'000) / 4, 448, 0, 320, 272, 8, 248);   // HTOTAL and VTOTAL are guessed
	m_screen->set_screen_update(FUNC(shadfrce_state::screen_update));
	m_screen->screen_vblank().set(m_spvideoram, FUNC(buffered_spriteram16_device::vblank_copy_rising));
	m_screen->set_palette(m_palette);

	GFXDECODE(config, m_gfxdecode, m_palette, gfx_shadfrce);
	PALETTE(config, m_palette).set_format(palette_device::xBGR_555, 0x4000);

	BUFFERED_SPRITERAM16(config, m_spvideoram);

	// sound hardware
	SPEAKER(config, "lspeaker").front_left();
	SPEAKER(config, "rspeaker").front_right();

	GENERIC_LATCH_8(config, m_soundlatch);
	m_soundlatch->data_pending_callback().set_inputline(m_audiocpu, INPUT_LINE_NMI);

	ym2151_device &ymsnd(YM2151(config, "ymsnd", XTAL(3'579'545)));      // verified on PCB
	ymsnd.irq_handler().set_inputline(m_audiocpu, 0);
	ymsnd.add_route(0, "lspeaker", 0.50);
	ymsnd.add_route(1, "rspeaker", 0.50);

	OKIM6295(config, m_oki, XTAL(13'495'200) / 8, okim6295_device::PIN7_HIGH); // verified on PCB
	m_oki->add_route(ALL_OUTPUTS, "lspeaker", 0.50);
	m_oki->add_route(ALL_OUTPUTS, "rspeaker", 0.50);
}

// Rom Defs.

// one of the high score tables in attract mode ends up corrupt on this set due to the game triggering a text dialog box, is this due to a timing error?
ROM_START( shadfrce )
	ROM_REGION( 0x100000, "maincpu", 0 ) // 68000 code
	ROM_LOAD16_BYTE( "32a12-011.34", 0x00001, 0x40000, CRC(0c041e08) SHA1(7b9d52cb1f6bc217c6e64287bd9630aa37243513) ) // World Version 3
	ROM_LOAD16_BYTE( "32a13-010.26", 0x00000, 0x40000, CRC(00985361) SHA1(e9da1b096b25a6ee46bab6230dda66dccdd4bed8) )
	ROM_LOAD16_BYTE( "32a14-010.33", 0x80001, 0x40000, CRC(ea03ca25) SHA1(7af1ee7c36c70f80ba1e096473b5786b205ab00b) )
	ROM_LOAD16_BYTE( "32j15-01.14",  0x80000, 0x40000, CRC(3dc3a84a) SHA1(166ad91b93192d94e3f6d2fe6dde02f59d334f75) ) // matches Japan version 2, not US version 2

	ROM_REGION( 0x10000, "audiocpu", 0 ) // Z80 code
	ROM_LOAD( "32j10-0.42",  0x00000, 0x10000, CRC(65daf475) SHA1(7144332b2d17af8645e22e1926b33113db0d20e2) )

	ROM_REGION( 0x020000, "chars", 0 )
	ROM_LOAD( "32j11-0.55",  0x00000, 0x20000, CRC(7252d993) SHA1(43f7de381841039aa290486aafb98e2cf3b8579b) )

	ROM_REGION( 0xa00000, "sprites", 0 )
	ROM_LOAD( "32j4-0.12",  0x000000, 0x200000, CRC(1ebea5b6) SHA1(35bd49dda9ad75326d45ffb10c87d83fc4f1b7a8) )
	ROM_LOAD( "32j5-0.13",  0x200000, 0x200000, CRC(600026b5) SHA1(5641246300d7e20dcff1eae004647faaee6cd1c6) )
	ROM_LOAD( "32j6-0.24",  0x400000, 0x200000, CRC(6cde8ebe) SHA1(750933798235951fe24b2e667c33f692612c0aa0) )
	ROM_LOAD( "32j7-0.25",  0x600000, 0x200000, CRC(bcb37922) SHA1(f3eee73c8b9f4873a7f1cc42e334e7502eaee3c8) )
	ROM_LOAD( "32j8-0.32",  0x800000, 0x200000, CRC(201bebf6) SHA1(c89d2895ea5b19daea1f88542419f4e10f437c73) )

	ROM_REGION( 0x300000, "tiles", 0 )
	ROM_LOAD( "32j1-0.4",  0x000000, 0x100000, CRC(f1cca740) SHA1(339079b95ca137e66b4f032ad67a0adf58cca100) )
	ROM_LOAD( "32j2-0.5",  0x100000, 0x100000, CRC(5fac3e01) SHA1(20c30f4c76e303285ae37e596afe86aa4812c3b9) )
	ROM_LOAD( "32j3-0.6",  0x200000, 0x100000, CRC(d297925e) SHA1(5bc4d37bf0dc54114884c816b94a64ef1ccfeda5) )

	ROM_REGION( 0x080000, "oki", 0 )
	ROM_LOAD( "32j9-0.76",  0x000000, 0x080000, CRC(16001e81) SHA1(67928d2024f963aee91f1498b6f4c76101d2f3b8) )
ROM_END

ROM_START( shadfrceu )
	ROM_REGION( 0x100000, "maincpu", 0 ) // 68000 code
	ROM_LOAD16_BYTE( "32a12-01.34", 0x00001, 0x40000, CRC(04501198) SHA1(50f981c13f9ed19d681d494376018ba86464ea13) ) // US Version 2
	ROM_LOAD16_BYTE( "32a13-01.26", 0x00000, 0x40000, CRC(b8f8a05c) SHA1(bd9d4218a7cf57b56aec1f7e710e02af8471f9d7) )
	ROM_LOAD16_BYTE( "32a14-0.33",  0x80001, 0x40000, CRC(08279be9) SHA1(1833526b23feddb58b21874070ad2bf3b6be8dca) )
	ROM_LOAD16_BYTE( "32a15-0.14",  0x80000, 0x40000, CRC(bfcadfea) SHA1(1caa9fc30d8622ce4c7221039c446e99cc8f5346) )

	ROM_REGION( 0x10000, "audiocpu", 0 ) // Z80 code
	ROM_LOAD( "32j10-0.42",  0x00000, 0x10000, CRC(65daf475) SHA1(7144332b2d17af8645e22e1926b33113db0d20e2) )

	ROM_REGION( 0x020000, "chars", 0 )
	ROM_LOAD( "32a11-0.55",  0x00000, 0x20000, CRC(cfaf5e77) SHA1(eab76e085f695c74cc868aaf95f04ff2acf66ee9) )

	ROM_REGION( 0xa00000, "sprites", 0 )
	ROM_LOAD( "32j4-0.12",  0x000000, 0x200000, CRC(1ebea5b6) SHA1(35bd49dda9ad75326d45ffb10c87d83fc4f1b7a8) )
	ROM_LOAD( "32j5-0.13",  0x200000, 0x200000, CRC(600026b5) SHA1(5641246300d7e20dcff1eae004647faaee6cd1c6) )
	ROM_LOAD( "32j6-0.24",  0x400000, 0x200000, CRC(6cde8ebe) SHA1(750933798235951fe24b2e667c33f692612c0aa0) )
	ROM_LOAD( "32j7-0.25",  0x600000, 0x200000, CRC(bcb37922) SHA1(f3eee73c8b9f4873a7f1cc42e334e7502eaee3c8) )
	ROM_LOAD( "32j8-0.32",  0x800000, 0x200000, CRC(201bebf6) SHA1(c89d2895ea5b19daea1f88542419f4e10f437c73) )

	ROM_REGION( 0x300000, "tiles", 0 )
	ROM_LOAD( "32j1-0.4",  0x000000, 0x100000, CRC(f1cca740) SHA1(339079b95ca137e66b4f032ad67a0adf58cca100) )
	ROM_LOAD( "32j2-0.5",  0x100000, 0x100000, CRC(5fac3e01) SHA1(20c30f4c76e303285ae37e596afe86aa4812c3b9) )
	ROM_LOAD( "32j3-0.6",  0x200000, 0x100000, CRC(d297925e) SHA1(5bc4d37bf0dc54114884c816b94a64ef1ccfeda5) )

	ROM_REGION( 0x080000, "oki", 0 )
	ROM_LOAD( "32j9-0.76",  0x000000, 0x080000, CRC(16001e81) SHA1(67928d2024f963aee91f1498b6f4c76101d2f3b8) )
ROM_END

ROM_START( shadfrcej )
	ROM_REGION( 0x100000, "maincpu", 0 ) // 68000 code
	ROM_LOAD16_BYTE( "32j12-01.34", 0x00001, 0x40000, CRC(38fdbe1d) SHA1(476d8ef2c0d2a8c568ce44631f93f8c730f91b08) ) // Japan Version 2
	ROM_LOAD16_BYTE( "32j13-01.26", 0x00000, 0x40000, CRC(6e1df6f1) SHA1(c165553fe967b437413dd7ddc87a267548dd0ca9) )
	ROM_LOAD16_BYTE( "32j14-01.33", 0x80001, 0x40000, CRC(89e3fb60) SHA1(90de38558d63215a0079079030e8b1097599c9e5) )
	ROM_LOAD16_BYTE( "32j15-01.14", 0x80000, 0x40000, CRC(3dc3a84a) SHA1(166ad91b93192d94e3f6d2fe6dde02f59d334f75) )

	ROM_REGION( 0x10000, "audiocpu", 0 ) // Z80 code
	ROM_LOAD( "32j10-0.42",  0x00000, 0x10000, CRC(65daf475) SHA1(7144332b2d17af8645e22e1926b33113db0d20e2) )

	ROM_REGION( 0x020000, "chars", 0 )
	ROM_LOAD( "32j11-0.55",  0x00000, 0x20000, CRC(7252d993) SHA1(43f7de381841039aa290486aafb98e2cf3b8579b) )

	ROM_REGION( 0xa00000, "sprites", 0 )
	ROM_LOAD( "32j4-0.12",  0x000000, 0x200000, CRC(1ebea5b6) SHA1(35bd49dda9ad75326d45ffb10c87d83fc4f1b7a8) )
	ROM_LOAD( "32j5-0.13",  0x200000, 0x200000, CRC(600026b5) SHA1(5641246300d7e20dcff1eae004647faaee6cd1c6) )
	ROM_LOAD( "32j6-0.24",  0x400000, 0x200000, CRC(6cde8ebe) SHA1(750933798235951fe24b2e667c33f692612c0aa0) )
	ROM_LOAD( "32j7-0.25",  0x600000, 0x200000, CRC(bcb37922) SHA1(f3eee73c8b9f4873a7f1cc42e334e7502eaee3c8) )
	ROM_LOAD( "32j8-0.32",  0x800000, 0x200000, CRC(201bebf6) SHA1(c89d2895ea5b19daea1f88542419f4e10f437c73) )

	ROM_REGION( 0x300000, "tiles", 0 )
	ROM_LOAD( "32j1-0.4",  0x000000, 0x100000, CRC(f1cca740) SHA1(339079b95ca137e66b4f032ad67a0adf58cca100) )
	ROM_LOAD( "32j2-0.5",  0x100000, 0x100000, CRC(5fac3e01) SHA1(20c30f4c76e303285ae37e596afe86aa4812c3b9) )
	ROM_LOAD( "32j3-0.6",  0x200000, 0x100000, CRC(d297925e) SHA1(5bc4d37bf0dc54114884c816b94a64ef1ccfeda5) )

	ROM_REGION( 0x080000, "oki", 0 )
	ROM_LOAD( "32j9-0.76",  0x000000, 0x080000, CRC(16001e81) SHA1(67928d2024f963aee91f1498b6f4c76101d2f3b8) )
ROM_END

} // anonymous namespace


GAME( 1993, shadfrce,   0,        shadfrce, shadfrce, shadfrce_state, empty_init, ROT0, "Technos Japan", "Shadow Force (World, Version 3)",                 MACHINE_NO_COCKTAIL | MACHINE_SUPPORTS_SAVE )
GAME( 1993, shadfrceu,  shadfrce, shadfrce, shadfrce, shadfrce_state, empty_init, ROT0, "Technos Japan", "Shadow Force (US, Version 2)",                    MACHINE_NO_COCKTAIL | MACHINE_SUPPORTS_SAVE )
GAME( 1993, shadfrcej,  shadfrce, shadfrce, shadfrce, shadfrce_state, empty_init, ROT0, "Technos Japan", "Shadow Force - Henshin Ninja (Japan, Version 2)", MACHINE_NO_COCKTAIL | MACHINE_SUPPORTS_SAVE )
