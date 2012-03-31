/* Puckman Pockimon
  -- original driver by Luca Elia

Seems to be based around genesis hardware, despite containing no original Sega chips

Supported:

Puckman Pockimon - (c)2000 Genie? (there should be a way to show Sun Mixing copyright, roms are the same
                                   on a version with the SM (c)

|---------------------------------------|
| VOL    4558    4MHz   PAL     62256   |
| YM3812 YM3014                         |
| 3.579545MHz    555    PAL     |------||
| LM324     M6295        |----| |TV16B ||
|          ROM.U3   PAL  |YBOX| |      ||
|J   PAL                 |----| |      ||
|A         PAL      PAL         |------||
|M         PAL      PAL  PAL            |
|M         PAL      PAL     53.693175MHz|
|A   DSW2                               |
|          PAL      PAL    |------|     |
|    DSW1                  |TA06SD|     |
|          ROM.U5   ROM.U4 |      |     |
|                          |------|     |
|          ROM.U8   ROM.U7 62256  D41264|
|          *        *      62256  D41264|
|---------------------------------------|
Notes:
      Main CPU is 68000-based, but actual CPU chip is not known
      Master clock 53.693175MHz. CPU likely running at 53.693175/7 or /6 (??)
      YM3812 clock 3.579545MHz
      M6295 clock 1.000MHz (4/4]. Sample rate = 1000000/132
      VSync 60Hz
      HSync 16.24kHz
      62256 - 8k x8 SRAM (DIP28)
      D41264 - NEC D41264V-15V 64k x4 VRAM (ZIP24)
      * Unpopulated DIP32 socket
      Custom ICs -
                  Y-BOX TA891945 (QFP100)
                  TA-06SD 9933 B816453 (QFP128)
                  TV16B 0010 ME251271 (QFP160)
*/

#include "emu.h"
#include "cpu/m68000/m68000.h"
#include "sound/okim6295.h"
#include "sound/sn76496.h"
#include "sound/2612intf.h"

#include "includes/megadriv.h"


/* Puckman Pockimon Input Ports */
static INPUT_PORTS_START( puckpkmn )
	PORT_START("P2")	/* $700011.b */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_PLAYER(2)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_PLAYER(2)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_PLAYER(2)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(2)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)

	PORT_START("P1")	/* $700013.b */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_COIN1 ) PORT_IMPULSE(10)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_PLAYER(1)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_PLAYER(1)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_PLAYER(1)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(1)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1)

	PORT_START("UNK")	/* $700015.b */

	PORT_START("DSW1")	/* $700017.b */
	PORT_DIPNAME( 0x07, 0x07, DEF_STR( Coinage ) )
	PORT_DIPSETTING(    0x03, DEF_STR( 5C_1C ) )
	PORT_DIPSETTING(    0x04, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(    0x05, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x06, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x07, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x02, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x01, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_4C ) )
	PORT_DIPNAME( 0x38, 0x28, DEF_STR( Lives ) )
	PORT_DIPSETTING(    0x38, "1" )
	PORT_DIPSETTING(    0x30, "2" )
	PORT_DIPSETTING(    0x28, "3" )
	PORT_DIPSETTING(    0x20, "4" )
	PORT_DIPSETTING(    0x18, "5" )
	PORT_DIPSETTING(    0x10, "6" )
	PORT_DIPSETTING(    0x08, "7" )
	PORT_DIPSETTING(    0x00, "8" )
	PORT_DIPNAME( 0xc0, 0x80, DEF_STR( Difficulty ) )
	PORT_DIPSETTING(    0xc0, DEF_STR( Easy ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Normal ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Hard ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Hardest ) )

	PORT_START("DSW2")	/* $700019.b */
	PORT_SERVICE( 0x01, IP_ACTIVE_LOW )
	PORT_DIPNAME( 0x02, 0x02, DEF_STR( Demo_Sounds ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x02, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Unused ) )
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Unused ) )
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Unused ) )
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Unused ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Unused ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unused ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
INPUT_PORTS_END


static ADDRESS_MAP_START( puckpkmn_map, AS_PROGRAM, 16, md_boot_state )
	AM_RANGE(0x000000, 0x1fffff) AM_ROM								/* Main 68k Program Roms */
	AM_RANGE(0x700010, 0x700011) AM_READ_PORT("P2")
	AM_RANGE(0x700012, 0x700013) AM_READ_PORT("P1")
	AM_RANGE(0x700014, 0x700015) AM_READ_PORT("UNK")
	AM_RANGE(0x700016, 0x700017) AM_READ_PORT("DSW1")
	AM_RANGE(0x700018, 0x700019) AM_READ_PORT("DSW2")
	AM_RANGE(0x700022, 0x700023) AM_DEVREADWRITE8_MODERN("oki", okim6295_device, read, write, 0x00ff)
	AM_RANGE(0xa04000, 0xa04003) AM_DEVREADWRITE8("ymsnd", megadriv_68k_YM2612_read, megadriv_68k_YM2612_write, 0xffff)
	AM_RANGE(0xc00000, 0xc0001f) AM_READWRITE(megadriv_vdp_r, megadriv_vdp_w)
	AM_RANGE(0xe00000, 0xe0ffff) AM_RAM AM_MIRROR(0x1f0000) AM_BASE(&megadrive_ram)

	/* Unknown reads/writes: */
	AM_RANGE(0xa00000, 0xa00551) AM_WRITENOP							/* ? */
//  AM_RANGE(0xa10000, 0xa10001) AM_READNOP                                             /* ? once */
	AM_RANGE(0xa10002, 0xa10005) AM_NOP								/* ? alternative way of reading inputs ? */
	AM_RANGE(0xa11100, 0xa11101) AM_NOP								/* ? */
//  AM_RANGE(0xa10008, 0xa1000d) AM_WRITENOP                                            /* ? once */
//  AM_RANGE(0xa14000, 0xa14003) AM_WRITENOP                                            /* ? once */
	AM_RANGE(0xa11200, 0xa11201) AM_WRITENOP							/* ? */
ADDRESS_MAP_END

static READ16_HANDLER(puckpkmna_70001c_r)
{
	return 0x0e;
}

static READ16_HANDLER(puckpkmna_4b2476_r)
{
	return 0x3400;
}

static ADDRESS_MAP_START( puckpkmna_map, AS_PROGRAM, 16, md_boot_state )
	AM_IMPORT_FROM( puckpkmn_map )
	AM_RANGE(0x4b2476, 0x4b2477) AM_READ(puckpkmna_4b2476_r)
	AM_RANGE(0x70001c, 0x70001d) AM_READ(puckpkmna_70001c_r)
ADDRESS_MAP_END

static MACHINE_CONFIG_START( puckpkmn, md_boot_state )
	MCFG_FRAGMENT_ADD(md_ntsc)

	MCFG_CPU_MODIFY("maincpu")
	MCFG_CPU_PROGRAM_MAP(puckpkmn_map)

	MCFG_DEVICE_REMOVE("genesis_snd_z80")

	MCFG_OKIM6295_ADD("oki", 1056000, OKIM6295_PIN7_HIGH) // clock frequency & pin 7 not verified
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "lspeaker", 0.25)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "rspeaker",0.25)
MACHINE_CONFIG_END

static MACHINE_CONFIG_DERIVED( puckpkmna, puckpkmn )

	MCFG_CPU_MODIFY("maincpu")
	MCFG_CPU_PROGRAM_MAP(puckpkmna_map)

MACHINE_CONFIG_END

/* Genie's Hardware (contains no real sega parts) */

/***************************************************************************
Puckman Pokemon Genie 2000
(c) 2000?  Manufacturer ?

Hardware looks bootleg-ish, but is newly manufactured.

CPU: ? (one of the SMD chips)
SND: OKI6295, U6612 (probably YM3812), U6614B (Probably YM3014B)
XTAL: 3.579545MHz, 4.0000MHz
OSC: 53.693175MHz
Other Chips: Possible CPU: TA-06SD 9933 B816453 128 pin square SMD
             GFX support chips: Y-BOX TA891945 100 pin SMD
                                TV16B 0010 ME251271 160 pin SMD

There are 13 PAL's on the PCB !

RAM: 62256 x 3, D41264 x 2 (ZIP Ram)
DIPS: 2 x 8 position
SW1:
                        1   2   3   4   5   6   7   8
coins   1coin 1 Cred.   off off off
        2c 1c           on  off off
        3c 1c           off on  off
        4c 1c           on  on  off
        5c 1c           off off on
        1c 2c           on  off on
        1c 3c           off on  on
        1c 4c           on  on  on

players 1                           off off off
        2                           on  off off
        3                           off on  off
        4                           on  on  off
        5                           off off on
        6                           on  off on
        7                           off on  on
        8                           on  on  on

diffic-
ulty    v.easy                                  off off
        normal                                  on  off
        diffic.                                 off on
        v. diffic.                              on  on


SW2

note position 3-8 not used

                    1   2   3   4   5   6   7   8
test mode   no      off
            yes     on

demo sound  yes         off
            no          on


ROMS:
PUCKPOKE.U3 M5M27C201   Sound
PUCKPOKE.U4 27C040--\
PUCKPOKE.U5 27C040---\
PUCKPOKE.U7 27C040----- Main program & GFX
PUCKPOKE.U8 27C4001---/

ROM sockets U63 & U64 empty

Hi-res PCB scan available on request.
Screenshots available on my site at http://guru.mameworld.info/oldnews2001.html (under PCB Shop Raid #2)


****************************************************************************/

static DRIVER_INIT( puckpkmn )
{
	UINT8 *rom = machine.region("maincpu")->base();
	size_t len = machine.region("maincpu")->bytes();
	int i;

	for (i = 0; i < len; i++)
		rom[i] = BITSWAP8(rom[i],1,4,2,0,7,5,3,6);

	DRIVER_INIT_CALL(megadriv);
}

ROM_START( puckpkmn ) /* Puckman Pockimon  (c)2000 Genie */
	ROM_REGION( 0x200000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "puckpoke.u5", 0x000000, 0x080000, CRC(fd334b91) SHA1(cf8bf6645a4082ea4392937e169b1686c9c7e246) )
	ROM_LOAD16_BYTE( "puckpoke.u4", 0x000001, 0x080000, CRC(839cc76b) SHA1(e15662a7175db7a8e222dda176a8ed92e0d56e9d) )
	ROM_LOAD16_BYTE( "puckpoke.u8", 0x100000, 0x080000, CRC(7936bec8) SHA1(4b350105abe514fbfeabae1c6f3aeee695c3d07a) )
	ROM_LOAD16_BYTE( "puckpoke.u7", 0x100001, 0x080000, CRC(96b66bdf) SHA1(3cc2861ad9bc232cbe683e01b58090f832d03db5) )

	ROM_REGION( 0x40000, "oki", 0 )
	ROM_LOAD( "puckpoke.u3", 0x00000, 0x40000, CRC(7b066bac) SHA1(429616e21c672b07e0705bc63234249cac3af56f) )
ROM_END

/*
Puckman Pokimon (alt.)


PCB Layout
----------

|------------------------------------------------|
|UPC1241                      4.000MHz     6264  |
|    VOL             6295           555          |
|                                                |
|                                                |
|  LM324                                TV16B    |
|                    A.U3                        |
|                                                |
|                                                |
|J                                               |
|A                                   59.693175MHz|
|M                                               |
|M      DSW2(8)                                  |
|A                                               |
|                    62256    62256              |
|       DSW1(8)      PAL      PAL                |
|                                                |
|                         B.U59          TA-06S  |
|  PAL               TK-20K   PAL                |
|                             PAL       MB81461  |
|  PAL     PAL                          MB81461  |
|------------------------------------------------|
Notes:
      TV16B  - custom graphics chip (QFP160)
      TA-06S - custom chip (QFP128)
      TK-20K - custom chip, probably the CPU (QFP100). Clock unknown.
      M6295  - clock 1.000MHz [4/4]

      4x 1Mx8 flashROMs (B*.U59) are mounted onto a DIP42 carrier board to make a
      32MBit EPROM equivalent. It appears to contain graphics plus the main program.
      ROM A.U3 contains samples for the M6295.

*/

ROM_START( puckpkmna ) /* Puckman Pockimon  (c)2000 IBS Co. Ltd */
	ROM_REGION( 0x200000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "b2.u59", 0x000000, 0x080000, CRC(3fbea2c7) SHA1(89f3770ae92c62714f0795ddd2f311a9532eb25a) ) // FIRST AND SECOND HALF IDENTICAL
	ROM_IGNORE(0x080000)
	ROM_LOAD16_BYTE( "b1.u59", 0x000001, 0x080000, CRC(dc7b4254) SHA1(8ba5c5e8123e62e9af091971d0d0401d2df49350) ) // FIRST AND SECOND HALF IDENTICAL
	ROM_IGNORE(0x080000)
	ROM_LOAD16_BYTE( "b4.u59", 0x100000, 0x080000, CRC(375c9f80) SHA1(9b0eb729e95c22355e4117eec596f90e10282492) ) // FIRST AND SECOND HALF IDENTICAL
	ROM_IGNORE(0x080000)
	ROM_LOAD16_BYTE( "b3.u59", 0x100001, 0x080000, CRC(d5487df6) SHA1(d1d3d717e184a4e8e067665bbbe94e7cf45db478) ) // FIRST AND SECOND HALF IDENTICAL
	ROM_IGNORE(0x080000)

	ROM_REGION( 0x80000, "oki", 0 )
	ROM_LOAD( "a.u3", 0x00000, 0x80000, CRC(77891c9b) SHA1(66f28b418a480a89ddb3fae3a7c2fe702c62364c) )
ROM_END

/* Genie Hardware (uses Genesis VDP) also has 'Sun Mixing Co' put into tile ram */
GAME( 2000, puckpkmn, 0,        puckpkmn,  puckpkmn, puckpkmn, ROT0, "Genie",                  "Puckman Pockimon (set 1)", 0 )
GAME( 2000, puckpkmna,puckpkmn, puckpkmna, puckpkmn, puckpkmn, ROT0, "IBS",                    "Puckman Pockimon (set 2)", 0 )
