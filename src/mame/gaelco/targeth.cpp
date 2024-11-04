// license:BSD-3-Clause
// copyright-holders: Manuel Abadia, David Haywood

/***************************************************************************

Target Hits (c) 1994 Gaelco (Designed & Developed by Zigurat. Produced by Gaelco)

Driver by Manuel Abadia <emumanu+mame@gmail.com>

** NOTES: Merge with wrally.cpp???  Address map nearly identical & PCB
          is reworked to add connections for light guns and different RAM

          is the visible area correct? if it's larger than the startup
          screen then scene 5 of desert chariots doesn't cover the entire
          screen either.

          the instructions say to reload the gun after each shot, but
          there is no reload button listed in service mode, and it doesn't
          seem to be required, was it a mechanical feature of the gun or
          is our logic inverted somewhere, the gun test shows 'ON' when
          you're not pressing fire too.

REF.940531
+-------------------------------------------------+
|       C1                                 6116   |
|  VOL  C2                                 6116   |
|          30MHz                                  |
|    M6295                   +----------+         |
|     1MHz                   |TMS       |         |
|      6116                  |TPC1020AFN|         |
|J     6116                  |   -084C  |      I7 |
|A     +------------+        +----------+  H8* I9 |
|M     |DS5002FP Box|        +----------+      I11|
|M     +------------+        |TMS       | H12* I13|
|A             65756         |TPC1020AFN|         |
| JP4          65756         |   -084C  |         |
| JP5                        +----------+         |
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
       UM6116BK-35  2K x 8 SRAM (x6)
  PAL: TI F20L8-25CNT DIP24 (x2)
  VOL: Volume pot
   SW: Two 8 switch dipswitches (SW1 unpopulated)
  JP4: 5 pin header for light gun player 2
  JP5: 5 pin header for light gun player 1

DS5002FP Box contains:
  Dallas DS5002SP @ 12MHz
  KM62256BLG-7L - 32Kx8 Low Power CMOS SRAM
  3.6v Battery
  JP1 - 5 pin port to program SRAM

* NOTE: PCB can use four 27C040 EPROMs at I7, I9, I11 & I13 or two 8Meg MASK
        ROMs at H8 & H12.  Same set up as used on the World Rally PCBs

***************************************************************************/

#include "emu.h"

#include "gaelco_ds5002fp.h"

#include "cpu/mcs51/mcs51.h"
#include "cpu/m68000/m68000.h"
#include "machine/74259.h"
#include "sound/okim6295.h"

#include "emupal.h"
#include "screen.h"
#include "speaker.h"
#include "tilemap.h"


namespace {

class targeth_state : public driver_device
{
public:
	targeth_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this,"maincpu"),
		m_gfxdecode(*this, "gfxdecode"),
		m_screen(*this, "screen"),
		m_palette(*this, "palette"),
		m_outlatch(*this, "outlatch"),
		m_videoram(*this, "videoram"),
		m_vregs(*this, "vregs"),
		m_spriteram(*this, "spriteram"),
		m_shareram(*this, "shareram"),
		m_okibank(*this, "okibank")
	{ }

	void targeth(machine_config &config);

protected:
	virtual void video_start() override ATTR_COLD;
	virtual void machine_start() override ATTR_COLD;

private:
	void oki_bankswitch_w(uint8_t data);
	void output_latch_w(offs_t offset, uint16_t data);
	template <uint8_t Which> void coin_counter_w(int state);
	void shareram_w(offs_t offset, uint8_t data);
	uint8_t shareram_r(offs_t offset);

	void vram_w(offs_t offset, uint16_t data);

	template<int Layer> TILE_GET_INFO_MEMBER(get_tile_info);

	TIMER_CALLBACK_MEMBER(gun1_irq);
	TIMER_CALLBACK_MEMBER(gun2_irq);

	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

	void main_map(address_map &map) ATTR_COLD;
	void mcu_hostmem_map(address_map &map) ATTR_COLD;
	void oki_map(address_map &map) ATTR_COLD;

	void draw_sprites(bitmap_ind16 &bitmap, const rectangle &cliprect);

	required_device<cpu_device> m_maincpu;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<screen_device> m_screen;
	required_device<palette_device> m_palette;
	required_device<ls259_device> m_outlatch;

	required_shared_ptr<uint16_t> m_videoram;
	required_shared_ptr<uint16_t> m_vregs;
	required_shared_ptr<uint16_t> m_spriteram;
	required_shared_ptr<uint16_t> m_shareram;

	required_memory_bank m_okibank;

	emu_timer *m_gun_irq_timer[2]{};

	tilemap_t *m_pant[2]{};
};


/***************************************************************************

    Callbacks for the TileMap code

***************************************************************************/

/*
    Tile format
    -----------

    Screen 0 & 1: (64*32, 16x16 tiles)

    Word | Bit(s)            | Description
    -----+-FEDCBA98-76543210-+--------------------------
      0  | --xxxxxx xxxxxxxx | code
      0  | xx------ -------- | not used?
      1  | -------- ---xxxxx | color (uses 1st half of the palette)
      1  | -------- --x----- | flip y
      1  | -------- -x------ | flip x
      1  | xxxxxxxx x------- | not used?
*/

template<int Layer>
TILE_GET_INFO_MEMBER(targeth_state::get_tile_info)
{
	int const data = m_videoram[(Layer * 0x2000 / 2) + (tile_index << 1)];
	int const data2 = m_videoram[(Layer * 0x2000 / 2) + (tile_index << 1) + 1];
	int const code = data & 0x3fff;

	tileinfo.set(0, code, data2 & 0x1f, TILE_FLIPXY((data2 >> 5) & 0x03));
}

/***************************************************************************

    Memory Handlers

***************************************************************************/

void targeth_state::vram_w(offs_t offset, uint16_t data)
{
	m_videoram[offset] = data;
	m_pant[(offset & 0x1fff) >> 12]->mark_tile_dirty(((offset << 1) & 0x1fff) >> 2);
}


/***************************************************************************

    Start/Stop the video hardware emulation.

***************************************************************************/

void targeth_state::video_start()
{
	m_pant[0] = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(targeth_state::get_tile_info<0>)), TILEMAP_SCAN_ROWS, 16,16, 64,32);
	m_pant[1] = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(targeth_state::get_tile_info<1>)), TILEMAP_SCAN_ROWS, 16,16, 64,32);

	m_pant[0]->set_transparent_pen(0);
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
      0  | --xxxxxx -------- | not used?
      0  | -x------ -------- | flipx
      0  | x------- -------- | flipy
      1  | xxxxxxxx xxxxxxxx | not used?
      2  | ------xx xxxxxxxx | x position
      2  | -xxxxx-- -------- | sprite color (uses 2nd half of the palette)
      3  | --xxxxxx xxxxxxxx | sprite code
      3  | xx------ -------- | not used?
*/

void targeth_state::draw_sprites(bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	gfx_element *gfx = m_gfxdecode->gfx(0);

	for (int i = 3; i < (0x1000 - 6)/2; i += 4){
		int const sx = m_spriteram[i+2] & 0x03ff;
		int const sy = (240 - (m_spriteram[i] & 0x00ff)) & 0x00ff;
		int const number = m_spriteram[i+3] & 0x3fff;
		int const color = (m_spriteram[i+2] & 0x7c00) >> 10;
		int const attr = (m_spriteram[i] & 0xfe00) >> 9;

		int const xflip = attr & 0x20;
		int const yflip = attr & 0x40;

		gfx->transpen(bitmap, cliprect, number,
				0x20 + color, xflip, yflip,
				sx - 0x0f, sy, 0);
	}
}

/***************************************************************************

    Display Refresh

***************************************************************************/

uint32_t targeth_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	// set scroll registers
	m_pant[0]->set_scrolly(0, m_vregs[0]);
	m_pant[0]->set_scrollx(0, m_vregs[1] + 0x04);
	m_pant[1]->set_scrolly(0, m_vregs[2]);
	m_pant[1]->set_scrollx(0, m_vregs[3]);

	m_pant[1]->draw(screen, bitmap, cliprect, 0, 0);
	m_pant[0]->draw(screen, bitmap, cliprect, 0, 0);
	draw_sprites(bitmap,cliprect);

	return 0;
}


static const gfx_layout tilelayout =
{
	16,16,                                                          // 16x16 tiles
	RGN_FRAC(1,4),                                                  // number of tiles
	4,                                                              // bitplanes
	{ RGN_FRAC(3,4), RGN_FRAC(2,4), RGN_FRAC(1,4), RGN_FRAC(0,4) }, // plane offsets
	{ STEP8(0,1), STEP8(16*8,1) },
	{ STEP16(0,8) },
	32*8
};

static GFXDECODE_START( gfx_targeth )
	GFXDECODE_ENTRY( "gfx", 0x000000, tilelayout, 0, 64 )
GFXDECODE_END

TIMER_CALLBACK_MEMBER(targeth_state::gun1_irq)
{
	// IRQ 4: Read 1P Gun
	m_maincpu->set_input_line(4, HOLD_LINE);
	m_gun_irq_timer[0]->adjust( m_screen->time_until_pos(128, 0 ) );
}

TIMER_CALLBACK_MEMBER(targeth_state::gun2_irq)
{
	// IRQ 6: Read 2P Gun
	m_maincpu->set_input_line(6, HOLD_LINE);
	m_gun_irq_timer[1]->adjust( m_screen->time_until_pos(160, 0 ) );
}

void targeth_state::oki_bankswitch_w(uint8_t data)
{
	m_okibank->set_entry(data & 0x0f);
}

void targeth_state::output_latch_w(offs_t offset, uint16_t data)
{
	m_outlatch->write_bit(offset >> 3, BIT(data, 0));
}

template <uint8_t Which>
void targeth_state::coin_counter_w(int state)
{
	machine().bookkeeping().coin_counter_w(Which, state);
}

void targeth_state::shareram_w(offs_t offset, uint8_t data)
{
	// why isn't there address map functionality for this?
	util::big_endian_cast<uint8_t>(m_shareram.target())[offset] = data;
}

uint8_t targeth_state::shareram_r(offs_t offset)
{
	// why isn't there address map functionality for this?
	return util::big_endian_cast<uint8_t const>(m_shareram.target())[offset];
}


void targeth_state::mcu_hostmem_map(address_map &map)
{
	map(0x8000, 0xffff).rw(FUNC(targeth_state::shareram_r), FUNC(targeth_state::shareram_w)); // confirmed that 0x8000 - 0xffff is a window into 68k shared RAM
}

void targeth_state::main_map(address_map &map)
{
	map(0x000000, 0x0fffff).rom();
	map(0x100000, 0x103fff).ram().w(FUNC(targeth_state::vram_w)).share(m_videoram);
	map(0x108000, 0x108001).portr("GUNX1");
	map(0x108002, 0x108003).portr("GUNY1");
	map(0x108004, 0x108005).portr("GUNX2");
	map(0x108006, 0x108007).portr("GUNY2");
	map(0x108000, 0x108007).writeonly().share(m_vregs);
	map(0x10800c, 0x10800d).nopw();                    /* CLR Video INT */
	map(0x200000, 0x2007ff).ram().w(m_palette, FUNC(palette_device::write16)).share("palette");
	map(0x440000, 0x440fff).ram().share(m_spriteram);
	map(0x700000, 0x700001).portr("DSW2");
	map(0x700002, 0x700003).portr("DSW1");
	map(0x700006, 0x700007).portr("SYSTEM");             // Coins, Start & Fire buttons
	map(0x700008, 0x700009).portr("SERVICE");            // Service & Guns Reload?
	map(0x70000a, 0x70000b).select(0x000070).w(FUNC(targeth_state::output_latch_w));
	map(0x70000d, 0x70000d).w(FUNC(targeth_state::oki_bankswitch_w));
	map(0x70000f, 0x70000f).rw("oki", FUNC(okim6295_device::read), FUNC(okim6295_device::write));
	map(0x700010, 0x700011).nopw();                        // ??? Guns reload related?
	map(0xfe0000, 0xfe7fff).ram();                                          // Work RAM
	map(0xfe8000, 0xfeffff).ram().share(m_shareram);                     // Work RAM (shared with D5002FP)
}


void targeth_state::oki_map(address_map &map)
{
	map(0x00000, 0x2ffff).rom();
	map(0x30000, 0x3ffff).bankr(m_okibank);
}

void targeth_state::machine_start()
{
	m_okibank->configure_entries(0, 16, memregion("oki")->base(), 0x10000);

	m_gun_irq_timer[0] = timer_alloc(FUNC(targeth_state::gun1_irq), this);
	m_gun_irq_timer[1] = timer_alloc(FUNC(targeth_state::gun2_irq), this);

	m_gun_irq_timer[0]->adjust(m_screen->time_until_pos(128, 0));
	m_gun_irq_timer[1]->adjust(m_screen->time_until_pos(160, 0));
}

static INPUT_PORTS_START( targeth )
	PORT_START("GUNX1")
	PORT_BIT( 0x01ff, 200, IPT_LIGHTGUN_X ) PORT_CROSSHAIR(X, 1.20, -0.133, 0) PORT_MINMAX( 0, 400 + 4) PORT_SENSITIVITY(100) PORT_KEYDELTA(20) PORT_PLAYER(1)
	PORT_BIT( 0xfe00, IP_ACTIVE_HIGH, IPT_UNKNOWN )

	PORT_START("GUNY1")
	PORT_BIT( 0x01ff, 128, IPT_LIGHTGUN_Y ) PORT_CROSSHAIR(Y, 1.12, -0.055, 0) PORT_MINMAX(4,255) PORT_SENSITIVITY(100) PORT_KEYDELTA(20) PORT_PLAYER(1)
	PORT_BIT( 0xfe00, IP_ACTIVE_HIGH, IPT_UNKNOWN )

	PORT_START("GUNX2")
	PORT_BIT( 0x01ff, 200, IPT_LIGHTGUN_X ) PORT_CROSSHAIR(X, 1.20, -0.133, 0) PORT_MINMAX( 0, 400 + 4) PORT_SENSITIVITY(100) PORT_KEYDELTA(20) PORT_PLAYER(2)
	PORT_BIT( 0xfe00, IP_ACTIVE_HIGH, IPT_UNKNOWN )

	PORT_START("GUNY2")
	PORT_BIT( 0x01ff, 128, IPT_LIGHTGUN_Y ) PORT_CROSSHAIR(Y, 1.12, -0.055, 0) PORT_MINMAX(4,255) PORT_SENSITIVITY(100) PORT_KEYDELTA(20) PORT_PLAYER(2)
	PORT_BIT( 0xfe00, IP_ACTIVE_HIGH, IPT_UNKNOWN )

	PORT_START("DSW1")
	PORT_DIPNAME( 0x07, 0x07, DEF_STR( Coin_A ) )
	PORT_DIPSETTING(    0x02, DEF_STR( 6C_1C ) )
	PORT_DIPSETTING(    0x03, DEF_STR( 5C_1C ) )
	PORT_DIPSETTING(    0x04, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(    0x05, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x06, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x01, DEF_STR( 3C_2C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 4C_3C ) )
	PORT_DIPSETTING(    0x07, DEF_STR( 1C_1C ) )
	PORT_DIPNAME( 0x38, 0x38, DEF_STR( Coin_B ) )
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

	PORT_START("DSW2")
	PORT_DIPNAME( 0x03, 0x03, DEF_STR( Difficulty ) )
	PORT_DIPSETTING(    0x01, DEF_STR( Easy ) )
	PORT_DIPSETTING(    0x03, DEF_STR( Normal ) )
	PORT_DIPSETTING(    0x02, DEF_STR( Hard ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Hardest ) )
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Demo_Sounds ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x20, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, "Gun alarm" ) // Service mode gets default from here when uninitialized
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_SERVICE( 0x80, IP_ACTIVE_LOW )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("SYSTEM")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_START2 )

	PORT_START("SERVICE")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )    // this MUST be low or the game doesn't boot
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(1) // Reload 1P?
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2) // Reload 2P?
	PORT_BIT( 0xf0, IP_ACTIVE_LOW, IPT_UNKNOWN )
INPUT_PORTS_END


void targeth_state::targeth(machine_config &config)
{
	// basic machine hardware
	M68000(config, m_maincpu, XTAL(24'000'000) / 2);          // 12 MHz
	m_maincpu->set_addrmap(AS_PROGRAM, &targeth_state::main_map);
	m_maincpu->set_vblank_int("screen", FUNC(targeth_state::irq2_line_hold));

	gaelco_ds5002fp_device &ds5002fp(GAELCO_DS5002FP(config, "gaelco_ds5002fp", XTAL(24'000'000) / 2));
	ds5002fp.set_addrmap(0, &targeth_state::mcu_hostmem_map);
	config.set_perfect_quantum("gaelco_ds5002fp:mcu");

	LS259(config, m_outlatch);
	m_outlatch->q_out_cb<2>().set(FUNC(targeth_state::coin_counter_w<0>));
	m_outlatch->q_out_cb<3>().set(FUNC(targeth_state::coin_counter_w<1>));

	// video hardware
	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	m_screen->set_refresh_hz(60);
	m_screen->set_vblank_time(ATTOSECONDS_IN_USEC(2500)); // not accurate
	m_screen->set_size(64*16, 16*16);
	m_screen->set_visarea(3*8, 23*16-8-1, 16, 16*16-8-1);
	m_screen->set_screen_update(FUNC(targeth_state::screen_update));
	m_screen->set_palette(m_palette);

	GFXDECODE(config, "gfxdecode", m_palette, gfx_targeth);
	PALETTE(config, m_palette).set_format(palette_device::xBGR_555, 1024);

	// sound hardware
	SPEAKER(config, "mono").front_center();

	okim6295_device &oki(OKIM6295(config, "oki", XTAL(1'000'000), okim6295_device::PIN7_HIGH)); // 1MHz resonator - pin 7 not verified
	oki.set_addrmap(0, &targeth_state::oki_map);
	oki.add_route(ALL_OUTPUTS, "mono", 1.0);
}

ROM_START( targeth )
	ROM_REGION( 0x100000, "maincpu", 0 )    // 68000 code
	ROM_LOAD16_BYTE( "th2_b_c_23.c23", 0x000000, 0x040000, CRC(840887d6) SHA1(9a36b346608d531a62a2e0704ea44f12e07f9d91) ) // The "B" was hand written
	ROM_LOAD16_BYTE( "th2_b_c_22.c22", 0x000001, 0x040000, CRC(d2435eb8) SHA1(ce75a115dad8019c8e66a1c3b3e15f54781f65ae) ) // The "B" was hand written

	ROM_REGION( 0x8000, "gaelco_ds5002fp:sram", 0 ) // DS5002FP code
	ROM_LOAD( "targeth_ds5002fp.bin", 0x00000, 0x8000, CRC(abcdfee4) SHA1(c5955d5dbbcecbe1c2ae77d59671ae40eb814d30) )

	ROM_REGION( 0x100, "gaelco_ds5002fp:mcu:internal", ROMREGION_ERASE00 )
	ROM_LOAD( "targeth_ds5002fp_scratch", 0x00, 0x80, CRC(c927bcb1) SHA1(86b5c7ee6a4a5f0aa538a6742253da1afadb4345) ) // default state so you don't have to manually initialize game
	DS5002FP_SET_MON( 0x49 )
	DS5002FP_SET_RPCTL( 0x00 )
	DS5002FP_SET_CRCR( 0x80 )

	ROM_REGION( 0x200000, "gfx", 0 )
	ROM_LOAD( "targeth.i13",    0x000000, 0x080000, CRC(b892be24) SHA1(9cccaaacf20e77c7358f0ceac60b8a1012f1216c) )
	ROM_LOAD( "targeth.i11",    0x080000, 0x080000, CRC(6797faf9) SHA1(112cffe72f91cb46c262e19a47b0cab3237dd60f) )
	ROM_LOAD( "targeth.i9",     0x100000, 0x080000, CRC(0e922c1c) SHA1(6920e345c82e76f7e0af6101f39eb65ac1f112b9) )
	ROM_LOAD( "targeth.i7",     0x180000, 0x080000, CRC(d8b41000) SHA1(cbe91eb91bdc7a60b2333c6bea37d08a57902669) )

	ROM_REGION( 0x100000, "oki", 0 )
	ROM_LOAD( "targeth.c1",     0x000000, 0x080000, CRC(d6c9dfbc) SHA1(3ec70dea94fc89df933074012a52de6034571e87) )
	// 0x00000-0x2ffff is fixed, 0x30000-0x3ffff is bank switched from all the ROMs
	ROM_LOAD( "targeth.c3",     0x080000, 0x080000, CRC(d4c771df) SHA1(7cc0a86ef6aa3d26ab8f19d198f62112bf012870) )
ROM_END

ROM_START( targetha )
	ROM_REGION( 0x100000, "maincpu", 0 )    /* 68000 code */
	ROM_LOAD16_BYTE( "th2_n_c_23.c23", 0x000000, 0x040000, CRC(b99b25dc) SHA1(1bf35b2c05a58f934d06eb6ef93f592d9f16344a) ) // The "N" was hand written
	ROM_LOAD16_BYTE( "th2_n_c_22.c22", 0x000001, 0x040000, CRC(6d34f0cf) SHA1(f44a1231f4fac1f9d443990e8fe2b4aaa3f338be) ) // The "N" was hand written

	ROM_REGION( 0x8000, "gaelco_ds5002fp:sram", 0 ) /* DS5002FP code */
	ROM_LOAD( "targeth_ds5002fp.bin", 0x00000, 0x8000, CRC(abcdfee4) SHA1(c5955d5dbbcecbe1c2ae77d59671ae40eb814d30) )

	ROM_REGION( 0x100, "gaelco_ds5002fp:mcu:internal", ROMREGION_ERASE00 )
	ROM_LOAD( "targeth_ds5002fp_scratch", 0x00, 0x80, CRC(c927bcb1) SHA1(86b5c7ee6a4a5f0aa538a6742253da1afadb4345) ) // default state so you don't have to manually initialize game
	DS5002FP_SET_MON( 0x49 )
	DS5002FP_SET_RPCTL( 0x00 )
	DS5002FP_SET_CRCR( 0x80 )

	ROM_REGION( 0x200000, "gfx", 0 )
	ROM_LOAD( "targeth.i13",    0x000000, 0x080000, CRC(b892be24) SHA1(9cccaaacf20e77c7358f0ceac60b8a1012f1216c) )
	ROM_LOAD( "targeth.i11",    0x080000, 0x080000, CRC(6797faf9) SHA1(112cffe72f91cb46c262e19a47b0cab3237dd60f) )
	ROM_LOAD( "targeth.i9",     0x100000, 0x080000, CRC(0e922c1c) SHA1(6920e345c82e76f7e0af6101f39eb65ac1f112b9) )
	ROM_LOAD( "targeth.i7",     0x180000, 0x080000, CRC(d8b41000) SHA1(cbe91eb91bdc7a60b2333c6bea37d08a57902669) )

	ROM_REGION( 0x100000, "oki", 0 )
	ROM_LOAD( "targeth.c1",     0x000000, 0x080000, CRC(d6c9dfbc) SHA1(3ec70dea94fc89df933074012a52de6034571e87) )
	// 0x00000-0x2ffff is fixed, 0x30000-0x3ffff is bank switched from all the ROMs
	ROM_LOAD( "targeth.c3",     0x080000, 0x080000, CRC(d4c771df) SHA1(7cc0a86ef6aa3d26ab8f19d198f62112bf012870) )
ROM_END

ROM_START( targeth10 )
	ROM_REGION( 0x100000, "maincpu", 0 )    // 68000 code
	ROM_LOAD16_BYTE( "c23.bin", 0x000000, 0x040000, CRC(e38a54e2) SHA1(239bfa6f1c0fc8aa0ad7de9be237bef55b384007) )
	ROM_LOAD16_BYTE( "c22.bin", 0x000001, 0x040000, CRC(24fe3efb) SHA1(8f48f08a6db28966c9263be119883c9179e349ed) )

	ROM_REGION( 0x8000, "gaelco_ds5002fp:sram", 0 ) // DS5002FP code
	ROM_LOAD( "targeth_ds5002fp.bin", 0x00000, 0x8000, CRC(abcdfee4) SHA1(c5955d5dbbcecbe1c2ae77d59671ae40eb814d30) )

	ROM_REGION( 0x100, "gaelco_ds5002fp:mcu:internal", ROMREGION_ERASE00 )
	ROM_LOAD( "targeth_ds5002fp_scratch", 0x00, 0x80, CRC(c927bcb1) SHA1(86b5c7ee6a4a5f0aa538a6742253da1afadb4345) ) // default state so you don't have to manually initialize game
	DS5002FP_SET_MON( 0x49 )
	DS5002FP_SET_RPCTL( 0x00 )
	DS5002FP_SET_CRCR( 0x80 )

	ROM_REGION( 0x200000, "gfx", 0 )
	ROM_LOAD( "targeth.i13",    0x000000, 0x080000, CRC(b892be24) SHA1(9cccaaacf20e77c7358f0ceac60b8a1012f1216c) )
	ROM_LOAD( "targeth.i11",    0x080000, 0x080000, CRC(6797faf9) SHA1(112cffe72f91cb46c262e19a47b0cab3237dd60f) )
	ROM_LOAD( "targeth.i9",     0x100000, 0x080000, CRC(0e922c1c) SHA1(6920e345c82e76f7e0af6101f39eb65ac1f112b9) )
	ROM_LOAD( "targeth.i7",     0x180000, 0x080000, CRC(d8b41000) SHA1(cbe91eb91bdc7a60b2333c6bea37d08a57902669) )

	ROM_REGION( 0x100000, "oki", 0 )
	ROM_LOAD( "targeth.c1",     0x000000, 0x080000, CRC(d6c9dfbc) SHA1(3ec70dea94fc89df933074012a52de6034571e87) )
	// 0x00000-0x2ffff is fixed, 0x30000-0x3ffff is bank switched from all the ROMs
	ROM_LOAD( "targeth.c3",     0x080000, 0x080000, CRC(d4c771df) SHA1(7cc0a86ef6aa3d26ab8f19d198f62112bf012870) )
ROM_END

} // anonymous namespace


GAME( 1994, targeth,   0,       targeth, targeth, targeth_state, empty_init, ROT0, "Gaelco", "Target Hits (ver 1.1, Checksum 5152)", MACHINE_SUPPORTS_SAVE )
GAME( 1994, targetha,  targeth, targeth, targeth, targeth_state, empty_init, ROT0, "Gaelco", "Target Hits (ver 1.1, Checksum 86E1)", MACHINE_SUPPORTS_SAVE )
GAME( 1994, targeth10, targeth, targeth, targeth, targeth_state, empty_init, ROT0, "Gaelco", "Target Hits (ver 1.0, Checksum FBCB)", MACHINE_SUPPORTS_SAVE )
