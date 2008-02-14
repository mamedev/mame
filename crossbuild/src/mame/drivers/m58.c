/****************************************************************************

    Irem M58 hardware

    L Taylor
    J Clegg

    Loosely based on the Kung Fu Master driver.

****************************************************************************/

#include "driver.h"
#include "m58.h"
#include "audio/irem.h"

#define MASTER_CLOCK		XTAL_18_432MHz



/*************************************
 *
 *  Outputs
 *
 *************************************/

static WRITE8_HANDLER( yard_flipscreen_w )
{
	flip_screen_set((data & 0x01) ^ (~readinputport(4) & 0x01));

	coin_counter_w(0, data & 0x02);
	coin_counter_w(1, data & 0x20);
}



/*************************************
 *
 *  Memory maps
 *
 *************************************/

static ADDRESS_MAP_START( yard_map, ADDRESS_SPACE_PROGRAM, 8 )
	AM_RANGE(0x0000, 0x5fff) AM_ROM
	AM_RANGE(0x8000, 0x8fff) AM_RAM AM_WRITE(yard_videoram_w) AM_BASE(&videoram)
	AM_RANGE(0x9000, 0x9fff) AM_WRITE(yard_scroll_panel_w)
	AM_RANGE(0xc820, 0xc87f) AM_RAM AM_BASE(&spriteram) AM_SIZE(&spriteram_size)
	AM_RANGE(0xa000, 0xa000) AM_RAM AM_BASE(&yard_scroll_x_low)
	AM_RANGE(0xa200, 0xa200) AM_RAM AM_BASE(&yard_scroll_x_high)
	AM_RANGE(0xa400, 0xa400) AM_RAM AM_BASE(&yard_scroll_y_low)
	AM_RANGE(0xa800, 0xa800) AM_RAM AM_BASE(&yard_score_panel_disabled)
	AM_RANGE(0xd000, 0xd000) AM_READWRITE(input_port_0_r, irem_sound_cmd_w)
	AM_RANGE(0xd001, 0xd001) AM_READWRITE(input_port_1_r, yard_flipscreen_w)
	AM_RANGE(0xd002, 0xd002) AM_READ(input_port_2_r)
	AM_RANGE(0xd003, 0xd003) AM_READ(input_port_3_r)
	AM_RANGE(0xd004, 0xd004) AM_READ(input_port_4_r)
	AM_RANGE(0xe000, 0xefff) AM_RAM
ADDRESS_MAP_END



/*************************************
 *
 *  Port definitions
 *
 *************************************/

static INPUT_PORTS_START( yard )
	PORT_START_TAG("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_SERVICE1 ) PORT_IMPULSE(19) // coin input must be active for 19 frames to be consistently recognized
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START_TAG("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT  ) PORT_8WAY
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN  ) PORT_8WAY
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_UP    ) PORT_8WAY
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON1 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_BUTTON2 )

	PORT_START_TAG("IN2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT  ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN  ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_UP    ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_COCKTAIL
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_COCKTAIL

	PORT_START_TAG("DSW1")
	PORT_DIPNAME( 0x01, 0x01, DEF_STR( Allow_Continue ) ) PORT_DIPLOCATION("SW1:1") /* Listed as "Unused" */
	PORT_DIPSETTING(	0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(	0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, "Defensive Man Pause" ) PORT_DIPLOCATION("SW1:2") /* Listed as "Unused" */
	PORT_DIPSETTING(	0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(	0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x0c, 0x0c, "Time Reduced by Ball Dead" ) PORT_DIPLOCATION("SW1:3,4")
	PORT_DIPSETTING(    0x0c, DEF_STR( Normal ) )
	PORT_DIPSETTING(    0x08, "x1.3" )
	PORT_DIPSETTING(    0x04, "x1.5" )
	PORT_DIPSETTING(    0x00, "x1.8" )
    // coin mode 1
	PORT_DIPNAME( 0xf0, 0xf0, DEF_STR( Coinage ) )      PORT_CONDITION("DSW2", 0x04, PORTCOND_NOTEQUALS, 0x00) PORT_DIPLOCATION("SW1:5,6,7,8")
    PORT_DIPSETTING(    0xa0, DEF_STR( 6C_1C ) )
    PORT_DIPSETTING(    0xb0, DEF_STR( 5C_1C ) )
    PORT_DIPSETTING(    0xc0, DEF_STR( 4C_1C ) )
    PORT_DIPSETTING(    0xd0, DEF_STR( 3C_1C ) )
    PORT_DIPSETTING(    0xe0, DEF_STR( 2C_1C ) )
    PORT_DIPSETTING(    0xf0, DEF_STR( 1C_1C ) )
    PORT_DIPSETTING(    0x70, DEF_STR( 1C_2C ) )
    PORT_DIPSETTING(    0x60, DEF_STR( 1C_3C ) )
    PORT_DIPSETTING(    0x50, DEF_STR( 1C_4C ) )
    PORT_DIPSETTING(    0x40, DEF_STR( 1C_5C ) )
    PORT_DIPSETTING(    0x30, DEF_STR( 1C_6C ) )
    PORT_DIPSETTING(    0x00, DEF_STR( Free_Play ) )
    // coin mode 2
    PORT_DIPNAME( 0x30, 0x30, DEF_STR( Coin_A ) )       PORT_CONDITION("DSW2", 0x04, PORTCOND_EQUALS, 0x00) PORT_DIPLOCATION("SW1:5,6")
    PORT_DIPSETTING(    0x10, DEF_STR( 3C_1C ) )
    PORT_DIPSETTING(    0x20, DEF_STR( 2C_1C ) )
    PORT_DIPSETTING(    0x30, DEF_STR( 1C_1C ) )
    PORT_DIPSETTING(    0x00, DEF_STR( Free_Play ) )
    PORT_DIPNAME( 0xc0, 0xc0, DEF_STR( Coin_B ) )       PORT_CONDITION("DSW2", 0x04, PORTCOND_EQUALS, 0x00) PORT_DIPLOCATION("SW1:7,8")
    PORT_DIPSETTING(    0xc0, DEF_STR( 1C_2C ) )
    PORT_DIPSETTING(    0x80, DEF_STR( 1C_3C ) )
    PORT_DIPSETTING(    0x40, DEF_STR( 1C_5C ) )
    PORT_DIPSETTING(    0x00, DEF_STR( 1C_6C ) )

	PORT_START_TAG("DSW2")
	PORT_DIPNAME( 0x01, 0x01, DEF_STR( Flip_Screen ) ) PORT_DIPLOCATION("SW2:1")
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x00, DEF_STR( Cabinet ) ) PORT_DIPLOCATION("SW2:2")
	PORT_DIPSETTING(    0x00, DEF_STR( Upright ) )
	PORT_DIPSETTING(    0x02, DEF_STR( Cocktail ) )
	PORT_DIPNAME( 0x04, 0x04, "Coin Mode" ) PORT_DIPLOCATION("SW2:3")
	PORT_DIPSETTING(    0x04, "Mode 1" )
	PORT_DIPSETTING(    0x00, "Mode 2" )
	PORT_DIPNAME( 0x08, 0x08, "Slow Motion" ) PORT_DIPLOCATION("SW2:4") /* Listed as "Unused" */
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, "Freeze Picture" ) PORT_DIPLOCATION("SW2:5") /* 2P Start stops gameplay, 1P Start continues */
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Level_Select ) ) PORT_DIPLOCATION("SW2:6") /* Listed as "Unused" */
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, "Invulnerability" ) PORT_DIPLOCATION("SW2:7")
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_SERVICE_DIPLOC(0x80, IP_ACTIVE_LOW, "SW2:8" )
INPUT_PORTS_END

static INPUT_PORTS_START( vsyard )
	PORT_INCLUDE(yard)

	PORT_MODIFY("DSW1")
	PORT_DIPNAME( 0x01, 0x01, "Allow Continue (Vs. Mode)" ) PORT_DIPLOCATION("SW1:1")
	PORT_DIPSETTING( 0x01, DEF_STR( No ) )
	PORT_DIPSETTING( 0x00, DEF_STR( Yes ) )
INPUT_PORTS_END



/*************************************
 *
 *  Graphics layouts
 *
 *************************************/

static const gfx_layout spritelayout =
{
	16, 16,
	RGN_FRAC(1,3),
	3,
	{ RGN_FRAC(2,3), RGN_FRAC(1,3), RGN_FRAC(0,3) },
	{ STEP8(0,1), STEP8(16*8,1) },
	{ STEP16(0,8) },
	32*8
};


static GFXDECODE_START( yard )
	GFXDECODE_ENTRY( REGION_GFX1, 0, gfx_8x8x3_planar,   0, 32 )
	GFXDECODE_ENTRY( REGION_GFX2, 0, spritelayout,     512, 32 )
GFXDECODE_END



/*************************************
 *
 *  Machine drivers
 *
 *************************************/

static MACHINE_DRIVER_START( yard )

	/* basic machine hardware */
	MDRV_CPU_ADD(Z80, MASTER_CLOCK/3/2)
	MDRV_CPU_PROGRAM_MAP(yard_map, 0)
	MDRV_CPU_VBLANK_INT(irq0_line_hold, 1)

	/* video hardware */
	MDRV_VIDEO_ATTRIBUTES(VIDEO_TYPE_RASTER)
	MDRV_GFXDECODE(yard)
	MDRV_PALETTE_LENGTH(256+256+256)

	MDRV_SCREEN_ADD("main", 0)
	MDRV_SCREEN_RAW_PARAMS(MASTER_CLOCK/3, 384, 0, 256, 282, 42, 266)
	MDRV_SCREEN_FORMAT(BITMAP_FORMAT_INDEXED16)

	MDRV_PALETTE_INIT(yard)
	MDRV_VIDEO_START(yard)
	MDRV_VIDEO_UPDATE(yard)

	/* sound hardware */
	MDRV_IMPORT_FROM(m52_large_audio)
MACHINE_DRIVER_END



/*************************************
 *
 *  ROM definitions
 *
 *************************************/

ROM_START( 10yard )
	ROM_REGION( 0x10000, REGION_CPU1, 0 )
	ROM_LOAD( "yf-a-3p-b",    0x0000, 0x2000, CRC(2e205ec2) SHA1(fcfa08f45423b35f2c99d4e6b5474ab1b3a84fec) )
	ROM_LOAD( "yf-a-3n-b",    0x2000, 0x2000, CRC(82fcd980) SHA1(7846705b29961cb95ee1571ee7e16baceea522d4) )
	ROM_LOAD( "yf-a-3m-b",    0x4000, 0x2000, CRC(a8d5c311) SHA1(28edb5cfd943a2262d7e37ef9a7245f7017cbc51) )

	ROM_REGION( 0x10000, REGION_CPU2, 0 )
	ROM_LOAD( "yf-s.3b",      0x8000, 0x2000, CRC(0392a60c) SHA1(68030504eafc58db250099edd3c3323bdb9eff6b) )
	ROM_LOAD( "yf-s.1b",      0xa000, 0x2000, CRC(6588f41a) SHA1(209305efc68171886427216b9a0b37333f40daa8) )
	ROM_LOAD( "yf-s.3a",      0xc000, 0x2000, CRC(bd054e44) SHA1(f10c32c70d60680229fc0891d0e1308015fa69d6) )
	ROM_LOAD( "yf-s.1a",      0xe000, 0x2000, CRC(2490d4c3) SHA1(e4da7b01e8ad075b7e3c8beb6668faff72db9aa2) )

	ROM_REGION( 0x06000, REGION_GFX1, ROMREGION_DISPOSE )
	ROM_LOAD( "yf-a.3e",      0x00000, 0x2000, CRC(77e9e9cc) SHA1(90b0226fc125713dbee2804aeceeb5aa2c8e275e) )	/* chars */
	ROM_LOAD( "yf-a.3d",      0x02000, 0x2000, CRC(854d5ff4) SHA1(9ba09bfabf159facb57faecfe73a6258fa48d152) )
	ROM_LOAD( "yf-a.3c",      0x04000, 0x2000, CRC(0cd8ffad) SHA1(bd1262de3823c34f7394b718477fb5bc58a6e293) )

	ROM_REGION( 0x0c000, REGION_GFX2, ROMREGION_DISPOSE )
	ROM_LOAD( "yf-b.5b",      0x00000, 0x2000, CRC(1299ae30) SHA1(07d47f827d8bc78a41011ec02ab64036fb8a7a18) )	/* sprites */
	ROM_LOAD( "yf-b.5c",      0x02000, 0x2000, CRC(8708b888) SHA1(8c4f305a339f23ec8ed40dfd72fac0f62ee65378) )
	ROM_LOAD( "yf-b.5f",      0x04000, 0x2000, CRC(d9bb8ab8) SHA1(1325308b4c85355298fec4aa3e5fec1b4b13ad86) )
	ROM_LOAD( "yf-b.5e",      0x06000, 0x2000, CRC(47077e8d) SHA1(5f78b15fb360e9926ef11841d5d86f2bd9af04d1) )
	ROM_LOAD( "yf-b.5j",      0x08000, 0x2000, CRC(713ef31f) SHA1(b48df9ed4f26fded3c7eaac3a52b580b2dd60477) )
	ROM_LOAD( "yf-b.5k",      0x0a000, 0x2000, CRC(f49651cc) SHA1(5b87d7360bcd5883ec265b2a01a3e02e10a85345) )

	ROM_REGION( 0x0520, REGION_PROMS, 0 )
	ROM_LOAD( "yard.1c",      0x0000, 0x0100, CRC(08fa5103) SHA1(98af48dafbbaa42f58232bf74ccbf5da41723e71) ) /* chars palette low 4 bits */
	ROM_LOAD( "yard.1d",      0x0100, 0x0100, CRC(7c04994c) SHA1(790bf1616335b9df4943cffcafa48d8e8aee009e) ) /* chars palette high 4 bits */
	ROM_LOAD( "yard.1f",      0x0200, 0x0020, CRC(b8554da5) SHA1(963ca815b5f791b8a7b0937a5d392d5203049eb3) ) /* sprites palette */
	ROM_LOAD( "yard.2h",      0x0220, 0x0100, CRC(e1cdfb06) SHA1(a8cc3456cfc272e3faac80370b2298d8e1f8c2fe) ) /* sprites lookup table */
	ROM_LOAD( "yard.2n",      0x0320, 0x0100, CRC(cd85b646) SHA1(5268db705006058eec308afe474f4df3c15465bb) ) /* radar palette low 4 bits */
	ROM_LOAD( "yard.2m",      0x0420, 0x0100, CRC(45384397) SHA1(e4c662ee81aef63efd8b4a45f85c4a78dc2d419e) ) /* radar palette high 4 bits */
ROM_END

ROM_START( 10yardj )
	ROM_REGION( 0x10000, REGION_CPU1, 0 )
	ROM_LOAD( "yf-a.3p",      0x0000, 0x2000, CRC(4586114f) SHA1(a31c68770e7a7eed805c5ba46af302c2895e3cee) )
	ROM_LOAD( "yf-a.3n",      0x2000, 0x2000, CRC(947fa760) SHA1(bd6c2ee6e6800b063b81dbdd9fc929120019439d) )
	ROM_LOAD( "yf-a.3m",      0x4000, 0x2000, CRC(d4975633) SHA1(84a506ae680a9dd26ef6f33880400e965ccf8260) )

	ROM_REGION( 0x10000, REGION_CPU2, 0 )
	ROM_LOAD( "yf-s.3b",      0x8000, 0x2000, CRC(0392a60c) SHA1(68030504eafc58db250099edd3c3323bdb9eff6b) )
	ROM_LOAD( "yf-s.1b",      0xa000, 0x2000, CRC(6588f41a) SHA1(209305efc68171886427216b9a0b37333f40daa8) )
	ROM_LOAD( "yf-s.3a",      0xc000, 0x2000, CRC(bd054e44) SHA1(f10c32c70d60680229fc0891d0e1308015fa69d6) )
	ROM_LOAD( "yf-s.1a",      0xe000, 0x2000, CRC(2490d4c3) SHA1(e4da7b01e8ad075b7e3c8beb6668faff72db9aa2) )

	ROM_REGION( 0x06000, REGION_GFX1, ROMREGION_DISPOSE )
	ROM_LOAD( "yf-a.3e",      0x00000, 0x2000, CRC(77e9e9cc) SHA1(90b0226fc125713dbee2804aeceeb5aa2c8e275e) )	/* chars */
	ROM_LOAD( "yf-a.3d",      0x02000, 0x2000, CRC(854d5ff4) SHA1(9ba09bfabf159facb57faecfe73a6258fa48d152) )
	ROM_LOAD( "yf-a.3c",      0x04000, 0x2000, CRC(0cd8ffad) SHA1(bd1262de3823c34f7394b718477fb5bc58a6e293) )

	ROM_REGION( 0x0c000, REGION_GFX2, ROMREGION_DISPOSE )
	ROM_LOAD( "yf-b.5b",      0x00000, 0x2000, CRC(1299ae30) SHA1(07d47f827d8bc78a41011ec02ab64036fb8a7a18) )	/* sprites */
	ROM_LOAD( "yf-b.5c",      0x02000, 0x2000, CRC(8708b888) SHA1(8c4f305a339f23ec8ed40dfd72fac0f62ee65378) )
	ROM_LOAD( "yf-b.5f",      0x04000, 0x2000, CRC(d9bb8ab8) SHA1(1325308b4c85355298fec4aa3e5fec1b4b13ad86) )
	ROM_LOAD( "yf-b.5e",      0x06000, 0x2000, CRC(47077e8d) SHA1(5f78b15fb360e9926ef11841d5d86f2bd9af04d1) )
	ROM_LOAD( "yf-b.5j",      0x08000, 0x2000, CRC(713ef31f) SHA1(b48df9ed4f26fded3c7eaac3a52b580b2dd60477) )
	ROM_LOAD( "yf-b.5k",      0x0a000, 0x2000, CRC(f49651cc) SHA1(5b87d7360bcd5883ec265b2a01a3e02e10a85345) )

	ROM_REGION( 0x0520, REGION_PROMS, 0 )
	ROM_LOAD( "yard.1c",      0x0000, 0x0100, CRC(08fa5103) SHA1(98af48dafbbaa42f58232bf74ccbf5da41723e71) ) /* chars palette low 4 bits */
	ROM_LOAD( "yard.1d",      0x0100, 0x0100, CRC(7c04994c) SHA1(790bf1616335b9df4943cffcafa48d8e8aee009e) ) /* chars palette high 4 bits */
	ROM_LOAD( "yard.1f",      0x0200, 0x0020, CRC(b8554da5) SHA1(963ca815b5f791b8a7b0937a5d392d5203049eb3) ) /* sprites palette */
	ROM_LOAD( "yard.2h",      0x0220, 0x0100, CRC(e1cdfb06) SHA1(a8cc3456cfc272e3faac80370b2298d8e1f8c2fe) ) /* sprites lookup table */
	ROM_LOAD( "yard.2n",      0x0320, 0x0100, CRC(cd85b646) SHA1(5268db705006058eec308afe474f4df3c15465bb) ) /* radar palette low 4 bits */
	ROM_LOAD( "yard.2m",      0x0420, 0x0100, CRC(45384397) SHA1(e4c662ee81aef63efd8b4a45f85c4a78dc2d419e) ) /* radar palette high 4 bits */
ROM_END

ROM_START( vs10yard )
	ROM_REGION( 0x10000, REGION_CPU1, 0 )
	ROM_LOAD( "a.3p",         0x0000, 0x2000, CRC(1edac08f) SHA1(c6a3290e9dba663dccf0613853abfab8e912477d) )
	ROM_LOAD( "vyf-a.3m",     0x2000, 0x2000, CRC(3b9330f8) SHA1(b35fe72cf724cfb887906060bbcf40b0c896ccf0) )
	ROM_LOAD( "a.3m",         0x4000, 0x2000, CRC(cf783dad) SHA1(0b1b875ac65ba90c92ca06d0aa01c477b7427322) )

	ROM_REGION( 0x10000, REGION_CPU2, 0 )
	ROM_LOAD( "yf-s.3b",      0x8000, 0x2000, CRC(0392a60c) SHA1(68030504eafc58db250099edd3c3323bdb9eff6b) )
	ROM_LOAD( "yf-s.1b",      0xa000, 0x2000, CRC(6588f41a) SHA1(209305efc68171886427216b9a0b37333f40daa8) )
	ROM_LOAD( "yf-s.3a",      0xc000, 0x2000, CRC(bd054e44) SHA1(f10c32c70d60680229fc0891d0e1308015fa69d6) )
	ROM_LOAD( "yf-s.1a",      0xe000, 0x2000, CRC(2490d4c3) SHA1(e4da7b01e8ad075b7e3c8beb6668faff72db9aa2) )

	ROM_REGION( 0x06000, REGION_GFX1, ROMREGION_DISPOSE )
	ROM_LOAD( "vyf-a.3a",     0x00000, 0x2000, CRC(354d7330) SHA1(0dac87e502d5e9089c4e5ca87c7626940a17e9b2) )	/* chars */
	ROM_LOAD( "vyf-a.3c",     0x02000, 0x2000, CRC(f48eedca) SHA1(6aef3208de8b1dd4078de20c0b5ce96219c79d40) )
	ROM_LOAD( "vyf-a.3d",     0x04000, 0x2000, CRC(7d1b4d93) SHA1(9389de1230b93f529c492af6fb911c00280cae8a) )

	ROM_REGION( 0x0c000, REGION_GFX2, ROMREGION_DISPOSE )
	ROM_LOAD( "yf-b.5b",      0x00000, 0x2000, CRC(1299ae30) SHA1(07d47f827d8bc78a41011ec02ab64036fb8a7a18) )	/* sprites */
	ROM_LOAD( "yf-b.5c",      0x02000, 0x2000, CRC(8708b888) SHA1(8c4f305a339f23ec8ed40dfd72fac0f62ee65378) )
	ROM_LOAD( "yf-b.5f",      0x04000, 0x2000, CRC(d9bb8ab8) SHA1(1325308b4c85355298fec4aa3e5fec1b4b13ad86) )
	ROM_LOAD( "yf-b.5e",      0x06000, 0x2000, CRC(47077e8d) SHA1(5f78b15fb360e9926ef11841d5d86f2bd9af04d1) )
	ROM_LOAD( "yf-b.5j",      0x08000, 0x2000, CRC(713ef31f) SHA1(b48df9ed4f26fded3c7eaac3a52b580b2dd60477) )
	ROM_LOAD( "yf-b.5k",      0x0a000, 0x2000, CRC(f49651cc) SHA1(5b87d7360bcd5883ec265b2a01a3e02e10a85345) )

	ROM_REGION( 0x0520, REGION_PROMS, 0 )
	ROM_LOAD( "yard.1c",      0x0000, 0x0100, CRC(08fa5103) SHA1(98af48dafbbaa42f58232bf74ccbf5da41723e71) ) /* chars palette low 4 bits */
	ROM_LOAD( "yard.1d",      0x0100, 0x0100, CRC(7c04994c) SHA1(790bf1616335b9df4943cffcafa48d8e8aee009e) ) /* chars palette high 4 bits */
	ROM_LOAD( "yard.1f",      0x0200, 0x0020, CRC(b8554da5) SHA1(963ca815b5f791b8a7b0937a5d392d5203049eb3) ) /* sprites palette */
	ROM_LOAD( "yard.2h",      0x0220, 0x0100, CRC(e1cdfb06) SHA1(a8cc3456cfc272e3faac80370b2298d8e1f8c2fe) ) /* sprites lookup table */
	ROM_LOAD( "yard.2n",      0x0320, 0x0100, CRC(cd85b646) SHA1(5268db705006058eec308afe474f4df3c15465bb) ) /* radar palette low 4 bits */
	ROM_LOAD( "yard.2m",      0x0420, 0x0100, CRC(45384397) SHA1(e4c662ee81aef63efd8b4a45f85c4a78dc2d419e) ) /* radar palette high 4 bits */
ROM_END

ROM_START( vs10yarj )
	ROM_REGION( 0x10000, REGION_CPU1, 0 )
	ROM_LOAD( "vyf-a.3n",     0x0000, 0x2000, CRC(418e01fc) SHA1(56a6515735cd88ec803e24574a28aef823a5d36b) )
	ROM_LOAD( "vyf-a.3m",     0x2000, 0x2000, CRC(3b9330f8) SHA1(b35fe72cf724cfb887906060bbcf40b0c896ccf0) )
	ROM_LOAD( "vyf-a.3k",     0x4000, 0x2000, CRC(a0ec15bb) SHA1(a5ce9341e9d05e33c025ac62a27faf738c88326e) )

	ROM_REGION( 0x10000, REGION_CPU2, 0 )
	ROM_LOAD( "yf-s.3b",      0x8000, 0x2000, CRC(0392a60c) SHA1(68030504eafc58db250099edd3c3323bdb9eff6b) )
	ROM_LOAD( "yf-s.1b",      0xa000, 0x2000, CRC(6588f41a) SHA1(209305efc68171886427216b9a0b37333f40daa8) )
	ROM_LOAD( "yf-s.3a",      0xc000, 0x2000, CRC(bd054e44) SHA1(f10c32c70d60680229fc0891d0e1308015fa69d6) )
	ROM_LOAD( "yf-s.1a",      0xe000, 0x2000, CRC(2490d4c3) SHA1(e4da7b01e8ad075b7e3c8beb6668faff72db9aa2) )

	ROM_REGION( 0x06000, REGION_GFX1, ROMREGION_DISPOSE )
	ROM_LOAD( "vyf-a.3a",     0x00000, 0x2000, CRC(354d7330) SHA1(0dac87e502d5e9089c4e5ca87c7626940a17e9b2) )	/* chars */
	ROM_LOAD( "vyf-a.3c",     0x02000, 0x2000, CRC(f48eedca) SHA1(6aef3208de8b1dd4078de20c0b5ce96219c79d40) )
	ROM_LOAD( "vyf-a.3d",     0x04000, 0x2000, CRC(7d1b4d93) SHA1(9389de1230b93f529c492af6fb911c00280cae8a) )

	ROM_REGION( 0x0c000, REGION_GFX2, ROMREGION_DISPOSE )
	ROM_LOAD( "yf-b.5b",      0x00000, 0x2000, CRC(1299ae30) SHA1(07d47f827d8bc78a41011ec02ab64036fb8a7a18) )	/* sprites */
	ROM_LOAD( "yf-b.5c",      0x02000, 0x2000, CRC(8708b888) SHA1(8c4f305a339f23ec8ed40dfd72fac0f62ee65378) )
	ROM_LOAD( "yf-b.5f",      0x04000, 0x2000, CRC(d9bb8ab8) SHA1(1325308b4c85355298fec4aa3e5fec1b4b13ad86) )
	ROM_LOAD( "yf-b.5e",      0x06000, 0x2000, CRC(47077e8d) SHA1(5f78b15fb360e9926ef11841d5d86f2bd9af04d1) )
	ROM_LOAD( "yf-b.5j",      0x08000, 0x2000, CRC(713ef31f) SHA1(b48df9ed4f26fded3c7eaac3a52b580b2dd60477) )
	ROM_LOAD( "yf-b.5k",      0x0a000, 0x2000, CRC(f49651cc) SHA1(5b87d7360bcd5883ec265b2a01a3e02e10a85345) )

	ROM_REGION( 0x0520, REGION_PROMS, 0 )
	ROM_LOAD( "yard.1c",      0x0000, 0x0100, CRC(08fa5103) SHA1(98af48dafbbaa42f58232bf74ccbf5da41723e71) ) /* chars palette low 4 bits */
	ROM_LOAD( "yard.1d",      0x0100, 0x0100, CRC(7c04994c) SHA1(790bf1616335b9df4943cffcafa48d8e8aee009e) ) /* chars palette high 4 bits */
	ROM_LOAD( "yard.1f",      0x0200, 0x0020, CRC(b8554da5) SHA1(963ca815b5f791b8a7b0937a5d392d5203049eb3) ) /* sprites palette */
	ROM_LOAD( "yard.2h",      0x0220, 0x0100, CRC(e1cdfb06) SHA1(a8cc3456cfc272e3faac80370b2298d8e1f8c2fe) ) /* sprites lookup table */
	ROM_LOAD( "yard.2n",      0x0320, 0x0100, CRC(cd85b646) SHA1(5268db705006058eec308afe474f4df3c15465bb) ) /* radar palette low 4 bits */
	ROM_LOAD( "yard.2m",      0x0420, 0x0100, CRC(45384397) SHA1(e4c662ee81aef63efd8b4a45f85c4a78dc2d419e) ) /* radar palette high 4 bits */
ROM_END



/*************************************
 *
 *  Game drivers
 *
 *************************************/

GAME( 1983, 10yard,        0, yard, yard,   0, ROT0, "Irem", "10-Yard Fight (World)", 0 )
GAME( 1983, 10yardj,  10yard, yard, yard,   0, ROT0, "Irem", "10-Yard Fight (Japan)", 0 )
GAME( 1984, vs10yard, 10yard, yard, vsyard, 0, ROT0, "Irem", "Vs 10-Yard Fight (World, 11/05/84)", 0 )
GAME( 1984, vs10yarj, 10yard, yard, vsyard, 0, ROT0, "Irem", "Vs 10-Yard Fight (Japan)", 0 )
