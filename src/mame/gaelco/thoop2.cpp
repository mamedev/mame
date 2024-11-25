// license:BSD-3-Clause
// copyright-holders: Manuel Abadia, Peter Ferrie, David Haywood

/***************************************************************************

Thunder Hoop II: Strikes Back (c) 1994 Gaelco

Driver by Manuel Abadia <emumanu+mame@gmail.com>

updated by Peter Ferrie <peter.ferrie@gmail.com>


REF.940411
+-------------------------------------------------+
|       C1                                  6116  |
|  VOL  C2*                                 6116  |
|          30MHz                            6116  |
|    M6295                    +----------+  6116  |
|     1MHz                    |TMS       |        |
|       6116                  |TPC1020AFN|        |
|J      6116                  |   -084C  |    H8  |
|A     +------------+         +----------+        |
|M     |DS5002FP Box|         +----------+        |
|M     +------------+         |TMS       |    H12 |
|A             65756          |TPC1020AFN|        |
|              65756          |   -084C  |        |
|                             +----------+        |
|SW1                                   PAL   65764|
|     24MHz    MC68000P12                    65764|
|SW2           C22                    6116        |
|      PAL     C23                    6116        |
+-------------------------------------------------+

  CPU: MC68000P12 & DS5002FP (used for protection)
Sound: OKI M6295
  OSC: 30MHz, 24MHz & 1MHz resonator
  RAM: MHS HM3-65756K-5  32K x 8 SRAM (x2)
       MHS HM3-65764E-5  8K x 8 SRAM (x2)
       UM6116BK-35  2K x 8 SRAM (x8)
  PAL: TI F20L8-25CNT DIP24 (x2)
  VOL: Volume pot
   SW: Two 8 switch dipswitches

DS5002FP Box contains:
  Dallas DS5002SP @ 12MHz
  KM62256BLG-7L - 32Kx8 Low Power CMOS SRAM
  3.6v Battery
  JP1 - 5 pin port to program SRAM

Measurements from actual PCB:
  DS5002FP - 12MHz
  OKI MSM6295 - 1MHz, pin 7 is disconnected (neither pulled LOW or HIGH)
  H-SYNC - 15.151KHz
  V-SYNC - 59.24Hz

***************************************************************************/

#include "emu.h"

#include "gaelco_ds5002fp.h"

#include "cpu/m68000/m68000.h"
#include "cpu/mcs51/mcs51.h"
#include "machine/74259.h"
#include "machine/watchdog.h"
#include "sound/okim6295.h"

#include "emupal.h"
#include "screen.h"
#include "speaker.h"
#include "tilemap.h"


namespace {

class thoop2_state : public driver_device
{
public:
	thoop2_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_pant{ nullptr, nullptr },
		m_maincpu(*this, "maincpu"),
		m_outlatch(*this, "outlatch"),
		m_gfxdecode(*this, "gfxdecode"),
		m_palette(*this, "palette"),
		m_okibank(*this, "okibank"),
		m_videoram(*this, "videoram"),
		m_vregs(*this, "vregs"),
		m_spriteram(*this, "spriteram"),
		m_shareram(*this, "shareram")
	{ }

	void thoop2(machine_config &config);

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void video_start() override ATTR_COLD;

private:
	void oki_bankswitch_w(uint8_t data);
	template <uint8_t Which> void coin_lockout_w(int state);
	template <uint8_t Which> void coin_counter_w(int state);

	void vram_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	void shareram_w(offs_t offset, uint8_t data);
	uint8_t shareram_r(offs_t offset);

	template<int Layer> TILE_GET_INFO_MEMBER(get_tile_info);
	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

	void mcu_hostmem_map(address_map &map) ATTR_COLD;
	void oki_map(address_map &map) ATTR_COLD;
	void thoop2_map(address_map &map) ATTR_COLD;

	void draw_sprites(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

	tilemap_t *m_pant[2]{};

	required_device<cpu_device> m_maincpu;
	required_device<ls259_device> m_outlatch;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;
	required_memory_bank m_okibank;

	required_shared_ptr<uint16_t> m_videoram;
	required_shared_ptr<uint16_t> m_vregs;
	required_shared_ptr<uint16_t> m_spriteram;
	required_shared_ptr<uint16_t> m_shareram;
};


/***************************************************************************

  Gaelco Type 1 Video Hardware Rev B

  The video hardware is nearly identical to the previous
  revision but it can handle more tiles and more sprites

***************************************************************************/

/***************************************************************************

    Callbacks for the TileMap code

***************************************************************************/

/*
    Tile format
    -----------

    Screen 0 & 1: (32*32, 16x16 tiles)

    Word | Bit(s)            | Description
    -----+-FEDCBA98-76543210-+--------------------------
      0  | -------- ------xx | code (high bits)
      0  | xxxxxxxx xxxxxx-- | code (low bits)
      1  | -------- --xxxxxx | color
      1  | -------- xx------ | priority
      1  | --xxxxxx -------- | not used
      1  | -x------ -------- | flip x
      1  | x------- -------- | flip y
*/

template<int Layer>
TILE_GET_INFO_MEMBER(thoop2_state::get_tile_info)
{
	int const data = m_videoram[(Layer * 0x1000 / 2) + (tile_index << 1)];
	int const data2 = m_videoram[(Layer * 0x1000 / 2) + (tile_index << 1) + 1];
	int const code = ((data & 0xfffc) >> 2) | ((data & 0x0003) << 14);

	tileinfo.category = (data2 >> 6) & 0x03;

	tileinfo.set(1, code, data2 & 0x3f, TILE_FLIPYX((data2 >> 14) & 0x03));
}

/***************************************************************************

    Memory Handlers

***************************************************************************/

void thoop2_state::vram_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	COMBINE_DATA(&m_videoram[offset]);
	m_pant[offset >> 11]->mark_tile_dirty(((offset << 1) & 0x0fff) >> 2);
}

/***************************************************************************

    Start/Stop the video hardware emulation.

***************************************************************************/

void thoop2_state::video_start()
{
	m_pant[0] = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(thoop2_state::get_tile_info<0>)), TILEMAP_SCAN_ROWS, 16,16, 32,32);
	m_pant[1] = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(thoop2_state::get_tile_info<1>)), TILEMAP_SCAN_ROWS, 16,16, 32,32);

	m_pant[0]->set_transmask(0, 0xff01, 0x00ff); // pens 1-7 opaque, pens 0, 8-15 transparent
	m_pant[1]->set_transmask(0, 0xff01, 0x00ff); // pens 1-7 opaque, pens 0, 8-15 transparent
}

/***************************************************************************

    Sprites

***************************************************************************/

/*
    Sprite Format
    -------------

    Word | Bit(s)            | Description
    -----+-FEDCBA98-76543210-+--------------------------
      0  | -------- xxxxxxxx | y position
      0  | -----xxx -------- | not used
      0  | ----x--- -------- | sprite size
      0  | --xx---- -------- | sprite priority
      0  | -x------ -------- | flipx
      0  | x------- -------- | flipy
      1  | xxxxxxxx xxxxxxxx | not used
      2  | -------x xxxxxxxx | x position
      2  | -xxxxxx- -------- | sprite color
      3  | -------- ------xx | sprite code (high bits)
      3  | xxxxxxxx xxxxxx-- | sprite code (low bits)
*/

void thoop2_state::draw_sprites(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	gfx_element *gfx = m_gfxdecode->gfx(0);

	static const int x_offset[2] = {0x0, 0x2};
	static const int y_offset[2] = {0x0, 0x1};

	for (int i = 0x800 - 4 - 1; i >= 3; i -= 4)
	{
		int const sx = m_spriteram[i + 2] & 0x01ff;
		int const sy = (240 - (m_spriteram[i] & 0x00ff)) & 0x00ff;
		int number = m_spriteram[i + 3];
		int const color = (m_spriteram[i + 2] & 0x7e00) >> 9;
		int const attr = (m_spriteram[i] & 0xfe00) >> 9;
		int priority = (m_spriteram[i] & 0x3000) >> 12;

		int const xflip = attr & 0x20;
		int const yflip = attr & 0x40;
		int spr_size, pri_mask;

		// palettes 0x38-0x3f are used for high priority sprites
		if (color >= 0x38)
			priority = 4;

		switch (priority)
		{
			case 0: pri_mask = 0xff00; break;
			case 1: pri_mask = 0xff00 | 0xf0f0; break;
			case 2: pri_mask = 0xff00 | 0xf0f0 | 0xcccc; break;
			case 3: pri_mask = 0xff00 | 0xf0f0 | 0xcccc | 0xaaaa; break;
			default:
			case 4: pri_mask = 0; break;
		}

		number |= ((number & 0x03) << 16);

		if (attr & 0x04)
		{
			spr_size = 1;
		}
		else
		{
			spr_size = 2;
			number &= (~3);
		}

		for (int y = 0; y < spr_size; y++)
		{
			for (int x = 0; x < spr_size; x++)
			{
				int const ex = xflip ? (spr_size-1-x) : x;
				int const ey = yflip ? (spr_size-1-y) : y;

				gfx->prio_transpen(bitmap, cliprect, number + x_offset[ex] + y_offset[ey],
						color, xflip, yflip,
						sx - 0x0f + x * 8, sy + y * 8,
						screen.priority(), pri_mask, 0);
			}
		}
	}
}

/***************************************************************************

    Display Refresh

***************************************************************************/

uint32_t thoop2_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	// set scroll registers
	m_pant[0]->set_scrolly(0, m_vregs[0]);
	m_pant[0]->set_scrollx(0, m_vregs[1] + 4);
	m_pant[1]->set_scrolly(0, m_vregs[2]);
	m_pant[1]->set_scrollx(0, m_vregs[3]);

	screen.priority().fill(0, cliprect);
	bitmap.fill(0, cliprect);

	m_pant[1]->draw(screen, bitmap, cliprect, TILEMAP_DRAW_LAYER1 | 3, 0);
	m_pant[0]->draw(screen, bitmap, cliprect, TILEMAP_DRAW_LAYER1 | 3, 0);

	m_pant[1]->draw(screen, bitmap, cliprect, TILEMAP_DRAW_LAYER0 | 3, 1);
	m_pant[0]->draw(screen, bitmap, cliprect, TILEMAP_DRAW_LAYER0 | 3, 1);

	m_pant[1]->draw(screen, bitmap, cliprect, TILEMAP_DRAW_LAYER1 | 2, 1);
	m_pant[0]->draw(screen, bitmap, cliprect, TILEMAP_DRAW_LAYER1 | 2, 1);

	m_pant[1]->draw(screen, bitmap, cliprect, TILEMAP_DRAW_LAYER0 | 2, 2);
	m_pant[0]->draw(screen, bitmap, cliprect, TILEMAP_DRAW_LAYER0 | 2, 2);

	m_pant[1]->draw(screen, bitmap, cliprect, TILEMAP_DRAW_LAYER1 | 1, 2);
	m_pant[0]->draw(screen, bitmap, cliprect, TILEMAP_DRAW_LAYER1 | 1, 2);

	m_pant[1]->draw(screen, bitmap, cliprect, TILEMAP_DRAW_LAYER0 | 1, 4);
	m_pant[0]->draw(screen, bitmap, cliprect, TILEMAP_DRAW_LAYER0 | 1, 4);

	m_pant[1]->draw(screen, bitmap, cliprect, TILEMAP_DRAW_LAYER1 | 0, 4);
	m_pant[0]->draw(screen, bitmap, cliprect, TILEMAP_DRAW_LAYER1 | 0, 4);

	m_pant[1]->draw(screen, bitmap, cliprect, TILEMAP_DRAW_LAYER0 | 0, 8);
	m_pant[0]->draw(screen, bitmap, cliprect, TILEMAP_DRAW_LAYER0 | 0, 8);

	draw_sprites(screen, bitmap, cliprect);
	return 0;
}


void thoop2_state::machine_start()
{
	m_okibank->configure_entries(0, 16, memregion("oki")->base(), 0x10000);
}

void thoop2_state::oki_bankswitch_w(uint8_t data)
{
	m_okibank->set_entry(data & 0x0f);
}

template <uint8_t Which>
void thoop2_state::coin_lockout_w(int state)
{
	machine().bookkeeping().coin_lockout_w(Which, !state);
}

template <uint8_t Which>
void thoop2_state::coin_counter_w(int state)
{
	machine().bookkeeping().coin_counter_w(Which, state);
}

void thoop2_state::shareram_w(offs_t offset, uint8_t data)
{
	// why isn't there address map functionality for this?
	util::big_endian_cast<uint8_t>(m_shareram.target())[offset] = data;
}

uint8_t thoop2_state::shareram_r(offs_t offset)
{
	// why isn't there address map functionality for this?
	return util::big_endian_cast<uint8_t const>(m_shareram.target())[offset];
}


void thoop2_state::mcu_hostmem_map(address_map &map)
{
	map(0x8000, 0xffff).rw(FUNC(thoop2_state::shareram_r), FUNC(thoop2_state::shareram_w)); // confirmed that 0x8000 - 0xffff is a window into 68k shared RAM
}


void thoop2_state::thoop2_map(address_map &map)
{
	map(0x000000, 0x0fffff).rom();
	map(0x100000, 0x101fff).ram().w(FUNC(thoop2_state::vram_w)).share(m_videoram);
	map(0x108000, 0x108007).writeonly().share(m_vregs);
	map(0x10800c, 0x10800d).w("watchdog", FUNC(watchdog_timer_device::reset16_w));                           // INT 6 ACK / Watchdog timer
	map(0x200000, 0x2007ff).ram().w(m_palette, FUNC(palette_device::write16)).share("palette");
	map(0x440000, 0x440fff).ram().share(m_spriteram);
	map(0x700000, 0x700001).portr("DSW2");
	map(0x700002, 0x700003).portr("DSW1");
	map(0x700004, 0x700005).portr("P1");
	map(0x700006, 0x700007).portr("P2");
	map(0x700008, 0x700009).portr("SYSTEM");
	map(0x70000b, 0x70000b).select(0x000070).lw8(NAME([this] (offs_t offset, u8 data) { m_outlatch->write_d0(offset >> 4, data); }));
	map(0x70000d, 0x70000d).w(FUNC(thoop2_state::oki_bankswitch_w));
	map(0x70000f, 0x70000f).rw("oki", FUNC(okim6295_device::read), FUNC(okim6295_device::write));
	map(0xfe0000, 0xfe7fff).ram();                                          // Work RAM
	map(0xfe8000, 0xfeffff).ram().share(m_shareram);                     // Work RAM (shared with D5002FP)
}


void thoop2_state::oki_map(address_map &map)
{
	map(0x00000, 0x2ffff).rom();
	map(0x30000, 0x3ffff).bankr(m_okibank);
}


static INPUT_PORTS_START( thoop2 )
	PORT_START("DSW1")
	PORT_DIPNAME( 0x07, 0x07, DEF_STR( Coin_B ) )
	PORT_DIPSETTING(    0x02, DEF_STR( 6C_1C ) )
	PORT_DIPSETTING(    0x03, DEF_STR( 5C_1C ) )
	PORT_DIPSETTING(    0x04, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(    0x05, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x06, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x01, DEF_STR( 3C_2C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 4C_3C ) )
	PORT_DIPSETTING(    0x07, DEF_STR( 1C_1C ) )
	PORT_DIPNAME( 0x38, 0x38, DEF_STR( Coin_A ) )
	PORT_DIPSETTING(    0x38, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 3C_4C ) )
	PORT_DIPSETTING(    0x08, DEF_STR( 2C_3C ) )
	PORT_DIPSETTING(    0x30, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x28, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0x20, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(    0x18, DEF_STR( 1C_5C ) )
	PORT_DIPSETTING(    0x10, DEF_STR( 1C_6C ) )
	PORT_DIPNAME( 0x40, 0x40, "Credit configuration" )
	PORT_DIPSETTING(    0x40, "Start 1C/Continue 1C" )
	PORT_DIPSETTING(    0x00, "Start 2C/Continue 1C" )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Free_Play ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("DSW2")
	PORT_DIPNAME( 0x03, 0x03, DEF_STR( Difficulty ) )
	PORT_DIPSETTING(    0x03, DEF_STR( Normal ) )
	PORT_DIPSETTING(    0x02, DEF_STR( Easy ) )
	PORT_DIPSETTING(    0x01, DEF_STR( Hard ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Hardest ) )
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x18, 0x18, DEF_STR( Lives ) )
	PORT_DIPSETTING(    0x18, "2" )
	PORT_DIPSETTING(    0x10, "3" )
	PORT_DIPSETTING(    0x08, "4" )
	PORT_DIPSETTING(    0x00, "1" )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Demo_Sounds ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x20, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_SERVICE( 0x80, IP_ACTIVE_LOW )

	PORT_START("P1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(1)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_COIN2 )

	PORT_START("P2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_START2 )

	PORT_START("SYSTEM")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_SERVICE2 )   // test button
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(1)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(2)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNUSED )
INPUT_PORTS_END


static const gfx_layout thoop2_tilelayout =
{
	8,8,                                    // 8x8 tiles
	RGN_FRAC(1,2),                          // number of tiles
	4,                                      // 4 bpp
	{ 8, 0, RGN_FRAC(1,2)+8, RGN_FRAC(1,2)+0 },
	{ STEP8(0,1) },
	{ STEP8(0,8*2) },
	16*8
};

static const gfx_layout thoop2_tilelayout_16 =
{
	16,16,                                  // 16x16 tiles
	RGN_FRAC(1,2),                          // number of tiles
	4,                                      // 4 bpp
	{ 8, 0, RGN_FRAC(1,2)+8, RGN_FRAC(1,2)+0 },
	{ STEP8(0,1), STEP8(8*2*16,1) },
	{ STEP16(0,8*2) },
	64*8
};


static GFXDECODE_START( gfx_thoop2 )
	GFXDECODE_ENTRY( "gfx", 0x000000, thoop2_tilelayout,    0, 64 )
	GFXDECODE_ENTRY( "gfx", 0x000000, thoop2_tilelayout_16, 0, 64 )
GFXDECODE_END


void thoop2_state::thoop2(machine_config &config)
{
	// basic machine hardware
	M68000(config, m_maincpu, XTAL(24'000'000) / 2); // 12MHz verified
	m_maincpu->set_addrmap(AS_PROGRAM, &thoop2_state::thoop2_map);
	m_maincpu->set_vblank_int("screen", FUNC(thoop2_state::irq6_line_hold));

	gaelco_ds5002fp_device &ds5002fp(GAELCO_DS5002FP(config, "gaelco_ds5002fp", XTAL(24'000'000) / 2)); // 12MHz verified
	ds5002fp.set_addrmap(0, &thoop2_state::mcu_hostmem_map);
	config.set_perfect_quantum("gaelco_ds5002fp:mcu");

	LS259(config, m_outlatch);
	m_outlatch->q_out_cb<0>().set(FUNC(thoop2_state::coin_lockout_w<0>));
	m_outlatch->q_out_cb<1>().set(FUNC(thoop2_state::coin_lockout_w<1>));
	m_outlatch->q_out_cb<2>().set(FUNC(thoop2_state::coin_counter_w<0>));
	m_outlatch->q_out_cb<3>().set(FUNC(thoop2_state::coin_counter_w<1>));
	m_outlatch->q_out_cb<4>().set_nop(); // unknown. Sound related?
	m_outlatch->q_out_cb<5>().set_nop(); // unknown

	WATCHDOG_TIMER(config, "watchdog");

	// video hardware
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_refresh_hz(59.24);
	screen.set_vblank_time(ATTOSECONDS_IN_USEC(2500)); // not accurate
	screen.set_size(32*16, 32*16);
	screen.set_visarea(0, 320-1, 16, 256-1);
	screen.set_screen_update(FUNC(thoop2_state::screen_update));
	screen.set_palette(m_palette);

	GFXDECODE(config, m_gfxdecode, m_palette, gfx_thoop2);
	PALETTE(config, m_palette).set_format(palette_device::xBGR_555, 1024);

	// sound hardware
	SPEAKER(config, "mono").front_center();

	okim6295_device &oki(OKIM6295(config, "oki", XTAL(1'000'000), okim6295_device::PIN7_HIGH)); // 1MHz resonator - pin 7 not connected
	oki.set_addrmap(0, &thoop2_state::oki_map);
	oki.add_route(ALL_OUTPUTS, "mono", 1.0);
}



ROM_START( thoop2 ) // REF.940411 PCB
	ROM_REGION( 0x100000, "maincpu", 0 )    // 68000 code
	ROM_LOAD16_BYTE(    "th2c23.c23",   0x000000, 0x080000, CRC(3e465753) SHA1(1ea1173b9fe5d652e7b5fafb822e2535cecbc198) )
	ROM_LOAD16_BYTE(    "th2c22.c22",   0x000001, 0x080000, CRC(837205b7) SHA1(f78b90c2be0b4dddaba26f074ea00eff863cfdb2) )

	ROM_REGION( 0x8000, "gaelco_ds5002fp:sram", 0 ) // DS5002FP code
	ROM_LOAD( "thoop2_ds5002fp.bin", 0x00000, 0x8000, CRC(6881384d) SHA1(c1eff5558716293e1325b766e2205783286c12f9) ) // dumped from 3 boards, reconstructed with 2/3 wins rule, all bytes verified by hand as correct

	ROM_REGION( 0x100, "gaelco_ds5002fp:mcu:internal", ROMREGION_ERASE00 )
	// these are the default states stored in NVRAM
	DS5002FP_SET_MON( 0x79 )
	DS5002FP_SET_RPCTL( 0x00 )
	DS5002FP_SET_CRCR( 0x80 )

	ROM_REGION( 0x800000, "gfx", 0 )
	ROM_LOAD( "th2-h8.h8",     0x000000, 0x400000, CRC(60328a11) SHA1(fcdb374d2fc7ef5351a4181c471d192199dc2081) )
	ROM_LOAD( "th2-h12.h12",   0x400000, 0x400000, CRC(b25c2d3e) SHA1(d70f3e4e2432d80c2ac87cd81208ada303bac04a) )

	ROM_REGION( 0x100000, "oki", 0 )
	ROM_LOAD( "th2-c1.c1",     0x000000, 0x100000, CRC(8fac8c30) SHA1(8e49bb596144761eae95f3e1266e57fb386664f2) )
	// 0x00000-0x2ffff is fixed, 0x30000-0x3ffff is bank switched

	ROM_REGION(0x0a00, "plds", 0)
	ROM_LOAD("pal16r8-1.b16",   0x0000, 0x0104, CRC(27b1ca8b) SHA1(038d1352baff18f619ac4149e5825ef9664c983b) )
	ROM_LOAD("pal20l8-2.b23",   0x0200, 0x0144, CRC(87e5e6ab) SHA1(f42b952128bd26fe565b06403c7b1c95061e5034) )
	ROM_LOAD("pal16r4-3.e2",    0x0400, 0x0104, CRC(0488f37b) SHA1(e2e4e3ca57713da7b0fcae57b34b0bfc5c9d3635) )
	ROM_LOAD("pal20l8-4.h15",   0x0600, 0x0144, CRC(49053906) SHA1(f6c92f26d66c7286f9845ebf7d3ee729ed0b4f22) )
	ROM_LOAD("palce16v8-5.h21", 0x0800, 0x0117, CRC(b651bc3b) SHA1(89f8bc2d3ae710189912373464c2f35b6780357d) )
ROM_END

ROM_START( thoop2a ) // REF.940411 PCB
	ROM_REGION( 0x100000, "maincpu", 0 )    // 68000 code
	ROM_LOAD16_BYTE(    "3.c23",   0x000000, 0x080000, CRC(6cd4a8dc) SHA1(7d0cdce64b390c3f9769b07d57cf1eee1e6a7bf5) )
	ROM_LOAD16_BYTE(    "2.c22",   0x000001, 0x080000, CRC(59ba9b43) SHA1(6c6690a2e389fc9f1e166c87748da1175e3b58f8) )

	ROM_REGION( 0x8000, "gaelco_ds5002fp:sram", 0 ) // DS5002FP code
	ROM_LOAD( "thoop2_ds5002fp.bin", 0x00000, 0x8000, CRC(6881384d) SHA1(c1eff5558716293e1325b766e2205783286c12f9) ) // dumped from 3 boards, reconstructed with 2/3 wins rule, all bytes verified by hand as correct

	ROM_REGION( 0x100, "gaelco_ds5002fp:mcu:internal", ROMREGION_ERASE00 )
	// these are the default states stored in NVRAM
	DS5002FP_SET_MON( 0x79 )
	DS5002FP_SET_RPCTL( 0x00 )
	DS5002FP_SET_CRCR( 0x80 )

	ROM_REGION( 0x800000, "gfx", 0 )
	ROM_LOAD( "th2-h8.h8",     0x000000, 0x400000, CRC(60328a11) SHA1(fcdb374d2fc7ef5351a4181c471d192199dc2081) )
	ROM_LOAD( "th2-h12.h12",   0x400000, 0x400000, CRC(b25c2d3e) SHA1(d70f3e4e2432d80c2ac87cd81208ada303bac04a) )

	ROM_REGION( 0x100000, "oki", 0 )
	ROM_LOAD( "th2-c1.c1",     0x000000, 0x100000, CRC(8fac8c30) SHA1(8e49bb596144761eae95f3e1266e57fb386664f2) )
	// 0x00000-0x2ffff is fixed, 0x30000-0x3ffff is bank switched

	ROM_REGION(0x0a00, "plds", 0)
	ROM_LOAD("pal16r8-1.b16",   0x0000, 0x0104, CRC(27b1ca8b) SHA1(038d1352baff18f619ac4149e5825ef9664c983b) )
	ROM_LOAD("pal20l8-2.b23",   0x0200, 0x0144, CRC(87e5e6ab) SHA1(f42b952128bd26fe565b06403c7b1c95061e5034) )
	ROM_LOAD("pal16r4-3.e2",    0x0400, 0x0104, CRC(0488f37b) SHA1(e2e4e3ca57713da7b0fcae57b34b0bfc5c9d3635) )
	ROM_LOAD("pal20l8-4.h15",   0x0600, 0x0144, CRC(49053906) SHA1(f6c92f26d66c7286f9845ebf7d3ee729ed0b4f22) )
	ROM_LOAD("palce16v8-5.h21", 0x0800, 0x0117, CRC(b651bc3b) SHA1(89f8bc2d3ae710189912373464c2f35b6780357d) )
ROM_END

} // anonymous namespace


GAME( 1994, thoop2,       0, thoop2, thoop2, thoop2_state, empty_init, ROT0, "Gaelco", "TH Strikes Back (Non North America, Version 1.0, Checksum 020E0867)", MACHINE_IMPERFECT_GRAPHICS | MACHINE_SUPPORTS_SAVE )
GAME( 1994, thoop2a, thoop2, thoop2, thoop2, thoop2_state, empty_init, ROT0, "Gaelco", "TH Strikes Back (Non North America, Version 1.0, Checksum 020EB356)", MACHINE_IMPERFECT_GRAPHICS | MACHINE_SUPPORTS_SAVE )
