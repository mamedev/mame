/***************************************************************************

Flak Attack/MX5000 (Konami GX669)

Driver by:
    Manuel Abadia <manu@teleline.es>

TO DO:
    -What does 0x900X do? (Z80)

***************************************************************************/

#include "driver.h"
#include "deprecat.h"
#include "cpu/z80/z80.h"
#include "cpu/hd6309/hd6309.h"
#include "sound/2151intf.h"
#include "sound/k007232.h"

/* from video/flkatck.c */
VIDEO_START( flkatck );
VIDEO_UPDATE( flkatck );
WRITE8_HANDLER( flkatck_k007121_w );
WRITE8_HANDLER( flkatck_k007121_regs_w );

extern UINT8 *k007121_ram;
extern int flkatck_irq_enabled;

static int multiply_reg[2];

/***************************************************************************/

static MACHINE_RESET( flkatck )
{
	K007232_set_bank( 0, 0, 1 );
}

static INTERRUPT_GEN( flkatck_interrupt )
{
	if (flkatck_irq_enabled)
		cpunum_set_input_line(machine, 0, HD6309_IRQ_LINE, HOLD_LINE);
}

static WRITE8_HANDLER( flkatck_bankswitch_w )
{
	UINT8 *RAM = memory_region(REGION_CPU1);
	int bankaddress = 0;

	/* bits 3-4: coin counters */
	coin_counter_w(0,data & 0x08);
	coin_counter_w(1,data & 0x10);

	/* bits 0-1: bank # */
	bankaddress += 0x10000 + (data & 0x03)*0x2000;
	if ((data & 0x03) != 0x03)	/* for safety */
		memory_set_bankptr(1,&RAM[bankaddress]);
}

static READ8_HANDLER( flkatck_ls138_r )
{
	int data = 0;

	switch ((offset & 0x1c) >> 2){
		case 0x00:	/* inputs + DIPSW #3 + coinsw */
			if (offset & 0x02)
				data = input_port_read_indexed(machine, 2 + (offset & 0x01));
			else
				data = input_port_read_indexed(machine, 4 + (offset & 0x01));
			break;
		case 0x01:	/* DIPSW #1 & DIPSW #2 */
			if (offset & 0x02)
				data = input_port_read_indexed(machine, 1 - (offset & 0x01));
			break;
	}

	return data;
}

static WRITE8_HANDLER( flkatck_ls138_w )
{
	switch ((offset & 0x1c) >> 2){
		case 0x04:	/* bankswitch */
			flkatck_bankswitch_w(machine, 0, data);
			break;
		case 0x05:	/* sound code number */
			soundlatch_w(machine, 0, data);
			break;
		case 0x06:	/* Cause interrupt on audio CPU */
			cpunum_set_input_line(machine, 1,0,HOLD_LINE);
			break;
		case 0x07:	/* watchdog reset */
			watchdog_reset_w(machine, 0, data);
			break;
	}
}

/* Protection - an external multiplyer connected to the sound CPU */
static READ8_HANDLER( multiply_r )
{
	return (multiply_reg[0] * multiply_reg[1]) & 0xFF;
}

static WRITE8_HANDLER( multiply_w )
{
	multiply_reg[offset] = data;
}


static ADDRESS_MAP_START( flkatck_readmem, ADDRESS_SPACE_PROGRAM, 8 )
	AM_RANGE(0x0000, 0x03ff) AM_READ(SMH_RAM)
	AM_RANGE(0x0400, 0x041f) AM_READ(flkatck_ls138_r)			/* inputs + DIPS */
	AM_RANGE(0x0800, 0x0bff) AM_READ(SMH_RAM)		/* palette */
	AM_RANGE(0x1000, 0x1fff) AM_READ(SMH_RAM)					/* RAM */
	AM_RANGE(0x2000, 0x3fff) AM_READ(SMH_RAM)		/* Video RAM (007121) */
	AM_RANGE(0x4000, 0x5fff) AM_READ(SMH_BANK1)					/* banked ROM */
	AM_RANGE(0x6000, 0xffff) AM_READ(SMH_ROM)					/* ROM */
ADDRESS_MAP_END

static ADDRESS_MAP_START( flkatck_writemem, ADDRESS_SPACE_PROGRAM, 8 )
	AM_RANGE(0x0000, 0x0007) AM_WRITE(flkatck_k007121_regs_w) 	/* 007121 registers */
	AM_RANGE(0x0000, 0x03ff) AM_WRITE(SMH_RAM)
	AM_RANGE(0x0400, 0x041f) AM_WRITE(flkatck_ls138_w)			/* bankswitch + counters + sound command */
	AM_RANGE(0x0800, 0x0bff) AM_WRITE(paletteram_xBBBBBGGGGGRRRRR_le_w) AM_BASE(&paletteram)/* palette */
	AM_RANGE(0x1000, 0x1fff) AM_WRITE(SMH_RAM)					/* RAM */
	AM_RANGE(0x2000, 0x3fff) AM_WRITE(flkatck_k007121_w) AM_BASE(&k007121_ram)			/* Video RAM (007121) */
	AM_RANGE(0x4000, 0x5fff) AM_WRITE(SMH_BANK1)					/* banked ROM */
	AM_RANGE(0x6000, 0xffff) AM_WRITE(SMH_ROM)					/* ROM */
ADDRESS_MAP_END

static ADDRESS_MAP_START( flkatck_readmem_sound, ADDRESS_SPACE_PROGRAM, 8 )
	AM_RANGE(0x0000, 0x7fff) AM_READ(SMH_ROM)				/* ROM */
	AM_RANGE(0x8000, 0x87ff) AM_READ(SMH_RAM)				/* RAM */
	AM_RANGE(0x9000, 0x9000) AM_READ(multiply_r)				/* ??? */
//  AM_RANGE(0x9001, 0x9001) AM_READ(SMH_RAM)               /* ??? */
	AM_RANGE(0x9004, 0x9004) AM_READ(SMH_RAM)				/* ??? */
	AM_RANGE(0xa000, 0xa000) AM_READ(soundlatch_r)			/* soundlatch_r */
	AM_RANGE(0xb000, 0xb00d) AM_READ(K007232_read_port_0_r)	/* 007232 registers */
	AM_RANGE(0xc001, 0xc001) AM_READ(YM2151_status_port_0_r) /* YM2151 */
ADDRESS_MAP_END

static ADDRESS_MAP_START( flkatck_writemem_sound, ADDRESS_SPACE_PROGRAM, 8 )
	AM_RANGE(0x0000, 0x7fff) AM_WRITE(SMH_ROM)					/* ROM */
	AM_RANGE(0x8000, 0x87ff) AM_WRITE(SMH_RAM)					/* RAM */
	AM_RANGE(0x9000, 0x9001) AM_WRITE(multiply_w)					/* ??? */
//  AM_RANGE(0x9001, 0x9001) AM_WRITE(SMH_RAM)                  /* ??? */
	AM_RANGE(0x9006, 0x9006) AM_WRITE(SMH_RAM)					/* ??? */
	AM_RANGE(0xb000, 0xb00d) AM_WRITE(K007232_write_port_0_w) 	/* 007232 registers */
	AM_RANGE(0xc000, 0xc000) AM_WRITE(YM2151_register_port_0_w)	/* YM2151 */
	AM_RANGE(0xc001, 0xc001) AM_WRITE(YM2151_data_port_0_w)		/* YM2151 */
ADDRESS_MAP_END


static INPUT_PORTS_START( flkatck )
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
	//PORT_DIPSETTING(    0x00, "Invalid" )

	PORT_START	/* DSW #2 */
	PORT_DIPNAME( 0x03, 0x01, DEF_STR( Lives ) )
	PORT_DIPSETTING(	0x03, "1" )
	PORT_DIPSETTING(	0x02, "2" )
	PORT_DIPSETTING(	0x01, "3" )
	PORT_DIPSETTING(	0x00, "5" )
	PORT_DIPNAME( 0x04, 0x00, DEF_STR( Cabinet ) )
	PORT_DIPSETTING(	0x00, DEF_STR( Upright ) )
	PORT_DIPSETTING(	0x04, DEF_STR( Cocktail ) )
	PORT_DIPNAME( 0x18, 0x10, DEF_STR( Bonus_Life ) )
	PORT_DIPSETTING(	0x18, "30000 70000" )
	PORT_DIPSETTING(	0x10, "40000 80000" )
	PORT_DIPSETTING(	0x08, "30000" )
	PORT_DIPSETTING(	0x00, "40000" )
	PORT_DIPNAME( 0x60, 0x40, DEF_STR( Difficulty ) )
	PORT_DIPSETTING(	0x60, DEF_STR( Easy ) )
	PORT_DIPSETTING(	0x40, DEF_STR( Normal ) )
	PORT_DIPSETTING(	0x20, "Difficult" )
	PORT_DIPSETTING(	0x00, "Very difficult" )
	PORT_DIPNAME( 0x80, 0x00, DEF_STR( Demo_Sounds ) )
	PORT_DIPSETTING(	0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(	0x00, DEF_STR( On ) )

	PORT_START	/* DSW #3 */
	PORT_DIPNAME( 0x01, 0x01, DEF_STR( Flip_Screen ) )
	PORT_DIPSETTING(	0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(	0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, "Upright Controls" )
	PORT_DIPSETTING(	0x02, DEF_STR( Single ) )
	PORT_DIPSETTING(	0x00, DEF_STR( Dual ) )
	PORT_SERVICE( 0x04, IP_ACTIVE_LOW )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Unknown ) )
	PORT_DIPSETTING(	0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(	0x00, DEF_STR( On ) )
	PORT_BIT( 0xf0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START	/* COINSW & START */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_COIN3 )	/* service */
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START	/* PLAYER 1 INPUTS */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_UP	 ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(1)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START	/* PLAYER 2 INPUTS */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_UP	 ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNUSED )
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

static GFXDECODE_START( flkatck )
	GFXDECODE_ENTRY( REGION_GFX1, 0, gfxlayout, 0, 32 )
GFXDECODE_END

static void volume_callback0(int v)
{
	K007232_set_volume(0,0,(v >> 4) * 0x11,0);
	K007232_set_volume(0,1,0,(v & 0x0f) * 0x11);
}

static const struct K007232_interface k007232_interface =
{
	REGION_SOUND1,		/* memory region */
	volume_callback0	/* external port callback */
};


static MACHINE_DRIVER_START( flkatck )

	/* basic machine hardware */
	MDRV_CPU_ADD(HD6309,3000000*4) /* HD63C09EP, 24/8 MHz */
	MDRV_CPU_PROGRAM_MAP(flkatck_readmem,flkatck_writemem)
	MDRV_CPU_VBLANK_INT("main", flkatck_interrupt)

	MDRV_CPU_ADD(Z80,3579545)	/* NEC D780C-1, 3.579545 MHz */
	MDRV_CPU_PROGRAM_MAP(flkatck_readmem_sound,flkatck_writemem_sound)

	MDRV_INTERLEAVE(10)

	MDRV_MACHINE_RESET(flkatck)

	/* video hardware */
	MDRV_SCREEN_ADD("main", RASTER)
	MDRV_SCREEN_REFRESH_RATE(60)
	MDRV_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MDRV_SCREEN_FORMAT(BITMAP_FORMAT_INDEXED16)
	MDRV_SCREEN_SIZE(37*8, 32*8)
	MDRV_SCREEN_VISIBLE_AREA(0*8, 35*8-1, 2*8, 30*8-1)

	MDRV_GFXDECODE(flkatck)
	MDRV_PALETTE_LENGTH(512)

	MDRV_VIDEO_START(flkatck)
	MDRV_VIDEO_UPDATE(flkatck)

	/* sound hardware */
	MDRV_SPEAKER_STANDARD_STEREO("left", "right")

	MDRV_SOUND_ADD(YM2151, 3579545)
	MDRV_SOUND_ROUTE(0, "left", 1.0)
	MDRV_SOUND_ROUTE(1, "right", 1.0)

	MDRV_SOUND_ADD(K007232, 3579545)
	MDRV_SOUND_CONFIG(k007232_interface)
	MDRV_SOUND_ROUTE(0, "left", 0.50)
	MDRV_SOUND_ROUTE(0, "right", 0.50)
	MDRV_SOUND_ROUTE(1, "left", 0.50)
	MDRV_SOUND_ROUTE(1, "right", 0.50)
MACHINE_DRIVER_END



ROM_START( mx5000 )
	ROM_REGION( 0x18000, REGION_CPU1, 0 )		/* 6309 code */
	ROM_LOAD( "r01",          0x010000, 0x006000, CRC(79b226fc) SHA1(3bc4d93717230fecd54bd08a0c3eeedc1c8f571d) )/* banked ROM */
	ROM_CONTINUE(			  0x006000, 0x00a000 )			/* fixed ROM */

	ROM_REGION( 0x10000, REGION_CPU2, 0 )		/* 64k for the SOUND CPU */
	ROM_LOAD( "m02.bin",        0x000000, 0x008000, CRC(7e11e6b9) SHA1(7a7d65a458b15842a6345388007c8f682aec20a7) )

	ROM_REGION( 0x080000, REGION_GFX1, ROMREGION_DISPOSE )
	ROM_LOAD( "mask4m.bin",     0x000000, 0x080000, CRC(ff1d718b) SHA1(d44fe3ed5a3ba1b3036264e37f9cd3500b706635) )/* tiles + sprites */

	ROM_REGION( 0x040000, REGION_SOUND1, 0 )	/* 007232 data (chip 1) */
	ROM_LOAD( "mask2m.bin",     0x000000, 0x040000, CRC(6d1ea61c) SHA1(9e6eb9ac61838df6e1f74e74bb72f3edf1274aed) )
ROM_END

ROM_START( flkatck )
	ROM_REGION( 0x18000, REGION_CPU1, 0 )		/* 6309 code */
	ROM_LOAD( "gx669_p1.16c", 0x010000, 0x006000, CRC(c5cd2807) SHA1(22ddd911a23954ff2d52552e07323f5f0ddaeead) )/* banked ROM */
	ROM_CONTINUE(			  0x006000, 0x00a000 )			/* fixed ROM */

	ROM_REGION( 0x10000, REGION_CPU2, 0 )		/* 64k for the SOUND CPU */
	ROM_LOAD( "m02.bin",        0x000000, 0x008000, CRC(7e11e6b9) SHA1(7a7d65a458b15842a6345388007c8f682aec20a7) )

	ROM_REGION( 0x080000, REGION_GFX1, ROMREGION_DISPOSE )
	ROM_LOAD( "mask4m.bin",     0x000000, 0x080000, CRC(ff1d718b) SHA1(d44fe3ed5a3ba1b3036264e37f9cd3500b706635) )/* tiles + sprites */

	ROM_REGION( 0x040000, REGION_SOUND1, 0 )	/* 007232 data (chip 1) */
	ROM_LOAD( "mask2m.bin",     0x000000, 0x040000, CRC(6d1ea61c) SHA1(9e6eb9ac61838df6e1f74e74bb72f3edf1274aed) )
ROM_END



GAME( 1987, mx5000,  0, 	 flkatck, flkatck, 0, ROT90, "Konami", "MX5000", 0 )
GAME( 1987, flkatck, mx5000, flkatck, flkatck, 0, ROT90, "Konami", "Flak Attack (Japan)", 0 )

