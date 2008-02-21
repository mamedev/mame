/***************************************************************************

Fast Lane(GX752) (c) 1987 Konami

Driver by Manuel Abadia <manu@teleline.es>

TODO:
- is the game playable? How do you end a level?
- colors don't seem 100% accurate.
- verify that sound is correct (volume and bank switching)

***************************************************************************/

#include "driver.h"
#include "deprecat.h"
#include "cpu/hd6309/hd6309.h"
#include "video/konamiic.h"
#include "sound/k007232.h"

/* from video/fastlane.c */
extern UINT8 *fastlane_k007121_regs,*fastlane_videoram1,*fastlane_videoram2;
WRITE8_HANDLER( fastlane_vram1_w );
WRITE8_HANDLER( fastlane_vram2_w );
PALETTE_INIT( fastlane );
VIDEO_START( fastlane );
VIDEO_UPDATE( fastlane );

static INTERRUPT_GEN( fastlane_interrupt )
{
	if (cpu_getiloops() == 0)
	{
		if (K007121_ctrlram[0][0x07] & 0x02)
			cpunum_set_input_line(machine, 0, HD6309_IRQ_LINE, HOLD_LINE);
	}
	else if (cpu_getiloops() % 2)
	{
		if (K007121_ctrlram[0][0x07] & 0x01)
			cpunum_set_input_line(machine, 0, INPUT_LINE_NMI, PULSE_LINE);
	}
}

static WRITE8_HANDLER( k007121_registers_w )
{
	if (offset < 8)
		K007121_ctrl_0_w(offset,data);
	else	/* scroll registers */
		fastlane_k007121_regs[offset] = data;
}

static WRITE8_HANDLER( fastlane_bankswitch_w )
{
	int bankaddress;
	UINT8 *RAM = memory_region(REGION_CPU1);

	/* bits 0 & 1 coin counters */
	coin_counter_w(0,data & 0x01);
	coin_counter_w(1,data & 0x02);

	/* bits 2 & 3 = bank number */
	bankaddress = 0x10000 + ((data & 0x0c) >> 2) * 0x4000;
	memory_set_bankptr(1,&RAM[bankaddress]);

	/* bit 4: bank # for the 007232 (chip 2) */
	K007232_set_bank(1,0 + ((data & 0x10) >> 4),2 + ((data & 0x10) >> 4));

	/* other bits seems to be unused */
}

/* Read and write handlers for one K007232 chip:
   even and odd register are mapped swapped */

static READ8_HANDLER( fastlane_K007232_read_port_0_r )
{
	return K007232_read_port_0_r(offset ^ 1);
}
static WRITE8_HANDLER( fastlane_K007232_write_port_0_w )
{
	K007232_write_port_0_w(offset ^ 1, data);
}
static READ8_HANDLER( fastlane_K007232_read_port_1_r )
{
	return K007232_read_port_1_r(offset ^ 1);
}
static WRITE8_HANDLER( fastlane_K007232_write_port_1_w )
{
	K007232_write_port_1_w(offset ^ 1, data);
}



static ADDRESS_MAP_START( fastlane_readmem, ADDRESS_SPACE_PROGRAM, 8 )
	AM_RANGE(0x0000, 0x005f) AM_READ(MRA8_RAM)
	AM_RANGE(0x0800, 0x0800) AM_READ(input_port_2_r) 	/* DIPSW #3 */
	AM_RANGE(0x0801, 0x0801) AM_READ(input_port_5_r) 	/* 2P inputs */
	AM_RANGE(0x0802, 0x0802) AM_READ(input_port_4_r) 	/* 1P inputs */
	AM_RANGE(0x0803, 0x0803) AM_READ(input_port_3_r) 	/* COINSW */
	AM_RANGE(0x0900, 0x0900) AM_READ(input_port_0_r) 	/* DIPSW #1 */
	AM_RANGE(0x0901, 0x0901) AM_READ(input_port_1_r) 	/* DISPW #2 */
	AM_RANGE(0x0d00, 0x0d0d) AM_READ(fastlane_K007232_read_port_0_r)/* 007232 registers (chip 1) */
	AM_RANGE(0x0e00, 0x0e0d) AM_READ(fastlane_K007232_read_port_1_r)/* 007232 registers (chip 2) */
	AM_RANGE(0x0f00, 0x0f1f) AM_READ(K051733_r)			/* 051733 (protection) */
	AM_RANGE(0x1000, 0x1fff) AM_READ(MRA8_RAM)			/* Palette RAM/Work RAM */
	AM_RANGE(0x2000, 0x3fff) AM_READ(MRA8_RAM)			/* Video RAM + Sprite RAM */
	AM_RANGE(0x4000, 0x7fff) AM_READ(MRA8_BANK1)			/* banked ROM */
	AM_RANGE(0x8000, 0xffff) AM_READ(MRA8_ROM)			/* ROM */
ADDRESS_MAP_END

static ADDRESS_MAP_START( fastlane_writemem, ADDRESS_SPACE_PROGRAM, 8 )
	AM_RANGE(0x0000, 0x005f) AM_WRITE(k007121_registers_w) AM_BASE(&fastlane_k007121_regs)/* 007121 registers */
	AM_RANGE(0x0b00, 0x0b00) AM_WRITE(watchdog_reset_w)		/* watchdog reset */
	AM_RANGE(0x0c00, 0x0c00) AM_WRITE(fastlane_bankswitch_w)	/* bankswitch control */
	AM_RANGE(0x0d00, 0x0d0d) AM_WRITE(fastlane_K007232_write_port_0_w)	/* 007232 registers (chip 1) */
	AM_RANGE(0x0e00, 0x0e0d) AM_WRITE(fastlane_K007232_write_port_1_w)	/* 007232 registers (chip 2) */
	AM_RANGE(0x0f00, 0x0f1f) AM_WRITE(K051733_w)				/* 051733 (protection) */
	AM_RANGE(0x1000, 0x17ff) AM_WRITE(MWA8_RAM) AM_BASE(&paletteram)/* palette RAM */
	AM_RANGE(0x1800, 0x1fff) AM_WRITE(MWA8_RAM)				/* Work RAM */
	AM_RANGE(0x2000, 0x27ff) AM_WRITE(fastlane_vram1_w) AM_BASE(&fastlane_videoram1)
	AM_RANGE(0x2800, 0x2fff) AM_WRITE(fastlane_vram2_w) AM_BASE(&fastlane_videoram2)
	AM_RANGE(0x3000, 0x3fff) AM_WRITE(MWA8_RAM) AM_BASE(&spriteram)	/* Sprite RAM */
	AM_RANGE(0x4000, 0xffff) AM_WRITE(MWA8_ROM)				/* ROM/banked ROM */
ADDRESS_MAP_END

/***************************************************************************

    Input Ports

***************************************************************************/

static INPUT_PORTS_START( fastlane )
	PORT_START	/* DSW #1 */
	PORT_DIPNAME( 0x0f, 0x0f, DEF_STR( Coin_A ) )
	PORT_DIPSETTING(	0x02, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(	0x05, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(	0x08, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(	0x04, DEF_STR( 3C_2C ) )
	PORT_DIPSETTING(	0x01, DEF_STR( 4C_3C ) )
	PORT_DIPSETTING(	0x0f, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(	0x03, DEF_STR( 3C_4C ) )
	PORT_DIPSETTING(	0x07, DEF_STR( 2C_3C ) )
	PORT_DIPSETTING(	0x0e, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(	0x06, DEF_STR( 2C_5C ) )
	PORT_DIPSETTING(	0x0d, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(	0x0c, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(	0x0b, DEF_STR( 1C_5C ) )
	PORT_DIPSETTING(	0x0a, DEF_STR( 1C_6C ) )
	PORT_DIPSETTING(	0x09, DEF_STR( 1C_7C ) )
	PORT_DIPSETTING(	0x00, DEF_STR( Free_Play ) )
	PORT_DIPNAME( 0xf0, 0xf0, DEF_STR( Coin_B ) )
	PORT_DIPSETTING(	0x20, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(	0x50, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(	0x80, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(	0x40, DEF_STR( 3C_2C ) )
	PORT_DIPSETTING(	0x10, DEF_STR( 4C_3C ) )
	PORT_DIPSETTING(	0xf0, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(	0x30, DEF_STR( 3C_4C ) )
	PORT_DIPSETTING(	0x70, DEF_STR( 2C_3C ) )
	PORT_DIPSETTING(	0xe0, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(	0x60, DEF_STR( 2C_5C ) )
	PORT_DIPSETTING(	0xd0, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(	0xc0, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(	0xb0, DEF_STR( 1C_5C ) )
	PORT_DIPSETTING(	0xa0, DEF_STR( 1C_6C ) )
	PORT_DIPSETTING(	0x90, DEF_STR( 1C_7C ) )
//  PORT_DIPSETTING(    0x00, "Invalid" )

	PORT_START	/* DSW #2 */
	PORT_DIPNAME( 0x03, 0x02, DEF_STR( Lives ) )
	PORT_DIPSETTING(	0x03, "2" )
	PORT_DIPSETTING(	0x02, "3" )
	PORT_DIPSETTING(	0x01, "4" )
	PORT_DIPSETTING(	0x00, "7" )
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Unknown ) )
	PORT_DIPSETTING(	0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(	0x00, DEF_STR( On ) )
	/* The bonus life affects the starting high score too, 20000 or 30000 */
	PORT_DIPNAME( 0x18, 0x18, DEF_STR( Bonus_Life ) )
	PORT_DIPSETTING(	0x18, "20000 100000" )
	PORT_DIPSETTING(	0x10, "30000 150000" )
	PORT_DIPSETTING(	0x08, "20000" )
	PORT_DIPSETTING(	0x00, "30000" )
	PORT_DIPNAME( 0x60, 0x40, DEF_STR( Difficulty ) )
	PORT_DIPSETTING(	0x60, DEF_STR( Easy ) )
	PORT_DIPSETTING(	0x40, DEF_STR( Medium ) )
	PORT_DIPSETTING(	0x20, DEF_STR( Hard ) )
	PORT_DIPSETTING(	0x00, DEF_STR( Hardest ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unknown ) )
	PORT_DIPSETTING(	0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(	0x00, DEF_STR( On ) )

	PORT_START	/* DSW #3 */
	PORT_DIPNAME( 0x01, 0x01, DEF_STR( Flip_Screen ) )
	PORT_DIPSETTING(	0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(	0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, DEF_STR( Unknown ) )
	PORT_DIPSETTING(	0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(	0x00, DEF_STR( On ) )
	PORT_SERVICE( 0x04, IP_ACTIVE_LOW )
	PORT_DIPNAME( 0x08, 0x08, "Continue" )
	PORT_DIPSETTING(	0x08, "3" )
	PORT_DIPSETTING(	0x00, DEF_STR( Infinite ) )
	PORT_BIT( 0xf0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START	/* COINSW */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_SERVICE1 )	/* service */
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0xc0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START	/* PLAYER 1 INPUTS */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_UP	 ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(1)
	PORT_BIT( 0xc0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START	/* PLAYER 2 INPUTS */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_UP	 ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2)
	PORT_BIT( 0xc0, IP_ACTIVE_LOW, IPT_UNUSED )

INPUT_PORTS_END

static const gfx_layout gfxlayout =
{
	8,8,
	0x80000/32,
	4,
	{ 0, 1, 2, 3 },
	{ 2*4, 3*4, 0*4, 1*4, 6*4, 7*4, 4*4, 5*4 },
	{ 0*32, 1*32, 2*32, 3*32, 4*32, 5*32, 6*32, 7*32 },
	32*8
};

static GFXDECODE_START( fastlane )
	GFXDECODE_ENTRY( REGION_GFX1, 0, gfxlayout, 0, 64*16 )
GFXDECODE_END

/***************************************************************************

    Machine Driver

***************************************************************************/

static void volume_callback0(int v)
{
	K007232_set_volume(0,0,(v >> 4) * 0x11,0);
	K007232_set_volume(0,1,0,(v & 0x0f) * 0x11);
}

static void volume_callback1(int v)
{
	K007232_set_volume(1,0,(v >> 4) * 0x11,0);
	K007232_set_volume(1,1,0,(v & 0x0f) * 0x11);
}

static const struct K007232_interface k007232_interface_1 =
{
	REGION_SOUND1,
	volume_callback0
};

static const struct K007232_interface k007232_interface_2 =
{
	REGION_SOUND2,
	volume_callback1
};

static MACHINE_DRIVER_START( fastlane )

	/* basic machine hardware */
	MDRV_CPU_ADD(HD6309, 3000000*4)		/* 24MHz/8? */
	MDRV_CPU_PROGRAM_MAP(fastlane_readmem,fastlane_writemem)
	MDRV_CPU_VBLANK_INT(fastlane_interrupt,16)	/* 1 IRQ + ??? NMI (generated by the 007121) */

	/* video hardware */
	MDRV_SCREEN_ADD("main", RASTER)
	MDRV_SCREEN_REFRESH_RATE(60)
	MDRV_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MDRV_SCREEN_FORMAT(BITMAP_FORMAT_INDEXED16)
	MDRV_SCREEN_SIZE(37*8, 32*8)
	MDRV_SCREEN_VISIBLE_AREA(0*8, 35*8-1, 2*8, 30*8-1)

	MDRV_GFXDECODE(fastlane)
	MDRV_PALETTE_LENGTH(1024*16)

	MDRV_PALETTE_INIT(fastlane)
	MDRV_VIDEO_START(fastlane)
	MDRV_VIDEO_UPDATE(fastlane)

	/* sound hardware */
	MDRV_SPEAKER_STANDARD_MONO("mono")

	MDRV_SOUND_ADD(K007232, 3579545)
	MDRV_SOUND_CONFIG(k007232_interface_1)
	MDRV_SOUND_ROUTE(0, "mono", 0.50)
	MDRV_SOUND_ROUTE(1, "mono", 0.50)

	MDRV_SOUND_ADD(K007232, 3579545)
	MDRV_SOUND_CONFIG(k007232_interface_2)
	MDRV_SOUND_ROUTE(0, "mono", 0.50)
	MDRV_SOUND_ROUTE(1, "mono", 0.50)
MACHINE_DRIVER_END


/***************************************************************************

  Game ROMs

***************************************************************************/

ROM_START( fastlane )
	ROM_REGION( 0x21000, REGION_CPU1, 0 ) /* code + banked roms */
	ROM_LOAD( "752_m02.9h",  0x08000, 0x08000, CRC(e1004489) SHA1(615b608d22abc3611f1620503cd6a8c9a6218db8) )  /* fixed ROM */
	ROM_LOAD( "752_e01.10h", 0x10000, 0x10000, CRC(ff4d6029) SHA1(b5c5d8654ce728300d268628bd3dd878570ba7b8) )  /* banked ROM */

	ROM_REGION( 0x80000, REGION_GFX1, ROMREGION_DISPOSE )
	ROM_LOAD( "752e04.2i",   0x00000, 0x80000, CRC(a126e82d) SHA1(6663230c2c36dec563969bccad8c62e3d454d240) )  /* tiles + sprites */

	ROM_REGION( 0x0100, REGION_PROMS, 0 )
	ROM_LOAD( "752e03.6h",   0x0000, 0x0100, CRC(44300aeb) SHA1(580c6e88cbb3b6d8156ea0b9103834f199ec2747) )

	ROM_REGION( 0x20000, REGION_SOUND1, 0 )	/* 007232 data */
	ROM_LOAD( "752e06.4c",   0x00000, 0x20000, CRC(85d691ed) SHA1(7f8d05562a68c75672141fc80ce7e7acb80588b9) ) /* chip 1 */

	ROM_REGION( 0x80000, REGION_SOUND2, 0 )	/* 007232 data */
	ROM_LOAD( "752e05.12b",  0x00000, 0x80000, CRC(119e9cbf) SHA1(21e3def9ab10b210632df11b6df4699140c473db) ) /* chip 2 */
ROM_END



GAME( 1987, fastlane, 0, fastlane, fastlane, 0, ROT90, "Konami", "Fast Lane", GAME_IMPERFECT_COLORS )
