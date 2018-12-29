// license:BSD-3-Clause
// copyright-holders:Lee Taylor, Chris Moore
/*************************************************************************
    Universal Cheeky Mouse Driver
    (c)Lee Taylor May/June 1998, All rights reserved.

**************************************************************************/

#include "emu.h"
#include "includes/cheekyms.h"

#include "cpu/z80/z80.h"


INPUT_CHANGED_MEMBER(cheekyms_state::coin_inserted)
{
	/* this starts a 556 one-shot timer (and triggers a sound effect) */
	if (newval)
		m_maincpu->pulse_input_line(INPUT_LINE_NMI, attotime::zero);
}


void cheekyms_state::main_map(address_map &map)
{
	map(0x0000, 0x1fff).rom();
	map(0x3000, 0x33ff).ram();
	map(0x3800, 0x3bff).ram().share("videoram");
}

void cheekyms_state::io_map(address_map &map)
{
	map.global_mask(0xff);
	map(0x00, 0x00).portr("DSW");
	map(0x01, 0x01).portr("INPUTS");
	map(0x20, 0x3f).writeonly().share("spriteram");
	map(0x40, 0x40).w(FUNC(cheekyms_state::port_40_w));
	map(0x80, 0x80).w(FUNC(cheekyms_state::port_80_w)).share("port_80");
}


static INPUT_PORTS_START( cheekyms )
	PORT_START("DSW")
	PORT_DIPNAME( 0x03, 0x01, DEF_STR( Lives ) )
	PORT_DIPSETTING(    0x00, "2" )
	PORT_DIPSETTING(    0x01, "3" )
	PORT_DIPSETTING(    0x02, "4" )
	PORT_DIPSETTING(    0x03, "5" )
	PORT_DIPNAME( 0x0c, 0x04, DEF_STR( Coinage ) )
	PORT_DIPSETTING(    0x08, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x04, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_2C ) )
	//PORT_DIPSETTING(    0x0c, DEF_STR( 1C_1C ) )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Cabinet ) )
	PORT_DIPSETTING(    0x10, DEF_STR( Upright ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Cocktail ) )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Demo_Sounds ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x20, DEF_STR( On ) )
	PORT_DIPNAME( 0xc0, 0x40, DEF_STR( Bonus_Life ) )
	PORT_DIPSETTING(    0x40, "3000" )
	PORT_DIPSETTING(    0x80, "4500" )
	PORT_DIPSETTING(    0xc0, "6000" )
	PORT_DIPSETTING(    0x00, DEF_STR( None ) )

	PORT_START("INPUTS")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_COCKTAIL
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_COCKTAIL
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_COCKTAIL
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_BUTTON1 )

	PORT_START("COIN")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_COIN1 ) PORT_CHANGED_MEMBER(DEVICE_SELF, cheekyms_state,coin_inserted, 0)
INPUT_PORTS_END



static const gfx_layout charlayout =
{
	8,8,
	RGN_FRAC(1,2),
	2,
	{ 0, RGN_FRAC(1,2) },
	{ 0, 1, 2, 3, 4, 5, 6, 7 },
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8 },
	8*8
};

static const gfx_layout spritelayout =
{
	16,16,
	RGN_FRAC(1,2),
	2,
	{ RGN_FRAC(1,2), 0 },
	{ 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15 },
	{ 0*16, 1*16,  2*16,  3*16,  4*16,  5*16,  6*16,  7*16,
			8*16, 9*16, 10*16, 11*16, 12*16, 13*16, 14*16, 15*16 },
	32*8
};



static GFXDECODE_START( gfx_cheekyms )
	GFXDECODE_ENTRY( "gfx1", 0, charlayout,   0x00, 0x20 )
	GFXDECODE_ENTRY( "gfx2", 0, spritelayout, 0x80, 0x10 )
GFXDECODE_END


void cheekyms_state::machine_start()
{
	save_item(NAME(m_irq_mask));
}

INTERRUPT_GEN_MEMBER(cheekyms_state::vblank_irq)
{
	if(m_irq_mask)
		device.execute().set_input_line(0, HOLD_LINE);
}


MACHINE_CONFIG_START(cheekyms_state::cheekyms)

	/* basic machine hardware */
	MCFG_DEVICE_ADD("maincpu", Z80,5000000/2)  /* 2.5 MHz */
	MCFG_DEVICE_PROGRAM_MAP(main_map)
	MCFG_DEVICE_IO_MAP(io_map)
	MCFG_DEVICE_VBLANK_INT_DRIVER("screen", cheekyms_state,  vblank_irq)


	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MCFG_SCREEN_SIZE(32*8, 32*8)
	MCFG_SCREEN_VISIBLE_AREA(0*8, 32*8-1, 4*8, 28*8-1)
	MCFG_SCREEN_UPDATE_DRIVER(cheekyms_state, screen_update)
	MCFG_SCREEN_PALETTE(m_palette)

	GFXDECODE(config, m_gfxdecode, m_palette, gfx_cheekyms);
	PALETTE(config, m_palette, FUNC(cheekyms_state::cheekyms_palette), 0xc0);

	/* audio hardware */
	CHEEKY_MOUSE_AUDIO(config, m_sound_board, 0);
MACHINE_CONFIG_END




/***************************************************************************

  Game driver(s)

***************************************************************************/

ROM_START( cheekyms )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "cm03.c5",       0x0000, 0x0800, CRC(1ad0cb40) SHA1(2a751395ac19a3218c22cfd3597f9a17b8e31527) )
	ROM_LOAD( "cm04.c6",       0x0800, 0x0800, CRC(2238f607) SHA1(35df9eb49f6e3c6351fae220d773442cf8536f90) )
	ROM_LOAD( "cm05.c7",       0x1000, 0x0800, CRC(4169eba8) SHA1(52a059f29c724d087483c7cd733f75d7b8a5b103) )
	ROM_LOAD( "cm06.c8",       0x1800, 0x0800, CRC(7031660c) SHA1(1370702e30897e45ee172609c1d983f8a4fdf157) )

	ROM_REGION( 0x1000, "gfx1", 0 )
	ROM_LOAD( "cm01.c1",       0x0000, 0x0800, CRC(26f73bd7) SHA1(fa4db5df5be1a5f4531cba86a83f89b7eb7fa3ec) )
	ROM_LOAD( "cm02.c2",       0x0800, 0x0800, CRC(885887c3) SHA1(62ce8e39d27c0cfea9ebd51757ad31b0baf6b3cd) )

	ROM_REGION( 0x1000, "gfx2", 0 )
	ROM_LOAD( "cm07.n5",       0x0000, 0x0800, CRC(2738c88d) SHA1(3ccd6c1d49bfe2c1b141854ec705e692252e8af8) )
	ROM_LOAD( "cm08.n6",       0x0800, 0x0800, CRC(b3fbd4ac) SHA1(9f45cc6d9e0bf580149e18de5c3e37d4de347b92) )

	ROM_REGION( 0x0060, "proms", 0 )
	ROM_LOAD( "cm.m9",         0x0000, 0x0020, CRC(db9c59a5) SHA1(357ed5ac8e954a4c8b4d78d36e57bf2de36c1d57) )    /* Character colors /                                */
	ROM_LOAD( "cm.m8",         0x0020, 0x0020, CRC(2386bc68) SHA1(6676082860cd8678a71339a352d2c6286e78ba44) )    /* Character colors \ Selected by Bit 6 of Port 0x80 */
	ROM_LOAD( "cm.p3",         0x0040, 0x0020, CRC(6ac41516) SHA1(05bf40790a0de1e859362df892f7f158c183e247) )  /* Sprite colors */
ROM_END



GAME( 1980, cheekyms, 0, cheekyms, cheekyms, cheekyms_state, empty_init, ROT270, "Universal", "Cheeky Mouse", MACHINE_IMPERFECT_SOUND | MACHINE_SUPPORTS_SAVE )
