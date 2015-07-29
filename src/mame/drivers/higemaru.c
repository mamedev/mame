// license:BSD-3-Clause
// copyright-holders:Mirko Buffoni
/****************************************************************************

Higemaru

driver by Mirko Buffoni


Press Player 1 Button 1 during boot sequence to enter the "test mode".
Use Player 1 joystick and button, then press START1 to go to next screen.

****************************************************************************/

#include "emu.h"
#include "cpu/z80/z80.h"
#include "sound/ay8910.h"
#include "includes/higemaru.h"


TIMER_DEVICE_CALLBACK_MEMBER(higemaru_state::higemaru_scanline)
{
	int scanline = param;

	if(scanline == 240) // vblank-out irq
		m_maincpu->set_input_line_and_vector(0, HOLD_LINE, 0xcf);   /* RST 08h - vblank */

	if(scanline == 0) // unknown irq event, does various stuff like copying the spriteram
		m_maincpu->set_input_line_and_vector(0, HOLD_LINE, 0xd7);   /* RST 10h */
}


static ADDRESS_MAP_START( higemaru_map, AS_PROGRAM, 8, higemaru_state )
	AM_RANGE(0x0000, 0x7fff) AM_ROM
	AM_RANGE(0xc000, 0xc000) AM_READ_PORT("P1")
	AM_RANGE(0xc001, 0xc001) AM_READ_PORT("P2")
	AM_RANGE(0xc002, 0xc002) AM_READ_PORT("SYSTEM")
	AM_RANGE(0xc003, 0xc003) AM_READ_PORT("DSW1")
	AM_RANGE(0xc004, 0xc004) AM_READ_PORT("DSW2")
	AM_RANGE(0xc800, 0xc800) AM_WRITE(higemaru_c800_w)
	AM_RANGE(0xc801, 0xc802) AM_DEVWRITE("ay1", ay8910_device, address_data_w)
	AM_RANGE(0xc803, 0xc804) AM_DEVWRITE("ay2", ay8910_device, address_data_w)
	AM_RANGE(0xd000, 0xd3ff) AM_RAM_WRITE(higemaru_videoram_w) AM_SHARE("videoram")
	AM_RANGE(0xd400, 0xd7ff) AM_RAM_WRITE(higemaru_colorram_w) AM_SHARE("colorram")
	AM_RANGE(0xd880, 0xd9ff) AM_RAM AM_SHARE("spriteram")
	AM_RANGE(0xe000, 0xefff) AM_RAM
ADDRESS_MAP_END


static INPUT_PORTS_START( higemaru )
	PORT_START("P1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_4WAY
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT )  PORT_4WAY
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN )  PORT_4WAY
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_UP )    PORT_4WAY
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("P2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_4WAY PORT_COCKTAIL
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT )  PORT_4WAY PORT_COCKTAIL
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN )  PORT_4WAY PORT_COCKTAIL
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_UP )    PORT_4WAY PORT_COCKTAIL
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("SYSTEM")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_COCKTAIL
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_SPECIAL ) PORT_NAME("Freeze") PORT_CODE(KEYCODE_F1) PORT_TOGGLE   /* code at 0x0252 */
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_BUTTON1 )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_COIN1 )

	PORT_START("DSW1")
	PORT_DIPNAME( 0x07, 0x07, DEF_STR( Coin_A ) )
	PORT_DIPSETTING(    0x01, DEF_STR( 5C_1C ) )
	PORT_DIPSETTING(    0x02, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(    0x03, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x04, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x07, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x06, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x05, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Free_Play ) )
	PORT_DIPNAME( 0x38, 0x38, DEF_STR( Coin_B ) )
	PORT_DIPSETTING(    0x08, DEF_STR( 5C_1C ) )
	PORT_DIPSETTING(    0x10, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(    0x18, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x20, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x38, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x30, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x28, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Free_Play ) )
	PORT_DIPNAME( 0xc0, 0xc0, DEF_STR( Lives ) )
	PORT_DIPSETTING(    0x80, "1" )
	PORT_DIPSETTING(    0x40, "2" )
	PORT_DIPSETTING(    0xc0, "3" )
	PORT_DIPSETTING(    0x00, "5" )

	PORT_START("DSW2")
	PORT_DIPNAME( 0x01, 0x00, DEF_STR( Cabinet ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Upright ) )
	PORT_DIPSETTING(    0x01, DEF_STR( Cocktail ) )
	PORT_DIPNAME( 0x0e, 0x0e, DEF_STR( Bonus_Life ) )       /* table at 0x0148 */
	PORT_DIPSETTING(    0x0e, "10k 50k 50k+" )
	PORT_DIPSETTING(    0x0c, "10k 60k 60k+" )
	PORT_DIPSETTING(    0x0a, "20k 60k 60k+" )
	PORT_DIPSETTING(    0x08, "20k 70k 70k+" )
	PORT_DIPSETTING(    0x06, "30k 70k 70k+" )
	PORT_DIPSETTING(    0x04, "30k 80k 80k+" )
	PORT_DIPSETTING(    0x02, "40k 100k 100k+" )
	PORT_DIPSETTING(    0x00, DEF_STR( None ) )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Demo_Sounds ) )      /* code at 0x6234 */
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x10, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, "Demo Music" )                /* code at 0x6226 - when is it called ? */
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x20, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Flip_Screen ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPUNUSED( 0x80, IP_ACTIVE_LOW )
INPUT_PORTS_END


static const gfx_layout charlayout =
{
	8,8,
	RGN_FRAC(1,1),
	2,
	{ 4, 0 },
	{ 0, 1, 2, 3, 8+0, 8+1, 8+2, 8+3 },
	{ 0*16, 1*16, 2*16, 3*16, 4*16, 5*16, 6*16, 7*16 },
	16*8
};

static const gfx_layout spritelayout =
{
	16,16,
	RGN_FRAC(1,2),
	4,
	{ RGN_FRAC(1,2)+4, RGN_FRAC(1,2)+0, 4, 0 },
	{ 0, 1, 2, 3, 8+0, 8+1, 8+2, 8+3,
			32*8+0, 32*8+1, 32*8+2, 32*8+3, 33*8+0, 33*8+1, 33*8+2, 33*8+3 },
	{ 0*16, 1*16, 2*16, 3*16, 4*16, 5*16, 6*16, 7*16,
			8*16, 9*16, 10*16, 11*16, 12*16, 13*16, 14*16, 15*16 },
	64*8
};

static GFXDECODE_START( higemaru )
	GFXDECODE_ENTRY( "gfx1", 0, charlayout,       0, 32 )
	GFXDECODE_ENTRY( "gfx2", 0, spritelayout,  32*4, 16 )
GFXDECODE_END


static MACHINE_CONFIG_START( higemaru, higemaru_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", Z80, XTAL_12MHz/4)  /* 3 MHz Sharp LH0080A Z80A-CPU-D */
	MCFG_CPU_PROGRAM_MAP(higemaru_map)
	MCFG_TIMER_DRIVER_ADD_SCANLINE("scantimer", higemaru_state, higemaru_scanline, "screen", 0, 1)

	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MCFG_SCREEN_SIZE(32*8, 32*8)
	MCFG_SCREEN_VISIBLE_AREA(0*8, 32*8-1, 2*8, 30*8-1)
	MCFG_SCREEN_UPDATE_DRIVER(higemaru_state, screen_update_higemaru)
	MCFG_SCREEN_PALETTE("palette")

	MCFG_GFXDECODE_ADD("gfxdecode", "palette", higemaru)

	MCFG_PALETTE_ADD("palette", 32*4+16*16)
	MCFG_PALETTE_INDIRECT_ENTRIES(32)
	MCFG_PALETTE_INIT_OWNER(higemaru_state, higemaru)

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")

	MCFG_SOUND_ADD("ay1", AY8910, XTAL_12MHz/8)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.25)

	MCFG_SOUND_ADD("ay2", AY8910, XTAL_12MHz/8)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.25)
MACHINE_CONFIG_END

/***************************************************************************

  Game driver(s)

***************************************************************************/

ROM_START( higemaru )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "hg4.p12", 0x0000, 0x2000, CRC(dc67a7f9) SHA1(701875e2e85efbe84bf66515117861563f3883c0) )
	ROM_LOAD( "hg5.m12", 0x2000, 0x2000, CRC(f65a4b68) SHA1(687d46406de389c8bad6cc052a2516135db93d4a) )
	ROM_LOAD( "hg6.p11", 0x4000, 0x2000, CRC(5f5296aa) SHA1(410ee1df63492e488b3578b9c4cfbfbd2f41c888) )
	ROM_LOAD( "hg7.m11", 0x6000, 0x2000, CRC(dc5d455d) SHA1(7d253d6680d35943792746da11d91d7be57367cc) )

	ROM_REGION( 0x2000, "gfx1", 0 )
	ROM_LOAD( "hg3.m1", 0x0000, 0x2000, CRC(b37b88c8) SHA1(7933270969806154f0774d31fda75a5352cf26ad) )  /* characters */

	ROM_REGION( 0x4000, "gfx2", 0 )
	ROM_LOAD( "hg1.c14", 0x0000, 0x2000, CRC(ef4c2f5d) SHA1(247ce819cdc4ed4ec99c25c9006bac1911354bc8) ) /* tiles */
	ROM_LOAD( "hg2.e14", 0x2000, 0x2000, CRC(9133f804) SHA1(93661c028709a7134537321e52da85e3c0f917ba) )

	ROM_REGION( 0x0420, "proms", 0 )
	ROM_LOAD( "hgb3.l6", 0x0000, 0x0020, CRC(629cebd8) SHA1(c28cd0f341f4f1c7be97f4d8c289860db8ac0857) ) /* palette */
	ROM_LOAD( "hgb5.m4", 0x0020, 0x0100, CRC(dbaa4443) SHA1(cca2f9b187abd735f2309b38570edcd745042b3e) ) /* char lookup table */
	ROM_LOAD( "hgb1.h7", 0x0120, 0x0100, CRC(07c607ce) SHA1(c048602d62f47129152bbc7ccd38627d78a4392f) ) /* sprite lookup table */
	ROM_LOAD( "hgb4.l9", 0x0220, 0x0100, CRC(712ac508) SHA1(5349d722ab6733afdda65f6e0a98322f0d515e86) ) /* interrupt timing (not used) */
	ROM_LOAD( "hgb2.k7", 0x0320, 0x0100, CRC(4921635c) SHA1(aee37d6cdc36acf0f11ff5f93e7b16e4b12f6c39) ) /* video timing? (not used) */
ROM_END


GAME( 1984, higemaru, 0, higemaru, higemaru, driver_device, 0, ROT0, "Capcom", "Pirate Ship Higemaru", MACHINE_SUPPORTS_SAVE )
