/***************************************************************************

World Rally (c) 1993 Gaelco (Designed & Developed by Zigurat. Produced by Gaelco)

Preliminary driver by Manuel Abadia <manu@teleline.es>

Encyption tables provided by Mike Coates who connected a fluke to the PCB.
Nicola Salmoria made the decode function based on that info.

Current decryption is incomplete
The DS5002FP has 32KB undumped gameplay code making the game unplayable :(

Main PCB components:
====================

CPUs related:
=============
* 1xDS5002FP @ D12 (Dallas security processor @ 12 MHz)
* 1xHM62256ALFP-8T (32KB NVSRAM) @ C11 (encrypted DS5002FP program code)
* 1xLithium cell
* 2xMS6264A-20NC (32KB SRAM) @ D14 & D15 (shared memory between M68000 & DS5002FP)
* 4x74LS157 (Quad 2 input multiplexer) @ F14, F15, F16 & F17 (used to select M68000 or DS5002FP address bus)
* 4x74LS245 (Octal bus transceiver) @ C14, C15, C16 & C17 (used to store shared RAM data)
* 2x74LS373 (Octal tristate latch) @ D16 & D17 (used by DS5002FP to access data from shared RAM)
* 1xMC68000P12 @ C20 (Motorola 68000 @ 12 MHz)
* 1xOSC24MHz @ B20
* 2xM27C4001 @ C22 & C23 (M68000 program ROMs)
* 1xPAL20L8 @ B23 (handles 1st level M68000 memory map)
    0 -> DTACK (M68000 data ack)
    1 -> SELACT
    2 -> Input/sound (see below)
    3 -> ACTEXT
    4 -> SELMOV
    5 -> CSW
    6 -> CSR
    7 -> EXT

* 1x74LS138 (3 to 8 line decoder) @ B13 (handles 2nd level M68000 memory map)
    0 -> IN0    DIPSW #1 & #2
    1 -> IN1    Joystick 1P & 2P, COINSW, STARTSW
    2 -> IN2    Wheel input
    3 -> -
    4 -> IN4    TESTSW & SERVICESW
    5 -> OUT (see below)
    6 -> CSBAN  OKIM6295 bankswitch
    7 -> CSSON  OKIM6295 R/W

* 1x74LS259 (8 bit addressable latches) @A7 (handles 3rd level M68000 memory map)
    0 -> Coin lockout 1
    1 -> Coin lockout 2
    2 -> Coin counter 1
    3 -> Coin counter 2
    4 -> Sound muting
    5 -> flip screen
    6 -> ENA/D?
    7 -> CKA/D?

Sound related:
==============
* 1xOKIM6295 @ C6
* 2xM27C4001 @ C1 & C3 (OKI ADPCM samples)
* 1xPAL16R4 @ E2 (handles OKI ROM banking)

Graphics related:
=================
* 1xOSC30MHz @ D5
* 2xTPC1020AFN-84C (FPGA) @ G8 & G13 (GFX processing)
* 2xMS6264A-20NC (8KB SRAM) @ I16 & I17 (Video RAM)
* 4xUM6116BK-25 (2KB SRAM) @ H1, H2, H4 & H5
* 2xUM6116BK-25 (2KB SRAM) @ H22 & H23

Palette related:
================
* 2xMS6264A-20NC (8KB SRAM) @ C8 & C9 (palette RAM (xxxxBBBBRRRRGGGG))
* 2x74HCT273 (octal D-Type flip-flop with clear) @ B8 & B9 (connected to RGB output)

***************************************************************************/

#include "driver.h"
#include "cpu/m68000/m68000.h"
#include "sound/okim6295.h"

/* from video/wrally.c */
extern UINT16 *wrally_vregs;
extern UINT16 *wrally_videoram;
extern UINT16 *wrally_spriteram;

WRITE16_HANDLER( wrally_vram_w );
VIDEO_START( wrally );
VIDEO_UPDATE( wrally );

/* from machine/wrally.c */
DRIVER_INIT( wrally );
WRITE16_HANDLER( OKIM6295_bankswitch_w );
WRITE16_HANDLER( wrally_coin_counter_w );
WRITE16_HANDLER( wrally_coin_lockout_w );



static ADDRESS_MAP_START( wrally_readmem, ADDRESS_SPACE_PROGRAM, 16 )
	AM_RANGE(0x000000, 0x0fffff) AM_READ(SMH_ROM)			/* ROM */
	AM_RANGE(0x100000, 0x103fff) AM_READ(SMH_RAM)			/* encrypted Video RAM */
	AM_RANGE(0x200000, 0x203fff) AM_READ(SMH_RAM)			/* Palette */
	AM_RANGE(0x440000, 0x440fff) AM_READ(SMH_RAM)			/* Sprite RAM */
	AM_RANGE(0x700000, 0x700001) AM_READ(input_port_0_word_r)/* DSW #1 & #2 */
	AM_RANGE(0x700002, 0x700003) AM_READ(input_port_1_word_r)/* INPUT 1P & 2P, COINSW, STARTSW */
	AM_RANGE(0x700004, 0x700005) AM_READ(input_port_2_word_r)/* Wheel */
	AM_RANGE(0x700008, 0x700009) AM_READ(input_port_3_word_r)/* TESTSW & SERVICESW */
	AM_RANGE(0x70000e, 0x70000f) AM_READ(OKIM6295_status_0_lsb_r)/* OKI6295 status register */
	AM_RANGE(0xfe0000, 0xfeffff) AM_READ(SMH_RAM)			/* Work RAM (partially shared with DS5002FP) */
ADDRESS_MAP_END

static WRITE16_HANDLER( unknown_w )
{
	popmessage("write %04x to %04x", data, offset*2 + 0x6a);
}

static ADDRESS_MAP_START( wrally_writemem, ADDRESS_SPACE_PROGRAM, 16 )
	AM_RANGE(0x000000, 0x0fffff) AM_WRITE(SMH_ROM)								/* ROM */
	AM_RANGE(0x100000, 0x103fff) AM_WRITE(wrally_vram_w) AM_BASE(&wrally_videoram)		/* encrypted Video RAM */
	AM_RANGE(0x108000, 0x108007) AM_WRITE(SMH_RAM) AM_BASE(&wrally_vregs)				/* Video Registers */
	AM_RANGE(0x10800c, 0x10800d) AM_WRITE(SMH_NOP)								/* CLR INT Video */
	AM_RANGE(0x200000, 0x203fff) AM_WRITE(paletteram16_xxxxBBBBRRRRGGGG_word_w) AM_BASE(&paletteram16)/* Palette */
	AM_RANGE(0x440000, 0x440fff) AM_WRITE(SMH_RAM) AM_BASE(&wrally_spriteram)			/* Sprite RAM */
	AM_RANGE(0x70000c, 0x70000d) AM_WRITE(OKIM6295_bankswitch_w)					/* OKI6295 bankswitch */
	AM_RANGE(0x70000e, 0x70000f) AM_WRITE(OKIM6295_data_0_lsb_w)					/* OKI6295 data register */
	AM_RANGE(0x70000a, 0x70001b) AM_WRITE(wrally_coin_lockout_w)					/* Coin lockouts */
	AM_RANGE(0x70002a, 0x70003b) AM_WRITE(wrally_coin_counter_w)					/* Coin counters */
	AM_RANGE(0x70004a, 0x70004b) AM_WRITE(SMH_NOP)								/* sound muting */
	AM_RANGE(0x70005a, 0x70005b) AM_WRITE(SMH_NOP)								/* flip screen */
	AM_RANGE(0x70006a, 0x70007b) AM_WRITE(unknown_w)								/* ??? */
	AM_RANGE(0xfe0000, 0xfeffff) AM_WRITE(SMH_RAM)								/* Work RAM (partially shared with DS5002FP) */
ADDRESS_MAP_END


static INPUT_PORTS_START( wrally )
PORT_START	/* DSW #1 & #2 */
	PORT_DIPNAME( 0x0003, 0x0003, DEF_STR( Difficulty ) )
	PORT_DIPSETTING(      0x0003, DEF_STR( Normal ) )
	PORT_DIPSETTING(      0x0002, DEF_STR( Easy ) )
	PORT_DIPSETTING(      0x0001, DEF_STR( Hard ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( Hardest ) )
	PORT_DIPNAME( 0x0004, 0x0004, "Number of Joysticks" )
	PORT_DIPSETTING(      0x0004, "2" )
	PORT_DIPSETTING(      0x0000, "1" )
	PORT_DIPNAME( 0x0018, 0x0018, "Control Configuration" )
	PORT_DIPSETTING(      0x0018, DEF_STR( Joystick ) )
	PORT_DIPSETTING(      0x0010, "Pot Wheel" )
	PORT_DIPSETTING(      0x0000, "Opt Wheel" )
	PORT_DIPNAME( 0x0020, 0x0020, DEF_STR( Demo_Sounds ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0020, DEF_STR( On ) )
	PORT_DIPNAME( 0x0040, 0x0040, DEF_STR( Cabinet ) )
	PORT_DIPSETTING(      0x0040, DEF_STR( Upright ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( Cocktail ) )
	PORT_SERVICE( 0x0080, IP_ACTIVE_LOW )
	PORT_DIPNAME( 0x0700, 0x0700, DEF_STR( Coin_A ) )
	PORT_DIPSETTING(      0x0700, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( 3C_4C ) )
	PORT_DIPSETTING(      0x0100, DEF_STR( 2C_3C ) )
	PORT_DIPSETTING(      0x0600, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(      0x0500, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(      0x0400, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(      0x0300, DEF_STR( 1C_5C ) )
	PORT_DIPSETTING(      0x0200, DEF_STR( 1C_6C ) )
	PORT_DIPNAME( 0x3800, 0x3800, DEF_STR( Coin_B ) )
	PORT_DIPSETTING(      0x1000, DEF_STR( 6C_1C ) )
	PORT_DIPSETTING(      0x1800, DEF_STR( 5C_1C ) )
	PORT_DIPSETTING(      0x2000, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(      0x2800, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(      0x3000, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(      0x0800, DEF_STR( 3C_2C ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( 4C_3C ) )
	PORT_DIPSETTING(      0x3800, DEF_STR( 1C_1C ) )
	PORT_DIPNAME( 0x4000, 0x4000, "Credit configuration" )
	PORT_DIPSETTING(      0x4000, "Start 1C/Continue 1C" )
	PORT_DIPSETTING(      0x0000, "Start 2C/Continue 1C" )
	PORT_DIPNAME( 0x8000, 0x8000, DEF_STR( Free_Play ) )
	PORT_DIPSETTING(      0x8000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )

PORT_START	/* INPUTS, COINSW & STARTSW */
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1)
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(1)
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2)
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_START2 )

PORT_START	/* Wheel control? */
	PORT_BIT( 0xffff, IP_ACTIVE_LOW, IPT_UNKNOWN ) /* not implemented yet */

PORT_START	/* INPUTS, TEST & SERVICE */
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_SERVICE2 )	/* Go to test mode NOW */
	PORT_BIT( 0xfffc, IP_ACTIVE_LOW, IPT_UNKNOWN )
INPUT_PORTS_END



static const gfx_layout wrally_tilelayout16 =
{
	16,16,									/* 16x16 tiles */
	RGN_FRAC(1,2),							/* number of tiles */
	4,										/* 4 bpp */
	{ RGN_FRAC(1,2)+8, RGN_FRAC(1,2)+0, 8, 0 },
	{ 0, 1, 2, 3, 4, 5, 6, 7,
		16*16+0, 16*16+1, 16*16+2, 16*16+3, 16*16+4, 16*16+5, 16*16+6, 16*16+7 },
	{ 0*16, 1*16, 2*16, 3*16, 4*16, 5*16, 6*16, 7*16,
	  8*16, 9*16, 10*16, 11*16, 12*16, 13*16, 14*16, 15*16 },
	64*8
};

static GFXDECODE_START( wrally )
	GFXDECODE_ENTRY( REGION_GFX1, 0x000000, wrally_tilelayout16, 0, 64*8 )
GFXDECODE_END



static MACHINE_DRIVER_START( wrally )
	/* basic machine hardware */
	MDRV_CPU_ADD(M68000,XTAL_24MHz/2)	/* verified on pcb */
	MDRV_CPU_PROGRAM_MAP(wrally_readmem,wrally_writemem)
	MDRV_CPU_VBLANK_INT("main", irq6_line_hold)

	/* video hardware */
	MDRV_SCREEN_ADD("main", RASTER)
	MDRV_SCREEN_REFRESH_RATE(60)
	MDRV_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(2500) /* not accurate */)
	MDRV_SCREEN_FORMAT(BITMAP_FORMAT_INDEXED16)
	MDRV_SCREEN_SIZE(64*16, 32*16)
	MDRV_SCREEN_VISIBLE_AREA(0, 64*16-1, 0, 32*16-1)
//  MDRV_SCREEN_VISIBLE_AREA(0, 320-1, 16, 256-1)

	MDRV_GFXDECODE(wrally)
	MDRV_PALETTE_LENGTH(1024*8)

	MDRV_VIDEO_START(wrally)
	MDRV_VIDEO_UPDATE(wrally)

	/* sound hardware */
	MDRV_SPEAKER_STANDARD_MONO("mono")

	MDRV_SOUND_ADD(OKIM6295, XTAL_1MHz) /* verified on pcb */
	MDRV_SOUND_CONFIG(okim6295_interface_region_1_pin7high) /* verified on pcb */
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "mono", 1.0)
MACHINE_DRIVER_END


ROM_START( wrally )
	ROM_REGION( 0x100000, REGION_CPU1, 0 )	/* 68000 code */
	ROM_LOAD16_BYTE(	"worldr17.c23",	0x000000, 0x080000, CRC(050f5629) SHA1(74fc2cd5114f3bc4b2429f1d8d7eeb1658f9f179) )
	ROM_LOAD16_BYTE(	"worldr16.c22",	0x000001, 0x080000, CRC(9e0d126c) SHA1(369360b7ec2c3497af3bf62b4eba24c3d9f94675) )

	ROM_REGION( 0x200000, REGION_GFX1, ROMREGION_DISPOSE )
	ROM_LOAD16_BYTE( "worldr21.i13",	0x000000, 0x080000, CRC(b7fddb12) SHA1(619a75daac8cbba7e85c97ca19733e2196d66d5c) )
	ROM_LOAD16_BYTE( "worldr20.i11",	0x000001, 0x080000, CRC(58b2809a) SHA1(8741ec544c54e2a2f5d17ac2f8400ee2ce382e83) )
	ROM_LOAD16_BYTE( "worldr19.i09",	0x100000, 0x080000, CRC(018b35bb) SHA1(ca789e23d18cc7d7e48b6858e6b61e03bf88b475) )
	ROM_LOAD16_BYTE( "worldr18.i07",	0x100001, 0x080000, CRC(b37c807e) SHA1(9e6155a2b5206c0d4dca669d24d9fe9830027651) )

	ROM_REGION( 0x140000, REGION_SOUND1, 0 )	/* ADPCM samples - sound chip is OKIM6295 */
	ROM_LOAD( "worldr14.c01",	0x000000, 0x080000, CRC(e931c2ee) SHA1(ea1cf8ad52713e5136a370e289567eea9e6403d6) )
	/* 0x00000-0x2ffff is fixed, 0x30000-0x3ffff is bank switched from all the ROMs */
	ROM_RELOAD(		0x040000, 0x080000 )
	ROM_LOAD( "worldr15.c03",	0x0c0000, 0x080000, CRC(11f0fe2c) SHA1(96c2a04874fa036576b7cfc5559bb0e33582ffd2) )

	ROM_REGION( 0x0514, REGION_PLDS, 0 ) /* PAL's and GAL's */
	ROM_LOAD( "tibpal20l8-25cnt.b23", 0x0000, 0x0104, NO_DUMP )
	ROM_LOAD( "gal16v8-25lnc.h21",    0x0000, 0x0104, NO_DUMP )
	ROM_LOAD( "tibpal20l8-25cnt.h15", 0x0000, 0x0104, NO_DUMP )
	ROM_LOAD( "pal16r4-e2.bin",       0x0000, 0x0104, CRC(15fee75c) SHA1(b9ee5121dd41f2535d9abd78ff5fcfeaa1ac6b62) )
	ROM_LOAD( "pal16r8-b15.bin",      0x0000, 0x0104, CRC(b50337a6) SHA1(1f922753cb9982cad9a3c9246894ecd38273236e) )
ROM_END

ROM_START( wrallya )
	ROM_REGION( 0x100000, REGION_CPU1, 0 )	/* 68000 code */
	ROM_LOAD16_BYTE(	"c23.bin",	0x000000, 0x080000, CRC(8b7d93c3) SHA1(ce4163eebc5d4a0c1266d650523b1ffc702d1b87) )
	ROM_LOAD16_BYTE(	"c22.bin",	0x000001, 0x080000, CRC(56da43b6) SHA1(02db8f969ed5e7f5e5356c45c0312faf5f000335) )

	ROM_REGION( 0x200000, REGION_GFX1, ROMREGION_DISPOSE )
	ROM_LOAD16_BYTE( "worldr21.i13",	0x000000, 0x080000, CRC(b7fddb12) SHA1(619a75daac8cbba7e85c97ca19733e2196d66d5c) )
	ROM_LOAD16_BYTE( "worldr20.i11",	0x000001, 0x080000, CRC(58b2809a) SHA1(8741ec544c54e2a2f5d17ac2f8400ee2ce382e83) )
	ROM_LOAD16_BYTE( "worldr19.i09",	0x100000, 0x080000, CRC(018b35bb) SHA1(ca789e23d18cc7d7e48b6858e6b61e03bf88b475) )
	ROM_LOAD16_BYTE( "worldr18.i07",	0x100001, 0x080000, CRC(b37c807e) SHA1(9e6155a2b5206c0d4dca669d24d9fe9830027651) )
//  same data, different layout
//  ROM_LOAD( "h12.bin",    0x000000, 0x100000, CRC(3353dc00) )
//  ROM_LOAD( "h8.bin",     0x100000, 0x100000, CRC(58dcd024) )

	ROM_REGION( 0x140000, REGION_SOUND1, 0 )	/* ADPCM samples - sound chip is OKIM6295 */
	ROM_LOAD( "worldr14.c01",	0x000000, 0x080000, CRC(e931c2ee) SHA1(ea1cf8ad52713e5136a370e289567eea9e6403d6) )
	/* 0x00000-0x2ffff is fixed, 0x30000-0x3ffff is bank switched from all the ROMs */
	ROM_RELOAD(		0x040000, 0x080000 )
	ROM_LOAD( "worldr15.c03",	0x0c0000, 0x080000, CRC(11f0fe2c) SHA1(96c2a04874fa036576b7cfc5559bb0e33582ffd2) )

	ROM_REGION( 0x0514, REGION_PLDS, 0 ) /* PAL's and GAL's */
	ROM_LOAD( "tibpal20l8-25cnt.b23", 0x0000, 0x0104, NO_DUMP )
	ROM_LOAD( "gal16v8-25lnc.h21",    0x0000, 0x0104, NO_DUMP )
	ROM_LOAD( "tibpal20l8-25cnt.h15", 0x0000, 0x0104, NO_DUMP )
	ROM_LOAD( "pal16r4-e2.bin",       0x0000, 0x0104, CRC(15fee75c) SHA1(b9ee5121dd41f2535d9abd78ff5fcfeaa1ac6b62) )
	ROM_LOAD( "pal16r8-b15.bin",      0x0000, 0x0104, CRC(b50337a6) SHA1(1f922753cb9982cad9a3c9246894ecd38273236e) )
ROM_END

ROM_START( wrallyb ) /* Board Marked 930217 */
	ROM_REGION( 0x100000, REGION_CPU1, 0 )	/* 68000 code */
	ROM_LOAD16_BYTE( "rally.c23", 0x000000, 0x080000, CRC(366595ad) SHA1(e16341ed9eacf9b729c28184268150ea9b62f185) )
	ROM_LOAD16_BYTE( "rally.c22", 0x000001, 0x080000, CRC(0ad4ec6f) SHA1(991557cf25fe960b1c586e990e6019befe5a11d0) )

	ROM_REGION( 0x400000, REGION_GFX1, ROMREGION_DISPOSE )
	ROM_LOAD( "rally h-12.h12", 0x000000, 0x200000, CRC(38a44370) SHA1(cb427aa337232ae3a8effab3804d3d1d85d1f40b) )
	ROM_LOAD( "rally h-8.h8",   0x000000, 0x200000, CRC(c36d1b1e) SHA1(598f72f11100742eda1eda795ded69a0a2d62225) )

	ROM_REGION( 0x100000, REGION_SOUND1, 0 )	/* ADPCM samples - sound chip is OKIM6295 */
	ROM_LOAD( "sound c-1.c1", 0x000000, 0x100000, CRC(2d69c9b8) SHA1(328cb3c928dc6921c0c3f0277f59bca6c747c504) )

	ROM_REGION( 0x0514, REGION_PLDS, 0 ) /* PAL's and GAL's */
	ROM_LOAD( "tibpal20l8-25cnt.b23", 0x0000, 0x0104, NO_DUMP )
	ROM_LOAD( "gal16v8-25lnc.h21",    0x0000, 0x0104, NO_DUMP )
	ROM_LOAD( "tibpal20l8-25cnt.h15", 0x0000, 0x0104, NO_DUMP )
	ROM_LOAD( "pal16r4-e2.bin",       0x0000, 0x0104, CRC(15fee75c) SHA1(b9ee5121dd41f2535d9abd78ff5fcfeaa1ac6b62) )
	ROM_LOAD( "pal16r8-b15.bin",      0x0000, 0x0104, CRC(b50337a6) SHA1(1f922753cb9982cad9a3c9246894ecd38273236e) )
ROM_END


GAME( 1993, wrally,  0, 	 wrally, wrally, wrally, ROT0, "Gaelco", "World Rally (set 1)", GAME_NOT_WORKING )
GAME( 1993, wrallya, wrally, wrally, wrally, wrally, ROT0, "Gaelco", "World Rally (set 2)", GAME_NOT_WORKING )
GAME( 1993, wrallyb, wrally, wrally, wrally, wrally, ROT0, "Gaelco", "World Rally (set 3 - 930217)", GAME_NOT_WORKING )
