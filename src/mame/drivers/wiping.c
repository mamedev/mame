// license:BSD-3-Clause
// copyright-holders:Allard van der Bas
/***************************************************************************
                Wiping
                (C) 1982 Nichibutsu

                    driver by

            Allard van der Bas (allard@mindless.com)

1 x Z80 CPU main game, 1 x Z80 with ???? sound hardware.
----------------------------------------------------------------------------
Main processor :

0xA800 - 0xA807 : 64 bits of input and dipswitches.

dip: 0.7 1.7 2.7
       0   0   0    coin 1: 1 coin 0 credit.

       1   1   1    coin 1: 1 coin 7 credit.

dip: 3.7 4.7 5.7
       0   0   0    coin 2: 0 coin 1 credit.

       1   1   1    coin 2: 7 coin 1 credit.

dip:  7.6
    0       bonus at 30K and 70K
    1       bonus at 50K and 150K

dip: 6.7 7.7
       0   0        2 lives
       0   1        3 lives
       1   0        4 lives
       1   1        5 lives

***************************************************************************/
#include "emu.h"
#include "cpu/z80/z80.h"
#include "audio/wiping.h"
#include "includes/wiping.h"


void wiping_state::machine_start()
{
	save_item(NAME(m_flipscreen));
	save_item(NAME(m_main_irq_mask));
	save_item(NAME(m_sound_irq_mask));
}

/* input ports are rotated 90 degrees */
READ8_MEMBER(wiping_state::ports_r)
{
	int i,res;
	static const char *const portnames[] = { "P1", "P2", "IN2", "IN3", "IN4", "IN5", "SYSTEM", "DSW" };

	res = 0;
	for (i = 0; i < 8; i++)
		res |= ((ioport(portnames[i])->read() >> offset) & 1) << i;

	return res;
}

WRITE8_MEMBER(wiping_state::subcpu_reset_w)
{
	m_audiocpu->set_input_line(INPUT_LINE_RESET, (data & 1) ? CLEAR_LINE : ASSERT_LINE);
}

WRITE8_MEMBER(wiping_state::main_irq_mask_w)
{
	m_main_irq_mask = data & 1;
}

WRITE8_MEMBER(wiping_state::sound_irq_mask_w)
{
	m_sound_irq_mask = data & 1;
}

static ADDRESS_MAP_START( main_map, AS_PROGRAM, 8, wiping_state )
	AM_RANGE(0x0000, 0x5fff) AM_ROM
	AM_RANGE(0x8000, 0x83ff) AM_SHARE("videoram")
	AM_RANGE(0x8400, 0x87ff) AM_SHARE("colorram")
	AM_RANGE(0x8800, 0x88ff) AM_SHARE("spriteram")
	AM_RANGE(0x8000, 0x8bff) AM_RAM
	AM_RANGE(0x9000, 0x93ff) AM_RAM AM_SHARE("share1")
	AM_RANGE(0x9800, 0x9bff) AM_RAM AM_SHARE("share2")
	AM_RANGE(0xa000, 0xa000) AM_WRITE(main_irq_mask_w)
	AM_RANGE(0xa002, 0xa002) AM_WRITE(flipscreen_w)
	AM_RANGE(0xa003, 0xa003) AM_WRITE(subcpu_reset_w)
	AM_RANGE(0xa800, 0xa807) AM_READ(ports_r)
	AM_RANGE(0xb000, 0xb7ff) AM_RAM
	AM_RANGE(0xb800, 0xb800) AM_WRITE(watchdog_reset_w)
ADDRESS_MAP_END

static ADDRESS_MAP_START( sound_map, AS_PROGRAM, 8, wiping_state )
	AM_RANGE(0x0000, 0x1fff) AM_ROM
	AM_RANGE(0x4000, 0x7fff) AM_DEVWRITE("wiping", wiping_sound_device, sound_w)
	AM_RANGE(0x9000, 0x93ff) AM_RAM AM_SHARE("share1")
	AM_RANGE(0x9800, 0x9bff) AM_RAM AM_SHARE("share2")
	AM_RANGE(0xa001, 0xa001) AM_WRITE(sound_irq_mask_w)
ADDRESS_MAP_END


static INPUT_PORTS_START( wiping )
	PORT_START("P1")    /* 0 */
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP ) PORT_4WAY
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN ) PORT_4WAY
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_4WAY
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_4WAY
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_BUTTON1 )
	PORT_BIT( 0xe0, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("P2")    /* 1 */
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP ) PORT_4WAY PORT_COCKTAIL
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN ) PORT_4WAY PORT_COCKTAIL
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_4WAY PORT_COCKTAIL
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_4WAY PORT_COCKTAIL
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_COCKTAIL
	PORT_BIT( 0xe0, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("IN2")   /* 2 */
	PORT_BIT( 0xff, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("IN3")   /* 3 */
	PORT_BIT( 0xff, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("IN4")   /* 4 */
	PORT_BIT( 0xff, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("IN5")   /* 5 */
	PORT_BIT( 0xff, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("SYSTEM")    /* 6 */
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x05, IP_ACTIVE_LOW, IPT_COIN2 )  /* note that this changes two bits */
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_START1 )
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_START2 )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Cabinet ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Upright ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Cocktail ) )
	PORT_SERVICE( 0x40, IP_ACTIVE_HIGH )
	PORT_DIPNAME( 0x80, 0x00, DEF_STR( Bonus_Life ) )
	PORT_DIPSETTING(    0x00, "30000 70000" )
	PORT_DIPSETTING(    0x80, "50000 150000" )

	PORT_START("DSW")   /* 7 */
	PORT_DIPNAME( 0x07, 0x01, DEF_STR( Coin_B )  )
	PORT_DIPSETTING(    0x01, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x02, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x03, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0x04, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(    0x05, DEF_STR( 1C_5C ) )
	PORT_DIPSETTING(    0x06, DEF_STR( 1C_6C ) )
	PORT_DIPSETTING(    0x07, DEF_STR( 1C_7C ) )
//  PORT_DIPSETTING(    0x00, "Disable" )
	PORT_DIPNAME( 0x38, 0x08, DEF_STR( Coin_A ) )
	PORT_DIPSETTING(    0x38, DEF_STR( 7C_1C ) )
	PORT_DIPSETTING(    0x30, DEF_STR( 6C_1C ) )
	PORT_DIPSETTING(    0x28, DEF_STR( 5C_1C ) )
	PORT_DIPSETTING(    0x20, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(    0x18, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x10, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x08, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Free_Play ) )
	PORT_DIPNAME( 0xc0, 0x40, DEF_STR( Lives ) )
	PORT_DIPSETTING(    0x00, "2" )
	PORT_DIPSETTING(    0x40, "3" )
	PORT_DIPSETTING(    0x80, "4" )
	PORT_DIPSETTING(    0xc0, "5" )
INPUT_PORTS_END

/* identical apart from bonus life */
static INPUT_PORTS_START( rugrats )
	PORT_START("P1")    /* 0 */
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP ) PORT_4WAY
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN ) PORT_4WAY
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_4WAY
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_4WAY
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_BUTTON1 )
	PORT_BIT( 0xe0, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("P2")    /* 1 */
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP ) PORT_4WAY PORT_COCKTAIL
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN ) PORT_4WAY PORT_COCKTAIL
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_4WAY PORT_COCKTAIL
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_4WAY PORT_COCKTAIL
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_COCKTAIL
	PORT_BIT( 0xe0, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("IN2")   /* 2 */
	PORT_BIT( 0xff, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("IN3")   /* 3 */
	PORT_BIT( 0xff, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("IN4")   /* 4 */
	PORT_BIT( 0xff, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("IN5")   /* 5 */
	PORT_BIT( 0xff, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("SYSTEM")    /* 6 */
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x05, IP_ACTIVE_LOW, IPT_COIN2 )  /* note that this changes two bits */
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_START1 )
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_START2 )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Cabinet ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Upright ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Cocktail ) )
	PORT_SERVICE( 0x40, IP_ACTIVE_HIGH )
	PORT_DIPNAME( 0x80, 0x00, DEF_STR( Bonus_Life ) )
	PORT_DIPSETTING(    0x00, "100000 200000" )
	PORT_DIPSETTING(    0x80, "150000 300000" )

	PORT_START("DSW")   /* 7 */
	PORT_DIPNAME( 0x07, 0x01, DEF_STR( Coin_B )  )
	PORT_DIPSETTING(    0x01, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x02, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x03, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0x04, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(    0x05, DEF_STR( 1C_5C ) )
	PORT_DIPSETTING(    0x06, DEF_STR( 1C_6C ) )
	PORT_DIPSETTING(    0x07, DEF_STR( 1C_7C ) )
//  PORT_DIPSETTING(    0x00, "Disable" )
	PORT_DIPNAME( 0x38, 0x08, DEF_STR( Coin_A ) )
	PORT_DIPSETTING(    0x38, DEF_STR( 7C_1C ) )
	PORT_DIPSETTING(    0x30, DEF_STR( 6C_1C ) )
	PORT_DIPSETTING(    0x28, DEF_STR( 5C_1C ) )
	PORT_DIPSETTING(    0x20, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(    0x18, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x10, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x08, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Free_Play ) )
	PORT_DIPNAME( 0xc0, 0x40, DEF_STR( Lives ) )
	PORT_DIPSETTING(    0x00, "2" )
	PORT_DIPSETTING(    0x40, "3" )
	PORT_DIPSETTING(    0x80, "4" )
	PORT_DIPSETTING(    0xc0, "5" )
INPUT_PORTS_END



static const gfx_layout charlayout =
{
	8,8,    /* 8*8 characters */
	256,    /* 256 characters */
	2,  /* 2 bits per pixel */
	{ 0, 4 },   /* the two bitplanes are packed in one byte */
	{ 0, 1, 2, 3, 8+0, 8+1, 8+2, 8+3 },
	{ 0*16, 1*16, 2*16, 3*16, 4*16, 5*16, 6*16, 7*16 },
	16*8    /* every char takes 16 consecutive bytes */
};

static const gfx_layout spritelayout =
{
	16,16,  /* 16*16 sprites */
	128,    /* 128 sprites */
	2,  /* 2 bits per pixel */
	{ 0, 4 },   /* the two bitplanes are packed in one byte */
	{ 0, 1, 2, 3, 8+0, 8+1, 8+2, 8+3,
			16*8+0, 16*8+1, 16*8+2, 16*8+3, 17*8+0, 17*8+1, 17*8+2, 17*8+3 },
	{ 0*16, 1*16, 2*16, 3*16, 4*16, 5*16, 6*16, 7*16,
			16*16, 17*16, 18*16, 19*16, 20*16, 21*16, 22*16, 23*16 },
	64*8    /* every sprite takes 64 consecutive bytes */
};

static GFXDECODE_START( wiping )
	GFXDECODE_ENTRY( "gfx1", 0, charlayout,      0, 64 )
	GFXDECODE_ENTRY( "gfx2", 0, spritelayout, 64*4, 64 )
GFXDECODE_END

INTERRUPT_GEN_MEMBER(wiping_state::vblank_irq)
{
	if(m_main_irq_mask)
		device.execute().set_input_line(0, HOLD_LINE);
}

INTERRUPT_GEN_MEMBER(wiping_state::sound_timer_irq)
{
	if(m_sound_irq_mask)
		device.execute().set_input_line(0, HOLD_LINE);
}



static MACHINE_CONFIG_START( wiping, wiping_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", Z80,18432000/6) /* 3.072 MHz */
	MCFG_CPU_PROGRAM_MAP(main_map)
	MCFG_CPU_VBLANK_INT_DRIVER("screen", wiping_state,  vblank_irq)

	MCFG_CPU_ADD("audiocpu", Z80,18432000/6)    /* 3.072 MHz */
	MCFG_CPU_PROGRAM_MAP(sound_map)
	MCFG_CPU_PERIODIC_INT_DRIVER(wiping_state, sound_timer_irq, 120)    /* periodic interrupt, don't know about the frequency */

	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MCFG_SCREEN_SIZE(36*8, 28*8)
	MCFG_SCREEN_VISIBLE_AREA(0*8, 36*8-1, 0*8, 28*8-1)
	MCFG_SCREEN_UPDATE_DRIVER(wiping_state, screen_update)
	MCFG_SCREEN_PALETTE("palette")
	MCFG_SCREEN_ORIENTATION(ROT90)

	MCFG_GFXDECODE_ADD("gfxdecode", "palette", wiping)
	MCFG_PALETTE_ADD("palette", 64*4+64*4)
	MCFG_PALETTE_INDIRECT_ENTRIES(32)
	MCFG_PALETTE_INIT_OWNER(wiping_state, wiping)

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")

	MCFG_SOUND_ADD("wiping", WIPING, 0)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 1.0)
MACHINE_CONFIG_END



/***************************************************************************

  Game driver(s)

***************************************************************************/

ROM_START( wiping )
	ROM_REGION( 0x10000, "maincpu", 0 ) /* main cpu code */
	ROM_LOAD( "1",            0x0000, 0x2000, CRC(b55d0d19) SHA1(dac6096d3ee9dd8b1b6da5c2c613b54ce303cb7b) )
	ROM_LOAD( "2",            0x2000, 0x2000, CRC(b1f96e47) SHA1(8f3f882a3c366e6a2d2682603d425eb0491b5487) )
	ROM_LOAD( "3",            0x4000, 0x2000, CRC(c67bab5a) SHA1(3d74ed4be5a6bdc02cf1feb3ce3f4b1607ec6b80) )

	ROM_REGION( 0x10000, "audiocpu", 0 )    /* sound cpu */
	ROM_LOAD( "4",            0x0000, 0x1000, CRC(a1547e18) SHA1(1f86d770e42ff1d94bf1f8b12f9b74accc3bb193) )

	ROM_REGION( 0x1000, "gfx1", 0 )
	ROM_LOAD( "8",            0x0000, 0x1000, CRC(601160f6) SHA1(2465a1319d442a96d3b1b5e3ad544b0a0126762c) ) /* chars */

	ROM_REGION( 0x2000, "gfx2", 0 )
	ROM_LOAD( "7",            0x0000, 0x2000, CRC(2c2cc054) SHA1(31851983de61bb8616856b0067c4e237819df5fb) ) /* sprites */

	ROM_REGION( 0x0220, "proms", 0 )
	ROM_LOAD( "wip-g13.bin",  0x0000, 0x0020, CRC(b858b897) SHA1(5fc87e210bdaa675fdf8c6762526c345bd451eab) )    /* palette */
	ROM_LOAD( "wip-f4.bin",   0x0020, 0x0100, CRC(3f56c8d5) SHA1(7d279b2f29911c44b4136068770accf7196057d7) )    /* char lookup table */
	ROM_LOAD( "wip-e11.bin",  0x0120, 0x0100, CRC(e7400715) SHA1(c67193e5f0a43942ddf03058a0bb8b3275308459) )    /* sprite lookup table */

	ROM_REGION( 0x4000, "samples", 0 )  /* samples */
	ROM_LOAD( "rugr5c8",      0x0000, 0x2000, CRC(67bafbbf) SHA1(2085492b58ce44f61a42320c54595b79fdf7a91c) )
	ROM_LOAD( "rugr6c9",      0x2000, 0x2000, CRC(cac84a87) SHA1(90f6c514d0cdbeb4c8c979597db79ebcdf443df4) )

	ROM_REGION( 0x0200, "soundproms", 0 )   /* 4bit->8bit sample expansion PROMs */
	ROM_LOAD( "wip-e8.bin",   0x0000, 0x0100, CRC(bd2c080b) SHA1(9782bb5001e96db56bc29df398187f700bce4f8e) )    /* low 4 bits */
	ROM_LOAD( "wip-e9.bin",   0x0100, 0x0100, CRC(4017a2a6) SHA1(dadef2de7a1119758c8e6d397aa42815b0218889) )    /* high 4 bits */
ROM_END

ROM_START( rugrats )
	ROM_REGION( 0x10000, "maincpu", 0 ) /* main cpu code */
	ROM_LOAD( "rugr1d1",      0x0000, 0x2000, CRC(e7e1bd6d) SHA1(985799b1bfd001c6304e6166180745cb019f834e) )
	ROM_LOAD( "rugr2d2",      0x2000, 0x2000, CRC(5f47b9ad) SHA1(2d3eb737ea8e86691293e432e866d2623d6b6b1b) )
	ROM_LOAD( "rugr3d3",      0x4000, 0x2000, CRC(3d748d1a) SHA1(2b301119b6eb3f0f9bb2ad734cff1d25365dfe99) )

	ROM_REGION( 0x10000, "audiocpu", 0 )    /* sound cpu */
	ROM_LOAD( "rugr4c4",      0x0000, 0x2000, CRC(d4a92c38) SHA1(4a31cfef9f084b4d2934595155bf0f3dd589efb3) )

	ROM_REGION( 0x1000, "gfx1", 0 )
	ROM_LOAD( "rugr8d2",      0x0000, 0x1000, CRC(a3dcaca5) SHA1(d71f9090bf95dfd035ee0e0619a1cce575033cf3) ) /* chars */

	ROM_REGION( 0x2000, "gfx2", 0 )
	ROM_LOAD( "rugr7c13",     0x0000, 0x2000, CRC(fe1191dd) SHA1(80ebf093f7a32f4cc9dc89dcc44cab6e3db4fca1) ) /* sprites */

	ROM_REGION( 0x0220, "proms", 0 )
	ROM_LOAD( "prom.13g",     0x0000, 0x0020, CRC(f21238f0) SHA1(944627d1551453c7f828d96b83fd4eeb038b20ad) )    /* palette */
	ROM_LOAD( "prom.4f",      0x0020, 0x0100, CRC(cfc90f3d) SHA1(99f7dc0d14c62d4c676c96310c219c696c9a7897) )    /* char lookup table */
	ROM_LOAD( "prom.11e",     0x0120, 0x0100, CRC(cfc90f3d) SHA1(99f7dc0d14c62d4c676c96310c219c696c9a7897) )    /* sprite lookup table */

	ROM_REGION( 0x4000, "samples", 0 )  /* samples */
	ROM_LOAD( "rugr5c8",      0x0000, 0x2000, CRC(67bafbbf) SHA1(2085492b58ce44f61a42320c54595b79fdf7a91c) )
	ROM_LOAD( "rugr6c9",      0x2000, 0x2000, CRC(cac84a87) SHA1(90f6c514d0cdbeb4c8c979597db79ebcdf443df4) )

	ROM_REGION( 0x0200, "soundproms", 0 )   /* 4bit->8bit sample expansion PROMs */
	ROM_LOAD( "wip-e8.bin",   0x0000, 0x0100, CRC(bd2c080b) SHA1(9782bb5001e96db56bc29df398187f700bce4f8e) )    /* low 4 bits */
	ROM_LOAD( "wip-e9.bin",   0x0100, 0x0100, CRC(4017a2a6) SHA1(dadef2de7a1119758c8e6d397aa42815b0218889) )    /* high 4 bits */
ROM_END



GAME( 1982, wiping,  0,      wiping, wiping, driver_device,  0, ROT90, "Nichibutsu", "Wiping", MACHINE_SUPPORTS_SAVE )
GAME( 1983, rugrats, wiping, wiping, rugrats, driver_device, 0, ROT90, "Nichibutsu", "Rug Rats", MACHINE_SUPPORTS_SAVE )
