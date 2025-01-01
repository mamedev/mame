// license:BSD-3-Clause
// copyright-holders:Ernesto Corvi
/*******************************************************************************

    Kyugo hardware games

    driver by:
    Ernesto Corvi
    someone@secureshell.com

    Games supported:
        * Gyrodine - (c) 1984 Crux
        * Repulse - (c) 1985 Sega
        * '99 The last war - (c) 1985 Proma
        * Son of Phoenix - (c) 1985 Associated Overseas MFR, Inc.
        * Flashgal - (c) 1985 Sega
        * SRD Mission - (c) 1986 Taito Corporation.
        * Legend - no copyright, but readme says: (c) 1986 SEGA/Coreland
        * Airwolf - (c) 1987 Kyugo

    Known issues:
        * attract mode in Son of Phoenix doesn't work

********************************************************************************

Repulse bootleg
Hardware Info By Guru

PCB Layout
----------

Top PCB

 |------------------------------------------------|
 |                          PAL                   |
|-|       2016        PAL                         |
| |                                               |
| |       ROM.2K            Z80                   |
| |                                               |
| |       ROM.2J   ROM.4J                         |
| |                                               |
|-|       ROM.2H   ROM.4H                         |
 |                                                |
 |        ROM.2F   ROM.4F              AY-3-8910  |
 |                                                |
 |1             Z80                    AY-3-8910  |
 |8                                               |
 |W                                      DSW2     |
 |A                                               |
 |Y                                      DSW1     |
 |           AMP      VOL                         |
 |------------------------------------------------|
Notes:
      2016      - 2kx8 SRAM
      Z80       - Clock 3.072MHz [18.432/6] (both)
      AY-3-8910 - Clock 1.536MHz [18.432/12] (both)
      HSync     - 15.5154kHz
      VSync     - 59.6575Hz

      ALL ROMs match the original set.
      ROM CRC32 Table (to match ROMs from original to bootleg locations)
      ------------------------------------------------------------------
      Location    CRC32      Repulse ROM-set Name
      2K          86b267f3   repulse.b4
      2J          197e314c   3.j2
      2H          b3c6a886   2.h2
      2F          c485c621   1.f2
      4J          57a8e900   7.j4
      4H          99129918   repulse.b6
      4F          fb2b7c9d   repulse.b5


Bottom PCB

 |-----------------------------------------------------------------------------|
 |                             18.432MHz                                       |
|-|                                                                            |
| |                                                                            |
| |                                                                            |
| |               2016                                              2016  2016 |
| |                                                                            |
| |                                   2114                                     |
|-|                                                                            |
 |                        6148        2114                                     |
 |                                                    ROM.9H  ROM.10H  ROM.11H |
 |                6148    6148                                                 |
 |                             PROM.5H%                                        |
 |                6148                                                         |
 |                                                                             |
 |  PROM.1H                                                                    |
 |  PROM.1G                                                                    |
 |  PROM.1F                                                                    |
 |                                                                             |
 |                                                                             |
 |                ROM.4A      ROM.6A  ROM.7A  ROM.8A  ROM.9A  ROM.10A  ROM.11A |
 |                                                                             |
 |-----------------------------------------------------------------------------|
Notes:
      2016  - 2kx8 SRAM
      6148  - 1kx4 SRAM
      2114  - 1kx4 SRAM
      PROM* - 82S129 Bi-Polar PROM
      PROM% - Package looks like other PROMs so it might be a PROM but no part# on it

      ALL ROMs match the original set.
      ROM CRC32 Table (to match ROMs from original to bootleg locations)
      ------------------------------------------------------------------
      Location    CRC32      Repulse ROM-set Name
      4A          c79f05eb*  repulse.a11
      6A          0e9f757e   8.6a
      7A          f7d2e650   9.7a
      8A          e717baf4   10.8a
      9A          04b2250b   11.9a
      10A         d110e140   12.10a
      11A         8fdc713c   13.11a
      9H          c9213469   15.9h
      10H         7de5d39e   16.10h
      11H         0ba5f72c   17.11h

      * This ROM is a 2764 with the data doubled. The original dumped ROM at A11 is only 4k
        so held in a 2732 EPROM. The original ROM might be the same and doubled to fill a
        2764 like the bootleg was but possibly chopped in half for emulation, hence the 4k
        size of the original ROM compared to the 8k size of the bootleg ROM.

*******************************************************************************/

#include "emu.h"

#include "cpu/z80/z80.h"
#include "machine/74259.h"
#include "machine/timer.h"
#include "machine/watchdog.h"
#include "sound/ay8910.h"

#include "emupal.h"
#include "screen.h"
#include "speaker.h"
#include "tilemap.h"

namespace {

class kyugo_state : public driver_device
{
public:
	kyugo_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_fgvideoram(*this, "fgvideoram"),
		m_bgvideoram(*this, "bgvideoram"),
		m_bgattribram(*this, "bgattribram"),
		m_spriteram(*this, "spriteram_%u", 1U),
		m_shared_ram(*this, "shared_ram"),
		m_maincpu(*this, "maincpu"),
		m_subcpu(*this, "sub"),
		m_gfxdecode(*this, "gfxdecode"),
		m_palette(*this, "palette")
	{ }

	void kyugo_base(machine_config &config);
	void repulse(machine_config &config);
	void flashgala(machine_config &config);
	void srdmissn(machine_config &config);
	void legend(machine_config &config);
	void gyrodine(machine_config &config);

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;
	virtual void video_start() override ATTR_COLD;

private:
	void nmi_mask_w(int state);
	void coin_counter_w(offs_t offset, uint8_t data);
	void fgvideoram_w(offs_t offset, uint8_t data);
	void bgvideoram_w(offs_t offset, uint8_t data);
	void bgattribram_w(offs_t offset, uint8_t data);
	uint8_t spriteram_2_r(offs_t offset);
	void scroll_x_lo_w(uint8_t data);
	void gfxctrl_w(uint8_t data);
	void scroll_y_w(uint8_t data);

	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	TIMER_DEVICE_CALLBACK_MEMBER(interrupt);

	void flashgala_sub_map(address_map &map) ATTR_COLD;
	void flashgala_sub_portmap(address_map &map) ATTR_COLD;
	void gyrodine_main_map(address_map &map) ATTR_COLD;
	void gyrodine_sub_map(address_map &map) ATTR_COLD;
	void gyrodine_sub_portmap(address_map &map) ATTR_COLD;
	void kyugo_main_map(address_map &map) ATTR_COLD;
	void kyugo_main_portmap(address_map &map) ATTR_COLD;
	void srdmissn_main_map(address_map &map) ATTR_COLD;
	void legend_sub_map(address_map &map) ATTR_COLD;
	void repulse_sub_map(address_map &map) ATTR_COLD;
	void repulse_sub_portmap(address_map &map) ATTR_COLD;
	void srdmissn_sub_map(address_map &map) ATTR_COLD;
	void srdmissn_sub_portmap(address_map &map) ATTR_COLD;

	// memory pointers
	required_shared_ptr<uint8_t> m_fgvideoram;
	required_shared_ptr<uint8_t> m_bgvideoram;
	required_shared_ptr<uint8_t> m_bgattribram;
	required_shared_ptr_array<uint8_t, 2> m_spriteram;
	required_shared_ptr<uint8_t> m_shared_ram;

	uint8_t m_nmi_mask = 0U;

	// video-related
	tilemap_t *m_bg_tilemap = nullptr;
	tilemap_t *m_fg_tilemap = nullptr;
	uint8_t m_scroll_x_lo = 0U;
	uint8_t m_scroll_x_hi = 0U;
	uint8_t m_scroll_y = 0U;
	uint8_t m_bgpalbank = 0U;
	uint8_t m_fgcolor = 0U;
	const uint8_t *m_color_codes = nullptr;

	// devices
	required_device<cpu_device> m_maincpu;
	required_device<cpu_device> m_subcpu;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;

	TILE_GET_INFO_MEMBER(get_fg_tile_info);
	TILE_GET_INFO_MEMBER(get_bg_tile_info);
	void draw_sprites(bitmap_ind16 &bitmap, const rectangle &cliprect);
};



/***************************************************************************

  Callbacks for the TileMap code

***************************************************************************/

TILE_GET_INFO_MEMBER(kyugo_state::get_fg_tile_info)
{
	int code = m_fgvideoram[tile_index];
	int color = m_color_codes[code >> 3] << 1 | m_fgcolor;

	tileinfo.set(0, code, color, 0);
}


TILE_GET_INFO_MEMBER(kyugo_state::get_bg_tile_info)
{
	int attr = m_bgattribram[tile_index];
	int code = m_bgvideoram[tile_index] | ((attr & 0x03) << 8);
	int color = (attr >> 4) | (m_bgpalbank << 4);

	tileinfo.set(1, code, color, TILE_FLIPYX((attr & 0x0c) >> 2));
}



/*************************************
 *
 *  Video system start
 *
 *************************************/

void kyugo_state::video_start()
{
	m_color_codes = memregion("proms")->base() + 0x300;

	m_fg_tilemap = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(kyugo_state::get_fg_tile_info)), TILEMAP_SCAN_ROWS, 8, 8, 64, 32);
	m_bg_tilemap = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(kyugo_state::get_bg_tile_info)), TILEMAP_SCAN_ROWS, 8, 8, 64, 32);

	m_fg_tilemap->set_transparent_pen(0);

	m_bg_tilemap->set_scrolldx(-32, 288+32);
}



/*************************************
 *
 *  Memory handlers
 *
 *************************************/

void kyugo_state::fgvideoram_w(offs_t offset, uint8_t data)
{
	m_fgvideoram[offset] = data;
	m_fg_tilemap->mark_tile_dirty(offset);
}


void kyugo_state::bgvideoram_w(offs_t offset, uint8_t data)
{
	m_bgvideoram[offset] = data;
	m_bg_tilemap->mark_tile_dirty(offset);
}


void kyugo_state::bgattribram_w(offs_t offset, uint8_t data)
{
	m_bgattribram[offset] = data;
	m_bg_tilemap->mark_tile_dirty(offset);
}


uint8_t kyugo_state::spriteram_2_r(offs_t offset)
{
	// only the lower nibble is connected
	return m_spriteram[1][offset] | 0xf0;
}


void kyugo_state::scroll_x_lo_w(uint8_t data)
{
	m_scroll_x_lo = data;
}


void kyugo_state::gfxctrl_w(uint8_t data)
{
	// bit 0 is scroll MSB
	m_scroll_x_hi = data & 0x01;

	// bit 5 is front layer color (Son of Phoenix only)
	if (m_fgcolor != ((data & 0x20) >> 5))
	{
		m_fgcolor = (data & 0x20) >> 5;
		m_fg_tilemap->mark_all_dirty();
	}

	// bit 6 is background palette bank
	if (m_bgpalbank != ((data & 0x40) >> 6))
	{
		m_bgpalbank = (data & 0x40) >> 6;
		m_bg_tilemap->mark_all_dirty();
	}

	// other bits are unknown
}


void kyugo_state::scroll_y_w(uint8_t data)
{
	m_scroll_y = data;
}



/*************************************
 *
 *  Video update
 *
 *************************************/

void kyugo_state::draw_sprites(bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	// sprite information is scattered through memory
	// and uses a portion of the text layer memory (outside the visible area)
	uint8_t *spriteram_area1 = &m_spriteram[0][0x28];
	uint8_t *spriteram_area2 = &m_spriteram[1][0x28];
	uint8_t *spriteram_area3 = &m_fgvideoram[0x28];

	int flip = flip_screen();

	for (int n = 0; n < 12 * 2; n++)
	{
		int offs, sy, sx, color;

		offs = 2 * (n % 12) + 64 * (n / 12);

		sx = spriteram_area3[offs + 1] + 256 * (spriteram_area2[offs + 1] & 1);
		if (sx > 320)
			sx -= 512;

		sy = 255 - spriteram_area1[offs] + 2;
		if (sy > 0xf0)
			sy -= 256;

		if (flip)
			sy = 240 - sy;

		color = spriteram_area1[offs + 1] & 0x1f;

		for (int y = 0; y < 16; y++)
		{
			int code = spriteram_area3[offs + 128 * y];
			int attr = spriteram_area2[offs + 128 * y];

			code = code | ((attr & 0x01) << 9) | ((attr & 0x02) << 7);

			int flipx = attr & 0x08;
			int flipy = attr & 0x04;

			if (flip)
			{
				flipx = !flipx;
				flipy = !flipy;
			}

			m_gfxdecode->gfx(2)->transpen(bitmap,cliprect,
					code,
					color,
					flipx,flipy,
					sx, flip ? sy - 16*y : sy + 16*y, 0 );
		}
	}
}


uint32_t kyugo_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	if (flip_screen())
		m_bg_tilemap->set_scrollx(0, -(m_scroll_x_lo + (m_scroll_x_hi * 256)));
	else
		m_bg_tilemap->set_scrollx(0, m_scroll_x_lo + (m_scroll_x_hi * 256));

	m_bg_tilemap->set_scrolly(0, m_scroll_y);

	m_bg_tilemap->draw(screen, bitmap, cliprect, 0, 0);
	draw_sprites(bitmap, cliprect);
	m_fg_tilemap->draw(screen, bitmap, cliprect, 0, 0);

	return 0;
}



/*************************************
 *
 *  Main CPU memory handlers
 *
 *************************************/

void kyugo_state::kyugo_main_map(address_map &map)
{
	map(0x0000, 0x7fff).rom();
	map(0x8000, 0x87ff).ram().w(FUNC(kyugo_state::bgvideoram_w)).share(m_bgvideoram);
	map(0x8800, 0x8fff).ram().w(FUNC(kyugo_state::bgattribram_w)).share(m_bgattribram);
	map(0x9000, 0x97ff).ram().w(FUNC(kyugo_state::fgvideoram_w)).share(m_fgvideoram);
	map(0x9800, 0x9fff).ram().r(FUNC(kyugo_state::spriteram_2_r)).share(m_spriteram[1]);
	map(0xa000, 0xa7ff).ram().share(m_spriteram[0]);
	map(0xa800, 0xa800).w(FUNC(kyugo_state::scroll_x_lo_w));
	map(0xb000, 0xb000).w(FUNC(kyugo_state::gfxctrl_w));
	map(0xb800, 0xb800).w(FUNC(kyugo_state::scroll_y_w));
	map(0xf000, 0xf7ff).ram().share(m_shared_ram);
}

void kyugo_state::srdmissn_main_map(address_map &map)
{
	kyugo_main_map(map);
	map(0xe000, 0xe7ff).ram().share(m_shared_ram);
}

void kyugo_state::gyrodine_main_map(address_map &map)
{
	kyugo_main_map(map);
	map(0xe000, 0xe000).w("watchdog", FUNC(watchdog_timer_device::reset_w));
}



/*************************************
 *
 *  Main CPU port handlers
 *
 *************************************/

void kyugo_state::nmi_mask_w(int state)
{
	m_nmi_mask = state;
}

void kyugo_state::kyugo_main_portmap(address_map &map)
{
	map.global_mask(0x07);
	map(0x00, 0x07).w("mainlatch", FUNC(ls259_device::write_d0));
}



/*************************************
 *
 *  Sub CPU memory handlers
 *
 *************************************/

void kyugo_state::gyrodine_sub_map(address_map &map)
{
	map(0x0000, 0x1fff).rom();
	map(0x4000, 0x47ff).ram().share(m_shared_ram);
	map(0x8000, 0x8000).portr("P2");
	map(0x8040, 0x8040).portr("P1");
	map(0x8080, 0x8080).portr("SYSTEM");
}


void kyugo_state::repulse_sub_map(address_map &map)
{
	map(0x0000, 0x7fff).rom();
	map(0xa000, 0xa7ff).ram().share(m_shared_ram);
	map(0xc000, 0xc000).portr("P2");
	map(0xc040, 0xc040).portr("P1");
	map(0xc080, 0xc080).portr("SYSTEM");
}


void kyugo_state::srdmissn_sub_map(address_map &map)
{
	map(0x0000, 0x7fff).rom();
	map(0x8000, 0x87ff).ram().share(m_shared_ram);
	map(0x8800, 0x8fff).ram();
	map(0xf400, 0xf400).portr("SYSTEM");
	map(0xf401, 0xf401).portr("P1");
	map(0xf402, 0xf402).portr("P2");
}


void kyugo_state::legend_sub_map(address_map &map)
{
	map(0x0000, 0x7fff).rom();
	map(0xc000, 0xc7ff).ram().share(m_shared_ram);
	map(0xf800, 0xf800).portr("SYSTEM");
	map(0xf801, 0xf801).portr("P1");
	map(0xf802, 0xf802).portr("P2");
}


void kyugo_state::flashgala_sub_map(address_map &map)
{
	map(0x0000, 0x7fff).rom();
	map(0xc040, 0xc040).portr("SYSTEM");
	map(0xc080, 0xc080).portr("P1");
	map(0xc0c0, 0xc0c0).portr("P2");
	map(0xe000, 0xe7ff).ram().share(m_shared_ram);
}



/*************************************
 *
 *  Sub CPU port handlers
 *
 *************************************/

void kyugo_state::coin_counter_w(offs_t offset, uint8_t data)
{
	machine().bookkeeping().coin_counter_w(offset, data & 1);
}

void kyugo_state::gyrodine_sub_portmap(address_map &map)
{
	map.global_mask(0xff);
	map(0x00, 0x01).w("ay1", FUNC(ay8910_device::address_data_w));
	map(0x02, 0x02).r("ay1", FUNC(ay8910_device::data_r));
	map(0xc0, 0xc1).w("ay2", FUNC(ay8910_device::address_data_w));
}


void kyugo_state::repulse_sub_portmap(address_map &map)
{
	map.global_mask(0xff);
	map(0x00, 0x01).w("ay1", FUNC(ay8910_device::address_data_w));
	map(0x02, 0x02).r("ay1", FUNC(ay8910_device::data_r));
	map(0x40, 0x41).w("ay2", FUNC(ay8910_device::address_data_w));
	map(0xc0, 0xc1).w(FUNC(kyugo_state::coin_counter_w));
}


void kyugo_state::flashgala_sub_portmap(address_map &map)
{
	map.global_mask(0xff);
	map(0x40, 0x41).w("ay1", FUNC(ay8910_device::address_data_w));
	map(0x42, 0x42).r("ay1", FUNC(ay8910_device::data_r));
	map(0x80, 0x81).w("ay2", FUNC(ay8910_device::address_data_w));
	map(0xc0, 0xc1).w(FUNC(kyugo_state::coin_counter_w));
}


void kyugo_state::srdmissn_sub_portmap(address_map &map)
{
	map.global_mask(0xff);
	map(0x80, 0x81).w("ay1", FUNC(ay8910_device::address_data_w));
	map(0x82, 0x82).r("ay1", FUNC(ay8910_device::data_r));
	map(0x84, 0x85).w("ay2", FUNC(ay8910_device::address_data_w));
	map(0x90, 0x91).w(FUNC(kyugo_state::coin_counter_w));
}



/*************************************
 *
 *  Port definitions
 *
 *************************************/

static INPUT_PORTS_START( common )
	PORT_START("DSW2")
	PORT_DIPNAME( 0x07, 0x07, DEF_STR( Coin_A ) )           PORT_DIPLOCATION("DSW2:1,2,3")
	PORT_DIPSETTING(    0x02, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x01, DEF_STR( 3C_2C ) )
	PORT_DIPSETTING(    0x07, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x06, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x05, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0x04, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(    0x03, DEF_STR( 1C_6C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Free_Play ) )
	PORT_DIPNAME( 0x38, 0x38, DEF_STR( Coin_B ) )           PORT_DIPLOCATION("DSW2:4,5,6")
	PORT_DIPSETTING(    0x00, DEF_STR( 5C_1C ) )
	PORT_DIPSETTING(    0x08, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(    0x10, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x18, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x38, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x20, DEF_STR( 3C_4C ) )
	PORT_DIPSETTING(    0x30, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x28, DEF_STR( 1C_3C ) )
	PORT_DIPUNUSED( 0x40, IP_ACTIVE_LOW )                   PORT_DIPLOCATION("DSW2:7")
	PORT_DIPUNUSED( 0x80, IP_ACTIVE_LOW )                   PORT_DIPLOCATION("DSW2:8")

	PORT_START("SYSTEM")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_COIN1 )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_COIN2 )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_SERVICE1 )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_START1 )
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_START2 )
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNKNOWN )

	PORT_START("P1")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_8WAY
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_8WAY
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP ) PORT_8WAY
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN ) PORT_8WAY
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_BUTTON1 )
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_BUTTON2 )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNKNOWN )

	PORT_START("P2")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_COCKTAIL
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_COCKTAIL
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNKNOWN )
INPUT_PORTS_END

static INPUT_PORTS_START( gyrodine )
	PORT_START("DSW1")
	PORT_DIPNAME( 0x03, 0x03, DEF_STR( Lives ) )            PORT_DIPLOCATION("DSW1:1,2")
	PORT_DIPSETTING(    0x03, "3" )
	PORT_DIPSETTING(    0x02, "4" )
	PORT_DIPSETTING(    0x01, "5" )
	PORT_DIPSETTING(    0x00, "6" )
	PORT_DIPUNUSED_DIPLOC( 0x04, IP_ACTIVE_LOW, "DSW1:3" )
	PORT_DIPUNUSED_DIPLOC( 0x08, IP_ACTIVE_LOW, "DSW1:4" )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Difficulty ) )       PORT_DIPLOCATION("DSW1:5")
	PORT_DIPSETTING(    0x10, DEF_STR( Easy ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Hard ) )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Bonus_Life ) )       PORT_DIPLOCATION("DSW1:6")
	PORT_DIPSETTING(    0x20, "20000 50000" )
	PORT_DIPSETTING(    0x00, "40000 70000" )
	PORT_DIPNAME( 0x40, 0x00, DEF_STR( Cabinet ) )          PORT_DIPLOCATION("DSW1:7")
	PORT_DIPSETTING(    0x00, DEF_STR( Upright ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Cocktail ) )
	PORT_DIPNAME( 0x80, 0x80, "Freeze" )                    PORT_DIPLOCATION("DSW1:8")
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_INCLUDE( common )
INPUT_PORTS_END

static INPUT_PORTS_START( repulse )
	PORT_START("DSW1")
	PORT_DIPNAME( 0x03, 0x03, DEF_STR( Lives ) )            PORT_DIPLOCATION("DSW1:1,2")
	PORT_DIPSETTING(    0x03, "3" )
	PORT_DIPSETTING(    0x02, "4" )
	PORT_DIPSETTING(    0x01, "5" )
	PORT_DIPSETTING(    0x00, "6" )
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Bonus_Life ) )       PORT_DIPLOCATION("DSW1:3")
	PORT_DIPSETTING(    0x04, "Every 50000" )
	PORT_DIPSETTING(    0x00, "Every 70000" )
	PORT_DIPNAME( 0x08, 0x08, "Slow Motion" )               PORT_DIPLOCATION("DSW1:4")
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME(0x10,  0x10, "Invulnerability (Cheat)")    PORT_DIPLOCATION("DSW1:5")
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, "Sound Test" )                PORT_DIPLOCATION("DSW1:6")
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x00, DEF_STR( Cabinet ) )          PORT_DIPLOCATION("DSW1:7")
	PORT_DIPSETTING(    0x00, DEF_STR( Upright ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Cocktail ) )
	PORT_DIPNAME( 0x80, 0x80, "Freeze" )                    PORT_DIPLOCATION("DSW1:8")
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_INCLUDE( common )

	PORT_MODIFY("DSW2")
	PORT_DIPNAME( 0xc0, 0x80, DEF_STR( Difficulty ) )       PORT_DIPLOCATION("DSW2:7,8")
	PORT_DIPSETTING(    0xc0, DEF_STR( Easy ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Normal ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Hard ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Hardest ) )
INPUT_PORTS_END

static INPUT_PORTS_START( airwolf )
	PORT_START("DSW1")
	PORT_DIPNAME( 0x03, 0x03, DEF_STR( Lives ) )            PORT_DIPLOCATION("DSW1:1,2")
	PORT_DIPSETTING(    0x03, "4" )
	PORT_DIPSETTING(    0x02, "5" )
	PORT_DIPSETTING(    0x01, "6" )
	PORT_DIPSETTING(    0x00, "7" )
	PORT_DIPNAME( 0x04, 0x00, DEF_STR( Allow_Continue ) )   PORT_DIPLOCATION("DSW1:3")
	PORT_DIPSETTING(    0x04, DEF_STR( No ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x08, 0x08, "Slow Motion" )               PORT_DIPLOCATION("DSW1:4")
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, "Invulnerability (Cheat)")    PORT_DIPLOCATION("DSW1:5")
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, "Sound Test" )                PORT_DIPLOCATION("DSW1:6")
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x00, DEF_STR( Cabinet ) )          PORT_DIPLOCATION("DSW1:7")
	PORT_DIPSETTING(    0x00, DEF_STR( Upright ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Cocktail ) )
	PORT_DIPNAME( 0x80, 0x80, "Freeze" )                    PORT_DIPLOCATION("DSW1:8")
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_INCLUDE( common )
INPUT_PORTS_END

// Same as 'airwolf', but different "Lives" Dip Switch
static INPUT_PORTS_START( skywolf )
	PORT_INCLUDE( airwolf )

	PORT_MODIFY("DSW1")
	PORT_DIPNAME( 0x03, 0x03, DEF_STR( Lives ) )            PORT_DIPLOCATION("DSW1:1,2")
	PORT_DIPSETTING(    0x03, "3" )
	PORT_DIPSETTING(    0x02, "4" )
	PORT_DIPSETTING(    0x01, "5" )
	PORT_DIPSETTING(    0x00, "6" )
INPUT_PORTS_END

static INPUT_PORTS_START( flashgal )
	PORT_START("DSW1")
	PORT_DIPNAME( 0x03, 0x03, DEF_STR( Lives ) )            PORT_DIPLOCATION("DSW1:1,2")
	PORT_DIPSETTING(    0x03, "3" )
	PORT_DIPSETTING(    0x02, "4" )
	PORT_DIPSETTING(    0x01, "5" )
	PORT_DIPSETTING(    0x00, "6" )
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Bonus_Life ) )       PORT_DIPLOCATION("DSW1:3")
	PORT_DIPSETTING(    0x04, "Every 50000" )
	PORT_DIPSETTING(    0x00, "Every 70000" )
	PORT_DIPNAME( 0x08, 0x08, "Slow Motion" )               PORT_DIPLOCATION("DSW1:4")
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x00, DEF_STR( Allow_Continue ) )   PORT_DIPLOCATION("DSW1:5")
	PORT_DIPSETTING(    0x10, DEF_STR( No ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x20, 0x20, "Sound Test" )                PORT_DIPLOCATION("DSW1:6")
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x00, DEF_STR( Cabinet ) )          PORT_DIPLOCATION("DSW1:7")
	PORT_DIPSETTING(    0x00, DEF_STR( Upright ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Cocktail ) )
	PORT_DIPNAME( 0x80, 0x80, "Freeze" )                    PORT_DIPLOCATION("DSW1:8")
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_INCLUDE( common )
INPUT_PORTS_END

static INPUT_PORTS_START( srdmissn )
	PORT_START("DSW1")
	PORT_DIPNAME( 0x03, 0x03, DEF_STR( Lives ) )            PORT_DIPLOCATION("DSW1:1,2")
	PORT_DIPSETTING(    0x03, "3" )
	PORT_DIPSETTING(    0x02, "4" )
	PORT_DIPSETTING(    0x01, "5" )
	PORT_DIPSETTING(    0x00, "6" )
	PORT_DIPNAME( 0x04, 0x04, "Bonus Life/Continue" )       PORT_DIPLOCATION("DSW1:3")
	PORT_DIPSETTING(    0x04, "Every 50000/No" )
	PORT_DIPSETTING(    0x00, "Every 70000/Yes" )
	PORT_DIPNAME( 0x08, 0x08, "Slow Motion" )               PORT_DIPLOCATION("DSW1:4")
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME(0x10,  0x10, "Invulnerability (Cheat)")    PORT_DIPLOCATION("DSW1:5")
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, "Sound Test" )                PORT_DIPLOCATION("DSW1:6")
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x00, DEF_STR( Cabinet ) )          PORT_DIPLOCATION("DSW1:7")
	PORT_DIPSETTING(    0x00, DEF_STR( Upright ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Cocktail ) )
	PORT_DIPNAME( 0x80, 0x80, "Freeze" )                    PORT_DIPLOCATION("DSW1:8")
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_INCLUDE( common )
INPUT_PORTS_END

static INPUT_PORTS_START( legend )
	PORT_START("DSW1")
	PORT_DIPNAME( 0x03, 0x03, DEF_STR( Lives ) )            PORT_DIPLOCATION("DSW1:1,2")
	PORT_DIPSETTING(    0x03, "3" )
	PORT_DIPSETTING(    0x02, "4" )
	PORT_DIPSETTING(    0x01, "5" )
	PORT_DIPSETTING(    0x00, "6" )
	PORT_DIPNAME( 0x04, 0x04, "Bonus Life/Continue" )       PORT_DIPLOCATION("DSW1:3")
	PORT_DIPSETTING(    0x04, "Every 50000/No" )
	PORT_DIPSETTING(    0x00, "Every 70000/Yes" )
	PORT_DIPNAME( 0x08, 0x08, "Slow Motion" )               PORT_DIPLOCATION("DSW1:4")
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Unknown ) )          PORT_DIPLOCATION("DSW1:5") // probably unused
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, "Sound Test" )                PORT_DIPLOCATION("DSW1:6")
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x00, DEF_STR( Cabinet ) )          PORT_DIPLOCATION("DSW1:7")
	PORT_DIPSETTING(    0x00, DEF_STR( Upright ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Cocktail ) )
	PORT_DIPNAME( 0x80, 0x80, "Freeze" )                    PORT_DIPLOCATION("DSW1:8")
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_INCLUDE( common )
INPUT_PORTS_END



/*************************************
 *
 *  Graphics definitions
 *
 *************************************/

static const gfx_layout fg_tilelayout =
{
	8,8,
	RGN_FRAC(1,1),
	2,
	{ 0, 4 },
	{ 0, 1, 2, 3, 8*8+0, 8*8+1, 8*8+2, 8*8+3 },
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8 },
	8*8*2
};

static const gfx_layout bg_tilelayout =
{
	8,8,
	RGN_FRAC(1,3),
	3,
	{ RGN_FRAC(0,3), RGN_FRAC(1,3), RGN_FRAC(2,3) },
	{ 0, 1, 2, 3, 4, 5, 6, 7 },
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8 },
	8*8
};

static const gfx_layout spritelayout =
{
	16,16,
	RGN_FRAC(1,3),
	3,
	{ RGN_FRAC(0,3), RGN_FRAC(1,3), RGN_FRAC(2,3) },
	{     0,     1,     2,     3,     4,     5,     6,     7,
		8*8+0, 8*8+1, 8*8+2, 8*8+3, 8*8+4, 8*8+5, 8*8+6, 8*8+7 },
	{  0*8,  1*8,  2*8,  3*8,  4*8,  5*8,  6*8,  7*8,
		16*8, 17*8, 18*8, 19*8, 20*8, 21*8, 22*8, 23*8 },
	16*16
};

static GFXDECODE_START( gfx_kyugo )
	GFXDECODE_ENTRY( "gfx1", 0, fg_tilelayout, 0, 64 )
	GFXDECODE_ENTRY( "gfx2", 0, bg_tilelayout, 0, 32 )
	GFXDECODE_ENTRY( "gfx3", 0, spritelayout,  0, 32 )
GFXDECODE_END



/*************************************
 *
 *  Machine drivers
 *
 *************************************/

void kyugo_state::machine_start()
{
	save_item(NAME(m_scroll_x_lo));
	save_item(NAME(m_scroll_x_hi));
	save_item(NAME(m_scroll_y));
	save_item(NAME(m_bgpalbank));
	save_item(NAME(m_fgcolor));
}

void kyugo_state::machine_reset()
{
	m_scroll_x_lo = 0;
	m_scroll_x_hi = 0;
	m_scroll_y = 0;
	m_bgpalbank = 0;
	m_fgcolor = 0;
}

TIMER_DEVICE_CALLBACK_MEMBER(kyugo_state::interrupt)
{
	int scanline = param;

	// vblank interrupt
	if (scanline == 240 && m_nmi_mask)
		m_maincpu->pulse_input_line(INPUT_LINE_NMI, attotime::zero);

	// 4 sound interrupts per frame
	if ((scanline & 0x3f) == 0x20)
		m_subcpu->set_input_line(0, HOLD_LINE);
}


void kyugo_state::kyugo_base(machine_config &config)
{
	// basic machine hardware
	Z80(config, m_maincpu, 18.432_MHz_XTAL / 6); // verified on pcb
	m_maincpu->set_addrmap(AS_PROGRAM, &kyugo_state::kyugo_main_map);
	m_maincpu->set_addrmap(AS_IO, &kyugo_state::kyugo_main_portmap);
	TIMER(config, "scantimer").configure_scanline(FUNC(kyugo_state::interrupt), "screen", 0, 1);

	Z80(config, m_subcpu, 18.432_MHz_XTAL / 6); // verified on pcb
	m_subcpu->set_addrmap(AS_PROGRAM, &kyugo_state::gyrodine_sub_map);
	m_subcpu->set_addrmap(AS_IO, &kyugo_state::gyrodine_sub_portmap);

	config.set_maximum_quantum(attotime::from_hz(6000));

	ls259_device &mainlatch(LS259(config, "mainlatch"));
	mainlatch.q_out_cb<0>().set(FUNC(kyugo_state::nmi_mask_w));
	mainlatch.q_out_cb<1>().set(FUNC(kyugo_state::flip_screen_set));
	mainlatch.q_out_cb<2>().set_inputline(m_subcpu, INPUT_LINE_RESET).invert();

	// video hardware
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_raw(18.432_MHz_XTAL / 3, 396, 0, 288, 260, 16, 240);
	screen.set_screen_update(FUNC(kyugo_state::screen_update));
	screen.set_palette(m_palette);

	GFXDECODE(config, m_gfxdecode, m_palette, gfx_kyugo);
	PALETTE(config, m_palette, palette_device::RGB_444_PROMS, "proms", 256);

	// sound hardware
	SPEAKER(config, "mono").front_center();

	ay8910_device &ay1(AY8910(config, "ay1", 18.432_MHz_XTAL / 12)); // verified on pcb
	ay1.port_a_read_callback().set_ioport("DSW1");
	ay1.port_b_read_callback().set_ioport("DSW2");
	ay1.add_route(ALL_OUTPUTS, "mono", 0.30);

	AY8910(config, "ay2", 18.432_MHz_XTAL / 12).add_route(ALL_OUTPUTS, "mono", 0.30); // verified on pcb
}

void kyugo_state::gyrodine(machine_config &config)
{
	kyugo_base(config);

	// add watchdog
	WATCHDOG_TIMER(config, "watchdog");
	m_maincpu->set_addrmap(AS_PROGRAM, &kyugo_state::gyrodine_main_map);
}

void kyugo_state::repulse(machine_config &config)
{
	kyugo_base(config);

	// basic machine hardware
	m_subcpu->set_addrmap(AS_PROGRAM, &kyugo_state::repulse_sub_map);
	m_subcpu->set_addrmap(AS_IO, &kyugo_state::repulse_sub_portmap);
}

void kyugo_state::srdmissn(machine_config &config)
{
	kyugo_base(config);

	// basic machine hardware
	m_maincpu->set_addrmap(AS_PROGRAM, &kyugo_state::srdmissn_main_map);
	m_subcpu->set_addrmap(AS_PROGRAM, &kyugo_state::srdmissn_sub_map);
	m_subcpu->set_addrmap(AS_IO, &kyugo_state::srdmissn_sub_portmap);
}

void kyugo_state::flashgala(machine_config &config)
{
	kyugo_base(config);

	// basic machine hardware
	m_subcpu->set_addrmap(AS_PROGRAM, &kyugo_state::flashgala_sub_map);
	m_subcpu->set_addrmap(AS_IO, &kyugo_state::flashgala_sub_portmap);
}

void kyugo_state::legend(machine_config &config)
{
	kyugo_base(config);

	// basic machine hardware
	m_subcpu->set_addrmap(AS_PROGRAM, &kyugo_state::legend_sub_map);
	m_subcpu->set_addrmap(AS_IO, &kyugo_state::srdmissn_sub_portmap);
}



/*************************************
 *
 *  ROM definitions
 *
 *************************************/

ROM_START( gyrodine )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "rom2",   0x0000, 0x2000, CRC(85ddea38) SHA1(fe7e8d7962850b17c39cac627994d78768b094f8) )
	ROM_LOAD( "a21.03", 0x2000, 0x2000, CRC(4e9323bd) SHA1(86ae4c6a29898fdb0e559ec2aac99fc874910fea) )
	ROM_LOAD( "a21.04", 0x4000, 0x2000, CRC(57e659d4) SHA1(4c0e73d0661360731691a32a6e94f41b69315f93) )
	ROM_LOAD( "a21.05", 0x6000, 0x2000, CRC(1e7293f3) SHA1(64695b80b409b02314334fb325f4d0c42a6d4d5b) )

	ROM_REGION( 0x10000, "sub", 0 )
	ROM_LOAD( "a21.01", 0x0000, 0x2000, CRC(b2ce0aa2) SHA1(576754105819aec64781a5c8e8540b21fcfd346b) )

	ROM_REGION( 0x01000, "gfx1", 0 )
	ROM_LOAD( "a21.15", 0x00000, 0x1000, CRC(adba18d0) SHA1(b1afd7d8f2a8545a00525a23e087d9ca975a6401) ) // chars

	ROM_REGION( 0x06000, "gfx2", 0 )
	ROM_LOAD( "a21.08", 0x00000, 0x2000, CRC(a57df1c9) SHA1(63505f63e978c52c43fe863dca056b52f7ebd501) ) // tiles - plane 0
	ROM_LOAD( "a21.07", 0x02000, 0x2000, CRC(63623ba3) SHA1(bcb80fc0edf7c4d1f82a2ff6d0bad9d2ccaf48c6) ) // tiles - plane 1
	ROM_LOAD( "a21.06", 0x04000, 0x2000, CRC(4cc969a9) SHA1(0b89f4142c2fcc0a882fbc3514d8d11027e78e01) ) // tiles - plane 2

	ROM_REGION( 0x18000, "gfx3", 0 )
	ROM_LOAD( "a21.14", 0x00000, 0x2000, CRC(9c5c4d5b) SHA1(0d0c9efb63b761acfaf51d17c525f94430ad703d) ) // sprites - plane 0
	// 0x03000-0x04fff empty
	ROM_LOAD( "a21.13", 0x04000, 0x2000, CRC(d36b5aad) SHA1(8fac23474ffd3a62e2283eadbc8d278cd9c70105) ) // sprites - plane 0
	// 0x07000-0x08fff empty
	ROM_LOAD( "a21.12", 0x08000, 0x2000, CRC(f387aea2) SHA1(de4aad7ad9ecc6a058b88c67bb18ee02605d9951) ) // sprites - plane 1
	// 0x0b000-0x0cfff empty
	ROM_LOAD( "a21.11", 0x0c000, 0x2000, CRC(87967d7d) SHA1(d8026df749947a16f643d9b28640c9d293edd4a7) ) // sprites - plane 1
	// 0x0f000-0x10fff empty
	ROM_LOAD( "a21.10", 0x10000, 0x2000, CRC(59640ab4) SHA1(747cb265f4504399837111c0dd48f07e05a57cc4) ) // sprites - plane 2
	// 0x13000-0x14fff empty
	ROM_LOAD( "a21.09", 0x14000, 0x2000, CRC(22ad88d8) SHA1(3bdf93ca582d7454fc9e70bd6ce3cd076e0762aa) ) // sprites - plane 2
	// 0x17000-0x18fff empty

	ROM_REGION( 0x0340, "proms", 0 )
	ROM_LOAD( "a21.16", 0x0000, 0x0100, CRC(cc25fb56) SHA1(5f533c4b4f49ba147c83d6a20d1e795c71db3c41) ) // red
	ROM_LOAD( "a21.17", 0x0100, 0x0100, CRC(ca054448) SHA1(4bad8147905cbe9ec8bb5bcd8016e9950c5d95a9) ) // green
	ROM_LOAD( "a21.18", 0x0200, 0x0100, CRC(23c0c449) SHA1(4a37821a6a16ae0cfdcfb0fa64733c03ba9e4815) ) // blue
	ROM_LOAD( "a21.20", 0x0300, 0x0020, CRC(efc4985e) SHA1(b2fa02e388fbbe1077e79699efccb2d47cb83ba5) ) // char lookup table
	ROM_LOAD( "m1.2c",  0x0320, 0x0020, CRC(83a39201) SHA1(4fdc722c9e20ee152c890342ef0dce18e35e2ef8) ) // timing? (not used)
ROM_END

ROM_START( gyrodinet )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "a21.02", 0x0000, 0x2000, CRC(c5ec4a50) SHA1(4d012aabdc248143a4d3bab190ecb6e335c93427) )
	ROM_LOAD( "a21.03", 0x2000, 0x2000, CRC(4e9323bd) SHA1(86ae4c6a29898fdb0e559ec2aac99fc874910fea) )
	ROM_LOAD( "a21.04", 0x4000, 0x2000, CRC(57e659d4) SHA1(4c0e73d0661360731691a32a6e94f41b69315f93) )
	ROM_LOAD( "a21.05", 0x6000, 0x2000, CRC(1e7293f3) SHA1(64695b80b409b02314334fb325f4d0c42a6d4d5b) )

	ROM_REGION( 0x10000, "sub", 0 )
	ROM_LOAD( "a21.01", 0x0000, 0x2000, CRC(b2ce0aa2) SHA1(576754105819aec64781a5c8e8540b21fcfd346b) )

	ROM_REGION( 0x01000, "gfx1", 0 )
	ROM_LOAD( "a21.15", 0x00000, 0x1000, CRC(adba18d0) SHA1(b1afd7d8f2a8545a00525a23e087d9ca975a6401) ) // chars

	ROM_REGION( 0x06000, "gfx2", 0 )
	ROM_LOAD( "a21.08", 0x00000, 0x2000, CRC(a57df1c9) SHA1(63505f63e978c52c43fe863dca056b52f7ebd501) ) // tiles - plane 0
	ROM_LOAD( "a21.07", 0x02000, 0x2000, CRC(63623ba3) SHA1(bcb80fc0edf7c4d1f82a2ff6d0bad9d2ccaf48c6) ) // tiles - plane 1
	ROM_LOAD( "a21.06", 0x04000, 0x2000, CRC(4cc969a9) SHA1(0b89f4142c2fcc0a882fbc3514d8d11027e78e01) ) // tiles - plane 2

	ROM_REGION( 0x18000, "gfx3", 0 )
	ROM_LOAD( "a21.14", 0x00000, 0x2000, CRC(9c5c4d5b) SHA1(0d0c9efb63b761acfaf51d17c525f94430ad703d) ) // sprites - plane 0
	// 0x03000-0x04fff empty
	ROM_LOAD( "a21.13", 0x04000, 0x2000, CRC(d36b5aad) SHA1(8fac23474ffd3a62e2283eadbc8d278cd9c70105) ) // sprites - plane 0
	// 0x07000-0x08fff empty
	ROM_LOAD( "a21.12", 0x08000, 0x2000, CRC(f387aea2) SHA1(de4aad7ad9ecc6a058b88c67bb18ee02605d9951) ) // sprites - plane 1
	// 0x0b000-0x0cfff empty
	ROM_LOAD( "a21.11", 0x0c000, 0x2000, CRC(87967d7d) SHA1(d8026df749947a16f643d9b28640c9d293edd4a7) ) // sprites - plane 1
	// 0x0f000-0x10fff empty
	ROM_LOAD( "a21.10", 0x10000, 0x2000, CRC(59640ab4) SHA1(747cb265f4504399837111c0dd48f07e05a57cc4) ) // sprites - plane 2
	// 0x13000-0x14fff empty
	ROM_LOAD( "a21.09", 0x14000, 0x2000, CRC(22ad88d8) SHA1(3bdf93ca582d7454fc9e70bd6ce3cd076e0762aa) ) // sprites - plane 2
	// 0x17000-0x18fff empty

	ROM_REGION( 0x0340, "proms", 0 )
	ROM_LOAD( "a21.16", 0x0000, 0x0100, CRC(cc25fb56) SHA1(5f533c4b4f49ba147c83d6a20d1e795c71db3c41) ) // red
	ROM_LOAD( "a21.17", 0x0100, 0x0100, CRC(ca054448) SHA1(4bad8147905cbe9ec8bb5bcd8016e9950c5d95a9) ) // green
	ROM_LOAD( "a21.18", 0x0200, 0x0100, CRC(23c0c449) SHA1(4a37821a6a16ae0cfdcfb0fa64733c03ba9e4815) ) // blue
	ROM_LOAD( "a21.20", 0x0300, 0x0020, CRC(efc4985e) SHA1(b2fa02e388fbbe1077e79699efccb2d47cb83ba5) ) // char lookup table
	ROM_LOAD( "m1.2c",  0x0320, 0x0020, CRC(83a39201) SHA1(4fdc722c9e20ee152c890342ef0dce18e35e2ef8) ) // timing? (not used)
ROM_END

ROM_START( buzzard )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "rom2",       0x0000, 0x2000, CRC(85ddea38) SHA1(fe7e8d7962850b17c39cac627994d78768b094f8) )
	ROM_LOAD( "a21.03",     0x2000, 0x2000, CRC(4e9323bd) SHA1(86ae4c6a29898fdb0e559ec2aac99fc874910fea) )
	ROM_LOAD( "a21.04",     0x4000, 0x2000, CRC(57e659d4) SHA1(4c0e73d0661360731691a32a6e94f41b69315f93) )
	ROM_LOAD( "a21.05",     0x6000, 0x2000, CRC(1e7293f3) SHA1(64695b80b409b02314334fb325f4d0c42a6d4d5b) )

	ROM_REGION( 0x10000, "sub", 0 )
	ROM_LOAD( "a21.01",     0x0000, 0x2000, CRC(b2ce0aa2) SHA1(576754105819aec64781a5c8e8540b21fcfd346b) )

	ROM_REGION( 0x01000, "gfx1", 0 )
	ROM_LOAD( "buzl01.bin", 0x00000, 0x1000, CRC(65d728d0) SHA1(12da6cd2c9a8acca98c194dac6dd0edd6384118c) ) // chars

	ROM_REGION( 0x06000, "gfx2", 0 )
	ROM_LOAD( "a21.08",     0x00000, 0x2000, CRC(a57df1c9) SHA1(63505f63e978c52c43fe863dca056b52f7ebd501) ) // tiles - plane 0
	ROM_LOAD( "a21.07",     0x02000, 0x2000, CRC(63623ba3) SHA1(bcb80fc0edf7c4d1f82a2ff6d0bad9d2ccaf48c6) ) // tiles - plane 1
	ROM_LOAD( "a21.06",     0x04000, 0x2000, CRC(4cc969a9) SHA1(0b89f4142c2fcc0a882fbc3514d8d11027e78e01) ) // tiles - plane 2

	ROM_REGION( 0x18000, "gfx3", 0 )
	ROM_LOAD( "a21.14",     0x00000, 0x2000, CRC(9c5c4d5b) SHA1(0d0c9efb63b761acfaf51d17c525f94430ad703d) ) // sprites - plane 0
	// 0x03000-0x04fff empty
	ROM_LOAD( "a21.13",     0x04000, 0x2000, CRC(d36b5aad) SHA1(8fac23474ffd3a62e2283eadbc8d278cd9c70105) ) // sprites - plane 0
	// 0x07000-0x08fff empty
	ROM_LOAD( "a21.12",     0x08000, 0x2000, CRC(f387aea2) SHA1(de4aad7ad9ecc6a058b88c67bb18ee02605d9951) ) // sprites - plane 1
	// 0x0b000-0x0cfff empty
	ROM_LOAD( "a21.11",     0x0c000, 0x2000, CRC(87967d7d) SHA1(d8026df749947a16f643d9b28640c9d293edd4a7) ) // sprites - plane 1
	// 0x0f000-0x10fff empty
	ROM_LOAD( "a21.10",     0x10000, 0x2000, CRC(59640ab4) SHA1(747cb265f4504399837111c0dd48f07e05a57cc4) ) // sprites - plane 2
	// 0x13000-0x14fff empty
	ROM_LOAD( "a21.09",     0x14000, 0x2000, CRC(22ad88d8) SHA1(3bdf93ca582d7454fc9e70bd6ce3cd076e0762aa) ) // sprites - plane 2
	// 0x17000-0x18fff empty

	ROM_REGION( 0x0340, "proms", 0 )
	ROM_LOAD( "a21.16",     0x0000, 0x0100, CRC(cc25fb56) SHA1(5f533c4b4f49ba147c83d6a20d1e795c71db3c41) ) // red
	ROM_LOAD( "a21.17",     0x0100, 0x0100, CRC(ca054448) SHA1(4bad8147905cbe9ec8bb5bcd8016e9950c5d95a9) ) // green
	ROM_LOAD( "a21.18",     0x0200, 0x0100, CRC(23c0c449) SHA1(4a37821a6a16ae0cfdcfb0fa64733c03ba9e4815) ) // blue
	ROM_LOAD( "a21.20",     0x0300, 0x0020, CRC(efc4985e) SHA1(b2fa02e388fbbe1077e79699efccb2d47cb83ba5) ) // char lookup table
	ROM_LOAD( "m1.2c",      0x0320, 0x0020, CRC(83a39201) SHA1(4fdc722c9e20ee152c890342ef0dce18e35e2ef8) ) // timing? (not used)
ROM_END

ROM_START( repulse )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "repulse.b5",   0x0000, 0x2000, CRC(fb2b7c9d) SHA1(7a6867a9deda7eb399bf5b01c5422400d443faea) )
	ROM_LOAD( "repulse.b6",   0x2000, 0x2000, CRC(99129918) SHA1(9beba6ef62102d6a28cf7a52ce5ce2a2113f8dfc) )
	ROM_LOAD( "7.j4",         0x4000, 0x2000, CRC(57a8e900) SHA1(bc878e27130f0a9afb50c1926b47621e5e58d8b2) )

	ROM_REGION( 0x10000, "sub" , 0 )
	ROM_LOAD( "1.f2",         0x0000, 0x2000, CRC(c485c621) SHA1(14fa1b1403f4f2513e1a824f79b750cedf24a31e) )
	ROM_LOAD( "2.h2",         0x2000, 0x2000, CRC(b3c6a886) SHA1(efb136fc1671092fabc2fb2aff189a61bac90ca4) )
	ROM_LOAD( "3.j2",         0x4000, 0x2000, CRC(197e314c) SHA1(6921cd1bc3571b0ac7d8d7eb19b256daca85f17e) )
	ROM_LOAD( "repulse.b4",   0x6000, 0x2000, CRC(86b267f3) SHA1(5e352737e0ea0ca4a025d002b75c821c55660b4f) )

	ROM_REGION( 0x01000, "gfx1", 0 )
	ROM_LOAD( "repulse.a11",  0x00000, 0x1000, CRC(8e1de90a) SHA1(5e655e6d282f6c8ae8bdfb72db64212e9262f717) ) // chars

	ROM_REGION( 0x06000, "gfx2", 0 )
	ROM_LOAD( "15.9h",        0x00000, 0x2000, CRC(c9213469) SHA1(03bd7a86f9cbb28ecf39e7ac643a186cfeb38a35) ) // tiles - plane 0
	ROM_LOAD( "16.10h",       0x02000, 0x2000, CRC(7de5d39e) SHA1(47fc5740a972e105d282873b4d72774a4405dfff) ) // tiles - plane 1
	ROM_LOAD( "17.11h",       0x04000, 0x2000, CRC(0ba5f72c) SHA1(79292e16e2f6079f160d957a22e355457599669d) ) // tiles - plane 2

	ROM_REGION( 0x18000, "gfx3", 0 )
	ROM_LOAD( "8.6a",         0x00000, 0x4000, CRC(0e9f757e) SHA1(1d4a46b3f18fe5099cdc889ba5e55c1d171a0430) ) // sprites - plane 0
	ROM_LOAD( "9.7a",         0x04000, 0x4000, CRC(f7d2e650) SHA1(eac715e09ad22b1a1d18e5cade4955cb8d4156f4) ) // sprites - plane 0
	ROM_LOAD( "10.8a",        0x08000, 0x4000, CRC(e717baf4) SHA1(d52a6c5f8b915769cc6dfb50d34922c1a3cd1333) ) // sprites - plane 1
	ROM_LOAD( "11.9a",        0x0c000, 0x4000, CRC(04b2250b) SHA1(d9948277d3ba3cb8188de647e25848f5222d066a) ) // sprites - plane 1
	ROM_LOAD( "12.10a",       0x10000, 0x4000, CRC(d110e140) SHA1(eb528b437e7967ecbe56de51274f286e563f7100) ) // sprites - plane 2
	ROM_LOAD( "13.11a",       0x14000, 0x4000, CRC(8fdc713c) SHA1(c8933d1c45c886c22ee89d02b8941bbbb963d7b1) ) // sprites - plane 2

	ROM_REGION( 0x0340, "proms", 0 )
	ROM_LOAD( "b.1j",         0x0000, 0x0100, CRC(3ea35431) SHA1(b45318ce898f03a338435a3f6109483d246ff914) ) // blue
	ROM_LOAD( "g.1h",         0x0100, 0x0100, CRC(acd7a69e) SHA1(b18eab8f669f0a8105a4bbffa346c4b19491c451) ) // green
	ROM_LOAD( "r.1f",         0x0200, 0x0100, CRC(b7f48b41) SHA1(2d84dc29c0ab43729014129e6392207db0f56e9e) ) // red
	// 0x0300-0x031f empty - looks like there isn't a lookup table PROM or it wasn't dumped
	ROM_LOAD( "m1.2c",        0x0320, 0x0020, CRC(83a39201) SHA1(4fdc722c9e20ee152c890342ef0dce18e35e2ef8) ) // timing? (not used)
ROM_END

ROM_START( 99lstwar )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "1999.4f",      0x0000, 0x2000, CRC(e3cfc09f) SHA1(e48905726c6308194c596117dd30444dcb748908) )
	ROM_LOAD( "1999.4h",      0x2000, 0x2000, CRC(fd58c6e1) SHA1(005f3114425fd2bfb9452c790d40653661b3d1d9) )
	ROM_LOAD( "7.j4",         0x4000, 0x2000, CRC(57a8e900) SHA1(bc878e27130f0a9afb50c1926b47621e5e58d8b2) )

	ROM_REGION( 0x10000, "sub", 0 )
	ROM_LOAD( "1.f2",         0x0000, 0x2000, CRC(c485c621) SHA1(14fa1b1403f4f2513e1a824f79b750cedf24a31e) )
	ROM_LOAD( "2.h2",         0x2000, 0x2000, CRC(b3c6a886) SHA1(efb136fc1671092fabc2fb2aff189a61bac90ca4) )
	ROM_LOAD( "3.j2",         0x4000, 0x2000, CRC(197e314c) SHA1(6921cd1bc3571b0ac7d8d7eb19b256daca85f17e) )
	ROM_LOAD( "repulse.b4",   0x6000, 0x2000, CRC(86b267f3) SHA1(5e352737e0ea0ca4a025d002b75c821c55660b4f) )

	ROM_REGION( 0x01000, "gfx1", 0 )
	ROM_LOAD( "1999.4a",      0x00000, 0x1000, CRC(49a2383e) SHA1(b4be929abbde034df5ef12342fbcecb14772886a) ) // chars

	ROM_REGION( 0x06000, "gfx2", 0 )
	ROM_LOAD( "15.9h",        0x00000, 0x2000, CRC(c9213469) SHA1(03bd7a86f9cbb28ecf39e7ac643a186cfeb38a35) ) // tiles - plane 0
	ROM_LOAD( "16.10h",       0x02000, 0x2000, CRC(7de5d39e) SHA1(47fc5740a972e105d282873b4d72774a4405dfff) ) // tiles - plane 1
	ROM_LOAD( "17.11h",       0x04000, 0x2000, CRC(0ba5f72c) SHA1(79292e16e2f6079f160d957a22e355457599669d) ) // tiles - plane 2

	ROM_REGION( 0x18000, "gfx3", 0 )
	ROM_LOAD( "8.6a",         0x00000, 0x4000, CRC(0e9f757e) SHA1(1d4a46b3f18fe5099cdc889ba5e55c1d171a0430) ) // sprites - plane 0
	ROM_LOAD( "9.7a",         0x04000, 0x4000, CRC(f7d2e650) SHA1(eac715e09ad22b1a1d18e5cade4955cb8d4156f4) ) // sprites - plane 0
	ROM_LOAD( "10.8a",        0x08000, 0x4000, CRC(e717baf4) SHA1(d52a6c5f8b915769cc6dfb50d34922c1a3cd1333) ) // sprites - plane 1
	ROM_LOAD( "11.9a",        0x0c000, 0x4000, CRC(04b2250b) SHA1(d9948277d3ba3cb8188de647e25848f5222d066a) ) // sprites - plane 1
	ROM_LOAD( "12.10a",       0x10000, 0x4000, CRC(d110e140) SHA1(eb528b437e7967ecbe56de51274f286e563f7100) ) // sprites - plane 2
	ROM_LOAD( "13.11a",       0x14000, 0x4000, CRC(8fdc713c) SHA1(c8933d1c45c886c22ee89d02b8941bbbb963d7b1) ) // sprites - plane 2

	ROM_REGION( 0x0340, "proms", 0 )
	ROM_LOAD( "b.1j",         0x0000, 0x0100, CRC(3ea35431) SHA1(b45318ce898f03a338435a3f6109483d246ff914) ) // blue
	ROM_LOAD( "g.1h",         0x0100, 0x0100, CRC(acd7a69e) SHA1(b18eab8f669f0a8105a4bbffa346c4b19491c451) ) // green
	ROM_LOAD( "r.1f",         0x0200, 0x0100, CRC(b7f48b41) SHA1(2d84dc29c0ab43729014129e6392207db0f56e9e) ) // red
	ROM_LOAD( "n82s123n.5j",  0x0300, 0x0020, CRC(cce2e29f) SHA1(787c65b7d69bcd224b45138fdbbf3fdae296dda6) ) // char lookup table
	ROM_LOAD( "m1.2c",        0x0320, 0x0020, CRC(83a39201) SHA1(4fdc722c9e20ee152c890342ef0dce18e35e2ef8) ) // timing? (not used)

	ROM_REGION( 0x400, "plds", 0 )
	ROM_LOAD( "pal12l6.4m",   0x000, 0x117, CRC(b52fbcc0) SHA1(68d9cb70fb5945fc7002f4e506efa82944dfa6d8) )
	ROM_LOAD( "pal12l6.5n",   0x200, 0x117, CRC(453ce64a) SHA1(abce1251b66befcb53b574e5c3a8b14335c0977f) )
ROM_END

ROM_START( 99lstwara )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "4f.bin",       0x0000, 0x2000, CRC(efe2908d) SHA1(4de8661f523f002c6a9368f81d865c7cc98926dd) )
	ROM_LOAD( "4h.bin",       0x2000, 0x2000, CRC(5b79c342) SHA1(293990dab3360139727a5c90aad0826d4a3746b7) )
	ROM_LOAD( "4j.bin",       0x4000, 0x2000, CRC(d2a62c1b) SHA1(eef9103945db8cfc4c1e3a58d8ad222f8dc58492) )

	ROM_REGION( 0x10000, "sub", 0 )
	ROM_LOAD( "2f.bin",       0x0000, 0x2000, CRC(cb9d8291) SHA1(f26687edda70a8678708b14f4eb4dfd1b3cb8582) )
	ROM_LOAD( "2h.bin",       0x2000, 0x2000, CRC(24dbddc3) SHA1(76aa9e8b59747436650b6629ff5acea81d1d76da) )
	ROM_LOAD( "2j.bin",       0x4000, 0x2000, CRC(16879c4c) SHA1(5195abdd5ad8cf9aa081f96ed5d16c14af603289) )
	ROM_LOAD( "repulse.b4",   0x6000, 0x2000, CRC(86b267f3) SHA1(5e352737e0ea0ca4a025d002b75c821c55660b4f) )

	ROM_REGION( 0x01000, "gfx1", 0 )
	ROM_LOAD( "1999.4a",      0x00000, 0x1000, CRC(49a2383e) SHA1(b4be929abbde034df5ef12342fbcecb14772886a) ) // chars

	ROM_REGION( 0x06000, "gfx2", 0 )
	ROM_LOAD( "9h.bin",       0x00000, 0x2000, CRC(59993c27) SHA1(e7de7fbea4f09718f0a13d4a7f60360e6431c3a7) ) // tiles - plane 0
	ROM_LOAD( "10h.bin",      0x02000, 0x2000, CRC(dfbf0280) SHA1(c97923fcdd01bdcfbbc6308b04de8bb610e9b5d2) ) // tiles - plane 1
	ROM_LOAD( "11h.bin",      0x04000, 0x2000, CRC(e4f29fc0) SHA1(8ef393d5292b0eb1d6c253589e37b3ab2eaeb402) ) // tiles - plane 2

	ROM_REGION( 0x18000, "gfx3", 0 )
	ROM_LOAD( "6a.bin",       0x00000, 0x4000, CRC(98d44410) SHA1(3d3ca9d2d28038402daedc6b9c01977a1897d8b2) ) // sprites - plane 0
	ROM_LOAD( "7a.bin",       0x04000, 0x4000, CRC(4c54d281) SHA1(be7d14d2a8910e0b38d5614c1eba5e71a6bf7a6c) ) // sprites - plane 0
	ROM_LOAD( "8a.bin",       0x08000, 0x4000, CRC(81018101) SHA1(fa94b6bc07beb70bd6535c3f5620897c2ad49240) ) // sprites - plane 1
	ROM_LOAD( "9a.bin",       0x0c000, 0x4000, CRC(347b91fd) SHA1(6ae29d2c075c72d3435d0f3cc095e0bf9657f36b) ) // sprites - plane 1
	ROM_LOAD( "10a.bin",      0x10000, 0x4000, CRC(f07de4fa) SHA1(4b2e8386634205c84d8d32febd57efdb93d86e99) ) // sprites - plane 2
	ROM_LOAD( "11a.bin",      0x14000, 0x4000, CRC(34a04f48) SHA1(6c3f735469a6d97b6aaece69c955c4bd5e324c49) ) // sprites - plane 2

	ROM_REGION( 0x0340, "proms", 0 )
	ROM_LOAD( "b.1j",         0x0000, 0x0100, CRC(3ea35431) SHA1(b45318ce898f03a338435a3f6109483d246ff914) ) // blue
	ROM_LOAD( "g.1h",         0x0100, 0x0100, CRC(acd7a69e) SHA1(b18eab8f669f0a8105a4bbffa346c4b19491c451) ) // green
	ROM_LOAD( "r.1f",         0x0200, 0x0100, CRC(b7f48b41) SHA1(2d84dc29c0ab43729014129e6392207db0f56e9e) ) // red
	ROM_LOAD( "n82s123n.5j",  0x0300, 0x0020, CRC(cce2e29f) SHA1(787c65b7d69bcd224b45138fdbbf3fdae296dda6) ) // char lookup table
	ROM_LOAD( "m1.2c",        0x0320, 0x0020, CRC(83a39201) SHA1(4fdc722c9e20ee152c890342ef0dce18e35e2ef8) ) // timing? (not used)

	ROM_REGION( 0x400, "plds", 0 )
	ROM_LOAD( "pal12l6.4m",   0x000, 0x117, CRC(b52fbcc0) SHA1(68d9cb70fb5945fc7002f4e506efa82944dfa6d8) )
	ROM_LOAD( "pal12l6.5n",   0x200, 0x117, CRC(453ce64a) SHA1(abce1251b66befcb53b574e5c3a8b14335c0977f) )
ROM_END

ROM_START( 99lstwark )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "88.4f",        0x0000, 0x2000, CRC(e3cfc09f) SHA1(e48905726c6308194c596117dd30444dcb748908) )
	ROM_LOAD( "89.4h",        0x2000, 0x2000, CRC(fd58c6e1) SHA1(005f3114425fd2bfb9452c790d40653661b3d1d9) )
	ROM_LOAD( "90.j4",        0x4000, 0x2000, CRC(57a8e900) SHA1(bc878e27130f0a9afb50c1926b47621e5e58d8b2) )

	ROM_REGION( 0x10000, "sub", 0 )
	ROM_LOAD( "84.f2",        0x0000, 0x2000, CRC(c485c621) SHA1(14fa1b1403f4f2513e1a824f79b750cedf24a31e) )
	ROM_LOAD( "85.h2",        0x2000, 0x2000, CRC(b3c6a886) SHA1(efb136fc1671092fabc2fb2aff189a61bac90ca4) )
	ROM_LOAD( "86.j2",        0x4000, 0x2000, CRC(197e314c) SHA1(6921cd1bc3571b0ac7d8d7eb19b256daca85f17e) )
	ROM_LOAD( "87.k2",        0x6000, 0x2000, CRC(86b267f3) SHA1(5e352737e0ea0ca4a025d002b75c821c55660b4f) )

	ROM_REGION( 0x02000, "gfx1", 0 )
	ROM_LOAD( "97.4a",        0x0000, 0x1000, CRC(15ad6867) SHA1(0ef7d8f70adf09f2f2e40ac3b8301d024179d8d1) ) // chars, 1st and 2nd half identical verified on 2 PCBs
	ROM_IGNORE(0x1000)

	ROM_REGION( 0x06000, "gfx2", 0 )
	ROM_LOAD( "98.9h",        0x00000, 0x2000, CRC(c9213469) SHA1(03bd7a86f9cbb28ecf39e7ac643a186cfeb38a35) ) // tiles - plane 0
	ROM_LOAD( "99.10h",       0x02000, 0x2000, CRC(7de5d39e) SHA1(47fc5740a972e105d282873b4d72774a4405dfff) ) // tiles - plane 1
	ROM_LOAD( "00.11h",       0x04000, 0x2000, CRC(0ba5f72c) SHA1(79292e16e2f6079f160d957a22e355457599669d) ) // tiles - plane 2

	ROM_REGION( 0x18000, "gfx3", 0 )
	ROM_LOAD( "91.6a",        0x00000, 0x4000, CRC(0e9f757e) SHA1(1d4a46b3f18fe5099cdc889ba5e55c1d171a0430) ) // sprites - plane 0
	ROM_LOAD( "92.7a",        0x04000, 0x4000, CRC(f7d2e650) SHA1(eac715e09ad22b1a1d18e5cade4955cb8d4156f4) ) // sprites - plane 0
	ROM_LOAD( "93.8a",        0x08000, 0x4000, CRC(e717baf4) SHA1(d52a6c5f8b915769cc6dfb50d34922c1a3cd1333) ) // sprites - plane 1
	ROM_LOAD( "94.9a",        0x0c000, 0x4000, CRC(04b2250b) SHA1(d9948277d3ba3cb8188de647e25848f5222d066a) ) // sprites - plane 1
	ROM_LOAD( "95.10a",       0x10000, 0x4000, CRC(d110e140) SHA1(eb528b437e7967ecbe56de51274f286e563f7100) ) // sprites - plane 2
	ROM_LOAD( "96.11a",       0x14000, 0x4000, CRC(8fdc713c) SHA1(c8933d1c45c886c22ee89d02b8941bbbb963d7b1) ) // sprites - plane 2

	ROM_REGION( 0x0340, "proms", 0 )
	ROM_LOAD( "b.1j",         0x0000, 0x0100, CRC(3ea35431) SHA1(b45318ce898f03a338435a3f6109483d246ff914) ) // blue
	ROM_LOAD( "g.1h",         0x0100, 0x0100, CRC(acd7a69e) SHA1(b18eab8f669f0a8105a4bbffa346c4b19491c451) ) // green
	ROM_LOAD( "r.1f",         0x0200, 0x0100, CRC(b7f48b41) SHA1(2d84dc29c0ab43729014129e6392207db0f56e9e) ) // red
	ROM_LOAD( "n82s123n.5j",  0x0300, 0x0020, CRC(cce2e29f) SHA1(787c65b7d69bcd224b45138fdbbf3fdae296dda6) ) // char lookup table
	ROM_LOAD( "m1.2c",        0x0320, 0x0020, CRC(83a39201) SHA1(4fdc722c9e20ee152c890342ef0dce18e35e2ef8) ) // timing? (not used)

	ROM_REGION( 0x0800, "user1", 0 )
	ROM_LOAD( "1999-00.rom",  0x0000, 0x0800, CRC(0c0c449f) SHA1(efa4a8ac4c341ca5cdc3b5d2803eda43daf1bc93) ) // unknown

	ROM_REGION( 0x400, "plds", 0 )
	ROM_LOAD( "pal12l6.4m",   0x000, 0x117, CRC(b52fbcc0) SHA1(68d9cb70fb5945fc7002f4e506efa82944dfa6d8) )
	ROM_LOAD( "pal12l6.5n",   0x200, 0x117, CRC(453ce64a) SHA1(abce1251b66befcb53b574e5c3a8b14335c0977f) )
ROM_END

ROM_START( 99lstwarb ) // copyright blanked, seems based on 99lstwara, given it has a second shot type instead of the shields.
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "15.2764",      0x0000, 0x2000, CRC(f9367b9d) SHA1(08157add7a72208b273faec1dbb8cb2ef21f2438) )
	ROM_LOAD( "16.2764",      0x2000, 0x2000, CRC(04c3316a) SHA1(2db4ab98c8ac6c3eea1930c9dbbe6dd15b8e8a74) )
	ROM_LOAD( "17.2764",      0x4000, 0x2000, CRC(02aa4de5) SHA1(bab79e314764f4a1e05516e22bd1077476e53f5b) )

	ROM_REGION( 0x10000, "sub", 0 )
	ROM_LOAD( "11.2764",      0x0000, 0x2000, CRC(aa3e0996) SHA1(8e92edfc933be0b9e23991dbad686fb0c0286321) )
	ROM_LOAD( "12.2764",      0x2000, 0x2000, CRC(a59d3d1b) SHA1(6f0eee9d89cbe667f02a787b0ce16563c3b48e3f) )
	ROM_LOAD( "13.2764",      0x4000, 0x2000, CRC(fe31975e) SHA1(904d3fb8bf3b31152c18095234b5f62589c5b18c) )
	ROM_LOAD( "14.2764",      0x6000, 0x2000, CRC(683481a5) SHA1(56d52194a4df3712fd9b16b2c0029d565e8c8bee) )

	ROM_REGION( 0x01000, "gfx1", 0 )
	ROM_LOAD( "1.2732",      0x00000, 0x1000, CRC(8ed6855b) SHA1(9f3737162e63ba0e05e5b71b32802166ec39bf02) ) // chars

	ROM_REGION( 0x06000, "gfx2", 0 )
	ROM_LOAD( "8.2764",       0x00000, 0x2000, CRC(b161c853) SHA1(26913524001415c7426c0d439e078335761d9e7a) ) // tiles - plane 0
	ROM_LOAD( "9.2764",       0x02000, 0x2000, CRC(44fd4c31) SHA1(8f5da66eba4c8a1e3d00be201244b8039d8a5b76) ) // tiles - plane 1
	ROM_LOAD( "10.2764",      0x04000, 0x2000, CRC(b3dbc16b) SHA1(df57836695aadf5c82df22ddeef65e60e025848a) ) // tiles - plane 2

	ROM_REGION( 0x18000, "gfx3", 0 )
	ROM_LOAD( "2.27128",      0x00000, 0x4000, CRC(34dba8f9) SHA1(7abd909856729f5dadcf8143505016a454ae6217) ) // sprites - plane 0
	ROM_LOAD( "3.27128",      0x04000, 0x4000, CRC(8bd7d5b6) SHA1(73cf9828dbcb34e63c7cc70792f3dbab90bd7447) ) // sprites - plane 0
	ROM_LOAD( "4.27128",      0x08000, 0x4000, CRC(64036ea0) SHA1(929794a0105fc5c064f95c75cfcc6ca25a3724ce) ) // sprites - plane 1
	ROM_LOAD( "5.27128",      0x0c000, 0x4000, CRC(2f7352e4) SHA1(3e77c1a315fe082628cf69fede4063743a229055) ) // sprites - plane 1
	ROM_LOAD( "6.27128",      0x10000, 0x4000, CRC(7d9e1e7e) SHA1(81dd8e933a1195e5f119a2b957828a4bf22bea35) ) // sprites - plane 2
	ROM_LOAD( "7.27128",      0x14000, 0x4000, CRC(8b6fa1c4) SHA1(061ae1849e685dd838f61ea1d6e72a579f38ecc6) ) // sprites - plane 2

	ROM_REGION( 0x0340, "proms", 0 ) // not dumped for this PCB, the 1st stage background requires a palette not present here, so these are not correct for this set.
	ROM_LOAD( "b.1j",         0x0000, 0x0100, BAD_DUMP CRC(3ea35431) SHA1(b45318ce898f03a338435a3f6109483d246ff914) ) // blue
	ROM_LOAD( "g.1h",         0x0100, 0x0100, BAD_DUMP CRC(acd7a69e) SHA1(b18eab8f669f0a8105a4bbffa346c4b19491c451) ) // green
	ROM_LOAD( "r.1f",         0x0200, 0x0100, BAD_DUMP CRC(b7f48b41) SHA1(2d84dc29c0ab43729014129e6392207db0f56e9e) ) // red
	ROM_LOAD( "n82s123n.5j",  0x0300, 0x0020, BAD_DUMP CRC(cce2e29f) SHA1(787c65b7d69bcd224b45138fdbbf3fdae296dda6) ) // char lookup table
	ROM_LOAD( "m1.2c",        0x0320, 0x0020, CRC(83a39201) SHA1(4fdc722c9e20ee152c890342ef0dce18e35e2ef8) ) // timing? (not used)

	ROM_REGION( 0x400, "plds", 0 )
	ROM_LOAD( "pal12l6.4m",   0x000, 0x117, CRC(b52fbcc0) SHA1(68d9cb70fb5945fc7002f4e506efa82944dfa6d8) )
	ROM_LOAD( "pal12l6.5n",   0x200, 0x117, CRC(453ce64a) SHA1(abce1251b66befcb53b574e5c3a8b14335c0977f) )
ROM_END

ROM_START( sonofphx )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "5.f4",   0x0000, 0x2000, CRC(e0d2c6cf) SHA1(87befaefa3e4f07523e9c4db19f13ff9309a7dcc) )
	ROM_LOAD( "6.h4",   0x2000, 0x2000, CRC(3a0d0336) SHA1(8e538d45d27ad881fb2ed71647353c6535646047) )
	ROM_LOAD( "7.j4",   0x4000, 0x2000, CRC(57a8e900) SHA1(bc878e27130f0a9afb50c1926b47621e5e58d8b2) )

	ROM_REGION( 0x10000, "sub", 0 )
	ROM_LOAD( "1.f2",   0x0000, 0x2000, CRC(c485c621) SHA1(14fa1b1403f4f2513e1a824f79b750cedf24a31e) )
	ROM_LOAD( "2.h2",   0x2000, 0x2000, CRC(b3c6a886) SHA1(efb136fc1671092fabc2fb2aff189a61bac90ca4) )
	ROM_LOAD( "3.j2",   0x4000, 0x2000, CRC(197e314c) SHA1(6921cd1bc3571b0ac7d8d7eb19b256daca85f17e) )
	ROM_LOAD( "4.k2",   0x6000, 0x2000, CRC(4f3695a1) SHA1(63443d0a0c52e9a761934f8fd43792579fb9966b) )

	ROM_REGION( 0x01000, "gfx1", 0 )
	ROM_LOAD( "14.4a",  0x00000, 0x1000, CRC(b3859b8b) SHA1(9afec14bcee6093ff466ae00b721b177dfdac392) ) // chars

	ROM_REGION( 0x06000, "gfx2", 0 )
	ROM_LOAD( "15.9h",  0x00000, 0x2000, CRC(c9213469) SHA1(03bd7a86f9cbb28ecf39e7ac643a186cfeb38a35) ) // tiles - plane 0
	ROM_LOAD( "16.10h", 0x02000, 0x2000, CRC(7de5d39e) SHA1(47fc5740a972e105d282873b4d72774a4405dfff) ) // tiles - plane 1
	ROM_LOAD( "17.11h", 0x04000, 0x2000, CRC(0ba5f72c) SHA1(79292e16e2f6079f160d957a22e355457599669d) ) // tiles - plane 2

	ROM_REGION( 0x18000, "gfx3", 0 )
	ROM_LOAD( "8.6a",   0x00000, 0x4000, CRC(0e9f757e) SHA1(1d4a46b3f18fe5099cdc889ba5e55c1d171a0430) ) // sprites - plane 0
	ROM_LOAD( "9.7a",   0x04000, 0x4000, CRC(f7d2e650) SHA1(eac715e09ad22b1a1d18e5cade4955cb8d4156f4) ) // sprites - plane 0
	ROM_LOAD( "10.8a",  0x08000, 0x4000, CRC(e717baf4) SHA1(d52a6c5f8b915769cc6dfb50d34922c1a3cd1333) ) // sprites - plane 1
	ROM_LOAD( "11.9a",  0x0c000, 0x4000, CRC(04b2250b) SHA1(d9948277d3ba3cb8188de647e25848f5222d066a) ) // sprites - plane 1
	ROM_LOAD( "12.10a", 0x10000, 0x4000, CRC(d110e140) SHA1(eb528b437e7967ecbe56de51274f286e563f7100) ) // sprites - plane 2
	ROM_LOAD( "13.11a", 0x14000, 0x4000, CRC(8fdc713c) SHA1(c8933d1c45c886c22ee89d02b8941bbbb963d7b1) ) // sprites - plane 2

	ROM_REGION( 0x0340, "proms", 0 )
	ROM_LOAD( "b.1j",   0x0000, 0x0100, CRC(3ea35431) SHA1(b45318ce898f03a338435a3f6109483d246ff914) ) // blue
	ROM_LOAD( "g.1h",   0x0100, 0x0100, CRC(acd7a69e) SHA1(b18eab8f669f0a8105a4bbffa346c4b19491c451) ) // green
	ROM_LOAD( "r.1f",   0x0200, 0x0100, CRC(b7f48b41) SHA1(2d84dc29c0ab43729014129e6392207db0f56e9e) ) // red
	// 0x0300-0x031f empty - looks like there isn't a lookup table PROM or it wasn't dumped
	ROM_LOAD( "m1.2c",  0x0320, 0x0020, CRC(83a39201) SHA1(4fdc722c9e20ee152c890342ef0dce18e35e2ef8) ) // timing? (not used)
ROM_END

ROM_START( flashgal )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "epr-7167.4f",  0x0000, 0x2000, CRC(cf5ad733) SHA1(24561db9a3d72c7a69a7ce5a85aaa78254788675) )
	ROM_LOAD( "epr-7168.4h",  0x2000, 0x2000, CRC(00c4851f) SHA1(f29ef123702bb3506ac3740b2779ae2757d884c2) )
	ROM_LOAD( "epr-7169.4j",  0x4000, 0x2000, CRC(1ef0b8f7) SHA1(9c3ded1f985f4fb6b38843e0ca90ec458633d145) )
	ROM_LOAD( "epr-7170.4k",  0x6000, 0x2000, CRC(885d53de) SHA1(14c8d4d07574e2dc3fba9fc92483a810649e100a) )

	ROM_REGION( 0x10000, "sub", 0 )
	ROM_LOAD( "epr-7163.2f",  0x0000, 0x2000, CRC(eee2134d) SHA1(4d67a969ff033576988de73942717c84068a291d) )
	ROM_LOAD( "epr-7164.2h",  0x2000, 0x2000, CRC(e5e0cd22) SHA1(ad17f5a207a6d74cf0bd1cfebb061c8f65309563) )
	ROM_LOAD( "epr-7165.2j",  0x4000, 0x2000, CRC(4cd3fe5e) SHA1(a69e02acbcad825101f96dcabfada667ae25799c) )
	ROM_LOAD( "epr-7166.2k",  0x6000, 0x2000, CRC(552ca339) SHA1(2313c6aaec47957bce00d0d04f89012eafc014fd) )

	ROM_REGION( 0x01000, "gfx1", 0 )
	ROM_LOAD( "epr-7177.4a",  0x00000, 0x1000, CRC(dca9052f) SHA1(cbeb997db01b875a73a67e08c0b94ab74de77b7b) ) // chars

	ROM_REGION( 0x06000, "gfx2", 0 )
	ROM_LOAD( "epr-7178.9h",  0x00000, 0x2000, CRC(2f5b62c0) SHA1(8be09e4ef1c3e2b1494431d4f07570a109e93dfd) ) // tiles - plane 0
	ROM_LOAD( "epr-7179.10h", 0x02000, 0x2000, CRC(8fbb49b5) SHA1(111f0c42cee6ab2766a5322dd90e1516ffbbc35c) ) // tiles - plane 1
	ROM_LOAD( "epr-7180.11h", 0x04000, 0x2000, CRC(26a8e5c3) SHA1(9a2bc4de87e16bbb777b39564c08d99fd3ccd4ff) ) // tiles - plane 2

	ROM_REGION( 0x18000, "gfx3", 0 )
	ROM_LOAD( "epr-7171.6a",  0x00000, 0x4000, CRC(62caf2a1) SHA1(6fb2c1882630b32af27638d2e9b306d4fc028d62) ) // sprites - plane 0
	ROM_LOAD( "epr-7172.7a",  0x04000, 0x4000, CRC(10f78a10) SHA1(8745da857bdb873d5ffebf753dd66321e1fabc59) ) // sprites - plane 0
	ROM_LOAD( "epr-7173.8a",  0x08000, 0x4000, CRC(36ea1d59) SHA1(cba9117de745049c06c703dc1782b83376523ce3) ) // sprites - plane 1
	ROM_LOAD( "epr-7174.9a",  0x0c000, 0x4000, CRC(f527d837) SHA1(a674e1e71c98d263670962d60a9ac6e205df1204) ) // sprites - plane 1
	ROM_LOAD( "epr-7175.10a", 0x10000, 0x4000, CRC(ba76e4c1) SHA1(e521fbf6a6a8f0e9866c143a576b9dbbc19c4ffd) ) // sprites - plane 2
	ROM_LOAD( "epr-7176.11a", 0x14000, 0x4000, CRC(f095d619) SHA1(7f278c124141ae03f1d8a03b4ea06c81391f16eb) ) // sprites - plane 2

	ROM_REGION( 0x0340, "proms", 0 )
	ROM_LOAD( "7161.1j",      0x0000, 0x0100, CRC(02c4043f) SHA1(bc2af8e054d71a3d0835c795b0f2263c32f2bc42) ) // red
	ROM_LOAD( "7160.1h",      0x0100, 0x0100, CRC(225938d1) SHA1(22bf02832b9f08e4811f9d74a6007bf0ff030eef) ) // green
	ROM_LOAD( "7159.1f",      0x0200, 0x0100, CRC(1e0a1cd3) SHA1(cc120d8fba3e4fb5e18d789981ece77e589ee5a2) ) // blue
	ROM_LOAD( "7162.5j",      0x0300, 0x0020, CRC(cce2e29f) SHA1(787c65b7d69bcd224b45138fdbbf3fdae296dda6) ) // char lookup table
	ROM_LOAD( "bpr.2c",       0x0320, 0x0020, CRC(83a39201) SHA1(4fdc722c9e20ee152c890342ef0dce18e35e2ef8) ) // timing? (not used)

	ROM_REGION( 0x400, "plds", 0 )
	ROM_LOAD( "pal12l6.4m",   0x000, 0x117, CRC(b52fbcc0) SHA1(68d9cb70fb5945fc7002f4e506efa82944dfa6d8) )
	ROM_LOAD( "pal12l6.5n",   0x200, 0x117, CRC(453ce64a) SHA1(abce1251b66befcb53b574e5c3a8b14335c0977f) )
ROM_END

ROM_START( flashgalk )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "epr-7167.4f",  0x0000, 0x2000, CRC(cf5ad733) SHA1(24561db9a3d72c7a69a7ce5a85aaa78254788675) )
	ROM_LOAD( "epr-7168.4h",  0x2000, 0x2000, CRC(00c4851f) SHA1(f29ef123702bb3506ac3740b2779ae2757d884c2) )
	ROM_LOAD( "epr-7169.4j",  0x4000, 0x2000, CRC(1ef0b8f7) SHA1(9c3ded1f985f4fb6b38843e0ca90ec458633d145) )
	ROM_LOAD( "epr-7170.4k",  0x6000, 0x2000, CRC(885d53de) SHA1(14c8d4d07574e2dc3fba9fc92483a810649e100a) )

	ROM_REGION( 0x10000, "sub", 0 )
	ROM_LOAD( "epr-7163.2f",  0x0000, 0x2000, CRC(eee2134d) SHA1(4d67a969ff033576988de73942717c84068a291d) )
	ROM_LOAD( "epr-7164.2h",  0x2000, 0x2000, CRC(e5e0cd22) SHA1(ad17f5a207a6d74cf0bd1cfebb061c8f65309563) )
	ROM_LOAD( "epr-7165.2j",  0x4000, 0x2000, CRC(4cd3fe5e) SHA1(a69e02acbcad825101f96dcabfada667ae25799c) )
	ROM_LOAD( "epr-7166.2k",  0x6000, 0x2000, CRC(552ca339) SHA1(2313c6aaec47957bce00d0d04f89012eafc014fd) )

	ROM_REGION( 0x01000, "gfx1", 0 )
	ROM_LOAD( "4a.bin",  0x00000, 0x1000, CRC(83a30785) SHA1(685e833bbeaff5b4513627eebdee4cde114e8588) ) // chars, only different rom from the Sega set above

	ROM_REGION( 0x06000, "gfx2", 0 )
	ROM_LOAD( "epr-7178.9h",  0x00000, 0x2000, CRC(2f5b62c0) SHA1(8be09e4ef1c3e2b1494431d4f07570a109e93dfd) ) // tiles - plane 0
	ROM_LOAD( "epr-7179.10h", 0x02000, 0x2000, CRC(8fbb49b5) SHA1(111f0c42cee6ab2766a5322dd90e1516ffbbc35c) ) // tiles - plane 1
	ROM_LOAD( "epr-7180.11h", 0x04000, 0x2000, CRC(26a8e5c3) SHA1(9a2bc4de87e16bbb777b39564c08d99fd3ccd4ff) ) // tiles - plane 2

	ROM_REGION( 0x18000, "gfx3", 0 )
	ROM_LOAD( "epr-7171.6a",  0x00000, 0x4000, CRC(62caf2a1) SHA1(6fb2c1882630b32af27638d2e9b306d4fc028d62) ) // sprites - plane 0
	ROM_LOAD( "epr-7172.7a",  0x04000, 0x4000, CRC(10f78a10) SHA1(8745da857bdb873d5ffebf753dd66321e1fabc59) ) // sprites - plane 0
	ROM_LOAD( "epr-7173.8a",  0x08000, 0x4000, CRC(36ea1d59) SHA1(cba9117de745049c06c703dc1782b83376523ce3) ) // sprites - plane 1
	ROM_LOAD( "epr-7174.9a",  0x0c000, 0x4000, CRC(f527d837) SHA1(a674e1e71c98d263670962d60a9ac6e205df1204) ) // sprites - plane 1
	ROM_LOAD( "epr-7175.10a", 0x10000, 0x4000, CRC(ba76e4c1) SHA1(e521fbf6a6a8f0e9866c143a576b9dbbc19c4ffd) ) // sprites - plane 2
	ROM_LOAD( "epr-7176.11a", 0x14000, 0x4000, CRC(f095d619) SHA1(7f278c124141ae03f1d8a03b4ea06c81391f16eb) ) // sprites - plane 2

	ROM_REGION( 0x0340, "proms", 0 )
	ROM_LOAD( "7161.1j",      0x0000, 0x0100, CRC(02c4043f) SHA1(bc2af8e054d71a3d0835c795b0f2263c32f2bc42) ) // red
	ROM_LOAD( "7160.1h",      0x0100, 0x0100, CRC(225938d1) SHA1(22bf02832b9f08e4811f9d74a6007bf0ff030eef) ) // green
	ROM_LOAD( "7159.1f",      0x0200, 0x0100, CRC(1e0a1cd3) SHA1(cc120d8fba3e4fb5e18d789981ece77e589ee5a2) ) // blue
	ROM_LOAD( "7162.5j",      0x0300, 0x0020, CRC(cce2e29f) SHA1(787c65b7d69bcd224b45138fdbbf3fdae296dda6) ) // char lookup table
	ROM_LOAD( "bpr.2c",       0x0320, 0x0020, CRC(83a39201) SHA1(4fdc722c9e20ee152c890342ef0dce18e35e2ef8) ) // timing? (not used)

	ROM_REGION( 0x400, "plds", 0 )
	ROM_LOAD( "pal12l6.4m",   0x000, 0x117, CRC(b52fbcc0) SHA1(68d9cb70fb5945fc7002f4e506efa82944dfa6d8) )
	ROM_LOAD( "pal12l6.5n",   0x200, 0x117, CRC(453ce64a) SHA1(abce1251b66befcb53b574e5c3a8b14335c0977f) )
ROM_END

ROM_START( flashgala )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "flashgal.5",   0x0000, 0x2000, CRC(aa889ace) SHA1(7caac8fae723485adb6990367bdb8a94bd273322) )
	ROM_LOAD( "epr-7168.4h",  0x2000, 0x2000, CRC(00c4851f) SHA1(f29ef123702bb3506ac3740b2779ae2757d884c2) )
	ROM_LOAD( "epr-7169.4j",  0x4000, 0x2000, CRC(1ef0b8f7) SHA1(9c3ded1f985f4fb6b38843e0ca90ec458633d145) )
	ROM_LOAD( "epr-7170.4k",  0x6000, 0x2000, CRC(885d53de) SHA1(14c8d4d07574e2dc3fba9fc92483a810649e100a) )

	ROM_REGION( 0x10000, "sub", 0 )
	ROM_LOAD( "flashgal.1",   0x0000, 0x2000, CRC(55171cc1) SHA1(6780be187288bd7354769fa29a4250e065a91e28) )
	ROM_LOAD( "flashgal.2",   0x2000, 0x2000, CRC(3fd21aac) SHA1(c0d99de50537aa76cd1d2fba3dbf55e7101905db) )
	ROM_LOAD( "flashgal.3",   0x4000, 0x2000, CRC(a1223b74) SHA1(13e2a33e3968f23d60393d950660be4e6b8ae46d) )
	ROM_LOAD( "flashgal.4",   0x6000, 0x2000, CRC(04d2a05f) SHA1(009b8eafc850a100d3a2cde8f6fbba9c98828ddd) )

	ROM_REGION( 0x01000, "gfx1", 0 )
	ROM_LOAD( "epr-7177.4a",  0x00000, 0x1000, CRC(dca9052f) SHA1(cbeb997db01b875a73a67e08c0b94ab74de77b7b) ) // chars

	ROM_REGION( 0x06000, "gfx2", 0 )
	ROM_LOAD( "epr-7178.9h",  0x00000, 0x2000, CRC(2f5b62c0) SHA1(8be09e4ef1c3e2b1494431d4f07570a109e93dfd) ) // tiles - plane 0
	ROM_LOAD( "epr-7179.10h", 0x02000, 0x2000, CRC(8fbb49b5) SHA1(111f0c42cee6ab2766a5322dd90e1516ffbbc35c) ) // tiles - plane 1
	ROM_LOAD( "epr-7180.11h", 0x04000, 0x2000, CRC(26a8e5c3) SHA1(9a2bc4de87e16bbb777b39564c08d99fd3ccd4ff) ) // tiles - plane 2

	ROM_REGION( 0x18000, "gfx3", 0 )
	ROM_LOAD( "epr-7171.6a",  0x00000, 0x4000, CRC(62caf2a1) SHA1(6fb2c1882630b32af27638d2e9b306d4fc028d62) ) // sprites - plane 0
	ROM_LOAD( "epr-7172.7a",  0x04000, 0x4000, CRC(10f78a10) SHA1(8745da857bdb873d5ffebf753dd66321e1fabc59) ) // sprites - plane 0
	ROM_LOAD( "epr-7173.8a",  0x08000, 0x4000, CRC(36ea1d59) SHA1(cba9117de745049c06c703dc1782b83376523ce3) ) // sprites - plane 1
	ROM_LOAD( "epr-7174.9a",  0x0c000, 0x4000, CRC(f527d837) SHA1(a674e1e71c98d263670962d60a9ac6e205df1204) ) // sprites - plane 1
	ROM_LOAD( "epr-7175.10a", 0x10000, 0x4000, CRC(ba76e4c1) SHA1(e521fbf6a6a8f0e9866c143a576b9dbbc19c4ffd) ) // sprites - plane 2
	ROM_LOAD( "epr-7176.11a", 0x14000, 0x4000, CRC(f095d619) SHA1(7f278c124141ae03f1d8a03b4ea06c81391f16eb) ) // sprites - plane 2

	ROM_REGION( 0x0340, "proms", 0 )
	ROM_LOAD( "7161.1j",      0x0000, 0x0100, CRC(02c4043f) SHA1(bc2af8e054d71a3d0835c795b0f2263c32f2bc42) ) // red
	ROM_LOAD( "7160.1h",      0x0100, 0x0100, CRC(225938d1) SHA1(22bf02832b9f08e4811f9d74a6007bf0ff030eef) ) // green
	ROM_LOAD( "7159.1f",      0x0200, 0x0100, CRC(1e0a1cd3) SHA1(cc120d8fba3e4fb5e18d789981ece77e589ee5a2) ) // blue
	ROM_LOAD( "7162.5j",      0x0300, 0x0020, CRC(cce2e29f) SHA1(787c65b7d69bcd224b45138fdbbf3fdae296dda6) ) // char lookup table
	ROM_LOAD( "bpr.2c",       0x0320, 0x0020, CRC(83a39201) SHA1(4fdc722c9e20ee152c890342ef0dce18e35e2ef8) ) // timing? (not used)

	ROM_REGION( 0x400, "plds", 0 )
	ROM_LOAD( "pal12l6.4m",   0x000, 0x117, CRC(b52fbcc0) SHA1(68d9cb70fb5945fc7002f4e506efa82944dfa6d8) )
	ROM_LOAD( "pal12l6.5n",   0x200, 0x117, CRC(453ce64a) SHA1(abce1251b66befcb53b574e5c3a8b14335c0977f) )
ROM_END

ROM_START( srdmissn )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "5.t2",   0x0000, 0x4000, CRC(a682b48c) SHA1(c7348cbe42e45cd336e0d03052e839781d1481d1) )
	ROM_LOAD( "7.t3",   0x4000, 0x4000, CRC(1719c58c) SHA1(32faae584d0ada0a39b96655b1a9d7c449af4996) )

	ROM_REGION( 0x10000, "sub", 0 )
	ROM_LOAD( "1.t7",   0x0000, 0x4000, CRC(dc48595e) SHA1(cbf357db4555ba57eb0a696bf1ecfcfeb9871409) )
	ROM_LOAD( "3.t8",   0x4000, 0x4000, CRC(216be1e8) SHA1(90016f74ebad1aafb9bd086e28ca3d400839b0a2) )

	ROM_REGION( 0x01000, "gfx1", 0 )
	ROM_LOAD( "15.4a",  0x00000, 0x1000, CRC(4961f7fd) SHA1(8b08b9df0c3f71ceffd9b17a364aba611c05c774) ) // chars

	ROM_REGION( 0x06000, "gfx2", 0 )
	ROM_LOAD( "17.9h",  0x00000, 0x2000, CRC(41211458) SHA1(bb080f4d7b92a1dadb7e554fd640e14ddbbcfcb6) ) // tiles - plane 1
	ROM_LOAD( "18.10h", 0x02000, 0x2000, CRC(740eccd4) SHA1(a1ce4b1a9e7c26a7322b7cdd6c734889900485be) ) // tiles - plane 0
	ROM_LOAD( "16.11h", 0x04000, 0x2000, CRC(c1f4a5db) SHA1(b9a0e57fac6317dceec3d4fbcde25b6b883fbbb1) ) // tiles - plane 2

	ROM_REGION( 0x18000, "gfx3", 0 )
	ROM_LOAD( "14.6a",  0x00000, 0x4000, CRC(3d4c0447) SHA1(a064d43f5e8e0ac6ce441148057623f4a32f6056) ) // sprites - plane 0
	ROM_LOAD( "13.7a",  0x04000, 0x4000, CRC(22414a67) SHA1(aaeb1bd196967d201d0dfc06de88419d6651f788) ) // sprites - plane 0
	ROM_LOAD( "12.8a",  0x08000, 0x4000, CRC(61e34283) SHA1(35fff04aae4d5ab3269c1cd2c306c21507dde073) ) // sprites - plane 1
	ROM_LOAD( "11.9a",  0x0c000, 0x4000, CRC(bbbaffef) SHA1(f2de9eebd9f566a4aea9a30bfd4d0a7643ea320c) ) // sprites - plane 1
	ROM_LOAD( "10.10a", 0x10000, 0x4000, CRC(de564f97) SHA1(fa5e9a807ef170f01df9c1d7c9124e418a29c3bb) ) // sprites - plane 2
	ROM_LOAD( "9.11a",  0x14000, 0x4000, CRC(890dc815) SHA1(61a1f7a3ff9bde31e0f7427cea115a4c140798af) ) // sprites - plane 2

	ROM_REGION( 0x0340, "proms", 0 )
	ROM_LOAD( "mr.1j",  0x0000, 0x0100, CRC(110a436e) SHA1(1559a3bfc80c0e13f94dc929171af12010e1de28) ) // red
	ROM_LOAD( "mg.1h",  0x0100, 0x0100, CRC(0fbfd9f0) SHA1(eb69f076c2f12f17238c5189645da54a85bbc2be) ) // green
	ROM_LOAD( "mb.1f",  0x0200, 0x0100, CRC(a342890c) SHA1(6f2223ed68392b15a8751dba9bb28b85fd1d8dc0) ) // blue
	ROM_LOAD( "m2.5j",  0x0300, 0x0020, CRC(190a55ad) SHA1(de8a847bff8c343d69b853a215e6ee775ef2ef96) ) // char lookup table
	ROM_LOAD( "m1.2c",  0x0320, 0x0020, CRC(83a39201) SHA1(4fdc722c9e20ee152c890342ef0dce18e35e2ef8) ) // timing? not used
ROM_END

ROM_START( fx )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "fx.01", 0x0000, 0x4000, CRC(b651754b) SHA1(05024276aeac0c2a3d62f3a6f1027518fe206784) )
	ROM_LOAD( "fx.02", 0x4000, 0x4000, CRC(f3d2dcc1) SHA1(466bed28ecf25f9e2653662d7cc382ceb916d8db) )

	ROM_REGION( 0x10000, "sub", 0 )
	ROM_LOAD( "fx.03", 0x0000, 0x4000, CRC(8907df6b) SHA1(a6d3e804fdaaeddca6f6f1e29dc35f25ed9490ae) )
	ROM_LOAD( "fx.04", 0x4000, 0x4000, CRC(c665834f) SHA1(91ffb32790df324b5f47f8b88f47a1b6ec689cce) )

	ROM_REGION( 0x01000, "gfx1", 0 )
	ROM_LOAD( "fx.05", 0x0000, 0x1000, CRC(4a504286) SHA1(0d5ca6ce24ebaad466235fdb22471ac1ad7703db) )

	ROM_REGION( 0x06000, "gfx2", 0 )
	ROM_LOAD( "17.9h",  0x00000, 0x2000, CRC(41211458) SHA1(bb080f4d7b92a1dadb7e554fd640e14ddbbcfcb6) ) // tiles - plane 1
	ROM_LOAD( "18.10h", 0x02000, 0x2000, CRC(740eccd4) SHA1(a1ce4b1a9e7c26a7322b7cdd6c734889900485be) ) // tiles - plane 0
	ROM_LOAD( "16.11h", 0x04000, 0x2000, CRC(c1f4a5db) SHA1(b9a0e57fac6317dceec3d4fbcde25b6b883fbbb1) ) // tiles - plane 2

	ROM_REGION( 0x18000, "gfx3", 0 )
	ROM_LOAD( "14.6a",  0x00000, 0x4000, CRC(3d4c0447) SHA1(a064d43f5e8e0ac6ce441148057623f4a32f6056) ) // sprites - plane 0
	ROM_LOAD( "13.7a",  0x04000, 0x4000, CRC(22414a67) SHA1(aaeb1bd196967d201d0dfc06de88419d6651f788) ) // sprites - plane 0
	ROM_LOAD( "12.8a",  0x08000, 0x4000, CRC(61e34283) SHA1(35fff04aae4d5ab3269c1cd2c306c21507dde073) ) // sprites - plane 1
	ROM_LOAD( "11.9a",  0x0c000, 0x4000, CRC(bbbaffef) SHA1(f2de9eebd9f566a4aea9a30bfd4d0a7643ea320c) ) // sprites - plane 1
	ROM_LOAD( "10.10a", 0x10000, 0x4000, CRC(de564f97) SHA1(fa5e9a807ef170f01df9c1d7c9124e418a29c3bb) ) // sprites - plane 2
	ROM_LOAD( "9.11a",  0x14000, 0x4000, CRC(890dc815) SHA1(61a1f7a3ff9bde31e0f7427cea115a4c140798af) ) // sprites - plane 2

	ROM_REGION( 0x0340, "proms", 0 )
	ROM_LOAD( "mr.1j",  0x0000, 0x0100, CRC(110a436e) SHA1(1559a3bfc80c0e13f94dc929171af12010e1de28) ) // red
	ROM_LOAD( "mg.1h",  0x0100, 0x0100, CRC(0fbfd9f0) SHA1(eb69f076c2f12f17238c5189645da54a85bbc2be) ) // green
	ROM_LOAD( "mb.1f",  0x0200, 0x0100, CRC(a342890c) SHA1(6f2223ed68392b15a8751dba9bb28b85fd1d8dc0) ) // blue
	ROM_LOAD( "m2.5j",  0x0300, 0x0020, CRC(190a55ad) SHA1(de8a847bff8c343d69b853a215e6ee775ef2ef96) ) // char lookup table
	ROM_LOAD( "m1.2c",  0x0320, 0x0020, CRC(83a39201) SHA1(4fdc722c9e20ee152c890342ef0dce18e35e2ef8) ) // timing? not used

	ROM_REGION( 0x0600, "plds", 0 )
	ROM_LOAD( "pal16l8.1", 0x0000, 0x0104, NO_DUMP ) // protected
	ROM_LOAD( "pal16l8.2", 0x0200, 0x0104, NO_DUMP ) // protected
	ROM_LOAD( "pal16l8.3", 0x0400, 0x0104, NO_DUMP ) // protected
ROM_END

ROM_START( airwolf )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "b.2s",        0x0000, 0x8000, CRC(8c993cce) SHA1(925a5a9a2ee382556e2c2e928fd483344eba72c3) )

	ROM_REGION( 0x10000, "sub", 0 )
	ROM_LOAD( "a.7s",        0x0000, 0x8000, CRC(a3c7af5c) SHA1(0f70ca94f3d168d38e0e93252e9441973f72441a) )

	ROM_REGION( 0x01000, "gfx1", 0 )
	ROM_LOAD( "f.4a",        0x00000, 0x1000, CRC(4df44ce9) SHA1(145986009d4ae6f7dd98ce715838d0331dea005d) ) // chars

	ROM_REGION( 0x06000, "gfx2", 0 )
	ROM_LOAD( "09h_14.bin",  0x00000, 0x2000, CRC(25e57e1f) SHA1(bef24bced102cd470e10bd4aa19da3c608211258) ) // tiles - plane 1
	ROM_LOAD( "10h_13.bin",  0x02000, 0x2000, CRC(cf0de5e9) SHA1(32f3eb4c9298d59aca1dc2530b0e92f64311946d) ) // tiles - plane 0
	ROM_LOAD( "11h_12.bin",  0x04000, 0x2000, CRC(4050c048) SHA1(ca21e0750f01342d9791067160339eec436c9458) ) // tiles - plane 2

	ROM_REGION( 0x18000, "gfx3", 0 )
	ROM_LOAD( "e.6a",        0x00000, 0x2000, CRC(e8fbc7d2) SHA1(a9a651b4a714f490a051a89fd0327a665353d64b) ) // sprites - plane 0
	ROM_CONTINUE(            0x04000, 0x2000 )
	ROM_CONTINUE(            0x02000, 0x2000 )
	ROM_CONTINUE(            0x06000, 0x2000 )
	ROM_LOAD( "d.8a",        0x08000, 0x2000, CRC(c5d4156b) SHA1(f66ec33b67e39f3df016231b00e48c9757e322f3) ) // sprites - plane 1
	ROM_CONTINUE(            0x0c000, 0x2000 )
	ROM_CONTINUE(            0x0a000, 0x2000 )
	ROM_CONTINUE(            0x0e000, 0x2000 )
	ROM_LOAD( "c.10a",       0x10000, 0x2000, CRC(de91dfb1) SHA1(9f338542f44905d0b895d99510475113eb860f0d) ) // sprites - plane 2
	ROM_CONTINUE(            0x14000, 0x2000 )
	ROM_CONTINUE(            0x12000, 0x2000 )
	ROM_CONTINUE(            0x16000, 0x2000 )

	ROM_REGION( 0x0340, "proms", 0 )
	ROM_LOAD( "01j.bin",     0x0000, 0x0100, CRC(6a94b2a3) SHA1(b1f9bd97aa26c9fb6377ef32d5dd125583361f48) ) // red
	ROM_LOAD( "01h.bin",     0x0100, 0x0100, CRC(ec0923d3) SHA1(26f9eda4260a8b767893b8dea42819f192ef0b20) ) // green
	ROM_LOAD( "01f.bin",     0x0200, 0x0100, CRC(ade97052) SHA1(cc1b4cd57d7bc55ce44de6b89a322ff08eabb1a0) ) // blue
	ROM_LOAD( "74s288-2.bin",     0x0300, 0x0020, CRC(190a55ad) SHA1(de8a847bff8c343d69b853a215e6ee775ef2ef96) ) // blank lookup prom
	ROM_LOAD( "m1.2c",       0x0320, 0x0020, CRC(83a39201) SHA1(4fdc722c9e20ee152c890342ef0dce18e35e2ef8) ) // timing? not used

	ROM_REGION( 0x0400, "plds", 0 )
	ROM_LOAD( "pal16l8a.2j",   0x0000, 0x0104, NO_DUMP ) // PAL is read protected
	ROM_LOAD( "epl12p6a.9j",   0x0200, 0x0034, CRC(19808f14) SHA1(d5043237be8857d2cecaf7dec079461f6b53efa9) )
	ROM_LOAD( "epl12p6a.9k",   0x0300, 0x0034, CRC(f5acad85) SHA1(ee3caeedf3e91793b12895f109eae5417f5c7631) )
ROM_END

ROM_START( airwolfa )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "airwolf.2",        0x0000, 0x8000, CRC(bc1a8587) SHA1(5487096621c175759eb4a4a85b76ef32900ca522) )

	ROM_REGION( 0x10000, "sub", 0 ) // the rom with this set was bad with FIXED BITS (11xxxxxx), but the remaining bits matched
	ROM_LOAD( "airwolf.1",        0x0000, 0x8000, CRC(a3c7af5c) SHA1(0f70ca94f3d168d38e0e93252e9441973f72441a) )

	ROM_REGION( 0x02000, "gfx1", 0 )
	ROM_LOAD( "airwolf.6",        0x00000, 0x2000, CRC(5b0a01e9) SHA1(a2873054caf08fcfa51e2f87556e9529b7d4b865) ) // chars

	ROM_REGION( 0x06000, "gfx2", 0 )
	ROM_LOAD( "airwolf.9",  0x00000, 0x2000, CRC(25e57e1f) SHA1(bef24bced102cd470e10bd4aa19da3c608211258) ) // tiles - plane 1
	ROM_LOAD( "airwolf.8",  0x02000, 0x2000, CRC(cf0de5e9) SHA1(32f3eb4c9298d59aca1dc2530b0e92f64311946d) ) // tiles - plane 0
	ROM_LOAD( "airwolf.7",  0x04000, 0x2000, CRC(4050c048) SHA1(ca21e0750f01342d9791067160339eec436c9458) ) // tiles - plane 2

	ROM_REGION( 0x18000, "gfx3", 0 )
	ROM_LOAD( "airwolf.5",   0x00000, 0x2000, CRC(e8fbc7d2) SHA1(a9a651b4a714f490a051a89fd0327a665353d64b) ) // sprites - plane 0
	ROM_CONTINUE(            0x04000, 0x2000 )
	ROM_CONTINUE(            0x02000, 0x2000 )
	ROM_CONTINUE(            0x06000, 0x2000 )
	ROM_LOAD( "airwolf.4",   0x08000, 0x2000, CRC(c5d4156b) SHA1(f66ec33b67e39f3df016231b00e48c9757e322f3) ) // sprites - plane 1
	ROM_CONTINUE(            0x0c000, 0x2000 )
	ROM_CONTINUE(            0x0a000, 0x2000 )
	ROM_CONTINUE(            0x0e000, 0x2000 )
	ROM_LOAD( "airwolf.3",   0x10000, 0x2000, CRC(de91dfb1) SHA1(9f338542f44905d0b895d99510475113eb860f0d) ) // sprites - plane 2
	ROM_CONTINUE(            0x14000, 0x2000 )
	ROM_CONTINUE(            0x12000, 0x2000 )
	ROM_CONTINUE(            0x16000, 0x2000 )

	ROM_REGION( 0x0340, "proms", 0 )
	ROM_LOAD( "01j.bin",     0x0000, 0x0100, CRC(6a94b2a3) SHA1(b1f9bd97aa26c9fb6377ef32d5dd125583361f48) ) // red
	ROM_LOAD( "01h.bin",     0x0100, 0x0100, CRC(ec0923d3) SHA1(26f9eda4260a8b767893b8dea42819f192ef0b20) ) // green
	ROM_LOAD( "01f.bin",     0x0200, 0x0100, CRC(ade97052) SHA1(cc1b4cd57d7bc55ce44de6b89a322ff08eabb1a0) ) // blue
	ROM_LOAD( "74s288-2.bin",     0x0300, 0x0020, CRC(190a55ad) SHA1(de8a847bff8c343d69b853a215e6ee775ef2ef96) ) // blank lookup prom
	ROM_LOAD( "m1.2c",       0x0320, 0x0020, CRC(83a39201) SHA1(4fdc722c9e20ee152c890342ef0dce18e35e2ef8) ) // timing? not used

	ROM_REGION( 0x0400, "plds", 0 )
	ROM_LOAD( "pal16l8a.2j",   0x0000, 0x0104, NO_DUMP ) // PAL is read protected
	ROM_LOAD( "epl12p6a.9j",   0x0200, 0x0034, CRC(19808f14) SHA1(d5043237be8857d2cecaf7dec079461f6b53efa9) )
	ROM_LOAD( "epl12p6a.9k",   0x0300, 0x0034, CRC(f5acad85) SHA1(ee3caeedf3e91793b12895f109eae5417f5c7631) )
ROM_END

ROM_START( skywolf )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "02s_03.bin",  0x0000, 0x4000, CRC(a0891798) SHA1(c1b1e1fce529509fb1dd921a0022d5367c3c495c) )
	ROM_LOAD( "03s_04.bin",  0x4000, 0x4000, CRC(5f515d46) SHA1(ec12bddf72e98aeef5cd17d00f0fa6f2df59cf00) )

	ROM_REGION( 0x10000, "sub", 0 )
	ROM_LOAD( "07s_01.bin",  0x0000, 0x4000, CRC(c680a905) SHA1(9a65bade18d0340d428ee12d2e505def2339e3c3) )
	ROM_LOAD( "08s_02.bin",  0x4000, 0x4000, CRC(3d66bf26) SHA1(10a9f9888c1da12e2ba71748b8608b18e46e8db6) )

	ROM_REGION( 0x01000, "gfx1", 0 )
	ROM_LOAD( "04a_11.bin",  0x00000, 0x1000, CRC(219de9aa) SHA1(7f79b718427310f8725b64cb1953879d7277b212) ) // chars

	ROM_REGION( 0x06000, "gfx2", 0 )
	ROM_LOAD( "09h_14.bin",  0x00000, 0x2000, CRC(25e57e1f) SHA1(bef24bced102cd470e10bd4aa19da3c608211258) ) // tiles - plane 1
	ROM_LOAD( "10h_13.bin",  0x02000, 0x2000, CRC(cf0de5e9) SHA1(32f3eb4c9298d59aca1dc2530b0e92f64311946d) ) // tiles - plane 0
	ROM_LOAD( "11h_12.bin",  0x04000, 0x2000, CRC(4050c048) SHA1(ca21e0750f01342d9791067160339eec436c9458) ) // tiles - plane 2

	ROM_REGION( 0x18000, "gfx3", 0 )
	ROM_LOAD( "06a_10.bin",  0x00000, 0x4000, CRC(1c809383) SHA1(955aed87a8e9ae6ef7f0daa27a0cc4d85493ba90) ) // sprites - plane 0
	ROM_LOAD( "07a_09.bin",  0x04000, 0x4000, CRC(5665d774) SHA1(cd4359d2f970e2b9d09f5ddadcddf8e77caea6e9) ) // sprites - plane 0
	ROM_LOAD( "08a_08.bin",  0x08000, 0x4000, CRC(6dda8f2a) SHA1(db9fc81094fa8384da89f8f3349f09fc8e4f3c92) ) // sprites - plane 1
	ROM_LOAD( "09a_07.bin",  0x0c000, 0x4000, CRC(6a21ddb8) SHA1(f47b952f513ebe7202b219bfe29f20368f40dc70) ) // sprites - plane 1
	ROM_LOAD( "10a_06.bin",  0x10000, 0x4000, CRC(f2e548e0) SHA1(82b6a86eaa6dfbc4547a1e1009929fede7ba9f61) ) // sprites - plane 2
	ROM_LOAD( "11a_05.bin",  0x14000, 0x4000, CRC(8681b112) SHA1(2d6f580dcc0b5c2803c20cece01a896d41e2c8b6) ) // sprites - plane 2

	ROM_REGION( 0x0340, "proms", 0 )
	ROM_LOAD( "01j.bin",     0x0000, 0x0100, CRC(6a94b2a3) SHA1(b1f9bd97aa26c9fb6377ef32d5dd125583361f48) ) // red
	ROM_LOAD( "01h.bin",     0x0100, 0x0100, CRC(ec0923d3) SHA1(26f9eda4260a8b767893b8dea42819f192ef0b20) ) // green
	ROM_LOAD( "01f.bin",     0x0200, 0x0100, CRC(ade97052) SHA1(cc1b4cd57d7bc55ce44de6b89a322ff08eabb1a0) ) // blue
	ROM_LOAD( "74s288-2.bin",     0x0300, 0x0020, CRC(190a55ad) SHA1(de8a847bff8c343d69b853a215e6ee775ef2ef96) ) // blank lookup prom
	ROM_LOAD( "m1.2c",       0x0320, 0x0020, CRC(83a39201) SHA1(4fdc722c9e20ee152c890342ef0dce18e35e2ef8) ) // timing? not used
ROM_END

ROM_START( skywolf2 )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "z80_2.bin",   0x0000, 0x8000, CRC(34db7bda) SHA1(1a98d5cf97063453a0351f7dbe339c32d59a3d20) )

	ROM_REGION( 0x10000, "sub", 0 )
	ROM_LOAD( "07s_01.bin",  0x0000, 0x4000, CRC(c680a905) SHA1(9a65bade18d0340d428ee12d2e505def2339e3c3) )
	ROM_LOAD( "08s_02.bin",  0x4000, 0x4000, CRC(3d66bf26) SHA1(10a9f9888c1da12e2ba71748b8608b18e46e8db6) )

	ROM_REGION( 0x01000, "gfx1", 0 )
	ROM_LOAD( "04a_11.bin",  0x00000, 0x1000, CRC(219de9aa) SHA1(7f79b718427310f8725b64cb1953879d7277b212) ) // chars

	ROM_REGION( 0x06000, "gfx2", 0 )
	ROM_LOAD( "09h_14.bin",  0x00000, 0x2000, CRC(25e57e1f) SHA1(bef24bced102cd470e10bd4aa19da3c608211258) ) // tiles - plane 1
	ROM_LOAD( "10h_13.bin",  0x02000, 0x2000, CRC(cf0de5e9) SHA1(32f3eb4c9298d59aca1dc2530b0e92f64311946d) ) // tiles - plane 0
	ROM_LOAD( "11h_12.bin",  0x04000, 0x2000, CRC(4050c048) SHA1(ca21e0750f01342d9791067160339eec436c9458) ) // tiles - plane 2

	ROM_REGION( 0x18000, "gfx3", 0 )
	ROM_LOAD( "06a_10.bin",  0x00000, 0x4000, CRC(1c809383) SHA1(955aed87a8e9ae6ef7f0daa27a0cc4d85493ba90) ) // sprites - plane 0
	ROM_LOAD( "07a_09.bin",  0x04000, 0x4000, CRC(5665d774) SHA1(cd4359d2f970e2b9d09f5ddadcddf8e77caea6e9) ) // sprites - plane 0
	ROM_LOAD( "08a_08.bin",  0x08000, 0x4000, CRC(6dda8f2a) SHA1(db9fc81094fa8384da89f8f3349f09fc8e4f3c92) ) // sprites - plane 1
	ROM_LOAD( "09a_07.bin",  0x0c000, 0x4000, CRC(6a21ddb8) SHA1(f47b952f513ebe7202b219bfe29f20368f40dc70) ) // sprites - plane 1
	ROM_LOAD( "10a_06.bin",  0x10000, 0x4000, CRC(f2e548e0) SHA1(82b6a86eaa6dfbc4547a1e1009929fede7ba9f61) ) // sprites - plane 2
	ROM_LOAD( "11a_05.bin",  0x14000, 0x4000, CRC(8681b112) SHA1(2d6f580dcc0b5c2803c20cece01a896d41e2c8b6) ) // sprites - plane 2

	ROM_REGION( 0x0340, "proms", 0 )
	ROM_LOAD( "01j.bin",     0x0000, 0x0100, CRC(6a94b2a3) SHA1(b1f9bd97aa26c9fb6377ef32d5dd125583361f48) ) // red
	ROM_LOAD( "01h.bin",     0x0100, 0x0100, CRC(ec0923d3) SHA1(26f9eda4260a8b767893b8dea42819f192ef0b20) ) // green
	ROM_LOAD( "01f.bin",     0x0200, 0x0100, CRC(ade97052) SHA1(cc1b4cd57d7bc55ce44de6b89a322ff08eabb1a0) ) // blue
	ROM_LOAD( "74s288-2.bin",     0x0300, 0x0020, CRC(190a55ad) SHA1(de8a847bff8c343d69b853a215e6ee775ef2ef96) ) // blank lookup prom
	ROM_LOAD( "m1.2c",       0x0320, 0x0020, CRC(83a39201) SHA1(4fdc722c9e20ee152c890342ef0dce18e35e2ef8) ) // timing? not used
ROM_END

/*
Sky Wolf bootleg
 - this has all the data in 0x8000 sized roms.

on main PCB (CR208):
1x TMS27256JL (1)
3x M27256 (2,3,4)
3x PAL16L8NC (read protected)

on roms PCB (CR207):
7x M27256 (5,6,7,8,9,10,11)
3x PROM N82S129N
2x PROM DM74S288N (one is blank!)

the only real difference seems to be that you get less lives.

*/

ROM_START( skywolf3 )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "1.bin",   0x0000, 0x4000, CRC(74a86ec8) SHA1(f9e5622c855053f7aac81c4775654ee8bc802180) )
	ROM_CONTINUE(0x0000,0x4000) // 1.BIN        [2/2]      z80_2.bin    [1/2]      99.981689%
	ROM_LOAD( "2.bin",   0x4000, 0x4000, CRC(f02143de) SHA1(7695432c87bb4850f09b9d00c17f4b9216fb2b90) )
	ROM_CONTINUE(0x4000,0x4000)


	ROM_REGION( 0x10000, "sub", 0 )
	ROM_LOAD( "3.bin",  0x0000, 0x4000, CRC(787cdd0a) SHA1(6f53008ee96b690ef467b3436d4bebba82c71d6b) )
	ROM_CONTINUE(0x0000,0x4000)
	ROM_LOAD( "4.bin",  0x4000, 0x4000, CRC(07a2c814) SHA1(242bbce2b1b6e1668235d327e8c3a61906175af5) )
	ROM_CONTINUE(0x4000,0x4000)

	ROM_REGION( 0x01000, "gfx1", 0 )
	ROM_LOAD( "8.bin",  0x00000, 0x1000, CRC(b86d3dac) SHA1(d92e494d46f641fbfb107da218f5aab5bdf1e68c) ) // chars
	ROM_CONTINUE(0x0000,0x1000)
	ROM_CONTINUE(0x0000,0x1000)
	ROM_CONTINUE(0x0000,0x1000)
	ROM_CONTINUE(0x0000,0x1000)
	ROM_CONTINUE(0x0000,0x1000)
	ROM_CONTINUE(0x0000,0x1000)
	ROM_CONTINUE(0x0000,0x1000)

	ROM_REGION( 0x06000, "gfx2", 0 )
	ROM_LOAD( "11.bin",  0x00000, 0x2000, CRC(fc7bbf7a) SHA1(a10245d32efa9998a63008e3989b1a4958c85b0a) ) // tiles - plane 1
	ROM_CONTINUE(0x0000,0x2000)
	ROM_CONTINUE(0x0000,0x2000)
	ROM_CONTINUE(0x0000,0x2000)
	ROM_LOAD( "10.bin",  0x02000, 0x2000, CRC(1a3710ab) SHA1(6e61e94bb7f22beeb43af35c3299569c40c38ed9) ) // tiles - plane 0
	ROM_CONTINUE(0x2000,0x2000)
	ROM_CONTINUE(0x2000,0x2000)
	ROM_CONTINUE(0x2000,0x2000)
	ROM_LOAD( "9.bin",  0x04000, 0x2000, CRC(a184349a) SHA1(e67f3727e6b57dc5ab503f2aa00ec860ba722633) ) // tiles - plane 2
	ROM_CONTINUE(0x4000,0x2000)
	ROM_CONTINUE(0x4000,0x2000)
	ROM_CONTINUE(0x4000,0x2000)

	ROM_REGION( 0x18000, "gfx3", 0 )
	ROM_LOAD( "7.bin",  0x00000, 0x8000, CRC(086612e8) SHA1(c59296d720a65a69d8c558fda73702ec345c5a2d) ) // sprites - plane 0
	ROM_LOAD( "6.bin",  0x08000, 0x8000, CRC(3a9beabd) SHA1(a20ee42af04ef2e77dcc2040d9ebd6084005e009) ) // sprites - plane 1
	ROM_LOAD( "5.bin",  0x10000, 0x8000, CRC(bd83658e) SHA1(4b2a98c24c20e4deb819613e5fbcd63ae8c81700) ) // sprites - plane 2

	ROM_REGION( 0x0340, "proms", 0 )
	ROM_LOAD( "82s129-1.bin",     0x0000, 0x0100, CRC(6a94b2a3) SHA1(b1f9bd97aa26c9fb6377ef32d5dd125583361f48) ) // red
	//ROM_LOAD( "82s129-2.bin",     0x0100, 0x0100, CRC(ff7a7446) SHA1(ceeb375dc90142142a284969c104e581deb76f16) ) // green (bad?) - causes green outline on title
	//ROM_LOAD( "82s129-3.bin",     0x0200, 0x0100, CRC(6b0980bf) SHA1(6314f9e593f2d2a2f014f6eb82295cb3aa70cbd1)) ) // blue (bad) - high bit of colour fixed to 0
	ROM_LOAD( "82s129-2.bin",     0x0100, 0x0100, CRC(ec0923d3) SHA1(26f9eda4260a8b767893b8dea42819f192ef0b20) ) // green
	ROM_LOAD( "82s129-3.bin",     0x0200, 0x0100, CRC(ade97052) SHA1(cc1b4cd57d7bc55ce44de6b89a322ff08eabb1a0) ) // blue
	ROM_LOAD( "74s288-2.bin",     0x0300, 0x0020, CRC(190a55ad) SHA1(de8a847bff8c343d69b853a215e6ee775ef2ef96) ) // blank lookup prom
	ROM_LOAD( "74s288-1.bin",     0x0320, 0x0020, CRC(5ddb2d15) SHA1(422663566ebc7ea8cbc3089d806b0868e006fe0c) ) // timing? not used

	ROM_REGION( 0x0600, "plds", 0 )
	ROM_LOAD( "pal16l8nc.1", 0x0000, 0x0104, NO_DUMP ) // protected
	ROM_LOAD( "pal16l8nc.2", 0x0200, 0x0104, NO_DUMP ) // protected
	ROM_LOAD( "pal16l8nc.3", 0x0400, 0x0104, NO_DUMP ) // protected
ROM_END

ROM_START( legend )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "a_r2.rom",    0x0000, 0x4000, CRC(0cc1c4f4) SHA1(33f6a1b31eed75a92e06cb29f912321fe75c31e6) )
	ROM_LOAD( "a_r3.rom",    0x4000, 0x4000, CRC(4b270c6b) SHA1(95ad79a9de037b6aaca325da75c8aef9a72dbfed) )

	ROM_REGION( 0x10000, "sub", 0 )
	ROM_LOAD( "a_r7.rom",    0x0000, 0x2000, CRC(abfe5eb4) SHA1(fbeb5ee14aaebb6321fe97fe523f08833fad9c7c) )
	ROM_LOAD( "a_r8.rom",    0x2000, 0x2000, CRC(7e7b9ba9) SHA1(897779129108b0f3936234ea797d47cf46cb7a16) )
	ROM_LOAD( "a_r9.rom",    0x4000, 0x2000, CRC(66737f1e) SHA1(5eac6606ed3a11a00eb1172e35487b0d95b1d739) )
	ROM_LOAD( "a_n7.rom",    0x6000, 0x2000, CRC(13915a53) SHA1(25ba3babc8eb0df413bdfe7dbcd8642e4c658120) )

	ROM_REGION(  0x1000, "gfx1", 0 )    // fg tiles
	ROM_LOAD( "b_a4.rom",    0x0000, 0x1000, CRC(c7dd3cf7) SHA1(87f31c639d840e781d0f56f98f00d0642c6b87b4) )

	ROM_REGION(  0x6000, "gfx2", 0 )    // bg tiles
	ROM_LOAD( "b_h9.rom",    0x0000, 0x2000, CRC(1fe8644a) SHA1(42f5b93ffb3277321969a0bd805ec78796583d37) )
	ROM_LOAD( "b_h10.rom",   0x2000, 0x2000, CRC(5f7dc82e) SHA1(3e70be650b8046c2b34a2405a2fbc2a16bf73bf1) )
	ROM_LOAD( "b_h11.rom",   0x4000, 0x2000, CRC(46741643) SHA1(3fba31bac5a7d94af80035304647f39af3a9484f) )

	ROM_REGION( 0x18000, "gfx3", 0 )    // sprites
	ROM_LOAD( "b_a6.rom",   0x00000, 0x4000, CRC(1689f21c) SHA1(fafb13dc8ca27a7506065bbf08102afc6d722843) )
	ROM_LOAD( "b_a7.rom",   0x04000, 0x4000, CRC(f527c909) SHA1(40f44828502018c73283965eb7a2a68ed25ebfe5) )
	ROM_LOAD( "b_a8.rom",   0x08000, 0x4000, CRC(8d618629) SHA1(3cc49fd8066464ee940de010da3f33ed8573df3d) )
	ROM_LOAD( "b_a9.rom",   0x0c000, 0x4000, CRC(7d7e2d55) SHA1(efd4817a0f5e14cb5a3d0c1f69e6ad408a813202) )
	ROM_LOAD( "b_a10.rom",  0x10000, 0x4000, CRC(f12232fe) SHA1(2d8accc10f0703eeb075c4053d4165b90552e6a7) )
	ROM_LOAD( "b_a11.rom",  0x14000, 0x4000, CRC(8c09243d) SHA1(8f0f68921f8ab6c016b7481714febb68abb7ce79) )

	ROM_REGION( 0x0340, "proms", 0 )
	ROM_LOAD( "82s129.1j",   0x0000, 0x0100, CRC(40590ac0) SHA1(30a8e24e34c4ee0a7df91c0633becfce1c8d856c) ) // red
	ROM_LOAD( "82s129.1h",   0x0100, 0x0100, CRC(e542b363) SHA1(6775209b9a4aaf374878c06cf4dc693b921abd87) ) // green
	ROM_LOAD( "82s129.1f",   0x0200, 0x0100, CRC(75536fc8) SHA1(e713efafcdc7f2a595444af75d2083eb3e38a480) ) // blue
	ROM_LOAD( "82s123.5j",   0x0300, 0x0020, CRC(c98f0651) SHA1(4f1b95213c28ad017c8d6542e8d522e4d69f91e3) ) // char lookup table
	ROM_LOAD( "m1.2c",       0x0320, 0x0020, CRC(83a39201) SHA1(4fdc722c9e20ee152c890342ef0dce18e35e2ef8) ) // timing? not used

	ROM_REGION( 0x0300, "plds", 0 )
	ROM_LOAD( "epl10p8.2j", 0x0000, 0x002c, CRC(8abc03bf) SHA1(05a7085583f76cb46ec623adfc1e0dd35c6a36e6) )
	ROM_LOAD( "epl12p6.9k", 0x0100, 0x0034, CRC(9b0bd6f8) SHA1(9dceb37245969537301b1f3c74f2c4ee088faa93) )
	ROM_LOAD( "epl12p6.9j", 0x0200, 0x0034, CRC(dcae870d) SHA1(2224724a3faf0608083f5d6ff76712adc7616a54) )
ROM_END

ROM_START( legendb )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "06.s02",    0x0000, 0x2000, CRC(227f3e88) SHA1(d31da7fe685f3249af6f42940d22d17399b9582c) )
	ROM_LOAD( "07.s03",    0x2000, 0x2000, CRC(9352e9dc) SHA1(ad7e0edce658bc6c0069025512ea4a2050a61533) )
	ROM_LOAD( "08.s04",    0x4000, 0x2000, CRC(41cee2b2) SHA1(166c0581aa83ba67f1912ebed80c1f02bd843ab6) )
	ROM_LOAD( "05.n03",    0x6000, 0x2000, CRC(d8fd4e37) SHA1(7c9d2d48a57a4d75682e0f910151a5c0e3229413) )

	ROM_REGION( 0x10000, "sub", 0 )
	ROM_LOAD( "02.s07",    0x0000, 0x2000, CRC(abfe5eb4) SHA1(fbeb5ee14aaebb6321fe97fe523f08833fad9c7c) )
	ROM_LOAD( "03.s08",    0x2000, 0x2000, CRC(7e7b9ba9) SHA1(897779129108b0f3936234ea797d47cf46cb7a16) )
	ROM_LOAD( "04.s09",    0x4000, 0x2000, CRC(0dd50aa7) SHA1(001ba0d5e0b50fb030a95fdbeba40005ffc5c182) )
	ROM_LOAD( "01.n07",    0x6000, 0x2000, CRC(13915a53) SHA1(25ba3babc8eb0df413bdfe7dbcd8642e4c658120) )

	ROM_REGION(  0x1000, "gfx1", 0 )    // fg tiles
	ROM_LOAD( "15.b05",    0x0000, 0x1000, CRC(6c879f76) SHA1(9da84446e463264ed86e912589d826d86c27bf59) )

	ROM_REGION(  0x6000, "gfx2", 0 )    // bg tiles
	ROM_LOAD( "18.j09",   0x0000, 0x2000, CRC(3bdcd028) SHA1(2fb2ecc5333e50834badb4b00093ca8e9a64bce4) )
	ROM_LOAD( "17.j10",   0x2000, 0x2000, CRC(105c5b53) SHA1(269da6bdef55024e593ea0178597e37ff2fefc10) )
	ROM_LOAD( "16.j11",   0x4000, 0x2000, CRC(b9ca4efd) SHA1(680c3ca88c65c1643ae82945b937d34579c0efeb) )

	ROM_REGION( 0x18000, "gfx3", 0 )    // sprites
	ROM_LOAD( "14.b06",   0x00000, 0x4000, CRC(1689f21c) SHA1(fafb13dc8ca27a7506065bbf08102afc6d722843) )
	ROM_LOAD( "13.b07",   0x04000, 0x4000, CRC(f527c909) SHA1(40f44828502018c73283965eb7a2a68ed25ebfe5) )
	ROM_LOAD( "12.b08",   0x08000, 0x4000, CRC(8d618629) SHA1(3cc49fd8066464ee940de010da3f33ed8573df3d) )
	ROM_LOAD( "11.b09",   0x0c000, 0x4000, CRC(7d7e2d55) SHA1(efd4817a0f5e14cb5a3d0c1f69e6ad408a813202) )
	ROM_LOAD( "10.b10",   0x10000, 0x4000, CRC(f12232fe) SHA1(2d8accc10f0703eeb075c4053d4165b90552e6a7) )
	ROM_LOAD( "09.b11",   0x14000, 0x4000, CRC(8c09243d) SHA1(8f0f68921f8ab6c016b7481714febb68abb7ce79) )

	ROM_REGION( 0x0340, "proms", 0 )
	ROM_LOAD( "82s129.1j",   0x0000, 0x0100, CRC(40590ac0) SHA1(30a8e24e34c4ee0a7df91c0633becfce1c8d856c) ) // red
	ROM_LOAD( "82s129.1h",   0x0100, 0x0100, CRC(e542b363) SHA1(6775209b9a4aaf374878c06cf4dc693b921abd87) ) // green
	ROM_LOAD( "82s129.1f",   0x0200, 0x0100, CRC(75536fc8) SHA1(e713efafcdc7f2a595444af75d2083eb3e38a480) ) // blue
	ROM_LOAD( "82s123.5j",   0x0300, 0x0020, CRC(c98f0651) SHA1(4f1b95213c28ad017c8d6542e8d522e4d69f91e3) ) // char lookup table
	ROM_LOAD( "m1.2c",       0x0320, 0x0020, CRC(83a39201) SHA1(4fdc722c9e20ee152c890342ef0dce18e35e2ef8) ) // timing? not used

	ROM_REGION( 0x0300, "plds", 0 )
	ROM_LOAD( "epl10p8.2j", 0x0000, 0x002c, CRC(8abc03bf) SHA1(05a7085583f76cb46ec623adfc1e0dd35c6a36e6) )
	ROM_LOAD( "epl12p6.9k", 0x0100, 0x0034, CRC(9b0bd6f8) SHA1(9dceb37245969537301b1f3c74f2c4ee088faa93) )
	ROM_LOAD( "epl12p6.9j", 0x0200, 0x0034, CRC(dcae870d) SHA1(2224724a3faf0608083f5d6ff76712adc7616a54) )
ROM_END

} // anonymous namespace



/*************************************
 *
 *  Game drivers
 *
 *************************************/

GAME( 1984, gyrodine,  0,        gyrodine,  gyrodine, kyugo_state, empty_init,    ROT90, "Crux",                                    "Gyrodine",                             MACHINE_SUPPORTS_SAVE )
GAME( 1984, gyrodinet, gyrodine, gyrodine,  gyrodine, kyugo_state, empty_init,    ROT90, "Crux (Taito Corporation license)",        "Gyrodine (Taito Corporation license)", MACHINE_SUPPORTS_SAVE )
GAME( 1984, buzzard,   gyrodine, gyrodine,  gyrodine, kyugo_state, empty_init,    ROT90, "Crux",                                    "Buzzard",                              MACHINE_SUPPORTS_SAVE )
GAME( 1985, repulse,   0,        repulse,   repulse,  kyugo_state, empty_init,    ROT90, "Crux / Sega",                             "Repulse",                              MACHINE_SUPPORTS_SAVE )
GAME( 1985, 99lstwar,  repulse,  repulse,   repulse,  kyugo_state, empty_init,    ROT90, "Crux / Proma",                            "'99: The Last War (set 1)",            MACHINE_SUPPORTS_SAVE ) // Crux went bankrupt during Repulse development,
GAME( 1985, 99lstwara, repulse,  repulse,   repulse,  kyugo_state, empty_init,    ROT90, "Crux / Proma",                            "'99: The Last War (set 2)",            MACHINE_SUPPORTS_SAVE ) // some of their staff later worked on the newer games on this hardware,
GAME( 1985, 99lstwark, repulse,  repulse,   repulse,  kyugo_state, empty_init,    ROT90, "Crux / Kyugo",                            "'99: The Last War (Kyugo)",            MACHINE_SUPPORTS_SAVE ) // directly for Kyugo? (Flashgal, Legend, SRD Mission, Airwolf, Planet Probe)
GAME( 1985, 99lstwarb, repulse,  repulse,   repulse,  kyugo_state, empty_init,    ROT90, "bootleg",                                 "'99: The Last War (bootleg)",          MACHINE_SUPPORTS_SAVE | MACHINE_IMPERFECT_GRAPHICS ) // bg_tilemap is wrong in some levels
GAME( 1985, sonofphx,  repulse,  repulse,   repulse,  kyugo_state, empty_init,    ROT90, "bootleg (Associated Overseas MFR, Inc.)", "Son of Phoenix (bootleg of Repulse)",  MACHINE_SUPPORTS_SAVE )
GAME( 1985, flashgal,  0,        repulse,   flashgal, kyugo_state, empty_init,    ROT0,  "Kyugo / Sega",                            "Flashgal (set 1)",                     MACHINE_SUPPORTS_SAVE )
GAME( 1985, flashgalk, flashgal, repulse,   flashgal, kyugo_state, empty_init,    ROT0,  "Kyugo / Sega",                            "Flashgal (set 1, Kyugo logo)",         MACHINE_SUPPORTS_SAVE )
GAME( 1985, flashgala, flashgal, flashgala, flashgal, kyugo_state, empty_init,    ROT0,  "Kyugo / Sega",                            "Flashgal (set 2)",                     MACHINE_SUPPORTS_SAVE )
GAME( 1986, srdmissn,  0,        srdmissn,  srdmissn, kyugo_state, empty_init,    ROT90, "Kyugo / Taito Corporation",               "S.R.D. Mission",                       MACHINE_SUPPORTS_SAVE )
GAME( 1986, fx,        srdmissn, srdmissn,  srdmissn, kyugo_state, empty_init,    ROT90, "bootleg",                                 "F-X (bootleg of S.R.D. Mission)",      MACHINE_SUPPORTS_SAVE )
GAME( 1986, legend,    0,        legend,    legend,   kyugo_state, empty_init,    ROT0,  "Kyugo / Sega",                            "Legend",                               MACHINE_SUPPORTS_SAVE ) // no copyright (maybe also a bootleg?)
GAME( 1986, legendb,   legend,   legend,    legend,   kyugo_state, empty_init,    ROT0,  "bootleg",                                 "Legion (bootleg of Legend)",           MACHINE_SUPPORTS_SAVE ) // no copyright
GAME( 1987, airwolf,   0,        srdmissn,  airwolf,  kyugo_state, empty_init,    ROT0,  "Kyugo",                                   "Airwolf",                              MACHINE_SUPPORTS_SAVE )
GAME( 1987, airwolfa,  airwolf,  srdmissn,  airwolf,  kyugo_state, empty_init,    ROT0,  "Kyugo (United Amusements license)",       "Airwolf (US)",                         MACHINE_SUPPORTS_SAVE )
GAME( 1987, skywolf,   airwolf,  srdmissn,  skywolf,  kyugo_state, empty_init,    ROT0,  "bootleg",                                 "Sky Wolf (set 1)",                     MACHINE_SUPPORTS_SAVE )
GAME( 1987, skywolf2,  airwolf,  srdmissn,  airwolf,  kyugo_state, empty_init,    ROT0,  "bootleg",                                 "Sky Wolf (set 2)",                     MACHINE_SUPPORTS_SAVE )
GAME( 1987, skywolf3,  airwolf,  srdmissn,  airwolf,  kyugo_state, empty_init,    ROT0,  "bootleg",                                 "Sky Wolf (set 3)",                     MACHINE_SUPPORTS_SAVE )
