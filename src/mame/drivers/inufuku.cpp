// license:BSD-3-Clause
// copyright-holders:Takahiro Nogi
/******************************************************************************

    Video Hardware for Video System Games.

    Quiz & Variety Sukusuku Inufuku (Japan)
    (c)1998 Video System Co.,Ltd.

    3 On 3 Dunk Madness (US, prototype?)
    (c)1996 Video System Co.,Ltd.

    Driver by Takahiro Nogi <nogi@kt.rim.or.jp> 2003/08/09 -

    based on other Video System drivers

******************************************************************************/
/******************************************************************************

Quiz & Variety Sukusuku Inufuku
(c)1998 Video System

VSBB-31-1

CPU  : MC68HC000P-16
Sound: TMPZ84C000AP-8 YM2610 YM3016
OSC  : 32.0000MHz 14.31818MHz

ROMs:
U107.BIN     - Sound Program (27C1001)

U146.BIN     - Main Programs (27C240)
U147.BIN     |
LHMN5L28.148 / (32M Mask)

Others:
93C46 (EEPROM)
UMAG1 (ALTERA MAX EPM7128ELC84-10 BG9625)
PLD00?? (ALTERA EPM7032LC44-15 BA9631)
002 (PALCE16V8-10PC)
003 (PALCE16V8-15PC)

Custom Chips:
VS920A
VS920E
VS9210
VS9108 (Fujitsu CG10103)
(blank pattern for VS9210 and VS9108)

VSBB31-ROM

ROMs:
LHMN5KU6.U53 - 32M SOP Mask ROMs
LHMN5KU8.U40 |
LHMN5KU7.U8  |
LHMN5KUB.U34 |
LHMN5KUA.U36 |
LHMN5KU9.U38 /

******************************************************************************/
/******************************************************************************

TODO:

- User must initialize NVRAM at first boot in test mode (factory settings).

- Sometimes, sounds are not played (especially SFX), but this is a bug of real machine.

- Sound Code 0x08 remains unknown.

- Priority of tests and sprites seems to be correct, but I may have mistaken.

******************************************************************************/

#include "emu.h"
#include "includes/inufuku.h"

#include "cpu/m68000/m68000.h"
#include "cpu/z80/z80.h"
#include "machine/eepromser.h"
#include "sound/2610intf.h"
#include "screen.h"
#include "speaker.h"


/******************************************************************************

    Sound CPU interface

******************************************************************************/

WRITE8_MEMBER(inufuku_state::inufuku_soundrombank_w)
{
	membank("bank1")->set_entry(data & 0x03);
}

/******************************************************************************

    Input/Output port interface

******************************************************************************/

CUSTOM_INPUT_MEMBER(inufuku_state::soundflag_r)
{
	return m_soundlatch->pending_r() ? 0 : 1;
}

/******************************************************************************

    Main CPU memory handlers

******************************************************************************/

void inufuku_state::inufuku_map(address_map &map)
{
	map(0x000000, 0x0fffff).rom();         // main rom

//  AM_RANGE(0x100000, 0x100007) AM_WRITENOP    // ?

	map(0x180000, 0x180001).portr("P1");
	map(0x180002, 0x180003).portr("P2");
	map(0x180004, 0x180005).portr("SYSTEM");
	map(0x180006, 0x180007).portr("P4");
	map(0x180008, 0x180009).portr("EXTRA");
	map(0x18000a, 0x18000b).portr("P3");

	map(0x200000, 0x200001).portw("EEPROMOUT");
	map(0x280001, 0x280001).w(m_soundlatch, FUNC(generic_latch_8_device::write));   // sound command

	map(0x300000, 0x301fff).ram().w(m_palette, FUNC(palette_device::write16)).share("palette");                        // palette ram
	map(0x380000, 0x3801ff).writeonly().share("bg_rasterram");                                  // bg raster ram
	map(0x400000, 0x401fff).rw(FUNC(inufuku_state::inufuku_bg_videoram_r), FUNC(inufuku_state::inufuku_bg_videoram_w)).share("bg_videoram");     // bg ram
	map(0x402000, 0x403fff).rw(FUNC(inufuku_state::inufuku_tx_videoram_r), FUNC(inufuku_state::inufuku_tx_videoram_w)).share("tx_videoram");     // text ram
	map(0x404000, 0x40ffff).ram(); // ?? mirror (3on3dunk)
	map(0x580000, 0x581fff).ram().share("spriteram1");                          // sprite table + sprite attribute
	map(0x600000, 0x61ffff).ram().share("spriteram2");                                          // cell table

	map(0x780000, 0x780013).w(FUNC(inufuku_state::inufuku_palettereg_w)); // bg & text palettebank register
	map(0x7a0000, 0x7a0023).w(FUNC(inufuku_state::inufuku_scrollreg_w));  // bg & text scroll register
//  AM_RANGE(0x7e0000, 0x7e0001) AM_WRITENOP                    // ?

	map(0x800000, 0xbfffff).rom(); // data rom
	map(0xfd0000, 0xfdffff).ram(); // work ram
}


/******************************************************************************

    Sound CPU memory handlers

******************************************************************************/

void inufuku_state::inufuku_sound_map(address_map &map)
{
	map(0x0000, 0x77ff).rom();
	map(0x7800, 0x7fff).ram();
	map(0x8000, 0xffff).bankr("bank1");
}

void inufuku_state::inufuku_sound_io_map(address_map &map)
{
	map.global_mask(0xff);
	map(0x00, 0x00).w(FUNC(inufuku_state::inufuku_soundrombank_w));
	map(0x04, 0x04).rw(m_soundlatch, FUNC(generic_latch_8_device::read), FUNC(generic_latch_8_device::acknowledge_w));
	map(0x08, 0x0b).rw("ymsnd", FUNC(ym2610_device::read), FUNC(ym2610_device::write));
}

/******************************************************************************

    Port definitions

******************************************************************************/

static INPUT_PORTS_START( inufuku )
	PORT_START("P1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_PLAYER(1)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_PLAYER(1)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_PLAYER(1)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(1)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(1)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(1)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_PLAYER(1)

	PORT_START("P2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_PLAYER(2)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_PLAYER(2)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_PLAYER(2)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(2)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(2)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_PLAYER(2)

	PORT_START("SYSTEM")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_START2 )
	PORT_SERVICE_NO_TOGGLE( 0x10, IP_ACTIVE_LOW )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_TILT )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("P4")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_PLAYER(4)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_PLAYER(4)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_PLAYER(4)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(4)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(4)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(4)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(4)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_PLAYER(4)

	PORT_START("EXTRA")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_COIN3 )
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_COIN4 )
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_START3 )
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_START4 )
	PORT_DIPNAME( 0x0010, 0x0010, "3P/4P" )
	PORT_DIPSETTING(    0x0010, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x0000, DEF_STR( On ) )
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0040, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_READ_LINE_DEVICE_MEMBER("eeprom", eeprom_serial_93cxx_device, do_read)
	PORT_BIT( 0x0080, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_CUSTOM_MEMBER(DEVICE_SELF, inufuku_state,soundflag_r, nullptr)    // pending sound command
	PORT_BIT( 0xff00, IP_ACTIVE_LOW, IPT_UNKNOWN ) // 3on3dunk cares about something in here, possibly a vblank flag

	PORT_START( "EEPROMOUT" )
	PORT_BIT( 0x0800, IP_ACTIVE_HIGH, IPT_OUTPUT ) PORT_WRITE_LINE_DEVICE_MEMBER("eeprom", eeprom_serial_93cxx_device, di_write)
	PORT_BIT( 0x1000, IP_ACTIVE_HIGH, IPT_OUTPUT ) PORT_WRITE_LINE_DEVICE_MEMBER("eeprom", eeprom_serial_93cxx_device, clk_write)
	PORT_BIT( 0x2000, IP_ACTIVE_HIGH, IPT_OUTPUT ) PORT_WRITE_LINE_DEVICE_MEMBER("eeprom", eeprom_serial_93cxx_device, cs_write)

	PORT_START("P3")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_PLAYER(3)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_PLAYER(3)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_PLAYER(3)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(3)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(3)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(3)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(3)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_PLAYER(3)
INPUT_PORTS_END




/******************************************************************************

    Graphics definitions

******************************************************************************/

static const gfx_layout tilelayout =
{
	8, 8,
	RGN_FRAC(1, 1),
	8,
	{ 0, 1, 2, 3, 4, 5, 6, 7 },
	{ 1*8, 0*8, 3*8, 2*8, 5*8, 4*8, 7*8, 6*8 },
	{ 0*64, 1*64, 2*64, 3*64, 4*64, 5*64, 6*64, 7*64 },
	64*8
};

static const gfx_layout spritelayout =
{
	16, 16,
	RGN_FRAC(1, 1),
	4,
	{ 0, 1, 2, 3 },
	{ 2*4, 3*4, 0*4, 1*4, 6*4, 7*4, 4*4, 5*4,
			10*4, 11*4, 8*4, 9*4, 14*4, 15*4, 12*4, 13*4 },
	{ 0*64, 1*64, 2*64, 3*64, 4*64, 5*64, 6*64, 7*64,
			8*64, 9*64, 10*64, 11*64, 12*64, 13*64, 14*64, 15*64 },
	128*8
};

static const gfx_layout spritelayout_alt =
{
	16, 16,
	RGN_FRAC(1, 1),
	4,
	{ 0, 1, 2, 3 },
	{ 1*4, 0*4, 3*4, 2*4, 5*4, 4*4, 7*4, 6*4,
			9*4, 8*4, 11*4, 10*4, 13*4, 12*4, 15*4, 14*4 },
	{ 0*64, 1*64, 2*64, 3*64, 4*64, 5*64, 6*64, 7*64,
			8*64, 9*64, 10*64, 11*64, 12*64, 13*64, 14*64, 15*64 },
	128*8
};

static GFXDECODE_START( gfx_inufuku )
	GFXDECODE_ENTRY( "gfx1", 0, tilelayout,    0, 256*16 )  // bg
	GFXDECODE_ENTRY( "gfx2", 0, tilelayout,    0, 256*16 )  // text
	GFXDECODE_ENTRY( "gfx3", 0, spritelayout,  0, 256*16 )  // sprite
GFXDECODE_END

static GFXDECODE_START( gfx_3on3dunk )
	GFXDECODE_ENTRY( "gfx1", 0, tilelayout,    0, 256*16 )  // bg
	GFXDECODE_ENTRY( "gfx2", 0, tilelayout,    0, 256*16 )  // text
	GFXDECODE_ENTRY( "gfx3", 0, spritelayout_alt,  0, 256*16 )  // sprite
GFXDECODE_END


/******************************************************************************

    Machine driver

******************************************************************************/

void inufuku_state::machine_start()
{
	uint8_t *ROM = memregion("audiocpu")->base();

	membank("bank1")->configure_entries(0, 4, &ROM[0x00000], 0x8000);
	membank("bank1")->set_entry(0);

	save_item(NAME(m_bg_scrollx));
	save_item(NAME(m_bg_scrolly));
	save_item(NAME(m_tx_scrollx));
	save_item(NAME(m_tx_scrolly));
	save_item(NAME(m_bg_raster));
	save_item(NAME(m_bg_palettebank));
	save_item(NAME(m_tx_palettebank));
}

void inufuku_state::machine_reset()
{
	m_bg_scrollx = 0;
	m_bg_scrolly = 0;
	m_tx_scrollx = 0;
	m_tx_scrolly = 0;
	m_bg_raster = 0;
	m_bg_palettebank = 0;
	m_tx_palettebank = 0;
}

void inufuku_state::inufuku(machine_config &config)
{
	/* basic machine hardware */
	M68000(config, m_maincpu, 32000000/2); /* 16.00 MHz */
	m_maincpu->set_addrmap(AS_PROGRAM, &inufuku_state::inufuku_map);
	m_maincpu->set_vblank_int("screen", FUNC(inufuku_state::irq1_line_hold));

	Z80(config, m_audiocpu, 32000000/4);       /* 8.00 MHz */
	m_audiocpu->set_addrmap(AS_PROGRAM, &inufuku_state::inufuku_sound_map);
	m_audiocpu->set_addrmap(AS_IO, &inufuku_state::inufuku_sound_io_map); /* IRQs are triggered by the YM2610 */

	EEPROM_93C46_16BIT(config, "eeprom");

	/* video hardware */
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_refresh_hz(60);
	screen.set_vblank_time(ATTOSECONDS_IN_USEC(2300));
	screen.set_size(2048, 256);
	screen.set_visarea(0, 319, 0, 223);
	screen.set_screen_update(FUNC(inufuku_state::screen_update_inufuku));
	screen.screen_vblank().set(FUNC(inufuku_state::screen_vblank_inufuku));
	screen.set_palette(m_palette);

	VSYSTEM_SPR(config, m_spr, 0);
	m_spr->set_offsets(0, 1); // reference videos confirm at least the +1 against tilemaps in 3on3dunk (the highscore header text and black box are meant to be 1 pixel misaligned, although there is currently a priority bug there too)
	m_spr->set_pdraw(true);
	m_spr->set_tile_indirect_cb(FUNC(inufuku_state::inufuku_tile_callback), this);
	m_spr->set_gfx_region(2);
	m_spr->set_gfxdecode_tag(m_gfxdecode);

	GFXDECODE(config, m_gfxdecode, m_palette, gfx_inufuku);
	PALETTE(config, m_palette).set_format(palette_device::xGBR_555, 4096);

	/* sound hardware */
	SPEAKER(config, "mono").front_center();

	GENERIC_LATCH_8(config, m_soundlatch);
	m_soundlatch->data_pending_callback().set_inputline(m_audiocpu, INPUT_LINE_NMI);
	m_soundlatch->set_separate_acknowledge(true);

	ym2610_device &ymsnd(YM2610(config, "ymsnd", 32000000/4));
	ymsnd.irq_handler().set_inputline(m_audiocpu, 0);
	ymsnd.add_route(0, "mono", 0.50);
	ymsnd.add_route(1, "mono", 0.75);
	ymsnd.add_route(2, "mono", 0.75);
}

void inufuku_state::_3on3dunk(machine_config &config)
{
	inufuku(config);
	m_gfxdecode->set_info(gfx_3on3dunk);
}





/******************************************************************************

    ROM definitions

******************************************************************************/

ROM_START( inufuku )
	ROM_REGION( 0x1000000, "maincpu", 0 )   // main cpu + data
	ROM_LOAD16_WORD_SWAP( "u147.bin",     0x0000000, 0x080000, CRC(ab72398c) SHA1(f5dc266ffa936ea6528b46a34113f5e2f8141d71) )
	ROM_LOAD16_WORD_SWAP( "u146.bin",     0x0080000, 0x080000, CRC(e05e9bd4) SHA1(af0fdf31c2bdf851bf15c9de725dcbbb58464d54) )
	ROM_LOAD16_WORD_SWAP( "lhmn5l28.148", 0x0800000, 0x400000, CRC(802d17e7) SHA1(43b26efea65fd051c094d19784cb977ced39a1a0) )

	ROM_REGION( 0x0020000, "audiocpu", 0 )  // sound cpu
	ROM_LOAD( "u107.bin", 0x0000000, 0x020000, CRC(1744ef90) SHA1(e019f4ca83e21aa25710cc0ca40ffe765c7486c9) )

	ROM_REGION( 0x0400000, "gfx1", 0 )  // bg
	ROM_LOAD16_WORD_SWAP( "lhmn5ku8.u40", 0x0000000, 0x400000, CRC(8cbca80a) SHA1(063e9be97f5a1f021f8326f2994b51f9af5e1eaf) )

	ROM_REGION( 0x0400000, "gfx2", 0 )  // text
	ROM_LOAD16_WORD_SWAP( "lhmn5ku7.u8",  0x0000000, 0x400000, CRC(a6c0f07f) SHA1(971803d1933d8296767d8766ea9f04dcd6ab065c) )

	ROM_REGION( 0x0c00000, "gfx3", 0 )  // sprite
	ROM_LOAD16_WORD_SWAP( "lhmn5kub.u34", 0x0000000, 0x400000, CRC(7753a7b6) SHA1(a2e8747ce83ea5a57e2fe62f2452de355d7f48b6) )
	ROM_LOAD16_WORD_SWAP( "lhmn5kua.u36", 0x0400000, 0x400000, CRC(1ac4402a) SHA1(c15acc6fce4fe0b54e92d14c31a1bd78acf2c8fc) )
	ROM_LOAD16_WORD_SWAP( "lhmn5ku9.u38", 0x0800000, 0x400000, CRC(e4e9b1b6) SHA1(4d4ad85fbe6a442d4f8cafad748bcae4af6245b7) )

	ROM_REGION( 0x0400000, "ymsnd", 0 ) // adpcm data
	ROM_LOAD( "lhmn5ku6.u53", 0x0000000, 0x400000, CRC(b320c5c9) SHA1(7c99da2d85597a3c008ed61a3aa5f47ad36186ec) )
ROM_END

ROM_START( 3on3dunk )
	ROM_REGION( 0x1000000, "maincpu", 0 )   // main cpu + data
	ROM_LOAD16_WORD_SWAP( "prog0_2_4_usa.u147",     0x0000000, 0x080000, CRC(957924ab) SHA1(6fe8ca711d11239310d58188e9d6d28cd27bc5af) )
	ROM_LOAD16_WORD_SWAP( "prog1_2_4_usa.u146",     0x0080000, 0x080000, CRC(2479e236) SHA1(729e6c85d34d6925c8d6557b138e2bed43e1de6a) )
	ROM_LOAD16_WORD_SWAP( "lh535l5y.u148", 0x0800000, 0x400000, CRC(aa33e02a) SHA1(86381ecf18fba9065cbc02112751c435bbf8b8b4) )

	ROM_REGION( 0x0020000, "audiocpu", 0 )  // sound cpu
	ROM_LOAD( "sound_prog_97_1_13.u107", 0x0000000, 0x020000, CRC(d9d42805) SHA1(ab5cb7c141d9c9ed5121ba4dbc1d0fa187bd9f68) )

	ROM_REGION( 0x0400000, "gfx1", 0 )  // bg
	ROM_LOAD16_WORD_SWAP( "lh525kwy.u40", 0x0000000, 0x400000, CRC(aaa426d1) SHA1(2f9a2981f336caf3188baec9a34f61452dee2203) )

	ROM_REGION( 0x0400000, "gfx2", 0 )  // text
	ROM_LOAD16_WORD_SWAP( "lh537nn4.u8",  0x0000000, 0x200000, CRC(2b7be1d8) SHA1(aac274a8f4028db7429478601a1761e61ab4f9a2) )

	ROM_REGION( 0x2000000, "gfx3", 0 )  // sprite
	ROM_LOAD( "lh535kwz.u34", 0x0000000, 0x400000, CRC(7372ce78) SHA1(ed2a861986357fad7ef983750cd906c3d722b862) )
	ROM_LOAD( "lh535kv0.u36", 0x0400000, 0x400000, CRC(247e5741) SHA1(8d71d964791fb4b86e390bcdf7744f616d6357b1) )
	ROM_LOAD( "lh535kv2.u38", 0x0800000, 0x400000, CRC(76449b1e) SHA1(b63d50c6f0dc91dc94dbcdda9842598529c1c26e) )
	ROM_LOAD( "lh537nn5.u20", 0x0c00000, 0x200000, CRC(f457cd3b) SHA1(cc13f5dc44e4675c1074a365b10f34e684817d81) )
	/*               0x0e00000, 0x200000 empty */
	ROM_LOAD( "lh536pnm.u32", 0x1000000, 0x400000, CRC(bc39e449) SHA1(5aea90b66ee03c70797ddc42dbcb064d83ce8cc7) )

	ROM_REGION( 0x0400000, "ymsnd", 0 ) // ADPCM data
	ROM_LOAD( "lh5388r1.u53", 0x0000000, 0x100000, CRC(765d892f) SHA1(9b078c879d0437d1669bf4301fd52a768aa4d293) )

	ROM_REGION( 0x400000, "ymsnd.deltat", 0 ) // speech
	ROM_LOAD( "lh536pkl.u51", 0x0000000, 0x300000, CRC(e4919abf) SHA1(d6af4b9c6ff62f92216c9927027d3b2376416bae) )
ROM_END



/******************************************************************************

    Game drivers

******************************************************************************/

GAME( 1998, inufuku,  0, inufuku,   inufuku, inufuku_state, empty_init, ROT0, "Video System Co.", "Quiz & Variety Sukusuku Inufuku (Japan)",         MACHINE_NO_COCKTAIL | MACHINE_SUPPORTS_SAVE )
GAME( 1996, 3on3dunk, 0, _3on3dunk, inufuku, inufuku_state, empty_init, ROT0, "Video System Co.", "3 On 3 Dunk Madness (US, prototype? 1997/02/04)", MACHINE_NO_COCKTAIL | MACHINE_SUPPORTS_SAVE | MACHINE_IMPERFECT_GRAPHICS ) // tilemap priority is wrong in places (basketball before explosion in attract, highscores)
