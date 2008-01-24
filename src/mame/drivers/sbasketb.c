/***************************************************************************

Super Basketball memory map (preliminary)
(Hold down Start 1 & Start 2 keys to enter test mode on start up;
 use Start 1 to change modes)

driver by Zsolt Vasvari

MAIN BOARD:
2000-2fff RAM
3000-33ff Color RAM
3400-37ff Video RAM
3800-39ff Sprite RAM
6000-ffff ROM

***************************************************************************/

#include "driver.h"
#include "cpu/m6809/m6809.h"
#include "sound/vlm5030.h"
#include "sound/dac.h"


extern void konami1_decode(void);

extern UINT8 *sbasketb_scroll;
extern UINT8 *sbasketb_palettebank;
extern UINT8 *sbasketb_spriteram_select;

extern WRITE8_HANDLER( sbasketb_videoram_w );
extern WRITE8_HANDLER( sbasketb_colorram_w );
extern WRITE8_HANDLER( sbasketb_flipscreen_w );
extern WRITE8_HANDLER( sbasketb_scroll_w );

extern PALETTE_INIT( sbasketb );
extern VIDEO_START( sbasketb );
extern VIDEO_UPDATE( sbasketb );

extern WRITE8_HANDLER( konami_SN76496_latch_w );
extern WRITE8_HANDLER( konami_SN76496_0_w );
extern WRITE8_HANDLER( hyperspt_sound_w );
extern READ8_HANDLER( hyperspt_sh_timer_r );


static WRITE8_HANDLER( sbasketb_sh_irqtrigger_w )
{
	cpunum_set_input_line_and_vector(Machine, 1,0,HOLD_LINE,0xff);
}

static WRITE8_HANDLER( sbasketb_coin_counter_w )
{
	coin_counter_w(offset,data);
}


static ADDRESS_MAP_START( readmem, ADDRESS_SPACE_PROGRAM, 8 )
	AM_RANGE(0x2000, 0x3bff) AM_READ(MRA8_RAM)
	AM_RANGE(0x3c10, 0x3c10) AM_READ(MRA8_NOP)    /* ???? */
	AM_RANGE(0x3e00, 0x3e00) AM_READ(input_port_0_r)
	AM_RANGE(0x3e01, 0x3e01) AM_READ(input_port_1_r)
	AM_RANGE(0x3e02, 0x3e02) AM_READ(input_port_2_r)
	AM_RANGE(0x3e03, 0x3e03) AM_READ(MRA8_NOP)
	AM_RANGE(0x3e80, 0x3e80) AM_READ(input_port_3_r)
	AM_RANGE(0x3f00, 0x3f00) AM_READ(input_port_4_r)
	AM_RANGE(0x6000, 0xffff) AM_READ(MRA8_ROM)
ADDRESS_MAP_END

static ADDRESS_MAP_START( writemem, ADDRESS_SPACE_PROGRAM, 8 )
	AM_RANGE(0x2000, 0x2fff) AM_WRITE(MWA8_RAM)
	AM_RANGE(0x3000, 0x33ff) AM_WRITE(sbasketb_colorram_w) AM_BASE(&colorram)
	AM_RANGE(0x3400, 0x37ff) AM_WRITE(sbasketb_videoram_w) AM_BASE(&videoram)
	AM_RANGE(0x3800, 0x39ff) AM_WRITE(MWA8_RAM) AM_BASE(&spriteram) AM_SIZE(&spriteram_size)
	AM_RANGE(0x3a00, 0x3bff) AM_WRITE(MWA8_RAM)           /* Probably unused, but initialized */
	AM_RANGE(0x3c00, 0x3c00) AM_WRITE(watchdog_reset_w)
	AM_RANGE(0x3c20, 0x3c20) AM_WRITE(MWA8_RAM) AM_BASE(&sbasketb_palettebank)
	AM_RANGE(0x3c80, 0x3c80) AM_WRITE(sbasketb_flipscreen_w)
	AM_RANGE(0x3c81, 0x3c81) AM_WRITE(interrupt_enable_w)
	AM_RANGE(0x3c83, 0x3c84) AM_WRITE(sbasketb_coin_counter_w)
	AM_RANGE(0x3c85, 0x3c85) AM_WRITE(MWA8_RAM) AM_BASE(&sbasketb_spriteram_select)
	AM_RANGE(0x3d00, 0x3d00) AM_WRITE(soundlatch_w)
	AM_RANGE(0x3d80, 0x3d80) AM_WRITE(sbasketb_sh_irqtrigger_w)
	AM_RANGE(0x3f80, 0x3f80) AM_WRITE(sbasketb_scroll_w)
ADDRESS_MAP_END

static ADDRESS_MAP_START( sound_readmem, ADDRESS_SPACE_PROGRAM, 8 )
	AM_RANGE(0x0000, 0x1fff) AM_READ(MRA8_ROM)
	AM_RANGE(0x4000, 0x43ff) AM_READ(MRA8_RAM)
	AM_RANGE(0x6000, 0x6000) AM_READ(soundlatch_r)
	AM_RANGE(0x8000, 0x8000) AM_READ(hyperspt_sh_timer_r)
ADDRESS_MAP_END

static ADDRESS_MAP_START( sound_writemem, ADDRESS_SPACE_PROGRAM, 8 )
	AM_RANGE(0x4000, 0x43ff) AM_WRITE(MWA8_RAM)
	AM_RANGE(0xa000, 0xa000) AM_WRITE(VLM5030_data_w) /* speech data */
	AM_RANGE(0xc000, 0xdfff) AM_WRITE(hyperspt_sound_w)     /* speech and output controll */
	AM_RANGE(0xe000, 0xe000) AM_WRITE(DAC_0_data_w)
	AM_RANGE(0xe001, 0xe001) AM_WRITE(konami_SN76496_latch_w)  /* Loads the snd command into the snd latch */
	AM_RANGE(0xe002, 0xe002) AM_WRITE(konami_SN76496_0_w)      /* This address triggers the SN chip to read the data port. */
ADDRESS_MAP_END



static INPUT_PORTS_START( sbasketb )
	PORT_START	/* IN0 */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START	/* IN1 */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON2 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_BUTTON3 )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START	/* IN2 */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_COCKTAIL
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_COCKTAIL
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_BUTTON3  ) PORT_COCKTAIL
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START	/* DSW0 */
	PORT_DIPNAME( 0x03, 0x00, DEF_STR( Game_Time ) )
	PORT_DIPSETTING(    0x03, "30" )
	PORT_DIPSETTING(    0x01, "40" )
	PORT_DIPSETTING(    0x02, "50" )
	PORT_DIPSETTING(    0x00, "60" )
	PORT_DIPNAME( 0x04, 0x00, DEF_STR( Cabinet ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Upright ) )
	PORT_DIPSETTING(    0x04, DEF_STR( Cocktail ) )
	PORT_DIPNAME( 0x08, 0x08, "Starting Score" )
	PORT_DIPSETTING(    0x08, "70-78" )
	PORT_DIPSETTING(    0x00, "100-115" )
	PORT_DIPNAME( 0x10, 0x00, "Ranking" )
	PORT_DIPSETTING(    0x00, "Data Remaining" )
	PORT_DIPSETTING(    0x10, "Data Initialized" )
	PORT_DIPNAME( 0x60, 0x60, DEF_STR( Difficulty ) )
	PORT_DIPSETTING(    0x60, DEF_STR( Easy ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Medium ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Hard ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Hardest ) )
	PORT_DIPNAME( 0x80, 0x00, DEF_STR( Demo_Sounds ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START	/* DSW1 */
	PORT_DIPNAME( 0x0f, 0x0f, DEF_STR( Coin_A ) )
	PORT_DIPSETTING(    0x02, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(    0x05, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x08, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x04, DEF_STR( 3C_2C ) )
	PORT_DIPSETTING(    0x01, DEF_STR( 4C_3C ) )
	PORT_DIPSETTING(    0x0f, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x03, DEF_STR( 3C_4C ) )
	PORT_DIPSETTING(    0x07, DEF_STR( 2C_3C ) )
	PORT_DIPSETTING(    0x0e, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x06, DEF_STR( 2C_5C ) )
	PORT_DIPSETTING(    0x0d, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0x0c, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(    0x0b, DEF_STR( 1C_5C ) )
	PORT_DIPSETTING(    0x0a, DEF_STR( 1C_6C ) )
	PORT_DIPSETTING(    0x09, DEF_STR( 1C_7C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Free_Play ) )
	PORT_DIPNAME( 0xf0, 0xf0, DEF_STR( Coin_B ) )
	PORT_DIPSETTING(    0x20, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(    0x50, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x80, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x40, DEF_STR( 3C_2C ) )
	PORT_DIPSETTING(    0x10, DEF_STR( 4C_3C ) )
	PORT_DIPSETTING(    0xf0, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x30, DEF_STR( 3C_4C ) )
	PORT_DIPSETTING(    0x70, DEF_STR( 2C_3C ) )
	PORT_DIPSETTING(    0xe0, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x60, DEF_STR( 2C_5C ) )
	PORT_DIPSETTING(    0xd0, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0xc0, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(    0xb0, DEF_STR( 1C_5C ) )
	PORT_DIPSETTING(    0xa0, DEF_STR( 1C_6C ) )
	PORT_DIPSETTING(    0x90, DEF_STR( 1C_7C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Free_Play ) )
INPUT_PORTS_END




static const gfx_layout charlayout =
{
	8,8,    /* 8*8 characters */
	512,    /* 512 characters */
	4,      /* 4 bits per pixel */
	{ 0, 1, 2, 3 }, /* the bitplanes are packed */
	{ 0*4, 1*4, 2*4, 3*4, 4*4, 5*4, 6*4, 7*4 },
	{ 0*4*8, 1*4*8, 2*4*8, 3*4*8, 4*4*8, 5*4*8, 6*4*8, 7*4*8 },
	8*4*8     /* every char takes 32 consecutive bytes */
};

static const gfx_layout spritelayout =
{
	16,16,  /* 16*16 sprites */
	128 * 3,/* 384 sprites */
	4,      /* 4 bits per pixel */
	{ 0, 1, 2, 3 },        /* the bitplanes are packed */
	{ 0*4, 1*4,  2*4,  3*4,  4*4,  5*4,  6*4,  7*4,
			8*4, 9*4, 10*4, 11*4, 12*4, 13*4, 14*4, 15*4 },
	{ 0*4*16, 1*4*16,  2*4*16,  3*4*16,  4*4*16,  5*4*16,  6*4*16,  7*4*16,
			8*4*16, 9*4*16, 10*4*16, 11*4*16, 12*4*16, 13*4*16, 14*4*16, 15*4*16 },
	32*4*8    /* every sprite takes 128 consecutive bytes */
};



static GFXDECODE_START( sbasketb )
	GFXDECODE_ENTRY( REGION_GFX1, 0, charlayout,       0, 16 )
	GFXDECODE_ENTRY( REGION_GFX2, 0, spritelayout, 16*16, 16*16 )
GFXDECODE_END


static const struct VLM5030interface sbasketb_vlm5030_interface =
{
	REGION_SOUND1,	/* memory region  */
	0           /* memory size    */
};


static MACHINE_DRIVER_START( sbasketb )

	/* basic machine hardware */
	MDRV_CPU_ADD(M6809, 1400000)        /* 1.400 MHz ??? */
	MDRV_CPU_PROGRAM_MAP(readmem,writemem)
	MDRV_CPU_VBLANK_INT(irq0_line_hold,1)

	MDRV_CPU_ADD(Z80,14318000/4)
	/* audio CPU */	/* 3.5795 MHz */
	MDRV_CPU_PROGRAM_MAP(sound_readmem,sound_writemem)

	MDRV_SCREEN_REFRESH_RATE(60)
	MDRV_SCREEN_VBLANK_TIME(DEFAULT_60HZ_VBLANK_DURATION)

	/* video hardware */
	MDRV_VIDEO_ATTRIBUTES(VIDEO_TYPE_RASTER)
	MDRV_SCREEN_FORMAT(BITMAP_FORMAT_INDEXED16)
	MDRV_SCREEN_SIZE(32*8, 32*8)
	MDRV_SCREEN_VISIBLE_AREA(0*8, 32*8-1, 2*8, 30*8-1)
	MDRV_GFXDECODE(sbasketb)
	MDRV_PALETTE_LENGTH(256)
	MDRV_COLORTABLE_LENGTH(16*16+16*16*16)

	MDRV_PALETTE_INIT(sbasketb)
	MDRV_VIDEO_START(sbasketb)
	MDRV_VIDEO_UPDATE(sbasketb)

	/* sound hardware */
	MDRV_SPEAKER_STANDARD_MONO("mono")

	MDRV_SOUND_ADD(DAC, 0)
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.80)

	MDRV_SOUND_ADD(SN76496, 14318180/8)
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "mono", 1.0)

	MDRV_SOUND_ADD(VLM5030, 3580000)
	MDRV_SOUND_CONFIG(sbasketb_vlm5030_interface)
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "mono", 1.0)
MACHINE_DRIVER_END



/***************************************************************************

  Game driver(s)

***************************************************************************/

ROM_START( sbasketb )
	ROM_REGION( 0x10000, REGION_CPU1, 0 )
	ROM_LOAD( "405g05",  0x6000, 0x2000, CRC(336dc0ab) SHA1(0fe47fdbf183683c569785fc6b980337a9cfde95) )
	ROM_LOAD( "405g04",  0x8000, 0x2000, CRC(f064a9bc) SHA1(4f1b94a880385c6ba74cc0883b24f6fec934e35d) )
	ROM_LOAD( "405g03",  0xa000, 0x2000, CRC(b9de7d53) SHA1(5a4e5491ff3511992d949367fd7b5d383c2727db) )
	ROM_LOAD( "405g02",  0xc000, 0x2000, CRC(e98470a0) SHA1(79af25af941fe357a8c9f0a2f11e5558670b8027) )
	ROM_LOAD( "405g01",  0xe000, 0x2000, CRC(1bd0cd2e) SHA1(d162f9b989f718d9882a02a8c64743adf3d8e239) )

	ROM_REGION( 0x10000, REGION_CPU2, 0 )
	ROM_LOAD( "405e13",       0x0000, 0x2000, CRC(1ec7458b) SHA1(a015b982bff5f9e7ece33f2e69ff8c6c2174e710) )

	ROM_REGION( 0x04000, REGION_GFX1, ROMREGION_DISPOSE )
	ROM_LOAD( "405e12",       0x0000, 0x4000, CRC(e02c54da) SHA1(2fa19f3bce894ef05820f95e0b88428e4f946a35) )

	ROM_REGION( 0x0c000, REGION_GFX2, ROMREGION_DISPOSE )
	ROM_LOAD( "405e06",       0x0000, 0x2000, CRC(7e2f5bb2) SHA1(e22008c0ef7ae000dcca7f43a386d43064aaea62) )
	ROM_LOAD( "405e07",       0x2000, 0x2000, CRC(963a44f9) SHA1(03cd7699668b010f27af025ba6bd44509526ec7b) )
	ROM_LOAD( "405e08",       0x4000, 0x2000, CRC(63901deb) SHA1(c65d896298846ed8b70a4d38b32820746214fa5c) )
	ROM_LOAD( "405e09",       0x6000, 0x2000, CRC(e1873677) SHA1(19788e43cc1a6cf5ab375cbc2c745bb6cc8c163d) )
	ROM_LOAD( "405e10",       0x8000, 0x2000, CRC(824815e8) SHA1(470e9d74fa2c397605a74e0bf173a6d9db4cc721) )
	ROM_LOAD( "405e11",       0xa000, 0x2000, CRC(dca9b447) SHA1(12d7e85dc2fc6bd4ea7ad9035ae0b7487e4bc4bc) )

	ROM_REGION( 0x0500, REGION_PROMS, 0 )
	ROM_LOAD( "405e17",       0x0000, 0x0100, CRC(b4c36d57) SHA1(c4a63f57edce2b9588e2394ff54a28f91213d550) ) /* palette red component */
	ROM_LOAD( "405e16",       0x0100, 0x0100, CRC(0b7b03b8) SHA1(81297cb2b0b28b0fc0939a37ff30844d69fb65ac) ) /* palette green component */
	ROM_LOAD( "405e18",       0x0200, 0x0100, CRC(9e533bad) SHA1(611e7af6813caaf2bc36c311ae48a5efd30e6f0c) ) /* palette blue component */
	ROM_LOAD( "405e20",       0x0300, 0x0100, CRC(8ca6de2f) SHA1(67d29708d1a07d17c5dc5793a3293e7ace3a4e19) ) /* character lookup table */
	ROM_LOAD( "405e19",       0x0400, 0x0100, CRC(e0bc782f) SHA1(9f71e696d11a60f771535f6837ecad6132047b0a) ) /* sprite lookup table */

	ROM_REGION( 0x10000, REGION_SOUND1, 0 )     /* 64k for speech rom */
	ROM_LOAD( "405e15",       0x0000, 0x2000, CRC(01bb5ce9) SHA1(f48477b4011befba13c8bcd83e0c9f7deb14a1e1) )
ROM_END

ROM_START( sbasketo )
	ROM_REGION( 0x10000, REGION_CPU1, 0 )
	ROM_LOAD( "405e05",  0x6000, 0x2000, CRC(32ea5b71) SHA1(d917c31d2c9a7229396e4a930e8d27394329533a) )
	ROM_LOAD( "405e04",  0x8000, 0x2000, CRC(7abf3087) SHA1(fbaaaaae0b8bed1bc6ad7f2da267c2ef8bd75b15) )
	ROM_LOAD( "405e03",  0xa000, 0x2000, CRC(9c6fcdcd) SHA1(a644ec98f49f84311829149c181aba25e7681793) )
	ROM_LOAD( "405e02",  0xc000, 0x2000, CRC(0f145648) SHA1(2e238eb0663295887bf6b4905f1fd386db16d82a) )
	ROM_LOAD( "405e01",  0xe000, 0x2000, CRC(6a27f1b1) SHA1(38c0be98fb122a7a6ed833af011bda5663a06510) )

	ROM_REGION( 0x10000, REGION_CPU2, 0 )
	ROM_LOAD( "405e13",       0x0000, 0x2000, CRC(1ec7458b) SHA1(a015b982bff5f9e7ece33f2e69ff8c6c2174e710) )

	ROM_REGION( 0x04000, REGION_GFX1, ROMREGION_DISPOSE )
	ROM_LOAD( "405e12",       0x0000, 0x4000, CRC(e02c54da) SHA1(2fa19f3bce894ef05820f95e0b88428e4f946a35) )

	ROM_REGION( 0x0c000, REGION_GFX2, ROMREGION_DISPOSE )
	ROM_LOAD( "405e06",       0x0000, 0x2000, CRC(7e2f5bb2) SHA1(e22008c0ef7ae000dcca7f43a386d43064aaea62) )
	ROM_LOAD( "405e07",       0x2000, 0x2000, CRC(963a44f9) SHA1(03cd7699668b010f27af025ba6bd44509526ec7b) )
	ROM_LOAD( "405e08",       0x4000, 0x2000, CRC(63901deb) SHA1(c65d896298846ed8b70a4d38b32820746214fa5c) )
	ROM_LOAD( "405e09",       0x6000, 0x2000, CRC(e1873677) SHA1(19788e43cc1a6cf5ab375cbc2c745bb6cc8c163d) )
	ROM_LOAD( "405e10",       0x8000, 0x2000, CRC(824815e8) SHA1(470e9d74fa2c397605a74e0bf173a6d9db4cc721) )
	ROM_LOAD( "405e11",       0xa000, 0x2000, CRC(dca9b447) SHA1(12d7e85dc2fc6bd4ea7ad9035ae0b7487e4bc4bc) )

	ROM_REGION( 0x0500, REGION_PROMS, 0 )
	ROM_LOAD( "405e17",       0x0000, 0x0100, CRC(b4c36d57) SHA1(c4a63f57edce2b9588e2394ff54a28f91213d550) ) /* palette red component */
	ROM_LOAD( "405e16",       0x0100, 0x0100, CRC(0b7b03b8) SHA1(81297cb2b0b28b0fc0939a37ff30844d69fb65ac) ) /* palette green component */
	ROM_LOAD( "405e18",       0x0200, 0x0100, CRC(9e533bad) SHA1(611e7af6813caaf2bc36c311ae48a5efd30e6f0c) ) /* palette blue component */
	ROM_LOAD( "405e20",       0x0300, 0x0100, CRC(8ca6de2f) SHA1(67d29708d1a07d17c5dc5793a3293e7ace3a4e19) ) /* character lookup table */
	ROM_LOAD( "405e19",       0x0400, 0x0100, CRC(e0bc782f) SHA1(9f71e696d11a60f771535f6837ecad6132047b0a) ) /* sprite lookup table */

	ROM_REGION( 0x10000, REGION_SOUND1, 0 )     /* 64k for speech rom */
	ROM_LOAD( "405e15",       0x0000, 0x2000, CRC(01bb5ce9) SHA1(f48477b4011befba13c8bcd83e0c9f7deb14a1e1) )
ROM_END

ROM_START( sbasketu )
	ROM_REGION( 0x10000, REGION_CPU1, 0 )
	ROM_LOAD( "sbb_j13.bin",  0x6000, 0x2000, CRC(263ec36b) SHA1(b445b600726ba4935623311e1a178aeb4a356b0a) )
	ROM_LOAD( "sbb_j11.bin",  0x8000, 0x4000, CRC(0a4d7a82) SHA1(2e0153b41e23284427881258a44bd55be3570eb2) )
	ROM_LOAD( "sbb_j09.bin",  0xc000, 0x4000, CRC(4f9dd9a0) SHA1(97f4c208509d50a7ce4c1ebe8a3f643ad75e833b) )

	ROM_REGION( 0x10000, REGION_CPU2, 0 )
	ROM_LOAD( "405e13",       0x0000, 0x2000, CRC(1ec7458b) SHA1(a015b982bff5f9e7ece33f2e69ff8c6c2174e710) )

	ROM_REGION( 0x04000, REGION_GFX1, ROMREGION_DISPOSE )
	ROM_LOAD( "405e12",       0x0000, 0x4000, CRC(e02c54da) SHA1(2fa19f3bce894ef05820f95e0b88428e4f946a35) )

	ROM_REGION( 0x0c000, REGION_GFX2, ROMREGION_DISPOSE )
	ROM_LOAD( "405e06",       0x0000, 0x2000, CRC(7e2f5bb2) SHA1(e22008c0ef7ae000dcca7f43a386d43064aaea62) )
	ROM_LOAD( "405e07",       0x2000, 0x2000, CRC(963a44f9) SHA1(03cd7699668b010f27af025ba6bd44509526ec7b) )
	ROM_LOAD( "405e08",       0x4000, 0x2000, CRC(63901deb) SHA1(c65d896298846ed8b70a4d38b32820746214fa5c) )
	ROM_LOAD( "405e09",       0x6000, 0x2000, CRC(e1873677) SHA1(19788e43cc1a6cf5ab375cbc2c745bb6cc8c163d) )
	ROM_LOAD( "405e10",       0x8000, 0x2000, CRC(824815e8) SHA1(470e9d74fa2c397605a74e0bf173a6d9db4cc721) )
	ROM_LOAD( "405e11",       0xa000, 0x2000, CRC(dca9b447) SHA1(12d7e85dc2fc6bd4ea7ad9035ae0b7487e4bc4bc) )

	ROM_REGION( 0x0500, REGION_PROMS, 0 )
	ROM_LOAD( "405e17",       0x0000, 0x0100, CRC(b4c36d57) SHA1(c4a63f57edce2b9588e2394ff54a28f91213d550) ) /* palette red component */
	ROM_LOAD( "405e16",       0x0100, 0x0100, CRC(0b7b03b8) SHA1(81297cb2b0b28b0fc0939a37ff30844d69fb65ac) ) /* palette green component */
	ROM_LOAD( "405e18",       0x0200, 0x0100, CRC(9e533bad) SHA1(611e7af6813caaf2bc36c311ae48a5efd30e6f0c) ) /* palette blue component */
	ROM_LOAD( "405e20",       0x0300, 0x0100, CRC(8ca6de2f) SHA1(67d29708d1a07d17c5dc5793a3293e7ace3a4e19) ) /* character lookup table */
	ROM_LOAD( "405e19",       0x0400, 0x0100, CRC(e0bc782f) SHA1(9f71e696d11a60f771535f6837ecad6132047b0a) ) /* sprite lookup table */

	ROM_REGION( 0x10000, REGION_SOUND1, 0 )     /* 64k for speech rom */
	ROM_LOAD( "405e15",       0x0000, 0x2000, CRC(01bb5ce9) SHA1(f48477b4011befba13c8bcd83e0c9f7deb14a1e1) )
ROM_END


static DRIVER_INIT( sbasketb )
{
	konami1_decode();
}


GAME( 1984, sbasketb, 0,        sbasketb, sbasketb, sbasketb, ROT90, "Konami", "Super Basketball (version G)", 0 )
GAME( 1984, sbasketo, sbasketb, sbasketb, sbasketb, sbasketb, ROT90, "Konami", "Super Basketball (version E)", 0 )
GAME( 1984, sbasketu, sbasketb, sbasketb, sbasketb, 0,        ROT90, "Konami", "Super Basketball (not encrypted)", 0 )
