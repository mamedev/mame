// license:BSD-3-Clause
// copyright-holders:David Haywood, Pierpaolo Prazzoli
/*

 One + Two
 (c) Barko 1997

 Driver by David Haywood and Pierpaolo Prazzoli

PCB has D.G.R.M. silkscreened on it.  Dated 1997.4.11

+----------------------------------+
|  YM3014 YM3812  Z80   24MHz      |
|   M6295 sample  sound_prog       |
|                 6116       3_grfx|
|J       6116                      |
|A COR_B 6116                4_grfx|
|M COR_G                           |
|M COR_R       A1020B              |
|A                           5_grfx|
|DSW   62256         6264          |
|     main_prog                    |
|DSW   Z80  4MHz                   |
+----------------------------------+

Goldstar Z8400A PS (4 MHz rated) both CPUs
Actel A1020B PL84C
YM3812/YM3014 (badged as UA011 & UA010)
OKI M6295

 main_prog 27c010
    x_grfx 27c040
    sample 27c020
sound_prog 27512

COR_x are LN60G resistor packs

-------------------------------------

Note: this is quite clearly a 'Korean bootleg' of Shisensho - Joshiryo-Hen / Match-It

*/

#include "emu.h"
#include "cpu/z80/z80.h"
#include "machine/gen_latch.h"
#include "machine/watchdog.h"
#include "sound/okim6295.h"
#include "sound/ymopl.h"
#include "emupal.h"
#include "screen.h"
#include "speaker.h"
#include "tilemap.h"


namespace {

class onetwo_state : public driver_device
{
public:
	onetwo_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_fgram(*this, "fgram"),
		m_mainbank(*this, "mainbank"),
		m_maincpu(*this, "maincpu"),
		m_audiocpu(*this, "audiocpu"),
		m_watchdog(*this, "watchdog"),
		m_gfxdecode(*this, "gfxdecode"),
		m_palette(*this, "palette"),
		m_soundlatch(*this, "soundlatch")
	{ }

	void onetwo(machine_config &config);

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void video_start() override ATTR_COLD;

private:
	// memory pointers
	required_shared_ptr<uint8_t> m_fgram;
	required_memory_bank m_mainbank;

	// video-related
	tilemap_t *m_fg_tilemap;

	// devices
	required_device<cpu_device> m_maincpu;
	required_device<cpu_device> m_audiocpu;
	required_device<watchdog_timer_device> m_watchdog;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;
	required_device<generic_latch_8_device> m_soundlatch;

	void fgram_w(offs_t offset, uint8_t data);
	void cpubank_w(uint8_t data);
	void coin_counters_w(uint8_t data);
	TILE_GET_INFO_MEMBER(get_fg_tile_info);
	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	static rgb_t BBBGGGGGxBBRRRRR(uint32_t raw);
	void main_cpu(address_map &map) ATTR_COLD;
	void main_cpu_io(address_map &map) ATTR_COLD;
	void sound_cpu(address_map &map) ATTR_COLD;
	void sound_cpu_io(address_map &map) ATTR_COLD;
};



/*************************************
 *
 *  Video emulation
 *
 *************************************/

TILE_GET_INFO_MEMBER(onetwo_state::get_fg_tile_info)
{
	int code = (m_fgram[tile_index * 2 + 1] << 8) | m_fgram[tile_index * 2];
	int color = (m_fgram[tile_index * 2 + 1] & 0x80) >> 7;

	code &= 0x7fff;

	tileinfo.set(0, code, color, 0);
}

void onetwo_state::video_start()
{
	m_fg_tilemap = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(onetwo_state::get_fg_tile_info)), TILEMAP_SCAN_ROWS, 8, 8, 64, 32);
}

uint32_t onetwo_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	m_fg_tilemap->draw(screen, bitmap, cliprect, 0, 0);
	return 0;
}

/*************************************
 *
 *  Memory handlers
 *
 *************************************/

void onetwo_state::fgram_w(offs_t offset, uint8_t data)
{
	m_fgram[offset] = data;
	m_fg_tilemap->mark_tile_dirty(offset / 2);
}

void onetwo_state::cpubank_w(uint8_t data)
{
	m_mainbank->set_entry(data);
}

void onetwo_state::coin_counters_w(uint8_t data)
{
	m_watchdog->watchdog_reset();
	machine().bookkeeping().coin_counter_w(0, BIT(data, 1));
	machine().bookkeeping().coin_counter_w(1, BIT(data, 2));
}

rgb_t onetwo_state::BBBGGGGGxBBRRRRR(uint32_t raw)
{
	uint8_t const r = pal5bit((raw >> 0) & 0x1f);
	uint8_t const g = pal5bit((raw >> 8) & 0x1f);
	uint8_t const b = pal5bit(((raw >> 2) & 0x18) | ((raw >> 13) & 0x7));
	return rgb_t(r, g, b);
}

/*************************************
 *
 *  Address maps
 *
 *************************************/

void onetwo_state::main_cpu(address_map &map)
{
	map(0x0000, 0x7fff).rom().region("maincpu", 0);
	map(0x8000, 0xbfff).bankr(m_mainbank);
	map(0xc800, 0xc87f).ram().w(m_palette, FUNC(palette_device::write8)).share("palette");
	map(0xc900, 0xc97f).ram().w(m_palette, FUNC(palette_device::write8_ext)).share("palette_ext");
	map(0xd000, 0xdfff).ram().w(FUNC(onetwo_state::fgram_w)).share(m_fgram);
	map(0xe000, 0xffff).ram();
}

void onetwo_state::main_cpu_io(address_map &map)
{
	map.global_mask(0xff);
	map(0x00, 0x00).portr("DSW1").w(FUNC(onetwo_state::coin_counters_w));
	map(0x01, 0x01).portr("DSW2").w(m_soundlatch, FUNC(generic_latch_8_device::write));
	map(0x02, 0x02).portr("P1").w(FUNC(onetwo_state::cpubank_w));
	map(0x03, 0x03).portr("P2");
	map(0x04, 0x04).portr("SYSTEM");
}

void onetwo_state::sound_cpu(address_map &map)
{
	map(0x0000, 0x5fff).rom();
	map(0xf000, 0xf7ff).ram();
	map(0xf800, 0xf800).r(m_soundlatch, FUNC(generic_latch_8_device::read));
}

void onetwo_state::sound_cpu_io(address_map &map)
{
	map.global_mask(0xff);
	map(0x00, 0x00).rw("ymsnd", FUNC(ym3812_device::status_r), FUNC(ym3812_device::address_w));
	map(0x20, 0x20).w("ymsnd", FUNC(ym3812_device::data_w));
	map(0x40, 0x40).rw("oki", FUNC(okim6295_device::read), FUNC(okim6295_device::write));
	map(0xc0, 0xc0).w(m_soundlatch, FUNC(generic_latch_8_device::acknowledge_w));
}

/*************************************
 *
 *  Input ports
 *
 *************************************/

static INPUT_PORTS_START( onetwo )
	PORT_START("DSW1")
	PORT_DIPNAME( 0x03, 0x03, "Timer" ) PORT_DIPLOCATION("SW1:1,2")
	PORT_DIPSETTING(    0x03, DEF_STR( Easy ) )
	PORT_DIPSETTING(    0x02, DEF_STR( Normal ) )
	PORT_DIPSETTING(    0x01, DEF_STR( Hard ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Hardest ) )
	PORT_DIPNAME( 0x0c, 0x0c, DEF_STR( Difficulty ) ) PORT_DIPLOCATION("SW1:3,4")
	PORT_DIPSETTING(    0x0c, DEF_STR( Easy ) )
	PORT_DIPSETTING(    0x08, DEF_STR( Normal ) )
	PORT_DIPSETTING(    0x04, DEF_STR( Hard ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Hardest ) )
	PORT_DIPNAME( 0xf0, 0xf0, DEF_STR( Coinage ) ) PORT_CONDITION("DSW2",0x04,EQUALS,0x04) PORT_DIPLOCATION("SW1:5,6,7,8")
	PORT_DIPSETTING(    0xa0, DEF_STR( 6C_1C ) )
	PORT_DIPSETTING(    0xb0, DEF_STR( 5C_1C ) )
	PORT_DIPSETTING(    0xc0, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(    0xd0, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x10, DEF_STR( 8C_3C ) )
	PORT_DIPSETTING(    0xe0, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x20, DEF_STR( 5C_3C ) )
	PORT_DIPSETTING(    0x30, DEF_STR( 3C_2C ) )
	PORT_DIPSETTING(    0xf0, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x40, DEF_STR( 2C_3C ) )
	PORT_DIPSETTING(    0x90, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x80, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0x70, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(    0x60, DEF_STR( 1C_5C ) )
	PORT_DIPSETTING(    0x50, DEF_STR( 1C_6C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Free_Play ) )
	PORT_DIPNAME( 0x30, 0x30, DEF_STR( Coin_A ) ) PORT_CONDITION("DSW2",0x04,NOTEQUALS,0x04) PORT_DIPLOCATION("SW1:5,6")
	PORT_DIPSETTING(    0x00, DEF_STR( 5C_1C ) )
	PORT_DIPSETTING(    0x10, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x20, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x30, DEF_STR( 1C_1C ) )
	PORT_DIPNAME( 0xc0, 0xc0, DEF_STR( Coin_B ) ) PORT_CONDITION("DSW2",0x04,NOTEQUALS,0x04) PORT_DIPLOCATION("SW1:7,8")
	PORT_DIPSETTING(    0xc0, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x80, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0x40, DEF_STR( 1C_5C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_6C ) )

	PORT_START("DSW2")
	PORT_DIPNAME( 0x01, 0x01, DEF_STR( Unknown) ) PORT_DIPLOCATION("SW2:1") // Flip Screen?
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x00, DEF_STR( Demo_Sounds ) ) PORT_DIPLOCATION("SW2:2")
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x00, "Coin Chute" ) PORT_DIPLOCATION("SW2:3")
	PORT_DIPSETTING(    0x04, "Common" )
	PORT_DIPSETTING(    0x00, "Separate" )
	PORT_DIPNAME( 0x08, 0x08, "Nude Pictures" ) PORT_DIPLOCATION("SW2:4")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x08, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, "Women Select" ) PORT_DIPLOCATION("SW2:5")
	PORT_DIPSETTING(    0x00, DEF_STR( No ) )
	PORT_DIPSETTING(    0x10, DEF_STR( Yes ) )
	// In stop mode, press 2 to stop and 1 to restart
	PORT_DIPNAME( 0x20, 0x20, "Stop Mode (Cheat)") PORT_DIPLOCATION("SW2:6")
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, "Play Mode" ) PORT_DIPLOCATION("SW2:7")
	PORT_DIPSETTING(    0x00, "1 Player" )
	PORT_DIPSETTING(    0x40, "2 Player" )
	PORT_DIPNAME( 0x80, 0x00, "Freeze" ) PORT_DIPLOCATION("SW2:8")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x80, DEF_STR( On ) )

	PORT_START("P1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_PLAYER(1)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_PLAYER(1)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_PLAYER(1)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(1)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(1)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(1)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("P2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_PLAYER(2)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_PLAYER(2)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_PLAYER(2)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(2)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(2)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("SYSTEM")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )
INPUT_PORTS_END

/*************************************
 *
 *  Graphics definitions
 *
 *************************************/

static const gfx_layout tiles8x8x6_layout =
{
	8,8,
	RGN_FRAC(1,3),
	6,
	{ RGN_FRAC(2,3)+0, RGN_FRAC(2,3)+4, RGN_FRAC(0,3)+0, RGN_FRAC(0,3)+4, RGN_FRAC(1,3)+0, RGN_FRAC(1,3)+4 },
	{ STEP4(0,1), STEP4(4*2*8,1) },
	{ STEP8(0,4*2) },
	16*8
};

static GFXDECODE_START( gfx_onetwo )
	GFXDECODE_ENTRY( "gfx1", 0, tiles8x8x6_layout, 0, 2 )
GFXDECODE_END


/*************************************
 *
 *  Machine driver
 *
 *************************************/

void onetwo_state::machine_start()
{
	m_mainbank->configure_entries(0, 8, memregion("maincpu")->base(), 0x4000);
}

void onetwo_state::onetwo(machine_config &config)
{
	// basic machine hardware
	Z80(config, m_maincpu, 4_MHz_XTAL);
	m_maincpu->set_addrmap(AS_PROGRAM, &onetwo_state::main_cpu);
	m_maincpu->set_addrmap(AS_IO, &onetwo_state::main_cpu_io);
	m_maincpu->set_vblank_int("screen", FUNC(onetwo_state::irq0_line_hold));

	Z80(config, m_audiocpu, 4_MHz_XTAL);
	m_audiocpu->set_addrmap(AS_PROGRAM, &onetwo_state::sound_cpu);
	m_audiocpu->set_addrmap(AS_IO, &onetwo_state::sound_cpu_io);

	WATCHDOG_TIMER(config, m_watchdog);

	// video hardware
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_refresh_hz(60);
	screen.set_vblank_time(ATTOSECONDS_IN_USEC(16));
	screen.set_size(512, 256);
	screen.set_visarea(0, 512-1, 0, 256-1);
	screen.set_screen_update(FUNC(onetwo_state::screen_update));
	screen.set_palette(m_palette);

	GFXDECODE(config, m_gfxdecode, m_palette, gfx_onetwo);
	PALETTE(config, m_palette).set_format(2, &onetwo_state::BBBGGGGGxBBRRRRR, 0x80);

	// sound hardware
	SPEAKER(config, "mono").front_center();

	GENERIC_LATCH_8(config, m_soundlatch);
	m_soundlatch->data_pending_callback().set_inputline(m_audiocpu, INPUT_LINE_NMI);
	m_soundlatch->set_separate_acknowledge(true);

	ym3812_device &ymsnd(YM3812(config, "ymsnd", 4_MHz_XTAL));
	ymsnd.irq_handler().set_inputline(m_audiocpu, 0);
	ymsnd.add_route(ALL_OUTPUTS, "mono", 1.0);

	okim6295_device &oki(OKIM6295(config, "oki", 4_MHz_XTAL / 2, okim6295_device::PIN7_LOW)); // clock frequency & pin 7 not verified, no resonator on PCB so probably derived from the 4 MHz XTAL
	oki.add_route(ALL_OUTPUTS, "mono", 1.0);
}

/*************************************
 *
 *  ROM definition(s)
 *
 *************************************/

ROM_START( onetwo )
	ROM_REGION( 0x20000, "maincpu", 0 )
	ROM_LOAD( "maincpu", 0x00000,  0x20000, CRC(83431e6e) SHA1(61ab386a1d0af050f091f5df28c55ad5ad1a0d4b) )

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "sound_prog",  0x00000,  0x10000, CRC(90aba4f3) SHA1(914b1c8684993ddc7200a3d61e07f4f6d59e9d02) )

	ROM_REGION( 0x180000, "gfx1", 0 )
	ROM_LOAD( "3_graphics", 0x000000, 0x80000, CRC(c72ff3a0) SHA1(17394d8a8b5ef4aee9522d87ba92ef1285f4d76a) )
	ROM_LOAD( "4_graphics", 0x080000, 0x80000, CRC(0ca40557) SHA1(ca2db57d64ece90f2066f15b276c8d5827dcb4fa) )
	ROM_LOAD( "5_graphics", 0x100000, 0x80000, CRC(664b6679) SHA1(f9f78bd34fb58e24f890a540382392e1c9d01220) )

	ROM_REGION( 0x40000, "oki", 0 )
	ROM_LOAD( "sample", 0x000000, 0x40000, CRC(b10d3132) SHA1(42613e17b6a1300063b8355596a2dc7bcd903777) )
ROM_END

ROM_START( onetwoe )
	ROM_REGION( 0x20000, "maincpu", 0 )
	ROM_LOAD( "main_prog", 0x00000,  0x20000, CRC(6c1936e9) SHA1(d8fb3056299c9b45e0b537e77dc0d633882705dd) )

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "sound_prog",  0x00000,  0x10000, CRC(90aba4f3) SHA1(914b1c8684993ddc7200a3d61e07f4f6d59e9d02) )

	ROM_REGION( 0x180000, "gfx1", 0 )
	ROM_LOAD( "3_grfx", 0x000000, 0x80000, CRC(0f9f39ff) SHA1(85d107306c8c5718da3b751221791404cfe12a3d) )
	ROM_LOAD( "4_grfx", 0x080000, 0x80000, CRC(2b0e0564) SHA1(092bf0bb7be12ed1aa8a4ed1e88143ea88819497) )
	ROM_LOAD( "5_grfx", 0x100000, 0x80000, CRC(69807a9b) SHA1(6c1d79e86e3575da29bc299670e38019eef53493) )

	ROM_REGION( 0x40000, "oki", 0 )
	ROM_LOAD( "sample", 0x000000, 0x40000, CRC(b10d3132) SHA1(42613e17b6a1300063b8355596a2dc7bcd903777) )
ROM_END

} // Anonymous namespace


/*************************************
 *
 *  Game driver(s)
 *
 *************************************/

GAME( 1997, onetwo,       0, onetwo, onetwo, onetwo_state, empty_init, ROT0, "Barko", "One + Two", MACHINE_SUPPORTS_SAVE )
GAME( 1997, onetwoe, onetwo, onetwo, onetwo, onetwo_state, empty_init, ROT0, "Barko", "One + Two (earlier)", MACHINE_SUPPORTS_SAVE )
