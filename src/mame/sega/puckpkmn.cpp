// license:BSD-3-Clause
// copyright-holders:Luca Elia, David Haywood
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
#include "sound/ymopn.h"

#include "megadriv.h"
#include "megadriv_acbl.h"

/* Puckman Pockimon Input Ports */
static INPUT_PORTS_START( puckpkmn )
	PORT_START("P2")    /* $700011.b */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_PLAYER(2)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_PLAYER(2)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_PLAYER(2)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(2)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)

	PORT_START("P1")    /* $700013.b */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_COIN1 ) PORT_IMPULSE(10)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_PLAYER(1)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_PLAYER(1)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_PLAYER(1)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(1)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1)

	PORT_START("UNK")   /* $700015.b */

	PORT_START("DSW1")  /* $700017.b */
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

	PORT_START("DSW2")  /* $700019.b */
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




static INPUT_PORTS_START( jzth )
	PORT_START("P2")    /* $700011.b */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_PLAYER(2)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_PLAYER(2)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_PLAYER(2)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(2)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(2)

	PORT_START("P1")    /* $700013.b */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_COIN1 ) PORT_IMPULSE(10)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_PLAYER(1)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_PLAYER(1)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_PLAYER(1)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(1)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(1)

	PORT_START("UNK")   /* $700015.b */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(1)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )


	PORT_START("DSW1")  /* $700017.b */
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

	PORT_START("DSW2")  /* $700019.b */
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


void md_boot_state::puckpkmn_map(address_map &map)
{
	map(0x000000, 0x3fffff).rom();                             /* Main 68k Program Roms */
	map(0x700010, 0x700011).portr("P2");
	map(0x700012, 0x700013).portr("P1");
	map(0x700014, 0x700015).portr("UNK");
	map(0x700016, 0x700017).portr("DSW1");
	map(0x700018, 0x700019).portr("DSW2");
	map(0x700023, 0x700023).rw("oki", FUNC(okim6295_device::read), FUNC(okim6295_device::write));
	map(0xa04000, 0xa04003).rw(FUNC(md_boot_state::megadriv_68k_YM2612_read), FUNC(md_boot_state::megadriv_68k_YM2612_write));
	map(0xc00000, 0xc0001f).rw(m_vdp, FUNC(sega315_5313_device::vdp_r), FUNC(sega315_5313_device::vdp_w));

	map(0xe00000, 0xe0ffff).ram().mirror(0x1f0000);

	/* Unknown reads/writes: */
	map(0xa00000, 0xa00551).nopw();                            /* ? */
//  map(0xa10000, 0xa10001).nopr();                                             /* ? once */
	map(0xa10002, 0xa10005).noprw();                             /* ? alternative way of reading inputs ? */
	map(0xa11100, 0xa11101).noprw();                             /* ? */
//  map(0xa10008, 0xa1000d).nopw();                                            /* ? once */
//  map(0xa14000, 0xa14003).nopw();                                            /* ? once */
	map(0xa11200, 0xa11201).nopw();                            /* ? */
}


void md_boot_state::jzth_map(address_map &map)
{
	map(0x000000, 0x3fffff).rom();
	map(0x700010, 0x700011).portr("P2");
	map(0x700012, 0x700013).portr("P1");
	map(0x700014, 0x700015).portr("UNK");
	map(0x700016, 0x700017).portr("DSW1");
	map(0x700018, 0x700019).portr("DSW2");
	map(0x700023, 0x700023).rw("oki", FUNC(okim6295_device::read), FUNC(okim6295_device::write));
	map(0xa04000, 0xa04003).rw(FUNC(md_boot_state::megadriv_68k_YM2612_read), FUNC(md_boot_state::megadriv_68k_YM2612_write));
	map(0xc00000, 0xc0001f).rw(m_vdp, FUNC(sega315_5313_device::vdp_r), FUNC(sega315_5313_device::vdp_w));

	map(0xe00000, 0xe0ffff).ram().mirror(0x1f0000);

	map(0xa00000, 0xa00551).noprw();

	map(0xA11100, 0xA11101).noprw();

	map(0x710000, 0x710001).rw(FUNC(md_boot_state::bl_710000_r), FUNC(md_boot_state::bl_710000_w)); // protection, will erase the VDP address causing writes to 0 unless this returns 0xe
}

uint16_t md_boot_state::puckpkmna_70001c_r()
{
	return 0x0e;
}

uint16_t md_boot_state::puckpkmna_4b2476_r()
{
	if (!strcmp(machine().system().name, "puckpkmnb")) return 0x3100;

	return 0x3400;
}

void md_boot_state::puckpkmna_map(address_map &map)
{
	puckpkmn_map(map);
	map(0x4b2476, 0x4b2477).r(FUNC(md_boot_state::puckpkmna_4b2476_r));
	map(0x70001c, 0x70001d).r(FUNC(md_boot_state::puckpkmna_70001c_r));
}

void md_boot_state::puckpkmn(machine_config &config)
{
	md_ntsc(config);

	m_maincpu->set_addrmap(AS_PROGRAM, &md_boot_state::puckpkmn_map);

	config.device_remove("genesis_snd_z80");

	okim6295_device &oki(OKIM6295(config, "oki", XTAL(4'000'000) / 4, okim6295_device::PIN7_HIGH));
	oki.add_route(ALL_OUTPUTS, "lspeaker", 0.25);
	oki.add_route(ALL_OUTPUTS, "rspeaker", 0.25);
}

void md_boot_state::puckpkmna(machine_config &config)
{
	puckpkmn(config);

	m_maincpu->set_addrmap(AS_PROGRAM, &md_boot_state::puckpkmna_map);
}

void md_boot_state::jzth(machine_config &config)
{
	puckpkmn(config);

	m_maincpu->set_addrmap(AS_PROGRAM, &md_boot_state::jzth_map);
}

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


****************************************************************************/

void md_boot_state::init_puckpkmn()
{
	uint8_t *rom = memregion("maincpu")->base();
	const size_t len = memregion("maincpu")->bytes();

	for (size_t i = 0; i < len; i++)
		rom[i] = bitswap<8>(rom[i],1,4,2,0,7,5,3,6);

	init_megadriv();
}

ROM_START( puckpkmn ) /* Puckman Pockimon  (c)2000 Genie */
	ROM_REGION( 0x400000, "maincpu", 0 )
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
	ROM_REGION( 0x400000, "maincpu", 0 )
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


ROM_START( puckpkmnb )
	ROM_REGION( 0x400000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "200061.u5", 0x000000, 0x080000, CRC(502a5093) SHA1(6dc1c79d52ebb653cb2e4388f74fd975ec323566) )
	ROM_LOAD16_BYTE( "200060.u4", 0x000001, 0x080000, CRC(5f160c18) SHA1(5a5ce1b9a81afe836e435e9d6f16cf57b63cbd31) )
	ROM_LOAD16_BYTE( "200063.u8", 0x100000, 0x080000, CRC(0c29781e) SHA1(db442f9b588608b2ac04d65fd830103296599a6a) )
	ROM_LOAD16_BYTE( "200062.u7", 0x100001, 0x080000, CRC(00bbf9a9) SHA1(924c1ed85090c497ce89528082c15d1548a854a0) )

	ROM_REGION( 0x40000, "oki", 0 )
	ROM_LOAD( "206295.u3", 0x00000, 0x40000, CRC(7b066bac) SHA1(429616e21c672b07e0705bc63234249cac3af56f) )
ROM_END


//決戰天皇/Juézhàn tiānhuáng (Traditional Chinese)
ROM_START( jzth )
	ROM_REGION( 0x400000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "s.y.u5", 0x000000, 0x080000, CRC(a4a526b5) SHA1(85d0299caf91ff50b6870f845b9aacbd358ed81f) )
	ROM_LOAD16_BYTE( "s.y.u4", 0x000001, 0x080000, CRC(c16654eb) SHA1(dca4b772a3b9caa7be3fa01511c401b591c2e6f3) )
	ROM_LOAD16_BYTE( "s.y.u8", 0x100000, 0x080000, CRC(b62e1068) SHA1(2484ae49a4a2a2c551b3b84bbc0b4e40e5d281e7) )
	ROM_LOAD16_BYTE( "s.y.u7", 0x100001, 0x080000, CRC(27fe424c) SHA1(14bee8c16aac3d5b04123c994167531f817634fd) )
	ROM_LOAD16_BYTE( "s.y.u64", 0x200000, 0x080000, CRC(62f52886) SHA1(07fc9765274c03eff4a09f48a0b1b2b2afc6078e) )
	ROM_LOAD16_BYTE( "s.y.u63", 0x200001, 0x080000, CRC(a6a32c8c) SHA1(d0c779751e4af459e9bf63e55c5e2b19a243b70d) )
	ROM_LOAD16_BYTE( "s.y.u66", 0x300000, 0x080000, CRC(fa4a09f5) SHA1(67d77c91a994ecb8b29e7661c3a12e84a64eb837))
	ROM_LOAD16_BYTE( "s.y.u65", 0x300001, 0x080000, CRC(de64e526) SHA1(e3b3e5c95b8ae36c0c57f8c9a6f55084464c4c05) )

	ROM_REGION( 0x80000, "oki", 0 ) // there are 2 banks in here, so find bank switch
	ROM_LOAD( "s.y.u3", 0x00000, 0x40000, CRC(38eef2f2) SHA1(2f750dbf71fea0622e8493f0a8be7c43555ed5cf) )
	ROM_CONTINUE(0x40000,0x40000)
ROM_END

/* Genie Hardware (uses Genesis VDP) also has 'Sun Mixing Co' put into tile ram */  // is 'Genie 2000' part of the title, and the parent set a bootleg?
GAME( 2000, puckpkmn,  0,        puckpkmn,  puckpkmn, md_boot_state, init_puckpkmn, ROT0, "Genie",                  "Puckman Pockimon (set 1)", 0 )
GAME( 2000, puckpkmna, puckpkmn, puckpkmna, puckpkmn, md_boot_state, init_puckpkmn, ROT0, "IBS",                    "Puckman Pockimon (set 2)", 0 )
GAME( 2000, puckpkmnb, puckpkmn, puckpkmna, puckpkmn, md_boot_state, init_puckpkmn, ROT0, "Sun Mixing",             "Puckman Pockimon (set 3)", 0 )
GAME( 2000, jzth,      0,        jzth,      jzth,     md_boot_state, init_puckpkmn, ROT0, "<unknown>",              "Juezhan Tianhuang", MACHINE_IMPERFECT_SOUND )
