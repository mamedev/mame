// license:???
// copyright-holders:insideoutboy, David Haywood, Stephh
/*

Flower (c)1986 Komax (USA license)
       (c)1986 Sega/Alpha (Sega game number 834-5998)

 - Driver by InsideOutBoy, further improvements by MAME team

There is a PCB picture that shows two stickers, the first says
 "Flower (c) 1986 Clarue" while the second one is an original
 serial number tag also showing "Clarue". GFX ROM contents also
 show (C) 1986 CLARUE. A Wood Place Inc. spinoff perhaps?


todo:

fix sound and timing


        FLOWER   CHIP PLACEMENT

XTAL: 18.4320 MHz
USES THREE Z80A CPU'S

CHIP #  POSITION   TYPE
------------------------
1        5J         27256   CONN BD
2        5F         27256    "
3        D9         27128    "
4        12A        27128    "
5        16A        27256    "
6        7E         2764    BOTTOM BD
15       9E          "       "
8        10E         "       "
9        12E         "       "
10       13E         "       "
11       14E         "       "
12       16E         "       "
13       17E         "       "
14       19E         "       "

                Upright or Cocktail cabinet
     Two 8-Way joysticks with three (3) fire buttons each

    Button 1: Laser    Button 2: Missle    Button 3: Cutter

                        44 Pin Edge Connector
          Solder Side             |             Parts Side
------------------------------------------------------------------
             GND             |  1 | 2  |             GND
             GND             |  3 | 4  |             GND
             +5V             |  5 | 6  |             +5V
             +5V             |  7 | 8  |             +5V
             +12V            |  9 | 10 |             +5V
         Speaker (-)         | 11 | 12 |        Speaker (+)
       Player 1 - Up         | 13 | 14 |       Player 1 - Down
       Player 1 - Left       | 15 | 16 |       Player 1 - Right
       Player 1 - Laser      | 17 | 18 |       Player 1 - Missile
       Player 1 - Cutter     | 19 | 20 |
       Player 2 - Up         | 21 | 22 |       Player 2 - Down
       Player 2 - Left       | 23 | 24 |       Player 2 - Right
       Player 2 - Laser      | 25 | 26 |       Player 2 - Missile
       Player 2 - Cutter     | 27 | 28 |
        Coin Switch 1        | 29 | 30 |       Player 1 Start
       Player 2 Start        | 31 | 32 |
                             | 33 | 34 |
       Coin Counter 1        | 35 | 36 |
        Video Sync           | 37 | 38 |        Video Blue
        Video Green          | 39 | 40 |        Video Red
             GND             | 41 | 42 |           GND
             GND             | 43 | 44 |           GND
*/

#include "emu.h"
#include "cpu/z80/z80.h"
#include "includes/flower.h"


WRITE8_MEMBER(flower_state::flower_maincpu_irq_ack)
{
	m_maincpu->set_input_line(0, CLEAR_LINE);
}

WRITE8_MEMBER(flower_state::flower_subcpu_irq_ack)
{
	m_subcpu->set_input_line(0, CLEAR_LINE);
}

WRITE8_MEMBER(flower_state::flower_soundcpu_irq_ack)
{
	m_audiocpu->set_input_line(0, CLEAR_LINE);
}

WRITE8_MEMBER(flower_state::flower_coin_counter_w)
{
	machine().bookkeeping().coin_counter_w(0, data & 1);
}

WRITE8_MEMBER(flower_state::flower_coin_lockout_w)
{
	machine().bookkeeping().coin_lockout_global_w(~data & 1);
}

WRITE8_MEMBER(flower_state::sound_command_w)
{
	soundlatch_byte_w(space, 0, data);

	if (*m_sn_nmi_enable & 1)
		m_audiocpu->set_input_line(INPUT_LINE_NMI, PULSE_LINE);
}

static ADDRESS_MAP_START( flower_cpu1_2, AS_PROGRAM, 8, flower_state )
	AM_RANGE(0x0000, 0x7fff) AM_ROM
	AM_RANGE(0xa000, 0xa000) AM_WRITE(flower_coin_lockout_w)
	AM_RANGE(0xa001, 0xa001) AM_WRITE(flower_flipscreen_w)
	AM_RANGE(0xa002, 0xa002) AM_WRITE(flower_maincpu_irq_ack)
	AM_RANGE(0xa003, 0xa003) AM_WRITE(flower_subcpu_irq_ack)
	AM_RANGE(0xa004, 0xa004) AM_WRITE(flower_coin_counter_w)
	AM_RANGE(0xa005, 0xa005) AM_WRITENOP // subcpu nmi (unused)
	AM_RANGE(0xa100, 0xa100) AM_READ_PORT("IN0CPU1")
	AM_RANGE(0xa101, 0xa101) AM_READ_PORT("IN1CPU1")
	AM_RANGE(0xa102, 0xa102) AM_READ_PORT("IN0CPU0")
	AM_RANGE(0xa103, 0xa103) AM_READ_PORT("IN1CPU0")
	AM_RANGE(0xa400, 0xa400) AM_WRITE(sound_command_w)
	AM_RANGE(0xc000, 0xddff) AM_RAM AM_SHARE("mainram1")
	AM_RANGE(0xde00, 0xdfff) AM_RAM AM_SHARE("spriteram")
	AM_RANGE(0xe000, 0xe7ff) AM_RAM_WRITE(flower_textram_w) AM_SHARE("textram")
	AM_RANGE(0xe000, 0xefff) AM_RAM AM_SHARE("mainram2") // only cleared?
	AM_RANGE(0xf000, 0xf1ff) AM_RAM_WRITE(flower_bg0ram_w) AM_SHARE("bg0ram")
	AM_RANGE(0xf200, 0xf200) AM_RAM AM_SHARE("bg0_scroll")
	AM_RANGE(0xf800, 0xf9ff) AM_RAM_WRITE(flower_bg1ram_w) AM_SHARE("bg1ram")
	AM_RANGE(0xfa00, 0xfa00) AM_RAM AM_SHARE("bg1_scroll")
ADDRESS_MAP_END

static ADDRESS_MAP_START( flower_sound_cpu, AS_PROGRAM, 8, flower_state )
	AM_RANGE(0x0000, 0x3fff) AM_ROM
	AM_RANGE(0x4000, 0x4000) AM_WRITE(flower_soundcpu_irq_ack)
	AM_RANGE(0x4001, 0x4001) AM_WRITEONLY AM_SHARE("sn_nmi_enable")
	AM_RANGE(0x6000, 0x6000) AM_READ(soundlatch_byte_r)
	AM_RANGE(0x8000, 0x803f) AM_DEVWRITE("flower", flower_sound_device, sound1_w)
	AM_RANGE(0xa000, 0xa03f) AM_DEVWRITE("flower", flower_sound_device, sound2_w)
	AM_RANGE(0xc000, 0xc7ff) AM_RAM
ADDRESS_MAP_END


INPUT_CHANGED_MEMBER(flower_state::coin_inserted)
{
	m_maincpu->set_input_line(INPUT_LINE_NMI, newval ? CLEAR_LINE : ASSERT_LINE);
}

static INPUT_PORTS_START( flower )
	PORT_START("IN0CPU0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_COIN1 ) PORT_CHANGED_MEMBER(DEVICE_SELF, flower_state,coin_inserted, 0)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_START1  )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_START2 )
	PORT_DIPNAME( 0x08, 0x08, "Energy Decrease" )       PORT_DIPLOCATION("SW2:4")
	PORT_DIPSETTING(    0x08, "Slow" )
	PORT_DIPSETTING(    0x00, "Fast" )
	PORT_DIPNAME( 0x10, 0x10, "Invulnerability (Cheat)") PORT_DIPLOCATION("SW2:5")
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, "Keep Weapons When Destroyed" ) PORT_DIPLOCATION("SW2:6") // check code at 0x74a2
	PORT_DIPSETTING(    0x20, DEF_STR( No ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Difficulty ) )   PORT_DIPLOCATION("SW2:7")       // "Enemy Bullets"
	PORT_DIPSETTING(    0x40, DEF_STR( Normal ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Hard ) )
	PORT_DIPNAME( 0x80, 0x80, "Shot Range" )            PORT_DIPLOCATION("SW2:8")       // check code at 0x75f9
	PORT_DIPSETTING(    0x80, "Short" )
	PORT_DIPSETTING(    0x00, "Long" )

	PORT_START("IN1CPU0")
	PORT_DIPNAME( 0x07, 0x05, DEF_STR( Lives ) )        PORT_DIPLOCATION("SW1:1,2,3")
	PORT_DIPSETTING(    0x07, "1" )
	PORT_DIPSETTING(    0x06, "2" )
	PORT_DIPSETTING(    0x05, "3" )
	PORT_DIPSETTING(    0x04, "4" )
	PORT_DIPSETTING(    0x03, "5" )
	PORT_DIPSETTING(    0x02, "6" )
	PORT_DIPSETTING(    0x01, "7" )
	PORT_DIPSETTING(    0x00, "Infinite (Cheat)")
	PORT_DIPNAME( 0x18, 0x18, DEF_STR( Coinage ) )      PORT_DIPLOCATION("SW1:4,5")
	PORT_DIPSETTING(    0x00, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x08, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x18, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x10, DEF_STR( 1C_2C ) )
	PORT_DIPNAME( 0x20, 0x00, DEF_STR( Cabinet ) )      PORT_DIPLOCATION("SW1:6")       // check code at 0x759f
	PORT_DIPSETTING(    0x00, DEF_STR( Upright ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Cocktail ) )
	PORT_DIPNAME( 0x40, 0x00, DEF_STR( Demo_Sounds ) )  PORT_DIPLOCATION("SW1:7")
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Bonus_Life ) )   PORT_DIPLOCATION("SW1:8")
	PORT_DIPSETTING(    0x80, "30k, then every 50k" )
	PORT_DIPSETTING(    0x00, "50k, then every 80k" )

	PORT_START("IN0CPU1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_UP )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_NAME("P1 Laser")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_NAME("P1 Missile")
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_NAME("P1 Cutter")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("IN1CPU1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_COCKTAIL
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_COCKTAIL
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_COCKTAIL
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_COCKTAIL
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_COCKTAIL PORT_NAME("P2 Laser")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_COCKTAIL PORT_NAME("P2 Missile")
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_COCKTAIL PORT_NAME("P2 Cutter")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )
INPUT_PORTS_END

static const gfx_layout flower_charlayout =
{
	8,8,
	RGN_FRAC(1,1),
	2,
	{ 0, 4 },
	{ STEP4(0,1), STEP4(8,1) },
	{ STEP8(0,16) },
	8*8*2
};

static const gfx_layout flower_tilelayout =
{
	16,16,
	RGN_FRAC(1,2),
	4,
	{ 0, 4, RGN_FRAC(1,2), RGN_FRAC(1,2)+4 },
	{ STEP4(0,1), STEP4(8,1), STEP4(8*8*2,1), STEP4(8*8*2+8,1) },
	{ STEP8(0,16), STEP8(8*8*4,16) },
	16*16*2
};

static GFXDECODE_START( flower )
	GFXDECODE_ENTRY( "gfx1", 0, flower_charlayout, 0,  64 )
	GFXDECODE_ENTRY( "gfx2", 0, flower_tilelayout, 0,  16 )
	GFXDECODE_ENTRY( "gfx3", 0, flower_tilelayout, 0,  16 )
GFXDECODE_END


static MACHINE_CONFIG_START( flower, flower_state )

	/* basic machine hardware */
	// clock divider (of all cpus) is unknown. /6 (3.072 MHz) is too slow
	// cpus are Z80 "A" type, official maximum speed of 4 MHz, but 4.6 MHz has been proven to work in practice
	MCFG_CPU_ADD("maincpu", Z80,XTAL_18_432MHz/4)
	MCFG_CPU_PROGRAM_MAP(flower_cpu1_2)
	MCFG_CPU_VBLANK_INT_DRIVER("screen", flower_state, irq0_line_hold)

	MCFG_CPU_ADD("subcpu", Z80,XTAL_18_432MHz/4)
	MCFG_CPU_PROGRAM_MAP(flower_cpu1_2)
	MCFG_CPU_PERIODIC_INT_DRIVER(flower_state, irq0_line_hold, 120) // controls game speed? irqsource and frequency unknown

	MCFG_CPU_ADD("audiocpu", Z80,XTAL_18_432MHz/4)
	MCFG_CPU_PROGRAM_MAP(flower_sound_cpu)
	MCFG_CPU_PERIODIC_INT_DRIVER(flower_state, irq0_line_hold, 90) // controls music speed. irqsource and frequency unknown, same as subcpu perhaps?

	// tight sync, slowdowns otherwise
//  MCFG_QUANTUM_PERFECT_CPU("maincpu")
	MCFG_QUANTUM_TIME(attotime::from_hz(6000))

	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(60) // ?
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MCFG_SCREEN_SIZE(34*8, 33*8)
	MCFG_SCREEN_VISIBLE_AREA(0*8, 34*8-1, 0*8, 28*8-1)
	MCFG_SCREEN_UPDATE_DRIVER(flower_state, screen_update_flower)
	MCFG_SCREEN_PALETTE("palette")

	MCFG_GFXDECODE_ADD("gfxdecode", "palette", flower)
	MCFG_PALETTE_ADD_RRRRGGGGBBBB_PROMS("palette", 256)

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")

	MCFG_SOUND_ADD("flower", FLOWER, 0)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 1.0)
MACHINE_CONFIG_END


ROM_START( flower ) /* Komax version */
	ROM_REGION( 0x10000, "maincpu", 0 ) /* main cpu */
	ROM_LOAD( "1.5j",   0x0000, 0x8000, CRC(a4c3af78) SHA1(d149b0e0d82318273dd9cc5a143b175cdc818d0d) )

	ROM_REGION( 0x10000, "subcpu", 0 ) /* sub cpu */
	ROM_LOAD( "2.5f",   0x0000, 0x8000, CRC(7c7ee2d8) SHA1(1e67bfe0f3585be5a6e6719ccf9db764bafbcb01) )

	ROM_REGION( 0x10000, "audiocpu", 0 ) /* sound cpu */
	ROM_LOAD( "3.d9",   0x0000, 0x4000, CRC(8866c2b0) SHA1(d00f31994673e8087a1406f98e8832d07cedeb66) ) // 1xxxxxxxxxxxxx = 0xFF

	ROM_REGION( 0x2000, "gfx1", ROMREGION_INVERT ) /* tx layer */
	ROM_LOAD( "10.13e", 0x0000, 0x2000, CRC(62f9b28c) SHA1(d57d06b99e72a4f68f197a5b6c042c926cc70ca0) ) // FIRST AND SECOND HALF IDENTICAL

	ROM_REGION( 0x8000, "gfx2", ROMREGION_INVERT ) /* sprites */
	ROM_LOAD( "14.19e", 0x0000, 0x2000, CRC(11b491c5) SHA1(be1c4a0fbe8fd4e124c21e0f700efa0428376691) )
	ROM_LOAD( "13.17e", 0x2000, 0x2000, CRC(ea743986) SHA1(bbef4fd0f7d21cc89a52061fa50d7c2ea37287bd) )
	ROM_LOAD( "12.16e", 0x4000, 0x2000, CRC(e3779f7f) SHA1(8e12d06b3cdc2fcb7b77cc35f8eca45544cc4873) )
	ROM_LOAD( "11.14e", 0x6000, 0x2000, CRC(8801b34f) SHA1(256059fcd16b21e076db1c18fd9669128df1d658) )

	ROM_REGION( 0x8000, "gfx3", ROMREGION_INVERT ) /* bg layers */
	ROM_LOAD( "8.10e",  0x0000, 0x2000, CRC(f85eb20f) SHA1(699edc970c359143dee6de2a97cc2a552454785b) )
	ROM_LOAD( "6.7e",   0x2000, 0x2000, CRC(3e97843f) SHA1(4e4e5625dbf78eca97536b1428b2e49ad58c618f) )
	ROM_LOAD( "9.12e",  0x4000, 0x2000, CRC(f1d9915e) SHA1(158e1cc8c402f9ae3906363d99f2b25c94c64212) )
	ROM_LOAD( "15.9e",  0x6000, 0x2000, CRC(1cad9f72) SHA1(c38dbea266246ed4d47d12bdd8f9fae22a5f8bb8) )

	ROM_REGION( 0x8000, "sound1", 0 )
	ROM_LOAD( "4.12a",  0x0000, 0x8000, CRC(851ed9fd) SHA1(5dc048b612e45da529502bf33d968737a7b0a646) )  /* 8-bit samples */

	ROM_REGION( 0x4000, "sound2", 0 )
	ROM_LOAD( "5.16a",  0x0000, 0x4000, CRC(42fa2853) SHA1(cc1e8b8231d6f27f48b05d59390e93ea1c1c0e4c) )  /* volume tables? */

	ROM_REGION( 0x300, "proms", 0 ) /* RGB proms */
	ROM_LOAD( "82s129.k1",  0x0200, 0x0100, CRC(d311ed0d) SHA1(1d530c874aecf93133d610ab3ce668548712913a) ) // r
	ROM_LOAD( "82s129.k2",  0x0100, 0x0100, CRC(ababb072) SHA1(a9d46d12534c8662c6b54df94e96907f3a156968) ) // g
	ROM_LOAD( "82s129.k3",  0x0000, 0x0100, CRC(5aab7b41) SHA1(8d44639c7c9f1ba34fe9c4e74c8a38b6453f7ac0) ) // b

	ROM_REGION( 0x0520, "user1", 0 ) /* Other proms, (zoom table?) */
	ROM_LOAD( "82s147.d7",  0x0000, 0x0200, CRC(f0dbb2a7) SHA1(03cd8fd41d6406894c6931e883a9ac6a4a4effc9) )
	ROM_LOAD( "82s147.j18", 0x0200, 0x0200, CRC(d7de0860) SHA1(5d3d8c5476b1edffdacde09d592c64e78d2b90c0) )
	ROM_LOAD( "82s123.k7",  0x0400, 0x0020, CRC(ea9c65e4) SHA1(1bdd77a7f3ef5f8ec4dbb9524498c0c4a356f089) )
	ROM_LOAD( "82s129.a1",  0x0420, 0x0100, CRC(c8dad3fc) SHA1(8e852efac70223d02e45b20ed8a12e38c5010a78) )
ROM_END

ROM_START( flowerj ) /* Sega/Alpha version.  Sega game number 834-5998 */
	ROM_REGION( 0x10000, "maincpu", 0 ) /* main cpu */
	ROM_LOAD( "1",   0x0000, 0x8000, CRC(63a2ef04) SHA1(0770f5a18d58b780abcda7e000c2a5e46f96d319) ) // hacked? "AKINA.N" changed to "JUKYUNG"

	ROM_REGION( 0x10000, "subcpu", 0 ) /* sub cpu */
	ROM_LOAD( "2.5f",   0x0000, 0x8000, CRC(7c7ee2d8) SHA1(1e67bfe0f3585be5a6e6719ccf9db764bafbcb01) )

	ROM_REGION( 0x10000, "audiocpu", 0 ) /* sound cpu */
	ROM_LOAD( "3.d9",   0x0000, 0x4000, CRC(8866c2b0) SHA1(d00f31994673e8087a1406f98e8832d07cedeb66) ) // 1xxxxxxxxxxxxx = 0xFF

	ROM_REGION( 0x2000, "gfx1", ROMREGION_INVERT ) /* tx layer */
	ROM_LOAD( "10.13e", 0x0000, 0x2000, CRC(62f9b28c) SHA1(d57d06b99e72a4f68f197a5b6c042c926cc70ca0) ) // FIRST AND SECOND HALF IDENTICAL

	ROM_REGION( 0x8000, "gfx2", ROMREGION_INVERT ) /* sprites */
	ROM_LOAD( "14.19e", 0x0000, 0x2000, CRC(11b491c5) SHA1(be1c4a0fbe8fd4e124c21e0f700efa0428376691) )
	ROM_LOAD( "13.17e", 0x2000, 0x2000, CRC(ea743986) SHA1(bbef4fd0f7d21cc89a52061fa50d7c2ea37287bd) )
	ROM_LOAD( "12.16e", 0x4000, 0x2000, CRC(e3779f7f) SHA1(8e12d06b3cdc2fcb7b77cc35f8eca45544cc4873) )
	ROM_LOAD( "11.14e", 0x6000, 0x2000, CRC(8801b34f) SHA1(256059fcd16b21e076db1c18fd9669128df1d658) )

	ROM_REGION( 0x8000, "gfx3", ROMREGION_INVERT ) /* bg layers */
	ROM_LOAD( "8.10e",  0x0000, 0x2000, CRC(f85eb20f) SHA1(699edc970c359143dee6de2a97cc2a552454785b) )
	ROM_LOAD( "6.7e",   0x2000, 0x2000, CRC(3e97843f) SHA1(4e4e5625dbf78eca97536b1428b2e49ad58c618f) )
	ROM_LOAD( "9.12e",  0x4000, 0x2000, CRC(f1d9915e) SHA1(158e1cc8c402f9ae3906363d99f2b25c94c64212) )
	ROM_LOAD( "7.9e",   0x6000, 0x2000, CRC(e350f36c) SHA1(f97204dc95b4000c268afc053a2333c1629e07d8) )

	ROM_REGION( 0x8000, "sound1", 0 )
	ROM_LOAD( "4.12a",  0x0000, 0x8000, CRC(851ed9fd) SHA1(5dc048b612e45da529502bf33d968737a7b0a646) )  /* 8-bit samples */

	ROM_REGION( 0x4000, "sound2", 0 )
	ROM_LOAD( "5.16a",  0x0000, 0x4000, CRC(42fa2853) SHA1(cc1e8b8231d6f27f48b05d59390e93ea1c1c0e4c) )  /* volume tables? */

	ROM_REGION( 0x300, "proms", 0 ) /* RGB proms */
	ROM_LOAD( "82s129.k1",  0x0200, 0x0100, CRC(d311ed0d) SHA1(1d530c874aecf93133d610ab3ce668548712913a) ) // r
	ROM_LOAD( "82s129.k2",  0x0100, 0x0100, CRC(ababb072) SHA1(a9d46d12534c8662c6b54df94e96907f3a156968) ) // g
	ROM_LOAD( "82s129.k3",  0x0000, 0x0100, CRC(5aab7b41) SHA1(8d44639c7c9f1ba34fe9c4e74c8a38b6453f7ac0) ) // b

	ROM_REGION( 0x0520, "user1", 0 ) /* Other proms, (zoom table?) */
	ROM_LOAD( "82s147.d7",  0x0000, 0x0200, CRC(f0dbb2a7) SHA1(03cd8fd41d6406894c6931e883a9ac6a4a4effc9) )
	ROM_LOAD( "82s147.j18", 0x0200, 0x0200, CRC(d7de0860) SHA1(5d3d8c5476b1edffdacde09d592c64e78d2b90c0) )
	ROM_LOAD( "82s123.k7",  0x0400, 0x0020, CRC(ea9c65e4) SHA1(1bdd77a7f3ef5f8ec4dbb9524498c0c4a356f089) )
	ROM_LOAD( "82s129.a1",  0x0420, 0x0100, CRC(c8dad3fc) SHA1(8e852efac70223d02e45b20ed8a12e38c5010a78) )
ROM_END


GAME( 1986, flower,  0,      flower, flower, driver_device, 0, ROT0, "Clarue (Komax license)", "Flower (US)", MACHINE_IMPERFECT_SOUND | MACHINE_SUPPORTS_SAVE )
GAME( 1986, flowerj, flower, flower, flower, driver_device, 0, ROT0, "Clarue (Sega / Alpha Denshi Co. license)", "Flower (Japan)", MACHINE_IMPERFECT_SOUND | MACHINE_SUPPORTS_SAVE)
