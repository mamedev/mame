// license:BSD-3-Clause
// copyright-holders:David Haywood, Nicola Salmoria
/*

US Games - Trivia / Quiz / 'Amusement Only' Gambling Games


there is a 'Security Test' in service mode

'superten', 'usg82', and 'usg83x' don't seem to be able to record
the changes you make in the "test mode" 8(


readme info

US Games
Counter Top Mini Games
1987-1992

In this archive are different versions.
Version 3.2, 8.3, 18.5, and 25.2.

Hardware Specs: MC6809P, MC6845P, MB8146A x 3
Sound: AY-3-8912

*/

#include "emu.h"
#include "cpu/m6809/m6809.h"
#include "video/mc6845.h"
#include "sound/ay8910.h"
#include "includes/usgames.h"
#include "machine/nvram.h"

void usgames_state::machine_start()
{
	membank("bank1")->configure_entries(0, 16, memregion("maincpu")->base() + 0x10000, 0x4000);
}

WRITE8_MEMBER(usgames_state::rombank_w)
{
	membank("bank1")->set_entry(data);
}

WRITE8_MEMBER(usgames_state::lamps1_w)
{
	/* button lamps */
	set_led_status(machine(), 0,data & 0x01);
	set_led_status(machine(), 1,data & 0x02);
	set_led_status(machine(), 2,data & 0x04);
	set_led_status(machine(), 3,data & 0x08);
	set_led_status(machine(), 4,data & 0x10);

	/* bit 5 toggles all the time - extra lamp? */
}

WRITE8_MEMBER(usgames_state::lamps2_w)
{
	/* bit 5 toggles all the time - extra lamp? */
}


static ADDRESS_MAP_START( usgames_map, AS_PROGRAM, 8, usgames_state )
	AM_RANGE(0x0000, 0x1fff) AM_RAM AM_SHARE("nvram")
	AM_RANGE(0x2000, 0x2000) AM_READ_PORT("DSW")
	AM_RANGE(0x2010, 0x2010) AM_READ_PORT("INPUTS")
	AM_RANGE(0x2020, 0x2020) AM_WRITE(lamps1_w)
	AM_RANGE(0x2030, 0x2030) AM_WRITE(lamps2_w)
	AM_RANGE(0x2040, 0x2040) AM_DEVWRITE("crtc", mc6845_device, address_w)
	AM_RANGE(0x2041, 0x2041) AM_READ_PORT("UNK1")
	AM_RANGE(0x2041, 0x2041) AM_DEVWRITE("crtc", mc6845_device, register_w)
	AM_RANGE(0x2060, 0x2060) AM_WRITE(rombank_w)
	AM_RANGE(0x2070, 0x2070) AM_READ_PORT("UNK2")
	AM_RANGE(0x2400, 0x2401) AM_DEVWRITE("aysnd", ay8910_device, address_data_w)
	AM_RANGE(0x2800, 0x2fff) AM_RAM_WRITE(charram_w) AM_SHARE("charram")
	AM_RANGE(0x3000, 0x3fff) AM_RAM_WRITE(videoram_w) AM_SHARE("videoram")
	AM_RANGE(0x4000, 0x7fff) AM_ROMBANK("bank1")
	AM_RANGE(0x8000, 0xffff) AM_ROM
ADDRESS_MAP_END


static ADDRESS_MAP_START( usg185_map, AS_PROGRAM, 8, usgames_state )
	AM_RANGE(0x0000, 0x1fff) AM_RAM AM_SHARE("nvram")
	AM_RANGE(0x2000, 0x2001) AM_DEVWRITE("aysnd", ay8910_device, address_data_w)
	AM_RANGE(0x2400, 0x2400) AM_READ_PORT("DSW")
	AM_RANGE(0x2410, 0x2410) AM_READ_PORT("INPUTS")
	AM_RANGE(0x2420, 0x2420) AM_WRITE(lamps1_w)
	AM_RANGE(0x2430, 0x2430) AM_WRITE(lamps2_w)
	AM_RANGE(0x2440, 0x2440) AM_DEVWRITE("crtc", mc6845_device, address_w)
	AM_RANGE(0x2441, 0x2441) AM_READ_PORT("UNK1")
	AM_RANGE(0x2441, 0x2441) AM_DEVWRITE("crtc", mc6845_device, register_w)
	AM_RANGE(0x2460, 0x2460) AM_WRITE(rombank_w)
	AM_RANGE(0x2470, 0x2470) AM_READ_PORT("UNK2")
	AM_RANGE(0x2800, 0x2fff) AM_RAM_WRITE(charram_w) AM_SHARE("charram")
	AM_RANGE(0x3000, 0x3fff) AM_RAM_WRITE(videoram_w) AM_SHARE("videoram")
	AM_RANGE(0x4000, 0x7fff) AM_ROMBANK("bank1")
	AM_RANGE(0x8000, 0xffff) AM_ROM
ADDRESS_MAP_END


static INPUT_PORTS_START( usg32 )
	PORT_START("INPUTS")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_NAME("Button 1") PORT_CODE(KEYCODE_Z)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_NAME("Button 2") PORT_CODE(KEYCODE_X)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_NAME("Button 3") PORT_CODE(KEYCODE_C)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_NAME("Button 4") PORT_CODE(KEYCODE_V)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON5 ) PORT_NAME("Button 5") PORT_CODE(KEYCODE_B)
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("DSW")
	PORT_DIPNAME( 0x01, 0x01, "Service Keyboard Attached?" )    // Not actually a DIP, when keyboard is plugged in, this goes low
	PORT_DIPSETTING(    0x01, DEF_STR( No ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Yes ) )
	PORT_SERVICE_NO_TOGGLE( 0x02, IP_ACTIVE_HIGH )
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_SPECIAL ) // +12 Volts?
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_CUSTOM ) PORT_VBLANK("screen")

	PORT_START("UNK1")
	PORT_DIPNAME( 0x01, 0x01, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("UNK2")
	PORT_DIPNAME( 0x01, 0x01, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
INPUT_PORTS_END

/* From here, the hardware was slightly upgraded, but not too different. */
static INPUT_PORTS_START( usg83 )
	PORT_INCLUDE( usg32 )

	PORT_MODIFY("DSW")
	PORT_DIPNAME( 0x04, 0x04, "Test_Switch" )
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
INPUT_PORTS_END


static const gfx_layout charlayout =
{
	8,8,
	0x100,
	1,
	{ 0 },
	{ 0, 1, 2, 3, 4, 5, 6, 7 },
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8 },
	8*8
};

static GFXDECODE_START( usgames )
	GFXDECODE_ENTRY( 0x0000, 0x2800, charlayout, 0, 256 )
GFXDECODE_END


static MACHINE_CONFIG_START( usg32, usgames_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", M6809, 2000000) /* ?? */
	MCFG_CPU_PROGRAM_MAP(usgames_map)
	MCFG_CPU_PERIODIC_INT_DRIVER(usgames_state, irq0_line_hold, 5*60) /* ?? */

	MCFG_NVRAM_ADD_0FILL("nvram")

	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(2500) /* not accurate */)
	MCFG_SCREEN_SIZE(64*8, 32*8)
	MCFG_SCREEN_VISIBLE_AREA(7*8, 57*8-1, 0*8, 31*8-1)
	MCFG_SCREEN_UPDATE_DRIVER(usgames_state, screen_update)
	MCFG_SCREEN_PALETTE("palette")

	MCFG_GFXDECODE_ADD("gfxdecode", "palette", usgames)
	MCFG_PALETTE_ADD("palette", 2*256)
	MCFG_PALETTE_INIT_OWNER(usgames_state, usgames)

	MCFG_MC6845_ADD("crtc", MC6845, "screen", XTAL_18MHz / 16)
	MCFG_MC6845_SHOW_BORDER_AREA(false)
	MCFG_MC6845_CHAR_WIDTH(8)

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")

	MCFG_SOUND_ADD("aysnd", AY8910, 2000000)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.30)
MACHINE_CONFIG_END

static MACHINE_CONFIG_DERIVED( usg185, usg32 )
	MCFG_CPU_MODIFY("maincpu")
	MCFG_CPU_PROGRAM_MAP(usg185_map)
MACHINE_CONFIG_END


ROM_START( usg32 )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD( "usg32-0.bin", 0x08000, 0x08000, CRC(bc313387) SHA1(8df2e2736f14e965303993ae4105176bdd59f49d) )
	/* for the banked region */
	ROM_LOAD( "usg32-1.bin", 0x18000, 0x08000, CRC(baaea800) SHA1(1f35b8c0d40a923488c591497a3c3806d6d104e1) )
	ROM_LOAD( "usg32-2.bin", 0x28000, 0x08000, CRC(d73d7f48) SHA1(a76582b80acd38abbb6f0f61d27b2920a3128516) )
	ROM_LOAD( "usg32-3.bin", 0x38000, 0x08000, CRC(22747804) SHA1(b86af1db1733ddd0629843e44da9bc8d6b102eb6) )
ROM_END

ROM_START( superten ) /*  Version 8.3 - You can't change the status of "Sexy Triv I" and "Sexy Triv II" */
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD( "usg83-rom0.bin", 0x08000, 0x08000, CRC(aae84186) SHA1(8385b5c1dded1ea6f90c277b045778c7110a45db) ) /* FRI JUL 01 22:43:10 1988, Checksum=30DF19 */
	/* for the banked region */
	ROM_LOAD( "usg83-rom1.bin", 0x18000, 0x08000, CRC(7b520b6f) SHA1(2231e63fecc6e9026dd4b6ee3e21a74cc0e0ae44) ) /* WED JUL 13 02:01:33 1988, Checksum=26F176 */
	ROM_LOAD( "usg83-rom2.bin", 0x28000, 0x08000, CRC(29fbb23b) SHA1(6c2c17897e60ec8d4cdeaf9b382ef00ab71f6e0a) ) /* THU FEB 04 12:27:14 1988, Checksum=7068E1 */
	ROM_LOAD( "usg83-rom3.bin", 0x38000, 0x10000, CRC(4e110844) SHA1(b51c596a41760f1f0f70f49ae81f03d98a17fb6f) ) /* WED NOV 25 14:57:05 1987, Checksum=6E3126 */
	ROM_LOAD( "usg83-rom4.bin", 0x48000, 0x08000, CRC(437697c4) SHA1(d14ae6f0b7adfb921c69ae3fdcd2cb525cb731fa) ) /* WED NOV 25 14:53:29 1987, Checksum DC7BF5 */
ROM_END

ROM_START( usg83x ) /* Version 8.3x - You can change the status of "Sexy Triv I" */
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD( "usg83x-rom0.bin", 0x08000, 0x08000, CRC(4ad9b6e0) SHA1(54940619511b37577bbcd9d05b941079ba793c72) ) /* FRI JUL 01 22:43:10 1988, Checksum=30DF51 */
	/* for the banked region */
	ROM_LOAD( "usg83-rom1.bin",  0x18000, 0x08000, CRC(7b520b6f) SHA1(2231e63fecc6e9026dd4b6ee3e21a74cc0e0ae44) ) /* WED JUL 13 02:01:33 1988, Checksum=26F176 */
	ROM_LOAD( "usg83-rom2.bin",  0x28000, 0x08000, CRC(29fbb23b) SHA1(6c2c17897e60ec8d4cdeaf9b382ef00ab71f6e0a) ) /* THU FEB 04 12:27:14 1988, Checksum=7068E1 */
	ROM_LOAD( "usg83x-rom3.bin", 0x38000, 0x08000, CRC(41c475ac) SHA1(48019843e2f57bf4c2fca5136e3d0a64de3dfc04) ) /* WED NOV 25 14:50:08 1987, Checksum=6E815F */
	ROM_LOAD( "usg83-rom4.bin",  0x48000, 0x08000, CRC(437697c4) SHA1(d14ae6f0b7adfb921c69ae3fdcd2cb525cb731fa) ) /* WED NOV 25 14:53:29 1987, Checksum DC7BF5 */
ROM_END

ROM_START( usg82 ) /*  Version 8.2 - "Sport Triv" and "Rush Hour" aren't available by default */
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD( "usg82-rom0.bin", 0x08000, 0x08000, CRC(09c20b78) SHA1(8b622fef536e98e22866a15c6a5b5da583169e8c) ) /* MON FEB 08 11:23:01 1988, Checksum=31911A */
	/* for the banked region */
	ROM_LOAD( "usg82-rom1.bin", 0x18000, 0x08000, CRC(915a9ff4) SHA1(5007210ed46a9cea530c18a8c4a67b07b87cb781) ) /* FRI FEB 05 16:17:13 1988, Checksum=36B7A3 */
	ROM_LOAD( "usg82-rom2.bin", 0x28000, 0x08000, CRC(29fbb23b) SHA1(6c2c17897e60ec8d4cdeaf9b382ef00ab71f6e0a) ) /* THU FEB 04 12:27:14 1988, Checksum=7068E1 */
	ROM_LOAD( "usg82-rom3.bin", 0x38000, 0x10000, CRC(4e110844) SHA1(b51c596a41760f1f0f70f49ae81f03d98a17fb6f) ) /* WED NOV 25 14:57:05 1987, Checksum=6E3126 */
	ROM_LOAD( "usg82-rom4.bin", 0x48000, 0x08000, CRC(437697c4) SHA1(d14ae6f0b7adfb921c69ae3fdcd2cb525cb731fa) ) /* WED NOV 25 14:53:29 1987, Checksum DC7BF5 */
ROM_END


/*
Games V18.2
US Games, 1989

A Trivia game by US Games.
The PCB looks _A LOT_ like a Williams Pinball PCB.
Perhaps they manufactured it?

PCB Layout
----------

|--------------------------------------------------|
|   DS1225   6809     68B45     AY-3-8912   TDA2003|
|                                      18MHz       |
|    GROM0                                         |-|
|           PAL                            ULN2805   |
|    GROM1          6116  6116                      2|
|                                                   2|
|    GROM2                                          W|
|                                                   A|
|    GROM3                                          Y|
|LED                                                 |
|LED GROM4                                         |-|
|    PAL      6116                                 |
|--------------------------------------------------|

Rom labels verified for v18.5, which look like this:

Copyright 1986-90     Copyright 1986-90     Copyright 1986-90     Copyright 1986-90     Copyright 1986-90
  U.S.Games.Inc.        U.S.Games.Inc.        U.S.Games.Inc.        U.S.Games.Inc.        U.S.Games.Inc.
   Version 18.5          Version 18.5          Version 18.5          Version 18.5          Version 18.5
  ROM 0 CS=315D5C       ROM 1 CS=2CB91D       ROM 2 CS=8973C0       ROM 3 CS=76AEBF       ROM 4 CS=90B95E

The first screen that comes up reports:

+--------------+-----------------------------+
|SPECIAL THANKS| ROM0  V18.5  315D5C  ****** |
|MARGARET+JAMIE+-----------------------------+
|FOR EVERYTHING| ROM1  V18.5  2CB91D  2CB91D |
|TESTSWITCH:OK!+-----------------------------+
|SYSTEM RAM:OK!| ROM2  V18.5  4957A2  8973C0 |
|CHARAC RAM:OK!+-----------------------------+
|COLOR  RAM:OK!| ROM3  V18.5  3ADB28  76AEBF |
|BLOCK  RAM:OK!+-----------------------------+
|+12 VOLTS :OK!| ROM4  V18.5  53125B  90B95E |
+--------------+-----------------------------+

This shows corresponding version number and checksums as listed on the labels.

Versions 18.3 and 18.6 confirmed to exist, just not currently dumped.

*/

ROM_START( usgames ) /* Version 25.4x */
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD( "version_25.4x_rom0_cs=324591.u12", 0x08000, 0x08000, CRC(766a855a) SHA1(e67ca9944d92192de423de6aa8a60f2e28b17db1) )
	/* for the banked region */
	ROM_LOAD( "version_25.4x_rom2_cs=6a42e7.u28", 0x1c000, 0x04000, CRC(d44d2ffa) SHA1(8bd756418b4f8ad11cb0f2044fb91c63d7771497) )  // ROM 2
	ROM_CONTINUE(                                 0x18000, 0x04000 )
	ROM_CONTINUE(                                 0x14000, 0x04000 )
	ROM_CONTINUE(                                 0x10000, 0x04000 )
	ROM_LOAD( "version_25.4x_rom1_cs=31a223.u18", 0x2c000, 0x04000, CRC(2fff1da2) SHA1(c44718f7aab82f45379f21b68e8ee2668fe3a378) )  // ROM 1
	ROM_CONTINUE(                                 0x28000, 0x04000 )
	ROM_CONTINUE(                                 0x24000, 0x04000 )
	ROM_CONTINUE(                                 0x20000, 0x04000 )
	ROM_LOAD( "version_25.4x_rom4_cs=5a447e.u36", 0x3c000, 0x04000, CRC(b6d007be) SHA1(ec2afe983fd925d9f4602f47ddadd117bcc74972) )  // ROM 4
	ROM_CONTINUE(                                 0x38000, 0x04000 )
	ROM_CONTINUE(                                 0x34000, 0x04000 )
	ROM_CONTINUE(                                 0x30000, 0x04000 )
	ROM_LOAD( "version_25.4x_rom3_cs=7924ba.u35", 0x4c000, 0x04000, CRC(9542295b) SHA1(56dd7b8fd581779656cb71cc42dbb9f77fb303f4) )  // ROM 3
	ROM_CONTINUE(                                 0x48000, 0x04000 )
	ROM_CONTINUE(                                 0x44000, 0x04000 )
	ROM_CONTINUE(                                 0x40000, 0x04000 )
ROM_END


ROM_START( usg187c ) /* Version 18.7C */
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD( "version_18.7c_rom0_cs=30a6ba.u12", 0x08000, 0x08000, CRC(2f4ed125) SHA1(6ea2ce263b8abe8d283d1c85d403ec908a422448) )
	/* for the banked region */
	ROM_LOAD( "version_18.7c_rom4_cs=90b95e.u36", 0x10000, 0x10000, CRC(b104744d) SHA1(fa2128c39a135b119ef625eed447afa523f912c0) )
	ROM_LOAD( "version_18.7c_rom3_cs=76aebf.u35", 0x20000, 0x10000, CRC(795e71c8) SHA1(852dceab906f79d05da67a81f855c71738662430) )
	ROM_LOAD( "version_18.7c_rom2_cs=8973c0.u28", 0x30000, 0x10000, CRC(c6ba8a81) SHA1(e826492626707e30782d4d2f42419357970d67b3) )
	ROM_LOAD( "version_18.7c_rom1_cs=6dcfd3.u18", 0x40000, 0x10000, CRC(1cfd934d) SHA1(544c41c5fcc2e576f5a8c88996f9257956f6c580) )
ROM_END


ROM_START( usg185 ) /* Version 18.5 */
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD( "version_18.5_rom0_cs=315d5c.u12", 0x08000, 0x08000, CRC(2cc68502) SHA1(e00b5bb5ca5a4a5f5ee3c8b0fd779659337f881f) )
	/* for the banked region */
	ROM_LOAD( "version_18.5_rom4_cs=90b95e.u36", 0x10000, 0x10000, CRC(b104744d) SHA1(fa2128c39a135b119ef625eed447afa523f912c0) )
	ROM_LOAD( "version_18.5_rom3_cs=76aebf.u35", 0x20000, 0x10000, CRC(795e71c8) SHA1(852dceab906f79d05da67a81f855c71738662430) )
	ROM_LOAD( "version_18.5_rom2_cs=8973c0.u28", 0x30000, 0x10000, CRC(c6ba8a81) SHA1(e826492626707e30782d4d2f42419357970d67b3) )
	ROM_LOAD( "version_18.5_rom1_cs=2cb91d.u18", 0x48000, 0x08000, CRC(bd384e5a) SHA1(1a09e2485471aa22f4260e6993f4636f7310e0b5) )
ROM_END


ROM_START( usg182 ) /* Version 18.2 */
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD( "version_18.2_rom0_cs=2e6ae3.u12",    0x08000, 0x08000, CRC(f5a053c1) SHA1(ae2740cd9af0af7a74a88720ebafd785bfc8614b) )
	/* for the banked region */
	ROM_LOAD( "version_18.2_rom4_cs=90b95e.u36", 0x10000, 0x10000, CRC(b104744d) SHA1(fa2128c39a135b119ef625eed447afa523f912c0) )
	ROM_LOAD( "version_18.2_rom3_cs=76aebf.u35", 0x20000, 0x10000, CRC(795e71c8) SHA1(852dceab906f79d05da67a81f855c71738662430) )
	ROM_LOAD( "version_18.2_rom2_cs=8973c0.u28", 0x30000, 0x10000, CRC(c6ba8a81) SHA1(e826492626707e30782d4d2f42419357970d67b3) )
	ROM_LOAD( "version_18.2_rom1_cs=2bf00d.u18", 0x48000, 0x08000, CRC(73bbc1c8) SHA1(9bb5067bf914b7c87a1ee29d6818de782fa28637) )
ROM_END


GAME( 1987, usg32,    0,        usg32,  usg32, driver_device, 0, ROT0, "U.S. Games", "Super Duper Casino (California V3.2)", MACHINE_SUPPORTS_SAVE )
GAME( 1988, superten, 0,        usg32,  usg83, driver_device, 0, ROT0, "U.S. Games", "Super Ten V8.3", MACHINE_SUPPORTS_SAVE )
GAME( 1988, usg83x,   superten, usg32,  usg83, driver_device, 0, ROT0, "U.S. Games", "Super Ten V8.3X", MACHINE_SUPPORTS_SAVE ) /* "Experimental" version?? */
GAME( 1988, usg82,    superten, usg32,  usg83, driver_device, 0, ROT0, "U.S. Games", "Super Ten V8.2" , MACHINE_SUPPORTS_SAVE )
GAME( 1992, usgames,  0,        usg185, usg83, driver_device, 0, ROT0, "U.S. Games", "Games V25.4X", MACHINE_SUPPORTS_SAVE )
GAME( 1991, usg187c,  usgames,  usg185, usg83, driver_device, 0, ROT0, "U.S. Games", "Games V18.7C", MACHINE_SUPPORTS_SAVE )
GAME( 1990, usg185,   usgames,  usg185, usg83, driver_device, 0, ROT0, "U.S. Games", "Games V18.5", MACHINE_SUPPORTS_SAVE )
GAME( 1989, usg182,   usgames,  usg185, usg83, driver_device, 0, ROT0, "U.S. Games", "Games V18.2", MACHINE_SUPPORTS_SAVE )
