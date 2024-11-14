// license:BSD-3-Clause
// copyright-holders:Tomasz Slanina, David Haywood
/****************************************************************************************
Chanbara
Data East, 1985

PCB Layout
----------

DE-0207-0
|-----------------------------------------------------------|
|             CP12.17H                     CP00-2.17C       |
|                          2016                       6809  |
|             CP13.15H                     CP01.15C         |
|                          2016                             |
|1  RCDM-I4   CP14.13H                     CP02.14C         |
|8  RCDM-I1                                2016             |
|W  RCDM-I1                                CP03.11C   2016  |
|A  RCDM-I1                         2148   CP04.10C         |
|Y  RCDM-I1                                CP05.9C          |
|   RCDM-I1                2148            CP06.8C          |
|  RM-C2                   2148                   TC15G032AY|
|  PM-C1                            2148   CP07.6C          |
|       DSW1                               CP08.5C          |
|CP15.6K                   DECO                     DECO    |
|CP16.5K                   VSC30           CP09.3C  HMC20   |
|CP17.4K                                   CP10.2C          |
|MB3730  558  558  YM3014  YM2203          CP11.1C   12MHz  |
|-----------------------------------------------------------|
Notes:
      6809   - MC68B09EP, clock 1.500MHz [12/8]
      YM2203 - clock 1.500MHz [12/8]
      VSC30  - clock 3.000MHz [12/4, pin 7), custom DECO DIP40 IC
      HMC20  - DECO HMC20 custom DIP28 IC. Provides many clocks each divided by 2
               (i.e. 12MHz, 6MHz, 3MHz, 1.5MHz, 750kHz etc)
               HSync is generated on pins 12 and 13 with 12/256/3. Actual xtal measures
               11.9931MHz, which accounts for the measured HSync error (12/256/3 = 15.625kHz)
      VSync  - 57.4122Hz
      HSync  - 15.6161kHz

------------------------

 Driver by Tomasz Slanina & David Haywood
 Inputs and Dip Switches by stephh

TODO:
 - Support screen flipping for sprites
 - If you force-scroll an enemy off the screen rather than fight them, you'll get graphical
   corruption (bad sprites) before a new enemy appears, does this happen on the PCB?
 - BGM tempo is incorrect, but clocks are verified above? ( see https://www.youtube.com/watch?v=pW9nhx1hcLM )

****************************************************************************************/

#include "emu.h"
#include "cpu/m6809/m6809.h"
#include "sound/ymopn.h"
#include "emupal.h"
#include "screen.h"
#include "speaker.h"
#include "tilemap.h"


// configurable logging
#define LOG_AYOUTS (1U << 1)

//#define VERBOSE (LOG_GENERAL | LOG_AYOUTS)

#include "logmacro.h"

#define LOGAYOUTS(...) LOGMASKED(LOG_AYOUTS, __VA_ARGS__)

namespace {

class chanbara_state : public driver_device
{
public:
	chanbara_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_videoram(*this, "videoram%u", 1U),
		m_colorram(*this, "colorram%u", 1U),
		m_spriteram(*this, "spriteram"),
		m_rombank(*this, "rombank"),
		m_maincpu(*this, "maincpu"),
		m_gfxdecode(*this, "gfxdecode"),
		m_palette(*this, "palette")
	{ }

	void init_chanbara();

	void chanbara(machine_config &config);

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;
	virtual void video_start() override ATTR_COLD;

private:
	template <uint8_t Which> void videoram_w(offs_t offset, uint8_t data);
	template <uint8_t Which> void colorram_w(offs_t offset, uint8_t data);
	void ay_out_0_w(uint8_t data);
	void ay_out_1_w(uint8_t data);
	TILE_GET_INFO_MEMBER(get_bg_tile_info);
	TILE_GET_INFO_MEMBER(get_bg2_tile_info);
	void palette(palette_device &palette) const;
	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void draw_sprites(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void prg_map(address_map &map) ATTR_COLD;

	// memory pointers
	required_shared_ptr_array<uint8_t, 2> m_videoram;
	required_shared_ptr_array<uint8_t, 2> m_colorram;
	required_shared_ptr<uint8_t> m_spriteram;
	required_memory_bank m_rombank;

	// video-related
	tilemap_t  *m_bg_tilemap[2];
	uint8_t    m_scroll;
	uint8_t    m_scrollhi;

	// devices
	required_device<cpu_device> m_maincpu;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;
};


void chanbara_state::palette(palette_device &palette) const
{
	uint8_t const *const color_prom = memregion("proms")->base();

	for (int i = 0; i < palette.entries(); i++)
	{
		int const red = color_prom[i];
		int const green = color_prom[palette.entries() + i];
		int const blue = color_prom[2 * palette.entries() + i];

		palette.set_pen_color(i, pal4bit(red << 1), pal4bit(green << 1), pal4bit(blue << 1));
	}
}

template <uint8_t Which>
void chanbara_state::videoram_w(offs_t offset, uint8_t data)
{
	m_videoram[Which][offset] = data;
	m_bg_tilemap[Which]->mark_tile_dirty(offset);
}

template <uint8_t Which>
void chanbara_state::colorram_w(offs_t offset, uint8_t data)
{
	m_colorram[Which][offset] = data;
	m_bg_tilemap[Which]->mark_tile_dirty(offset);
}


TILE_GET_INFO_MEMBER(chanbara_state::get_bg_tile_info)
{
	int code = m_videoram[0][tile_index] + ((m_colorram[0][tile_index] & 1) << 8);
	int color = (m_colorram[0][tile_index] >> 1) & 0x1f;
	//int flipy = (m_colorram[0][tile_index]) & 0x01; // not on this layer (although bit is used)

	tileinfo.set(0, code, color, 0);
}

TILE_GET_INFO_MEMBER(chanbara_state::get_bg2_tile_info)
{
	int code = m_videoram[1][tile_index];
	int color = (m_colorram[1][tile_index] >> 1) & 0x1f;
	int flipy = (m_colorram[1][tile_index]) & 0x01;

	tileinfo.set(2, code, color, TILE_FLIPXY(flipy));
}

void chanbara_state::video_start()
{
	m_bg_tilemap[0] = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(chanbara_state::get_bg_tile_info)), TILEMAP_SCAN_ROWS, 8, 8, 32, 32);
	m_bg_tilemap[1] = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(chanbara_state::get_bg2_tile_info)), TILEMAP_SCAN_ROWS, 16, 16, 16, 32);
	m_bg_tilemap[0]->set_transparent_pen(0);
	m_bg_tilemap[1]->set_transparent_pen(0);
}

void chanbara_state::draw_sprites(screen_device &screen, bitmap_ind16& bitmap, const rectangle& cliprect)
{
	for (int offs = 0x80 - 4; offs >= 0x00; offs -= 4)
	{
		int pri_mask = (m_spriteram[offs + 0x80] & 0x80) ? 0xfffc : 0xfffe;
		int attr = m_spriteram[offs + 0];
		int code = m_spriteram[offs + 1];
		int color = m_spriteram[offs + 0x80] & 0x1f;
		int flipx = attr & 4;
		int flipy = attr & 2;
		int sx = 240 - m_spriteram[offs + 3];
		int sy = 232 - m_spriteram[offs + 2];

		sy += 16;

		// could be simplified by rearranging gfx in loading / init
		if (m_spriteram[offs + 0x80] & 0x10) code += 0x200;
		if (m_spriteram[offs + 0x80] & 0x20) code += 0x400;
		if (m_spriteram[offs + 0x80] & 0x40) code += 0x100;

		if (attr & 0x10)
		{
			if (!flipy)
			{
				m_gfxdecode->gfx(1)->prio_transpen(bitmap, cliprect, code, color, flipx, flipy, sx, sy - 16, screen.priority(), pri_mask, 0);
				m_gfxdecode->gfx(1)->prio_transpen(bitmap, cliprect, code + 1, color, flipx, flipy, sx, sy, screen.priority(), pri_mask, 0);
			}
			else
			{
				m_gfxdecode->gfx(1)->prio_transpen(bitmap, cliprect, code, color, flipx, flipy, sx, sy, screen.priority(), pri_mask, 0);
				m_gfxdecode->gfx(1)->prio_transpen(bitmap, cliprect, code + 1, color, flipx, flipy, sx, sy - 16, screen.priority(), pri_mask, 0);
			}
		}
		else
		{
			m_gfxdecode->gfx(1)->prio_transpen(bitmap, cliprect, code, color, flipx, flipy, sx, sy, screen.priority(), pri_mask, 0);
		}
	}
}

uint32_t chanbara_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	screen.priority().fill(0, cliprect);

	m_bg_tilemap[1]->set_scrolly(0, m_scroll | (m_scrollhi << 8));
	m_bg_tilemap[1]->draw(screen, bitmap, cliprect, TILEMAP_DRAW_OPAQUE, 0); // ensure bg pen for each tile gets drawn behind sprites
	m_bg_tilemap[1]->draw(screen, bitmap, cliprect, 0, 1);
	m_bg_tilemap[0]->draw(screen, bitmap, cliprect, 0, 2);

	draw_sprites(screen, bitmap, cliprect);

	return 0;
}

/***************************************************************************/

void chanbara_state::prg_map(address_map &map)
{
	map(0x0000, 0x07ff).ram();
	map(0x0800, 0x0bff).ram().w(FUNC(chanbara_state::videoram_w<0>)).share(m_videoram[0]);
	map(0x0c00, 0x0fff).ram().w(FUNC(chanbara_state::colorram_w<0>)).share(m_colorram[0]);
	map(0x1000, 0x10ff).ram().share(m_spriteram);
	map(0x1800, 0x19ff).ram().w(FUNC(chanbara_state::videoram_w<1>)).share(m_videoram[1]);
	map(0x1a00, 0x1bff).ram().w(FUNC(chanbara_state::colorram_w<1>)).share(m_colorram[1]);
	map(0x2000, 0x2000).portr("DSW1");
	map(0x2001, 0x2001).portr("SYSTEM");
	map(0x2002, 0x2002).portr("P2");
	map(0x2003, 0x2003).portr("P1");
	map(0x3800, 0x3801).rw("ymsnd", FUNC(ym2203_device::read), FUNC(ym2203_device::write));
	map(0x4000, 0x7fff).bankr(m_rombank);
	map(0x8000, 0xffff).rom();
}

/***************************************************************************/

// verified from M6809 code
static INPUT_PORTS_START( chanbara )
	PORT_START ("DSW1")
	PORT_DIPNAME( 0x03, 0x03, DEF_STR( Coin_B ) ) PORT_DIPLOCATION("SW1:1,2")
	PORT_DIPSETTING(    0x00, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x03, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x02, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x01, DEF_STR( 1C_3C ) )
	PORT_DIPNAME( 0x0c, 0x0c, DEF_STR( Coin_A ) ) PORT_DIPLOCATION("SW1:3,4")
	PORT_DIPSETTING(    0x00, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x0c, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x08, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x04, DEF_STR( 1C_3C ) )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Difficulty ) ) PORT_DIPLOCATION("SW1:5")       // code at 0xedc0
	PORT_DIPSETTING(    0x10, DEF_STR( Easy ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Hard ) )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Lives ) ) PORT_DIPLOCATION("SW1:6")
	PORT_DIPSETTING(    0x00, "1" )
	PORT_DIPSETTING(    0x20, "3" )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Bonus_Life ) ) PORT_DIPLOCATION("SW1:7")       // table at 0xc249 (2 * 2 words)
	PORT_DIPSETTING(    0x40, "50k and 70k" )
	PORT_DIPSETTING(    0x00, DEF_STR( None ) )
	PORT_DIPNAME( 0x80, 0x00, DEF_STR( Cabinet ) ) PORT_DIPLOCATION("SW1:8")
	PORT_DIPSETTING(    0x00, DEF_STR( Upright ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Cocktail ) )

	PORT_START ("SYSTEM")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_SERVICE1 )           // same coinage as COIN1
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_VBLANK("screen")

	PORT_START ("P1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICKRIGHT_DOWN )   PORT_4WAY
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICKRIGHT_UP )     PORT_4WAY
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICKRIGHT_LEFT )   PORT_4WAY
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICKRIGHT_RIGHT )  PORT_4WAY
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_JOYSTICKLEFT_DOWN )    PORT_4WAY
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_JOYSTICKLEFT_UP )      PORT_4WAY
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_JOYSTICKLEFT_LEFT )    PORT_4WAY
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_JOYSTICKLEFT_RIGHT )   PORT_4WAY

	PORT_START ("P2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICKRIGHT_DOWN )   PORT_4WAY PORT_COCKTAIL
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICKRIGHT_UP )     PORT_4WAY PORT_COCKTAIL
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICKRIGHT_LEFT )   PORT_4WAY PORT_COCKTAIL
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICKRIGHT_RIGHT )  PORT_4WAY PORT_COCKTAIL
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_JOYSTICKLEFT_DOWN )    PORT_4WAY PORT_COCKTAIL
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_JOYSTICKLEFT_UP )      PORT_4WAY PORT_COCKTAIL
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_JOYSTICKLEFT_LEFT )    PORT_4WAY PORT_COCKTAIL
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_JOYSTICKLEFT_RIGHT )   PORT_4WAY PORT_COCKTAIL
INPUT_PORTS_END

/***************************************************************************/

static const gfx_layout tilelayout =
{
	8,8,    // tile size
	RGN_FRAC(1,2),  // number of tiles
	2,  // bits per pixel
	{ 0, 4 }, // plane offsets
	{ RGN_FRAC(1,2)+0,  RGN_FRAC(1,2)+1, RGN_FRAC(1,2)+2, RGN_FRAC(1,2)+3, 0,1,2,3 }, // x offsets
	{ 0*8,1*8,2*8,3*8, 4*8, 5*8, 6*8, 7*8 }, // y offsets
	8*8 // offset to next tile
};

static const gfx_layout tile16layout =
{
	16,16,  // tile size
	RGN_FRAC(1,4),  // number of tiles
	3,  // bits per pixel
	{ RGN_FRAC(1,2),0,4 }, // plane offsets
	{ 16*8+RGN_FRAC(1,4)+0,16*8+ RGN_FRAC(1,4)+1,16*8+ RGN_FRAC(1,4)+2,16*8+ RGN_FRAC(1,4)+3,
		0,1,2,3,
		RGN_FRAC(1,4)+0,  RGN_FRAC(1,4)+1, RGN_FRAC(1,4)+2, RGN_FRAC(1,4)+3,
		16*8+0, 16*8+1, 16*8+2, 16*8+3,

	}, // x offsets
	{ 0*8,1*8,2*8,3*8, 4*8, 5*8, 6*8, 7*8,8*8,9*8,10*8,11*8,12*8,13*8,14*8,15*8 }, // y offsets
	32*8    // offset to next tile
};


static const gfx_layout spritelayout =
{
	16,16,
	RGN_FRAC(1,3),
	3,
	{  RGN_FRAC(2,3),RGN_FRAC(1,3), 0},
	{ 2*8*8+0,2*8*8+1,2*8*8+2,2*8*8+3,2*8*8+4,2*8*8+5,2*8*8+6,2*8*8+7,
	0,1,2,3,4,5,6,7 },
	{ 0*8,1*8,2*8,3*8,4*8,5*8,6*8,7*8,
	1*8*8+0*8,1*8*8+1*8,1*8*8+2*8,1*8*8+3*8,1*8*8+4*8,1*8*8+5*8,1*8*8+6*8,1*8*8+7*8 },
	16*16
};

static GFXDECODE_START( gfx_chanbara )
	GFXDECODE_ENTRY( "gfx1", 0x00000, tilelayout,   0x40, 32 )
	GFXDECODE_ENTRY( "sprites", 0x00000, spritelayout, 0x80, 16 )
	GFXDECODE_ENTRY( "gfx3", 0x00000, tile16layout, 0, 32 )
GFXDECODE_END

/***************************************************************************/


void chanbara_state::ay_out_0_w(uint8_t data)
{
	LOGAYOUTS("ay_out_0_w %02x\n", data);

	m_scroll = data;
}

void chanbara_state::ay_out_1_w(uint8_t data)
{
	LOGAYOUTS("ay_out_1_w %02x\n", data);

	m_scrollhi = data & 0x01;

	flip_screen_set(data & 0x02);

	m_rombank->set_entry((data & 0x04) >> 2);

	if (data & 0xf8)
		LOGAYOUTS("ay_out_1_w unused bits set %02x\n", data & 0xf8);
}

void chanbara_state::machine_start()
{
	save_item(NAME(m_scroll));
	save_item(NAME(m_scrollhi));
}

void chanbara_state::machine_reset()
{
	m_scroll = 0;
	m_scrollhi = 0;
}

void chanbara_state::chanbara(machine_config &config)
{
	MC6809E(config, m_maincpu, XTAL(12'000'000)/8);
	m_maincpu->set_addrmap(AS_PROGRAM, &chanbara_state::prg_map);


	/* video hardware */
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
//  screen.set_refresh_hz(57.4122);
//  screen.set_vblank_time(ATTOSECONDS_IN_USEC(2500) /* not accurate */);
//  screen.set_size(32*8, 32*8);
//  screen.set_visarea(0, 32*8-1, 2*8, 30*8-1);
	// DECO video CRTC
	screen.set_raw(XTAL(12'000'000)/2,384,0,256,272,16,240);
	screen.set_screen_update(FUNC(chanbara_state::screen_update));
	screen.set_palette(m_palette);

	GFXDECODE(config, m_gfxdecode, m_palette, gfx_chanbara);

	PALETTE(config, m_palette, FUNC(chanbara_state::palette), 256);

	SPEAKER(config, "mono").front_center();

	ym2203_device &ymsnd(YM2203(config, "ymsnd", 12000000/8));
	ymsnd.irq_handler().set_inputline(m_maincpu, 0);
	ymsnd.port_a_write_callback().set(FUNC(chanbara_state::ay_out_0_w));
	ymsnd.port_b_write_callback().set(FUNC(chanbara_state::ay_out_1_w));
	ymsnd.add_route(ALL_OUTPUTS, "mono", 1.0);
}


ROM_START( chanbara )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "cp01.16c",     0x08000, 0x4000, CRC(a0c3c24c) SHA1(8445dc39dd763187a2d66c6165b487f146e7d474))
	ROM_LOAD( "cp00-2.17c",   0x0c000, 0x4000, CRC(a045e463) SHA1(2eb546e16f163be6ed72238f2f0203527a957efd) )

	ROM_REGION( 0x8000, "user1", 0 ) // background data
	ROM_LOAD( "cp02.14c",     0x00000, 0x8000, CRC(c2b66cea) SHA1(f72f57add5f38313a72f5c521dce157edf49f70e) )

	ROM_REGION( 0x02000, "gfx1", 0 ) // text layer
	ROM_LOAD( "cp12.17h",       0x00000, 0x2000, CRC(b87b96de) SHA1(f8bb9f094917df305c4fed071edaa775071e40fd) )

	ROM_REGION( 0x08000, "gfx3", 0 ) // bg layer
	ROM_LOAD( "cp13.15h",       0x00000, 0x4000, CRC(2dc38c3d) SHA1(4bb1335b8285e91b51c28e74d8de11a8d6df0486) )
	// ROM cp14.13h is expanded at 0x4000 - 0x8000

	ROM_REGION( 0x08000, "gfx4", 0 )
	ROM_LOAD( "cp14.13h",       0x00000, 0x2000, CRC(d31db368) SHA1(b62834137bfe4ac2013d2d16b0ead10bf2a2df83) )

	ROM_REGION( 0x30000, "sprites", ROMREGION_ERASE00 )
	ROM_LOAD( "cp05.9c",      0x00000, 0x4000, CRC(df2dc3cb) SHA1(3505042c91566bb09fcd2102fecbe2034551b8eb) )
	ROM_LOAD( "cp04.10c",     0x04000, 0x4000, CRC(f7dce87b) SHA1(129ae41d70d96720e020ec1bc1d3f2d9e87ebf47) )
	ROM_LOAD( "cp03.12c",     0x08000, 0x4000, CRC(dea247fb) SHA1(d54fa30813613ef6c3b5f86b563e9ab618a9f627))

	ROM_LOAD( "cp08.5c",     0x10000, 0x4000, CRC(4cf35192) SHA1(1891dcc412caf72ba5a2ea56c1cab35cb3ae6123) )
	ROM_LOAD( "cp07.6c",     0x14000, 0x4000, CRC(0e3727f2) SHA1(d177651bc20a56f5651ae5ce6f3d3ff7ad0e2053) )
	ROM_LOAD( "cp06.7c",     0x18000, 0x4000,  CRC(2f337c08) SHA1(657ee6776780fa0a979a278ff27a49b459232cad) )

	ROM_LOAD( "cp11.1c",     0x20000, 0x4000, CRC(33e6160a) SHA1(b0171b554825072eebe935d12a6085d158b87bdc) )
	ROM_LOAD( "cp10.2c",     0x24000, 0x4000, CRC(bfa324c0) SHA1(c7ff09bb5f1dd2d3707970fae1fd60b6004250c0) )
	ROM_LOAD( "cp09.4c",     0x28000, 0x4000, CRC(3f58b647) SHA1(4eb212667aedd7c397a4911ac7f1b542c5c0a70d) )

	ROM_REGION( 0x0300, "proms", 0 )
	ROM_LOAD( "cp17.4k", 0x0000, 0x0100, CRC(cf03706e) SHA1(2dd2b29067f418ec590c56a38cc64d09d8dc8e09) ) // red
	ROM_LOAD( "cp16.5k", 0x0100, 0x0100, CRC(5fedc8ba) SHA1(8b685ce71d833fefb3e4502d1dd0cca96ba9162a) ) // green
	ROM_LOAD( "cp15.6k", 0x0200, 0x0100, CRC(655936eb) SHA1(762b419c0571fafd8e1c5e96d0d94999768ba325) ) // blue
ROM_END


void chanbara_state::init_chanbara()
{
	uint8_t *src = memregion("gfx4")->base();
	uint8_t *dst = memregion("gfx3")->base() + 0x4000;
	uint8_t *bg = memregion("user1")->base();

	for (int i = 0; i < 0x1000; i++)
	{
		dst[i + 0x1000] = src[i] & 0xf0;
		dst[i + 0x0000] = (src[i] & 0x0f) << 4;
		dst[i + 0x3000] = src[i + 0x1000] & 0xf0;
		dst[i + 0x2000] = (src[i + 0x1000] & 0x0f) << 4;
	}

	m_rombank->configure_entries(0, 2, &bg[0x0000], 0x4000);
}

} // Anonymous namespace


GAME( 1985, chanbara, 0, chanbara, chanbara, chanbara_state, init_chanbara, ROT270, "Data East Corporation", "Chanbara (Japan)", MACHINE_SUPPORTS_SAVE | MACHINE_NO_COCKTAIL ) // title & flyer suggests "Chan Bara" but it's actually チャンバラ
