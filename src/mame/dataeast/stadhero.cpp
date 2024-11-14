// license:BSD-3-Clause
// copyright-holders: Bryan McPhail

/***************************************************************************

    Stadium Hero (Japan)            (c) 1988 Data East Corporation

    Emulation by Bryan McPhail, mish@tendril.co.uk

    PCB Layout and Hardware Info by Guru


    PCB Layouts
    -----------

    DE-0303-3    |-----------|     |------------|
    |------------|-----------|-----|------------|-------------|
    |                                                         |
    |                    20MHz                           6264 |
    |          PV-2A                                          |
    |          (PAL)                                     X    |
    |                                 |----|                  |
    |    SW2                          |    |                  |
    |                                 | 6  |             X    |
    |J                                | 8  |                  |
    |A                                | 0  |                  |
    |M      EF18.7F   65C02           | 0  |           EF15.9A|
    |M                                | 0  |                  |
    |A      5814                      |    |                  |
    |                                 |----|             X    |
    |    SW1                                                  |
    |                                  PV-1                   |
    |       YM2203   YM3812           (PAL)  PV-0        X    |
    |                                        (PAL)            |
    | YM3014  YM3014 M6295                             EF13.4A|
    | UPC3403  UPC3403  1.056MHz                              |
    |                                                    6264 |
    | MB3730         EF17.1E                                  |
    |---------------------------------------------------------|
    Notes:
      68000  - Clock 10.000MHz [20/2]
      65C02  - Clock 1.500MHz [24/16]
      YM3812 - Clock 3.000MHz [24/8]
      YM2203 - Clock 1.500MHz [24/16]
      M6295  - Clock 1.056MHz (via resonator), pin 7 HIGH
      YM3014 - Yamaha YM3014B Serial Input Floating D/A Converter
      6264   - 8kx8 SRAM
      5814   - 2kx8 SRAM
      X      - unpopulated DIP28 socket
      SW1/2  - 8-position DIP Switch
      MB3730 - Fujitsu MB3730 Audio Power AMP
      uPC3403- NEC uPC3403 High Performance Quad Operational Amplifier
      HSync  - 15.6246kHz
      VSync  - 57.4434Hz


    DE-0304-3    |-----------|     |------------|
    |------------|-----------|-----|------------|-------------|
    |EF12.14J                                                 |
    |                                      HM3-65728          |
    |EF11.13J                                                 |
    |             |--------|               HM3-65728  EF07.12A|
    |EF10.11J     |DATAEAST|          PV-3                    |
    |             |L7B0072 |         (PAL)            EF06.11A|
    | 2063        |BAC 06  |                                  |
    |             |--------|                          EF05.9A |
    | 2063                          5814                      |
    |           5814                                  EF04.8A |
    |                                                         |
    |                               5814                      |
    |           5814                       |--------| EF03.6A |
    |                                      |DATAEAST|         |
    |                                      |L7B0073 | EF02.5A |
    |                                      |MXC 06  |         |
    |                  |--------|          |--------| EF01.4A |
    |EF09.4J           |DATAEAST|                             |
    |                  |TC17G042|    EF-19.3D         EF00.2A |
    |EF08.2J           |        |                             |
    |                  |--------|                      24MHz  |
    |---------------------------------------------------------|
    Notes:
      HM3-65728 - 2kx8 SRAM
      2063      - 8kx8 SRAM
      5814      - 2kx8 SRAM

    TODO : RNG issue? Some behavior isn't correct (ex: BGM randomizer).
    reference: https://youtu.be/6azneK6uUnA



***************************************************************************/

#include "emu.h"

#include "decbac06.h"
#include "decmxc06.h"

#include "cpu/m6502/m6502.h"
#include "cpu/m68000/m68000.h"
#include "machine/gen_latch.h"
#include "sound/okim6295.h"
#include "sound/ymopl.h"
#include "sound/ymopn.h"

#include "emupal.h"
#include "screen.h"
#include "speaker.h"
#include "tilemap.h"


namespace {

class stadhero_state : public driver_device
{
public:
	stadhero_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_audiocpu(*this, "audiocpu"),
		m_tilegen(*this, "tilegen"),
		m_spritegen(*this, "spritegen"),
		m_gfxdecode(*this, "gfxdecode"),
		m_soundlatch(*this, "soundlatch"),
		m_screen(*this, "screen"),
		m_spriteram(*this, "spriteram"),
		m_pf1_data(*this, "pf1_data"),
		m_coin(*this, "COIN")
	{
	}

	void stadhero(machine_config &config);

protected:
	virtual void video_start() override ATTR_COLD;

private:
	required_device<cpu_device> m_maincpu;
	required_device<cpu_device> m_audiocpu;
	required_device<deco_bac06_device> m_tilegen;
	required_device<deco_mxc06_device> m_spritegen;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<generic_latch_8_device> m_soundlatch;
	required_device<screen_device> m_screen;

	required_shared_ptr<uint16_t> m_spriteram;
	required_shared_ptr<uint16_t> m_pf1_data;

	required_ioport m_coin;

	tilemap_t *m_pf1_tilemap = nullptr;

	void int_ack_w(uint16_t data);
	void pf1_data_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	uint8_t mystery_r();

	TILE_GET_INFO_MEMBER(get_pf1_tile_info);
	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void audio_map(address_map &map) ATTR_COLD;
	void main_map(address_map &map) ATTR_COLD;
};


/******************************************************************************/

uint32_t stadhero_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	bool const flip = m_tilegen->get_flip_state();
	m_tilegen->set_flip_screen(flip);
	m_spritegen->set_flip_screen(flip);
	m_pf1_tilemap->set_flip(flip ? (TILEMAP_FLIPY | TILEMAP_FLIPX) : 0);

	m_tilegen->deco_bac06_pf_draw(screen, bitmap, cliprect, TILEMAP_DRAW_OPAQUE, 0);
	m_spritegen->draw_sprites(screen, bitmap, cliprect, m_spriteram, 0x800 / 2);
	m_pf1_tilemap->draw(screen, bitmap, cliprect, 0, 0);
	return 0;
}

/******************************************************************************/

void stadhero_state::pf1_data_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	COMBINE_DATA(&m_pf1_data[offset]);
	m_pf1_tilemap->mark_tile_dirty(offset);
}


/******************************************************************************/

TILE_GET_INFO_MEMBER(stadhero_state::get_pf1_tile_info)
{
	int tile = m_pf1_data[tile_index];
	int const color = tile >> 12;

	tile = tile & 0xfff;
	tileinfo.set(0,
			tile,
			color,
			0);
}

void stadhero_state::video_start()
{
	m_pf1_tilemap = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(stadhero_state::get_pf1_tile_info)), TILEMAP_SCAN_ROWS, 8, 8, 32, 32);
	m_pf1_tilemap->set_transparent_pen(0);
}

/******************************************************************************/

/******************************************************************************/

void stadhero_state::int_ack_w(uint16_t data)
{
	m_maincpu->set_input_line(M68K_IRQ_5, CLEAR_LINE);
}

uint8_t stadhero_state::mystery_r()
{
	// Polled, very frequently at times, to randomly determine stage music
	// selection, attract mode teams, base-stealing attempts, etc. etc.
	return m_screen->hpos() / 2;
}


/******************************************************************************/

void stadhero_state::main_map(address_map &map)
{
	map(0x000000, 0x01ffff).rom();
	map(0x200000, 0x2007ff).ram().w(FUNC(stadhero_state::pf1_data_w)).share(m_pf1_data);
	map(0x240000, 0x240007).w(m_tilegen, FUNC(deco_bac06_device::pf_control_0_w)); // text layer
	map(0x240010, 0x240017).w(m_tilegen, FUNC(deco_bac06_device::pf_control_1_w));
	map(0x260000, 0x261fff).rw(m_tilegen, FUNC(deco_bac06_device::pf_data_r), FUNC(deco_bac06_device::pf_data_w));
	map(0x30c000, 0x30c001).portr("INPUTS");
	map(0x30c002, 0x30c002).lr8(NAME([this] () { return uint8_t(m_coin->read()); }));
	map(0x30c003, 0x30c003).r(FUNC(stadhero_state::mystery_r));
	map(0x30c004, 0x30c005).portr("DSW").w(FUNC(stadhero_state::int_ack_w));
	map(0x30c007, 0x30c007).w(m_soundlatch, FUNC(generic_latch_8_device::write));
	map(0x310000, 0x3107ff).ram().w("palette", FUNC(palette_device::write16)).share("palette");
	map(0xff8000, 0xffbfff).ram(); // Main RAM
	map(0xffc000, 0xffc7ff).mirror(0x000800).ram().share(m_spriteram);
}

/******************************************************************************/

void stadhero_state::audio_map(address_map &map)
{
	map(0x0000, 0x05ff).ram();
	map(0x0800, 0x0801).w("ym1", FUNC(ym2203_device::write));
	map(0x1000, 0x1001).w("ym2", FUNC(ym3812_device::write));
	map(0x3000, 0x3000).r(m_soundlatch, FUNC(generic_latch_8_device::read));
	map(0x3800, 0x3800).rw("oki", FUNC(okim6295_device::read), FUNC(okim6295_device::write));
	map(0x8000, 0xffff).rom();
}

/******************************************************************************/

static INPUT_PORTS_START( stadhero )
	PORT_START("INPUTS")    // 0x30c000 - 0x30c001
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1)
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(1)
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(1)
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2)
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(2)
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_START2 )

	PORT_START("DSW")   // 0x30c004 - 0x30c005
	PORT_DIPNAME( 0x0003, 0x0003, DEF_STR( Coin_A ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(      0x0001, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(      0x0003, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(      0x0002, DEF_STR( 1C_2C ) )
	PORT_DIPNAME( 0x000c, 0x000c, DEF_STR( Coin_B ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(      0x0004, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(      0x000c, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(      0x0008, DEF_STR( 1C_2C ) )
	PORT_DIPUNUSED( 0x0010, IP_ACTIVE_LOW )
	PORT_DIPNAME( 0x0020, 0x0020, DEF_STR( Demo_Sounds ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0020, DEF_STR( On ) )
	PORT_DIPNAME( 0x0040, 0x0040, DEF_STR( Flip_Screen ) )
	PORT_DIPSETTING(      0x0040, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPUNUSED( 0x0080, IP_ACTIVE_LOW )
	PORT_DIPNAME( 0x0300, 0x0300, "Time (1P Vs CPU)" )          // Table at 0x0014f6
	PORT_DIPSETTING(      0x0200, "600" )
	PORT_DIPSETTING(      0x0300, "500" )
	PORT_DIPSETTING(      0x0100, "450" )
	PORT_DIPSETTING(      0x0000, "400" )
	PORT_DIPNAME( 0x0c00, 0x0c00, "Time (1P Vs 2P)" )           // Table at 0x0014fe
	PORT_DIPSETTING(      0x0800, "270" )
	PORT_DIPSETTING(      0x0c00, "210" )
	PORT_DIPSETTING(      0x0400, "180" )
	PORT_DIPSETTING(      0x0000, "120" )
	PORT_DIPNAME( 0x3000, 0x3000, "Final Set" )                 // Table at 0x00078c
	PORT_DIPSETTING(      0x2000, "3 Credits" )
	PORT_DIPSETTING(      0x3000, "4 Credits" )
	PORT_DIPSETTING(      0x1000, "5 Credits" )
	PORT_DIPSETTING(      0x0000, "6 Credits" )
	PORT_DIPUNUSED( 0x4000, IP_ACTIVE_LOW )
	PORT_DIPUNUSED( 0x8000, IP_ACTIVE_LOW )

	PORT_START("COIN")  // 0x30c002
	PORT_BIT( 0x0f, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_VBLANK("screen")
INPUT_PORTS_END

/******************************************************************************/

static const gfx_layout charlayout =
{
	8,8,    // 8*8 chars
	RGN_FRAC(1,3),
	3,      // 4 bits per pixel
	{ RGN_FRAC(0,3), RGN_FRAC(1,3), RGN_FRAC(2,3) },
	{ STEP8(0,1) },
	{ STEP8(0,8) },
	8*8 // every char takes 8 consecutive bytes
};

static const gfx_layout tile_3bpp =
{
	16,16,
	RGN_FRAC(1,3),
	3,
	{ RGN_FRAC(2,3), RGN_FRAC(1,3), RGN_FRAC(0,3) },
	{ STEP8(16*8,1), STEP8(0,1) },
	{ STEP16(0,8) },
	16*16
};

static const gfx_layout spritelayout =
{
	16,16,
	RGN_FRAC(1,4),
	4,
	{ RGN_FRAC(3,4), RGN_FRAC(2,4), RGN_FRAC(1,4), RGN_FRAC(0,4) },
	{ STEP8(16*8,1), STEP8(0,1) },
	{ STEP16(0,8) },
	16*16
};

static GFXDECODE_START( gfx_stadhero )
	GFXDECODE_ENTRY( "chars",   0, charlayout,     0, 16 ) // 8x8
	GFXDECODE_ENTRY( "tiles",   0, tile_3bpp,    512, 16 ) // 16x16
GFXDECODE_END

static GFXDECODE_START( gfx_stadhero_spr )
	GFXDECODE_ENTRY( "sprites", 0, spritelayout, 256, 16 ) // 16x16
GFXDECODE_END

/******************************************************************************/

void stadhero_state::stadhero(machine_config &config)
{
	// basic machine hardware
	M68000(config, m_maincpu, 20_MHz_XTAL / 2);
	m_maincpu->set_addrmap(AS_PROGRAM, &stadhero_state::main_map);
	m_maincpu->set_vblank_int("screen", FUNC(stadhero_state::irq5_line_assert));

	M6502(config, m_audiocpu, 24_MHz_XTAL / 16);
	m_audiocpu->set_addrmap(AS_PROGRAM, &stadhero_state::audio_map);

	// video hardware
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_refresh(HZ_TO_ATTOSECONDS(58));
	screen.set_vblank_time(ATTOSECONDS_IN_USEC(529));
	screen.set_size(32*8, 32*8);
	screen.set_visarea(0*8, 32*8-1, 1*8, 31*8-1);
	screen.set_screen_update(FUNC(stadhero_state::screen_update));
	screen.set_palette("palette");

	GFXDECODE(config, m_gfxdecode, "palette", gfx_stadhero);
	PALETTE(config, "palette").set_format(palette_device::xBGR_444, 1024);

	DECO_BAC06(config, m_tilegen, 0);
	m_tilegen->set_gfx_region_wide(1, 1, 2);
	m_tilegen->set_gfxdecode_tag(m_gfxdecode);

	DECO_MXC06(config, m_spritegen, 0, "palette", gfx_stadhero_spr);

	// sound hardware
	SPEAKER(config, "mono").front_center();

	GENERIC_LATCH_8(config, m_soundlatch, 0);
	m_soundlatch->data_pending_callback().set_inputline(m_audiocpu, INPUT_LINE_NMI);

	ym2203_device &ym1(YM2203(config, "ym1", 24_MHz_XTAL / 16));
	ym1.add_route(0, "mono", 0.95);
	ym1.add_route(1, "mono", 0.95);
	ym1.add_route(2, "mono", 0.95);
	ym1.add_route(3, "mono", 0.40);

	ym3812_device &ym2(YM3812(config, "ym2", 24_MHz_XTAL / 8));
	ym2.irq_handler().set_inputline(m_audiocpu, M6502_IRQ_LINE);
	ym2.add_route(ALL_OUTPUTS, "mono", 0.80);

	okim6295_device &oki(OKIM6295(config, "oki", 1.056_MHz_XTAL, okim6295_device::PIN7_HIGH));
	oki.add_route(ALL_OUTPUTS, "mono", 0.80);
}

/******************************************************************************/

ROM_START( stadhero )
	ROM_REGION( 0x20000, "maincpu", 0 ) // 68000
	ROM_LOAD16_BYTE( "ef15.9a",  0x00000, 0x10000, CRC(bbba364e) SHA1(552096102f402085596635f02096462c6b8e13a7) )
	ROM_LOAD16_BYTE( "ef13.4e",  0x00001, 0x10000, CRC(97c6717a) SHA1(6c81260f49a59f70c71f520e51330a6833828684) )

	ROM_REGION( 0x10000, "audiocpu", 0 ) // 6502
	ROM_LOAD( "ef18.7f",  0x08000, 0x08000, CRC(20fd9668) SHA1(058e34a0ebfc372aaa9230c2bc9164ee2e85e217) )

	ROM_REGION( 0x18000, "chars", 0 )
	ROM_LOAD( "ef08.2j",  0x00000, 0x10000, CRC(e84752fe) SHA1(9af2140ddbb44be793ab5b39787bac27f5b1c1f2) )
	ROM_LOAD( "ef09.4j",  0x10000, 0x08000, CRC(2ade874d) SHA1(5c884535214438a4ea79fd262700a346bc12ad81) )

	ROM_REGION( 0x30000, "tiles", 0 )
	ROM_LOAD( "ef11.13j", 0x00000, 0x10000, CRC(af563e96) SHA1(c88eaff4a1ea133d708f4511bb1dbc99ef066eed) )
	ROM_LOAD( "ef10.11j", 0x10000, 0x10000, CRC(dca3d599) SHA1(2b97a70065f3065e7fbb54fb53cb120d9e5013b3) )
	ROM_LOAD( "ef12.14j", 0x20000, 0x10000, CRC(9a1bf51c) SHA1(e733c193b305496878551fc6eefc21587ba75c82) )

	ROM_REGION( 0x80000, "sprites", 0 )
	ROM_LOAD( "ef00.2a",  0x00000, 0x10000, CRC(94ed257c) SHA1(caa4a4c8bf3b34d2288e117cfc704cca4c6f913b) )
	ROM_LOAD( "ef01.4a",  0x10000, 0x10000, CRC(6eb9a721) SHA1(0f9dce614e67e57612e3a4ce187f0f9c12b78281) )
	ROM_LOAD( "ef02.5a",  0x20000, 0x10000, CRC(850cb771) SHA1(ccb54036191674d76965270a5831fba3e62f47c0) )
	ROM_LOAD( "ef03.7a",  0x30000, 0x10000, CRC(24338b96) SHA1(7730486bd0b84ba0a69b5547e348ee0058d4e7f1) )
	ROM_LOAD( "ef04.8a",  0x40000, 0x10000, CRC(9e3d97a7) SHA1(d02722376721caa5d8498f15f16959f42b75e7c1) )
	ROM_LOAD( "ef05.9a",  0x50000, 0x10000, CRC(88631005) SHA1(3c1787fb3aabdd9fecf679b2f4a9f833bf660885) )
	ROM_LOAD( "ef06.11a", 0x60000, 0x10000, CRC(9f47848f) SHA1(e23337684c8999483cbd11d3d953b06c34f13069) )
	ROM_LOAD( "ef07.12a", 0x70000, 0x10000, CRC(8859f655) SHA1(b3d69c5808b3ba7347ddb7f9693499903e9bfe6b) )

	ROM_REGION( 0x40000, "oki", 0 ) // ADPCM samples
	ROM_LOAD( "ef17.1e",  0x00000, 0x10000, CRC(07c78358) SHA1(ce82b429eec0193fd9665b717336756a514db144) )

	ROM_REGION( 0x00200, "proms", 0 )
	ROM_LOAD( "ef19.3d",  0x00000, 0x00200, CRC(852ff668) SHA1(d3053b68f86dcc81c3c3be280f75a4acd0b05be2) )  // ?
ROM_END

} // anonymous namespace


/******************************************************************************/

GAME( 1988, stadhero, 0, stadhero, stadhero, stadhero_state, empty_init, ROT0, "Data East Corporation", "Stadium Hero (Japan)", MACHINE_SUPPORTS_SAVE )
