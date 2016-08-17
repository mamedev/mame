// license:BSD-3-Clause
// copyright-holders:BUT
/*

Chack'n Pop driver by BUT

Note: The 68705 MCU isn't dumped, because it's protected, however we simulate
      it using data extracted with a trojan. See machine/chaknpop.c

Chack'n Pop
Taito 1983

PCB Layout
----------

Top board

J1000045A
K1000220A
 |------------------------|
 |                        |
 |             AO4_06.IC27|
 |                        |
 |                        |
 |                        |
 |                        |
 |                        |
 |                        |
 |                        |
 |                        |
|-|                       |
| |              Z80A     |
| |                       |
| | AO4_05.IC3   MSM2128  |
| |                       |
| |                       |
| |                       |
|-|                       |
 |                        |
 |------------------------|
Notes:
      AO4_06.IC23 - Motorola MC68705P5 Micro-controller. Clock 3.000MHz [18/6]
          MSM2128 - 2k x8 SRAM
              Z80 - Clock 3.000MHz [18/6]


Middle Board

J1000043A
K1000218A
M4200367A (sticker)
|-----------------------------------------------------|
| VOL  MB3731                                         |
|             MC14584                416  416        |-|
|                                    416  416        | |
|                    NE555           416  416        | |
|                                    416  416        | |
|    TD62003                         416  416        | |
|                                    416  416        | |
|2                                   416  416        | |
|2                                   416  416        |-|
|W                                                    |
|A                                                   |-|
|Y                                                   | |
|                        AO4_01.28                   | |
|    LM3900                                          | |
|         AY3-8910       AO4_02.27                   | |
|                    S                               | |
|         AY3-8910       AO4_03.26                   | |
|                                                    |-|
|DSWC  DSWA  DSWB        AO4_04.25                    |
|-----------------------------------------------------|
Notes:
           S - Flat cable connector joining to top PCB
         416 - NEC uPC416C 16k x1 DRAM
    AY3-8910 - Clock 1.500MHz [18/12]
       HSync - 15.1430kHz
       VSync - 59.1828Hz


Bottom Board

J1000044A
K1000219A
|-----------------------------------------------------|
|  AO4_07.IC15                                        |
|                                                    |-|
|  AO4_08.IC14                      2114             | |
|                 2114              2114             | |
|                                         AO4_09.IC98| |
|                 2114                               | |
|1                                        AO4_10.IC97| |
|8                                                   | |
|W                                        AO4-11.IC96|-|
|A                                        AO4-12.IC95 |
|Y                                                   |-|
|                                               18MHz| |
|                                                    | |
|                                                    | |
| HM2510     HM2510                                  | |
| HM2510     HM2510                                  | |
| HM2510     HM2510                                  | |
| HM2510     HM2510                                  |-|
| HM2510     HM2510                                   |
|-----------------------------------------------------|
Notes:
      HM2510 - Hitachi HM2510 1k x1 SRAM
        2114 - 1k x4 SRAM

*/

#include "emu.h"
#include "cpu/z80/z80.h"
#include "sound/ay8910.h"
#include "includes/chaknpop.h"

/***************************************************************************

  Memory Handler(s)

***************************************************************************/

WRITE8_MEMBER(chaknpop_state::unknown_port_1_w)
{
	//logerror("%s: write to unknow port 1: 0x%02x\n", machine().describe_context(), data);
}

WRITE8_MEMBER(chaknpop_state::unknown_port_2_w)
{
	//logerror("%s: write to unknow port 2: 0x%02x\n", machine().describe_context(), data);
}

WRITE8_MEMBER(chaknpop_state::coinlock_w)
{
	logerror("%04x: coin lock %sable\n", space.device().safe_pc(), data ? "dis" : "en");
}


/***************************************************************************

  Memory Map(s)

***************************************************************************/

static ADDRESS_MAP_START( chaknpop_map, AS_PROGRAM, 8, chaknpop_state )
	AM_RANGE(0x0000, 0x7fff) AM_ROM
	AM_RANGE(0x8000, 0x87ff) AM_RAM AM_SHARE("mcu_ram")
	AM_RANGE(0x8800, 0x8800) AM_READWRITE(mcu_port_a_r, mcu_port_a_w)
	AM_RANGE(0x8801, 0x8801) AM_READWRITE(mcu_port_b_r, mcu_port_b_w)
	AM_RANGE(0x8802, 0x8802) AM_READWRITE(mcu_port_c_r, mcu_port_c_w)
	AM_RANGE(0x8804, 0x8805) AM_DEVREADWRITE("ay1", ay8910_device, data_r, address_data_w)
	AM_RANGE(0x8806, 0x8807) AM_DEVREADWRITE("ay2", ay8910_device, data_r, address_data_w)
	AM_RANGE(0x8808, 0x8808) AM_READ_PORT("DSWC")
	AM_RANGE(0x8809, 0x8809) AM_READ_PORT("P1")
	AM_RANGE(0x880a, 0x880a) AM_READ_PORT("SYSTEM")
	AM_RANGE(0x880b, 0x880b) AM_READ_PORT("P2")
	AM_RANGE(0x880c, 0x880c) AM_READWRITE(gfxmode_r, gfxmode_w)
	AM_RANGE(0x880d, 0x880d) AM_WRITE(coinlock_w)                                               // coin lock out
	AM_RANGE(0x9000, 0x93ff) AM_RAM_WRITE(txram_w) AM_SHARE("tx_ram")          // TX tilemap
	AM_RANGE(0x9800, 0x983f) AM_RAM_WRITE(attrram_w) AM_SHARE("attr_ram")      // Color attribute
	AM_RANGE(0x9840, 0x98ff) AM_RAM AM_SHARE("spr_ram") // sprite
	AM_RANGE(0xa000, 0xbfff) AM_ROM
	AM_RANGE(0xc000, 0xffff) AM_RAMBANK("bank1")                                                        // bitmap plane 1-4
ADDRESS_MAP_END

/***************************************************************************

  Input Port(s)

***************************************************************************/

static INPUT_PORTS_START( chaknpop )
	PORT_START("SYSTEM")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_COIN1 )  // LEFT COIN
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_COIN2 )  // RIGHT COIN
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_TILT )

	PORT_START("P1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(1)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("P2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("DSWC")
	PORT_DIPNAME( 0x01, 0x01, DEF_STR( Language ) )     PORT_DIPLOCATION("SWC:1")
	PORT_DIPSETTING(    0x00, DEF_STR( English ) )
	PORT_DIPSETTING(    0x01, DEF_STR( Japanese ) )
	PORT_DIPNAME( 0x02, 0x00, DEF_STR( Demo_Sounds ) )  PORT_DIPLOCATION("SWC:2")
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, "Super Chack'n" )     PORT_DIPLOCATION("SWC:3")
	PORT_DIPSETTING(    0x04, "pi" )
	PORT_DIPSETTING(    0x00, "1st Chance" )
	PORT_DIPNAME( 0x08, 0x08, "Endless (Cheat)")        PORT_DIPLOCATION("SWC:4")
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, "Credit Info" )       PORT_DIPLOCATION("SWC:5")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x10, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, "Show Year" )         PORT_DIPLOCATION("SWC:6")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x20, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, "Infinite (Cheat)")       PORT_DIPLOCATION("SWC:7")
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x00, DEF_STR( Coinage ) )      PORT_DIPLOCATION("SWC:8")
	PORT_DIPSETTING(    0x00, "1 Way" )
	PORT_DIPSETTING(    0x80, "2 Way" )

	PORT_START("DSWB")
	PORT_DIPNAME( 0x03, 0x03, DEF_STR( Bonus_Life ) )   PORT_DIPLOCATION("SWB:1,2")
	PORT_DIPSETTING(    0x00, "80k and every 100k" )
	PORT_DIPSETTING(    0x01, "60k and every 100k" )
	PORT_DIPSETTING(    0x02, "40k and every 100k" )
	PORT_DIPSETTING(    0x03, "20k and every 100k" )
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Free_Play ) )    PORT_DIPLOCATION("SWB:3")
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x18, 0x08, DEF_STR( Lives ) )        PORT_DIPLOCATION("SWB:4,5")
	PORT_DIPSETTING(    0x00, "6" )
	PORT_DIPSETTING(    0x08, "3" )
	PORT_DIPSETTING(    0x10, "2" )
	PORT_DIPSETTING(    0x18, "1" )
	PORT_DIPNAME( 0x20, 0x00, "Training/Difficulty" )   PORT_DIPLOCATION("SWB:6")
	PORT_DIPSETTING(    0x20, "Off/Every 10 Min." )
	PORT_DIPSETTING(    0x00, "On/Every 7 Min." )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Flip_Screen ) )  PORT_DIPLOCATION("SWB:7")
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x00, DEF_STR( Cabinet ) )      PORT_DIPLOCATION("SWB:8")
	PORT_DIPSETTING(    0x00, DEF_STR( Upright ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Cocktail ) )

	PORT_START("DSWA")
	PORT_DIPNAME(0x0f,  0x00, DEF_STR( Coin_A ) )       PORT_DIPLOCATION("SWA:1,2,3,4")
	PORT_DIPSETTING(    0x0f, DEF_STR( 9C_1C ) )
	PORT_DIPSETTING(    0x0e, DEF_STR( 8C_1C ) )
	PORT_DIPSETTING(    0x0d, DEF_STR( 7C_1C ) )
	PORT_DIPSETTING(    0x0c, DEF_STR( 6C_1C ) )
	PORT_DIPSETTING(    0x0b, DEF_STR( 5C_1C ) )
	PORT_DIPSETTING(    0x0a, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(    0x09, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x08, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x01, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x02, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0x03, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(    0x04, DEF_STR( 1C_5C ) )
	PORT_DIPSETTING(    0x05, DEF_STR( 1C_6C ) )
	PORT_DIPSETTING(    0x06, DEF_STR( 1C_7C ) )
	PORT_DIPSETTING(    0x07, DEF_STR( 1C_8C ) )
	PORT_DIPNAME(0xf0,  0x00, DEF_STR( Coin_B ) )       PORT_DIPLOCATION("SWA:5,6,7,8")
	PORT_DIPSETTING(    0xf0, DEF_STR( 9C_1C ) )
	PORT_DIPSETTING(    0xe0, DEF_STR( 8C_1C ) )
	PORT_DIPSETTING(    0xd0, DEF_STR( 7C_1C ) )
	PORT_DIPSETTING(    0xc0, DEF_STR( 6C_1C ) )
	PORT_DIPSETTING(    0xb0, DEF_STR( 5C_1C ) )
	PORT_DIPSETTING(    0xa0, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(    0x90, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x80, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x10, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x20, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0x30, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(    0x40, DEF_STR( 1C_5C ) )
	PORT_DIPSETTING(    0x50, DEF_STR( 1C_6C ) )
	PORT_DIPSETTING(    0x60, DEF_STR( 1C_7C ) )
	PORT_DIPSETTING(    0x70, DEF_STR( 1C_8C ) )
INPUT_PORTS_END


/***************************************************************************

  Machine Driver(s)

***************************************************************************/

static const gfx_layout spritelayout =
{
	16,16,  /* 16*16 characters */
	256,    /* 256 characters */
	2,  /* 2 bits per pixel */
	{ 0, 0x2000*8 },    /* the two bitplanes are separated */
	{ 0, 1, 2, 3, 4, 5, 6, 7 ,
			8*8+0, 8*8+1, 8*8+2, 8*8+3, 8*8+4, 8*8+5, 8*8+6, 8*8+7 },
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8,
			16*8, 17*8, 18*8, 19*8, 20*8, 21*8, 22*8, 23*8 },
	32*8    /* every sprite takes 32 consecutive bytes */
};

static const gfx_layout charlayout =
{
	8,8,    /* 8*8 characters */
	1024,   /* 1024 characters */
	2,  /* 2 bits per pixel */
	{ 0, 0x2000*8 },    /* the two bitplanes are separated */
	{ 0, 1, 2, 3, 4, 5, 6, 7 },
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8 },
	8*8 /* every char takes 8 consecutive bytes */
};

static GFXDECODE_START( chaknpop )
	GFXDECODE_ENTRY( "gfx1", 0, spritelayout, 0,  8 )
	GFXDECODE_ENTRY( "gfx2", 0, charlayout,   32, 8 )
GFXDECODE_END


void chaknpop_state::machine_start()
{
	UINT8 *ROM = memregion("maincpu")->base();

	membank("bank1")->configure_entries(0, 2, &ROM[0x10000], 0x4000);

	save_item(NAME(m_gfxmode));
	save_item(NAME(m_flip_x));
	save_item(NAME(m_flip_y));

	save_item(NAME(m_mcu_seed));
	save_item(NAME(m_mcu_result));
	save_item(NAME(m_mcu_select));
}

void chaknpop_state::machine_reset()
{
	m_gfxmode = 0;
	m_flip_x = 0;
	m_flip_y = 0;

	m_mcu_seed = MCU_INITIAL_SEED;
	m_mcu_result = 0;
	m_mcu_select = 0;
}

static MACHINE_CONFIG_START( chaknpop, chaknpop_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", Z80, XTAL_18MHz / 6)    /* Verified on PCB */
	MCFG_CPU_PROGRAM_MAP(chaknpop_map)
	MCFG_CPU_VBLANK_INT_DRIVER("screen", chaknpop_state,  irq0_line_hold)


	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(59.1828)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MCFG_SCREEN_SIZE(32*8, 32*8)
	MCFG_SCREEN_VISIBLE_AREA(0*8, 32*8-1, 2*8, 30*8-1)
	MCFG_SCREEN_UPDATE_DRIVER(chaknpop_state, screen_update)
	MCFG_SCREEN_PALETTE("palette")

	MCFG_GFXDECODE_ADD("gfxdecode", "palette", chaknpop)
	MCFG_PALETTE_ADD("palette", 1024)
	MCFG_PALETTE_INIT_OWNER(chaknpop_state, chaknpop)

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")

	MCFG_SOUND_ADD("ay1", AY8910, XTAL_18MHz / 12)  /* Verified on PCB */
	MCFG_AY8910_PORT_A_READ_CB(IOPORT("DSWA"))
	MCFG_AY8910_PORT_B_READ_CB(IOPORT("DSWB"))
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.15)

	MCFG_SOUND_ADD("ay2", AY8910, XTAL_18MHz / 12)  /* Verified on PCB */
	MCFG_AY8910_PORT_A_WRITE_CB(WRITE8(chaknpop_state, unknown_port_1_w))   // ??
	MCFG_AY8910_PORT_B_WRITE_CB(WRITE8(chaknpop_state, unknown_port_2_w))    // ??
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.10)
MACHINE_CONFIG_END


/***************************************************************************

  Game driver(s)

***************************************************************************/

ROM_START( chaknpop )
	ROM_REGION( 0x18000, "maincpu", 0 )         /* Main CPU */
	ROM_LOAD( "ao4_01.ic28", 0x00000, 0x2000, CRC(386fe1c8) SHA1(cca24abfb8a7f439251e7936036475c694002561) )
	ROM_LOAD( "ao4_02.ic27", 0x02000, 0x2000, CRC(5562a6a7) SHA1(0c5d81f9aaf858f88007a6bca7f83dc3ef59c5b5) )
	ROM_LOAD( "ao4_03.ic26", 0x04000, 0x2000, CRC(3e2f0a9c) SHA1(f1cf87a4cb07f77104d4a4d369807dac522e052c) )
	ROM_LOAD( "ao4_04.ic25", 0x06000, 0x2000, CRC(5209c7d4) SHA1(dcba785a697df55d84d65735de38365869a1da9d) )
	ROM_LOAD( "ao4_05.ic3",  0x0a000, 0x2000, CRC(8720e024) SHA1(99e445c117d1501a245f9eb8d014abc4712b4963) )

	ROM_REGION( 0x0800, "mcu", 0 )  /* 2k for the Motorola MC68705P5 Micro-controller */
	ROM_LOAD( "ao4_06.ic23", 0x0000, 0x0800, NO_DUMP )

	ROM_REGION( 0x4000, "gfx1", 0 ) /* Sprite */
	ROM_LOAD( "ao4_08.ic14", 0x0000, 0x2000, CRC(5575a021) SHA1(c2fad53fe6a12c19cec69d27c13fce6aea2502f2) )
	ROM_LOAD( "ao4_07.ic15", 0x2000, 0x2000, CRC(ae687c18) SHA1(65b25263da88d30cbc0dad94511869596e5c975a) )

	ROM_REGION( 0x4000, "gfx2", 0 ) /* Text */
	ROM_LOAD( "ao4_09.ic98", 0x0000, 0x2000, CRC(757a723a) SHA1(62ab84d2aaa9bc1ea5aa9df8155aa3b5a1e93889) )
	ROM_LOAD( "ao4_10.ic97", 0x2000, 0x2000, CRC(3e3fd608) SHA1(053a8fbdb35bf1c142349f78a63e8cd1adb41ef6) )

	ROM_REGION( 0x0800, "proms", 0 )            /* Palette */
	ROM_LOAD( "ao4-11.ic96", 0x0000, 0x0400, CRC(9bf0e85f) SHA1(44f0a4712c99a715dec54060afb0b27dc48998b4) )
	ROM_LOAD( "ao4-12.ic95", 0x0400, 0x0400, CRC(954ce8fc) SHA1(e187f9e2cb754264d149c2896ca949dea3bcf2eb) )
ROM_END


/*  ( YEAR  NAME      PARENT    MACHINE   INPUT     INIT      MONITOR  COMPANY              FULLNAME ) */
GAME( 1983, chaknpop, 0,        chaknpop, chaknpop, driver_device, 0,        ROT0,    "Taito Corporation", "Chack'n Pop", MACHINE_SUPPORTS_SAVE )
