/*******************************************************************************

    Battle Rangers                  (c) 1988 Data East Corporation
    Bloody Wolf                     (c) 1988 Data East USA

    Emulation by Bryan McPhail, mish@tendril.co.uk

    This board is a modified PC-Engine PCB, differences from PC-Engine console:

    Input ports are different (2 dips, 2 joysticks, 1 coin port)
    _Interface_ to palette chip is different, palette data is the same.
    Extra sound chips, and extra processor to drive them.
    Twice as much VRAM.

    Todo:
    - Priority is wrong for the submarine at the end of level 1.
    - There seems to be a bug with a stuck note from the YM2203 FM channel
      at the start of scene 3 and near the ending when your characters are
      flying over a forest in a helicopter.

**********************************************************************/

#include "driver.h"
#include "deprecat.h"
#include "cpu/h6280/h6280.h"
#include "sound/2203intf.h"
#include "sound/msm5205.h"
#include "sound/c6280.h"

VIDEO_UPDATE( battlera );
VIDEO_START( battlera );
INTERRUPT_GEN( battlera_interrupt );

READ8_HANDLER( HuC6270_register_r );
WRITE8_HANDLER( HuC6270_register_w );
READ8_HANDLER( HuC6270_data_r );
WRITE8_HANDLER( HuC6270_data_w );
WRITE8_HANDLER( battlera_palette_w );

READ8_HANDLER( HuC6270_debug_r );
WRITE8_HANDLER( HuC6270_debug_w );

static int control_port_select;

/******************************************************************************/

static WRITE8_HANDLER( battlera_sound_w )
{
	if (offset==0) {
		soundlatch_w(machine,0,data);
		cpunum_set_input_line(machine, 1, 0, HOLD_LINE);
	}
}

/******************************************************************************/

static WRITE8_HANDLER( control_data_w )
{
	control_port_select=data;
}

static READ8_HANDLER( control_data_r )
{
	switch (control_port_select) {
		case 0xfe: return readinputportbytag("IN0"); /* Player 1 */
		case 0xfd: return readinputportbytag("IN1"); /* Player 2 */
		case 0xfb: return readinputportbytag("IN2"); /* Coins */
		case 0xf7: return readinputportbytag("DSW2"); /* Dip 2 */
		case 0xef: return readinputportbytag("DSW1"); /* Dip 1 */
	}

    return 0xff;
}

/******************************************************************************/

static ADDRESS_MAP_START( battlera_readmem, ADDRESS_SPACE_PROGRAM, 8 )
	AM_RANGE(0x000000, 0x0fffff) AM_READ(MRA8_ROM)
	AM_RANGE(0x100000, 0x10ffff) AM_READ(HuC6270_debug_r) /* Cheat to view vram data */
	AM_RANGE(0x1f0000, 0x1f1fff) AM_READ(MRA8_BANK8)
	AM_RANGE(0x1fe000, 0x1fe001) AM_READ(HuC6270_register_r)
	AM_RANGE(0x1ff000, 0x1ff001) AM_READ(control_data_r)
ADDRESS_MAP_END

static ADDRESS_MAP_START( battlera_writemem, ADDRESS_SPACE_PROGRAM, 8 )
	AM_RANGE(0x000000, 0x0fffff) AM_WRITE(MWA8_ROM)
	AM_RANGE(0x100000, 0x10ffff) AM_WRITE(HuC6270_debug_w) /* Cheat to edit vram data */
	AM_RANGE(0x1e0800, 0x1e0801) AM_WRITE(battlera_sound_w)
	AM_RANGE(0x1e1000, 0x1e13ff) AM_WRITE(battlera_palette_w) AM_BASE(&paletteram)
	AM_RANGE(0x1f0000, 0x1f1fff) AM_WRITE(MWA8_BANK8) /* Main ram */
	AM_RANGE(0x1fe000, 0x1fe001) AM_WRITE(HuC6270_register_w)
	AM_RANGE(0x1fe002, 0x1fe003) AM_WRITE(HuC6270_data_w)
	AM_RANGE(0x1ff000, 0x1ff001) AM_WRITE(control_data_w)
	AM_RANGE(0x1ff400, 0x1ff403) AM_WRITE(H6280_irq_status_w)
ADDRESS_MAP_END

static ADDRESS_MAP_START( battlera_portwrite, ADDRESS_SPACE_IO, 8 )
	AM_RANGE(0x00, 0x01) AM_WRITE(HuC6270_register_w)
	AM_RANGE(0x02, 0x03) AM_WRITE(HuC6270_data_w)
ADDRESS_MAP_END

/******************************************************************************/

static WRITE8_HANDLER( YM2203_w )
{
	switch (offset) {
	case 0: YM2203_control_port_0_w(machine,0,data); break;
	case 1: YM2203_write_port_0_w(machine,0,data); break;
	}
}

static int msm5205next;

static void battlera_adpcm_int(int data)
{
	static int toggle;

	MSM5205_data_w(0,msm5205next >> 4);
	msm5205next<<=4;

	toggle = 1 - toggle;
	if (toggle)
		cpunum_set_input_line(Machine, 1, 1, HOLD_LINE);
}

static WRITE8_HANDLER( battlera_adpcm_data_w )
{
	msm5205next=data;
}

static WRITE8_HANDLER( battlera_adpcm_reset_w )
{
	MSM5205_reset_w(0,0);
}

static ADDRESS_MAP_START( sound_readmem, ADDRESS_SPACE_PROGRAM, 8 )
	AM_RANGE(0x000000, 0x00ffff) AM_READ(MRA8_ROM)
	AM_RANGE(0x1f0000, 0x1f1fff) AM_READ(MRA8_BANK7) /* Main ram */
	AM_RANGE(0x1ff000, 0x1ff001) AM_READ(soundlatch_r)
ADDRESS_MAP_END

static ADDRESS_MAP_START( sound_writemem, ADDRESS_SPACE_PROGRAM, 8 )
 	AM_RANGE(0x000000, 0x00ffff) AM_WRITE(MWA8_ROM)
	AM_RANGE(0x040000, 0x040001) AM_WRITE(YM2203_w)
	AM_RANGE(0x080000, 0x080001) AM_WRITE(battlera_adpcm_data_w)
	AM_RANGE(0x1fe800, 0x1fe80f) AM_WRITE(C6280_0_w)
	AM_RANGE(0x1f0000, 0x1f1fff) AM_WRITE(MWA8_BANK7) /* Main ram */
	AM_RANGE(0x1ff000, 0x1ff001) AM_WRITE(battlera_adpcm_reset_w)
	AM_RANGE(0x1ff400, 0x1ff403) AM_WRITE(H6280_irq_status_w)
ADDRESS_MAP_END

/******************************************************************************/

static INPUT_PORTS_START( battlera )
	PORT_START_TAG("IN0")  /* Player 1 controls */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(1)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(1)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_START1 )

	PORT_START_TAG("IN1")  /* Player 2 controls */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(2)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_START2 )

	PORT_START_TAG("IN2")	/* Coins */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_COIN3 )
	PORT_BIT( 0xf8, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START_TAG("DSW1")
	PORT_DIPNAME( 0x03, 0x03, DEF_STR( Coin_A ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x01, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x03, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x02, DEF_STR( 1C_2C ) )
	PORT_DIPNAME( 0x0c, 0x0c, DEF_STR( Coin_B ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x04, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x0c, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x08, DEF_STR( 1C_2C ) )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Unused ) )
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Demo_Sounds ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x20, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Unused ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unused ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START_TAG("DSW2")
	PORT_DIPNAME( 0x03, 0x03, DEF_STR( Lives ) )
	PORT_DIPSETTING(    0x01, "1" )
	PORT_DIPSETTING(    0x02, "2" )
	PORT_DIPSETTING(    0x03, "3" )
	PORT_DIPSETTING(    0x00, "Infinite (Cheat)")
	PORT_DIPNAME( 0x0c, 0x0c, DEF_STR( Difficulty ) )
	PORT_DIPSETTING(    0x08, DEF_STR( Easy ) )
	PORT_DIPSETTING(    0x0c, DEF_STR( Normal ) )
	PORT_DIPSETTING(    0x04, DEF_STR( Hard ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Very_Hard ) )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Allow_Continue ) )
	PORT_DIPSETTING(    0x00, DEF_STR( No ) )
	PORT_DIPSETTING(    0x10, DEF_STR( Yes ) )
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

/******************************************************************************/

static const gfx_layout tiles =
{
	8,8,
	4096,
	4,
	{ 16*8, 16*8+8, 0, 8 },
	{ 0, 1, 2, 3, 4, 5, 6, 7 },
	{ 0*16, 1*16, 2*16, 3*16, 4*16, 5*16, 6*16, 7*16 },
	32*8
};

static const gfx_layout sprites =
{
	16,16,
	1024,
	4,
	{ 96*8, 64*8, 32*8, 0 },
	{ 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15 },
	{ 0*16, 1*16, 2*16, 3*16, 4*16, 5*16, 6*16, 7*16,
			8*16, 9*16, 10*16, 11*16, 12*16, 13*16, 14*16, 15*16 },
	128*8
};

static GFXDECODE_START( battlera )
	GFXDECODE_ENTRY( 0, 0, tiles,       0,  16 ) /* Dynamically modified */
	GFXDECODE_ENTRY( 0, 0, sprites,   256,  16 ) /* Dynamically modified */
	GFXDECODE_ENTRY( 0, 0, tiles  ,   256,  16 ) /* Blank tile */
GFXDECODE_END

/******************************************************************************/

static const struct MSM5205interface msm5205_interface =
{
	battlera_adpcm_int,/* interrupt function */
	MSM5205_S48_4B		/* 8KHz            */
};

/******************************************************************************/

static MACHINE_DRIVER_START( battlera )

	/* basic machine hardware */
	MDRV_CPU_ADD(H6280,21477200/3)
	MDRV_CPU_PROGRAM_MAP(battlera_readmem,battlera_writemem)
	MDRV_CPU_IO_MAP(0,battlera_portwrite)
	MDRV_CPU_VBLANK_INT_HACK(battlera_interrupt,256) /* 8 prelines, 232 lines, 16 vblank? */

	MDRV_CPU_ADD(H6280,21477200/3)
	/* audio CPU */
	MDRV_CPU_PROGRAM_MAP(sound_readmem,sound_writemem)

	/* video hardware */
	MDRV_SCREEN_ADD("main", RASTER)
	MDRV_SCREEN_REFRESH_RATE(60)
	MDRV_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(2500) /* not accurate */)
	MDRV_SCREEN_FORMAT(BITMAP_FORMAT_INDEXED16)
	MDRV_SCREEN_SIZE(32*8, 32*8)
	MDRV_SCREEN_VISIBLE_AREA(0*8, 32*8-1, 1*8, 30*8-1)
	MDRV_GFXDECODE(battlera)
	MDRV_PALETTE_LENGTH(512)

	MDRV_VIDEO_START(battlera)
	MDRV_VIDEO_UPDATE(battlera)

	/* sound hardware */
	MDRV_SPEAKER_STANDARD_STEREO("left", "right")

	MDRV_SOUND_ADD(YM2203, 12000000 / 8)
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "left", 0.40)
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "right", 0.40)

	MDRV_SOUND_ADD(MSM5205, 384000)
	MDRV_SOUND_CONFIG(msm5205_interface)
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "left", 0.85)
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "right", 0.85)

	MDRV_SOUND_ADD(C6280, 21477270/6)
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "left", 0.60)
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "right", 0.60)
MACHINE_DRIVER_END

/******************************************************************************/

ROM_START( battlera )
	ROM_REGION( 0x100000, REGION_CPU1, 0 ) /* Main cpu code */
	ROM_LOAD( "00_e1.bin", 0x00000, 0x10000, CRC(aa1cbe69) SHA1(982530f3202bc7b8d94d2b818873b71f02c0e8de) ) /* ET00 */
	ROM_LOAD( "es01.rom",  0x10000, 0x10000, CRC(9fea3189) SHA1(0692df6df533dfe55f61df8aa0c5c11944ba3ae3) ) /* ET01 */
	ROM_LOAD( "02_e4.bin", 0x20000, 0x10000, CRC(cd72f580) SHA1(43b476c8f554348b02aa9558c0773f47cdb47fe0) ) /* ET02, etc */
	/* Rom sockets 0x30000 - 0x70000 are unused */
	ROM_LOAD( "es05.rom",  0x80000, 0x10000, CRC(551fa331) SHA1(a70c627c572ba1b8029f61eae6eaad9825c56339) )
	ROM_LOAD( "es06.rom",  0x90000, 0x10000, CRC(ab91aac8) SHA1(81d820c8b70281a4a52f7ec75a3c54377011d9d9) )
	ROM_LOAD( "es07.rom",  0xa0000, 0x10000, CRC(8d15a3d0) SHA1(afae081ee5e0de359cae6a7ea8401237c5ab7095) )
	ROM_LOAD( "es08.rom",  0xb0000, 0x10000, CRC(38f06039) SHA1(cc394f161b2c4423cd2da763701ceaad7d27f741) )
	ROM_LOAD( "es09.rom",  0xc0000, 0x10000, CRC(b718c47d) SHA1(1d5b2ec819b0848e5b883373887445a63ebddb06) )
	ROM_LOAD( "es10-1.rom",0xd0000, 0x10000, CRC(d3cddc02) SHA1(d212127a9d7aff384171d79c563f1516c0bd46ae) )
	/* Rom sockets 0xe0000 - 0x100000 are unused */

	ROM_REGION( 0x10000, REGION_CPU2, 0 ) /* Sound CPU */
	ROM_LOAD( "es11.rom",  0x00000, 0x10000, CRC(f5b29c9c) SHA1(44dcdf96f8deb9a29aa9d94a8b9cf91a0ed808d4) )
ROM_END

ROM_START( bldwolf )
	ROM_REGION( 0x100000, REGION_CPU1, 0 ) /* Main cpu code */
	ROM_LOAD( "es00-1.rom", 0x00000, 0x10000, CRC(ff4aa252) SHA1(3c190e49020bb6923abb3f3c2632d3c86443c292) )
	ROM_LOAD( "es01.rom",   0x10000, 0x10000, CRC(9fea3189) SHA1(0692df6df533dfe55f61df8aa0c5c11944ba3ae3) )
	ROM_LOAD( "es02-1.rom", 0x20000, 0x10000, CRC(49792753) SHA1(4f3fb6912607d373fc0c1096ac0a8cc939e33617) )
	/* Rom sockets 0x30000 - 0x70000 are unused */
	ROM_LOAD( "es05.rom",   0x80000, 0x10000, CRC(551fa331) SHA1(a70c627c572ba1b8029f61eae6eaad9825c56339) )
	ROM_LOAD( "es06.rom",   0x90000, 0x10000, CRC(ab91aac8) SHA1(81d820c8b70281a4a52f7ec75a3c54377011d9d9) )
	ROM_LOAD( "es07.rom",   0xa0000, 0x10000, CRC(8d15a3d0) SHA1(afae081ee5e0de359cae6a7ea8401237c5ab7095) )
	ROM_LOAD( "es08.rom",   0xb0000, 0x10000, CRC(38f06039) SHA1(cc394f161b2c4423cd2da763701ceaad7d27f741) )
	ROM_LOAD( "es09.rom",   0xc0000, 0x10000, CRC(b718c47d) SHA1(1d5b2ec819b0848e5b883373887445a63ebddb06) )
	ROM_LOAD( "es10-1.rom", 0xd0000, 0x10000, CRC(d3cddc02) SHA1(d212127a9d7aff384171d79c563f1516c0bd46ae) )
	/* Rom sockets 0xe0000 - 0x100000 are unused */

	ROM_REGION( 0x10000, REGION_CPU2, 0 ) /* Sound CPU */
	ROM_LOAD( "es11.rom",   0x00000, 0x10000, CRC(f5b29c9c) SHA1(44dcdf96f8deb9a29aa9d94a8b9cf91a0ed808d4) )
ROM_END

ROM_START( bldwolfj ) /* note, rom codes are ER not ES even if the content of some roms is identical */
	ROM_REGION( 0x100000, REGION_CPU1, 0 ) /* Main cpu code */
	ROM_LOAD( "er00-.0-0", 0x00000, 0x10000, CRC(3819a14e) SHA1(0222051e0b5ec87a18f2e6e9155034f91898c14f) )
	ROM_LOAD( "er01-.0-1", 0x10000, 0x10000, CRC(763cf206) SHA1(0f1c0f80a6aaad0c987c2ba3fdd01db1f5ceb7e6) )
	ROM_LOAD( "er02-.0-2", 0x20000, 0x10000, CRC(bcad8a0f) SHA1(e7c69d2c894eaedd10ce02f6bceaa43bb060afb9) )
	/* Rom sockets 0x30000 - 0x70000 are unused */
	ROM_LOAD( "er05-.1-0", 0x80000, 0x10000, CRC(551fa331) SHA1(a70c627c572ba1b8029f61eae6eaad9825c56339) )
	ROM_LOAD( "er06-.1-1", 0x90000, 0x10000, CRC(ab91aac8) SHA1(81d820c8b70281a4a52f7ec75a3c54377011d9d9) )
	ROM_LOAD( "er07-.1-2", 0xa0000, 0x10000, CRC(8d15a3d0) SHA1(afae081ee5e0de359cae6a7ea8401237c5ab7095) )
	ROM_LOAD( "er08-.1-3", 0xb0000, 0x10000, CRC(38f06039) SHA1(cc394f161b2c4423cd2da763701ceaad7d27f741) )
	ROM_LOAD( "er09-.1-4", 0xc0000, 0x10000, CRC(b718c47d) SHA1(1d5b2ec819b0848e5b883373887445a63ebddb06) )
	ROM_LOAD( "er10-.1-5", 0xd0000, 0x10000, CRC(d3cddc02) SHA1(d212127a9d7aff384171d79c563f1516c0bd46ae) )
	/* Rom sockets 0xe0000 - 0x100000 are unused */

	ROM_REGION( 0x10000, REGION_CPU2, 0 ) /* Sound CPU */
	ROM_LOAD( "er11-.tpg",   0x00000, 0x10000, CRC(f5b29c9c) SHA1(44dcdf96f8deb9a29aa9d94a8b9cf91a0ed808d4) )
ROM_END



/******************************************************************************/

GAME( 1988, battlera, 0,        battlera, battlera,  0,   ROT0, "Data East Corporation", "Battle Rangers (World)", GAME_IMPERFECT_SOUND | GAME_IMPERFECT_GRAPHICS )
GAME( 1988, bldwolf,  battlera, battlera, battlera,  0,   ROT0, "Data East USA", "Bloody Wolf (US)", GAME_IMPERFECT_SOUND | GAME_IMPERFECT_GRAPHICS )
GAME( 1988, bldwolfj, battlera, battlera, battlera,  0,   ROT0, "Data East Corporation", "Narazumono Sentoubutai Bloody Wolf (Japan)", GAME_IMPERFECT_SOUND | GAME_IMPERFECT_GRAPHICS )
