// license:BSD-3-Clause
// copyright-holders:Bryan McPhail
/***************************************************************************

    Lemmings                (c) 1991 Data East USA (DE-0357)

    Prototype!  Licensed from the home computer version this game never
    made it past the arcade field test stage.  Unlike most Data East games
    this hardware features a pixel layer and a VRAM layer, probably to
    make the transition from the pixel addressable computer code to the
    arcade hardware.

    As prototype software it seems to have a couple of non-critical bugs,
    the palette ram check and vram check both overrun their actual ramsize.

    Emulation by Bryan McPhail, mish@tendril.co.uk

***************************************************************************/

#include "emu.h"
#include "lemmings.h"

#include "cpu/m6809/m6809.h"
#include "cpu/m68000/m68000.h"
#include "sound/okim6295.h"
#include "sound/ymopm.h"
#include "screen.h"
#include "speaker.h"


void lemmings_state::lemmings_control_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	/* Offset==0 Pixel layer X scroll */
	if (offset == 4)
		return; /* Watchdog or IRQ ack */
	COMBINE_DATA(&m_control_data[offset]);
}

uint16_t lemmings_state::lemmings_trackball_r(offs_t offset)
{
	if ((offset & 2) == 0)
		return m_trackball_io[(offset & 1) | ((offset & 4) >> 1)]->read();
	return 0;
}

uint16_t lemmings_state::lem_protection_region_0_146_r(offs_t offset)
{
	int real_address = 0 + (offset *2);
	int deco146_addr = bitswap<32>(real_address, /* NC */31,30,29,28,27,26,25,24,23,22,21,20,19,18, 13,12,11,/**/      17,16,15,14,    10,9,8, 7,6,5,4, 3,2,1,0) & 0x7fff;
	uint8_t cs = 0;
	uint16_t data = m_deco146->read_data( deco146_addr, cs );
	return data;
}

void lemmings_state::lem_protection_region_0_146_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	int real_address = 0 + (offset *2);
	int deco146_addr = bitswap<32>(real_address, /* NC */31,30,29,28,27,26,25,24,23,22,21,20,19,18, 13,12,11,/**/      17,16,15,14,    10,9,8, 7,6,5,4, 3,2,1,0) & 0x7fff;
	uint8_t cs = 0;
	m_deco146->write_data( deco146_addr, data, mem_mask, cs );
}


/******************************************************************************/

void lemmings_state::lemmings_map(address_map &map)
{
	map(0x000000, 0x0fffff).rom();
	map(0x100000, 0x10ffff).ram();
	map(0x120000, 0x1207ff).ram().share("spriteram1");
	map(0x140000, 0x1407ff).ram().share("spriteram2");
	map(0x160000, 0x160fff).ram().w(m_palette, FUNC(palette_device::write16)).share("palette");
	map(0x170000, 0x17000f).ram().w(FUNC(lemmings_state::lemmings_control_w)).share("control_data");
	map(0x190000, 0x19000f).r(FUNC(lemmings_state::lemmings_trackball_r));
	map(0x1a0000, 0x1a3fff).rw(FUNC(lemmings_state::lem_protection_region_0_146_r), FUNC(lemmings_state::lem_protection_region_0_146_w)).share("prot16ram"); /* Protection device */
	map(0x1c0000, 0x1c0001).w(m_spriteram[0], FUNC(buffered_spriteram16_device::write)); /* 1 written once a frame */
	map(0x1e0000, 0x1e0001).w(m_spriteram[1], FUNC(buffered_spriteram16_device::write)); /* 1 written once a frame */
	map(0x200000, 0x201fff).ram().w(FUNC(lemmings_state::lemmings_vram_w)).share("vram_data");
	map(0x202000, 0x202fff).ram();
	map(0x300000, 0x37ffff).ram().w(FUNC(lemmings_state::lemmings_pixel_0_w)).share("pixel_0_data");
	map(0x380000, 0x39ffff).ram().w(FUNC(lemmings_state::lemmings_pixel_1_w)).share("pixel_1_data");
}

/******************************************************************************/

void lemmings_state::sound_map(address_map &map)
{
	map(0x0000, 0x07ff).ram();
	map(0x0800, 0x0801).rw("ymsnd", FUNC(ym2151_device::read), FUNC(ym2151_device::write));
	map(0x1000, 0x1000).rw("oki", FUNC(okim6295_device::read), FUNC(okim6295_device::write));
	map(0x1800, 0x1800).r(m_deco146, FUNC(deco_146_base_device::soundlatch_r)).nopw(); // writes an extra irq ack?
	map(0x8000, 0xffff).rom();
}

/******************************************************************************/

static INPUT_PORTS_START( lemmings )
	PORT_START("INPUTS")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_NAME("P1 Select")
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_NAME("P1 Hurry")
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2) PORT_NAME("P2 Select")
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2) PORT_NAME("P2 Hurry")
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("SYSTEM")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_SERVICE_NO_TOGGLE(0x0004, IP_ACTIVE_LOW)
	PORT_BIT( 0x0008, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_VBLANK("screen")


	PORT_START("DSW")
	PORT_DIPNAME( 0x0003, 0x0003, "Credits for 1 Player" )
	PORT_DIPSETTING(      0x0003, "1" )
	PORT_DIPSETTING(      0x0002, "2" )
	PORT_DIPSETTING(      0x0001, "3" )
	PORT_DIPSETTING(      0x0000, "4" )
	PORT_DIPNAME( 0x000c, 0x000c, "Credits for 2 Player" )
	PORT_DIPSETTING(      0x000c, "1" )
	PORT_DIPSETTING(      0x0008, "2" )
	PORT_DIPSETTING(      0x0004, "3" )
	PORT_DIPSETTING(      0x0000, "4" )
	PORT_DIPNAME( 0x0030, 0x0030, "Credits for Continue" )
	PORT_DIPSETTING(      0x0030, "1" )
	PORT_DIPSETTING(      0x0020, "2" )
	PORT_DIPSETTING(      0x0010, "3" )
	PORT_DIPSETTING(      0x0000, "4" )
	PORT_DIPNAME( 0x0040, 0x0040, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0040, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0080, 0x0080, DEF_STR( Free_Play ) )
	PORT_DIPSETTING(      0x0080, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )

	PORT_DIPNAME( 0x0700, 0x0700, DEF_STR( Coin_A ) )
	PORT_DIPSETTING(      0x0700, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(      0x0600, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(      0x0500, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(      0x0400, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(      0x0300, DEF_STR( 1C_5C ) )
	PORT_DIPSETTING(      0x0200, DEF_STR( 1C_6C ) )
	PORT_DIPSETTING(      0x0100, DEF_STR( 1C_7C ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( 1C_8C ) )
	PORT_DIPNAME( 0x3800, 0x3800, DEF_STR( Coin_B ) )
	PORT_DIPSETTING(      0x3800, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(      0x3000, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(      0x2800, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(      0x2000, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(      0x1800, DEF_STR( 1C_5C ) )
	PORT_DIPSETTING(      0x1000, DEF_STR( 1C_6C ) )
	PORT_DIPSETTING(      0x0800, DEF_STR( 1C_7C ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( 1C_8C ) )
	PORT_DIPUNKNOWN( 0x4000, 0x4000 )
	PORT_DIPUNKNOWN( 0x8000, 0x8000 )

	PORT_START("AN0")
	PORT_BIT( 0xff, 0x00, IPT_TRACKBALL_X ) PORT_SENSITIVITY(70) PORT_KEYDELTA(10) PORT_REVERSE PORT_PLAYER(1)

	PORT_START("AN1")
	PORT_BIT( 0xff, 0x00, IPT_TRACKBALL_Y ) PORT_SENSITIVITY(70) PORT_KEYDELTA(10) PORT_PLAYER(1)

	PORT_START("AN2")
	PORT_BIT( 0xff, 0x00, IPT_TRACKBALL_X ) PORT_SENSITIVITY(70) PORT_KEYDELTA(10) PORT_REVERSE PORT_PLAYER(2)

	PORT_START("AN3")
	PORT_BIT( 0xff, 0x00, IPT_TRACKBALL_Y ) PORT_SENSITIVITY(70) PORT_KEYDELTA(10) PORT_PLAYER(2)
INPUT_PORTS_END

/******************************************************************************/

static const gfx_layout charlayout =
{
	8,8,
	2048,
	4,
	{ STEP4(4,1) },
	{ STEP8(0,8) },
	{ STEP8(0,8*8) },
	8*8*8
};

static const gfx_layout sprite_layout =
{
	16,16,
	RGN_FRAC(1,1),
	4,
	{ STEP4(0,8) },
	{ STEP8(8*4*16,1), STEP8(0,1) },
	{ STEP16(0,8*4) },
	16*16*4
};

static GFXDECODE_START( gfx_lemmings )
	GFXDECODE_ENTRY( "gfx1", 0, sprite_layout,  512, 16 ) /* Sprites 16x16 */
	GFXDECODE_ENTRY( "gfx2", 0, sprite_layout,  768, 16 ) /* Sprites 16x16 */
	GFXDECODE_ENTRY( nullptr,0, charlayout,     0,   16 ) /* Dynamically modified */
GFXDECODE_END

/******************************************************************************/

void lemmings_state::machine_start()
{
}

void lemmings_state::lemmings(machine_config &config)
{
	/* basic machine hardware */
	M68000(config, m_maincpu, 28_MHz_XTAL / 2); // Data East 59
	m_maincpu->set_addrmap(AS_PROGRAM, &lemmings_state::lemmings_map);
	m_maincpu->set_vblank_int("screen", FUNC(lemmings_state::irq6_line_hold));

	MC6809E(config, m_audiocpu, 32.22_MHz_XTAL / 24); // MC68B09EP; clock not verified
	m_audiocpu->set_addrmap(AS_PROGRAM, &lemmings_state::sound_map);

	/* video hardware */
	BUFFERED_SPRITERAM16(config, m_spriteram[0]);
	BUFFERED_SPRITERAM16(config, m_spriteram[1]);

	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_refresh_hz(60);
	screen.set_vblank_time(ATTOSECONDS_IN_USEC(529));
	screen.set_size(40*8, 32*8);
	screen.set_visarea(0*8, 40*8-1, 2*8, 30*8-1);
	screen.set_screen_update(FUNC(lemmings_state::screen_update_lemmings));
	screen.screen_vblank().set(FUNC(lemmings_state::screen_vblank_lemmings));

	GFXDECODE(config, m_gfxdecode, m_palette, gfx_lemmings);
	PALETTE(config, m_palette).set_format(palette_device::xBGR_888, 1024);

	DECO_SPRITE(config, m_sprgen[0], 0);
	m_sprgen[0]->set_gfx_region(1);
	m_sprgen[0]->set_gfxdecode_tag(m_gfxdecode);

	DECO_SPRITE(config, m_sprgen[1], 0);
	m_sprgen[1]->set_gfx_region(0);
	m_sprgen[1]->set_gfxdecode_tag(m_gfxdecode);

	DECO146PROT(config, m_deco146, 0);
	m_deco146->port_a_cb().set_ioport("INPUTS");
	m_deco146->port_b_cb().set_ioport("SYSTEM");
	m_deco146->port_c_cb().set_ioport("DSW");
	m_deco146->set_use_magic_read_address_xor(true);
	m_deco146->soundlatch_irq_cb().set_inputline(m_audiocpu, M6809_FIRQ_LINE);

	/* sound hardware */
	SPEAKER(config, "lspeaker").front_left();
	SPEAKER(config, "rspeaker").front_right();

	GENERIC_LATCH_8(config, m_soundlatch);

	ym2151_device &ymsnd(YM2151(config, "ymsnd", 32.22_MHz_XTAL / 9)); // clock likely wrong
	ymsnd.irq_handler().set_inputline(m_audiocpu, M6809_IRQ_LINE);
	ymsnd.add_route(0, "lspeaker", 0.45);
	ymsnd.add_route(1, "rspeaker", 0.45);

	okim6295_device &oki(OKIM6295(config, "oki", 1023924, okim6295_device::PIN7_HIGH)); // clock frequency & pin 7 not verified
	oki.add_route(ALL_OUTPUTS, "lspeaker", 0.50);
	oki.add_route(ALL_OUTPUTS, "rspeaker", 0.50);
}

/******************************************************************************/

ROM_START( lemmings )
	ROM_REGION( 0x100000, "maincpu", 0 ) /* 68000 code */
	ROM_LOAD16_BYTE( "lemmings.5", 0x00000, 0x20000, CRC(e9a2b439) SHA1(873723a06d71bb41772951f451a75578b30267d5) )
	ROM_LOAD16_BYTE( "lemmings.1", 0x00001, 0x20000, CRC(bf52293b) SHA1(47a1ed64bf02776db086fdce80997b8a0c068791) )
	ROM_LOAD16_BYTE( "lemmings.6", 0x40000, 0x20000, CRC(0e3dc0ea) SHA1(533abf66ca4b578d03566d5de922dc5828c26eca) )
	ROM_LOAD16_BYTE( "lemmings.2", 0x40001, 0x20000, CRC(0cf3d7ce) SHA1(95dc43a8cded860fcf8743b62cbe4f2a97f43215) )
	ROM_LOAD16_BYTE( "lemmings.7", 0x80000, 0x20000, CRC(d020219c) SHA1(9678d8636798d1e528269fe2f9eb532e189c134e) )
	ROM_LOAD16_BYTE( "lemmings.3", 0x80001, 0x20000, CRC(c635494a) SHA1(e105dc79bd3c425d971629a3066c38dbf08b6428) )
	ROM_LOAD16_BYTE( "lemmings.8", 0xc0000, 0x20000, CRC(9166ce09) SHA1(7f0970cc07ebdbfc9a738342259d07d37b397161) )
	ROM_LOAD16_BYTE( "lemmings.4", 0xc0001, 0x20000, CRC(aa845488) SHA1(d17ec80f43d2a0123e93fad83d4e1319eb18d7c7) )

	ROM_REGION( 0x10000, "audiocpu", 0 )    /* Sound CPU */
	ROM_LOAD( "lemmings.15",    0x00000, 0x10000, CRC(f0b24a35) SHA1(1aaeb1e6faee04d2e433161fd7aa965b58e3b8a7) )

	ROM_REGION( 0x40000, "gfx1", ROMREGION_ERASE00 ) /* 3bpp data but sprite chip expects 4 */
	ROM_LOAD32_BYTE( "lemmings.9",  0x000003, 0x10000, CRC(e06442f5) SHA1(d9c8b681cce1d0257a0446bc820c7d679e2a1168) )
	ROM_LOAD32_BYTE( "lemmings.10", 0x000002, 0x10000, CRC(36398848) SHA1(6c6956607f889c35367e6df4a32359042fad695e) )
	ROM_LOAD32_BYTE( "lemmings.11", 0x000001, 0x10000, CRC(b46a54e5) SHA1(53b053346f80357aecff4ab888a8562f99cb318f) )

	ROM_REGION( 0x40000, "gfx2", ROMREGION_ERASE00 ) /* 3bpp data but sprite chip expects 4 */
	ROM_LOAD32_BYTE( "lemmings.12", 0x000003, 0x10000, CRC(dc9047ff) SHA1(1bbe573fa51127a9e8b970a353f3cceab00f486a) )
	ROM_LOAD32_BYTE( "lemmings.13", 0x000002, 0x10000, CRC(7cc15491) SHA1(73c1c11b2738f6679c70cae8ac4c55cdc9b8fc27) )
	ROM_LOAD32_BYTE( "lemmings.14", 0x000001, 0x10000, CRC(c162788f) SHA1(e1f669efa59699cd1b7da71b112701ee79240c18) )

	ROM_REGION( 0x40000, "oki", 0 ) /* ADPCM samples */
	ROM_LOAD( "lemmings.16",    0x00000, 0x20000, CRC(f747847c) SHA1(00880fa6dff979e5d15daea61938bd18c768c92f) )
ROM_END

/******************************************************************************/

GAME( 1991, lemmings, 0, lemmings, lemmings, lemmings_state, empty_init, ROT0, "Data East USA", "Lemmings (US prototype)", MACHINE_SUPPORTS_SAVE )
