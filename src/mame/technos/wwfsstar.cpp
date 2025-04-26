// license:BSD-3-Clause
// copyright-holders:David Haywood, Stephane Humbert
// thanks-to:Richard Bush
/*******************************************************************************
 WWF Superstars (C) 1989 Technos Japan
********************************************************************************
 driver by David Haywood

 Special Thanks to:

 Richard Bush & the Rest of the Raine Team - Raine's WWF Superstars driver on
 which most of this driver has been based.

********************************************************************************

WWF Superstars
Technos 1989

PCB Layout
----------

TA-0024-P1-05
|--------------------------------------------------------------------|
|M51516   558     558   YM3012        M6295       YM2151  3.579545MHz|
|                                1.056MHz                            |
|         558     558                               6116             |
|                              24A9-0.46                            |-|
|                                                   Z80             | |
|                              24J8-0.45                            | |
|                                              24AB-0.12            | |
|                                                                   | |
|               24AA-0.58                                           | |
|                                                   6116            | |
|                                                                   | |
|J                                                                  | |
|A                                                                  |-|
|M                                                                   |
|M                                                                   |
|A    DSW1                                       6116 6116           |
|     DSW2                                                           |
|                                                                   |-|
|                                                                   | |
|                                                                   | |
|                                                                   | |
|                                                                   | |
|                                                                   | |
|                                                                   | |
|                                                                   | |
|                                                                   | |
|                 68000        24AD-04.35    6264                   |-|
|                                                                    |
|                              24AC-04.34    6264                    |
|20MHz                                                               |
|--------------------------------------------------------------------|
Notes:
      Z80    - 3.579545MHz
      68000  - 10.000MHz [20/2]
      M6295  - 1.056MHz (resonator)
      YM2151 - 3.579545MHz
      VSync  - 57.4447Hz


Bottom Board

TA-0024-P2-23
|--------------------------------------------------------------------|
|                                     2018                           |
| IC119                                                              |
|                                                                    |
|                                                                   |-|
| IC118                                                             | |
|                                     2018                          | |
|                                                                   | |
| IC117                                                             | |
|                                                                   | |
|                                                                   | |
| IC116                                         2018                | |
|                                                                   | |
|                                                                   |-|
| IC115              2018             2018                           |
|                                                                    |
|                                     2018                           |
| IC114                                                              |
|                                                                   |-|
|                                                                   | |
|                                                                   | |
|                                                                   | |
|                                                                   | |
|                                                                   | |
| IC113                                                             | |
|                                                                   | |
|                                                                   | |
|                                       |-------|                   |-|
| IC112                                 |TECHNOS|  24MHz             |
|                                       |TJ-001 |                    |
|                                       |-------|                    |
|--------------------------------------------------------------------|
Notes:
      IC11x  - TC534000 MaskROMs
      TJ-001 - Probably a microcontroller badged as a Technos Custom IC (QFP80).
               Clocks: pin 1 - 24MHz, pin 3 - 24/2, pin 4 - 24/4, pin 5 - 24/8,
               pin 6 - 24/16, pin 7 - 24/32, pin 8 - 24/64, pin 64,65 - 1.5MHz

 Hardware:

 Primary CPU : 68000

 Sound CPUs : Z80

 Sound Chips : YM2151, M6295

 3 Layers from now on if mentioned will be referred to as

 BG0 - Background
 SPR - Sprites
 FG0 - Foreground / Text Layer

*******************************************************************************/

#include "emu.h"

#include "cpu/m68000/m68000.h"
#include "cpu/z80/z80.h"
#include "machine/gen_latch.h"
#include "machine/timer.h"
#include "sound/okim6295.h"
#include "sound/ymopm.h"
#include "sound/ymopn.h"

#include "emupal.h"
#include "screen.h"
#include "speaker.h"
#include "tilemap.h"


namespace {

class wwfsstar_state : public driver_device
{
public:
	wwfsstar_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_audiocpu(*this, "audiocpu"),
		m_gfxdecode(*this, "gfxdecode"),
		m_screen(*this, "screen"),
		m_palette(*this, "palette"),
		m_soundlatch(*this, "soundlatch"),
		m_spriteram(*this, "spriteram"),
		m_fg0_videoram(*this, "fg0_videoram"),
		m_bg0_videoram(*this, "bg0_videoram"),
		m_scroll(*this, "scroll")
	{ }

	void wwfsstar(machine_config &config);
	void wwfsstarb2(machine_config &config);

	int vblank_r();

protected:
	virtual void video_start() override ATTR_COLD;

private:
	required_device<cpu_device> m_maincpu;
	required_device<cpu_device> m_audiocpu;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<screen_device> m_screen;
	required_device<palette_device> m_palette;
	required_device<generic_latch_8_device> m_soundlatch;

	required_shared_ptr<uint16_t> m_spriteram;
	required_shared_ptr<uint16_t> m_fg0_videoram;
	required_shared_ptr<uint16_t> m_bg0_videoram;
	required_shared_ptr<uint16_t> m_scroll;

	uint8_t m_vblank = 0U;
	tilemap_t *m_fg0_tilemap = nullptr;
	tilemap_t *m_bg0_tilemap = nullptr;

	void flipscreen_w(uint16_t data);
	void irqack_w(offs_t offset, uint16_t data);
	void fg0_videoram_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	void bg0_videoram_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);

	TIMER_DEVICE_CALLBACK_MEMBER(scanline);

	TILE_GET_INFO_MEMBER(get_fg0_tile_info);
	TILEMAP_MAPPER_MEMBER(bg0_scan);
	TILE_GET_INFO_MEMBER(get_bg0_tile_info);

	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void draw_sprites(bitmap_ind16 &bitmap, const rectangle &cliprect);

	void main_map(address_map &map) ATTR_COLD;
	void sound_map(address_map &map) ATTR_COLD;
	void bootleg_sound_map(address_map &map) ATTR_COLD;
};


/*******************************************************************************
 Write Handlers
********************************************************************************
 for writes to Video Ram
*******************************************************************************/

void wwfsstar_state::fg0_videoram_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	COMBINE_DATA(&m_fg0_videoram[offset]);
	m_fg0_tilemap->mark_tile_dirty(offset / 2);
}

void wwfsstar_state::bg0_videoram_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	COMBINE_DATA(&m_bg0_videoram[offset]);
	m_bg0_tilemap->mark_tile_dirty(offset / 2);
}


/*******************************************************************************
 Tilemap Related Functions
*******************************************************************************/

TILE_GET_INFO_MEMBER(wwfsstar_state::get_fg0_tile_info)
{
	/*- FG0 RAM Format -**

	  0x1000 sized region (4096 bytes)

	  32x32 tilemap, 4 bytes per tile

	  ---- ----  CCCC TTTT  ---- ----  TTTT TTTT

	  C = Colour Bank (0-15)
	  T = Tile Number (0 - 4095)

	  other bits unknown / unused

	**- End of Comments -*/

	uint16_t *tilebase = &m_fg0_videoram[tile_index * 2];
	int tileno = (tilebase[1] & 0x00ff) | ((tilebase[0] & 0x000f) << 8);
	int colbank = (tilebase[0] & 0x00f0) >> 4;
	tileinfo.set(0, tileno, colbank, 0);
}

TILEMAP_MAPPER_MEMBER(wwfsstar_state::bg0_scan)
{
	return (col & 0x0f) + ((row & 0x0f) << 4) + ((col & 0x10) << 4) + ((row & 0x10) << 5);
}

TILE_GET_INFO_MEMBER(wwfsstar_state::get_bg0_tile_info)
{
	/*- BG0 RAM Format -**

	  0x1000 sized region (4096 bytes)

	  32x32 tilemap, 4 bytes per tile

	  ---- ----  FCCC TTTT  ---- ----  TTTT TTTT

	  C = Colour Bank (0-7)
	  T = Tile Number (0 - 4095)
	  F = FlipX

	  other bits unknown / unused

	**- End of Comments -*/

	uint16_t *tilebase = &m_bg0_videoram[tile_index * 2];
	int tileno = (tilebase[1] & 0x00ff) | ((tilebase[0] & 0x000f) << 8);
	int colbank = (tilebase[0] & 0x0070) >> 4;
	int flipx = (tilebase[0] & 0x0080) >> 7;
	tileinfo.set(2, tileno, colbank, flipx ? TILE_FLIPX : 0);
}


/*******************************************************************************
 Sprite Related Functions
********************************************************************************
 sprite colour marking could probably be improved..
*******************************************************************************/

void wwfsstar_state::draw_sprites(bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	/*- SPR RAM Format -**

	  0x400 sized region (1024 bytes)

	  10 bytes per sprite

	  ---- ---- yyyy yyyy ---- ---- CCCC XYLE ---- ---- fFNN NNNN ---- ---- nnnn nnnn ---- ---- xxxx xxxx

	  Yy = sprite Y Position
	  Xx = sprite X Position
	  C  = colour bank
	  f  = flip Y
	  F  = flip X
	  L  = chain sprite (32x16)
	  E  = sprite enable
	  Nn = Sprite Number

	  other bits unused

	**- End of Comments -*/

	gfx_element *gfx = m_gfxdecode->gfx(1);

	for (int offset = 0; offset <= m_spriteram.length() - 5; offset += 5)
	{
		const uint16_t *source = m_spriteram + offset;
		int const enable = (source[1] & 0x0001);

		if (enable)
		{
			int ypos = ((source[0] & 0x00ff) | ((source[1] & 0x0004) << 6));
			ypos = (((256 - ypos) & 0x1ff) - 16) ;
			int xpos = ((source[4] & 0x00ff) | ((source[1] & 0x0008) << 5));
			xpos = (((256 - xpos) & 0x1ff) - 16);
			int flipx = (source[2] & 0x0080 ) >> 7;
			int flipy = (source[2] & 0x0040 ) >> 6;
			int chain = (source[1] & 0x0002 ) >> 1;
			chain += 1;
			int number = (source[3] & 0x00ff) | ((source[2] & 0x003f) << 8);
			int const colourbank = (source[1] & 0x00f0) >> 4;

			number &= ~(chain - 1);

			if (flip_screen())
			{
				flipy = !flipy;
				flipx = !flipx;
				ypos = 240 - ypos;
				xpos = 240 - xpos;
			}

			for (int count = 0; count < chain; count++)
			{
				if (flip_screen())
				{
					if (!flipy)
						gfx->transpen(bitmap, cliprect, number + count, colourbank, flipx, flipy, xpos, ypos + 16 * count, 0);
					else
						gfx->transpen(bitmap, cliprect, number + count, colourbank, flipx, flipy, xpos, ypos + (16 * (chain - 1)) - (16 * count), 0);
				}
				else
				{
					if (!flipy)
						gfx->transpen(bitmap, cliprect, number + count, colourbank, flipx, flipy, xpos, ypos - (16 * (chain - 1)) + (16 * count), 0);
					else
						gfx->transpen(bitmap, cliprect, number + count, colourbank, flipx, flipy, xpos, ypos - 16 * count, 0);
				}
			}
		}
	}
}


/*******************************************************************************
 Video Start and Refresh Functions
********************************************************************************
 Drawing Order is simple
 BG0 - Back
 SPR - Middle
 FG0 - Front
*******************************************************************************/

void wwfsstar_state::video_start()
{
	m_fg0_tilemap = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(wwfsstar_state::get_fg0_tile_info)), TILEMAP_SCAN_ROWS, 8, 8, 32, 32);
	m_fg0_tilemap->set_transparent_pen(0);

	m_bg0_tilemap = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(wwfsstar_state::get_bg0_tile_info)), tilemap_mapper_delegate(*this, FUNC(wwfsstar_state::bg0_scan)), 16, 16, 32, 32);
	m_fg0_tilemap->set_transparent_pen(0);

	save_item(NAME(m_vblank));
}

uint32_t wwfsstar_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	m_bg0_tilemap->set_scrollx(0, m_scroll[0]);
	m_bg0_tilemap->set_scrolly(0, m_scroll[1]);

	m_bg0_tilemap->draw(screen, bitmap, cliprect, 0, 0);
	draw_sprites(bitmap, cliprect);
	m_fg0_tilemap->draw(screen, bitmap, cliprect, 0, 0);

	return 0;
}


/*******************************************************************************
 Memory Maps
********************************************************************************
 Pretty Straightforward
*******************************************************************************/

void wwfsstar_state::main_map(address_map &map)
{
	map(0x000000, 0x03ffff).rom();
	map(0x080000, 0x080fff).ram().w(FUNC(wwfsstar_state::fg0_videoram_w)).share(m_fg0_videoram);
	map(0x0c0000, 0x0c0fff).ram().w(FUNC(wwfsstar_state::bg0_videoram_w)).share(m_bg0_videoram);
	map(0x100000, 0x1003ff).ram().share(m_spriteram);
	map(0x140000, 0x1402ff).w(m_palette, FUNC(palette_device::write16)).share("palette");
	map(0x180000, 0x180003).w(FUNC(wwfsstar_state::irqack_w));
	map(0x180000, 0x180001).portr("DSW1");
	map(0x180002, 0x180003).portr("DSW2");
	map(0x180004, 0x180005).portr("P1");
	map(0x180004, 0x180007).writeonly().share(m_scroll);
	map(0x180006, 0x180007).portr("P2");
	map(0x180008, 0x180009).portr("SYSTEM");
	map(0x180009, 0x180009).w(m_soundlatch, FUNC(generic_latch_8_device::write));
	map(0x18000a, 0x18000b).w(FUNC(wwfsstar_state::flipscreen_w));
	map(0x1c0000, 0x1c3fff).ram(); // Work RAM
}

void wwfsstar_state::sound_map(address_map &map)
{
	map(0x0000, 0x7fff).rom();
	map(0x8000, 0x87ff).ram();
	map(0x8800, 0x8801).rw("ymsnd", FUNC(ym2151_device::read), FUNC(ym2151_device::write));
	map(0x9800, 0x9800).rw("oki", FUNC(okim6295_device::read), FUNC(okim6295_device::write));
	map(0xa000, 0xa000).r(m_soundlatch, FUNC(generic_latch_8_device::read));
}

void wwfsstar_state::bootleg_sound_map(address_map &map)
{
	map(0x0000, 0x7fff).rom();
	map(0x8000, 0x87ff).ram();
	map(0x8800, 0x8801).rw("ym1", FUNC(ym2203_device::read), FUNC(ym2203_device::write));
	map(0x8802, 0x8803).rw("ym2", FUNC(ym2203_device::read), FUNC(ym2203_device::write));
	map(0x8804, 0x8805).nopw(); // sound program expects 3xYM2203!!!
	map(0x9800, 0x9800).rw("oki", FUNC(okim6295_device::read), FUNC(okim6295_device::write));
	map(0xa000, 0xa000).r(m_soundlatch, FUNC(generic_latch_8_device::read));
}


/*******************************************************************************
 Read / Write Handlers
********************************************************************************
 as used by the above memory map
*******************************************************************************/

void wwfsstar_state::flipscreen_w(uint16_t data)
{
	flip_screen_set(data & 1);
}

void wwfsstar_state::irqack_w(offs_t offset, uint16_t data)
{
	if (offset == 0)
		m_maincpu->set_input_line(6, CLEAR_LINE);
	else
		m_maincpu->set_input_line(5, CLEAR_LINE);
}

TIMER_DEVICE_CALLBACK_MEMBER(wwfsstar_state::scanline)
{
	// Interrupt behaviour verified from actual PCB
	int scanline = param;

	// Vblank is lowered on scanline 0
	if (scanline == 0)
	{
		m_vblank = 0;
	}

	// An interrupt is generated every 16 scanlines
	if (scanline % 16 == 0)
	{
		if (scanline > 0)
			m_screen->update_partial(scanline - 1);
		m_maincpu->set_input_line(5, ASSERT_LINE);
	}

	// Vblank is raised on scanline 240
	if (scanline == 240)
	{
		m_vblank = 1;
		m_screen->update_partial(scanline - 1);
		m_maincpu->set_input_line(6, ASSERT_LINE);
	}
}

int wwfsstar_state::vblank_r()
{
	return m_vblank;
}


/*******************************************************************************
 Input Ports
********************************************************************************
 2 Sets of Player Controls
 A Misc Inputs inc. Coins
 2 Sets of Dipswitches
*******************************************************************************/

static INPUT_PORTS_START( wwfsstar )
	PORT_START("P1")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(1)
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_PLAYER(1)
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_PLAYER(1)
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_PLAYER(1)
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1)
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(1)
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_START1 ) PORT_NAME("Button A (1P VS CPU - Power Up)")

	PORT_START("P2")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(2)
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_PLAYER(2)
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_PLAYER(2)
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_PLAYER(2)
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2)
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_START3 ) PORT_NAME("Button C (1P/2P VS CPU)")
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_START2 ) PORT_NAME("Button B (1P VS 2P - Buy-in)")

	PORT_START("SYSTEM")
	PORT_BIT( 0x0001, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_READ_LINE_MEMBER(FUNC(wwfsstar_state::vblank_r)) // VBlank
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("DSW1")
	PORT_DIPNAME( 0x07, 0x07, DEF_STR( Coin_A ) )       PORT_DIPLOCATION("SW1:1,2,3")
	PORT_DIPSETTING(    0x00,  DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(    0x01,  DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x02,  DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x07,  DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x06,  DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x05,  DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0x04,  DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(    0x03,  DEF_STR( 1C_5C ) )
	PORT_DIPNAME( 0x38, 0x38, DEF_STR( Coin_B ) )       PORT_DIPLOCATION("SW1:4,5,6")
	PORT_DIPSETTING(    0x00,  DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(    0x08,  DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x10,  DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x38,  DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x30,  DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x28,  DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0x20,  DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(    0x18,  DEF_STR( 1C_5C ) )
	PORT_DIPUNUSED_DIPLOC( 0x40, 0x40, "SW1:7" )        // Manual shows Cabinet Type: Off = Upright & On = Table, has no effect
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Flip_Screen ) )  PORT_DIPLOCATION("SW1:8")
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("DSW2")
	PORT_DIPNAME( 0x03, 0x03, DEF_STR( Difficulty ) )   PORT_DIPLOCATION("SW2:1,2")
	PORT_DIPSETTING(    0x01, DEF_STR( Easy ) )
	PORT_DIPSETTING(    0x03, DEF_STR( Normal ) )
	PORT_DIPSETTING(    0x02, DEF_STR( Hard ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Hardest ) )
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Demo_Sounds ) )  PORT_DIPLOCATION("SW2:3")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x04, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, "Super Techniques" )      PORT_DIPLOCATION("SW2:4")   // Check code at 0x014272
	PORT_DIPSETTING(    0x08, DEF_STR( Normal ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Hard ) )
	PORT_DIPNAME( 0x30, 0x30, "Time" )          PORT_DIPLOCATION("SW2:5,6")
	PORT_DIPSETTING(    0x20, "+2:30" )
	PORT_DIPSETTING(    0x30, "Default" )
	PORT_DIPSETTING(    0x10, "-2:30" )
	PORT_DIPSETTING(    0x00, "-5:00" )
	PORT_DIPUNUSED_DIPLOC( 0x40, 0x40, "SW2:7" )        // Manual shows "3 Buttons" has no or unknown effect
	PORT_DIPNAME( 0x80, 0x80, "Health For Winning" )    PORT_DIPLOCATION("SW2:8")
	PORT_DIPSETTING(    0x80, DEF_STR( No ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Yes ) )
INPUT_PORTS_END


/*******************************************************************************
 Graphic Decoding
********************************************************************************
 Tiles are decoded the same as Double Dragon, strangely enough another
 Technos Game ;)
*******************************************************************************/

static const gfx_layout tiles8x8_layout =
{
	8,8,
	RGN_FRAC(1,1),
	4,
	{ 0, 2, 4, 6 },
	{ 1, 0, 8*8+1, 8*8+0, 16*8+1, 16*8+0, 24*8+1, 24*8+0 },
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8 },
	32*8
};

static const gfx_layout tiles16x16_layout =
{
	16,16,
	RGN_FRAC(1,2),
	4,
	{ RGN_FRAC(1,2)+0, RGN_FRAC(1,2)+4, 0, 4 },
	{ 3, 2, 1, 0, 16*8+3, 16*8+2, 16*8+1, 16*8+0,
			32*8+3, 32*8+2, 32*8+1, 32*8+0, 48*8+3, 48*8+2, 48*8+1, 48*8+0 },
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8,
			8*8, 9*8, 10*8, 11*8, 12*8, 13*8, 14*8, 15*8 },
	64*8
};

static GFXDECODE_START( gfx_wwfsstar )
	GFXDECODE_ENTRY( "fgtiles", 0, tiles8x8_layout,     0, 16 ) // colors 0-255
	GFXDECODE_ENTRY( "sprites", 0, tiles16x16_layout, 128, 16 ) // colors 128-383
	GFXDECODE_ENTRY( "bgtiles", 0, tiles16x16_layout, 256,  8 ) // colors 256-383
GFXDECODE_END


/*******************************************************************************
 Machine Driver(s)
*******************************************************************************/

void wwfsstar_state::wwfsstar(machine_config &config)
{
	// basic machine hardware
	M68000(config, m_maincpu, 20_MHz_XTAL / 2);
	m_maincpu->set_addrmap(AS_PROGRAM, &wwfsstar_state::main_map);

	TIMER(config, "scantimer").configure_scanline(FUNC(wwfsstar_state::scanline), "screen", 0, 1);

	Z80(config, m_audiocpu, 3.579545_MHz_XTAL);
	m_audiocpu->set_addrmap(AS_PROGRAM, &wwfsstar_state::sound_map);

	// video hardware
	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	m_screen->set_raw(20_MHz_XTAL / 4, 320, 0, 256, 272, 8, 248); // HTOTAL and VTOTAL are guessed
	m_screen->set_screen_update(FUNC(wwfsstar_state::screen_update));
	m_screen->set_palette(m_palette);

	GFXDECODE(config, m_gfxdecode, m_palette, gfx_wwfsstar);
	PALETTE(config, m_palette).set_format(palette_device::xBGR_444, 384);

	/* sound hardware */
	SPEAKER(config, "lspeaker").front_left();
	SPEAKER(config, "rspeaker").front_right();

	GENERIC_LATCH_8(config, m_soundlatch);
	m_soundlatch->data_pending_callback().set_inputline(m_audiocpu, INPUT_LINE_NMI);

	ym2151_device &ymsnd(YM2151(config, "ymsnd", 3.579545_MHz_XTAL));
	ymsnd.irq_handler().set_inputline(m_audiocpu, 0);
	ymsnd.add_route(0, "lspeaker", 0.45);
	ymsnd.add_route(1, "rspeaker", 0.45);

	okim6295_device &oki(OKIM6295(config, "oki", 1.056_MHz_XTAL, okim6295_device::PIN7_HIGH));
	oki.add_route(ALL_OUTPUTS, "lspeaker", 0.47);
	oki.add_route(ALL_OUTPUTS, "rspeaker", 0.47);
}

void wwfsstar_state::wwfsstarb2(machine_config &config)
{
	// this bootleg appears to have been produced by multiple manufacturers.
	// on the board the wwfsstarb2 romset was dumped from:
	// - 68000 has a 16 MHz crystal next to it but the CPU itself is rated for 10 MHz.
	// - graphics board has a 24 MHz crystal on it.
	//   there are width issues when hooked up to a real monitor, suggesting the pixel
	//   clock may be running fast.
	// - it doesn't appear there is a crystal or resonator for the OKI PCM chip.
	//   a 74ls393 is next to the 3.579 MHz crystal.
	// for now we use clockrates from the real PCB.

	// basic machine hardware
	M68000(config, m_maincpu, 20_MHz_XTAL / 2);
	m_maincpu->set_addrmap(AS_PROGRAM, &wwfsstar_state::main_map);

	TIMER(config, "scantimer").configure_scanline(FUNC(wwfsstar_state::scanline), "screen", 0, 1);

	Z80(config, m_audiocpu, 3.579545_MHz_XTAL);
	m_audiocpu->set_addrmap(AS_PROGRAM, &wwfsstar_state::bootleg_sound_map);

	// video hardware
	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	m_screen->set_raw(20_MHz_XTAL / 4, 320, 0, 256, 272, 8, 248);
	m_screen->set_screen_update(FUNC(wwfsstar_state::screen_update));
	m_screen->set_palette(m_palette);

	GFXDECODE(config, m_gfxdecode, m_palette, gfx_wwfsstar);
	PALETTE(config, m_palette).set_format(palette_device::xBGR_444, 384);

	// sound hardware is strictly mono
	SPEAKER(config, "mono").front_center();

	GENERIC_LATCH_8(config, m_soundlatch);
	m_soundlatch->data_pending_callback().set_inputline(m_audiocpu, INPUT_LINE_NMI);

	ym2203_device &ym1(YM2203(config, "ym1", 3.579545_MHz_XTAL));
	ym1.irq_handler().set_inputline(m_audiocpu, 0);
	ym1.add_route(ALL_OUTPUTS, "mono", 0.45);
	YM2203(config, "ym2", 3.579545_MHz_XTAL).add_route(ALL_OUTPUTS, "mono", 0.45);

	okim6295_device &oki(OKIM6295(config, "oki", 1.056_MHz_XTAL, okim6295_device::PIN7_HIGH));
	oki.add_route(ALL_OUTPUTS, "mono", 0.47);
}


/*******************************************************************************
 Rom Loaders / Game Drivers
*******************************************************************************/

ROM_START( wwfsstar )
	ROM_REGION( 0x40000, "maincpu", 0 ) // 68000
	ROM_LOAD16_BYTE( "24ac-0_j-1.34", 0x00000, 0x20000, CRC(ec8fd2c9) SHA1(04ab93e2a1becdc480750c3b55839328b2af4639) )
	ROM_LOAD16_BYTE( "24ad-0_j-1.35", 0x00001, 0x20000, CRC(54e614e4) SHA1(ee924dea977606fcb1222d1aa89211994126a182) )

	ROM_REGION( 0x08000, "audiocpu", 0 ) // Z80
	ROM_LOAD( "24ab-0.12", 0x00000, 0x08000, CRC(1e44f8aa) SHA1(e03857d6954e9b9b6073b211e2d6570032af8807) )

	ROM_REGION( 0x40000, "oki", 0 ) // ADPCM samples
	ROM_LOAD( "24a9-0.46", 0x00000, 0x20000, CRC(703ff08f) SHA1(08c4d33208eb4c76c751a1a0fe16a817bdc30820) )
	ROM_LOAD( "24j8-0.45", 0x20000, 0x20000, CRC(61138487) SHA1(6d5e3b12acdefb6923aa8ae0704f6c328f4747b3) )

	ROM_REGION( 0x20000, "fgtiles", 0 ) // 8x8
	ROM_LOAD( "24aa-0.58", 0x00000, 0x20000, CRC(cb12ba40) SHA1(2d39f778d9daf0d3606b63975bd6cfc45847a265) )

	ROM_REGION( 0x200000, "sprites", 0 ) // 16x16
	ROM_LOAD( "c951.114",   0x000000, 0x80000, CRC(fa76d1f0) SHA1(f69f8e6d1c5f27b054133e0faa49a8e1a9c391b2) )
	ROM_LOAD( "24j4-0.115", 0x080000, 0x40000, CRC(c4a589a3) SHA1(5511e77c8b381419d7c63971023783c26ef6d94b) )
	ROM_LOAD( "24j5-0.116", 0x0c0000, 0x40000, CRC(d6bca436) SHA1(25857a840b93f7f106a3a5c7dde8e0a732f45013) )
	ROM_LOAD( "c950.117",   0x100000, 0x80000, CRC(cca5703d) SHA1(d10ef7ef1789a4f1a732a7c08ab163ec0d347da1) )
	ROM_LOAD( "24j2-0.118", 0x180000, 0x40000, CRC(dc1b7600) SHA1(bd80d7d4063f2b739ac9420132859c23473d9968) )
	ROM_LOAD( "24j3-0.119", 0x1c0000, 0x40000, CRC(3ba12d43) SHA1(f60d5ff54fdef5a31fe1ee7041dda325ef6649c8) )

	ROM_REGION( 0x80000, "bgtiles", 0 ) // 16x16
	ROM_LOAD( "24j7-0.113", 0x00000, 0x40000, CRC(e0a1909e) SHA1(6ec0db2e0297256d1c6d003a0e5b29236048bd88) )
	ROM_LOAD( "24j6-0.112", 0x40000, 0x40000, CRC(77932ef8) SHA1(a6ee3fc05ca0001d5181b69f2b754170ba7a814a) )
ROM_END

ROM_START( wwfsstaru7 )
	ROM_REGION( 0x40000, "maincpu", 0 ) // 68000
	ROM_LOAD16_BYTE( "24ac-06.34", 0x00000, 0x20000, CRC(924a50e4) SHA1(e163ffc6bada5db0d979523dde77355acedcd456) )
	ROM_LOAD16_BYTE( "24ad-07.35", 0x00001, 0x20000, CRC(9a76a50e) SHA1(adde96956a7602ae1ece797732e8295dc176b071) )

	ROM_REGION( 0x08000, "audiocpu", 0 ) // Z80
	ROM_LOAD( "24ab-0.12", 0x00000, 0x08000, CRC(1e44f8aa) SHA1(e03857d6954e9b9b6073b211e2d6570032af8807) )

	ROM_REGION( 0x40000, "oki", 0 ) // ADPCM samples
	ROM_LOAD( "24a9-0.46", 0x00000, 0x20000, CRC(703ff08f) SHA1(08c4d33208eb4c76c751a1a0fe16a817bdc30820) )
	ROM_LOAD( "24j8-0.45", 0x20000, 0x20000, CRC(61138487) SHA1(6d5e3b12acdefb6923aa8ae0704f6c328f4747b3) )

	ROM_REGION( 0x20000, "fgtiles", 0 ) // 8x8
	ROM_LOAD( "24aa-0.58", 0x00000, 0x20000, CRC(cb12ba40) SHA1(2d39f778d9daf0d3606b63975bd6cfc45847a265) )

	ROM_REGION( 0x200000, "sprites", 0 ) // 16x16
	ROM_LOAD( "c951.114",   0x000000, 0x80000, CRC(fa76d1f0) SHA1(f69f8e6d1c5f27b054133e0faa49a8e1a9c391b2) )
	ROM_LOAD( "24j4-0.115", 0x080000, 0x40000, CRC(c4a589a3) SHA1(5511e77c8b381419d7c63971023783c26ef6d94b) )
	ROM_LOAD( "24j5-0.116", 0x0c0000, 0x40000, CRC(d6bca436) SHA1(25857a840b93f7f106a3a5c7dde8e0a732f45013) )
	ROM_LOAD( "c950.117",   0x100000, 0x80000, CRC(cca5703d) SHA1(d10ef7ef1789a4f1a732a7c08ab163ec0d347da1) )
	ROM_LOAD( "24j2-0.118", 0x180000, 0x40000, CRC(dc1b7600) SHA1(bd80d7d4063f2b739ac9420132859c23473d9968) )
	ROM_LOAD( "24j3-0.119", 0x1c0000, 0x40000, CRC(3ba12d43) SHA1(f60d5ff54fdef5a31fe1ee7041dda325ef6649c8) )

	ROM_REGION( 0x80000, "bgtiles", 0 ) // 16x16
	ROM_LOAD( "24j7-0.113", 0x00000, 0x40000, CRC(e0a1909e) SHA1(6ec0db2e0297256d1c6d003a0e5b29236048bd88) )
	ROM_LOAD( "24j6-0.112", 0x40000, 0x40000, CRC(77932ef8) SHA1(a6ee3fc05ca0001d5181b69f2b754170ba7a814a) )
ROM_END

ROM_START( wwfsstaru6 )
	ROM_REGION( 0x40000, "maincpu", 0 ) // 68000
	ROM_LOAD16_BYTE( "24ac-06.34", 0x00000, 0x20000, CRC(924a50e4) SHA1(e163ffc6bada5db0d979523dde77355acedcd456) )
	ROM_LOAD16_BYTE( "24ad-06.35", 0x00001, 0x20000, CRC(d32eee6d) SHA1(f5b75039118998c0cc60c0d45cb66be23b5f371e) )

	ROM_REGION( 0x08000, "audiocpu", 0 ) // Z80
	ROM_LOAD( "24ab-0.12", 0x00000, 0x08000, CRC(1e44f8aa) SHA1(e03857d6954e9b9b6073b211e2d6570032af8807) )

	ROM_REGION( 0x40000, "oki", 0 ) // ADPCM samples
	ROM_LOAD( "24a9-0.46", 0x00000, 0x20000, CRC(703ff08f) SHA1(08c4d33208eb4c76c751a1a0fe16a817bdc30820) )
	ROM_LOAD( "24j8-0.45", 0x20000, 0x20000, CRC(61138487) SHA1(6d5e3b12acdefb6923aa8ae0704f6c328f4747b3) )

	ROM_REGION( 0x20000, "fgtiles", 0 ) // 8x8
	ROM_LOAD( "24aa-0.58", 0x00000, 0x20000, CRC(cb12ba40) SHA1(2d39f778d9daf0d3606b63975bd6cfc45847a265) )

	ROM_REGION( 0x200000, "sprites", 0 ) // 16x16
	ROM_LOAD( "c951.114",   0x000000, 0x80000, CRC(fa76d1f0) SHA1(f69f8e6d1c5f27b054133e0faa49a8e1a9c391b2) )
	ROM_LOAD( "24j4-0.115", 0x080000, 0x40000, CRC(c4a589a3) SHA1(5511e77c8b381419d7c63971023783c26ef6d94b) )
	ROM_LOAD( "24j5-0.116", 0x0c0000, 0x40000, CRC(d6bca436) SHA1(25857a840b93f7f106a3a5c7dde8e0a732f45013) )
	ROM_LOAD( "c950.117",   0x100000, 0x80000, CRC(cca5703d) SHA1(d10ef7ef1789a4f1a732a7c08ab163ec0d347da1) )
	ROM_LOAD( "24j2-0.118", 0x180000, 0x40000, CRC(dc1b7600) SHA1(bd80d7d4063f2b739ac9420132859c23473d9968) )
	ROM_LOAD( "24j3-0.119", 0x1c0000, 0x40000, CRC(3ba12d43) SHA1(f60d5ff54fdef5a31fe1ee7041dda325ef6649c8) )

	ROM_REGION( 0x80000, "bgtiles", 0 ) // 16x16
	ROM_LOAD( "24j7-0.113", 0x00000, 0x40000, CRC(e0a1909e) SHA1(6ec0db2e0297256d1c6d003a0e5b29236048bd88) )
	ROM_LOAD( "24j6-0.112", 0x40000, 0x40000, CRC(77932ef8) SHA1(a6ee3fc05ca0001d5181b69f2b754170ba7a814a) )
ROM_END

ROM_START( wwfsstaru4 )
	ROM_REGION( 0x40000, "maincpu", 0 ) // 68000
	ROM_LOAD16_BYTE( "24ac-04.34", 0x00000, 0x20000, CRC(ee9b850e) SHA1(6b634ad98b6104b9e860d05e73f3a139c2a19a78) )
	ROM_LOAD16_BYTE( "24ad-04.35", 0x00001, 0x20000, CRC(057c2eef) SHA1(6eb5f60fa51b3e7f17fc6a81182a01ea406febea) )

	ROM_REGION( 0x08000, "audiocpu", 0 ) // Z80
	ROM_LOAD( "24ab-0.12", 0x00000, 0x08000, CRC(1e44f8aa) SHA1(e03857d6954e9b9b6073b211e2d6570032af8807) )

	ROM_REGION( 0x40000, "oki", 0 ) // ADPCM samples
	ROM_LOAD( "24a9-0.46", 0x00000, 0x20000, CRC(703ff08f) SHA1(08c4d33208eb4c76c751a1a0fe16a817bdc30820) )
	ROM_LOAD( "24j8-0.45", 0x20000, 0x20000, CRC(61138487) SHA1(6d5e3b12acdefb6923aa8ae0704f6c328f4747b3) )

	ROM_REGION( 0x20000, "fgtiles", 0 ) // 8x8
	ROM_LOAD( "24aa-0.58", 0x00000, 0x20000, CRC(cb12ba40) SHA1(2d39f778d9daf0d3606b63975bd6cfc45847a265) )

	ROM_REGION( 0x200000, "sprites", 0 ) // 16x16
	ROM_LOAD( "c951.114",   0x000000, 0x80000, CRC(fa76d1f0) SHA1(f69f8e6d1c5f27b054133e0faa49a8e1a9c391b2) )
	ROM_LOAD( "24j4-0.115", 0x080000, 0x40000, CRC(c4a589a3) SHA1(5511e77c8b381419d7c63971023783c26ef6d94b) )
	ROM_LOAD( "24j5-0.116", 0x0c0000, 0x40000, CRC(d6bca436) SHA1(25857a840b93f7f106a3a5c7dde8e0a732f45013) )
	ROM_LOAD( "c950.117",   0x100000, 0x80000, CRC(cca5703d) SHA1(d10ef7ef1789a4f1a732a7c08ab163ec0d347da1) )
	ROM_LOAD( "24j2-0.118", 0x180000, 0x40000, CRC(dc1b7600) SHA1(bd80d7d4063f2b739ac9420132859c23473d9968) )
	ROM_LOAD( "24j3-0.119", 0x1c0000, 0x40000, CRC(3ba12d43) SHA1(f60d5ff54fdef5a31fe1ee7041dda325ef6649c8) )

	ROM_REGION( 0x80000, "bgtiles", 0 ) // 16x16
	ROM_LOAD( "24j7-0.113", 0x00000, 0x40000, CRC(e0a1909e) SHA1(6ec0db2e0297256d1c6d003a0e5b29236048bd88) )
	ROM_LOAD( "24j6-0.112", 0x40000, 0x40000, CRC(77932ef8) SHA1(a6ee3fc05ca0001d5181b69f2b754170ba7a814a) )
ROM_END

ROM_START( wwfsstarj )
	ROM_REGION( 0x40000, "maincpu", 0 ) // 68000
	ROM_LOAD16_BYTE( "24ac-0_j-1_japan.34", 0x00000, 0x20000, CRC(f872e968) SHA1(e52298817348601ed88c369018d3110e467cf602) )
	ROM_LOAD16_BYTE( "24ad-0_j-1_japan.35", 0x00001, 0x20000, CRC(c70bcd23) SHA1(b6128b051b68fcca05da34b42ced01916b18a139) )

	ROM_REGION( 0x08000, "audiocpu", 0 ) // Z80
	ROM_LOAD( "24ab-0.12", 0x00000, 0x08000, CRC(1e44f8aa) SHA1(e03857d6954e9b9b6073b211e2d6570032af8807) )

	ROM_REGION( 0x40000, "oki", 0 ) // ADPCM samples
	ROM_LOAD( "24a9-0.46", 0x00000, 0x20000, CRC(703ff08f) SHA1(08c4d33208eb4c76c751a1a0fe16a817bdc30820) )
	ROM_LOAD( "24j8-0.45", 0x20000, 0x20000, CRC(61138487) SHA1(6d5e3b12acdefb6923aa8ae0704f6c328f4747b3) )

	ROM_REGION( 0x20000, "fgtiles", 0 ) // 8x8
	ROM_LOAD( "24aa-0_j.58", 0x00000, 0x20000, CRC(b9201b36) SHA1(743b86528f6936eb6a4e37d5a23c347ae9d68fa0) ) // Hand written "J" on label

	ROM_REGION( 0x200000, "sprites", 0 ) // 16x16
	ROM_LOAD( "c951.114",   0x000000, 0x80000, CRC(fa76d1f0) SHA1(f69f8e6d1c5f27b054133e0faa49a8e1a9c391b2) )
	ROM_LOAD( "24j4-0.115", 0x080000, 0x40000, CRC(c4a589a3) SHA1(5511e77c8b381419d7c63971023783c26ef6d94b) )
	ROM_LOAD( "24j5-0.116", 0x0c0000, 0x40000, CRC(d6bca436) SHA1(25857a840b93f7f106a3a5c7dde8e0a732f45013) )
	ROM_LOAD( "c950.117",   0x100000, 0x80000, CRC(cca5703d) SHA1(d10ef7ef1789a4f1a732a7c08ab163ec0d347da1) )
	ROM_LOAD( "24j2-0.118", 0x180000, 0x40000, CRC(dc1b7600) SHA1(bd80d7d4063f2b739ac9420132859c23473d9968) )
	ROM_LOAD( "24j3-0.119", 0x1c0000, 0x40000, CRC(3ba12d43) SHA1(f60d5ff54fdef5a31fe1ee7041dda325ef6649c8) )

	ROM_REGION( 0x80000, "bgtiles", 0 ) // 16x16
	ROM_LOAD( "24j7-0.113", 0x00000, 0x40000, CRC(e0a1909e) SHA1(6ec0db2e0297256d1c6d003a0e5b29236048bd88) )
	ROM_LOAD( "24j6-0.112", 0x40000, 0x40000, CRC(77932ef8) SHA1(a6ee3fc05ca0001d5181b69f2b754170ba7a814a) )
ROM_END

ROM_START( wwfsstarc )
	ROM_REGION( 0x40000, "maincpu", 0 ) // 68000
	ROM_LOAD16_BYTE( "24ac-0c.ic34", 0x00000, 0x20000, CRC(3e5eebf3) SHA1(888a7ce95dd38231e5bcd3273b78e2aa0c826a72) )
	ROM_LOAD16_BYTE( "24ad-0c.ic35", 0x00001, 0x20000, CRC(154bf54c) SHA1(1503e0d523e6bfddf7707361a835caa436f3d84c) )

	ROM_REGION( 0x08000, "audiocpu", 0 ) // Z80
	ROM_LOAD( "24ab-0.12", 0x00000, 0x08000, CRC(1e44f8aa) SHA1(e03857d6954e9b9b6073b211e2d6570032af8807) )

	ROM_REGION( 0x40000, "oki", 0 ) // ADPCM samples
	ROM_LOAD( "24a9-0.46", 0x00000, 0x20000, CRC(703ff08f) SHA1(08c4d33208eb4c76c751a1a0fe16a817bdc30820) )
	ROM_LOAD( "24j8-0.45", 0x20000, 0x20000, CRC(61138487) SHA1(6d5e3b12acdefb6923aa8ae0704f6c328f4747b3) )

	ROM_REGION( 0x20000, "fgtiles", 0 ) // 8x8
	ROM_LOAD( "24aa-0.58", 0x00000, 0x20000, CRC(cb12ba40) SHA1(2d39f778d9daf0d3606b63975bd6cfc45847a265) )

	ROM_REGION( 0x200000, "sprites", 0 ) // 16x16
	ROM_LOAD( "c951.114",   0x000000, 0x80000, CRC(fa76d1f0) SHA1(f69f8e6d1c5f27b054133e0faa49a8e1a9c391b2) )
	ROM_LOAD( "24j4-0.115", 0x080000, 0x40000, CRC(c4a589a3) SHA1(5511e77c8b381419d7c63971023783c26ef6d94b) )
	ROM_LOAD( "24j5-0.116", 0x0c0000, 0x40000, CRC(d6bca436) SHA1(25857a840b93f7f106a3a5c7dde8e0a732f45013) )
	ROM_LOAD( "c950.117",   0x100000, 0x80000, CRC(cca5703d) SHA1(d10ef7ef1789a4f1a732a7c08ab163ec0d347da1) )
	ROM_LOAD( "24j2-0.118", 0x180000, 0x40000, CRC(dc1b7600) SHA1(bd80d7d4063f2b739ac9420132859c23473d9968) )
	ROM_LOAD( "24j3-0.119", 0x1c0000, 0x40000, CRC(3ba12d43) SHA1(f60d5ff54fdef5a31fe1ee7041dda325ef6649c8) )

	ROM_REGION( 0x80000, "bgtiles", 0 ) // 16x16
	ROM_LOAD( "24j7-0.113", 0x00000, 0x40000, CRC(e0a1909e) SHA1(6ec0db2e0297256d1c6d003a0e5b29236048bd88) )
	ROM_LOAD( "24j6-0.112", 0x40000, 0x40000, CRC(77932ef8) SHA1(a6ee3fc05ca0001d5181b69f2b754170ba7a814a) )
ROM_END

ROM_START( wwfsstarb )
	ROM_REGION( 0x40000, "maincpu", 0 ) // 68000
	ROM_LOAD16_BYTE( "wwfs08.bin", 0x00000, 0x10000, CRC(621df265) SHA1(eded019352428f2caf1de88eac837beb4eea7562) ) // These 2 == 24ac-04.34
	ROM_LOAD16_BYTE( "wwfs10.bin", 0x20000, 0x10000, CRC(a3382dfe) SHA1(49f78464c51892a84c7f06ce08e900be849fb012) )
	ROM_LOAD16_BYTE( "wwfs07.bin", 0x00001, 0x10000, CRC(369559e6) SHA1(32afd7ea0e0e9e8d5c36e9ef2fb18f7f2cfdcf01) ) // These 2 == 24ad-04.35
	ROM_LOAD16_BYTE( "wwfs09.bin", 0x20001, 0x10000, CRC(8cbcd5aa) SHA1(cb3d7a4a48e4e414da758af248085322b5809914) )

	ROM_REGION( 0x08000, "audiocpu", 0 ) // Z80
	ROM_LOAD( "wwfs01.bin", 0x00000, 0x08000, CRC(1e44f8aa) SHA1(e03857d6954e9b9b6073b211e2d6570032af8807) ) // This == 24ab-0.12

	ROM_REGION( 0x40000, "oki", 0 ) // ADPCM samples
	ROM_LOAD( "wwfs02.bin", 0x00000, 0x10000, CRC(6e63c457) SHA1(9d87345fc55e7af7311974f3890874ebe719aca3) ) // These 2 == 24a9-0.46
	ROM_LOAD( "wwfs04.bin", 0x10000, 0x10000, CRC(d7018a9c) SHA1(7d3a6dd5f70654c8e617d9cba88fcaf1801c4d16) )
	ROM_LOAD( "wwfs03.bin", 0x20000, 0x10000, CRC(8a35a20e) SHA1(3bc1a43f956b6840a4bee9e8fb2a6e3d4ac18f75) ) // These 2 == 24j8-0.44
	ROM_LOAD( "wwfs05.bin", 0x30000, 0x10000, CRC(6df08962) SHA1(e3dec81644fe5867024a2fcf34a67924622f3a5b) )

	ROM_REGION( 0x20000, "fgtiles", 0 ) // 8x8
	ROM_LOAD( "wwfs06.bin", 0x00000, 0x10000, CRC(154ca5ce) SHA1(fc358cd8e1d62c9b299c4261901992d798bf6953) ) // These 2 == 24aa-0.58
	ROM_LOAD( "wwfs11.bin", 0x10000, 0x10000, CRC(3d4684dc) SHA1(f6372d41de9bd7458cbab59f29053325ffdf8d69) )

	ROM_REGION( 0x200000, "sprites", 0 ) // 16x16
	ROM_LOAD( "wwfs39.bin", 0x000000, 0x010000, CRC(d807b09a) SHA1(e5a221ac57e16cb3fb47d986e62f265ebbc5b0e6) ) // Data matches original MASK ROMs 100%
	ROM_LOAD( "wwfs38.bin", 0x010000, 0x010000, CRC(d8ea94d3) SHA1(3a9e200dbcd456364317858e4b5fa6a149cb3c61) )
	ROM_LOAD( "wwfs37.bin", 0x020000, 0x010000, CRC(5e8d7407) SHA1(829cc0c2013138097aa49c9072b87452bf8c8936) )
	ROM_LOAD( "wwfs36.bin", 0x030000, 0x010000, CRC(9005e942) SHA1(d0276419c21b866e17be85382f4e6f3baa4ce40b) )
	ROM_LOAD( "wwfs43.bin", 0x040000, 0x010000, CRC(aafc4a38) SHA1(ac48f13fc4d51e425748190f68b32c099acf532d) )
	ROM_LOAD( "wwfs42.bin", 0x050000, 0x010000, CRC(e48b88fb) SHA1(0fbf9109b86fc6376b8705d28c4c3aeb7fb9cdd8) )
	ROM_LOAD( "wwfs41.bin", 0x060000, 0x010000, CRC(ed7f69d5) SHA1(ae11aad3af43a0e240d17f4db26d68eaae7f1cf0) )
	ROM_LOAD( "wwfs40.bin", 0x070000, 0x010000, CRC(4d75fd89) SHA1(76a1f4a169648e00fcb150157393e3a45613f232) )
	ROM_LOAD( "wwfs19.bin", 0x080000, 0x010000, CRC(7426d444) SHA1(1c1af9492bb711701100bfcecab35f0c38260756) )
	ROM_LOAD( "wwfs18.bin", 0x090000, 0x010000, CRC(af11ad2a) SHA1(4214b16ada1679c6e18c5f2b6e5d6ddb4b731361) )
	ROM_LOAD( "wwfs17.bin", 0x0a0000, 0x010000, CRC(ef12069f) SHA1(5748646c0b0d6e00b6eea26fda3a3699e1553473) )
	ROM_LOAD( "wwfs16.bin", 0x0b0000, 0x010000, CRC(08343e7f) SHA1(2085350e2506cf2d9c7aa74211cca912b36b203d) )
	ROM_LOAD( "wwfs15.bin", 0x0c0000, 0x010000, CRC(aac5a928) SHA1(1298a5d29b388768ed6508522830e02f95fb54fc) )
	ROM_LOAD( "wwfs14.bin", 0x0d0000, 0x010000, CRC(67eb7bea) SHA1(1de39072f96a80a41c383e495bb686adb353586c) )
	ROM_LOAD( "wwfs13.bin", 0x0e0000, 0x010000, CRC(970b6e76) SHA1(c0da2237f759980d2d879c55c6855633c99fc418) )
	ROM_LOAD( "wwfs12.bin", 0x0f0000, 0x010000, CRC(242caff5) SHA1(9e2a836d9c5415c9313e6609a2eebcb661fa0301) )
	ROM_LOAD( "wwfs27.bin", 0x100000, 0x010000, CRC(f3eb8ab9) SHA1(4032f96d9c738706e353af7f00de921c2c1b72be) )
	ROM_LOAD( "wwfs26.bin", 0x110000, 0x010000, CRC(2ca91eaf) SHA1(191512aaf9542cbbd441886455cbfb5e7a0ab5d4) )
	ROM_LOAD( "wwfs25.bin", 0x120000, 0x010000, CRC(bbf69c6a) SHA1(c9502c9f1fa257f506a4aed22c015524a9fca074) )
	ROM_LOAD( "wwfs24.bin", 0x130000, 0x010000, CRC(76b08bcd) SHA1(c60bc47cf172203e570e693244a1c6308fa36f0b) )
	ROM_LOAD( "wwfs23.bin", 0x140000, 0x010000, CRC(681f5b5e) SHA1(17ac4dbfa84f5161f8d1c740ee91ccecf9f83f5f) )
	ROM_LOAD( "wwfs22.bin", 0x150000, 0x010000, CRC(81fe1bf7) SHA1(37102a6d276907bfeaccc81f1d6693e1c1f26cce) )
	ROM_LOAD( "wwfs21.bin", 0x160000, 0x010000, CRC(c52eee5e) SHA1(6bf7c63b3c18487dd7d964fe05cef348c6069775) )
	ROM_LOAD( "wwfs20.bin", 0x170000, 0x010000, CRC(b2a8050e) SHA1(6db9463321973a3141b6ceda35d11f851d0b9e1f) )
	ROM_LOAD( "wwfs35.bin", 0x180000, 0x010000, CRC(9d648d82) SHA1(81be2ca9f8384b29cf6ce9d59dedf8be1f37fd5d) )
	ROM_LOAD( "wwfs34.bin", 0x190000, 0x010000, CRC(742a79db) SHA1(5c2a5b578817ea1ed8b6993a8bc554840d7302a9) )
	ROM_LOAD( "wwfs33.bin", 0x1a0000, 0x010000, CRC(f6923db6) SHA1(5d0aba7f8e3fbde890ef67e91dbdd2bd3e67a23c) )
	ROM_LOAD( "wwfs32.bin", 0x1b0000, 0x010000, CRC(9becd621) SHA1(200c485d4d5acaf55f47d716a0df3218b64f813a) )
	ROM_LOAD( "wwfs31.bin", 0x1c0000, 0x010000, CRC(f94c74d5) SHA1(8f740860562876bd21a47ba8be758ecd6913207c) )
	ROM_LOAD( "wwfs30.bin", 0x1d0000, 0x010000, CRC(94094518) SHA1(e010b211ea9c08a3c1f36a0e04f2c4320acaa2b7) )
	ROM_LOAD( "wwfs29.bin", 0x1e0000, 0x010000, CRC(7b5b9d83) SHA1(e7381e48a3a63f28fc9a997bfda3e612f4fcccf9) )
	ROM_LOAD( "wwfs28.bin", 0x1f0000, 0x010000, CRC(70fda626) SHA1(049ef67f57953266ef2c750f58c0ee9baf963b39) )

	ROM_REGION( 0x80000, "bgtiles", 0 ) // 16x16
	ROM_LOAD( "wwfs51.bin", 0x00000, 0x10000, CRC(51157385) SHA1(fa9f74ace9432d8686402e410cbc03a8c3b86f4d) ) // Data matches original MASK ROMs 100%
	ROM_LOAD( "wwfs50.bin", 0x10000, 0x10000, CRC(7fc79df5) SHA1(c57e8bb55a1d176b9232395207c5a28c622de9a4) )
	ROM_LOAD( "wwfs49.bin", 0x20000, 0x10000, CRC(a14076b0) SHA1(6817f56d2c6e2d596ebc7827d816ad331b425eeb) )
	ROM_LOAD( "wwfs48.bin", 0x30000, 0x10000, CRC(251372fd) SHA1(e6036807c902fb34071da8287dedcef6cadae06a) )
	ROM_LOAD( "wwfs47.bin", 0x40000, 0x10000, CRC(b9edcb64) SHA1(76bb627a1ad49d153f904009d199759e3244f426) ) // redumped in wwfsstarb2; original was bad dump
	ROM_LOAD( "wwfs46.bin", 0x50000, 0x10000, CRC(985e5180) SHA1(9fd8b1ae844a2be465748e3a95ea24aa032e490d) )
	ROM_LOAD( "wwfs45.bin", 0x60000, 0x10000, CRC(b2fad792) SHA1(083977c041c42c50e4f1f7140d97a7b792f768e9) )
	ROM_LOAD( "wwfs44.bin", 0x70000, 0x10000, CRC(4f965fa9) SHA1(4312838e216d2a90fe413d027f46d77c74a0aa07) )
ROM_END

/**
 * WWF Superstars (bootleg with 2xYM2203)
 *
 * This is a three board stack consisting of:
 * - CPU Board, connected to...
 * - Graphics Board, connected to...
 * - Graphics ROM Board
 *
 * This is clearly a bootleg of a bootleg. Many of the existing ROMs match the existing
 * boot romset, but with two differences, that being the Z80 program and one of the
 * graphics ROMs.
 *
 * This bootleg could be called the "ato" bootleg because the word "ato" has been
 * etched into the graphics board and the gfx ROM board.
 *
 * It appears this board was shipped from the factory without any stickers on the
 * EPROMs. Someone had put electrical tape on the CPU board EPROMs, but not on any
 * on the graphics ROM board.
 *
 * The sound program has been hacked to work with three (yes, three) YM2203 chips.
 * A function has been hacked to jump to stub code at $7e00-$7eff, which "adapts"
 * the YM2151 writes to the YM2203s... poorly.
 *
 * ym1 is at $8800,01, and ym2 is at $8802,03. The code attempts to write
 * to a third chip at $8804,05, but it isn't present on the board.
 * The bootleg code also does not correctly check the YM2203 busy state when
 * writing to ym2 (and ym3 if present); those functions are stubbed out (immediate RET).
 * This likely results in missed writes on real hardware.
 *
 * All other ROMs match the US set 100%.
 */
ROM_START( wwfsstarb2 )

	// ROMs are on main board
	// filenames come from silkscreened numbers underneath the IC sockets

	ROM_REGION( 0x40000, "maincpu", 0 ) // 68000
	ROM_LOAD16_BYTE( "28.bin", 0x00000, 0x10000, CRC(621df265) SHA1(eded019352428f2caf1de88eac837beb4eea7562) ) // all match US/existing bootleg
	ROM_LOAD16_BYTE( "29.bin", 0x20000, 0x10000, CRC(a3382dfe) SHA1(49f78464c51892a84c7f06ce08e900be849fb012) )
	ROM_LOAD16_BYTE( "30.bin", 0x00001, 0x10000, CRC(369559e6) SHA1(32afd7ea0e0e9e8d5c36e9ef2fb18f7f2cfdcf01) )
	ROM_LOAD16_BYTE( "31.bin", 0x20001, 0x10000, CRC(8cbcd5aa) SHA1(cb3d7a4a48e4e414da758af248085322b5809914) )

	ROM_REGION( 0x08000, "audiocpu", 0 ) // Z80
	ROM_LOAD( "13.bin", 0x00000, 0x08000, CRC(03571814) SHA1(e13f1e101f83f89277863c9d3f85259c6849b90e) ) // hacked sound ROM

	ROM_REGION( 0x40000, "oki", 0 ) // ADPCM samples
	ROM_LOAD( "51.bin", 0x00000, 0x10000, CRC(6e63c457) SHA1(9d87345fc55e7af7311974f3890874ebe719aca3) )
	ROM_LOAD( "56.bin", 0x10000, 0x10000, CRC(d7018a9c) SHA1(7d3a6dd5f70654c8e617d9cba88fcaf1801c4d16) )
	ROM_LOAD( "50.bin", 0x20000, 0x10000, CRC(8a35a20e) SHA1(3bc1a43f956b6840a4bee9e8fb2a6e3d4ac18f75) )
	ROM_LOAD( "55.bin", 0x30000, 0x10000, CRC(6df08962) SHA1(e3dec81644fe5867024a2fcf34a67924622f3a5b) )

	ROM_REGION( 0x20000, "fgtiles", 0 ) // 8x8
	ROM_LOAD( "54.bin", 0x00000, 0x10000, CRC(154ca5ce) SHA1(fc358cd8e1d62c9b299c4261901992d798bf6953) )
	ROM_LOAD( "75.bin", 0x10000, 0x10000, CRC(3d4684dc) SHA1(f6372d41de9bd7458cbab59f29053325ffdf8d69) )

	// all graphics ROMs past this point are on a separate ROM board.
	// sprites and bgtiles ROMs match US set exactly, but are split into 27512 EPROMs.
	// filenames come from the number silkscreened on the board under the IC socket
	// and the number scratched onto the EPROMs.
	//
	// 34_36.bin was not dumped but should match existing bootleg.
	// silkscreen number guessed but assumed accurate.

	ROM_REGION( 0x200000, "sprites", 0 ) // 16x16
	ROM_LOAD( "1_12.bin",   0x000000, 0x010000, CRC(d807b09a) SHA1(e5a221ac57e16cb3fb47d986e62f265ebbc5b0e6) ) // all of these match US version 100%
	ROM_LOAD( "5_13.bin",   0x010000, 0x010000, CRC(d8ea94d3) SHA1(3a9e200dbcd456364317858e4b5fa6a149cb3c61) )
	ROM_LOAD( "9_14.bin",   0x020000, 0x010000, CRC(5e8d7407) SHA1(829cc0c2013138097aa49c9072b87452bf8c8936) )
	ROM_LOAD( "13_15.bin",  0x030000, 0x010000, CRC(9005e942) SHA1(d0276419c21b866e17be85382f4e6f3baa4ce40b) )
	ROM_LOAD( "2_16.bin",   0x040000, 0x010000, CRC(aafc4a38) SHA1(ac48f13fc4d51e425748190f68b32c099acf532d) )
	ROM_LOAD( "6_17.bin",   0x050000, 0x010000, CRC(e48b88fb) SHA1(0fbf9109b86fc6376b8705d28c4c3aeb7fb9cdd8) )
	ROM_LOAD( "10_18.bin",  0x060000, 0x010000, CRC(ed7f69d5) SHA1(ae11aad3af43a0e240d17f4db26d68eaae7f1cf0) )
	ROM_LOAD( "14_19.bin",  0x070000, 0x010000, CRC(4d75fd89) SHA1(76a1f4a169648e00fcb150157393e3a45613f232) )
	ROM_LOAD( "21_20.bin",  0x080000, 0x010000, CRC(7426d444) SHA1(1c1af9492bb711701100bfcecab35f0c38260756) )
	ROM_LOAD( "24_21.bin",  0x090000, 0x010000, CRC(af11ad2a) SHA1(4214b16ada1679c6e18c5f2b6e5d6ddb4b731361) )
	ROM_LOAD( "27_22.bin",  0x0a0000, 0x010000, CRC(ef12069f) SHA1(5748646c0b0d6e00b6eea26fda3a3699e1553473) )
	ROM_LOAD( "30_23.bin",  0x0b0000, 0x010000, CRC(08343e7f) SHA1(2085350e2506cf2d9c7aa74211cca912b36b203d) )
	ROM_LOAD( "33_24.bin",  0x0c0000, 0x010000, CRC(aac5a928) SHA1(1298a5d29b388768ed6508522830e02f95fb54fc) )
	ROM_LOAD( "36_25.bin",  0x0d0000, 0x010000, CRC(67eb7bea) SHA1(1de39072f96a80a41c383e495bb686adb353586c) )
	ROM_LOAD( "39_26.bin",  0x0e0000, 0x010000, CRC(970b6e76) SHA1(c0da2237f759980d2d879c55c6855633c99fc418) )
	ROM_LOAD( "42_27.bin",  0x0f0000, 0x010000, CRC(242caff5) SHA1(9e2a836d9c5415c9313e6609a2eebcb661fa0301) )
	ROM_LOAD( "22_32.bin",  0x100000, 0x010000, CRC(f3eb8ab9) SHA1(4032f96d9c738706e353af7f00de921c2c1b72be) )
	ROM_LOAD( "25_33.bin",  0x110000, 0x010000, CRC(2ca91eaf) SHA1(191512aaf9542cbbd441886455cbfb5e7a0ab5d4) )
	ROM_LOAD( "28_34.bin",  0x120000, 0x010000, CRC(bbf69c6a) SHA1(c9502c9f1fa257f506a4aed22c015524a9fca074) )
	ROM_LOAD( "31_35.bin",  0x130000, 0x010000, CRC(76b08bcd) SHA1(c60bc47cf172203e570e693244a1c6308fa36f0b) )
	ROM_LOAD( "34_36.bin",  0x140000, 0x010000, CRC(681f5b5e) SHA1(17ac4dbfa84f5161f8d1c740ee91ccecf9f83f5f) ) // contents guessed, might need redump.
	ROM_LOAD( "37_37.bin",  0x150000, 0x010000, CRC(81fe1bf7) SHA1(37102a6d276907bfeaccc81f1d6693e1c1f26cce) )
	ROM_LOAD( "40_38.bin",  0x160000, 0x010000, CRC(c52eee5e) SHA1(6bf7c63b3c18487dd7d964fe05cef348c6069775) )
	ROM_LOAD( "43_39.bin",  0x170000, 0x010000, CRC(b2a8050e) SHA1(6db9463321973a3141b6ceda35d11f851d0b9e1f) )
	ROM_LOAD( "23_44.bin",  0x180000, 0x010000, CRC(9d648d82) SHA1(81be2ca9f8384b29cf6ce9d59dedf8be1f37fd5d) )
	ROM_LOAD( "26_45.bin",  0x190000, 0x010000, CRC(742a79db) SHA1(5c2a5b578817ea1ed8b6993a8bc554840d7302a9) )
	ROM_LOAD( "29_46.bin",  0x1a0000, 0x010000, CRC(f6923db6) SHA1(5d0aba7f8e3fbde890ef67e91dbdd2bd3e67a23c) )
	ROM_LOAD( "32_47.bin",  0x1b0000, 0x010000, CRC(9becd621) SHA1(200c485d4d5acaf55f47d716a0df3218b64f813a) )
	ROM_LOAD( "35_48.bin",  0x1c0000, 0x010000, CRC(f94c74d5) SHA1(8f740860562876bd21a47ba8be758ecd6913207c) )
	ROM_LOAD( "38_49.bin",  0x1d0000, 0x010000, CRC(94094518) SHA1(e010b211ea9c08a3c1f36a0e04f2c4320acaa2b7) )
	ROM_LOAD( "41_50.bin",  0x1e0000, 0x010000, CRC(7b5b9d83) SHA1(e7381e48a3a63f28fc9a997bfda3e612f4fcccf9) )
	ROM_LOAD( "44_51.bin",  0x1f0000, 0x010000, CRC(70fda626) SHA1(049ef67f57953266ef2c750f58c0ee9baf963b39) )

	ROM_REGION( 0x80000, "bgtiles", 0 ) // 16x16
	ROM_LOAD( "4_40.bin",  0x00000, 0x10000, CRC(51157385) SHA1(fa9f74ace9432d8686402e410cbc03a8c3b86f4d) ) // these match US version 100%, too
	ROM_LOAD( "8_41.bin",  0x10000, 0x10000, CRC(7fc79df5) SHA1(c57e8bb55a1d176b9232395207c5a28c622de9a4) )
	ROM_LOAD( "12_42.bin", 0x20000, 0x10000, CRC(a14076b0) SHA1(6817f56d2c6e2d596ebc7827d816ad331b425eeb) )
	ROM_LOAD( "16_43.bin", 0x30000, 0x10000, CRC(251372fd) SHA1(e6036807c902fb34071da8287dedcef6cadae06a) )
	ROM_LOAD( "3_28.bin",  0x40000, 0x10000, CRC(b9edcb64) SHA1(76bb627a1ad49d153f904009d199759e3244f426) )
	ROM_LOAD( "7_29.bin",  0x50000, 0x10000, CRC(985e5180) SHA1(9fd8b1ae844a2be465748e3a95ea24aa032e490d) )
	ROM_LOAD( "11_30.bin", 0x60000, 0x10000, CRC(b2fad792) SHA1(083977c041c42c50e4f1f7140d97a7b792f768e9) )
	ROM_LOAD( "15_31.bin", 0x70000, 0x10000, CRC(4f965fa9) SHA1(4312838e216d2a90fe413d027f46d77c74a0aa07) )
ROM_END

} // anonymous namespace


// There is only 1 ROM difference between US revision 6 & 7.  Rev 7 has a patch to the way the 2nd coin slot works

GAME( 1989, wwfsstar,   0,        wwfsstar,   wwfsstar, wwfsstar_state, empty_init, ROT0, "Technos Japan", "WWF Superstars (Europe)",                MACHINE_SUPPORTS_SAVE )
GAME( 1989, wwfsstaru7, wwfsstar, wwfsstar,   wwfsstar, wwfsstar_state, empty_init, ROT0, "Technos Japan", "WWF Superstars (US revision 7)",         MACHINE_SUPPORTS_SAVE )
GAME( 1989, wwfsstaru6, wwfsstar, wwfsstar,   wwfsstar, wwfsstar_state, empty_init, ROT0, "Technos Japan", "WWF Superstars (US revision 6)",         MACHINE_SUPPORTS_SAVE )
GAME( 1989, wwfsstaru4, wwfsstar, wwfsstar,   wwfsstar, wwfsstar_state, empty_init, ROT0, "Technos Japan", "WWF Superstars (US revision 4)",         MACHINE_SUPPORTS_SAVE )
GAME( 1989, wwfsstarj,  wwfsstar, wwfsstar,   wwfsstar, wwfsstar_state, empty_init, ROT0, "Technos Japan", "WWF Superstars (Japan)",                 MACHINE_SUPPORTS_SAVE )
GAME( 1989, wwfsstarc,  wwfsstar, wwfsstar,   wwfsstar, wwfsstar_state, empty_init, ROT0, "Technos Japan", "WWF Superstars (Canada)",                MACHINE_SUPPORTS_SAVE )
GAME( 1989, wwfsstarb,  wwfsstar, wwfsstar,   wwfsstar, wwfsstar_state, empty_init, ROT0, "bootleg",       "WWF Superstars (bootleg)",               MACHINE_SUPPORTS_SAVE )
GAME( 1989, wwfsstarb2, wwfsstar, wwfsstarb2, wwfsstar, wwfsstar_state, empty_init, ROT0, "bootleg",       "WWF Superstars (bootleg with 2xYM2203)", MACHINE_SUPPORTS_SAVE )
