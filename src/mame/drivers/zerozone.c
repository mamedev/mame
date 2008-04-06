/***************************************************************************

Zero Zone memory map

driver by Brad Oliver

CPU 1 : 68000, uses irq 1

0x000000 - 0x01ffff : ROM
0x080000 - 0x08000f : input ports and dipswitches
0x088000 - 0x0881ff : palette RAM, 256 total colors
0x09ce00 - 0x09d9ff : video ram, 48x32
0x0c0000 - 0x0cffff : RAM
0x0f8000 - 0x0f87ff : RAM (unused?)

Stephh's notes :

  IMO, the game only has 2 buttons (1 to rotate the pieces and 1 for help).
  The 3rd button (when the Dip Switch is activated) subs one "line"
  (0x0c0966 for player 1 and 0x0c1082 for player 2) each time it is pressed.
  As I don't see why such thing would REALLY exist, I've added the
  IPF_CHEAT flag for the Dip Switch and the 3rd button of each player.

TODO:
    * adpcm samples don't seem to be playing at the proper tempo - too fast?


***************************************************************************/
#include "driver.h"
#include "deprecat.h"
#include "sound/okim6295.h"

VIDEO_START( zerozone );
VIDEO_UPDATE( zerozone );
WRITE16_HANDLER( zerozone_tilemap_w );
WRITE16_HANDLER( zerozone_tilebank_w );

extern UINT16 *zerozone_videoram;

static READ16_HANDLER( zerozone_input_r )
{
	switch (offset)
	{
		case 0x00:
			return input_port_read_indexed(machine, 0); /* IN0 */
		case 0x01:
			return (input_port_read_indexed(machine, 1) | (input_port_read_indexed(machine, 2) << 8)); /* IN1 & IN2 */
		case 0x04:
			return (input_port_read_indexed(machine, 4) << 8);
		case 0x05:
			return input_port_read_indexed(machine, 3);
	}

logerror("CPU #0 PC %06x: warning - read unmapped memory address %06x\n",activecpu_get_pc(),0x800000+offset);

	return 0x00;
}


static WRITE16_HANDLER( zerozone_sound_w )
{
	if (ACCESSING_BITS_8_15)
	{
		soundlatch_w(machine,offset,data >> 8);
		cpunum_set_input_line_and_vector(machine, 1,0,HOLD_LINE,0xff);
	}
}

static ADDRESS_MAP_START( main_map, ADDRESS_SPACE_PROGRAM, 16 )
	AM_RANGE(0x000000, 0x01ffff) AM_ROM
	AM_RANGE(0x080000, 0x08000f) AM_READ(zerozone_input_r)
	AM_RANGE(0x084000, 0x084001) AM_WRITE(zerozone_sound_w)
	AM_RANGE(0x088000, 0x0881ff) AM_READWRITE(SMH_RAM, paletteram16_RRRRGGGGBBBBRGBx_word_w) AM_BASE(&paletteram16)
	AM_RANGE(0x098000, 0x098001) AM_RAM		/* Watchdog? */
	AM_RANGE(0x09ce00, 0x09ffff) AM_READWRITE(SMH_RAM, zerozone_tilemap_w) AM_BASE(&zerozone_videoram) AM_SIZE(&videoram_size)
	AM_RANGE(0x0b4000, 0x0b4001) AM_WRITE(zerozone_tilebank_w)
	AM_RANGE(0x0c0000, 0x0cffff) AM_RAM
	AM_RANGE(0x0f8000, 0x0f87ff) AM_RAM		/* Never read from */
ADDRESS_MAP_END

static ADDRESS_MAP_START( sound_map, ADDRESS_SPACE_PROGRAM, 8 )
	AM_RANGE(0x0000, 0x7fff) AM_ROM
	AM_RANGE(0x8000, 0x87ff) AM_RAM
	AM_RANGE(0x9800, 0x9800) AM_READWRITE(OKIM6295_status_0_r, OKIM6295_data_0_w)
	AM_RANGE(0xa000, 0xa000) AM_READ(soundlatch_r)
ADDRESS_MAP_END


static INPUT_PORTS_START( zerozone )
	PORT_START_TAG("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START_TAG("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_4WAY PORT_PLAYER(1)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_4WAY PORT_PLAYER(1)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_4WAY PORT_PLAYER(1)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_4WAY PORT_PLAYER(1)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(1)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(1) PORT_NAME("P1 Score Line (Cheat)")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START_TAG("IN2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_4WAY PORT_PLAYER(2)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_4WAY PORT_PLAYER(2)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_4WAY PORT_PLAYER(2)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_4WAY PORT_PLAYER(2)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(2) PORT_NAME("P2 Score Line (Cheat)")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START_TAG("DSWA")
	PORT_DIPNAME( 0x07, 0x07, DEF_STR( Coinage ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 5C_1C ) )
	PORT_DIPSETTING(    0x01, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(    0x02, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x03, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x07, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x06, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x05, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0x04, DEF_STR( 1C_4C ) )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Difficulty ) )
	PORT_DIPSETTING(    0x08, "In Game Default" )		// 130, 162 or 255 "lines"
	PORT_DIPSETTING(    0x00, "Always Hard" )			// 255 "lines"
	PORT_DIPNAME( 0x10, 0x10, "Speed" )
	PORT_DIPSETTING(    0x10, DEF_STR( Normal ) )			// Drop every 20 frames
	PORT_DIPSETTING(    0x00, "Fast" )				// Drop every 18 frames
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Demo_Sounds ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x20, DEF_STR( On ) )
	PORT_DIPUNUSED( 0x40, IP_ACTIVE_LOW )
	PORT_DIPUNUSED( 0x80, IP_ACTIVE_LOW )

	PORT_START_TAG("DSWB")
	PORT_DIPUNUSED( 0x01, IP_ACTIVE_LOW )
	PORT_DIPUNUSED( 0x02, IP_ACTIVE_LOW )
	PORT_DIPNAME( 0x04, 0x04, "Helps" )
	PORT_DIPSETTING(    0x04, "1" )
	PORT_DIPSETTING(    0x00, "2" )
	PORT_DIPNAME( 0x08, 0x08, "Bonus Help" )
	PORT_DIPSETTING(    0x00, "30000" )
	PORT_DIPSETTING(    0x08, DEF_STR( None ) )
	PORT_DIPNAME( 0x10, 0x10, "Activate 'Score Line'? (Cheat)")
	PORT_DIPSETTING(    0x10, DEF_STR( No ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Yes ) )
	PORT_DIPUNUSED( 0x20, IP_ACTIVE_LOW )
	PORT_DIPUNUSED( 0x40, IP_ACTIVE_LOW )
	PORT_SERVICE( 0x80, IP_ACTIVE_LOW )
INPUT_PORTS_END


static const gfx_layout charlayout =
{
	8,8,	/* 8*8 characters */
	RGN_FRAC(1,1),	/* 4096 characters */
	4,	/* 4 bits per pixel */
	{ 0, 1, 2, 3 },
	{ 0, 4, 8+0, 8+4, 16+0, 16+4, 24+0, 24+4 },
	{ 0*32, 1*32, 2*32, 3*32, 4*32, 5*32, 6*32, 7*32 },
	32*8	/* every sprite takes 32 consecutive bytes */
};


static GFXDECODE_START( zerozone )
	GFXDECODE_ENTRY( REGION_GFX1, 0, charlayout, 0, 256 )         /* sprites & playfield */
GFXDECODE_END


static MACHINE_DRIVER_START( zerozone )

	/* basic machine hardware */
	MDRV_CPU_ADD(M68000, 10000000)	/* 10 MHz */
	MDRV_CPU_PROGRAM_MAP(main_map,0)
	MDRV_CPU_VBLANK_INT("main", irq1_line_hold)

	MDRV_CPU_ADD(Z80, 1000000)
	/* audio CPU */	/* 1 MHz ??? */
	MDRV_CPU_PROGRAM_MAP(sound_map,0)

	MDRV_INTERLEAVE(10)

	/* video hardware */
	MDRV_SCREEN_ADD("main", RASTER)
	MDRV_SCREEN_REFRESH_RATE(60)
	MDRV_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MDRV_SCREEN_FORMAT(BITMAP_FORMAT_INDEXED16)
	MDRV_SCREEN_SIZE(64*8, 32*8)
	MDRV_SCREEN_VISIBLE_AREA(1*8, 47*8-1, 2*8, 30*8-1)

	MDRV_GFXDECODE(zerozone)
	MDRV_PALETTE_LENGTH(256)

	MDRV_VIDEO_START(zerozone)
	MDRV_VIDEO_UPDATE(zerozone)

	/* sound hardware */
	MDRV_SPEAKER_STANDARD_MONO("mono")

	MDRV_SOUND_ADD(OKIM6295, 1056000)
	MDRV_SOUND_CONFIG(okim6295_interface_region_1_pin7high) // clock frequency & pin 7 not verified
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "mono", 1.0)
MACHINE_DRIVER_END



/***************************************************************************

  Game driver(s)

***************************************************************************/

ROM_START( zerozone )
	ROM_REGION( 0x20000, REGION_CPU1, 0 )     /* 128k for 68000 code */
	ROM_LOAD16_BYTE( "zz-4.rom", 0x0000, 0x10000, CRC(83718b9b) SHA1(b3fc6da5816142b9c92a7b8615eb5bcb2c78ea46) )
	ROM_LOAD16_BYTE( "zz-5.rom", 0x0001, 0x10000, CRC(18557f41) SHA1(6ef908732b7775c1ea2b33f799635075db5756de) )

	ROM_REGION( 0x10000, REGION_CPU2, 0 )      /* sound cpu */
	ROM_LOAD( "zz-1.rom", 0x00000, 0x08000, CRC(223ccce5) SHA1(3aa25ca914960b929dc853d07a958ed874e42fee) )

	ROM_REGION( 0x080000, REGION_GFX1, ROMREGION_DISPOSE )
	ROM_LOAD( "zz-6.rom", 0x00000, 0x80000, CRC(c8b906b9) SHA1(1775d69df6397d6772b20c65751d44556d76c033) )

	ROM_REGION( 0x40000, REGION_SOUND1, 0 )      /* ADPCM samples */
	ROM_LOAD( "zz-2.rom", 0x00000, 0x20000, CRC(c7551e81) SHA1(520de3074fa6a71fef10d5a76cba5580fd1cbbae) )
	ROM_LOAD( "zz-3.rom", 0x20000, 0x20000, CRC(e348ff5e) SHA1(6d2755d9b31366f4c2ddd296790234deb8f821c8) )
ROM_END

ROM_START( lvgirl94 )
	ROM_REGION( 0x20000, REGION_CPU1, 0 )     /* 128k for 68000 code */
	ROM_LOAD16_BYTE( "rom4", 0x0000, 0x10000, CRC(c4fb449e) SHA1(dd1c567ba2cf951267dd622e2e9af265e742f246) )
	ROM_LOAD16_BYTE( "rom5", 0x0001, 0x10000, CRC(5d446a1a) SHA1(2d7ea25e5b86e7cf4eb7f10daa1eaaaed6830a53) )

	ROM_REGION( 0x080000, REGION_GFX1, ROMREGION_DISPOSE )
	ROM_LOAD( "rom6", 0x00000, 0x40000, CRC(eeeb94ba) SHA1(9da09312c090ef2d40f596247d9a7decf3724e54) )

	/* sound roms are the same as zerozone */
	ROM_REGION( 0x10000, REGION_CPU2, 0 )      /* sound cpu */
	ROM_LOAD( "rom1", 0x00000, 0x08000, CRC(223ccce5) SHA1(3aa25ca914960b929dc853d07a958ed874e42fee) )

	ROM_REGION( 0x40000, REGION_SOUND1, 0 )      /* ADPCM samples */
	ROM_LOAD( "rom2", 0x00000, 0x20000, CRC(c7551e81) SHA1(520de3074fa6a71fef10d5a76cba5580fd1cbbae) )
	ROM_LOAD( "rom3", 0x20000, 0x20000, CRC(e348ff5e) SHA1(6d2755d9b31366f4c2ddd296790234deb8f821c8) )
ROM_END


GAME( 1993, zerozone, 0, zerozone, zerozone, 0, ROT0, "Comad", "Zero Zone", 0 )
GAME( 1994, lvgirl94, 0, zerozone, zerozone, 0, ROT0, "Comad", "Las Vegas Girl (Girl '94)", 0 )
