/* Flower (c)1986 Komax
 - Driver by InsideOutBoy

todo:

fix sound
improve interrupts
sprite flipping is incorrect for one of the enemies so its probably wrong
screenshots look like the game has sprite zooming
http://emustatus.rainemu.com/games/flower.htm


        FLOWER   CHIP PLACEMENT

USES THREE Z80 CPU'S

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
*/

#include "driver.h"
#include "deprecat.h"
#include "sound/custom.h"

extern UINT8 *flower_textram, *flower_bg0ram, *flower_bg1ram, *flower_bg0_scroll, *flower_bg1_scroll;

WRITE8_HANDLER( flower_textram_w );
WRITE8_HANDLER( flower_bg0ram_w );
WRITE8_HANDLER( flower_bg1ram_w );
WRITE8_HANDLER( flower_flipscreen_w );
VIDEO_UPDATE( flower );
VIDEO_START( flower );
PALETTE_INIT( flower );

extern UINT8 *flower_soundregs1,*flower_soundregs2;
void *flower_sh_start(int clock, const struct CustomSound_interface *config);
WRITE8_HANDLER( flower_sound1_w );
WRITE8_HANDLER( flower_sound2_w );


static WRITE8_HANDLER( flower_irq_ack )
{
	cpunum_set_input_line(Machine, 0, 0, CLEAR_LINE);
}


static int sn_irq_enable,sn_nmi_enable;

static WRITE8_HANDLER( sn_irq_enable_w )
{
	sn_irq_enable = data & 1;

	cpunum_set_input_line(Machine, 2, 0, CLEAR_LINE);
}

static INTERRUPT_GEN( sn_irq )
{
	if (sn_irq_enable)
		cpunum_set_input_line(Machine, 2, 0, ASSERT_LINE);
}

static WRITE8_HANDLER( sn_nmi_enable_w )
{
	sn_nmi_enable = data & 1;
}

static WRITE8_HANDLER( sound_command_w )
{
	soundlatch_w(machine,0,data);
	if (sn_nmi_enable)
		cpunum_set_input_line(machine, 2, INPUT_LINE_NMI, PULSE_LINE);
}

static ADDRESS_MAP_START( flower_cpu1, ADDRESS_SPACE_PROGRAM, 8 )
	AM_RANGE(0x0000, 0x7fff) AM_ROM
	AM_RANGE(0xa000, 0xa000) AM_WRITENOP	//watchdog?
	AM_RANGE(0xa001, 0xa001) AM_WRITE(flower_flipscreen_w)
	AM_RANGE(0xa002, 0xa002) AM_WRITE(flower_irq_ack)	//irq ack / enable, maybe?
	AM_RANGE(0xa004, 0xa004) AM_WRITENOP	//nmi enable (routine is empty)
	AM_RANGE(0xa102, 0xa102) AM_READ(input_port_0_r)
	AM_RANGE(0xa103, 0xa103) AM_READ(input_port_1_r)
	AM_RANGE(0xc000, 0xddff) AM_RAM AM_SHARE(1)
	AM_RANGE(0xde00, 0xdfff) AM_RAM AM_SHARE(2) AM_BASE(&spriteram)
	AM_RANGE(0xe000, 0xe7ff) AM_READWRITE(MRA8_RAM, flower_textram_w) AM_SHARE(3) AM_BASE(&flower_textram)
	AM_RANGE(0xe000, 0xefff) AM_RAM //only cleared?
	AM_RANGE(0xf000, 0xf1ff) AM_READWRITE(MRA8_RAM, flower_bg0ram_w)  AM_SHARE(4) AM_BASE(&flower_bg0ram)
	AM_RANGE(0xf200, 0xf200) AM_RAM AM_SHARE(5) AM_BASE(&flower_bg0_scroll)
	AM_RANGE(0xf800, 0xf9ff) AM_READWRITE(MRA8_RAM, flower_bg1ram_w)  AM_SHARE(6) AM_BASE(&flower_bg1ram)
	AM_RANGE(0xfa00, 0xfa00) AM_RAM AM_SHARE(7) AM_BASE(&flower_bg1_scroll)
ADDRESS_MAP_END

static ADDRESS_MAP_START( flower_cpu2, ADDRESS_SPACE_PROGRAM, 8 )
	AM_RANGE(0x0000, 0x7fff) AM_ROM
	AM_RANGE(0xa003, 0xa003) AM_WRITENOP	//irq enable
	AM_RANGE(0xa005, 0xa005) AM_WRITENOP	//nmi enable (routine is empty)
	AM_RANGE(0xa100, 0xa100) AM_READ(input_port_2_r)
	AM_RANGE(0xa101, 0xa101) AM_READ(input_port_3_r)
	AM_RANGE(0xa400, 0xa400) AM_WRITE(sound_command_w)
	AM_RANGE(0xc000, 0xddff) AM_RAM AM_SHARE(1)
	AM_RANGE(0xde00, 0xdfff) AM_RAM AM_SHARE(2)
	AM_RANGE(0xe000, 0xe7ff) AM_READWRITE(MRA8_RAM, flower_textram_w) AM_SHARE(3)
	AM_RANGE(0xf000, 0xf1ff) AM_READWRITE(MRA8_RAM, flower_bg0ram_w)  AM_SHARE(4)
	AM_RANGE(0xf200, 0xf200) AM_RAM AM_SHARE(5)
	AM_RANGE(0xf800, 0xf9ff) AM_READWRITE(MRA8_RAM, flower_bg1ram_w)  AM_SHARE(6)
	AM_RANGE(0xfa00, 0xfa00) AM_RAM AM_SHARE(7)
ADDRESS_MAP_END

static ADDRESS_MAP_START( flower_sound_cpu, ADDRESS_SPACE_PROGRAM, 8 )
	AM_RANGE(0x0000, 0x3fff) AM_ROM
	AM_RANGE(0x4000, 0x4000) AM_WRITE(sn_irq_enable_w)
	AM_RANGE(0x4001, 0x4001) AM_WRITE(sn_nmi_enable_w)
	AM_RANGE(0x6000, 0x6000) AM_READ(soundlatch_r)
	AM_RANGE(0x8000, 0x803f) AM_WRITE(flower_sound1_w) AM_BASE(&flower_soundregs1)
	AM_RANGE(0xa000, 0xa03f) AM_WRITE(flower_sound2_w) AM_BASE(&flower_soundregs2)
	AM_RANGE(0xc000, 0xc7ff) AM_RAM
ADDRESS_MAP_END


static INPUT_PORTS_START( flower )
	PORT_START_TAG("IN0CPU0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_START1  )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_START2 )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Difficulty ) )
	PORT_DIPSETTING(    0x08, DEF_STR( Easy ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Hard ) )
	PORT_DIPNAME( 0x10, 0x10, "Invulnerability (Cheat)")
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x00, "Keep Items on Life Loss" )	// check code at 0x74a2
	PORT_DIPSETTING(    0x20, DEF_STR( No ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Unused ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, "Shot Range" )			// check code at 0x75f9
	PORT_DIPSETTING(    0x80, DEF_STR( Medium ) )
	PORT_DIPSETTING(    0x00, "Long" )

	PORT_START_TAG("IN1CPU0")
	PORT_DIPNAME( 0x07, 0x05, DEF_STR( Lives ) )		// what should be the default value ?
	PORT_DIPSETTING(    0x07, "1" )
	PORT_DIPSETTING(    0x06, "2" )
	PORT_DIPSETTING(    0x05, "3" )
	PORT_DIPSETTING(    0x04, "4" )
	PORT_DIPSETTING(    0x03, "5" )
	PORT_DIPSETTING(    0x02, "6" )
	PORT_DIPSETTING(    0x01, "7" )
	PORT_DIPSETTING(    0x00, "Infinite (Cheat)")
	PORT_DIPNAME( 0x18, 0x18, DEF_STR( Coinage ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x08, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x18, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x10, DEF_STR( 1C_2C ) )
	PORT_DIPNAME( 0x20, 0x00, DEF_STR( Cabinet ) )		// check code at 0x759f
	PORT_DIPSETTING(    0x00, DEF_STR( Upright ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Cocktail ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Unused ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Bonus_Life ) )
	PORT_DIPSETTING(    0x80, "30k, 80k then every 50k" )
	PORT_DIPSETTING(    0x00, "50k, 130k then every 80k" )

	PORT_START_TAG("IN0CPU1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_UP    )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN  )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT  )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_NAME("P1 Fire")			// Fire (normal or laser)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_NAME("P1 Missile")			// Missile
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_NAME("P1 Cutter")			// Cutter
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START_TAG("IN1CPU1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_COCKTAIL
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_COCKTAIL
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_COCKTAIL
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_COCKTAIL
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_COCKTAIL	PORT_NAME("P2 Fire")// Fire (normal or laser)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_COCKTAIL	PORT_NAME("P2 Missile")// Missile
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_COCKTAIL	PORT_NAME("P2 Cutter")// Cutter
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
	GFXDECODE_ENTRY( REGION_GFX1, 0, flower_charlayout, 0,  64 )
	GFXDECODE_ENTRY( REGION_GFX2, 0, flower_tilelayout, 0,  16 )
	GFXDECODE_ENTRY( REGION_GFX3, 0, flower_tilelayout, 0,  16 )
GFXDECODE_END

static const struct CustomSound_interface custom_interface =
{
	flower_sh_start
};

static MACHINE_DRIVER_START( flower )

	/* basic machine hardware */
	MDRV_CPU_ADD(Z80,8000000)
	MDRV_CPU_PROGRAM_MAP(flower_cpu1,0)
	MDRV_CPU_VBLANK_INT_HACK(irq0_line_hold,10)
//  MDRV_CPU_VBLANK_INT("main", nmi_line_pulse) //nmis stuff up the writes to shared ram

	MDRV_CPU_ADD(Z80,8000000)
	MDRV_CPU_PROGRAM_MAP(flower_cpu2,0)
	MDRV_CPU_VBLANK_INT("main", irq0_line_hold)
//  MDRV_CPU_VBLANK_INT("main", nmi_line_pulse)

	MDRV_CPU_ADD(Z80,8000000)
	MDRV_CPU_PROGRAM_MAP(flower_sound_cpu,0)
	MDRV_CPU_PERIODIC_INT(sn_irq, 90)	/* periodic interrupt, don't know about the frequency */

	/* video hardware */
	MDRV_SCREEN_ADD("main", RASTER)
	MDRV_SCREEN_REFRESH_RATE(60)
	MDRV_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MDRV_SCREEN_FORMAT(BITMAP_FORMAT_INDEXED16)
	MDRV_SCREEN_SIZE(34*8, 33*8)
	MDRV_SCREEN_VISIBLE_AREA(0*8, 34*8-1, 0*8, 28*8-1)

	MDRV_GFXDECODE(flower)

	MDRV_PALETTE_INIT(flower)
	MDRV_PALETTE_LENGTH(256)

	MDRV_VIDEO_START(flower)
	MDRV_VIDEO_UPDATE(flower)

	/* sound hardware */
	MDRV_SPEAKER_STANDARD_MONO("mono")

	MDRV_SOUND_ADD(CUSTOM, 0)
	MDRV_SOUND_CONFIG(custom_interface)
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "mono", 1.0)
MACHINE_DRIVER_END


ROM_START( flower )
	ROM_REGION( 0x10000, REGION_CPU1, 0 ) /* main cpu */
	ROM_LOAD( "1.5j",   0x0000, 0x8000, CRC(a4c3af78) SHA1(d149b0e0d82318273dd9cc5a143b175cdc818d0d) )

	ROM_REGION( 0x10000, REGION_CPU2, 0 ) /* sub cpu */
	ROM_LOAD( "2.5f",   0x0000, 0x8000, CRC(7c7ee2d8) SHA1(1e67bfe0f3585be5a6e6719ccf9db764bafbcb01) )

	ROM_REGION( 0x10000, REGION_CPU3, 0 ) /* sound cpu */
	ROM_LOAD( "3.d9",   0x0000, 0x4000, CRC(8866c2b0) SHA1(d00f31994673e8087a1406f98e8832d07cedeb66) ) // 1xxxxxxxxxxxxx = 0xFF

	ROM_REGION( 0x2000, REGION_GFX1, ROMREGION_INVERT | ROMREGION_DISPOSE ) /* tx layer */
	ROM_LOAD( "10.13e", 0x0000, 0x2000, CRC(62f9b28c) SHA1(d57d06b99e72a4f68f197a5b6c042c926cc70ca0) ) // FIRST AND SECOND HALF IDENTICAL

	ROM_REGION( 0x8000, REGION_GFX2, ROMREGION_INVERT | ROMREGION_DISPOSE ) /* sprites */
	ROM_LOAD( "14.19e", 0x0000, 0x2000, CRC(11b491c5) SHA1(be1c4a0fbe8fd4e124c21e0f700efa0428376691) )
	ROM_LOAD( "13.17e", 0x2000, 0x2000, CRC(ea743986) SHA1(bbef4fd0f7d21cc89a52061fa50d7c2ea37287bd) )
	ROM_LOAD( "12.16e", 0x4000, 0x2000, CRC(e3779f7f) SHA1(8e12d06b3cdc2fcb7b77cc35f8eca45544cc4873) )
	ROM_LOAD( "11.14e", 0x6000, 0x2000, CRC(8801b34f) SHA1(256059fcd16b21e076db1c18fd9669128df1d658) )

	ROM_REGION( 0x8000, REGION_GFX3, ROMREGION_INVERT | ROMREGION_DISPOSE ) /* bg layers */
	ROM_LOAD( "8.10e",  0x0000, 0x2000, CRC(f85eb20f) SHA1(699edc970c359143dee6de2a97cc2a552454785b) )
	ROM_LOAD( "6.7e",   0x2000, 0x2000, CRC(3e97843f) SHA1(4e4e5625dbf78eca97536b1428b2e49ad58c618f) )
	ROM_LOAD( "9.12e",  0x4000, 0x2000, CRC(f1d9915e) SHA1(158e1cc8c402f9ae3906363d99f2b25c94c64212) )
	ROM_LOAD( "15.9e",  0x6000, 0x2000, CRC(1cad9f72) SHA1(c38dbea266246ed4d47d12bdd8f9fae22a5f8bb8) )

	ROM_REGION( 0x8000, REGION_SOUND1, 0 )
	ROM_LOAD( "4.12a",  0x0000, 0x8000, CRC(851ed9fd) SHA1(5dc048b612e45da529502bf33d968737a7b0a646) )	/* 8-bit samples */

	ROM_REGION( 0x4000, REGION_SOUND2, 0 )
	ROM_LOAD( "5.16a",  0x0000, 0x4000, CRC(42fa2853) SHA1(cc1e8b8231d6f27f48b05d59390e93ea1c1c0e4c) )	/* volume tables? */

	ROM_REGION( 0x300, REGION_PROMS, 0 ) /* RGB proms */
	ROM_LOAD( "82s129.k1",  0x0200, 0x0100, CRC(d311ed0d) SHA1(1d530c874aecf93133d610ab3ce668548712913a) ) // r?
	ROM_LOAD( "82s129.k2",  0x0100, 0x0100, CRC(ababb072) SHA1(a9d46d12534c8662c6b54df94e96907f3a156968) ) // g?
	ROM_LOAD( "82s129.k3",  0x0000, 0x0100, CRC(5aab7b41) SHA1(8d44639c7c9f1ba34fe9c4e74c8a38b6453f7ac0) ) // b?

	ROM_REGION( 0x0520, REGION_USER1, 0 ) /* Other proms, (zoom table?) */
	ROM_LOAD( "82s147.d7",  0x0000, 0x0200, CRC(f0dbb2a7) SHA1(03cd8fd41d6406894c6931e883a9ac6a4a4effc9) )
	ROM_LOAD( "82s147.j18", 0x0200, 0x0200, CRC(d7de0860) SHA1(5d3d8c5476b1edffdacde09d592c64e78d2b90c0) )
	ROM_LOAD( "82s123.k7",  0x0400, 0x0020, CRC(ea9c65e4) SHA1(1bdd77a7f3ef5f8ec4dbb9524498c0c4a356f089) )
	ROM_LOAD( "82s129.a1",  0x0420, 0x0100, CRC(c8dad3fc) SHA1(8e852efac70223d02e45b20ed8a12e38c5010a78) )
ROM_END

ROM_START( flowerbl )
	ROM_REGION( 0x10000, REGION_CPU1, 0 ) /* main cpu */
	ROM_LOAD( "1",   0x0000, 0x8000, CRC(63a2ef04) SHA1(0770f5a18d58b780abcda7e000c2a5e46f96d319) )

	ROM_REGION( 0x10000, REGION_CPU2, 0 ) /* sub cpu */
	ROM_LOAD( "2.5f",   0x0000, 0x8000, CRC(7c7ee2d8) SHA1(1e67bfe0f3585be5a6e6719ccf9db764bafbcb01) )

	ROM_REGION( 0x10000, REGION_CPU3, 0 ) /* sound cpu */
	ROM_LOAD( "3.d9",   0x0000, 0x4000, CRC(8866c2b0) SHA1(d00f31994673e8087a1406f98e8832d07cedeb66) ) // 1xxxxxxxxxxxxx = 0xFF

	ROM_REGION( 0x2000, REGION_GFX1, ROMREGION_INVERT | ROMREGION_DISPOSE ) /* tx layer */
	ROM_LOAD( "10.13e", 0x0000, 0x2000, CRC(62f9b28c) SHA1(d57d06b99e72a4f68f197a5b6c042c926cc70ca0) ) // FIRST AND SECOND HALF IDENTICAL

	ROM_REGION( 0x8000, REGION_GFX2, ROMREGION_INVERT | ROMREGION_DISPOSE ) /* sprites */
	ROM_LOAD( "14.19e", 0x0000, 0x2000, CRC(11b491c5) SHA1(be1c4a0fbe8fd4e124c21e0f700efa0428376691) )
	ROM_LOAD( "13.17e", 0x2000, 0x2000, CRC(ea743986) SHA1(bbef4fd0f7d21cc89a52061fa50d7c2ea37287bd) )
	ROM_LOAD( "12.16e", 0x4000, 0x2000, CRC(e3779f7f) SHA1(8e12d06b3cdc2fcb7b77cc35f8eca45544cc4873) )
	ROM_LOAD( "11.14e", 0x6000, 0x2000, CRC(8801b34f) SHA1(256059fcd16b21e076db1c18fd9669128df1d658) )

	ROM_REGION( 0x8000, REGION_GFX3, ROMREGION_INVERT | ROMREGION_DISPOSE ) /* bg layers */
	ROM_LOAD( "8.10e",  0x0000, 0x2000, CRC(f85eb20f) SHA1(699edc970c359143dee6de2a97cc2a552454785b) )
	ROM_LOAD( "6.7e",   0x2000, 0x2000, CRC(3e97843f) SHA1(4e4e5625dbf78eca97536b1428b2e49ad58c618f) )
	ROM_LOAD( "9.12e",  0x4000, 0x2000, CRC(f1d9915e) SHA1(158e1cc8c402f9ae3906363d99f2b25c94c64212) )
	ROM_LOAD( "7",      0x6000, 0x2000, CRC(e350f36c) SHA1(f97204dc95b4000c268afc053a2333c1629e07d8) )

	ROM_REGION( 0x8000, REGION_SOUND1, 0 )
	ROM_LOAD( "4.12a",  0x0000, 0x8000, CRC(851ed9fd) SHA1(5dc048b612e45da529502bf33d968737a7b0a646) )	/* 8-bit samples */

	ROM_REGION( 0x4000, REGION_SOUND2, 0 )
	ROM_LOAD( "5.16a",  0x0000, 0x4000, CRC(42fa2853) SHA1(cc1e8b8231d6f27f48b05d59390e93ea1c1c0e4c) )	/* volume tables? */

	ROM_REGION( 0x300, REGION_PROMS, 0 ) /* RGB proms */
	ROM_LOAD( "82s129.k1",  0x0200, 0x0100, CRC(d311ed0d) SHA1(1d530c874aecf93133d610ab3ce668548712913a) ) // r?
	ROM_LOAD( "82s129.k2",  0x0100, 0x0100, CRC(ababb072) SHA1(a9d46d12534c8662c6b54df94e96907f3a156968) ) // g?
	ROM_LOAD( "82s129.k3",  0x0000, 0x0100, CRC(5aab7b41) SHA1(8d44639c7c9f1ba34fe9c4e74c8a38b6453f7ac0) ) // b?

	ROM_REGION( 0x0520, REGION_USER1, 0 ) /* Other proms, (zoom table?) */
	ROM_LOAD( "82s147.d7",  0x0000, 0x0200, CRC(f0dbb2a7) SHA1(03cd8fd41d6406894c6931e883a9ac6a4a4effc9) )
	ROM_LOAD( "82s147.j18", 0x0200, 0x0200, CRC(d7de0860) SHA1(5d3d8c5476b1edffdacde09d592c64e78d2b90c0) )
	ROM_LOAD( "82s123.k7",  0x0400, 0x0020, CRC(ea9c65e4) SHA1(1bdd77a7f3ef5f8ec4dbb9524498c0c4a356f089) )
	ROM_LOAD( "82s129.a1",  0x0420, 0x0100, CRC(c8dad3fc) SHA1(8e852efac70223d02e45b20ed8a12e38c5010a78) )
ROM_END


GAME( 1986, flower, 0, flower, flower, 0, ROT0, "Komax", "Flower", GAME_IMPERFECT_SOUND )
GAME( 1986, flowerbl, flower, flower, flower, 0, ROT0, "bootleg", "Flower (bootleg)", GAME_IMPERFECT_SOUND )
