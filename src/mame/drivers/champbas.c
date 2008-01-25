/***************************************************************************

Championship Baseball

driver by Nicola Salmoria
ALPHA 8201 MCU handling by Tatsuyuki satoh

TODO:
  champbb2 , sometime mcu err and ACCESS VIOLATION trap.

champbbj and champbb2 has Alpha8201 mcu for protection.
champbja is a patched version of champbbj with different protection.

main CPU

0000-5fff ROM
6000-63ff MCU shared RAM
7800-7fff ROM (Champion Baseball 2 only)
8000-83ff Video RAM
8400-87ff Color RAM
8800-8fff RAM

read:
a000      IN0
a040      IN1
a080      DSW
a0a0      ?(same as DSW)
a0c0      COIN

write:
7000      8910 write
7001      8910 control
8ff0-8fff sprites
a000      ?
a006      MCU HALT controll
a007      NOP (MCU shared RAM switch)
a060-a06f sprites
a080      command for the sound CPU
a0c0      watchdog reset (watchdog time = 16xvblank)

sub CPU (speech DAC)

read:
0000-5fff   ROM
6000(-7fff) sound latch
e000-e3ff   RAM

write:

8000(-9fff) 4bit status for main CPU
a000(-bfff) clear sound latch
c000(-dfff) DAC
e000-e3ff   RAM

***************************************************************************/

#include "driver.h"
#include "deprecat.h"
#include "sound/ay8910.h"
#include "sound/dac.h"


extern WRITE8_HANDLER( champbas_videoram_w );
extern WRITE8_HANDLER( champbas_colorram_w );
extern WRITE8_HANDLER( champbas_gfxbank_w );
extern WRITE8_HANDLER( champbas_flipscreen_w );

extern PALETTE_INIT( champbas );
extern VIDEO_START( champbas );
extern VIDEO_UPDATE( champbas );

static WRITE8_HANDLER( champbas_dac_w )
{
	DAC_signed_data_w(0,data<<2);
}

///////////////////////////////////////////////////////////////////////////
// protection handling
///////////////////////////////////////////////////////////////////////////

static WRITE8_HANDLER( champbas_mcu_halt_w )
{
	data &= 1;
	cpunum_set_input_line(Machine, 2, INPUT_LINE_HALT, data ? ASSERT_LINE : CLEAR_LINE);
}

/* champbja another protection */
static READ8_HANDLER( champbja_alt_protection_r )
{
	UINT8 data = 0;
/*
(68BA) & 0x99 == 0x00
(6867) & 0x99 == 0x99
(68AB) & 0x80 == 0x80
(6854) & 0x99 == 0x19

BA 1011_1010
00 0--0_0--0

54 0101_0100
19 0--1_1--1

67 0110_0111
99 1--1_1--1

AB 1010_1011
80 1--0_0--0
*/
	/* bit7 =  bit0 */
	if(  (offset&0x01) ) data |= 0x80;
	/* bit4,3,0 =  bit6 */
	if(  (offset&0x40) ) data |= 0x19;

	return data;
}

static ADDRESS_MAP_START( main_map, ADDRESS_SPACE_PROGRAM, 8 )
	AM_RANGE(0x0000, 0x5fff) AM_ROM
	AM_RANGE(0x7000, 0x7000) AM_WRITE(AY8910_write_port_0_w)
	AM_RANGE(0x7001, 0x7001) AM_WRITE(AY8910_control_port_0_w)
	AM_RANGE(0x7800, 0x7fff) AM_ROM

	AM_RANGE(0x8000, 0x83ff) AM_WRITE(champbas_videoram_w) AM_READ(MRA8_RAM) AM_BASE(&videoram)
	AM_RANGE(0x8400, 0x87ff) AM_WRITE(champbas_colorram_w) AM_READ(MRA8_RAM) AM_BASE(&colorram)
	AM_RANGE(0x8800, 0x8fef) AM_RAM
	AM_RANGE(0x8ff0, 0x8fff) AM_WRITE(MWA8_RAM) AM_READ(MRA8_RAM) AM_BASE(&spriteram) AM_SIZE(&spriteram_size)
	AM_RANGE(0xa000, 0xa000) AM_WRITE(interrupt_enable_w) AM_READ(input_port_0_r)
	AM_RANGE(0xa002, 0xa002) AM_WRITE(champbas_gfxbank_w)
	AM_RANGE(0xa003, 0xa003) AM_WRITE(champbas_flipscreen_w)
	AM_RANGE(0xa005, 0xa005) AM_NOP

	AM_RANGE(0xa040, 0xa040)                            AM_READ(input_port_1_r)
	AM_RANGE(0xa060, 0xa06f) AM_WRITE(MWA8_RAM)                                   AM_BASE(&spriteram_2)
	AM_RANGE(0xa080, 0xa080) AM_WRITE(soundlatch_w)     AM_READ(input_port_2_r)
/*  AM_RANGE(0xa0a0, 0xa0a0)    ???? */
	AM_RANGE(0xa0c0, 0xa0c0) AM_WRITE(watchdog_reset_w) AM_READ(input_port_3_r)

	/* has MCU pcb only */
	AM_RANGE(0x6000, 0x63ff) AM_RAM AM_SHARE(1)
/*  AM_RANGE(0xa006, 0xa006) AM_WRITE(champbas_mcu_halt_w) */
	AM_RANGE(0xa007, 0xa007) AM_NOP /* MCU shared RAM switch , but no-hawdrare */

	/* champbja only */
	AM_RANGE(0x6800, 0x68ff) AM_READ(champbja_alt_protection_r)

ADDRESS_MAP_END

static ADDRESS_MAP_START( sub_map, ADDRESS_SPACE_PROGRAM, 8 )
	AM_RANGE(0x0000, 0x5fff) AM_ROM
	AM_RANGE(0x6000, 0x6000) AM_READ(soundlatch_r)
	AM_RANGE(0xe000, 0xe3ff) AM_READ(MRA8_RAM)
/*  AM_RANGE(0x8000, 0x8000) AM_WRITE(MWA8_NOP) return low 4bit to main CPU */
	AM_RANGE(0xa000, 0xa000) AM_WRITE(soundlatch_clear_w)
	AM_RANGE(0xc000, 0xc000) AM_WRITE(champbas_dac_w)
	AM_RANGE(0xe000, 0xe3ff) AM_WRITE(MWA8_RAM)
ADDRESS_MAP_END

static ADDRESS_MAP_START( mcu_map, ADDRESS_SPACE_PROGRAM, 8 )
	AM_RANGE(0x0000, 0x03ff) AM_RAM AM_SHARE(1) /* main CPU shared RAM */
ADDRESS_MAP_END

static INPUT_PORTS_START( champbas )
	PORT_START	/* IN0 */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON2 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON3 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_BUTTON4 )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY

	PORT_START	/* IN1 */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_COCKTAIL
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_COCKTAIL
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_COCKTAIL
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_COCKTAIL
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_COCKTAIL

	PORT_START	/* DSW */
	PORT_DIPNAME( 0x03, 0x02, DEF_STR( Coinage ) )
	PORT_DIPSETTING(    0x03, "A 2/1 B 3/2" )
	PORT_DIPSETTING(    0x02, "A 1/1 B 2/1")
	PORT_DIPSETTING(    0x01, "A 1/2 B 1/6" )
	PORT_DIPSETTING(    0x00, "A 1/3 B 1/6")
	PORT_DIPNAME( 0x04, 0x00, DEF_STR( Flip_Screen ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ))
	PORT_DIPSETTING(    0x04, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x00, DEF_STR( Demo_Sounds ) )
	PORT_DIPSETTING(    0x08, DEF_STR( Off ))
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x00, DEF_STR( Cabinet ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Upright ))
	PORT_DIPSETTING(    0x10, DEF_STR( Cocktail ) )
	PORT_DIPNAME( 0x20, 0x00, DEF_STR( Difficulty ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Easy ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Hard ))
	PORT_DIPNAME( 0x40, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ))
	PORT_DIPSETTING(    0x40, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x00, DEF_STR( Unknown ) ) /* watchdog half over : The game won't boot if set to ON */
	PORT_DIPSETTING(    0x00, DEF_STR( Off ))
	PORT_DIPSETTING(    0x80, DEF_STR( On ) )

	PORT_START	/* COIN */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )
INPUT_PORTS_END



static const gfx_layout charlayout =
{
	8,8,	/* 8*8 characters */
	256,	/* 256 characters */
	2,	/* 2 bits per pixel */
	{ 0, 4 },	/* the two bitplanes for 4 pixels are packed into one byte */
	{ 8*8+0, 8*8+1, 8*8+2, 8*8+3, 0, 1, 2, 3 },	/* bits are packed in groups of four */
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8 },
	16*8	/* every char takes 16 bytes */
};

static const gfx_layout spritelayout =
{
	16,16,	/* 16*16 sprites */
	64,	/* 64 sprites */
	2,	/* 2 bits per pixel */
	{ 0, 4 },	/* the two bitplanes for 4 pixels are packed into one byte */
	{ 8*8, 8*8+1, 8*8+2, 8*8+3, 16*8+0, 16*8+1, 16*8+2, 16*8+3,
			24*8+0, 24*8+1, 24*8+2, 24*8+3, 0, 1, 2, 3 },
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8,
			32*8, 33*8, 34*8, 35*8, 36*8, 37*8, 38*8, 39*8 },
	64*8	/* every sprite takes 64 bytes */
};

static GFXDECODE_START( champbas )
	GFXDECODE_ENTRY( REGION_GFX1, 0x0000, charlayout,   0, 64 )
	GFXDECODE_ENTRY( REGION_GFX2, 0x0000, charlayout,   0, 64 )
	GFXDECODE_ENTRY( REGION_GFX1, 0x1000, spritelayout, 0, 64 )
	GFXDECODE_ENTRY( REGION_GFX2, 0x1000, spritelayout, 0, 64 )
GFXDECODE_END




static const struct AY8910interface ay8910_interface =
{
	input_port_0_r,
	input_port_1_r
};

static MACHINE_DRIVER_START( champbas )

	/* basic machine hardware */

	/* main cpu */
	MDRV_CPU_ADD(Z80, 18432000/6)
	MDRV_CPU_PROGRAM_MAP(main_map,0)
	MDRV_CPU_VBLANK_INT(irq0_line_hold,1)

	/* audio CPU */
	MDRV_CPU_ADD(Z80, 18432000/6)
	MDRV_CPU_PROGRAM_MAP(sub_map,0)

	MDRV_SCREEN_REFRESH_RATE(60)
	MDRV_SCREEN_VBLANK_TIME(DEFAULT_60HZ_VBLANK_DURATION)

	/* video hardware */
	MDRV_VIDEO_ATTRIBUTES(VIDEO_TYPE_RASTER)
	MDRV_SCREEN_FORMAT(BITMAP_FORMAT_INDEXED16)
	MDRV_SCREEN_SIZE(32*8, 32*8)
	MDRV_SCREEN_VISIBLE_AREA(0*8, 32*8-1, 2*8, 30*8-1)
	MDRV_GFXDECODE(champbas)
	MDRV_PALETTE_LENGTH(32)
	MDRV_COLORTABLE_LENGTH(64*4)

	MDRV_PALETTE_INIT(champbas)
	MDRV_VIDEO_START(champbas)
	MDRV_VIDEO_UPDATE(champbas)

	/* sound hardware */
	MDRV_SPEAKER_STANDARD_MONO("mono")

	MDRV_SOUND_ADD(AY8910, 18432000/12)
	MDRV_SOUND_CONFIG(ay8910_interface)
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.30)

	MDRV_SOUND_ADD(DAC, 0)
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.70)
MACHINE_DRIVER_END


static MACHINE_DRIVER_START( champbb2 )
	/* basic machine hardware */
	MDRV_IMPORT_FROM(champbas)

	/* MCU */
	MDRV_CPU_ADD(ALPHA8201, 18432000/6/8)
	MDRV_CPU_PROGRAM_MAP(mcu_map,0)

	/* to MCU timeout champbbj */
	MDRV_INTERLEAVE(50)
MACHINE_DRIVER_END

/* MCU handling */
static DRIVER_INIT(champbb2)
{
	memory_install_write8_handler(0, ADDRESS_SPACE_PROGRAM, 0xa006, 0xa006, 0, 0, champbas_mcu_halt_w);
}

/***************************************************************************

  Game driver(s)

***************************************************************************/

ROM_START( champbas )
	ROM_REGION( 0x10000, REGION_CPU1, 0 )
	ROM_LOAD( "champbb.1",    0x0000, 0x2000, CRC(218de21e) SHA1(7577fd04bdda4666c017f3b36e81ec23bcddd845) )
	ROM_LOAD( "champbb.2",    0x2000, 0x2000, CRC(5ddd872e) SHA1(68e21572e27707c991180b1bd0a6b31f7b64abf6) )
	ROM_LOAD( "champbb.3",    0x4000, 0x2000, CRC(f39a7046) SHA1(3097bffe84ac74ce9e6481028a0ebbe8b1d6eaf9) )

	ROM_REGION( 0x10000, REGION_CPU2, 0 )	/* 64k for the speech CPU */
	ROM_LOAD( "champbb.6",    0x0000, 0x2000, CRC(26ab3e16) SHA1(019b9d34233a6b7a53e204154b782ceb42915d2b) )
	ROM_LOAD( "champbb.7",    0x2000, 0x2000, CRC(7c01715f) SHA1(b15b2001b8c110f2599eee3aeed79f67686ebd7e) )
	ROM_LOAD( "champbb.8",    0x4000, 0x2000, CRC(3c911786) SHA1(eea0c467e213d237b5bb9d04b19a418d6090c2dc) )

	ROM_REGION( 0x2000, REGION_GFX1, ROMREGION_DISPOSE )
	ROM_LOAD( "champbb.4",    0x0000, 0x2000, CRC(1930fb52) SHA1(cae0b2701c2b53b79e9df3a7496442ba3472e996) )

	ROM_REGION( 0x2000, REGION_GFX2, ROMREGION_DISPOSE )
	ROM_LOAD( "champbb.5",    0x0000, 0x2000, CRC(a4cef5a1) SHA1(fa00ed0d075e00992a1ddce3c1327ed74770a735) )

	ROM_REGION( 0x0120, REGION_PROMS, 0 )
	ROM_LOAD( "champbb.pr2",  0x0000, 0x020, CRC(2585ffb0) SHA1(ce7f62f37955c2bbb4f82b139cc716978b084767) ) /* palette */
	ROM_LOAD( "champbb.pr1",  0x0020, 0x100, CRC(872dd450) SHA1(6c1e2c4a2fc072f4bf4996c731adb0b01b347506) ) /* look-up table */
ROM_END

ROM_START( champbbj )
	ROM_REGION( 0x10000, REGION_CPU1, 0 )
	ROM_LOAD( "11.2e",      0x0000, 0x2000, CRC(e2dfc166) SHA1(482e084d7d21b1cf2d17431699e6bab4c4b6ac15) )
	ROM_LOAD( "12.2g",      0x2000, 0x2000, CRC(7b4e5faa) SHA1(b7201816a819ef313ddc81f312d26982b83ef1c7) )
	ROM_LOAD( "13.2h",      0x4000, 0x2000, CRC(b201e31f) SHA1(bba3b611ff60ad8d5dd8484df4cfc2026f4fd344) )

	ROM_REGION( 0x10000, REGION_CPU2, 0 ) /* 64k for the speech CPU */
	ROM_LOAD( "16.2k",      0x0000, 0x2000, CRC(24c482ee) SHA1(c25bdf77014e095fc11a9a6b17f16858f19db451) )
	ROM_LOAD( "17.2l",      0x2000, 0x2000, CRC(f10b148b) SHA1(d66516d509f6f16e51ee59d27c4867e276064c3f) )
	ROM_LOAD( "18.2n",      0x4000, 0x2000, CRC(2dc484dd) SHA1(28bd68c787d7e6989849ca52009948dbd5cdcc79) )

	ROM_REGION( 0x2000, REGION_GFX1, ROMREGION_DISPOSE )
	ROM_LOAD( "14.5e",      0x0000, 0x2000, CRC(1b8202b3) SHA1(889b77fc3d0cb029baf8c47be260f513f3ed59bd) )

	ROM_REGION( 0x2000, REGION_GFX2, ROMREGION_DISPOSE )
	ROM_LOAD( "15.5g",      0x0000, 0x2000, CRC(a67c0c40) SHA1(3845839eff8c1624d26937f28ffde67a5fcb4805) )

	ROM_REGION( 0x0120, REGION_PROMS, 0 )
	ROM_LOAD( "1e.bpr",    0x0000, 0x0020, CRC(f5ce825e) SHA1(956f580840f1a7d24bfbd72b2929d14e9ee1b660) ) /* palette */
	ROM_LOAD( "5k.bpr",    0x0020, 0x0100, CRC(2e481ffa) SHA1(bc8979efd43bee8be0ce96ebdacc873a5821e06e) ) /* look-up table */
ROM_END

ROM_START( champbja )
	ROM_REGION( 0x10000, REGION_CPU1, 0 )
	ROM_LOAD( "10",         0x0000, 0x2000, CRC(f7cdaf8e) SHA1(d4c840f2107394fadbcf822d64aaa381ac900367) )
	ROM_LOAD( "09",         0x2000, 0x2000, CRC(9d39e5b3) SHA1(11c1a1d2296c0bf16d7610eaa79b034bfd813740) )
	ROM_LOAD( "08",         0x4000, 0x2000, CRC(53468a0f) SHA1(d4b5ea48b27754eebe593c8b4fcf5bf117f27ae4) )

	ROM_REGION( 0x10000, REGION_CPU2, 0 ) /* 64k for the speech CPU */
	ROM_LOAD( "16.2k",      0x0000, 0x2000, CRC(24c482ee) SHA1(c25bdf77014e095fc11a9a6b17f16858f19db451) )
	ROM_LOAD( "17.2l",      0x2000, 0x2000, CRC(f10b148b) SHA1(d66516d509f6f16e51ee59d27c4867e276064c3f) )
	ROM_LOAD( "18.2n",      0x4000, 0x2000, CRC(2dc484dd) SHA1(28bd68c787d7e6989849ca52009948dbd5cdcc79) )

	ROM_REGION( 0x2000, REGION_GFX1, ROMREGION_DISPOSE )
	ROM_LOAD( "14.5e",      0x0000, 0x2000, CRC(1b8202b3) SHA1(889b77fc3d0cb029baf8c47be260f513f3ed59bd) )

	ROM_REGION( 0x2000, REGION_GFX2, ROMREGION_DISPOSE )
	ROM_LOAD( "15.5g",      0x0000, 0x2000, CRC(a67c0c40) SHA1(3845839eff8c1624d26937f28ffde67a5fcb4805) )

	ROM_REGION( 0x0120, REGION_PROMS, 0 )
	ROM_LOAD( "clr",       0x0000, 0x0020, CRC(8f989357) SHA1(d0916fb5ef4b43bdf84663cd403418ffc5e98c17) ) /* palette */
	ROM_LOAD( "5k.bpr",    0x0020, 0x0100, CRC(2e481ffa) SHA1(bc8979efd43bee8be0ce96ebdacc873a5821e06e) ) /* look-up table */
ROM_END

ROM_START( champbb2 )
	ROM_REGION( 0x10000, REGION_CPU1, 0 )
	ROM_LOAD( "epr5932",      0x0000, 0x2000, CRC(528e3c78) SHA1(ee300201580c1bace783f1340bd4f1ea2a00dffa) )
	ROM_LOAD( "epr5929",      0x2000, 0x2000, CRC(17b6057e) SHA1(67c5aed950acf4d045edf39019066af2896265e1) )
	ROM_LOAD( "epr5930",      0x4000, 0x2000, CRC(b6570a90) SHA1(5a2651aeac986000913b5854792b2d81df6b2fc6) )
	ROM_LOAD( "epr5931",      0x7800, 0x0800, CRC(0592434d) SHA1(a7f61546c39ffdbff46c4db485c9b3f6eefcf1ac) )

	ROM_REGION( 0x10000, REGION_CPU2, 0 )	/* 64k for the speech CPU */
	ROM_LOAD( "epr5933",      0x0000, 0x2000, CRC(26ab3e16) SHA1(019b9d34233a6b7a53e204154b782ceb42915d2b) )
	ROM_LOAD( "epr5934",      0x2000, 0x2000, CRC(7c01715f) SHA1(b15b2001b8c110f2599eee3aeed79f67686ebd7e) )
	ROM_LOAD( "epr5935",      0x4000, 0x2000, CRC(3c911786) SHA1(eea0c467e213d237b5bb9d04b19a418d6090c2dc) )

	ROM_REGION( 0x2000, REGION_GFX1, ROMREGION_DISPOSE )
	ROM_LOAD( "epr5936",      0x0000, 0x2000, CRC(c4a4df75) SHA1(7b85dbf405697b0b8881f910c08f6db6c828b19a) )

	ROM_REGION( 0x2000, REGION_GFX2, ROMREGION_DISPOSE )
	ROM_LOAD( "epr5937",      0x0000, 0x2000, CRC(5c80ec42) SHA1(9b79737577e48a6b2ec20ce145252545955e82c3) )

	ROM_REGION( 0x0120, REGION_PROMS, 0 )
	ROM_LOAD( "pr5957",       0x0000, 0x020, CRC(f5ce825e) SHA1(956f580840f1a7d24bfbd72b2929d14e9ee1b660) ) /* palette */
	ROM_LOAD( "pr5956",       0x0020, 0x100, CRC(872dd450) SHA1(6c1e2c4a2fc072f4bf4996c731adb0b01b347506) ) /* look-up table */
ROM_END

ROM_START( champb2a )
	ROM_REGION( 0x10000, REGION_CPU1, 0 )
	ROM_LOAD( "1.bin",        0x0000, 0x2000, CRC(9b75b44d) SHA1(35b67638a5e48cbe999907e3c9c3a33da9d76bba) )
	ROM_LOAD( "2.bin",        0x2000, 0x2000, CRC(736a1b62) SHA1(24c2d57506754ca789b378a595c03b7591eb5b5c) )
	ROM_LOAD( "3.bin",        0x4000, 0x2000, CRC(cf5f28cb) SHA1(d553f2085c9c8c77b241b4239cc1ad1764b490d0) )
	ROM_LOAD( "4.bin",        0x7800, 0x0800, NO_DUMP )

	/* not in this set, but probably the same */
	ROM_REGION( 0x10000, REGION_CPU2, 0 )	/* 64k for the speech CPU */
	ROM_LOAD( "epr5933",      0x0000, 0x2000, CRC(26ab3e16) SHA1(019b9d34233a6b7a53e204154b782ceb42915d2b) )
	ROM_LOAD( "epr5934",      0x2000, 0x2000, CRC(7c01715f) SHA1(b15b2001b8c110f2599eee3aeed79f67686ebd7e) )
	ROM_LOAD( "epr5935",      0x4000, 0x2000, CRC(3c911786) SHA1(eea0c467e213d237b5bb9d04b19a418d6090c2dc) )

	ROM_REGION( 0x2000, REGION_GFX1, ROMREGION_DISPOSE )
	ROM_LOAD( "epr5936",      0x0000, 0x2000, CRC(c4a4df75) SHA1(7b85dbf405697b0b8881f910c08f6db6c828b19a) )

	ROM_REGION( 0x2000, REGION_GFX2, ROMREGION_DISPOSE )
	ROM_LOAD( "epr5937",      0x0000, 0x2000, CRC(5c80ec42) SHA1(9b79737577e48a6b2ec20ce145252545955e82c3) )

	ROM_REGION( 0x0120, REGION_PROMS, 0 )
	ROM_LOAD( "pr5957",       0x0000, 0x020, CRC(f5ce825e) SHA1(956f580840f1a7d24bfbd72b2929d14e9ee1b660) ) /* palette */
	ROM_LOAD( "pr5956",       0x0020, 0x100, CRC(872dd450) SHA1(6c1e2c4a2fc072f4bf4996c731adb0b01b347506) ) /* look-up table */
ROM_END

GAME( 1983, champbas, 0,        champbas, champbas, 0, ROT0, "Sega",			 "Champion Baseball", 0 )
GAME( 1983, champbbj, champbas, champbb2, champbas, champbb2, ROT0, "Alpha Denshi Co.", "Champion Baseball (Japan set 1)", 0 )
GAME( 1983, champbja, champbas, champbb2, champbas, champbb2, ROT0, "Alpha Denshi Co.", "Champion Baseball (Japan set 2)", 0 )
GAME( 1983, champbb2, 0,        champbb2, champbas, champbb2, ROT0, "Sega",			 "Champion Baseball II (set 1)", 0 )

/* NO DUMP */
GAME( 1983, champb2a, champbb2, champbb2, champbas, champbb2, ROT0, "Alpha Denshi Co.", "Champion Baseball II (set 2)", GAME_NOT_WORKING)

