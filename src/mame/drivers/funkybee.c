// license:BSD-3-Clause
// copyright-holders:Zsolt Vasvari
/***************************************************************************

Funky Bee/Sky Lancer memory map (preliminary)

driver by Zsolt Vasvari

MAIN CPU:

0000-4fff ROM
8000-87ff RAM
a000-bfff video RAM (only 0x20 bytes of each 0x100 byte block is used)
                    (also contains sprite RAM)
c000-dfff color RAM (only 0x20 bytes of each 0x100 byte block is used)
                    (also contains sprite RAM)

read:
f000      interrupt ACK
f800      IN0/watchdog
f801      IN1
f802      IN2

write:
e000      row scroll
e800      flip screen
e802-e803 coin counter
e804      ???
e805      gfx bank select
e806      ???
f800      watchdog


I/0 ports:
write
00        8910  control
01        8910  write

AY8910 Port A = DSW


Stephh's notes (based on the games Z80 code and some tests) :

1) 'funkybee' and clones

1a) 'funkybee'

  - Possible "Lives" settings : 3, 4, 5 or 6 (code at 0x0501)
  - Bonus life routine at 0x2d03 (test on DSW bit 6)

1b) 'funkybeeb'

  - Removal of ORCA copyright on title screen (text at 0x0e9a).
    However, high scores table remains unchanged.
  - Bypass ROM check (code at 0x3ee3)
  - Possible "Lives" settings : 1, 2, 3 or 4 (code at 0x0501)
  - Bonus life routine at 0x2d03 (test on DSW bit 6)

2) 'skylancr' and clones

2a) 'skylancr'

  - Possible "Lives" settings : 1, 2, 3 or 4 (code at 0x0601)
  - Bonus life routine at 0x1ef6 (test on DSW bit 5 !)
    I can't tell if it's an ingame bug or if this was done on purpose,
    but "Bonus Life" settings depend on the starting number of lives.
    DSW bit 6 has no effect because of this.

2a) 'skylancre'

  - Possible "Lives" settings : 3, 4, 5 or 6 (code at 0x0601)
  - Bonus life routine at 0x1f28 (test on DSW bit 6)

***************************************************************************/

#include "emu.h"
#include "cpu/z80/z80.h"
#include "sound/ay8910.h"
#include "includes/funkybee.h"


READ8_MEMBER(funkybee_state::funkybee_input_port_0_r)
{
	watchdog_reset_r(space, 0);
	return ioport("IN0")->read();
}

WRITE8_MEMBER(funkybee_state::funkybee_coin_counter_w)
{
	coin_counter_w(machine(), offset, data);
}

static ADDRESS_MAP_START( funkybee_map, AS_PROGRAM, 8, funkybee_state )
	AM_RANGE(0x0000, 0x4fff) AM_ROM
	AM_RANGE(0x8000, 0x87ff) AM_RAM
	AM_RANGE(0xa000, 0xbfff) AM_RAM_WRITE(funkybee_videoram_w) AM_SHARE("videoram")
	AM_RANGE(0xc000, 0xdfff) AM_RAM_WRITE(funkybee_colorram_w) AM_SHARE("colorram")
	AM_RANGE(0xe000, 0xe000) AM_WRITE(funkybee_scroll_w)
	AM_RANGE(0xe800, 0xe800) AM_WRITE(funkybee_flipscreen_w)
	AM_RANGE(0xe802, 0xe803) AM_WRITE(funkybee_coin_counter_w)
	AM_RANGE(0xe805, 0xe805) AM_WRITE(funkybee_gfx_bank_w)
	AM_RANGE(0xf000, 0xf000) AM_READNOP /* IRQ Ack */
	AM_RANGE(0xf800, 0xf800) AM_READWRITE(funkybee_input_port_0_r, watchdog_reset_w)
	AM_RANGE(0xf801, 0xf801) AM_READ_PORT("IN1")
	AM_RANGE(0xf802, 0xf802) AM_READ_PORT("IN2")
ADDRESS_MAP_END

static ADDRESS_MAP_START( io_map, AS_IO, 8, funkybee_state )
	ADDRESS_MAP_GLOBAL_MASK(0xff)
	AM_RANGE(0x00, 0x01) AM_DEVWRITE("aysnd", ay8910_device, address_data_w)
	AM_RANGE(0x02, 0x02) AM_DEVREAD("aysnd", ay8910_device, data_r)
ADDRESS_MAP_END


static INPUT_PORTS_START( funkybee )
	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_COIN1 )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_COIN2 )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_START1 )
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_START2 )
	PORT_DIPNAME( 0x20, 0x20, "Freeze" )
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_8WAY
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_8WAY
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP ) PORT_8WAY
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN ) PORT_8WAY
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_BUTTON1 )
	PORT_BIT( 0xe0, IP_ACTIVE_HIGH, IPT_UNKNOWN )

	PORT_START("IN2")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_COCKTAIL
	PORT_BIT( 0xe0, IP_ACTIVE_HIGH, IPT_UNKNOWN )

	PORT_START("DSW")
	PORT_DIPNAME( 0x03, 0x03, DEF_STR( Coin_A ) )       PORT_DIPLOCATION("SW1:8,7")
	PORT_DIPSETTING(    0x03, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x02, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x01, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_4C ) )
	PORT_DIPNAME( 0x0c, 0x0c, DEF_STR( Coin_B ) )       PORT_DIPLOCATION("SW1:6,5")
	PORT_DIPSETTING(    0x08, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x0c, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x04, DEF_STR( 2C_3C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_6C ) )
	PORT_DIPNAME( 0x30, 0x30, DEF_STR( Lives ) )        PORT_DIPLOCATION("SW1:4,3")
	PORT_DIPSETTING(    0x30, "3" )
	PORT_DIPSETTING(    0x20, "4" )
	PORT_DIPSETTING(    0x10, "5" )
	PORT_DIPSETTING(    0x00, "6" )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Bonus_Life ) )   PORT_DIPLOCATION("SW1:2")
	PORT_DIPSETTING(    0x40, "20000" )
	PORT_DIPSETTING(    0x00, DEF_STR( None ) )
	PORT_DIPNAME( 0x80, 0x00, DEF_STR( Cabinet ) )      PORT_DIPLOCATION("SW1:1")
	PORT_DIPSETTING(    0x00, DEF_STR( Upright ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Cocktail ) )
INPUT_PORTS_END

static INPUT_PORTS_START( funkybeeb )
	PORT_INCLUDE(funkybee)

	PORT_MODIFY("DSW")
	PORT_DIPNAME( 0x30, 0x30, DEF_STR( Lives ) )        PORT_DIPLOCATION("SW1:4,3")
	PORT_DIPSETTING(    0x30, "1" )
	PORT_DIPSETTING(    0x20, "2" )
	PORT_DIPSETTING(    0x10, "3" )
	PORT_DIPSETTING(    0x00, "4" )
INPUT_PORTS_END

static INPUT_PORTS_START( skylancr )
	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_COIN1 )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_COIN2 )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_START1 )
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_START2 )
	PORT_DIPNAME( 0x20, 0x20, "Freeze" )
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_8WAY
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_8WAY
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP ) PORT_8WAY
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN ) PORT_8WAY
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_BUTTON1 )
	PORT_BIT( 0xe0, IP_ACTIVE_HIGH, IPT_UNKNOWN )

	PORT_START("IN2")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_COCKTAIL
	PORT_BIT( 0xe0, IP_ACTIVE_HIGH, IPT_UNKNOWN )

	PORT_START("DSW")
	PORT_DIPNAME( 0x03, 0x03, DEF_STR( Coin_A ) )       PORT_DIPLOCATION("SW1:8,7")
	PORT_DIPSETTING(    0x03, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x02, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x01, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_6C ) )
	PORT_DIPNAME( 0x0c, 0x0c, DEF_STR( Coin_B ) )       PORT_DIPLOCATION("SW1:6,5")
	PORT_DIPSETTING(    0x08, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x0c, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x04, DEF_STR( 2C_3C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_6C ) )
	PORT_DIPNAME( 0x30, 0x30, DEF_STR( Lives ) )        PORT_DIPLOCATION("SW1:4,3") /* Also affects bonus life */
	PORT_DIPSETTING(    0x30, "1" )             /* Bonus life at 20000 and 50000 */
	PORT_DIPSETTING(    0x20, "2" )             /* Bonus life at 20000 and 50000 */
	PORT_DIPSETTING(    0x10, "3" )             /* Bonus life at 40000 and 70000 */
	PORT_DIPSETTING(    0x00, "4" )             /* Bonus life at 40000 and 70000 */
	PORT_DIPUNUSED_DIPLOC( 0x40, IP_ACTIVE_LOW, "SW1:2" )
	PORT_DIPNAME( 0x80, 0x00, DEF_STR( Cabinet ) )      PORT_DIPLOCATION("SW1:1")
	PORT_DIPSETTING(    0x00, DEF_STR( Upright ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Cocktail ) )
INPUT_PORTS_END

static INPUT_PORTS_START( skylancre )
	PORT_INCLUDE(skylancr)

	PORT_MODIFY("DSW")
	PORT_DIPNAME( 0x30, 0x30, DEF_STR( Lives ) )        PORT_DIPLOCATION("SW1:4,3") /* Also affects bonus life */
	PORT_DIPSETTING(    0x30, "3" )
	PORT_DIPSETTING(    0x20, "4" )
	PORT_DIPSETTING(    0x10, "5" )
	PORT_DIPSETTING(    0x00, "6" )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Bonus_Life ) )   PORT_DIPLOCATION("SW1:2") /* Manual calls this "Excent Play" (Excellent or extended?) */
	PORT_DIPSETTING(    0x40, "20000 50000" )       /* Manual calls this "Normal Level" */
	PORT_DIPSETTING(    0x00, "40000 70000" )       /* Manual calls this "High Level" */
INPUT_PORTS_END


static const gfx_layout charlayout =
{
	8,8,
	RGN_FRAC(1,1),
	2,
	{ 0, 4 },
	{ 0, 1, 2, 3, 8*8+0, 8*8+1, 8*8+2, 8*8+3 },
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8 },
	16*8
};

static const gfx_layout spritelayout =
{
	8,32,
	RGN_FRAC(1,1),
	2,
	{ 0, 4 },
	{ 0, 1, 2, 3, 8*8+0, 8*8+1, 8*8+2, 8*8+3 },
	{  0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8,
		16*8, 17*8, 18*8, 19*8, 20*8, 21*8, 22*8, 23*8,
		32*8, 33*8, 34*8, 35*8, 36*8, 37*8, 38*8, 39*8,
		48*8, 49*8, 50*8, 51*8, 52*8, 53*8, 54*8, 55*8 },
	4*16*8
};

static GFXDECODE_START( funkybee )
	GFXDECODE_ENTRY( "gfx1", 0, charlayout,   0, 8 )
	GFXDECODE_ENTRY( "gfx2", 0, charlayout,   0, 8 )
	GFXDECODE_ENTRY( "gfx1", 0, spritelayout, 16, 4 )
	GFXDECODE_ENTRY( "gfx2", 0, spritelayout, 16, 4 )
GFXDECODE_END


void funkybee_state::machine_start()
{
	save_item(NAME(m_gfx_bank));
}

void funkybee_state::machine_reset()
{
	m_gfx_bank = 0;
}

static MACHINE_CONFIG_START( funkybee, funkybee_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", Z80, 3072000)   /* 3.072 MHz */
	MCFG_CPU_PROGRAM_MAP(funkybee_map)
	MCFG_CPU_IO_MAP(io_map)
	MCFG_CPU_VBLANK_INT_DRIVER("screen", funkybee_state,  irq0_line_hold)


	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(2500) /* not accurate */)
	MCFG_SCREEN_SIZE(32*8, 32*8)
	MCFG_SCREEN_VISIBLE_AREA(12, 32*8-8-1, 0*8, 28*8-1)
	MCFG_SCREEN_UPDATE_DRIVER(funkybee_state, screen_update_funkybee)
	MCFG_SCREEN_PALETTE("palette")
	MCFG_SCREEN_ORIENTATION(ROT90)

	MCFG_GFXDECODE_ADD("gfxdecode", "palette", funkybee)
	MCFG_PALETTE_ADD("palette", 32)
	MCFG_PALETTE_INIT_OWNER(funkybee_state, funkybee)

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")

	MCFG_SOUND_ADD("aysnd", AY8910, 1500000)
	MCFG_AY8910_PORT_A_READ_CB(IOPORT("DSW"))
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.50)
MACHINE_CONFIG_END


/***************************************************************************

  Game driver(s)

***************************************************************************/

ROM_START( funkybee )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "funkybee.1",    0x0000, 0x1000, CRC(3372cb33) SHA1(09f2673cdeaadba8211d86a19e727aebb4d8be9d) )
	ROM_LOAD( "funkybee.3",    0x1000, 0x1000, CRC(7bf7c62f) SHA1(f8e5514c17fddb8ed95e5e18aab81ad0ebcc41af) )
	ROM_LOAD( "funkybee.2",    0x2000, 0x1000, CRC(8cc0fe8e) SHA1(416d97db0a2219ea46f2caa55787253e16a5ef32) )
	ROM_LOAD( "funkybee.4",    0x3000, 0x1000, CRC(1e1aac26) SHA1(a2974e6a8da5568f91aa44adb58941b0a60b1536) )

	ROM_REGION( 0x2000, "gfx1", 0 )
	ROM_LOAD( "funkybee.5",    0x0000, 0x2000, CRC(86126655) SHA1(d91682121d7f6a70f10a946ab81b248cc29bdf8c) )

	ROM_REGION( 0x2000, "gfx2", 0 )
	ROM_LOAD( "funkybee.6",    0x0000, 0x2000, CRC(5fffd323) SHA1(9de9c869bd1e2daab3b94275444ecbe904bcd6aa) )

	ROM_REGION( 0x0020, "proms", 0 )
	ROM_LOAD( "funkybee.clr",  0x0000, 0x0020, CRC(e2cf5fe2) SHA1(50b293f48f078cbcebccb045aa779ced2fb298c8) )
ROM_END

/* This is a bootleg of "Funky Bee", where ORCA copyright has been removed and difficulty is harder,
   there are 2 lives less then in the original game
   TODO: insert correct DIPSWITCH, where lives is "1,2,3,4" instead of "3,4,5,6" */
ROM_START( funkybeeb )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "senza_orca.fb1", 0x0000, 0x1000, CRC(7f2e7f85) SHA1(d4b63add3a97fc80aeafcd72a261302ab52d60a7) )
	ROM_LOAD( "funkybee.3",     0x1000, 0x1000, CRC(7bf7c62f) SHA1(f8e5514c17fddb8ed95e5e18aab81ad0ebcc41af) )
	ROM_LOAD( "funkybee.2",     0x2000, 0x1000, CRC(8cc0fe8e) SHA1(416d97db0a2219ea46f2caa55787253e16a5ef32) )
	ROM_LOAD( "senza_orca.fb4", 0x3000, 0x1000, CRC(53c2db3b) SHA1(0bda1eb87d7c41b67a5ff00b6675defdc8fe9274) )

	ROM_REGION( 0x2000, "gfx1", 0 )
	ROM_LOAD( "funkybee.5",     0x0000, 0x2000, CRC(86126655) SHA1(d91682121d7f6a70f10a946ab81b248cc29bdf8c) )

	ROM_REGION( 0x2000, "gfx2", 0 )
	ROM_LOAD( "funkybee.6",     0x0000, 0x2000, CRC(5fffd323) SHA1(9de9c869bd1e2daab3b94275444ecbe904bcd6aa) )

	ROM_REGION( 0x0020, "proms", 0 )
	ROM_LOAD( "funkybee.clr",   0x0000, 0x0020, CRC(e2cf5fe2) SHA1(50b293f48f078cbcebccb045aa779ced2fb298c8) )
ROM_END


/*
Sky Lancer PCB Layout
---------------------

  |--------------------------------------------|
 _|                          ROM.U33           |
|                                              |
|                            ROM.U32           |
|    WF19054                                   |
|                                              |
|_                                             |
  |                                  6264      |
  |                     |------|     6116      |
 _|           DSW4(8)   |ACTEL |               |
|             DSW3(8)   |A1010B|               |
|             DSW2(8)   |      |          6264 |
|             DSW1(8)   |------|               |
|                                         6264 |
|    M5M82C255                                 |
|                                              |
|       ROM.U35                                |
|3.6V_BATT                                     |
|_          6116              Z80        12MHz |
  |--------------------------------------------|
Notes:
      Z80 @ 3.0MHz [12/4]
      WF19054 = AY-3-8910 @ 1.5MHz [12/8]
*/

ROM_START( skylancr )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "1sl.5a",        0x0000, 0x2000, CRC(e80b315e) SHA1(0c02aa9f0d4bdfc3482c400d0e4e38fd3912a512) )
	ROM_LOAD( "2sl.5c",        0x2000, 0x2000, CRC(9d70567b) SHA1(05ff6f0c4b4d928e937556d9943a76f6cbc0f05f) )
	ROM_LOAD( "3sl.5d",        0x4000, 0x2000, CRC(64c39457) SHA1(b54a57a8576c2f852b765350c4504ccc3f5a431c) )

	ROM_REGION( 0x2000, "gfx1", 0 )
	ROM_LOAD( "4sl.6a",        0x0000, 0x2000, CRC(9b4469a5) SHA1(a0964e6d4fbdd15153be258f0d78680559a962f2) )

	ROM_REGION( 0x2000, "gfx2", 0 )
	ROM_LOAD( "5sl.6c",        0x0000, 0x2000, CRC(29afa134) SHA1(d94f483b4d234fe0b1d406322409417daec092f2) )

	ROM_REGION( 0x0020, "proms", 0 )
	ROM_LOAD( "18s030.1a",     0x0000, 0x0020, CRC(e645bacb) SHA1(5f4c299c4cf165fd229731c0e5799a34892bf28e) )
ROM_END

ROM_START( skylancre )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "1.5a",          0x0000, 0x2000, CRC(82d55824) SHA1(5c457e720ac8611bea4bc7e63ba4ee1c11200471) )
	ROM_LOAD( "2.5c",          0x2000, 0x2000, CRC(dff3a682) SHA1(e3197e106c2c6d198d2769b63701222d48a196d1) )
	ROM_LOAD( "3.5d",          0x4000, 0x1000, CRC(7c006ee6) SHA1(22719d4d0ad5c4f534a1613e0d74cab73973bab7) )

	ROM_REGION( 0x2000, "gfx1", 0 )
	ROM_LOAD( "4.6a",          0x0000, 0x2000, CRC(0f8ede07) SHA1(e04456fe12e2282191aee4823941f23ad8bda99d) )

	ROM_REGION( 0x2000, "gfx2", 0 )
	ROM_LOAD( "5.6b",          0x0000, 0x2000, CRC(24cec070) SHA1(2b7977b07acbe1394765675cd469db13a3b495f2) )

	ROM_REGION( 0x0020, "proms", 0 )
	ROM_LOAD( "18s030.1a",     0x0000, 0x0020, CRC(e645bacb) SHA1(5f4c299c4cf165fd229731c0e5799a34892bf28e) )
ROM_END

GAME( 1982, funkybee,  0,        funkybee, funkybee, driver_device, 0, ROT90, "Orca",                           "Funky Bee",                            MACHINE_SUPPORTS_SAVE )
GAME( 1982, funkybeeb, funkybee, funkybee, funkybeeb, driver_device,0, ROT90, "bootleg",                        "Funky Bee (bootleg, harder)",          MACHINE_SUPPORTS_SAVE )
GAME( 1983, skylancr,  0,        funkybee, skylancr, driver_device, 0, ROT90, "Orca",                           "Sky Lancer",                           MACHINE_SUPPORTS_SAVE )
GAME( 1983, skylancre, skylancr, funkybee, skylancre, driver_device,0, ROT90, "Orca (Esco Trading Co license)", "Sky Lancer (Esco Trading Co license)", MACHINE_SUPPORTS_SAVE )
