/***************************************************************************

Crazy Balloon memory map (preliminary)

0000-2fff ROM
4000-43ff RAM
4800-4bff Video RAM
5000-53ff Color RAM

I/O:

read:
00        dsw
01        joystick
02        bit 0-3 from chip PC3259 (bit 3 is the sprite/char collision detection)
          bit 4-7 dsw
03        bit 0 dsw
          bit 1 high score name reset
          bit 2 service
          bit 3 tilt
          bit 4-7 from chip PC3092; coin inputs & start buttons
06-0a-0e  mirror addresses for 02; address lines 2 and 3 go to the PC3256 chip
          so they probably alter its output, while the dsw bits (4-7) stay the same.

write:
01        ?
02        bit 0-3 sprite code bit 4-7 sprite color
03        sprite X pos
04        sprite Y pos
05        music?? to a counter?
06        sound
          bit 0 IRQ enable/acknowledge
          bit 1 sound enable
          bit 2 sound related (to amplifier)
          bit 3 explosion (to 76477)
          bit 4 breath (to 76477)
          bit 5 appear (to 76477)
          bit 6 sound related (to 555)
          bit 7 to chip PC3259
07        to chip PC3092 (bits 0-3)
08        to chip PC3092 (bits 0-3)
          bit 0 seems to be flip screen
          bit 1 might enable coin input
09        to chip PC3092 (bits 0-3)
0a        to chip PC3092 (bits 0-3)
0b        to chip PC3092 (bits 0-3)
0c        MSK (to chip PC3259)
0d        CC (not used)

***************************************************************************/

#include "driver.h"
#include "sound/sn76477.h"
#include "sound/custom.h"
#include "crbaloon.h"



static UINT8 val06,val08,val0a;

static MACHINE_START( crbaloon )
{
	state_save_register_global(val06);
	state_save_register_global(val08);
	state_save_register_global(val0a);
}

static WRITE8_HANDLER( crbaloon_06_w )
{
	val06 = data;

	interrupt_enable_w(offset,data & 1);

	/* SOUND STOP is really sound enable */
	sound_global_enable(data & 0x02);

	/* enable is connected to EXPLOSION */
	SN76477_enable_w(0, (data & 0x08) ? 1 : 0);

	/* BREATH changes slf_res to 10k (middle of two 10k resistors) */
	/* it also puts a tantal capacitor against GND on the output,
       but this section of the schematics is not readable. */
	SN76477_slf_res_w(0, (data & 0x10) ? RES_K(10) : RES_K(20) );

	/* APPEAR is connected to MIXER B */
	SN76477_mixer_b_w(0, (data & 0x20) ? 1 : 0);

	discrete_sound_w(CRBALOON_MUSIC_EN, data & 0x04);
	discrete_sound_w(CRBALOON_LAUGH_EN, data & 0x40);
}

static WRITE8_HANDLER( crbaloon_08_w )
{
	val08 = data;

	crbaloon_flipscreen_w(offset,data & 1);
}

static WRITE8_HANDLER( crbaloon_0a_w )
{
	val0a = data;
}

static READ8_HANDLER( crbaloon_IN2_r )
{
	if (crbaloon_collision != 0)
	{
		return (input_port_2_r(0) & 0xf0) | 0x08;
    }

	/* the following is needed for the game to boot up */
	if (val06 & 0x80)
	{
logerror("PC %04x: %02x high\n",activecpu_get_pc(),offset);
		return (input_port_2_r(0) & 0xf0) | 0x07;
	}
	else
	{
logerror("PC %04x: %02x low\n",activecpu_get_pc(),offset);
		return (input_port_2_r(0) & 0xf0) | 0x07;
	}
}

static READ8_HANDLER( crbaloon_IN3_r )
{
	if (val08 & 0x02)
		/* enable coin & start input? Wild guess!!! */
		return input_port_3_r(0);

	/* the following is needed for the game to boot up */
	if (val0a & 0x01)
	{
logerror("PC %04x: 03 high\n",activecpu_get_pc());
		return (input_port_3_r(0) & 0x0f) | 0x00;
	}
	else
	{
logerror("PC %04x: 03 low\n",activecpu_get_pc());
		return (input_port_3_r(0) & 0x0f) | 0x00;
	}
}


static READ8_HANDLER( crbaloon_IN_r )
{
	switch (offset & 0x03)
	{
		case 0:
			return input_port_0_r(offset);

		case 1:
			return input_port_1_r(offset);

		case 2:
			return crbaloon_IN2_r(offset);

		case 3:
			return crbaloon_IN3_r(offset);
	}

	return 0;
}

static WRITE8_HANDLER( crbaloon_tone_w )
{
	discrete_sound_w(CRBALOON_MUSIC_DATA, data);
}



static ADDRESS_MAP_START( readmem, ADDRESS_SPACE_PROGRAM, 8 )
	AM_RANGE(0x0000, 0x2fff) AM_READ(MRA8_ROM)
	AM_RANGE(0x4000, 0x43ff) AM_READ(MRA8_RAM)
	AM_RANGE(0x4800, 0x4bff) AM_READ(MRA8_RAM)
	AM_RANGE(0x5000, 0x53ff) AM_READ(MRA8_RAM)
ADDRESS_MAP_END

static ADDRESS_MAP_START( writemem, ADDRESS_SPACE_PROGRAM, 8 )
	AM_RANGE(0x0000, 0x2fff) AM_WRITE(MWA8_ROM)
	AM_RANGE(0x4000, 0x43ff) AM_WRITE(MWA8_RAM)
	AM_RANGE(0x4800, 0x4bff) AM_WRITE(crbaloon_videoram_w) AM_BASE(&videoram)
	AM_RANGE(0x5000, 0x53ff) AM_WRITE(crbaloon_colorram_w) AM_BASE(&colorram)
ADDRESS_MAP_END

static ADDRESS_MAP_START( readport, ADDRESS_SPACE_IO, 8 )
	ADDRESS_MAP_FLAGS( AMEF_ABITS(8) )
	AM_RANGE(0x00, 0x0f) AM_READ(crbaloon_IN_r)
ADDRESS_MAP_END

static ADDRESS_MAP_START( writeport, ADDRESS_SPACE_IO, 8 )
	ADDRESS_MAP_FLAGS( AMEF_ABITS(8) )
	AM_RANGE(0x02, 0x04) AM_WRITE(crbaloon_spritectrl_w)
	AM_RANGE(0x05, 0x05) AM_WRITE(crbaloon_tone_w)
	AM_RANGE(0x06, 0x06) AM_WRITE(crbaloon_06_w)
	AM_RANGE(0x08, 0x08) AM_WRITE(crbaloon_08_w)
	AM_RANGE(0x0a, 0x0a) AM_WRITE(crbaloon_0a_w)
ADDRESS_MAP_END



static INPUT_PORTS_START( crbaloon )
	PORT_START_TAG("DSW0") // 0
	PORT_DIPNAME( 0x01, 0x01, "Test?" )
	PORT_DIPSETTING(    0x01, "I/O Check?" )
	PORT_DIPSETTING(    0x00, "RAM Check?" )
	PORT_DIPNAME( 0x02, 0x02, DEF_STR( Cabinet ) )
	PORT_DIPSETTING(    0x02, DEF_STR( Upright ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Cocktail ) )
	PORT_DIPNAME( 0x0c, 0x04, DEF_STR( Lives ) )
	PORT_DIPSETTING(    0x00, "2" )
	PORT_DIPSETTING(    0x04, "3" )
	PORT_DIPSETTING(    0x08, "4" )
	PORT_DIPSETTING(    0x0c, "5" )
	PORT_DIPNAME( 0x10, 0x00, DEF_STR( Bonus_Life ) )
	PORT_DIPSETTING(    0x00, "5000" )
	PORT_DIPSETTING(    0x10, "10000" )
	PORT_DIPNAME( 0xe0, 0x80, DEF_STR( Coinage ) )
	PORT_DIPSETTING(    0x20, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(    0x40, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x60, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x80, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0xa0, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0xc0, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0xe0, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(    0x00, "Disable" )

	PORT_START_TAG("IN0") // 1
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_COCKTAIL

	PORT_START_TAG("DSW1") // 2
	PORT_BIT( 0x0f, IP_ACTIVE_HIGH, IPT_UNKNOWN )	/* from chip PC3259 */
	PORT_DIPNAME( 0x10, 0x10, "Invulnerability (Cheat)")
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

	PORT_START_TAG("IN1") // 3
	PORT_DIPNAME( 0x01, 0x01, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_NAME("High Score Name Reset")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_COIN2 )	/* should be COIN2 */
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_TILT )
	/* the following four bits come from chip PC3092 */
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_COIN1 )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNKNOWN )

	PORT_START_TAG("VR2") // 4
	PORT_ADJUSTER(50, "VR2 - Beep")

	PORT_START_TAG("VR3") // 5
	PORT_ADJUSTER(50, "VR3 - Music")

INPUT_PORTS_END



static const gfx_layout charlayout =
{
	8,8,	/* 8*8 characters */
	256,	/* 256 characters */
	1,	/* 1 bit per pixel */
	{ 0 },
	{ 0, 1, 2, 3, 4, 5, 6, 7 },
	{ 7*8, 6*8, 5*8, 4*8, 3*8, 2*8, 1*8, 0*8 },
	8*8	/* every char takes 8 consecutive bytes */
};
static const gfx_layout spritelayout =
{
	32,32,	/* 32*32 sprites */
	16,	/* 16 sprites */
	1,	/* 1 bit per pixel */
	{ 0 },
	{ 3*32*8+0, 3*32*8+1, 3*32*8+2, 3*32*8+3, 3*32*8+4, 3*32*8+5, 3*32*8+6, 3*32*8+7,
			2*32*8+0, 2*32*8+1, 2*32*8+2, 2*32*8+3, 2*32*8+4, 2*32*8+5, 2*32*8+6, 2*32*8+7,
			1*32*8+0, 1*32*8+1, 1*32*8+2, 1*32*8+3, 1*32*8+4, 1*32*8+5, 1*32*8+6, 1*32*8+7,
			0*32*8+0, 0*32*8+1, 0*32*8+2, 0*32*8+3, 0*32*8+4, 0*32*8+5, 0*32*8+6, 0*32*8+7 },
	{ 31*8, 30*8, 29*8, 28*8, 27*8, 26*8, 25*8, 24*8,
			23*8, 22*8, 21*8, 20*8, 19*8, 18*8, 17*8, 16*8,
			15*8, 14*8, 13*8, 12*8, 11*8, 10*8, 9*8, 8*8,
			7*8, 6*8, 5*8, 4*8, 3*8, 2*8, 1*8, 0*8 },
	32*4*8  /* every sprite takes 128 consecutive bytes */
};


static GFXDECODE_START( crbaloon )
	GFXDECODE_ENTRY( REGION_GFX1, 0, charlayout,   0, 16 )
	GFXDECODE_ENTRY( REGION_GFX2, 0, spritelayout, 0, 16 )
GFXDECODE_END


static struct SN76477interface sn76477_interface =
{
	RES_K( 47),	/*  4 noise_res          */
	RES_K(330),	/*  5 filter_res         */
	CAP_P(470),	/*  6 filter_cap         */
	RES_K(220),	/*  7 decay_res          */
	CAP_U(1.0),	/*  8 attack_decay_cap   */
	RES_K(4.7),	/* 10 attack_res         */
	RES_M(  1),	/* 11 amplitude_res      */
	RES_K(200),	/* 12 feedback_res       */
	5.0,		/* 16 vco_voltage        */
	CAP_P(470),	/* 17 vco_cap            */
	RES_K(330),	/* 18 vco_res            */
	5.0,		/* 19 pitch_voltage      */
	RES_K( 20),	/* 20 slf_res (variable) */
	CAP_P(420),	/* 21 slf_cap            */
	CAP_U(1.0),	/* 23 oneshot_cap        */
	RES_K( 47),	/* 24 oneshot_res        */
	0,			/* 22 vco                */
	0,			/* 26 mixer A            */
	0,			/* 25 mixer B (variable) */
	1,			/* 27 mixer C            */
	1,			/* 1  envelope 1         */
	0,			/* 28 envelope 2         */
	0			/* 9  enable (variable)  */
};


static MACHINE_DRIVER_START( crbaloon )

	/* basic machine hardware */
	MDRV_CPU_ADD(Z80, 3072000)	/* 3.072 MHz ????? */
	MDRV_CPU_PROGRAM_MAP(readmem,writemem)
	MDRV_CPU_IO_MAP(readport,writeport)
	MDRV_CPU_VBLANK_INT(irq0_line_hold,1)

	MDRV_SCREEN_REFRESH_RATE(60)
	MDRV_SCREEN_VBLANK_TIME(DEFAULT_60HZ_VBLANK_DURATION)

	MDRV_MACHINE_START(crbaloon)

	/* video hardware */
	MDRV_VIDEO_ATTRIBUTES(VIDEO_TYPE_RASTER)
	MDRV_SCREEN_FORMAT(BITMAP_FORMAT_INDEXED16)
	MDRV_SCREEN_SIZE(32*8, 32*8)
	MDRV_SCREEN_VISIBLE_AREA(0*8, 32*8-1, 0*8, 28*8-1)
	MDRV_GFXDECODE(crbaloon)
	MDRV_PALETTE_LENGTH(16)
	MDRV_COLORTABLE_LENGTH(16*2)

	MDRV_PALETTE_INIT(crbaloon)
	MDRV_VIDEO_START(crbaloon)
	MDRV_VIDEO_UPDATE(crbaloon)

	/* sound hardware */
	MDRV_SPEAKER_STANDARD_MONO("mono")

	MDRV_SOUND_ADD(SN76477, 0)
	MDRV_SOUND_CONFIG(sn76477_interface)
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "mono", 2.0)

	MDRV_SOUND_ADD_TAG("discrete", DISCRETE, 0)
	MDRV_SOUND_CONFIG_DISCRETE(crbaloon)
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "mono", 1.0)
MACHINE_DRIVER_END



/***************************************************************************

  Game driver(s)

***************************************************************************/

ROM_START( crbaloon )
	ROM_REGION( 0x10000, REGION_CPU1, 0 )
	ROM_LOAD( "cl01.bin",     0x0000, 0x0800, CRC(9d4eef0b) SHA1(a8dd814ac2612073982123c91fa62deaf5bee242) )
	ROM_LOAD( "cl02.bin",     0x0800, 0x0800, CRC(10f7a6f7) SHA1(e672a7dcdaae08b202cfc2e19033846ebb267e1b) )
	ROM_LOAD( "cl03.bin",     0x1000, 0x0800, CRC(44ed6030) SHA1(8bbf5d9e893710138be15e56682037f128c83527) )
	ROM_LOAD( "cl04.bin",     0x1800, 0x0800, CRC(62f66f6c) SHA1(d173b12d6b5e0719d7b25ff0cafebbe64ec6b134) )
	ROM_LOAD( "cl05.bin",     0x2000, 0x0800, CRC(c8f1e2be) SHA1(a4603ce0268fa987f7f780702b6ca04e28759674) )
	ROM_LOAD( "cl06.bin",     0x2800, 0x0800, CRC(7d465691) SHA1(f5dc7abe8db232f702419d126cee6607ea6a5168) )

	ROM_REGION( 0x0800, REGION_GFX1, ROMREGION_DISPOSE )
	ROM_LOAD( "cl07.bin",     0x0000, 0x0800, CRC(2c1fbea8) SHA1(41cf2aef74d56173057886512d989f6fa3682056) )

	ROM_REGION( 0x0800, REGION_GFX2, ROMREGION_DISPOSE )
	ROM_LOAD( "cl08.bin",     0x0000, 0x0800, CRC(ba898659) SHA1(4291059b113ff91896f1f61a4c14956716edfe1e) )
ROM_END

ROM_START( crbalon2 )
	ROM_REGION( 0x10000, REGION_CPU1, 0 )
	ROM_LOAD( "cl01.bin",     0x0000, 0x0800, CRC(9d4eef0b) SHA1(a8dd814ac2612073982123c91fa62deaf5bee242) )
	ROM_LOAD( "crazybal.ep2", 0x0800, 0x0800, CRC(87572086) SHA1(dba842c7c4cb16154ae0da43d71f8f03a56441c3) )
	ROM_LOAD( "crazybal.ep3", 0x1000, 0x0800, CRC(575fe995) SHA1(829db1da27cc9b706db6d9563bd271ffcd42be4a) )
	ROM_LOAD( "cl04.bin",     0x1800, 0x0800, CRC(62f66f6c) SHA1(d173b12d6b5e0719d7b25ff0cafebbe64ec6b134) )
	ROM_LOAD( "cl05.bin",     0x2000, 0x0800, CRC(c8f1e2be) SHA1(a4603ce0268fa987f7f780702b6ca04e28759674) )
	ROM_LOAD( "crazybal.ep6", 0x2800, 0x0800, CRC(fed6ff5c) SHA1(e6ed276949fd1511c6abe97026793193fda36e92) )

	ROM_REGION( 0x0800, REGION_GFX1, ROMREGION_DISPOSE )
	ROM_LOAD( "cl07.bin",     0x0000, 0x0800, CRC(2c1fbea8) SHA1(41cf2aef74d56173057886512d989f6fa3682056) )

	ROM_REGION( 0x0800, REGION_GFX2, ROMREGION_DISPOSE )
	ROM_LOAD( "cl08.bin",     0x0000, 0x0800, CRC(ba898659) SHA1(4291059b113ff91896f1f61a4c14956716edfe1e) )
ROM_END



GAME( 1980, crbaloon, 0,		crbaloon, crbaloon, 0, ROT90, "Taito Corporation", "Crazy Balloon (set 1)", GAME_IMPERFECT_SOUND | GAME_SUPPORTS_SAVE )
GAME( 1980, crbalon2, crbaloon, crbaloon, crbaloon, 0, ROT90, "Taito Corporation", "Crazy Balloon (set 2)", GAME_IMPERFECT_SOUND | GAME_SUPPORTS_SAVE )
