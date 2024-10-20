// license:BSD-3-Clause
// copyright-holders:Uki, David Haywood
/*****************************************************************************

Himeshikibu (C) 1989 Hi-Soft
Android (C) 1987? Nasco

    Driver by Uki

*****************************************************************************/

// Android uses PCBS MK-P102 and MK-P101 ONLY, there is no MK-P103 (extra sprites used on Himeshikibu)
// Real hardware video of parent set can be seen at https://www.youtube.com/watch?v=5rtqZqMBACI (uploaded by Chris Hardy)
// for some reason music fails to play the 2nd attract loop in MAME?



/*
Himeshikibu
(c)1989 Hi-Soft (distributed by Rollertron)

CPU: Z80x2
XTAL1: 8.000MHz
Sound: YM2203C Y3014B


MK-P102 (top board)

  -----------CN1-----------  -----------CN2-----------
A
B
C                                       TMM2018D-45 8         |
D MB8416A-15                    (27512)4            2         |
E MB8416A-15                    (27512)3            5         J
                                                    5         A
F (EPL10P8)I                                                  M
G (27256)2                                          8 DSW1    M
J             (EPL10P8)M                            2         A
K (27256)1                                          5 DSW2    |
                                                    5         |

L M5M5165P-12L    Z                 Z               O
M                 8                 8               P
N 74HC132         0                 0   MB8416A-15  N
         BATTERY           XTAL1         (27256)5      3014B
 1         2      3          4      5        6      7    8


ET-P103A (middle board)

  -----------CN4-----------  -----------CN3-----------
  -----------CN2-----------  -----------CN1-----------
12
11
10       M5M5117          M5M5117
 9                                  (EMPTY)   (EMPTY)
 8                                  (EMPTY)   (EMPTY)
 7                                  (EMPTY)   (EMPTY)
 6                                  (EMPTY)   (EMPTY)
 5                                  (EMPTY)   (EMPTY)
 4                                  (27512)11 (27512)10
 3                                  (27512)9  (27512)8
 2                                  (27512)7  (27512)6
 1
    J    H    G     F        E   D     C     B     A


MK-P101 (bottom board)
  -----------CN2-----------  -----------CN1-----------
L
K     MB8416A-15
J     MB8416A-15
H
G
F 2   2
  7   7
  5   5
E 1   1
  2   2
 (13)(15)
D              TMM2018D-45
  2   2
  7   7
C 5   5        TMM2018D-45
  1   1
  2   2
B(12)(14)

A                                                   12.000MHz
  9   8    7        6       5      4      3      2      1

*/

#include "emu.h"

#include "cpu/z80/z80.h"
#include "machine/gen_latch.h"
#include "machine/i8255.h"
#include "sound/ymopn.h"

#include "emupal.h"
#include "screen.h"
#include "speaker.h"
#include "tilemap.h"


namespace {

class himesiki_state : public driver_device
{
public:
	himesiki_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_bg_ram(*this, "bg_ram"),
		m_spriteram_p103a(*this, "sprram_p103a"),
		m_spriteram(*this, "spriteram"),
		m_mainbank(*this, "mainbank"),
		m_maincpu(*this, "maincpu"),
		m_subcpu(*this, "sub"),
		m_gfxdecode(*this, "gfxdecode"),
		m_palette(*this, "palette"),
		m_soundlatch(*this, "soundlatch")
	{ }

	void himesiki(machine_config &config);

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;
	virtual void video_start() override ATTR_COLD;

private:
	// memory pointers
	required_shared_ptr<uint8_t> m_bg_ram;
	required_shared_ptr<uint8_t> m_spriteram_p103a;
	required_shared_ptr<uint8_t> m_spriteram;
	required_memory_bank m_mainbank;

	// video-related
	tilemap_t *m_bg_tilemap = nullptr;
	uint8_t m_scrollx[2]{};
	uint8_t m_scrolly = 0;
	uint8_t m_flipscreen = 0;

	// devices
	required_device<cpu_device> m_maincpu;
	required_device<cpu_device> m_subcpu;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;
	required_device<generic_latch_8_device> m_soundlatch;

	void rombank_w(uint8_t data);
	void sound_w(uint8_t data);
	void bg_ram_w(offs_t offset, uint8_t data);
	void scrollx_w(offs_t offset, uint8_t data);
	void scrolly_w(uint8_t data);
	TILE_GET_INFO_MEMBER(get_bg_tile_info);
	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void draw_sprites(bitmap_ind16 &bitmap, const rectangle &cliprect);
	void main_io_map(address_map &map) ATTR_COLD;
	void sound_io_map(address_map &map) ATTR_COLD;
	void main_prg_map(address_map &map) ATTR_COLD;
	void sound_prg_map(address_map &map) ATTR_COLD;
};


TILE_GET_INFO_MEMBER(himesiki_state::get_bg_tile_info)
{
	int code = m_bg_ram[tile_index * 2] + m_bg_ram[tile_index * 2 + 1] * 0x100;
	int col = code >> 12;

	code &= 0xfff;

	tileinfo.set(0, code, col, 0);
}

void himesiki_state::video_start()
{
	m_bg_tilemap = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(himesiki_state::get_bg_tile_info)), TILEMAP_SCAN_ROWS, 8, 8, 64, 32);
}

void himesiki_state::bg_ram_w(offs_t offset, uint8_t data)
{
	m_bg_ram[offset] = data;
	m_bg_tilemap->mark_tile_dirty(offset / 2);
}

void himesiki_state::scrollx_w(offs_t offset, uint8_t data)
{
	m_scrollx[offset] = data;
}

void himesiki_state::scrolly_w(uint8_t data)
{
	m_scrolly = data;
}

void himesiki_state::draw_sprites(bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	// these sprites are from the ET-P103A board (himesiki only)
	for (int offs = 0x00; offs < 0x60; offs += 4)
	{
		int attr = m_spriteram_p103a[offs + 1];
		int code = m_spriteram_p103a[offs + 0] | (attr & 3) << 8;
		int x = m_spriteram_p103a[offs + 3] | (attr & 8) << 5;
		int y = m_spriteram_p103a[offs + 2];

		int col = (attr & 0xf0) >> 4;
		int fx = attr & 4;
		int fy = 0;

		if (x > 0x1e0)
			x -= 0x200;

		if (m_flipscreen)
		{
			y = (y - 31) & 0xff;
			x = 224 - x;
			fx ^= 4;
			fy = 1;
		}
		else
		{
			y = 257 - y;
			if (y > 0xc0)
				y -= 0x100;
		}

		m_gfxdecode->gfx(2)->transpen(bitmap, cliprect, code, col, fx, fy, x, y, 15);
	}

	// 0xc0 - 0xff unused
	for (int offs = 0; offs < 0x100; offs += 4)
	{
		// not sure about this, but you sometimes get a garbage sprite in the corner otherwise.
		if ((m_spriteram[offs + 0] == 0x00) &&
			(m_spriteram[offs + 1] == 0x00) &&
			(m_spriteram[offs + 2] == 0x00) &&
			(m_spriteram[offs + 3] == 0x00))
				continue;

		int attr = m_spriteram[offs + 1];
		int code = m_spriteram[offs + 0] | (attr & 7) << 8;
		int x = m_spriteram[offs + 3] | (attr & 8) << 5;
		int y = m_spriteram[offs + 2];

		int col = (attr & 0xf0) >> 4;
		int f = 0;

		if (x > 0x1e0)
			x -= 0x200;

		if (m_flipscreen)
		{
			y = (y - 15) & 0xff;
			x = 240 - x;
			f = 1;
		}
		else
			y = 257 - y;

		y &= 0xff;
		if (y > 0xf0)
			y -= 0x100;

		m_gfxdecode->gfx(1)->transpen(bitmap, cliprect, code, col, f, f, x, y, 15);
	}
}

uint32_t himesiki_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	int x = -(m_scrollx[0] << 8 | m_scrollx[1]) & 0x1ff;
	m_bg_tilemap->set_scrolldx(x, x);
	m_bg_tilemap->set_scrolldy(-m_scrolly, -m_scrolly - 64);

	m_bg_tilemap->draw(screen, bitmap, cliprect, TILEMAP_DRAW_OPAQUE, 0);
	draw_sprites(bitmap, cliprect);

	return 0;
}


void himesiki_state::rombank_w(uint8_t data)
{
	m_mainbank->set_entry(((data & 0x0c) >> 2));

	m_flipscreen = (data & 0x10) >> 4;
	flip_screen_set(m_flipscreen);

	if (data & 0xe3)
		logerror("p06_w %02x\n", data);
}

void himesiki_state::sound_w(uint8_t data)
{
	m_soundlatch->write(data);
	m_subcpu->pulse_input_line(INPUT_LINE_NMI, attotime::zero);
}

/****************************************************************************/

void himesiki_state::main_prg_map(address_map &map)
{
	map(0x0000, 0x7fff).rom();
	map(0x8000, 0x9fff).ram();
	map(0xa000, 0xa0ff).ram().share(m_spriteram);
	map(0xa100, 0xa7ff).ram().share(m_spriteram_p103a); // not on Android
	map(0xa800, 0xafff).ram().w(m_palette, FUNC(palette_device::write8)).share("palette");
	map(0xb000, 0xbfff).ram().w(FUNC(himesiki_state::bg_ram_w)).share(m_bg_ram);
	map(0xc000, 0xffff).bankr(m_mainbank);
}

void himesiki_state::main_io_map(address_map &map)
{
	map.global_mask(0xff);
	map(0x00, 0x03).rw("ppi8255_0", FUNC(i8255_device::read), FUNC(i8255_device::write)); // inputs
	map(0x04, 0x07).rw("ppi8255_1", FUNC(i8255_device::read), FUNC(i8255_device::write)); // dips + rombank
	map(0x08, 0x08).w(FUNC(himesiki_state::scrolly_w));
	map(0x09, 0x0a).w(FUNC(himesiki_state::scrollx_w));
	map(0x0b, 0x0b).w(FUNC(himesiki_state::sound_w));
}




void himesiki_state::sound_prg_map(address_map &map)
{
	map(0x0000, 0x7fff).rom();
	map(0xf800, 0xffff).ram();
}

void himesiki_state::sound_io_map(address_map &map)
{
	map.global_mask(0xff);
	map(0x00, 0x01).rw("ym2203", FUNC(ym2203_device::read), FUNC(ym2203_device::write));
	map(0x04, 0x04).r(m_soundlatch, FUNC(generic_latch_8_device::read));
}

/****************************************************************************/

static INPUT_PORTS_START( himesiki )
	PORT_START("DSW1")
	PORT_DIPNAME( 0x01, 0x00, DEF_STR( Demo_Sounds ) )  PORT_DIPLOCATION("SW1:1")
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, "1-2" )                   PORT_DIPLOCATION("SW1:2")
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, "1-3" )                   PORT_DIPLOCATION("SW1:3")
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Cabinet ) )      PORT_DIPLOCATION("SW1:4")
	PORT_DIPSETTING(    0x08, DEF_STR( Upright ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Cocktail ) )
	PORT_DIPNAME( 0x10, 0x10, "1-5" )                   PORT_DIPLOCATION("SW1:5")
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, "1-6" )                   PORT_DIPLOCATION("SW1:6")
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Flip_Screen ) )  PORT_DIPLOCATION("SW1:7")
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_SERVICE_DIPLOC(0x80, IP_ACTIVE_LOW, "SW1:8" )

	PORT_START("DSW2")
	PORT_DIPNAME( 0x01, 0x01, "2-1" )                   PORT_DIPLOCATION("SW2:1")
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, "2-2" )                   PORT_DIPLOCATION("SW2:2")
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, "2-3" )                   PORT_DIPLOCATION("SW2:3")
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, "2-4" )                   PORT_DIPLOCATION("SW2:4")
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, "2-5" )                   PORT_DIPLOCATION("SW2:5")
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, "2-6" )                   PORT_DIPLOCATION("SW2:6")
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, "2-7" )                   PORT_DIPLOCATION("SW2:7")
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, "2-8" )                   PORT_DIPLOCATION("SW2:8")
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("1P")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_UP )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON2 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_BUTTON3 )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("2P")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_UP )    PORT_PLAYER(2)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN )  PORT_PLAYER(2)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT )  PORT_PLAYER(2)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(2)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 )        PORT_PLAYER(2)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON2 )        PORT_PLAYER(2)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_BUTTON3 )        PORT_PLAYER(2)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("OTHERS")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_SERVICE1 ) // coin?
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_COIN2 ) // service?
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )
INPUT_PORTS_END

static INPUT_PORTS_START( androidpo )
	PORT_START("DSW1")
	PORT_DIPNAME( 0x01, 0x01, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x0c, 0x08, DEF_STR( Lives ) )
	PORT_DIPSETTING(    0x0c, "1" )
	PORT_DIPSETTING(    0x04, "2" )
	PORT_DIPSETTING(    0x08, "3" )
	PORT_DIPSETTING(    0x00, "4" ) // first dragon scene only shows 3?
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("DSW2")
	PORT_DIPNAME( 0x07, 0x04, DEF_STR( Coinage ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 6C_1C ) )
	PORT_DIPSETTING(    0x01, DEF_STR( 5C_1C ) )
	PORT_DIPSETTING(    0x02, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(    0x04, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x05, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x06, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0x07, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(    0x03, "Invalid" ) // can't coin up or start? (probably a non-functioning freeplay)

	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("1P")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_UP )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON2 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_BUTTON3 )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("2P")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_UP )    PORT_PLAYER(2)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN )  PORT_PLAYER(2)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT )  PORT_PLAYER(2)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(2)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 )        PORT_PLAYER(2)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON2 )        PORT_PLAYER(2)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_BUTTON3 )        PORT_PLAYER(2)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("OTHERS")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_SERVICE1 ) // coin?
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_COIN2 ) // service?
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )
INPUT_PORTS_END


static INPUT_PORTS_START( androidp )
	PORT_INCLUDE(androidpo)

	PORT_MODIFY("DSW1")
	PORT_DIPNAME( 0x01, 0x00, DEF_STR( Cabinet ) )
	PORT_DIPSETTING(    0x01, DEF_STR( Cocktail ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Upright ) )

	PORT_DIPNAME( 0x0c, 0x04, DEF_STR( Lives ) )
	PORT_DIPSETTING(    0x08, "1" )
	PORT_DIPSETTING(    0x0c, "2" )
	PORT_DIPSETTING(    0x04, "3" )
	PORT_DIPSETTING(    0x00, "4" )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Allow_Continue ) )
	PORT_DIPSETTING(    0x00, DEF_STR( No ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Demo_Sounds ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x80, DEF_STR( On ) )

	PORT_MODIFY("DSW2")
	PORT_DIPNAME( 0x07, 0x07, DEF_STR( Coinage ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(    0x01, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x02, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x07, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x03, DEF_STR( 2C_3C ) )
	PORT_DIPSETTING(    0x06, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x05, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0x04, DEF_STR( 1C_4C ) )

INPUT_PORTS_END

/****************************************************************************/

static const gfx_layout layout_p103a =
{
	32,32,
	RGN_FRAC(1,1),
	4,
	{ 0,1,2,3 },
	{ 4,0,12,8,20,16,28,24,36,32,44,40,52,48,60,56, 68,64,76,72,84,80,92,88,100,96,108,104,116,112,124,120 },
	{ STEP32(0,128) },
	32*32*4
};


static GFXDECODE_START( gfx_himesiki )
	GFXDECODE_ENTRY( "bgtiles",   0, gfx_8x8x4_packed_lsb,   0x000, 16 )
	GFXDECODE_ENTRY( "sprites",   0, gfx_16x16x4_packed_lsb, 0x200, 16 )
	GFXDECODE_ENTRY( "spr_p103a", 0, layout_p103a,           0x200, 16 )
GFXDECODE_END


void himesiki_state::machine_start()
{
	m_mainbank->configure_entries(0, 4, memregion("banks")->base(), 0x4000);


	save_item(NAME(m_scrollx));
	save_item(NAME(m_scrolly));

	save_item(NAME(m_flipscreen));
}

void himesiki_state::machine_reset()
{
	m_scrollx[0] = 0;
	m_scrollx[1] = 0;
	m_scrolly = 0;

	m_flipscreen = 0;
}

void himesiki_state::himesiki(machine_config &config)
{
	[[maybe_unused]] constexpr XTAL MCLK = XTAL(12'000'000); // this is on the video board
	constexpr XTAL CLK2 = XTAL(8'000'000); // near the CPUs

	// basic machine hardware
	Z80(config, m_maincpu, CLK2); // it's a 6.000 MHz rated part, but near the 8 Mhz XTAL?? - Android skips lots of frames at 6, crashes at 4
	m_maincpu->set_addrmap(AS_PROGRAM, &himesiki_state::main_prg_map);
	m_maincpu->set_addrmap(AS_IO, &himesiki_state::main_io_map);
	m_maincpu->set_vblank_int("screen", FUNC(himesiki_state::irq0_line_hold));

	Z80(config, m_subcpu, CLK2 / 2); // 4.000 MHz (4Mhz rated part, near the 8 Mhz XTAL)
	m_subcpu->set_addrmap(AS_PROGRAM, &himesiki_state::sound_prg_map);
	m_subcpu->set_addrmap(AS_IO, &himesiki_state::sound_io_map);

	i8255_device &ppi0(I8255A(config, "ppi8255_0"));
	ppi0.in_pa_callback().set_ioport("1P");
	ppi0.in_pb_callback().set_ioport("2P");
	ppi0.in_pc_callback().set_ioport("OTHERS");

	i8255_device &ppi1(I8255A(config, "ppi8255_1"));
	ppi1.in_pa_callback().set_ioport("DSW1");
	ppi1.in_pb_callback().set_ioport("DSW2");
	ppi1.out_pc_callback().set(FUNC(himesiki_state::rombank_w));

	// video hardware
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_refresh_hz(60);
	screen.set_vblank_time(ATTOSECONDS_IN_USEC(2500));
	screen.set_size(32*8, 32*8);
	screen.set_visarea(0*8, 32*8-1, 0*8, 24*8-1);
	screen.set_screen_update(FUNC(himesiki_state::screen_update));
	screen.set_palette(m_palette);

	GFXDECODE(config, m_gfxdecode, m_palette, gfx_himesiki);
	PALETTE(config, m_palette, palette_device::BLACK).set_format(palette_device::xRGB_555, 1024);

	// sound hardware
	SPEAKER(config, "mono").front_center();

	GENERIC_LATCH_8(config, m_soundlatch);

	ym2203_device &ym2203(YM2203(config, "ym2203", CLK2 / 4)); // ??
	ym2203.irq_handler().set_inputline("sub", 0);
	ym2203.add_route(0, "mono", 0.10);
	ym2203.add_route(1, "mono", 0.10);
	ym2203.add_route(2, "mono", 0.10);
	ym2203.add_route(3, "mono", 0.50);
}


/****************************************************************************/

ROM_START( himesiki )
	ROM_REGION( 0x08000, "maincpu", 0 ) // z80
	ROM_LOAD( "1.1k",  0x00000,  0x08000, CRC(fb4604b3) SHA1(e8155bbafb881125e1bf9a04808d6a6546887e90) )

	ROM_REGION( 0x10000, "banks", 0 )
	ROM_LOAD( "2.1g",  0x00000,  0x04000, CRC(0c30ded1) SHA1(0ad67115fa15d0b6261a278a946a6d46c06430ef) )
	ROM_CONTINUE(      0x08000,  0x04000)
	// 1j is unpopulated on this game

	ROM_REGION( 0x08000, "sub", 0 ) // z80
	ROM_LOAD( "5.6n",  0x00000,  0x08000, CRC(b1214ac7) SHA1(ee5459c28d9c3c2eb3467261716b1259ec486534) )

	ROM_REGION( 0x020000, "bgtiles", 0 )
	ROM_LOAD( "3.5f",  0x000000,  0x010000, CRC(73843e60) SHA1(0d8a397d8798e15f3fa7bf7a83e4c2ee44f6fa86) )
	ROM_LOAD( "4.5d",  0x010000,  0x010000, CRC(443a3164) SHA1(08aa002214251a870581a01d775f497dd390957c) )

	ROM_REGION( 0x040000, "sprites", 0 )
	ROM_LOAD16_BYTE( "13.9e", 0x000000,  0x010000, CRC(43102682) SHA1(0d4bde8bece0cbc6c06071aa8ad210a0636d862f) )
	ROM_LOAD16_BYTE( "12.9c", 0x000001,  0x010000, CRC(19c8f9f4) SHA1(b14c8a6b94fd474be375e7a6a03d7f4517da2247) )
	ROM_LOAD16_BYTE( "15.8e", 0x020000,  0x010000, CRC(2630d394) SHA1(b2e9e836b1f053fce3212912c53d3cdca3372439) )
	ROM_LOAD16_BYTE( "14.8c", 0x020001,  0x010000, CRC(8103a207) SHA1(0dde8a0aaf2618d9c1589f35841db210439d0388) )


	ROM_REGION( 0x060000, "spr_p103a", 0 )
	ROM_LOAD16_BYTE( "6.1a",  0x000000,  0x010000, CRC(14989c22) SHA1(fe0c31df10237294ea8ef0ab8965ba5bb25113a2) )
	ROM_LOAD16_BYTE( "7.1c",  0x000001,  0x010000, CRC(cec56e16) SHA1(836ff413301044313fdf7af5d304c145137b898a) )
	ROM_LOAD16_BYTE( "8.2a",  0x020000,  0x010000, CRC(44ba127e) SHA1(d756b6c3075d75287f9c8be662c1eab02f4245a3) )
	ROM_LOAD16_BYTE( "9.2c",  0x020001,  0x010000, CRC(0dda724a) SHA1(2b064b1d657f896e8385f17def9e4ffc0802bf97) )
	ROM_LOAD16_BYTE( "10.4a", 0x040000,  0x010000, CRC(0adda8d1) SHA1(dfee2c7921fdc972b4e95fdf89520f74a4e8b4ee) )
	ROM_LOAD16_BYTE( "11.4c", 0x040001,  0x010000, CRC(aa032946) SHA1(bd8900e4a22580e3bfe33b8164909db19bb07a8f) )

ROM_END



ROM_START( androidpo )
	ROM_REGION( 0x08000, "maincpu", 0 )
	ROM_LOAD( "mitsubishi__ad1__m5l27256k.toppcb.k1", 0x00000, 0x08000, CRC(25ab85eb) SHA1(e1fab149c83ff880b119258206d5818f3db641c5) )

	ROM_REGION( 0x10000, "banks", 0 )
	ROM_LOAD( "mitsubishi__ad-3__m5l27256k.toppcb.g1", 0x00000, 0x04000, CRC(6cf5f48a) SHA1(b9b4e5e7bace0e8d98fbc9f4ad91bc56ef42099e) )
	ROM_CONTINUE(                                      0x08000, 0x04000)
	ROM_LOAD( "mitsubishi__ad2__m5l27256k.toppcb.j1",  0x04000, 0x04000, CRC(e41426be) SHA1(e7e06ef3ff5160bb7d870e148ba2799da52cf24c) )
	ROM_CONTINUE(                                      0x0c000, 0x04000)

	ROM_REGION( 0x08000, "sub", 0 )
	ROM_LOAD( "mitsubishi__ad-4__m5l27256k.toppcb.n6", 0x00000, 0x08000, CRC(13c38fe4) SHA1(34a35fa057159a5c83892a88b8c908faa39d5cb3) )

	ROM_REGION( 0x10000, "bgtiles", 0 )
	ROM_LOAD( "mitsubishi__ad-5__m5l27512k.toppcb.f5", 0x00000, 0x10000, CRC(4c72a930) SHA1(f1542844391b55fe43293eef7ce48c09b7aca75a) )

	ROM_REGION( 0x20000, "sprites", 0 )
	ROM_LOAD16_BYTE( "mitsubishi__ad-6__m5l27512k.botpcb.def9", 0x00000, 0x10000, CRC(5e42984e) SHA1(2a928960c740dfb94589e011cce093bed2fd7685) )
	ROM_LOAD16_BYTE( "mitsubishi__ad-7__m5l27512k.botpcb.bc9",  0x00001, 0x10000, CRC(611ff400) SHA1(1a9aed33d0e3f063811f92b9fee3ecbff0e965bf) )

	ROM_REGION( 0x2000, "plds", 0 )
	ROM_LOAD( "ricoh_7a2_19__epl10p8bp_japan_i.f1.jed", 0x0000, 0x0473, CRC(c5e51ea2) SHA1(3e35a30935f562227f0afa32a6be6eb33f9a8372) )
	ROM_LOAD( "ricoh_7a2_19__epl10p8bp_japan_m.j3.jed", 0x1000, 0x0473, CRC(807d1553) SHA1(257be9eacd57d2e2dbaab3be5725d8d06d6a9a0b) )

	ROM_REGION( 0x20000, "spr_p103a", ROMREGION_ERASEFF )
	// there's no P103A PCB for this on Android
ROM_END

ROM_START( androidp )
	ROM_REGION( 0x08000, "maincpu", 0 )
	ROM_LOAD( "andr1.bin", 0x00000, 0x08000, CRC(fff04130) SHA1(9bdafa8b311cc5d0851b04df3c6dd16eb087a5dd) )

	ROM_REGION( 0x10000, "banks", 0 )
	ROM_LOAD( "andr3.bin", 0x00000, 0x04000, CRC(112d5123) SHA1(653109eae7b58d9dcb8892ea9aca17427f14c145) )
	ROM_CONTINUE(          0x08000, 0x04000)

	ROM_REGION( 0x08000, "sub", 0 )
	ROM_LOAD( "andr4.bin", 0x00000, 0x08000, CRC(65f5e98b) SHA1(69f979d653695413a1c503c402d4bf5ffcfb6e5d) )

	ROM_REGION( 0x10000, "bgtiles", 0 )
	ROM_LOAD( "andr5.bin", 0x00000, 0x10000, CRC(0a0b44c0) SHA1(8d359b802c7dee5faea9464f06b672fd401799cf) )

	ROM_REGION( 0x20000, "sprites", 0 )
	ROM_LOAD16_BYTE( "andr6.bin", 0x00000, 0x10000, CRC(122b7dd1) SHA1(5dffd2b97c8222afc98552513b84a91d6127f41b) )
	ROM_LOAD16_BYTE( "andr7.bin", 0x00001, 0x10000, CRC(fc0f9234) SHA1(496a918cc1f4d0e7191a49cc43c51fbd71e0bdf5) )

	ROM_REGION( 0x20000, "spr_p103a", ROMREGION_ERASEFF )
	// there's no P103A PCB for this on Android


	// + 2 undumped PLDs (?)
ROM_END

} // anonymous namespace


GAME( 1989, himesiki,  0,        himesiki, himesiki,  himesiki_state, empty_init, ROT90, "Hi-Soft", "Himeshikibu (Japan)", MACHINE_SUPPORTS_SAVE )

// the game changed significantly between these 2 versions, it is possible the later build was actually released
GAME( 1987, androidp,  0,        himesiki, androidp,  himesiki_state, empty_init, ROT90, "Nasco", "Android (prototype, later build)", MACHINE_SUPPORTS_SAVE ) // shows 1987 copyright after staff list during ending
GAME( 198?, androidpo, androidp, himesiki, androidpo, himesiki_state, empty_init, ROT90, "Nasco", "Android (prototype, early build)", MACHINE_SUPPORTS_SAVE )
