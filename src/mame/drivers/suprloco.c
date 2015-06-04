// license:BSD-3-Clause
// copyright-holders:Zsolt Vasvari
/******************************************************************************

Super Locomotive

driver by Zsolt Vasvari

TODO:
- Bit 5 in control_w is pulsed when loco turns "super". This is supposed
  to make red parts of sprites blink to purple, it's not clear how this is
  implemented in hardware, there's a hack to support it.

Sega PCB 834-5137
 Sega 315-5015 (Sega custom encrypted Z80)
 Sega 315-5011
 Sega 315-5012
 Z80
 8255
 8 switch Dipswitch x 2

******************************************************************************/

#include "emu.h"
#include "cpu/z80/z80.h"
#include "machine/segacrpt.h"
#include "sound/sn76496.h"
#include "includes/suprloco.h"

WRITE8_MEMBER(suprloco_state::soundport_w)
{
	soundlatch_byte_w(space, 0, data);
	m_audiocpu->set_input_line(INPUT_LINE_NMI, PULSE_LINE);
	/* spin for a while to let the Z80 read the command (fixes hanging sound in Regulus) */
	space.device().execute().spin_until_time(attotime::from_usec(50));
}

static ADDRESS_MAP_START( main_map, AS_PROGRAM, 8, suprloco_state )
	AM_RANGE(0x0000, 0xbfff) AM_ROM
	AM_RANGE(0xc000, 0xc1ff) AM_RAM AM_SHARE("spriteram")
	AM_RANGE(0xc800, 0xc800) AM_READ_PORT("SYSTEM")
	AM_RANGE(0xd000, 0xd000) AM_READ_PORT("P1")
	AM_RANGE(0xd800, 0xd800) AM_READ_PORT("P2")
	AM_RANGE(0xe000, 0xe000) AM_READ_PORT("DSW1")
	AM_RANGE(0xe001, 0xe001) AM_READ_PORT("DSW2")
	AM_RANGE(0xe800, 0xe800) AM_WRITE(soundport_w)
	AM_RANGE(0xe801, 0xe801) AM_READWRITE(control_r, control_w)
	AM_RANGE(0xf000, 0xf6ff) AM_RAM_WRITE(videoram_w) AM_SHARE("videoram")
	AM_RANGE(0xf700, 0xf7df) AM_RAM /* unused */
	AM_RANGE(0xf7e0, 0xf7ff) AM_RAM_WRITE(scrollram_w) AM_SHARE("scrollram")
	AM_RANGE(0xf800, 0xffff) AM_RAM
ADDRESS_MAP_END

static ADDRESS_MAP_START( decrypted_opcodes_map, AS_DECRYPTED_OPCODES, 8, suprloco_state )
	AM_RANGE(0x0000, 0xbfff) AM_ROM AM_SHARE("decrypted_opcodes")
ADDRESS_MAP_END

static ADDRESS_MAP_START( sound_map, AS_PROGRAM, 8, suprloco_state )
	AM_RANGE(0x0000, 0x7fff) AM_ROM
	AM_RANGE(0x8000, 0x87ff) AM_RAM
	AM_RANGE(0xa000, 0xa003) AM_DEVWRITE("sn1", sn76496_device, write)
	AM_RANGE(0xc000, 0xc003) AM_DEVWRITE("sn2", sn76496_device, write)
	AM_RANGE(0xe000, 0xe000) AM_READ(soundlatch_byte_r)
ADDRESS_MAP_END



static INPUT_PORTS_START( suprloco )
	PORT_START("SYSTEM")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_SERVICE_NO_TOGGLE(0x04, IP_ACTIVE_LOW)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0xc0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("P1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON2 )
	PORT_BIT( 0xc0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("P2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_COCKTAIL
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_COCKTAIL
	PORT_BIT( 0xc0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("DSW1")
	PORT_DIPNAME( 0x07, 0x00, DEF_STR( Coin_A ) )       PORT_DIPLOCATION("SWA:1,2,3")
	PORT_DIPSETTING(    0x07, DEF_STR( 5C_1C ) )
	PORT_DIPSETTING(    0x06, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(    0x05, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x04, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x01, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x02, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0x03, DEF_STR( 1C_6C ) )
	PORT_DIPNAME( 0x38, 0x00, DEF_STR( Coin_B ) )       PORT_DIPLOCATION("SWA:4,5,6")
	PORT_DIPSETTING(    0x38, DEF_STR( 5C_1C ) )
	PORT_DIPSETTING(    0x30, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(    0x28, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x20, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x08, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x10, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0x18, DEF_STR( 1C_6C ) )
	PORT_DIPNAME( 0xc0, 0x40, DEF_STR( Lives ) )        PORT_DIPLOCATION("SWA:7,8")
	PORT_DIPSETTING(    0x00, "2" )
	PORT_DIPSETTING(    0x40, "3" )
	PORT_DIPSETTING(    0x80, "4" )
	PORT_DIPSETTING(    0xc0, "5" )

	PORT_START("DSW2")
	PORT_DIPNAME( 0x03, 0x00, DEF_STR( Bonus_Life ) )   PORT_DIPLOCATION("SWB:1,2")
	PORT_DIPSETTING(    0x00, "20000" )
	PORT_DIPSETTING(    0x01, "30000" )
	PORT_DIPSETTING(    0x02, "40000" )
	PORT_DIPSETTING(    0x03, "50000" )
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Unused ) )       PORT_DIPLOCATION("SWB:3")
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Free_Play ) )    PORT_DIPLOCATION("SWB:4")
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Difficulty ) )   PORT_DIPLOCATION("SWB:5")
	PORT_DIPSETTING(    0x10, DEF_STR( Easy ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Hard ) )
	PORT_DIPNAME( 0x20, 0x20, "Infinite Lives (Cheat)") PORT_DIPLOCATION("SWB:6")
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, "Initial Entry" )     PORT_DIPLOCATION("SWB:7")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x40, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x00, DEF_STR( Cabinet ) )      PORT_DIPLOCATION("SWB:8")
	PORT_DIPSETTING(    0x00, DEF_STR( Upright ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Cocktail ) )
INPUT_PORTS_END


static const gfx_layout charlayout =
{
	8,8,    /* 8 by 8 */
	1024,   /* 1024 characters */
	4,      /* 4 bits per pixel */
	{ 0, 1024*8*8, 2*1024*8*8, 3*1024*8*8 },            /* plane */
	{ 0, 1, 2, 3, 4, 5, 6, 7 },
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8 },
	8*8
};


static GFXDECODE_START( suprloco )
	/* sprites use colors 256-511 + 512-767 */
	GFXDECODE_ENTRY( "gfx1", 0x6000, charlayout, 0, 16 )
GFXDECODE_END


static MACHINE_CONFIG_START( suprloco, suprloco_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", Z80, 4000000)   /* 4 MHz (?) */
	MCFG_CPU_PROGRAM_MAP(main_map)
	MCFG_CPU_DECRYPTED_OPCODES_MAP(decrypted_opcodes_map)
	MCFG_CPU_VBLANK_INT_DRIVER("screen", suprloco_state,  irq0_line_hold)

	MCFG_CPU_ADD("audiocpu", Z80, 4000000)
	MCFG_CPU_PROGRAM_MAP(sound_map)
	MCFG_CPU_PERIODIC_INT_DRIVER(suprloco_state, irq0_line_hold, 4*60)          /* NMIs are caused by the main CPU */

	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(5000))
	MCFG_SCREEN_SIZE(32*8, 32*8)
	MCFG_SCREEN_VISIBLE_AREA(1*8, 31*8-1, 0*8, 28*8-1)
	MCFG_SCREEN_UPDATE_DRIVER(suprloco_state, screen_update)
	MCFG_SCREEN_PALETTE("palette")

	MCFG_GFXDECODE_ADD("gfxdecode", "palette", suprloco)
	MCFG_PALETTE_ADD("palette", 512+256)
	MCFG_PALETTE_INIT_OWNER(suprloco_state, suprloco)

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")

	MCFG_SOUND_ADD("sn1", SN76496, 4000000)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 1.0)

	MCFG_SOUND_ADD("sn2", SN76496, 2000000)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 1.0)
MACHINE_CONFIG_END


/***************************************************************************

  Game driver(s)

***************************************************************************/

ROM_START( suprloco )
	ROM_REGION( 0xc000, "maincpu", 0 )
	ROM_LOAD( "epr-5226a.37",    0x0000, 0x4000, CRC(33b02368) SHA1(c6e3116ad4b52bcc3174de5770f7a7ce024790d5) ) /* encrypted */
	ROM_LOAD( "epr-5227a.15",    0x4000, 0x4000, CRC(a5e67f50) SHA1(1dd52e4cf00ce414fe1db8259c9976cdc23513b4) ) /* encrypted */
	ROM_LOAD( "epr-5228.28",     0x8000, 0x4000, CRC(a597828a) SHA1(61004d112591fd2d752c39df71c1304d9308daae) )

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "epr-5222.64",     0x0000, 0x2000, CRC(0aa57207) SHA1(b29b533505cb5b47c90534f2f610baeb7265d030) )

	ROM_REGION( 0xe000, "gfx1", 0 )
	ROM_LOAD( "epr-5225.63",     0x0000, 0x2000, CRC(e571fe81) SHA1(ac2b5914a445b89b7456b2c4290e4630b525f05d) )
	ROM_LOAD( "epr-5224.62",     0x2000, 0x2000, CRC(6130f93c) SHA1(ae0657f46c10e75eec994e75359a89b5d61baf68) )
	ROM_LOAD( "epr-5223.61",     0x4000, 0x2000, CRC(3b03004e) SHA1(805b51cb14d3ace97f2e0f306db28921b2f5e322) )
							/* 0x6000- 0xe000 will be created by init_suprloco */

	ROM_REGION( 0x8000, "gfx2", 0 ) /* 32k for sprites data used at runtime */
	ROM_LOAD( "epr-5229.55",     0x0000, 0x4000, CRC(ee2d3ed3) SHA1(593f3cd5c4e7f20b5e31e6bac8864774442e4b75) )
	ROM_LOAD( "epr-5230.56",     0x4000, 0x2000, CRC(f04a4b50) SHA1(80363f0c508fb2a755bf684f9a6862c1e7285495) )
							/* 0x6000 empty */

	ROM_REGION( 0x0620, "proms", 0 )
	ROM_LOAD( "pr-5220.100",     0x0100, 0x0080, CRC(7b0c8ce5) SHA1(4e1ea5ce38198a3965dfeb609ba0c7e8211531c3) ) /* color PROM */
	ROM_CONTINUE(                0x0000, 0x0080 )
	ROM_CONTINUE(                0x0180, 0x0080 )
	ROM_CONTINUE(                0x0080, 0x0080 )
	ROM_LOAD( "pr-5219.89",      0x0200, 0x0400, CRC(1d4b02cb) SHA1(00d822f1bc4f57f2f5d5a0615241f8136246a842) ) /* 3bpp to 4bpp table */
	ROM_LOAD( "pr-5221.7",       0x0600, 0x0020, CRC(89ba674f) SHA1(17c87840c8011968675a5a6f55966467df02364b) ) /* unknown */
ROM_END

ROM_START( suprlocoo )
	ROM_REGION( 0xc000, "maincpu", 0 )
	ROM_LOAD( "epr-5226.37",     0x0000, 0x4000, CRC(57f514dd) SHA1(707800b90a22547a56b01d1e11775e9ee5555d23) ) /* encrypted */
	ROM_LOAD( "epr-5227.15",     0x4000, 0x4000, CRC(5a1d2fb0) SHA1(fdb9416e5530718245fd597073a63feddb233c3c) ) /* encrypted */
	ROM_LOAD( "epr-5228.28",     0x8000, 0x4000, CRC(a597828a) SHA1(61004d112591fd2d752c39df71c1304d9308daae) )

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "epr-5222.64",     0x0000, 0x2000, CRC(0aa57207) SHA1(b29b533505cb5b47c90534f2f610baeb7265d030) )

	ROM_REGION( 0xe000, "gfx1", 0 )
	ROM_LOAD( "epr-5225.63",     0x0000, 0x2000, CRC(e571fe81) SHA1(ac2b5914a445b89b7456b2c4290e4630b525f05d) )
	ROM_LOAD( "epr-5224.62",     0x2000, 0x2000, CRC(6130f93c) SHA1(ae0657f46c10e75eec994e75359a89b5d61baf68) )
	ROM_LOAD( "epr-5223.61",     0x4000, 0x2000, CRC(3b03004e) SHA1(805b51cb14d3ace97f2e0f306db28921b2f5e322) )
							/* 0x6000- 0xe000 will be created by init_suprloco */

	ROM_REGION( 0x8000, "gfx2", 0 ) /* 32k for sprites data used at runtime */
	ROM_LOAD( "epr-5229.55",     0x0000, 0x4000, CRC(ee2d3ed3) SHA1(593f3cd5c4e7f20b5e31e6bac8864774442e4b75) )
	ROM_LOAD( "epr-5230.56",     0x4000, 0x2000, CRC(f04a4b50) SHA1(80363f0c508fb2a755bf684f9a6862c1e7285495) )
							/* 0x6000 empty */

	ROM_REGION( 0x0620, "proms", 0 )
	ROM_LOAD( "pr-5220.100",     0x0100, 0x0080, CRC(7b0c8ce5) SHA1(4e1ea5ce38198a3965dfeb609ba0c7e8211531c3) ) /* color PROM */
	ROM_CONTINUE(                0x0000, 0x0080 )
	ROM_CONTINUE(                0x0180, 0x0080 )
	ROM_CONTINUE(                0x0080, 0x0080 )
	ROM_LOAD( "pr-5219.89",      0x0200, 0x0400, CRC(1d4b02cb) SHA1(00d822f1bc4f57f2f5d5a0615241f8136246a842) ) /* 3bpp to 4bpp table */
	ROM_LOAD( "pr-5221.7",       0x0600, 0x0020, CRC(89ba674f) SHA1(17c87840c8011968675a5a6f55966467df02364b) ) /* unknown */
ROM_END

DRIVER_INIT_MEMBER(suprloco_state,suprloco)
{
	/* convert graphics to 4bpp from 3bpp */

	int i, j, k, color_source, color_dest;
	UINT8 *source, *dest, *lookup;

	source = memregion("gfx1")->base();
	dest   = source + 0x6000;
	lookup = memregion("proms")->base() + 0x0200;

	for (i = 0; i < 0x80; i++, lookup += 8)
	{
		for (j = 0; j < 0x40; j++, source++, dest++)
		{
			dest[0] = dest[0x2000] = dest[0x4000] = dest[0x6000] = 0;

			for (k = 0; k < 8; k++)
			{
				color_source = (((source[0x0000] >> k) & 0x01) << 2) |
								(((source[0x2000] >> k) & 0x01) << 1) |
								(((source[0x4000] >> k) & 0x01) << 0);

				color_dest = lookup[color_source];

				dest[0x0000] |= (((color_dest >> 3) & 0x01) << k);
				dest[0x2000] |= (((color_dest >> 2) & 0x01) << k);
				dest[0x4000] |= (((color_dest >> 1) & 0x01) << k);
				dest[0x6000] |= (((color_dest >> 0) & 0x01) << k);
			}
		}
	}

	static const UINT8 convtable[32][4] =
	{
		/*       opcode                   data                     address      */
		/*  A    B    C    D         A    B    C    D                           */
		{ 0x20,0x00,0xa0,0x80 }, { 0xa8,0xa0,0x88,0x80 },   /* ...0...0...0...0 */
		{ 0x20,0x00,0xa0,0x80 }, { 0xa8,0xa0,0x88,0x80 },   /* ...0...0...0...1 */
		{ 0x20,0x00,0xa0,0x80 }, { 0xa8,0xa0,0x88,0x80 },   /* ...0...0...1...0 */
		{ 0x88,0x08,0x80,0x00 }, { 0xa0,0x80,0xa8,0x88 },   /* ...0...0...1...1 */
		{ 0x88,0x08,0x80,0x00 }, { 0xa0,0x80,0xa8,0x88 },   /* ...0...1...0...0 */
		{ 0x20,0x00,0xa0,0x80 }, { 0xa8,0xa0,0x88,0x80 },   /* ...0...1...0...1 */
		{ 0x88,0x08,0x80,0x00 }, { 0xa0,0x80,0xa8,0x88 },   /* ...0...1...1...0 */
		{ 0x28,0xa8,0x08,0x88 }, { 0x88,0x80,0x08,0x00 },   /* ...0...1...1...1 */
		{ 0x20,0x00,0xa0,0x80 }, { 0xa8,0xa0,0x88,0x80 },   /* ...1...0...0...0 */
		{ 0x88,0x08,0x80,0x00 }, { 0xa0,0x80,0xa8,0x88 },   /* ...1...0...0...1 */
		{ 0x88,0x08,0x80,0x00 }, { 0xa0,0x80,0xa8,0x88 },   /* ...1...0...1...0 */
		{ 0x20,0x00,0xa0,0x80 }, { 0xa8,0xa0,0x88,0x80 },   /* ...1...0...1...1 */
		{ 0x88,0x08,0x80,0x00 }, { 0xa0,0x80,0xa8,0x88 },   /* ...1...1...0...0 */
		{ 0x28,0xa8,0x08,0x88 }, { 0x88,0x80,0x08,0x00 },   /* ...1...1...0...1 */
		{ 0x20,0x00,0xa0,0x80 }, { 0xa8,0xa0,0x88,0x80 },   /* ...1...1...1...0 */
		{ 0x88,0x08,0x80,0x00 }, { 0xa0,0x80,0xa8,0x88 }    /* ...1...1...1...1 */
	};

	/* decrypt program ROMs */
	sega_decode(memregion("maincpu")->base(), m_decrypted_opcodes, 0xc000, convtable);
}



GAME( 1982, suprloco,         0, suprloco, suprloco, suprloco_state, suprloco, ROT0, "Sega", "Super Locomotive (Rev.A)", GAME_SUPPORTS_SAVE )
GAME( 1982, suprlocoo, suprloco, suprloco, suprloco, suprloco_state, suprloco, ROT0, "Sega", "Super Locomotive", GAME_SUPPORTS_SAVE )
