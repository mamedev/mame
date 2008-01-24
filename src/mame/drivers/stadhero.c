/***************************************************************************

    Stadium Hero (Japan)            (c) 1988 Data East Corporation

    Emulation by Bryan McPhail, mish@tendril.co.uk

***************************************************************************/

#include "driver.h"
#include "cpu/m6502/m6502.h"
#include "sound/2203intf.h"
#include "sound/3812intf.h"
#include "sound/okim6295.h"

/* Video emulation definitions */
VIDEO_START( stadhero );
VIDEO_UPDATE( stadhero );

extern UINT16 *stadhero_pf1_data;
extern UINT16 *stadhero_pf2_control_0;
extern UINT16 *stadhero_pf2_control_1;

WRITE16_HANDLER( stadhero_pf1_data_w );
READ16_HANDLER( stadhero_pf2_data_r );
WRITE16_HANDLER( stadhero_pf2_data_w );

/******************************************************************************/

static READ16_HANDLER( stadhero_control_r )
{
	switch (offset<<1)
	{
		case 0: /* Player 1 & 2 joystick & buttons */
			return (readinputport(0) + (readinputport(1) << 8));

		case 2: /* Credits, start buttons */
			return readinputport(2) | (readinputport(2)<<8);

		case 4: /* Byte 4: Dipswitch bank 2, Byte 5: Dipswitch Bank 1 */
			return (readinputport(3) + (readinputport(4) << 8));
	}

	logerror("CPU #0 PC %06x: warning - read unmapped memory address %06x\n",activecpu_get_pc(),0x30c000+offset);
	return ~0;
}

static WRITE16_HANDLER( stadhero_control_w )
{
	switch (offset<<1)
	{
		case 4: /* Interrupt ack (VBL - IRQ 5) */
			break;
		case 6: /* 6502 sound cpu */
			soundlatch_w(0,data & 0xff);
			cpunum_set_input_line(Machine, 1,INPUT_LINE_NMI,PULSE_LINE);
			break;
		default:
			logerror("CPU #0 PC %06x: warning - write %02x to unmapped memory address %06x\n",activecpu_get_pc(),data,0x30c010+offset);
			break;
	}
}


/******************************************************************************/

static ADDRESS_MAP_START( main_map, ADDRESS_SPACE_PROGRAM, 16 )
	AM_RANGE(0x000000, 0x01ffff) AM_ROM
	AM_RANGE(0x200000, 0x2007ff) AM_READWRITE(MRA16_RAM, stadhero_pf1_data_w) AM_BASE(&stadhero_pf1_data)
	AM_RANGE(0x240000, 0x240007) AM_READWRITE(MRA16_RAM, MWA16_RAM) AM_BASE(&stadhero_pf2_control_0)
	AM_RANGE(0x240010, 0x240017) AM_WRITE(MWA16_RAM) AM_BASE(&stadhero_pf2_control_1)
	AM_RANGE(0x260000, 0x261fff) AM_READWRITE(stadhero_pf2_data_r, stadhero_pf2_data_w)
	AM_RANGE(0x30c000, 0x30c00b) AM_READWRITE(stadhero_control_r, stadhero_control_w)
	AM_RANGE(0x310000, 0x3107ff) AM_READWRITE(MRA16_RAM, paletteram16_xxxxBBBBGGGGRRRR_word_w) AM_BASE(&paletteram16)
	AM_RANGE(0xff8000, 0xffbfff) AM_RAM /* Main ram */
	AM_RANGE(0xffc000, 0xffc7ff) AM_MIRROR(0x000800) AM_RAM AM_BASE(&spriteram16)
ADDRESS_MAP_END

/******************************************************************************/

static WRITE8_HANDLER( YM3812_w )
{
	switch (offset) {
	case 0:
		YM3812_control_port_0_w(0,data);
		break;
	case 1:
		YM3812_write_port_0_w(0,data);
		break;
	}
}

static WRITE8_HANDLER( YM2203_w )
{
	switch (offset) {
	case 0:
		YM2203_control_port_0_w(0,data);
		break;
	case 1:
		YM2203_write_port_0_w(0,data);
		break;
	}
}

static ADDRESS_MAP_START( audio_map, ADDRESS_SPACE_PROGRAM, 8 )
	AM_RANGE(0x0000, 0x05ff) AM_RAM
	AM_RANGE(0x0800, 0x0801) AM_WRITE(YM2203_w)
	AM_RANGE(0x1000, 0x1001) AM_WRITE(YM3812_w)
	AM_RANGE(0x3000, 0x3000) AM_READ(soundlatch_r)
	AM_RANGE(0x3800, 0x3800) AM_READWRITE(OKIM6295_status_0_r, OKIM6295_data_0_w)
	AM_RANGE(0x8000, 0xffff) AM_ROM
ADDRESS_MAP_END

/******************************************************************************/

static INPUT_PORTS_START( stadhero )
	PORT_START	/* Player 1 controls */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON2 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_BUTTON3 )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_START1 )

	PORT_START	/* Player 2 controls */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(2)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_START2 )

	PORT_START	/* Credits, start buttons */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_COIN3 ) /* Service */
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_VBLANK )

	PORT_START	/* Dip switch bank 1 */
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
	PORT_DIPNAME( 0x10, 0x00, DEF_STR( Demo_Sounds ) )
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Flip_Screen ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START	/* Dip switch bank 2 */
	PORT_DIPNAME( 0x01, 0x01, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
INPUT_PORTS_END

/******************************************************************************/

static const gfx_layout charlayout =
{
	8,8,	/* 8*8 chars */
	4096,
	3,		/* 4 bits per pixel  */
	{ 0x00000*8,0x8000*8,0x10000*8 },
	{ 0, 1, 2, 3, 4, 5, 6, 7 },
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8 },
	8*8	/* every char takes 8 consecutive bytes */
};

static const gfx_layout tile_3bpp =
{
	16,16,
	2048,
	3,
	{ 0x20000*8, 0x10000*8, 0x00000*8 },
	{ 16*8+0, 16*8+1, 16*8+2, 16*8+3, 16*8+4, 16*8+5, 16*8+6, 16*8+7,
			0, 1, 2, 3, 4, 5, 6, 7 },
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8,
			8*8, 9*8, 10*8, 11*8, 12*8, 13*8, 14*8, 15*8 },
	16*16
};

static const gfx_layout spritelayout =
{
	16,16,
	4096,
	4,
	{ 0x60000*8,0x40000*8,0x20000*8,0x00000*8 },
	{ 16*8+0, 16*8+1, 16*8+2, 16*8+3, 16*8+4, 16*8+5, 16*8+6, 16*8+7,
			0, 1, 2, 3, 4, 5, 6, 7 },
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8,
			8*8, 9*8, 10*8, 11*8, 12*8, 13*8, 14*8, 15*8 },
	16*16
};

static GFXDECODE_START( stadhero )
	GFXDECODE_ENTRY( REGION_GFX1, 0, charlayout,     0, 16 )	/* Characters 8x8 */
	GFXDECODE_ENTRY( REGION_GFX2, 0, tile_3bpp,    512, 16 )	/* Tiles 16x16 */
	GFXDECODE_ENTRY( REGION_GFX3, 0, spritelayout, 256, 16 )	/* Sprites 16x16 */
GFXDECODE_END

/******************************************************************************/

static void irqhandler(int linestate)
{
	cpunum_set_input_line(Machine, 1,0,linestate);
}

static const struct YM3812interface ym3812_interface =
{
	irqhandler
};

/******************************************************************************/

static MACHINE_DRIVER_START( stadhero )

	/* basic machine hardware */
	MDRV_CPU_ADD(M68000, 10000000)
	MDRV_CPU_PROGRAM_MAP(main_map,0)
	MDRV_CPU_VBLANK_INT(irq5_line_hold,1)/* VBL */

	MDRV_CPU_ADD(M6502, 1500000)
	/* audio CPU */
	MDRV_CPU_PROGRAM_MAP(audio_map,0)

	MDRV_SCREEN_REFRESH_RATE(58)
	MDRV_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(529))

	/* video hardware */
	MDRV_VIDEO_ATTRIBUTES(VIDEO_TYPE_RASTER)
	MDRV_SCREEN_FORMAT(BITMAP_FORMAT_INDEXED16)
	MDRV_SCREEN_SIZE(32*8, 32*8)
	MDRV_SCREEN_VISIBLE_AREA(0*8, 32*8-1, 1*8, 31*8-1)
	MDRV_GFXDECODE(stadhero)
	MDRV_PALETTE_LENGTH(1024)

	MDRV_VIDEO_START(stadhero)
	MDRV_VIDEO_UPDATE(stadhero)

	/* sound hardware */
	MDRV_SPEAKER_STANDARD_MONO("mono")

	MDRV_SOUND_ADD(YM2203, 1500000)
	MDRV_SOUND_ROUTE(0, "mono", 0.95)
	MDRV_SOUND_ROUTE(1, "mono", 0.95)
	MDRV_SOUND_ROUTE(2, "mono", 0.95)
	MDRV_SOUND_ROUTE(3, "mono", 0.40)

	MDRV_SOUND_ADD(YM3812, 3000000)
	MDRV_SOUND_CONFIG(ym3812_interface)
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.80)

	MDRV_SOUND_ADD(OKIM6295, 1023924)
	MDRV_SOUND_CONFIG(okim6295_interface_region_1_pin7high) // clock frequency & pin 7 not verified
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.80)
MACHINE_DRIVER_END

/******************************************************************************/

ROM_START( stadhero )
	ROM_REGION( 0x20000, REGION_CPU1, 0 )	/* 6*64k for 68000 code */
	ROM_LOAD16_BYTE( "ef15.bin",  0x00000, 0x10000, CRC(bbba364e) SHA1(552096102f402085596635f02096462c6b8e13a7) )
	ROM_LOAD16_BYTE( "ef13.bin",  0x00001, 0x10000, CRC(97c6717a) SHA1(6c81260f49a59f70c71f520e51330a6833828684) )

	ROM_REGION( 0x10000, REGION_CPU2, 0 )	/* 6502 Sound */
	ROM_LOAD( "ef18.bin",  0x8000, 0x8000, CRC(20fd9668) SHA1(058e34a0ebfc372aaa9230c2bc9164ee2e85e217) )

	ROM_REGION( 0x18000, REGION_GFX1, ROMREGION_DISPOSE )
	ROM_LOAD( "ef08.bin",     0x000000, 0x10000, CRC(e84752fe) SHA1(9af2140ddbb44be793ab5b39787bac27f5b1c1f2) )	/* chars */
	ROM_LOAD( "ef09.bin",     0x010000, 0x08000, CRC(2ade874d) SHA1(5c884535214438a4ea79fd262700a346bc12ad81) )

	ROM_REGION( 0x30000, REGION_GFX2, ROMREGION_DISPOSE )
	ROM_LOAD( "ef10.bin",     0x000000, 0x10000, CRC(dca3d599) SHA1(2b97a70065f3065e7fbb54fb53cb120d9e5013b3) )	/* tiles */
	ROM_LOAD( "ef11.bin",     0x010000, 0x10000, CRC(af563e96) SHA1(c88eaff4a1ea133d708f4511bb1dbc99ef066eed) )
	ROM_LOAD( "ef12.bin",     0x020000, 0x10000, CRC(9a1bf51c) SHA1(e733c193b305496878551fc6eefc21587ba75c82) )

	ROM_REGION( 0x80000, REGION_GFX3, ROMREGION_DISPOSE )
	ROM_LOAD( "ef00.bin",     0x000000, 0x10000, CRC(94ed257c) SHA1(caa4a4c8bf3b34d2288e117cfc704cca4c6f913b) )	/* sprites */
	ROM_LOAD( "ef01.bin",     0x010000, 0x10000, CRC(6eb9a721) SHA1(0f9dce614e67e57612e3a4ce187f0f9c12b78281) )
	ROM_LOAD( "ef02.bin",     0x020000, 0x10000, CRC(850cb771) SHA1(ccb54036191674d76965270a5831fba3e62f47c0) )
	ROM_LOAD( "ef03.bin",     0x030000, 0x10000, CRC(24338b96) SHA1(7730486bd0b84ba0a69b5547e348ee0058d4e7f1) )
	ROM_LOAD( "ef04.bin",     0x040000, 0x10000, CRC(9e3d97a7) SHA1(d02722376721caa5d8498f15f16959f42b75e7c1) )
	ROM_LOAD( "ef05.bin",     0x050000, 0x10000, CRC(88631005) SHA1(3c1787fb3aabdd9fecf679b2f4a9f833bf660885) )
	ROM_LOAD( "ef06.bin",     0x060000, 0x10000, CRC(9f47848f) SHA1(e23337684c8999483cbd11d3d953b06c34f13069) )
	ROM_LOAD( "ef07.bin",     0x070000, 0x10000, CRC(8859f655) SHA1(b3d69c5808b3ba7347ddb7f9693499903e9bfe6b) )

	ROM_REGION( 0x10000, REGION_SOUND1, 0 )	/* ADPCM samples */
	ROM_LOAD( "ef17.bin",  0x0000, 0x10000, CRC(07c78358) SHA1(ce82b429eec0193fd9665b717336756a514db144) )
ROM_END

/******************************************************************************/

GAME( 1988, stadhero, 0, stadhero, stadhero, 0, ROT0, "Data East Corporation", "Stadium Hero (Japan)", 0 )
