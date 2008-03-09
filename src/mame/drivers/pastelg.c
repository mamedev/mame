/******************************************************************************

    Game Driver for Nichibutsu Mahjong series.

    Pastel Gal
    (c)1985 Nihon Bussan Co.,Ltd.

    Driver by Takahiro Nogi <nogi@kt.rim.or.jp> 2000/06/07 -

******************************************************************************/
/******************************************************************************
Memo:

- Custom chip used by pastelg PCB is 1411M1.

- Some games display "GFXROM BANK OVER!!" or "GFXROM ADDRESS OVER!!"
  in Debug build.

- Screen flip is not perfect.

******************************************************************************/

#include "driver.h"
#include "nb1413m3.h"
#include "sound/ay8910.h"
#include "sound/dac.h"


#define SIGNED_DAC	0		// 0:unsigned DAC, 1:signed DAC
#if SIGNED_DAC
#define DAC_0_WRITE	DAC_0_signed_data_w
#else
#define DAC_0_WRITE	DAC_0_data_w
#endif


extern PALETTE_INIT( pastelg );
extern VIDEO_UPDATE( pastelg );
extern VIDEO_START( pastelg );

extern WRITE8_HANDLER( pastelg_clut_w );
extern WRITE8_HANDLER( pastelg_romsel_w );
extern WRITE8_HANDLER( pastelg_blitter_w );
extern int pastelg_blitter_src_addr_r(void);


static DRIVER_INIT( pastelg )
{
	nb1413m3_type = NB1413M3_PASTELG;
}

static READ8_HANDLER( pastelg_sndrom_r )
{
	UINT8 *ROM = memory_region(REGION_SOUND1);

	return ROM[pastelg_blitter_src_addr_r() & 0x7fff];
}


static ADDRESS_MAP_START( readmem_pastelg, ADDRESS_SPACE_PROGRAM, 8 )
	AM_RANGE(0x0000, 0xbfff) AM_READ(MRA8_ROM)
	AM_RANGE(0xe000, 0xe7ff) AM_READ(MRA8_RAM)
ADDRESS_MAP_END

static ADDRESS_MAP_START( writemem_pastelg, ADDRESS_SPACE_PROGRAM, 8 )
	AM_RANGE(0x0000, 0xbfff) AM_WRITE(MWA8_ROM)
	AM_RANGE(0xe000, 0xe7ff) AM_WRITE(MWA8_RAM) AM_BASE(&nb1413m3_nvram) AM_SIZE(&nb1413m3_nvram_size)
ADDRESS_MAP_END


static ADDRESS_MAP_START( readport_pastelg, ADDRESS_SPACE_IO, 8 )
	ADDRESS_MAP_GLOBAL_MASK(0xff)
	AM_RANGE(0x00, 0x7f) AM_READ(nb1413m3_sndrom_r)
	AM_RANGE(0x81, 0x81) AM_READ(AY8910_read_port_0_r)
	AM_RANGE(0x90, 0x90) AM_READ(nb1413m3_inputport0_r)
	AM_RANGE(0xa0, 0xa0) AM_READ(nb1413m3_inputport1_r)
	AM_RANGE(0xb0, 0xb0) AM_READ(nb1413m3_inputport2_r)
	AM_RANGE(0xc0, 0xc0) AM_READ(pastelg_sndrom_r)
	AM_RANGE(0xd0, 0xd0) AM_READ(MRA8_NOP)					// unknown
	AM_RANGE(0xe0, 0xe0) AM_READ(input_port_2_r)
ADDRESS_MAP_END

static ADDRESS_MAP_START( writeport_pastelg, ADDRESS_SPACE_IO, 8 )
	ADDRESS_MAP_GLOBAL_MASK(0xff)
//  AM_RANGE(0x00, 0x00) AM_WRITE(MWA8_NOP)
	AM_RANGE(0x82, 0x82) AM_WRITE(AY8910_write_port_0_w)
	AM_RANGE(0x83, 0x83) AM_WRITE(AY8910_control_port_0_w)
	AM_RANGE(0x90, 0x96) AM_WRITE(pastelg_blitter_w)
	AM_RANGE(0xa0, 0xa0) AM_WRITE(nb1413m3_inputportsel_w)
	AM_RANGE(0xb0, 0xb0) AM_WRITE(pastelg_romsel_w)
	AM_RANGE(0xc0, 0xcf) AM_WRITE(pastelg_clut_w)
	AM_RANGE(0xd0, 0xd0) AM_WRITE(DAC_0_WRITE)
ADDRESS_MAP_END


static INPUT_PORTS_START( pastelg )
	PORT_START	/* (0) DIPSW-A */
	PORT_DIPNAME( 0x03, 0x03, DEF_STR( Difficulty ) )
	PORT_DIPSETTING(    0x03, "1 (Easy)" )
	PORT_DIPSETTING(    0x02, "2" )
	PORT_DIPSETTING(    0x01, "3" )
	PORT_DIPSETTING(    0x00, "4 (Hard)" )
	PORT_DIPNAME( 0x04, 0x04, "DIPSW 1-3" )
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, "DIPSW 1-4" )
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, "DIPSW 1-5" )
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, "DIPSW 1-6" )
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, "DIPSW 1-7" )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, "DIPSW 1-8" )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START	/* (1) DIPSW-B */
	PORT_DIPNAME( 0x03, 0x00, "Number of last chance" )
	PORT_DIPSETTING(    0x03, "0" )
	PORT_DIPSETTING(    0x02, "1" )
	PORT_DIPSETTING(    0x01, "3" )
	PORT_DIPSETTING(    0x00, "10" )
	PORT_DIPNAME( 0x04, 0x04, "No. of tiles on final match" )
	PORT_DIPSETTING(    0x04, "20" )
	PORT_DIPSETTING(    0x00, "10" )
	PORT_DIPNAME( 0x08, 0x08, "DIPSW 2-4" )
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, "DIPSW 2-5" )
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x60, 0x00, "SANGEN Rush" )
	PORT_DIPSETTING(    0x60, "0" )
	PORT_DIPSETTING(    0x40, "1" )
	PORT_DIPSETTING(    0x20, "5" )
	PORT_DIPSETTING(    0x00, DEF_STR( Infinite ) )
	PORT_DIPNAME( 0x80, 0x80, "DIPSW 2-8" )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START	/* (2) DIPSW-C */
	PORT_DIPNAME( 0x03, 0x03, "Change Rate" )
	PORT_DIPSETTING(    0x03, "Type-A" )
	PORT_DIPSETTING(    0x02, "Type-B" )
	PORT_DIPSETTING(    0x01, "Type-C" )
	PORT_DIPSETTING(    0x00, "Type-D" )
	PORT_DIPNAME( 0x04, 0x00, "Open CPU's hand on Player's Reach" )
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, "DIPSW 3-4" )
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, "DIPSW 3-5" )
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x60, 0x60, "YAKUMAN cut" )
	PORT_DIPSETTING(    0x60, "10%" )
	PORT_DIPSETTING(    0x40, "30%" )
	PORT_DIPSETTING(    0x20, "50%" )
	PORT_DIPSETTING(    0x00, "90%" )
	PORT_DIPNAME( 0x80, 0x00, "Nudity" )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START	/* (3) PORT 0 */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNUSED )			// DRAW BUSY
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNUSED )			//
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_SERVICE3 )		// MEMORY RESET
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_SERVICE2 )		// ANALYZER
	PORT_SERVICE( 0x10, IP_ACTIVE_LOW )					// TEST
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_COIN1 )			// COIN1
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_SERVICE4 )		// CREDIT CLEAR
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_SERVICE1 )		// SERVICE

	NBMJCTRL_PORT1	/* (4)  PORT 1-1 */
	NBMJCTRL_PORT2	/* (5)  PORT 1-2 */
	NBMJCTRL_PORT3	/* (6)  PORT 1-3 */
	NBMJCTRL_PORT4	/* (7)  PORT 1-4 */
	NBMJCTRL_PORT5	/* (8)  PORT 1-5 */

	NBMJCTRL_PORT6	/* (9)  PORT 2-1 */
	NBMJCTRL_PORT7	/* (10) PORT 2-2 */
	NBMJCTRL_PORT8	/* (11) PORT 2-3 */
	NBMJCTRL_PORT9	/* (12) PORT 2-4 */
	NBMJCTRL_PORT10	/* (13) PORT 2-5 */
INPUT_PORTS_END


static const struct AY8910interface ay8910_interface =
{
	input_port_1_r,
	input_port_0_r
};


static MACHINE_DRIVER_START( pastelg )

	/* basic machine hardware */
	MDRV_CPU_ADD(Z80, 19968000/8)	/* 2.496 MHz ? */
	MDRV_CPU_PROGRAM_MAP(readmem_pastelg, writemem_pastelg)
	MDRV_CPU_IO_MAP(readport_pastelg, writeport_pastelg)
//  MDRV_CPU_VBLANK_INT_HACK(nb1413m3_interrupt,96)  // nmiclock not written, chip is 1411M1 instead of 1413M3
	MDRV_CPU_VBLANK_INT("main", nb1413m3_interrupt)

	MDRV_MACHINE_RESET(nb1413m3)
	MDRV_NVRAM_HANDLER(nb1413m3)

	/* video hardware */
	MDRV_SCREEN_ADD("main", RASTER)
	MDRV_SCREEN_REFRESH_RATE(60)
	MDRV_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MDRV_SCREEN_FORMAT(BITMAP_FORMAT_INDEXED16)
	MDRV_SCREEN_SIZE(256, 256)
	MDRV_SCREEN_VISIBLE_AREA(0, 256-1, 16, 240-1)

	MDRV_PALETTE_LENGTH(32)

	MDRV_PALETTE_INIT(pastelg)
	MDRV_VIDEO_START(pastelg)
	MDRV_VIDEO_UPDATE(pastelg)

	/* sound hardware */
	MDRV_SPEAKER_STANDARD_MONO("mono")

	MDRV_SOUND_ADD(AY8910, 1250000)
	MDRV_SOUND_CONFIG(ay8910_interface)
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.35)

	MDRV_SOUND_ADD(DAC, 0)
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.50)
MACHINE_DRIVER_END


ROM_START( pastelg )
	ROM_REGION( 0x10000, REGION_CPU1, 0 ) /* program */
	ROM_LOAD( "pgal_09.bin",  0x00000, 0x04000, CRC(1e494af3) SHA1(1597a7da22ecfbb1df83cf9d0acc7a8be461bc2c) )
	ROM_LOAD( "pgal_10.bin",  0x04000, 0x04000, CRC(677cccea) SHA1(a294bf4e3c5e74291160a0858371961868afc1d1) )
	ROM_LOAD( "pgal_11.bin",  0x08000, 0x04000, CRC(c2ccea38) SHA1(0374e8aa0e7961426e417ffe6e1a0d8dc7fd9ecf) )

	ROM_REGION( 0x08000, REGION_SOUND1, 0 ) /* voice */
	ROM_LOAD( "pgal_08.bin",  0x00000, 0x08000, CRC(895961a1) SHA1(f02d517f46cc490db02c4feb369e2a386c764297) )

	ROM_REGION( 0x38000, REGION_GFX1, 0 ) /* gfx */
	ROM_LOAD( "pgal_01.bin",  0x00000, 0x08000, CRC(1bb14d52) SHA1(b3974e3c9b56a752ddcb206f7bb2bc658b0e77f1) )
	ROM_LOAD( "pgal_02.bin",  0x08000, 0x08000, CRC(ea85673a) SHA1(85ef2bb736fe5229ce4153197db8a57bca982a8b) )
	ROM_LOAD( "pgal_03.bin",  0x10000, 0x08000, CRC(40011248) SHA1(935f442a47e02bf8c6ccb324c7fad1b481b8b19a) )
	ROM_LOAD( "pgal_04.bin",  0x18000, 0x08000, CRC(10613a66) SHA1(ad11f99f402e5b247d086cfccafea351da30c084) )
	ROM_LOAD( "pgal_05.bin",  0x20000, 0x08000, CRC(6a152703) SHA1(5dd46d876453c5c79f5a382d77234c690da75001) )
	ROM_LOAD( "pgal_06.bin",  0x28000, 0x08000, CRC(f56acfe8) SHA1(2f4ad3990f2d4d4a9fcec7adab119459423b308b) )
	ROM_LOAD( "pgal_07.bin",  0x30000, 0x08000, CRC(fa4226dc) SHA1(2313449521f81a191e87f1e4c0f3473f3c27dd9d) )

	ROM_REGION( 0x0040, REGION_PROMS, 0 ) /* color */
	ROM_LOAD( "pgal_bp1.bin", 0x0000, 0x0020, CRC(2b7fc61a) SHA1(278830e8728ea143208376feb20fff56de88ae1c) )
	ROM_LOAD( "pgal_bp2.bin", 0x0020, 0x0020, CRC(4433021e) SHA1(e0d6619a193d26ad24788d4af5ef01ee89cffacd) )
ROM_END


GAME( 1985, pastelg, 0, pastelg, pastelg, pastelg, ROT0, "Nichibutsu", "Pastel Gal (Japan 851224)", 0 )
