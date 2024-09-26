// license:BSD-3-Clause
// copyright-holders:David Haywood
/*

what is this HW cloned from? I doubt it's an original design

Promat / Oriental Soft SPR800E driver

1945K-III (Newer)    2000   Oriental Soft
1945K-III (Older)    1999   Oriental Soft
Solite Spirits       1999   Promat
'96 Flag Rally       1996   Promat???

NOTES
-----
1945K-III is clearly a retooled Solite Spirtis. There are spots in 1945K-III where it shows "SOLITE SPIRITS"
  such as the ship selection screen's background and after the continue countdown expires, but before the high score table.
  It's unknown if the changes were specifically meant for an Export version or if Promat was trying to capitalized
  on Psikyo's Striker 1945 series and it's name.
  Older version of 1945K-III there's no dipswitches for Demo Sound or Allow Continue - Demo sounds always OFF

Solite Spirit:
  Demo loop is extremely short and only shows one area
  The test screen is basically just for sound test (in 1945K-III you get inputs, then press P1 & P2 start to test sounds)
  In 2 player games when selecting ships, "player 2" uses the same "P1" graphic (fixed in 1945K-III)
  No "Lives", Demo Sound or Allow Continue dipswitches - Demo sounds always ON

---------------

1945K-III
Oriental, 2000

This game is a straight rip-off of Psikyo's Strikers 1945 III.

OPCX2 PCB Layout
----------

ORIENTAL SOFT INC., -OPCX2-
|--------------------------------------------|
|    AD-65   SND-1.SU7            M16M-1.U62 |
|                     PAL                    |
|    AD-65   SND-2.SU4                       |
|                                 M16M-2.U63 |
|                                            |
|                    KM681000                |
|J                   KM681000     6116       |
|A                                           |
|M          62256    |-------|    6116       |
|M          62256    |SPR800E|               |
|A                   |OP-CX1 |    6116  6116 |
|    6116   PRG-1.U51|QFP208 |               |
|                    |-------|    6116  6116 |
|    6116   PRG-2.U52                        |
|                 |-----| |------|           |
|           PAL   |     | |QL2003| M16M-3.U61|
|           PAL   |68000| |PLCC84|           |
|DSW1 DSW2        |-----| |------| PAL       |
|             16MHz        27MHz             |
|--------------------------------------------|
Notes:
     68000 clock : 16.000MHz
    M6295 clocks : 1.000MHz (both), sample rate = 1000000 / 132
           VSync : 60Hz



Promat / ORIENTAL SOFT INC., -OPCX1-
|--------------------------------------------|
|    AD-65   SU5                     U5  U58 |
|VR1         SU10*    PAL            U6  U59 |
|    AD-65   SU11*                   U7  U60 |
|            SU4                     U8  U61 |
|                                            |
|J                                      6116 |
|A                                      6116 |
|M 4.000MHz  PAL  KM681000 |-------|    6116 |
|M                         |SPR800E|    6116 |
|A           PAL  KM681000 |OP-CX1 |    6116 |
|                          |QFP208 |    6116 |
|      6116 62256  U34     |-------|         |
|      6116 62256  U35    |------|           |
|                         |QL2003|      U102 |
|             MC68000P16  |PLCC84| PAL  U103 |
|DSW1 DSW2                |------|      U104 |
|RESET1       16.000MHz    27.000MHz    U105 |
|--------------------------------------------|

  CPU: MC86000P16
Sound: OKIM6295 x 2
  OSC: 27.000MHz, 16.000MHz & 4.000MHz
Other: QuickLogic QL2003-XPL84C, SPR800E
       RESET1 Reset button, VR1 volume pot
  DSW: 8 poistion x 2

* denotes unpopulated socket

SPR800E on the 1945K-III OPCX1 PCB silkscreened  ORIENTAL  OP-CX1  SPR800E  9937E
SPR800E on the Solite Spirits PCB silkscreened  PROMAT  SPR800E  ES928
*/

#include "emu.h"
#include "cpu/m68000/m68000.h"
#include "sound/okim6295.h"
#include "emupal.h"
#include "screen.h"
#include "speaker.h"
#include "tilemap.h"


namespace {


class k3_state : public driver_device
{
public:
	k3_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_oki(*this, "oki%u", 1U) ,
		m_spriteram(*this, "spritera%u", 1U),
		m_bgram(*this, "bgram"),
		m_maincpu(*this, "maincpu"),
		m_gfxdecode(*this, "gfxdecode"),
		m_palette(*this, "palette"),
		m_screen(*this, "screen")
	{ }

	void flagrall(machine_config &config);
	void k3(machine_config &config);

private:
	void vram_buffer(bool newval);
	void scrollx_w(u16 data);
	void scrolly_w(u16 data);
	void k3_soundbanks_w(u16 data);
	void flagrall_soundbanks_w(offs_t offset, u16 data, u16 mem_mask = ~0);
	TILE_GET_INFO_MEMBER(get_tile_info);

	virtual void video_start() override ATTR_COLD;

	void k3_drawgfx(bitmap_ind16 &dest_bmp,const rectangle &clip,gfx_element *gfx,
							u32 code,u32 color,bool flipx,bool flipy,int offsx,int offsy,
							u8 transparent_color, bool flicker);
	void draw_sprites(bitmap_ind16 &bitmap, const rectangle &cliprect);
	u32 screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

	void k3_map(address_map &map) ATTR_COLD;
	void flagrall_map(address_map &map) ATTR_COLD;
	void k3_base_map(address_map &map) ATTR_COLD;

	// devices
	optional_device_array<okim6295_device, 2> m_oki;
	// memory pointers
	required_shared_ptr_array<u16, 2> m_spriteram;
	required_shared_ptr<u16> m_bgram;

	// video-related
	std::unique_ptr<u16[]> m_spriteram_buffer[2]{};
	std::unique_ptr<u16[]> m_bgram_buffer{};
	bool m_refresh = false;
	tilemap_t  *m_bg_tilemap = nullptr;
	required_device<cpu_device> m_maincpu;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;
	required_device<screen_device> m_screen;
};


TILE_GET_INFO_MEMBER(k3_state::get_tile_info)
{
	int tileno = m_bgram_buffer[tile_index];
	tileinfo.set(1, tileno, 0, 0);
}

void k3_state::video_start()
{
	m_bg_tilemap = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(k3_state::get_tile_info)), TILEMAP_SCAN_ROWS, 16, 16, 32, 32);
	for (int i = 0; i < 2; i++)
	{
		const u32 size = m_spriteram[i].bytes() / 2;
		m_spriteram_buffer[i] = make_unique_clear<u16[]>(size);
		save_pointer(NAME(m_spriteram_buffer[i]), size, i);
	}
	const u32 size = m_bgram.bytes() / 2;
	m_bgram_buffer = make_unique_clear<u16[]>(size);
	save_pointer(NAME(m_bgram_buffer), size);

	save_item(NAME(m_refresh));
}

void k3_state::k3_drawgfx(bitmap_ind16 &dest_bmp,const rectangle &clip,gfx_element *gfx,
							u32 code,u32 color,bool flipx,bool flipy,int offsx,int offsy,
							u8 transparent_color, bool flicker)
{
	// Start drawing
	const u16 pal = gfx->colorbase() + gfx->granularity() * (color % gfx->colors());
	const u8 *const source_base = gfx->get_data(code % gfx->elements());

	int const xinc = flipx ? -1 : 1;
	int const yinc = flipy ? -1 : 1;

	int x_index_base = flipx ? gfx->width() - 1 : 0;
	int y_index = flipy ? gfx->height() - 1 : 0;

	// start coordinates
	int sx = offsx;
	int sy = offsy;

	// end coordinates
	int ex = sx + gfx->width();
	int ey = sy + gfx->height();

	if (sx < clip.min_x)
	{ // clip left
		int pixels = clip.min_x - sx;
		sx += pixels;
		x_index_base += xinc * pixels;
	}
	if (sy < clip.min_y)
	{ // clip top
		int pixels = clip.min_y - sy;
		sy += pixels;
		y_index += yinc * pixels;
	}
	// NS 980211 - fixed incorrect clipping
	if (ex > clip.max_x + 1)
	{ // clip right
		ex = clip.max_x + 1;
	}
	if (ey > clip.max_y + 1)
	{ // clip bottom
		ey = clip.max_y + 1;
	}

	if (ex > sx)
	{ // skip if inner loop doesn't draw anything
		for (int y = sy; y < ey; y++)
		{
			u8 const *const source = source_base + y_index * gfx->rowbytes();
			u16 *const dest = &dest_bmp.pix(y);
			int x_index = x_index_base;
			for (int x = sx; x < ex; x++)
			{
				u8 const c = source[x_index];
				if (c != transparent_color)
				{
					if (flicker) // verified from PCB (reference : https://www.youtube.com/watch?v=ooXyyvpW1O0)
					{
						dest[x] = pal | 0xff;
					}
					else
					{
						dest[x] = pal | c;
					}
				}
				x_index += xinc;
			}
			y_index += yinc;
		}
	}
}

// sprite limitation reference : https://www.youtube.com/watch?v=_7V_SrEQwSY
// TODO : measure values from real hardware
void k3_state::draw_sprites(bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	const u32 max_cycle = m_screen->width() * m_screen->height(); // max usable cycle for sprites, TODO : related to whole screen?
	u32 cycle = 0;
	gfx_element *gfx = m_gfxdecode->gfx(0);
	const u16 *source = m_spriteram_buffer[0].get();
	const u16 *source2 = m_spriteram_buffer[1].get();
	const u16 *finish = source + (m_spriteram[0].bytes() / 2);

	while (source < finish)
	{
		cycle += 4 + 128; // 4 cycle per reading RAM, 128? cycle per each tiles
		if (cycle > max_cycle)
			break;

		const int xpos = (source[0] & 0xff00) >> 8 | (source2[0] & 0x0001) << 8;
		const int ypos = (source[0] & 0x00ff) >> 0;
		const u32 tileno = (source2[0] & 0x7ffe) >> 1;
		const bool flicker = BIT(source2[0], 15);

		k3_drawgfx(bitmap, cliprect, gfx, tileno, 1, false, false, xpos, ypos, 0, flicker);
		k3_drawgfx(bitmap, cliprect, gfx, tileno, 1, false, false, xpos, ypos - 0x100, 0, flicker); // wrap
		k3_drawgfx(bitmap, cliprect, gfx, tileno, 1, false, false, xpos - 0x200, ypos, 0, flicker); // wrap
		k3_drawgfx(bitmap, cliprect, gfx, tileno, 1, false, false, xpos - 0x200, ypos - 0x100, 0, flicker); // wrap

		source++;
		source2++;
	}
}

u32 k3_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	m_bg_tilemap->draw(screen, bitmap, cliprect, 0, 0);
	draw_sprites(bitmap, cliprect);
	return 0;
}

void k3_state::vram_buffer(bool newval)
{
	const bool oldval = m_refresh;
	if (!oldval && newval)
	{
		for (int j = 0; j < 2; j++)
		{
			const u32 size = m_spriteram[j].bytes() / 2;
			for (int i = 0; i < size; i++)
				m_spriteram_buffer[j][i] = m_spriteram[j][i];
		}
		const u32 size = m_bgram.bytes() / 2;
		for (int i = 0; i < size; i++)
			m_bgram_buffer[i] = m_bgram[i];

		m_bg_tilemap->mark_all_dirty();
	}
	m_refresh = newval;
}

void k3_state::scrollx_w(u16 data)
{
	m_bg_tilemap->set_scrollx(0, data);
}

void k3_state::scrolly_w(u16 data)
{
	m_bg_tilemap->set_scrolly(0, data);
}

void k3_state::k3_soundbanks_w(u16 data)
{
	/*
	    ---- ---- ---x ---- Unknown
	    ---- ---- ---- -x-- OKI1 Bank
	    ---- ---- ---- --x- OKI0 Bank
	    ---- ---- ---- ---x VRAM access flip-flop
	*/
	vram_buffer(BIT(data, 0)); // rising edge
	m_oki[1]->set_rom_bank(BIT(data, 2));
	m_oki[0]->set_rom_bank(BIT(data, 1));
}

void k3_state::flagrall_soundbanks_w(offs_t offset, u16 data, u16 mem_mask)
{
	data &= mem_mask;

	// 0x0200 on startup
	// 0x0100 on startup

	// 0x80 - ?
	// 0x40 - ?
	// 0x20 - VRAM access flip-flop
	// 0x10 - unknown, always on?
	// 0x08 - ?
	// 0x06 - oki bank
	// 0x01 - ?

	vram_buffer(BIT(~data, 5)); // falling edge
	if (data & 0xfcc9)
		popmessage("unk control %04x", data & 0xfcc9);

	m_oki[0]->set_rom_bank((data & 0x6) >> 1);
}


void k3_state::k3_base_map(address_map &map)
{
	map(0x0009ce, 0x0009cf).nopw();    // k3 - bug in code? (clean up log)
	map(0x0009d0, 0x0009d1).nopw();
	map(0x0009d2, 0x0009d3).nopw();    // l3 - bug in code? (clean up log)

	map(0x000000, 0x0fffff).rom(); // ROM
	map(0x100000, 0x10ffff).ram(); // Main Ram
	map(0x200000, 0x2003ff).ram().w(m_palette, FUNC(palette_device::write16)).share("palette");
	map(0x240000, 0x240fff).ram().share(m_spriteram[0]);
	map(0x280000, 0x280fff).ram().share(m_spriteram[1]);
	map(0x2c0000, 0x2c07ff).ram().share(m_bgram);
	map(0x2c0800, 0x2c0fff).ram(); // or does k3 have a bigger tilemap? (flagrall is definitely 32x32 tiles)
	map(0x340000, 0x340001).w(FUNC(k3_state::scrollx_w));
	map(0x380000, 0x380001).w(FUNC(k3_state::scrolly_w));
	map(0x400000, 0x400001).portr("INPUTS");
	map(0x440000, 0x440001).portr("SYSTEM");
	map(0x480000, 0x480001).portr("DSW");
}

void k3_state::k3_map(address_map &map)
{
	k3_base_map(map);

	map(0x3c0000, 0x3c0001).w(FUNC(k3_state::k3_soundbanks_w));

	map(0x4c0001, 0x4c0001).rw(m_oki[0], FUNC(okim6295_device::read), FUNC(okim6295_device::write)).cswidth(16);
	map(0x500001, 0x500001).rw(m_oki[1], FUNC(okim6295_device::read), FUNC(okim6295_device::write)).cswidth(16);
	map(0x8c0000, 0x8cffff).ram(); // not used? (bug in code?)
}


void k3_state::flagrall_map(address_map &map)
{
	k3_base_map(map);

	map(0x3c0000, 0x3c0001).w(FUNC(k3_state::flagrall_soundbanks_w));
	map(0x4c0001, 0x4c0001).rw(m_oki[0], FUNC(okim6295_device::read), FUNC(okim6295_device::write));
}


static INPUT_PORTS_START( k3 )
	PORT_START("INPUTS")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1)
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(1)
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(1)
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_PLAYER(1)
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2)
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(2)
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_PLAYER(2)

	PORT_START("SYSTEM")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0xfff0, IP_ACTIVE_LOW, IPT_UNKNOWN )  // Are these used at all?

	PORT_START("DSW")
	PORT_DIPNAME( 0x007,  0x0007, DEF_STR( Coin_A ) )           PORT_DIPLOCATION("SW1:1,2,3")
	PORT_DIPSETTING(      0x0002, DEF_STR( 5C_1C ) )
	PORT_DIPSETTING(      0x0001, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(      0x0005, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(      0x0006, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(      0x0007, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(      0x0004, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(      0x0003, DEF_STR( Free_Play ) )
	PORT_DIPNAME( 0x0018, 0x0008, DEF_STR( Difficulty ) )       PORT_DIPLOCATION("SW1:4,5")
	PORT_DIPSETTING(      0x0000, DEF_STR( Easy ) )
	PORT_DIPSETTING(      0x0008, DEF_STR( Normal ) )
	PORT_DIPSETTING(      0x0010, DEF_STR( Hard ) )
	PORT_DIPSETTING(      0x0018, DEF_STR( Hardest ) )
	PORT_DIPNAME( 0x0060, 0x0060, DEF_STR( Lives ) )            PORT_DIPLOCATION("SW1:6,7")
	PORT_DIPSETTING(      0x0040, "2" )
	PORT_DIPSETTING(      0x0060, "3" )
	PORT_DIPSETTING(      0x0020, "4" )
	PORT_DIPSETTING(      0x0000, "5" )
	PORT_SERVICE_DIPLOC(  0x0080, IP_ACTIVE_LOW, "SW1:8" )
	PORT_DIPNAME( 0x0100, 0x0000, DEF_STR( Demo_Sounds ) )      PORT_DIPLOCATION("SW2:1")
	PORT_DIPSETTING(      0x0100, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0200, 0x0200, DEF_STR( Allow_Continue ) )   PORT_DIPLOCATION("SW2:2")
	PORT_DIPSETTING(      0x0000, DEF_STR( No ) )
	PORT_DIPSETTING(      0x0200, DEF_STR( Yes ) )
	PORT_DIPUNKNOWN_DIPLOC( 0x0400, 0x0400, "SW2:3" )
	PORT_DIPUNKNOWN_DIPLOC( 0x0800, 0x0800, "SW2:4" )
	PORT_DIPUNKNOWN_DIPLOC( 0x1000, 0x1000, "SW2:5" )
	PORT_DIPUNKNOWN_DIPLOC( 0x2000, 0x2000, "SW2:6" )
	PORT_DIPUNKNOWN_DIPLOC( 0x4000, 0x4000, "SW2:7" )
	PORT_DIPUNKNOWN_DIPLOC( 0x8000, 0x8000, "SW2:8" )
INPUT_PORTS_END


static INPUT_PORTS_START( k3old )
	PORT_START("INPUTS")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1)
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(1)
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(1)
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_PLAYER(1)
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2)
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(2)
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_PLAYER(2)

	PORT_START("SYSTEM")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0xfff0, IP_ACTIVE_LOW, IPT_UNKNOWN )  // Are these used at all?

	PORT_START("DSW")
	PORT_DIPNAME( 0x007,  0x0007, DEF_STR( Coin_A ) )           PORT_DIPLOCATION("SW1:1,2,3")
	PORT_DIPSETTING(      0x0005, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(      0x0006, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(      0x0007, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(      0x0002, DEF_STR( 1C_1C ) )    // 5C_1C in newer versions
	PORT_DIPSETTING(      0x0001, DEF_STR( 1C_1C ) )    // 4C_1C in newer versions
	PORT_DIPSETTING(      0x0000, DEF_STR( 1C_1C ) )    // 1C_3C in newer versions
	PORT_DIPSETTING(      0x0004, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(      0x0003, DEF_STR( Free_Play ) )
	PORT_DIPNAME( 0x0018, 0x0008, DEF_STR( Difficulty ) )       PORT_DIPLOCATION("SW1:4,5")
	PORT_DIPSETTING(      0x0000, DEF_STR( Easy ) )
	PORT_DIPSETTING(      0x0008, DEF_STR( Normal ) )
	PORT_DIPSETTING(      0x0010, DEF_STR( Hard ) )
	PORT_DIPSETTING(      0x0018, DEF_STR( Hardest ) )
	PORT_DIPNAME( 0x0060, 0x0060, DEF_STR( Lives ) )            PORT_DIPLOCATION("SW1:6,7")
	PORT_DIPSETTING(      0x0040, "2" )
	PORT_DIPSETTING(      0x0060, "3" )
	PORT_DIPSETTING(      0x0020, "4" )
	PORT_DIPSETTING(      0x0000, "5" )
	PORT_SERVICE_DIPLOC(  0x0080, IP_ACTIVE_LOW, "SW1:8" )
	PORT_DIPUNKNOWN_DIPLOC( 0x0100, 0x0100, "SW2:1" )   // In newer 1945K-III sets this is Demo_Sounds - Demo Sounds always OFF here!
	PORT_DIPUNKNOWN_DIPLOC( 0x0200, 0x0200, "SW2:2" )   // In newer 1945K-III sets this is Allow_Continue - Always Continue
	PORT_DIPUNKNOWN_DIPLOC( 0x0400, 0x0400, "SW2:3" )
	PORT_DIPUNKNOWN_DIPLOC( 0x0800, 0x0800, "SW2:4" )
	PORT_DIPUNKNOWN_DIPLOC( 0x1000, 0x1000, "SW2:5" )
	PORT_DIPUNKNOWN_DIPLOC( 0x2000, 0x2000, "SW2:6" )
	PORT_DIPUNKNOWN_DIPLOC( 0x4000, 0x4000, "SW2:7" )
	PORT_DIPUNKNOWN_DIPLOC( 0x8000, 0x8000, "SW2:8" )
INPUT_PORTS_END


static INPUT_PORTS_START( solite )
	PORT_START("INPUTS")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1)
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(1)
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(1)
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_PLAYER(1)
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2)
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(2)
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_PLAYER(2)

	PORT_START("SYSTEM")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0xfff0, IP_ACTIVE_LOW, IPT_UNKNOWN )  // Are these used at all?

	PORT_START("DSW")
	PORT_DIPNAME( 0x007,  0x0007, DEF_STR( Coin_A ) )           PORT_DIPLOCATION("SW1:1,2,3")
	PORT_DIPSETTING(      0x0005, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(      0x0006, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(      0x0007, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(      0x0002, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(      0x0001, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(      0x0004, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(      0x0003, DEF_STR( Free_Play ) )
	PORT_DIPNAME( 0x0018, 0x0008, DEF_STR( Difficulty ) )       PORT_DIPLOCATION("SW1:4,5")
	PORT_DIPSETTING(      0x0000, DEF_STR( Easy ) )
	PORT_DIPSETTING(      0x0008, DEF_STR( Normal ) )
	PORT_DIPSETTING(      0x0010, DEF_STR( Hard ) )
	PORT_DIPSETTING(      0x0018, DEF_STR( Hardest ) )
	PORT_DIPUNKNOWN_DIPLOC( 0x0020, 0x0020, "SW1:6" )   // In 1945K-III sets 6&7 are Lives
	PORT_DIPUNKNOWN_DIPLOC( 0x0040, 0x0040, "SW1:7" )   // In 1945K-III sets 6&7 are Lives
	PORT_SERVICE_DIPLOC(  0x0080, IP_ACTIVE_LOW, "SW1:8" )
	PORT_DIPUNKNOWN_DIPLOC( 0x0100, 0x0100, "SW2:1" )   // In newer 1945K-III sets this is Demo_Sounds - Demo Sounds always ON here!
	PORT_DIPUNKNOWN_DIPLOC( 0x0200, 0x0200, "SW2:2" )   // In newer 1945K-III sets this is Allow_Continue - Always Continue
	PORT_DIPUNKNOWN_DIPLOC( 0x0400, 0x0400, "SW2:3" )
	PORT_DIPUNKNOWN_DIPLOC( 0x0800, 0x0800, "SW2:4" )
	PORT_DIPUNKNOWN_DIPLOC( 0x1000, 0x1000, "SW2:5" )
	PORT_DIPUNKNOWN_DIPLOC( 0x2000, 0x2000, "SW2:6" )
	PORT_DIPUNKNOWN_DIPLOC( 0x4000, 0x4000, "SW2:7" )
	PORT_DIPUNKNOWN_DIPLOC( 0x8000, 0x8000, "SW2:8" )
INPUT_PORTS_END


static INPUT_PORTS_START( flagrall )
	PORT_START("INPUTS")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1)
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(1)
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0xff00, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("SYSTEM")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_START1 ) // needed to use the continue feature even if it's not used to start the game
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0xff00, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("DSW")
	PORT_DIPNAME(  0x0003, 0x0003, DEF_STR( Coinage ) ) PORT_DIPLOCATION("SW1:1,2")
	PORT_DIPSETTING(       0x0000, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(       0x0001, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(       0x0003, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(       0x0002, DEF_STR( 1C_2C ) )
	PORT_DIPUNUSED_DIPLOC( 0x0004, IP_ACTIVE_LOW, "SW1:3" )
	PORT_DIPUNUSED_DIPLOC( 0x0008, IP_ACTIVE_LOW, "SW1:4" )
	PORT_DIPNAME(  0x0010, 0x0000, DEF_STR( Demo_Sounds ) )  PORT_DIPLOCATION("SW1:5")
	PORT_DIPSETTING(       0x0010, DEF_STR( Off ) )
	PORT_DIPSETTING(       0x0000, DEF_STR( On ) )
	PORT_DIPNAME(  0x0020, 0x0020, "Dip Control" )  PORT_DIPLOCATION("SW1:6")
	PORT_DIPSETTING(       0x0020, DEF_STR( Off ) )
	PORT_DIPSETTING(       0x0000, DEF_STR( On ) )
	PORT_DIPUNUSED_DIPLOC( 0x0040, IP_ACTIVE_LOW, "SW1:7" )
	PORT_DIPNAME(  0x0080, 0x0080, "Picture Test" )  PORT_DIPLOCATION("SW1:8")
	PORT_DIPSETTING(       0x0080, DEF_STR( Off ) )
	PORT_DIPSETTING(       0x0000, DEF_STR( On ) )

	PORT_DIPNAME(  0x0300, 0x0300, DEF_STR( Lives ) ) PORT_DIPLOCATION("SW2:1,2")
	PORT_DIPSETTING(       0x0200, "1" )
	PORT_DIPSETTING(       0x0100, "2" )
	PORT_DIPSETTING(       0x0300, "3" )
	PORT_DIPSETTING(       0x0000, "5" )
	PORT_DIPNAME(  0x0400, 0x0400, "Bonus Type" )  PORT_DIPLOCATION("SW2:3")
	PORT_DIPSETTING (      0x0400, "0" )
	PORT_DIPSETTING(       0x0000, "1" )
	PORT_DIPUNUSED_DIPLOC( 0x0800, IP_ACTIVE_LOW, "SW2:4" )
	PORT_DIPNAME(  0x3000, 0x3000, DEF_STR( Difficulty ) ) PORT_DIPLOCATION("SW2:5,6")
	PORT_DIPSETTING(       0x0000, DEF_STR( Very_Hard ) )
	PORT_DIPSETTING(       0x1000, DEF_STR( Hard ) )
	PORT_DIPSETTING(       0x2000, DEF_STR( Easy ) )
	PORT_DIPSETTING(       0x3000, DEF_STR( Normal ) )
	PORT_DIPUNUSED_DIPLOC( 0x4000, IP_ACTIVE_LOW, "SW2:7" )
	PORT_DIPNAME(  0x8000, 0x8000, DEF_STR( Free_Play ) ) PORT_DIPLOCATION("SW2:8")
	PORT_DIPSETTING(       0x8000, DEF_STR( Off ) )
	PORT_DIPSETTING(       0x0000, DEF_STR( On ) )
INPUT_PORTS_END


static GFXDECODE_START( gfx_1945kiii )
	GFXDECODE_ENTRY( "sprites", 0, gfx_16x16x8_raw, 0x000, 2 ) // sprites
	GFXDECODE_ENTRY( "tiles",   0, gfx_16x16x8_raw, 0x000, 2 ) // bg tiles
GFXDECODE_END

static constexpr XTAL MASTER_CLOCK = XTAL(16'000'000);


void k3_state::flagrall(machine_config &config)
{
	M68000(config, m_maincpu, MASTER_CLOCK); // ?
	m_maincpu->set_addrmap(AS_PROGRAM, &k3_state::flagrall_map);
	m_maincpu->set_vblank_int("screen", FUNC(k3_state::irq4_line_hold));

	GFXDECODE(config, m_gfxdecode, m_palette, gfx_1945kiii);

	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	m_screen->set_raw(XTAL(27'000'000)/4, 432, 0, 320, 315, 0, 240); // ~49.61Hz, reference : https://www.youtube.com/watch?v=CuGrTiQs4ww
	m_screen->set_screen_update(FUNC(k3_state::screen_update));
	m_screen->set_palette(m_palette);

	PALETTE(config, m_palette).set_format(palette_device::xBGR_555, 0x200);

	SPEAKER(config, "mono").front_center();

	OKIM6295(config, m_oki[0], MASTER_CLOCK/16, okim6295_device::PIN7_HIGH);  // dividers?
	m_oki[0]->add_route(ALL_OUTPUTS, "mono", 1.0);
}


void k3_state::k3(machine_config &config)
{
	flagrall(config);

	m_maincpu->set_addrmap(AS_PROGRAM, &k3_state::k3_map);

	OKIM6295(config, m_oki[1], MASTER_CLOCK/16, okim6295_device::PIN7_HIGH);  // dividers?
	m_oki[1]->add_route(ALL_OUTPUTS, "mono", 1.0);

	m_screen->set_raw(XTAL(27'000'000)/4, 432, 0, 320, 262, 0, 224); // ~60Hz
}



ROM_START( 1945kiii )
	ROM_REGION( 0x100000, "maincpu", 0 ) // 68000 Code
	ROM_LOAD16_BYTE( "prg-1.u51", 0x00001, 0x80000, CRC(6b345f27) SHA1(60867fa0e2ea7ebdd4b8046315ee0c83e5cf0d74) ) // identical halves
	ROM_LOAD16_BYTE( "prg-2.u52", 0x00000, 0x80000, CRC(ce09b98c) SHA1(a06bb712b9cf2249cc535de4055b14a21c68e0c5) ) // identical halves

	ROM_REGION( 0x080000, "oki2", 0 ) // Samples
	ROM_LOAD( "snd-2.su4", 0x00000, 0x80000, CRC(47e3952e) SHA1(d56524621a3f11981e4434e02f5fdb7e89fff0b4) )

	ROM_REGION( 0x080000, "oki1", 0 ) // Samples
	ROM_LOAD( "snd-1.su7", 0x00000, 0x80000, CRC(bbb7f0ff) SHA1(458cf3a0c2d42110bc2427db675226c6b8d30999) )

	ROM_REGION( 0x400000, "sprites", 0 )
	ROM_LOAD32_WORD( "m16m-1.u62", 0x000000, 0x200000, CRC(0b9a6474) SHA1(6110ecb17d0fef25935986af9a251fc6e88e3993) )
	ROM_LOAD32_WORD( "m16m-2.u63", 0x000002, 0x200000, CRC(368a8c2e) SHA1(4b1f360c4a3a86d922035774b2c712be810ec548) )

	ROM_REGION( 0x200000, "tiles", 0 )
	ROM_LOAD( "m16m-3.u61", 0x00000, 0x200000, CRC(32fc80dd) SHA1(bee32493a250e9f21997114bba26b9535b1b636c) )
ROM_END

ROM_START( 1945kiiin )
	ROM_REGION( 0x100000, "maincpu", 0 ) // 68000 Code
	ROM_LOAD16_BYTE( "u34", 0x00001, 0x80000, CRC(d0cf4f03) SHA1(3455927221afae5103c02b12c1b855f416c47e91) ) // 27C040 ROM had no label - identical halves
	ROM_LOAD16_BYTE( "u35", 0x00000, 0x80000, CRC(056c64ed) SHA1(b0eddad9c950676b94316d3aeb32f3ed4b9ade0f) ) // 27C040 ROM had no label - identical halves

	ROM_REGION( 0x080000, "oki2", 0 ) // Samples
	ROM_LOAD( "snd-2.su4", 0x00000, 0x80000, CRC(47e3952e) SHA1(d56524621a3f11981e4434e02f5fdb7e89fff0b4) ) // ROM had no label, but same data as SND-2.SU4

	ROM_REGION( 0x080000, "oki1", 0 ) // Samples
	ROM_LOAD( "snd-1.su7", 0x00000, 0x80000, CRC(bbb7f0ff) SHA1(458cf3a0c2d42110bc2427db675226c6b8d30999) ) // ROM had no label, but same data as SND-1.SU7

	ROM_REGION( 0x400000, "sprites", 0 )
	ROM_LOAD32_BYTE( "u5",  0x000000, 0x080000, CRC(f328f85e) SHA1(fe1e1b86a77a9b6da0f69b20da64e69b874d8ef9) ) // These 4 27C040 ROMs had no label
	ROM_LOAD32_BYTE( "u6",  0x000001, 0x080000, CRC(cfdabf1b) SHA1(9822def10e5213d1b5c86034637481b5349bfb70) )
	ROM_LOAD32_BYTE( "u7",  0x000002, 0x080000, CRC(59a6a944) SHA1(20a109edddd8ab9530b94b3b2d2f8a85af2c08f8) )
	ROM_LOAD32_BYTE( "u8",  0x000003, 0x080000, CRC(59995aaf) SHA1(29c2c638b0dd2bf1e79707ea6f5b38b37f45b822) )

	ROM_LOAD32_BYTE( "u58", 0x200000, 0x080000, CRC(6acf2ce4) SHA1(4b18678a9e03beb24494270d19c57bca32a72592) ) // These 4 27C040  ROMs had no label
	ROM_LOAD32_BYTE( "u59", 0x200001, 0x080000, CRC(ca6ff210) SHA1(d7e476bb41c193654495f5ed6ba39980cb3660bc) )
	ROM_LOAD32_BYTE( "u60", 0x200002, 0x080000, CRC(91eb038a) SHA1(b24082ba1675e87881a321ba87e079a1a027dfa4) )
	ROM_LOAD32_BYTE( "u61", 0x200003, 0x080000, CRC(1b358c6d) SHA1(1abe6422b420fd064a32ed9ca9a28c85996d4e57) )

	ROM_REGION( 0x200000, "tiles", 0 )
	ROM_LOAD32_BYTE( "5.u102", 0x000000, 0x80000, CRC(91b70a6b) SHA1(e53f62212d6e3ab5f892944b1933385a85e0ba8a) ) // These 4 ROMs had no label
	ROM_LOAD32_BYTE( "6.u103", 0x000001, 0x80000, CRC(7b5bfb85) SHA1(ef59d64513c7f7e6ee3dcc9bb7bb0e14a71ca957) ) // Same data as M16M-3.U61, just split up
	ROM_LOAD32_BYTE( "7.u104", 0x000002, 0x80000, CRC(cdafcedf) SHA1(82becd002a16185220131085db6576eb763429c8) )
	ROM_LOAD32_BYTE( "8.u105", 0x000003, 0x80000, CRC(2c3895d5) SHA1(ab5837d996c1bb70071db02f07412c182d7547f8) )
ROM_END

ROM_START( 1945kiiio )
	ROM_REGION( 0x100000, "maincpu", 0 ) // 68000 Code
	ROM_LOAD16_BYTE( "3.u34", 0x00001, 0x80000, CRC(5515baa0) SHA1(6fd4c9b7cc27035d6baaafa73f5f5930bfde62a4) ) // 0x40000 to 0x7FFFF 0x00 padded
	ROM_LOAD16_BYTE( "4.u35", 0x00000, 0x80000, CRC(fd177664) SHA1(0ea1854be8d88577129546a56d13bcdc4739ae52) ) // 0x40000 to 0x7FFFF 0x00 padded

	ROM_REGION( 0x080000, "oki2", 0 ) // Samples
	ROM_LOAD( "s21.su5", 0x00000, 0x80000, CRC(9d96fd55) SHA1(80025cc2c44e8cd938620818e0b0974026377f5c) )

	ROM_REGION( 0x080000, "oki1", 0 ) // Samples
	ROM_LOAD( "s13.su4", 0x00000, 0x80000, CRC(d45aec3b) SHA1(fc182a10e19687eb2f2f4a1d2ad976814185f0fc) )

	ROM_REGION( 0x400000, "sprites", 0 )
	ROM_LOAD32_BYTE( "9.u5",   0x000000, 0x080000, CRC(be0f432e) SHA1(7d63f97a8cb38c5351f2cd2f720de16a0c4ab1d7) )
	ROM_LOAD32_BYTE( "10.u6",  0x000001, 0x080000, CRC(cf9127b2) SHA1(e02f436662f47d8bb5a9d726889c6e86cf64bdcf) )
	ROM_LOAD32_BYTE( "11.u7",  0x000002, 0x080000, CRC(644ee8cc) SHA1(1742e31ba48a93c005cce0dc575d9b5d739d1dce) )
	ROM_LOAD32_BYTE( "12.u8",  0x000003, 0x080000, CRC(0900c208) SHA1(9446382d274dc7b6ccdf18738aa4db636fd9e3c9) )

	ROM_LOAD32_BYTE( "13.u58", 0x200000, 0x080000, CRC(8ea9c6be) SHA1(baf3af389417e1f14d0c38d8c872839a54008909) )
	ROM_LOAD32_BYTE( "14.u59", 0x200001, 0x080000, CRC(10c18fb4) SHA1(68934e73cfb6a49a4c1639dcb4c49246f16179b2) )
	ROM_LOAD32_BYTE( "15.u60", 0x200002, 0x080000, CRC(86ab6c7c) SHA1(59acaee6ba78a22f1423832a116ad41e19522aa1) )
	ROM_LOAD32_BYTE( "16.u61", 0x200003, 0x080000, CRC(ff419080) SHA1(542819bdd60976bddfa96570321ba3f7fb6fbf23) )

	ROM_REGION( 0x200000, "tiles", 0 )
	ROM_LOAD32_BYTE( "5.u102", 0x000000, 0x80000, CRC(91b70a6b) SHA1(e53f62212d6e3ab5f892944b1933385a85e0ba8a) ) // Same data as M16M-3.U61, just split up
	ROM_LOAD32_BYTE( "6.u103", 0x000001, 0x80000, CRC(7b5bfb85) SHA1(ef59d64513c7f7e6ee3dcc9bb7bb0e14a71ca957) )
	ROM_LOAD32_BYTE( "7.u104", 0x000002, 0x80000, CRC(cdafcedf) SHA1(82becd002a16185220131085db6576eb763429c8) )
	ROM_LOAD32_BYTE( "8.u105", 0x000003, 0x80000, CRC(2c3895d5) SHA1(ab5837d996c1bb70071db02f07412c182d7547f8) )
ROM_END

ROM_START( slspirit )
	ROM_REGION( 0x100000, "maincpu", 0 ) // 68000 Code
	ROM_LOAD16_BYTE( "3.u34", 0x00001, 0x40000, CRC(b5ac3272) SHA1(da223ef3b006be03a11559e00ae9d7bbd2d06ee5) )
	ROM_LOAD16_BYTE( "4.u35", 0x00000, 0x40000, CRC(86397bcb) SHA1(562dbb82c363039152aa574df72f73e9f71805d9) )

	ROM_REGION( 0x080000, "oki2", 0 ) // Samples
	ROM_LOAD( "s21.su5", 0x00000, 0x80000, CRC(d4cbc27c) SHA1(feffa530baa4d70788a3598a12761827694a6275) )

	ROM_REGION( 0x080000, "oki1", 0 ) // Samples
	ROM_LOAD( "s13.su4", 0x00000, 0x80000, CRC(d9c63d55) SHA1(7fd2fc08c859947dd4b1490132597ae23fcbed36) )

	ROM_REGION( 0x400000, "sprites", 0 )
	ROM_LOAD32_BYTE( "9.u5",   0x000000, 0x080000, CRC(4742aa38) SHA1(6e8d53afe7a6a5d60c135dd6a283d5bb47821f48) )
	ROM_LOAD32_BYTE( "10.u6",  0x000001, 0x080000, CRC(c137fb33) SHA1(6798bc4569bdcab02c2b16315c8827268e5674eb) )
	ROM_LOAD32_BYTE( "11.u7",  0x000002, 0x080000, CRC(d0593a03) SHA1(544e345e0849239b8156df8c50568bb2e2685bd3) )
	ROM_LOAD32_BYTE( "12.u8",  0x000003, 0x080000, CRC(baa9eeb1) SHA1(12d905143c707bc0ff6997b89816b7bce40bd9aa) )

	ROM_LOAD32_BYTE( "13.u58", 0x200000, 0x080000, CRC(eb586bd6) SHA1(1050021a663421be455c215cb9e724463e1fc425) )
	ROM_LOAD32_BYTE( "14.u59", 0x200001, 0x080000, CRC(abc8b869) SHA1(aea3e2bb447b6b9ac0dd19cac7922cc9bee6afb8) )
	ROM_LOAD32_BYTE( "15.u60", 0x200002, 0x080000, CRC(31f9b034) SHA1(d3224d9f11236fcaa65d477b87c46bea8a69db01) )
	ROM_LOAD32_BYTE( "16.u61", 0x200003, 0x080000, CRC(c1fc95e5) SHA1(2597e8553deb751d1a199c4eb3f321f0564b9c76) )

	ROM_REGION( 0x200000, "tiles", 0 )
	ROM_LOAD32_BYTE( "5.u102", 0x000000, 0x80000, CRC(9f0855a6) SHA1(13aa54641eb188f604cf32bb462331fab4c1bf68) )
	ROM_LOAD32_BYTE( "6.u103", 0x000001, 0x80000, CRC(0dda4489) SHA1(cfad31e58adf5aea51c528fb5d7a2f076e6cf0bf) )
	ROM_LOAD32_BYTE( "7.u104", 0x000002, 0x80000, CRC(6208cdc7) SHA1(242a93db057b6c01f0031cc8fd33da76baf6c879) )
	ROM_LOAD32_BYTE( "8.u105", 0x000003, 0x80000, CRC(eb5f67a2) SHA1(4f6bf3fde6df4bc878b882f4f52805963352ef35) )
ROM_END


ROM_START( flagrall )
	ROM_REGION( 0x100000, "maincpu", 0 ) // 68000 Code
	ROM_LOAD16_BYTE( "11_u34.bin", 0x00001, 0x40000, CRC(24dd439d) SHA1(88857ad5ed69f29de86702dcc746d35b69b3b93d) )
	ROM_LOAD16_BYTE( "12_u35.bin", 0x00000, 0x40000, CRC(373b71a5) SHA1(be9ab93129e2ffd9bfe296c341dbdf47f1949ac7) )

	ROM_REGION( 0x100000, "oki1", 0 ) // Samples
	// 3x banks
	ROM_LOAD( "13_su4.bin", 0x00000, 0x80000, CRC(7b0630b3) SHA1(c615e6630ffd12c122762751c25c249393bf7abd) )
	ROM_LOAD( "14_su6.bin", 0x80000, 0x40000, CRC(593b038f) SHA1(b00dcf321fe541ee52c34b79e69c44f3d7a9cd7c) )

	ROM_REGION( 0x300000, "sprites", 0 )
	ROM_LOAD32_BYTE( "1_u5.bin",  0x000000, 0x080000, CRC(9377704b) SHA1(ac516a8ba6d1a70086469504c2a46d47a1f4560b) )
	ROM_LOAD32_BYTE( "5_u6.bin",  0x000001, 0x080000, CRC(1ac0bd0c) SHA1(ab71bb84e61f5c7168601695f332a8d4a30d9948) )
	ROM_LOAD32_BYTE( "2_u7.bin",  0x000002, 0x080000, CRC(5f6db2b3) SHA1(84caa019d3b75be30a14d19ccc2f28e5e94028bd) )
	ROM_LOAD32_BYTE( "6_u8.bin",  0x000003, 0x080000, CRC(79e4643c) SHA1(274f2741f39c63e32f49c6a1a72ded1263bdcdaa) )

	ROM_LOAD32_BYTE( "3_u58.bin", 0x200000, 0x040000, CRC(c913df7d) SHA1(96e89ecb9e5f4d596d71d7ba35af7b2af4670342) )
	ROM_LOAD32_BYTE( "4_u59.bin", 0x200001, 0x040000, CRC(cb192384) SHA1(329b4c1a4dc388d9f4ce063f9a54cbf3b967682a) )
	ROM_LOAD32_BYTE( "7_u60.bin", 0x200002, 0x040000, CRC(f187a7bf) SHA1(f4ce9ac9fe376250fe426de6ee404fc7841ef08a) )
	ROM_LOAD32_BYTE( "8_u61.bin", 0x200003, 0x040000, CRC(b73fa441) SHA1(a5a3533563070c870276ead5e2f9cb9aaba303cc))

	ROM_REGION( 0x100000, "tiles", 0 )
	ROM_LOAD( "10_u102.bin", 0x00000, 0x80000, CRC(b1fd3279) SHA1(4a75581e13d43bef441ce81eae518c2f6bc1d5f8) )
	ROM_LOAD( "9_u103.bin",  0x80000, 0x80000, CRC(01e6d654) SHA1(821d61a5b16f5cb76e2a805c8504db1ef38c3a48) )
ROM_END

} // anonymous namespace


GAME( 2000, 1945kiii,  0,        k3,       k3,       k3_state, empty_init, ROT270, "Oriental Soft", "1945k III (newer, OPCX2 PCB)", MACHINE_SUPPORTS_SAVE )
GAME( 2000, 1945kiiin, 1945kiii, k3,       k3,       k3_state, empty_init, ROT270, "Oriental Soft", "1945k III (newer, OPCX1 PCB)", MACHINE_SUPPORTS_SAVE )
GAME( 1999, 1945kiiio, 1945kiii, k3,       k3old,    k3_state, empty_init, ROT270, "Oriental Soft", "1945k III (older, OPCX1 PCB)", MACHINE_SUPPORTS_SAVE )
GAME( 1999, slspirit,  0,        k3,       solite,   k3_state, empty_init, ROT270, "Promat",        "Solite Spirits",               MACHINE_SUPPORTS_SAVE )

GAME( 1996, flagrall,  0,        flagrall, flagrall, k3_state, empty_init, ROT0,   "Promat?",       "'96 Flag Rally",               MACHINE_SUPPORTS_SAVE )
