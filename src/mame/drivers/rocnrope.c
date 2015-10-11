// license:BSD-3-Clause
// copyright-holders:Chris Hardy
/***************************************************************************

    Roc'n Rope (c) 1983 Konami

    Based on drivers from Juno First emulator by Chris Hardy (chrish@kcbbs.gen.nz)

***************************************************************************/

#include "emu.h"
#include "cpu/m6809/m6809.h"
#include "machine/konami1.h"
#include "audio/timeplt.h"
#include "includes/konamipt.h"
#include "includes/rocnrope.h"

#define MASTER_CLOCK          XTAL_18_432MHz


/*************************************
 *
 *  Memory handlers
 *
 *************************************/

/* Roc'n'Rope has the IRQ vectors in RAM. The rom contains $FFFF at this address! */
WRITE8_MEMBER(rocnrope_state::rocnrope_interrupt_vector_w)
{
	UINT8 *RAM = memregion("maincpu")->base();

	RAM[0xfff2 + offset] = data;
}

WRITE8_MEMBER(rocnrope_state::irq_mask_w)
{
	m_irq_mask = data & 1;
	if (!m_irq_mask)
		m_maincpu->set_input_line(0, CLEAR_LINE);
}

/*************************************
 *
 *  Address maps
 *
 *************************************/

static ADDRESS_MAP_START( rocnrope_map, AS_PROGRAM, 8, rocnrope_state )
	AM_RANGE(0x3080, 0x3080) AM_READ_PORT("SYSTEM")
	AM_RANGE(0x3081, 0x3081) AM_READ_PORT("P1")
	AM_RANGE(0x3082, 0x3082) AM_READ_PORT("P2")
	AM_RANGE(0x3083, 0x3083) AM_READ_PORT("DSW1")
	AM_RANGE(0x3000, 0x3000) AM_READ_PORT("DSW2")
	AM_RANGE(0x3100, 0x3100) AM_READ_PORT("DSW3")
	AM_RANGE(0x4000, 0x402f) AM_RAM AM_SHARE("spriteram2")
	AM_RANGE(0x4400, 0x442f) AM_RAM AM_SHARE("spriteram")
	AM_RANGE(0x4000, 0x47ff) AM_RAM
	AM_RANGE(0x4800, 0x4bff) AM_RAM_WRITE(rocnrope_colorram_w) AM_SHARE("colorram")
	AM_RANGE(0x4c00, 0x4fff) AM_RAM_WRITE(rocnrope_videoram_w) AM_SHARE("videoram")
	AM_RANGE(0x5000, 0x5fff) AM_RAM
	AM_RANGE(0x8000, 0x8000) AM_WRITE(watchdog_reset_w)
	AM_RANGE(0x8080, 0x8080) AM_WRITE(rocnrope_flipscreen_w)
	AM_RANGE(0x8081, 0x8081) AM_DEVWRITE("timeplt_audio", timeplt_audio_device, sh_irqtrigger_w)  /* cause interrupt on audio CPU */
	AM_RANGE(0x8082, 0x8082) AM_WRITENOP    /* ??? */
	AM_RANGE(0x8083, 0x8083) AM_WRITENOP    /* Coin counter 1 */
	AM_RANGE(0x8084, 0x8084) AM_WRITENOP    /* Coin counter 2 */
	AM_RANGE(0x8087, 0x8087) AM_WRITE(irq_mask_w)
	AM_RANGE(0x8100, 0x8100) AM_WRITE(soundlatch_byte_w)
	AM_RANGE(0x8182, 0x818d) AM_WRITE(rocnrope_interrupt_vector_w)
	AM_RANGE(0x6000, 0xffff) AM_ROM
ADDRESS_MAP_END


/*************************************
 *
 *  Input ports
 *
 *************************************/

static INPUT_PORTS_START( rocnrope )
	PORT_START("SYSTEM")
	KONAMI8_SYSTEM_UNK

	PORT_START("P1")
	KONAMI8_MONO_4WAY_B12_UNK

	PORT_START("P2")
	KONAMI8_COCKTAIL_4WAY_B12_UNK

	PORT_START("DSW1")
	KONAMI_COINAGE_LOC(DEF_STR( Free_Play ), "No Coin B", SW1)
	/* "No Coin B" = coins produce sound, but no effect on coin counter */

	PORT_START("DSW2")
	PORT_DIPNAME( 0x03, 0x03, DEF_STR( Lives ) )        PORT_DIPLOCATION("SW2:1,2")
	PORT_DIPSETTING(    0x03, "3" )
	PORT_DIPSETTING(    0x02, "4" )
	PORT_DIPSETTING(    0x01, "5" )
	PORT_DIPSETTING(    0x00, "255 (Cheat)")
	PORT_DIPNAME( 0x04, 0x00, DEF_STR( Cabinet ) )      PORT_DIPLOCATION("SW2:3")
	PORT_DIPSETTING(    0x00, DEF_STR( Upright ) )
	PORT_DIPSETTING(    0x04, DEF_STR( Cocktail ) )
	PORT_DIPNAME( 0x78, 0x58, DEF_STR( Difficulty ) )   PORT_DIPLOCATION("SW2:4,5,6,7")
	PORT_DIPSETTING(    0x78, "1 (Easy)" )
	PORT_DIPSETTING(    0x70, "2" )
	PORT_DIPSETTING(    0x68, "3" )
	PORT_DIPSETTING(    0x60, "4" )
	PORT_DIPSETTING(    0x58, "5" )
	PORT_DIPSETTING(    0x50, "6" )
	PORT_DIPSETTING(    0x48, "7" )
	PORT_DIPSETTING(    0x40, "8" )
	PORT_DIPSETTING(    0x38, "9" )
	PORT_DIPSETTING(    0x30, "10" )
	PORT_DIPSETTING(    0x28, "11" )
	PORT_DIPSETTING(    0x20, "12" )
	PORT_DIPSETTING(    0x18, "13" )
	PORT_DIPSETTING(    0x10, "14" )
	PORT_DIPSETTING(    0x08, "15" )
	PORT_DIPSETTING(    0x00, "16 (Difficult)" )
	PORT_DIPNAME( 0x80, 0x00, DEF_STR( Demo_Sounds ) )  PORT_DIPLOCATION("SW2:8")
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("DSW3")
	PORT_DIPNAME( 0x07, 0x06, "First Bonus" )           PORT_DIPLOCATION("SW3:1,2,3")
//  PORT_DIPSETTING(    0x07, "20000" ) // unused
	PORT_DIPSETTING(    0x06, "20000" )
	PORT_DIPSETTING(    0x05, "30000" )
	PORT_DIPSETTING(    0x04, "40000" )
	PORT_DIPSETTING(    0x03, "50000" )
	PORT_DIPSETTING(    0x02, "60000" )
	PORT_DIPSETTING(    0x01, "70000" )
	PORT_DIPSETTING(    0x00, "80000" )
	PORT_DIPNAME( 0x38, 0x10, "Repeated Bonus" )        PORT_DIPLOCATION("SW3:4,5,6")
	/* 0x28, 0x30 and 0x38 (unused) all gives 40000 */
	PORT_DIPSETTING(    0x20, "40000" )
	PORT_DIPSETTING(    0x18, "50000" )
	PORT_DIPSETTING(    0x10, "60000" )
	PORT_DIPSETTING(    0x08, "70000" )
	PORT_DIPSETTING(    0x00, "80000" )
	PORT_DIPNAME( 0x40, 0x00, "Grant Repeated Bonus" )  PORT_DIPLOCATION("SW3:7")
	PORT_DIPSETTING(    0x40, DEF_STR( No ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Yes ) )
	PORT_DIPUNUSED_DIPLOC( 0x80, IP_ACTIVE_LOW, "SW3:8" )
INPUT_PORTS_END



/*************************************
 *
 *  Graphics definitions
 *
 *************************************/

static const gfx_layout charlayout =
{
	8,8,    /* 8*8 sprites */
	512,    /* 512 characters */
	4,  /* 4 bits per pixel */
	{ 0x2000*8+4, 0x2000*8+0, 4, 0 },
	{ 0, 1, 2, 3, 8*8+0, 8*8+1, 8*8+2, 8*8+3 },
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8 },
	16*8    /* every sprite takes 64 consecutive bytes */
};

static const gfx_layout spritelayout =
{
	16,16,  /* 16*16 sprites */
	256,    /* 256 sprites */
	4,  /* 4 bits per pixel */
	{ 256*64*8+4, 256*64*8+0, 4, 0 },
	{ 0, 1, 2, 3, 8*8+0, 8*8+1, 8*8+2, 8*8+3,
			16*8+0, 16*8+1, 16*8+2, 16*8+3, 24*8+0, 24*8+1, 24*8+2, 24*8+3 },
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8,
			32*8, 33*8, 34*8, 35*8, 36*8, 37*8, 38*8, 39*8 },
	64*8    /* every sprite takes 64 consecutive bytes */
};

static GFXDECODE_START( rocnrope )
	GFXDECODE_ENTRY( "gfx1", 0, spritelayout,     0, 16 )
	GFXDECODE_ENTRY( "gfx2", 0, charlayout,   16*16, 16 )
GFXDECODE_END



/*************************************
 *
 *  Machine driver
 *
 *************************************/

INTERRUPT_GEN_MEMBER(rocnrope_state::vblank_irq)
{
	if (m_irq_mask)
		device.execute().set_input_line(0, ASSERT_LINE);
}


static MACHINE_CONFIG_START( rocnrope, rocnrope_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", KONAMI1, MASTER_CLOCK / 3 / 4)        /* Verified in schematics */
	MCFG_CPU_PROGRAM_MAP(rocnrope_map)
	MCFG_CPU_VBLANK_INT_DRIVER("screen", rocnrope_state,  vblank_irq)

	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MCFG_SCREEN_SIZE(32*8, 32*8)
	MCFG_SCREEN_VISIBLE_AREA(0*8, 32*8-1, 2*8, 30*8-1)
	MCFG_SCREEN_UPDATE_DRIVER(rocnrope_state, screen_update_rocnrope)
	MCFG_SCREEN_PALETTE("palette")
	MCFG_SCREEN_ORIENTATION(ROT270)

	MCFG_GFXDECODE_ADD("gfxdecode", "palette", rocnrope)
	MCFG_PALETTE_ADD("palette", 16*16+16*16)
	MCFG_PALETTE_INDIRECT_ENTRIES(32)
	MCFG_PALETTE_INIT_OWNER(rocnrope_state, rocnrope)

	/* sound hardware */
	MCFG_FRAGMENT_ADD(timeplt_sound)
MACHINE_CONFIG_END

/*************************************
 *
 *  ROM definition(s)
 *
 *************************************/

/*
    Roc'n Rope

    CPU/Video Board: KT-A207-1C
    Sound Board:     KT-2207-2A
*/

ROM_START( rocnrope )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "rr1.1h",       0x6000, 0x2000, CRC(83093134) SHA1(c9509cfb9f9043cd6c226cc84dbc2e2b744488f6) )
	ROM_LOAD( "rr2.2h",       0x8000, 0x2000, CRC(75af8697) SHA1(70bb4b838cdafedf3d94425fad84f77815898d83) )
	ROM_LOAD( "rr3.3h",       0xa000, 0x2000, CRC(b21372b1) SHA1(c08ab3caaa646f4752f890d8339bce6b723864bb) )
	ROM_LOAD( "rr4.4h",       0xc000, 0x2000, CRC(7acb2a05) SHA1(93762d1890f40abc98372a2aa9fe0f63252b6389) )
	ROM_LOAD( "rnr_h5.vid",   0xe000, 0x2000, CRC(150a6264) SHA1(930ccf8dcf4971d0a15f406d9114be5ecfaa1727) )

	ROM_REGION( 0x10000, "tpsound", 0 )
	ROM_LOAD( "rnr_7a.snd",   0x0000, 0x1000, CRC(75d2c4e2) SHA1(b701019b4e7b06b268be660ce7958b5367318c27) )
	ROM_LOAD( "rnr_8a.snd",   0x1000, 0x1000, CRC(ca4325ae) SHA1(34ac035c0c2ed6bcafde1491d976bb9e9d2a2a7d) )

	ROM_REGION( 0x8000, "gfx1", 0 )
	ROM_LOAD( "rnr_a11.vid",  0x0000, 0x2000, CRC(afdaba5e) SHA1(27c090cb1c3767c997daeedbe1ba24786f9e78f1) )
	ROM_LOAD( "rnr_a12.vid",  0x2000, 0x2000, CRC(054cafeb) SHA1(4c3cd850b347217af3dd5c9bb84bcff7b30689bd) )
	ROM_LOAD( "rnr_a9.vid",   0x4000, 0x2000, CRC(9d2166b2) SHA1(42d2b05360e58b1b2b3ad06c98eb46d9da2b1c21) )
	ROM_LOAD( "rnr_a10.vid",  0x6000, 0x2000, CRC(aff6e22f) SHA1(476d67821519feddc9f9c8537b46e6eede790035) )

	ROM_REGION( 0x4000, "gfx2", 0 )
	ROM_LOAD( "rnr_h12.vid",  0x0000, 0x2000, CRC(e2114539) SHA1(0ea19ae4d7c2da14f23c81abb8e2c931785b2715) )
	ROM_LOAD( "rnr_h11.vid",  0x2000, 0x2000, CRC(169a8f3f) SHA1(182c7c9b9849ebb57b3ff7c0b629f2f8e2efa9ba) )

	ROM_REGION( 0x0220, "proms", 0 )
	ROM_LOAD( "a17_prom.bin", 0x0000, 0x0020, CRC(22ad2c3e) SHA1(1c2198b286c75aa9e78d000432795b1ce86ad6b9) )
	ROM_LOAD( "b16_prom.bin", 0x0020, 0x0100, CRC(750a9677) SHA1(7a5b4aed5f87180850657b8852bb3f3138d58b5b) )
	ROM_LOAD( "rocnrope.pr3", 0x0120, 0x0100, CRC(b5c75a27) SHA1(923d6ccf015fd7458494416cc05426cc922a9238) )

	ROM_REGION( 0x0001, "pal_cpuvidbd", 0 ) /* PAL located on the cpu/video board */
	ROM_LOAD( "h100.6g",      0x0000, 0x0001, NO_DUMP ) /* 20 Pin chip.  Appears to be a PAL.  Schematics obsfucated. */
ROM_END

ROM_START( rocnropek )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "rnr_h1.vid",   0x6000, 0x2000, CRC(0fddc1f6) SHA1(a9c6c033799883dc45eaa448387d4f0728b9e47e) )
	ROM_LOAD( "rnr_h2.vid",   0x8000, 0x2000, CRC(ce9db49a) SHA1(7a6ffb65cb65aa90e953706ee67c6ae91322ebf6) )
	ROM_LOAD( "rnr_h3.vid",   0xa000, 0x2000, CRC(6d278459) SHA1(a1417f158f288b0b0cbffc58f9e22b6c7cdf0fc7) )
	ROM_LOAD( "rnr_h4.vid",   0xc000, 0x2000, CRC(9b2e5f2a) SHA1(e91d7a9141dbe0fc5eacc2c5a672935993a3316f) )
	ROM_LOAD( "rnr_h5.vid",   0xe000, 0x2000, CRC(150a6264) SHA1(930ccf8dcf4971d0a15f406d9114be5ecfaa1727) )

	ROM_REGION( 0x10000, "tpsound", 0 )
	ROM_LOAD( "rnr_7a.snd",   0x0000, 0x1000, CRC(75d2c4e2) SHA1(b701019b4e7b06b268be660ce7958b5367318c27) )
	ROM_LOAD( "rnr_8a.snd",   0x1000, 0x1000, CRC(ca4325ae) SHA1(34ac035c0c2ed6bcafde1491d976bb9e9d2a2a7d) )

	ROM_REGION( 0x8000, "gfx1", 0 )
	ROM_LOAD( "rnr_a11.vid",  0x0000, 0x2000, CRC(afdaba5e) SHA1(27c090cb1c3767c997daeedbe1ba24786f9e78f1) )
	ROM_LOAD( "rnr_a12.vid",  0x2000, 0x2000, CRC(054cafeb) SHA1(4c3cd850b347217af3dd5c9bb84bcff7b30689bd) )
	ROM_LOAD( "rnr_a9.vid",   0x4000, 0x2000, CRC(9d2166b2) SHA1(42d2b05360e58b1b2b3ad06c98eb46d9da2b1c21) )
	ROM_LOAD( "rnr_a10.vid",  0x6000, 0x2000, CRC(aff6e22f) SHA1(476d67821519feddc9f9c8537b46e6eede790035) )

	ROM_REGION( 0x4000, "gfx2", 0 )
	ROM_LOAD( "rnr_h12.vid",  0x0000, 0x2000, CRC(e2114539) SHA1(0ea19ae4d7c2da14f23c81abb8e2c931785b2715) )
	ROM_LOAD( "rnr_h11.vid",  0x2000, 0x2000, CRC(169a8f3f) SHA1(182c7c9b9849ebb57b3ff7c0b629f2f8e2efa9ba) )

	ROM_REGION( 0x0220, "proms", 0 )
	ROM_LOAD( "a17_prom.bin", 0x0000, 0x0020, CRC(22ad2c3e) SHA1(1c2198b286c75aa9e78d000432795b1ce86ad6b9) )
	ROM_LOAD( "b16_prom.bin", 0x0020, 0x0100, CRC(750a9677) SHA1(7a5b4aed5f87180850657b8852bb3f3138d58b5b) )
	ROM_LOAD( "rocnrope.pr3", 0x0120, 0x0100, CRC(b5c75a27) SHA1(923d6ccf015fd7458494416cc05426cc922a9238) )

	ROM_REGION( 0x0001, "pal_cpuvidbd", 0 ) /* PAL located on the cpu/video board */
	ROM_LOAD( "h100.6g",      0x0000, 0x0001, NO_DUMP ) /* 20 Pin chip.  Appears to be a PAL.  Schematics obsfucated. */
ROM_END

/* Rope Man (a pirate of Roc'n'Rope)

4K files were 2732 and were on the sound board
8K files were 2764
The SUB devices were 82S153 PLS's
G06 was a MMI PAL10L8CN
A17 was a TBP18S030 read as an 82S123 and is probably the colour PROM.
B16 was a TBP24S10  read as an 82S131

Very popular bootleg that was once in MAME and was taken out due to only being a gfx/copyright hack.

    CPU/Video Board:          RR-324A
    Sound Board:              RR-324B
    Daughterboard on RR-324A: ** No markings **

*/

ROM_START( ropeman )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "r1.1j",            0x6000, 0x2000, CRC(6310a1fe) SHA1(7a32c5f24175b303bd40726c53c061951b9de594) )
	ROM_LOAD( "r2.2j",            0x8000, 0x2000, CRC(75af8697) SHA1(70bb4b838cdafedf3d94425fad84f77815898d83) )
	ROM_LOAD( "r3.3j",            0xa000, 0x2000, CRC(b21372b1) SHA1(c08ab3caaa646f4752f890d8339bce6b723864bb) )
	ROM_LOAD( "r4.4j",            0xc000, 0x2000, CRC(7acb2a05) SHA1(93762d1890f40abc98372a2aa9fe0f63252b6389) )
	ROM_LOAD( "r5.5j",            0xe000, 0x2000, CRC(150a6264) SHA1(930ccf8dcf4971d0a15f406d9114be5ecfaa1727) )

	ROM_REGION( 0x10000, "tpsound", 0 )
	ROM_LOAD( "r12.7a",           0x0000, 0x1000, CRC(75d2c4e2) SHA1(b701019b4e7b06b268be660ce7958b5367318c27) )
	ROM_LOAD( "r13.8a",           0x1000, 0x1000, CRC(ca4325ae) SHA1(34ac035c0c2ed6bcafde1491d976bb9e9d2a2a7d) )

	ROM_REGION( 0x8000, "gfx1", 0 )
	ROM_LOAD( "r10.11a",          0x0000, 0x2000, CRC(afdaba5e) SHA1(27c090cb1c3767c997daeedbe1ba24786f9e78f1) )
	ROM_LOAD( "r11.12a",          0x2000, 0x2000, CRC(054cafeb) SHA1(4c3cd850b347217af3dd5c9bb84bcff7b30689bd) )
	ROM_LOAD( "r8.9a",            0x4000, 0x2000, CRC(9d2166b2) SHA1(42d2b05360e58b1b2b3ad06c98eb46d9da2b1c21) )
	ROM_LOAD( "r9.10a",           0x6000, 0x2000, CRC(aff6e22f) SHA1(476d67821519feddc9f9c8537b46e6eede790035) )

	ROM_REGION( 0x4000, "gfx2", 0 )
	ROM_LOAD( "r7.12j",           0x0000, 0x2000, CRC(cd8ac4bf) SHA1(22bfd9ac0188bec6a1e8daa8ab915af1a5de7bd7) )
	ROM_LOAD( "r6.11j",           0x2000, 0x2000, CRC(35891835) SHA1(9dc6795e336c61b5349cf7bf69a3dc9438ae9336) )

	ROM_REGION( 0x0220, "proms", 0 )
	ROM_LOAD( "1.17a",            0x0000, 0x0020, CRC(22ad2c3e) SHA1(1c2198b286c75aa9e78d000432795b1ce86ad6b9) ) /* TBP18S030N */
	ROM_LOAD( "2.16b",            0x0020, 0x0100, CRC(750a9677) SHA1(7a5b4aed5f87180850657b8852bb3f3138d58b5b) ) /* TBP24S10N */
	ROM_LOAD( "3.16g",            0x0120, 0x0100, CRC(b5c75a27) SHA1(923d6ccf015fd7458494416cc05426cc922a9238) ) /* TBP24S10N */

	ROM_REGION( 0x0001, "pal_cpuvidbd", 0 ) /* MMI PAL10L8 located on the cpu/video board */
	ROM_LOAD( "pal10l8.6g",       0x0000, 0x0001, NO_DUMP )

	ROM_REGION( 0x01D6, "pals_daughterbd", 0 ) /* N82S153's located on the daughterboard of the cpu/video board */
	ROM_LOAD( "n82s153.pal1.bin", 0x0000, 0x00eb, CRC(baebe804) SHA1(c2e084b4df8a5c6d12cc34106583b532cd7a697b) ) /* Signetics N82S153 */
	ROM_LOAD( "n82s153.pal2.bin", 0x00eb, 0x00eb, CRC(a0e1b7a0) SHA1(7c3ce1a286bef69830a5e67a85965fe71f7ee283) ) /* Signetics N82S153 */
ROM_END


/*************************************
 *
 *  Driver initialization
 *
 *************************************/

DRIVER_INIT_MEMBER(rocnrope_state,rocnrope)
{
	memregion("maincpu")->base()[0x703d] = 0x98^0x22; // HACK: fix one instruction
}


/*************************************
 *
 *  Game driver(s)
 *
 *************************************/

GAME( 1983, rocnrope,  0,        rocnrope, rocnrope, rocnrope_state, rocnrope, ROT270, "Konami", "Roc'n Rope", MACHINE_SUPPORTS_SAVE )
GAME( 1983, rocnropek, rocnrope, rocnrope, rocnrope, driver_device,  0,        ROT270, "Konami (Kosuka license)", "Roc'n Rope (Kosuka)", MACHINE_SUPPORTS_SAVE )
GAME( 1983, ropeman,   rocnrope, rocnrope, rocnrope, rocnrope_state, rocnrope, ROT270, "bootleg", "Ropeman (bootleg of Roc'n Rope)", MACHINE_SUPPORTS_SAVE )
