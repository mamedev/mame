// license:BSD-3-Clause
// copyright-holders:David Haywood
/***************************************************************************

    Omori Battle Cross

    driver by David Haywood

    Stephh's notes :

    - I don't know exactly how to call the "Free Play" Dip Switch 8(
      Its effect is the following :
        * you need to insert at least one credit and start a game
        * when the game is over, you can start another games WITHOUT
          inserting another coins
      Note that the number of credits is decremented though.
      Credits are BCD coded on 3 bytes (0x000000-0x999999) at addresses
      0xa039 (LSB), 0xa03a and 0xa03b (MSB), but only the LSB is displayed.

     - Setting the flipscreen dip to ON also hides the copyright message (?)

    TO DO :

    - missing starfield

    - game speed, its seems to be controlled by the IRQ's, how fast should it
      be? firing seems frustratingly inconsistant (better with PORT_IMPULSE)

    - background tile colors, not understood well

****************************************************************************

    Battle Cross (c)1982 Omori

    CPU: Z80A
    Sound: AY-3-8910
    Other: 93419 (in socket marked 93219)

    RAM: 4116(x12), 2114(x2), 2114(x6)
    PROMS: none

    XTAL: 10.0 MHz

***************************************************************************/


#include "emu.h"
#include "cpu/z80/z80.h"
#include "sound/ay8910.h"
#include "includes/battlex.h"


INTERRUPT_GEN_MEMBER(battlex_state::battlex_interrupt)
{
	m_in0_b4 = 1;
	device.execute().set_input_line(0, ASSERT_LINE);
}

CUSTOM_INPUT_MEMBER(battlex_state::battlex_in0_b4_r)
{
	UINT32 ret = m_in0_b4;
	if (m_in0_b4)
	{
		m_maincpu->set_input_line(0, CLEAR_LINE);
		m_in0_b4 = 0;
	}

	return ret;
}


/*************************************
 *
 *  Memory maps
 *
 *************************************/

static ADDRESS_MAP_START( battlex_map, AS_PROGRAM, 8, battlex_state )
	AM_RANGE(0x0000, 0x5fff) AM_ROM
	AM_RANGE(0x8000, 0x8fff) AM_RAM_WRITE(battlex_videoram_w) AM_SHARE("videoram")
	AM_RANGE(0x9000, 0x91ff) AM_RAM AM_SHARE("spriteram")
	AM_RANGE(0xa000, 0xa3ff) AM_RAM
	AM_RANGE(0xe000, 0xe03f) AM_RAM_WRITE(battlex_palette_w)
ADDRESS_MAP_END


static ADDRESS_MAP_START( io_map, AS_IO, 8, battlex_state )
	ADDRESS_MAP_GLOBAL_MASK(0xff)
	AM_RANGE(0x00, 0x00) AM_READ_PORT("DSW1")
	AM_RANGE(0x01, 0x01) AM_READ_PORT("SYSTEM")
	AM_RANGE(0x02, 0x02) AM_READ_PORT("INPUTS")
	AM_RANGE(0x03, 0x03) AM_READ_PORT("DSW2")
	AM_RANGE(0x10, 0x10) AM_WRITE(battlex_flipscreen_w)

	/* verify all of these */
	AM_RANGE(0x22, 0x23) AM_DEVWRITE("aysnd", ay8910_device, data_address_w)
	AM_RANGE(0x30, 0x30) AM_WRITE(battlex_scroll_starfield_w)
	AM_RANGE(0x32, 0x32) AM_WRITE(battlex_scroll_x_lsb_w)
	AM_RANGE(0x33, 0x33) AM_WRITE(battlex_scroll_x_msb_w)
ADDRESS_MAP_END


/*************************************
 *
 *  Input ports
 *
 *************************************/

static INPUT_PORTS_START( battlex )
	PORT_START("DSW1")      /* IN0 */
	PORT_DIPNAME( 0x03, 0x00, DEF_STR( Coin_A ) )
	PORT_DIPSETTING(    0x02, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x03, DEF_STR( 2C_3C ) )
	PORT_DIPSETTING(    0x01, DEF_STR( 1C_2C ) )
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Allow_Continue ) )   // Not on 1st stage
	PORT_DIPSETTING(    0x00, DEF_STR( No ) )
	PORT_DIPSETTING(    0x04, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Demo_Sounds ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x08, DEF_STR( On ) )
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_SPECIAL ) PORT_CUSTOM_MEMBER(DEVICE_SELF, battlex_state,battlex_in0_b4_r, NULL)
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Cabinet ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Upright ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Cocktail ) )
	PORT_DIPNAME( 0x40, 0x00, DEF_STR( Flip_Screen ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x40, DEF_STR( On ) )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("SYSTEM")    /* IN1 */
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_COIN1 )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_COIN2 )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_IMPULSE(4)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_IMPULSE(4) PORT_COCKTAIL
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_START1 )
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_START2 )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNKNOWN )

	PORT_START("INPUTS")    /* IN2 */
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP ) PORT_8WAY
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_8WAY
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN ) PORT_8WAY
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_8WAY
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_COCKTAIL

	PORT_START("DSW2")      /* IN3 */
	PORT_DIPNAME( 0x07, 0x00, DEF_STR( Coin_B ) )
	PORT_DIPSETTING(    0x07, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x05, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x06, DEF_STR( 2C_3C ) )
	PORT_DIPSETTING(    0x01, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x02, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0x03, DEF_STR( 1C_6C ) )
	PORT_DIPSETTING(    0x04, DEF_STR( 1C_7C ) )
	PORT_DIPNAME( 0x18, 0x10, DEF_STR( Lives ) )
	PORT_DIPSETTING(    0x00, "1" )
	PORT_DIPSETTING(    0x08, "2" )
	PORT_DIPSETTING(    0x10, "3" )
	PORT_DIPSETTING(    0x18, "5" )
	PORT_DIPNAME( 0x60, 0x00, DEF_STR( Bonus_Life ) )
	PORT_DIPSETTING(    0x00, "5000" )
	PORT_DIPSETTING(    0x20, "10000" )
	PORT_DIPSETTING(    0x40, "15000" )
	PORT_DIPSETTING(    0x60, "20000" )
	PORT_DIPNAME( 0x80, 0x00, DEF_STR( Free_Play ) )        // See notes
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x80, DEF_STR( On ) )
INPUT_PORTS_END


/*************************************
 *
 *  Graphics definitions
 *
 *************************************/

static const gfx_layout battlex_charlayout =
{
	8,8,
	RGN_FRAC(1,3),
	3,
	{ 0,RGN_FRAC(1,3),RGN_FRAC(2,3) },
	{ 7,6,5,4,3,2,1,0 },
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8 },
	8*8
};

static const gfx_layout battlex_spritelayout =
{
	16,16,
	RGN_FRAC(1,3),
	3,
	{ 0,RGN_FRAC(1,3),RGN_FRAC(2,3) },
	{ 7,6,5,4,3,2,1,0,
		15,14,13,12,11,10,9,8 },
	{ 0*16, 1*16, 2*16, 3*16, 4*16, 5*16, 6*16, 7*16,
		8*16, 9*16, 10*16, 11*16, 12*16, 13*16, 14*16, 15*16 },
	16*16
};

static GFXDECODE_START( battlex )
	GFXDECODE_ENTRY( "gfx1", 0, battlex_charlayout,   0, 8 )
	GFXDECODE_ENTRY( "gfx2", 0, battlex_spritelayout, 0, 8 )
GFXDECODE_END


/*************************************
 *
 *  Machine driver
 *
 *************************************/

void battlex_state::machine_start()
{
	/* register for save states */
	save_item(NAME(m_scroll_lsb));
	save_item(NAME(m_scroll_msb));
	save_item(NAME(m_starfield_enabled));
	save_item(NAME(m_in0_b4));
}

void battlex_state::machine_reset()
{
	m_scroll_lsb = 0;
	m_scroll_msb = 0;
	m_starfield_enabled = 0;
	m_in0_b4 = 0;
}

static MACHINE_CONFIG_START( battlex, battlex_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", Z80,XTAL_10MHz/4 )      // ?
	MCFG_CPU_PROGRAM_MAP(battlex_map)
	MCFG_CPU_IO_MAP(io_map)
	MCFG_CPU_PERIODIC_INT_DRIVER(battlex_state, battlex_interrupt, 400) /* controls game speed? */


	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MCFG_SCREEN_SIZE(32*8, 32*8)
	MCFG_SCREEN_VISIBLE_AREA(0*8, 32*8-1, 2*8, 30*8-1)
	MCFG_SCREEN_UPDATE_DRIVER(battlex_state, screen_update_battlex)
	MCFG_SCREEN_PALETTE("palette")
	MCFG_SCREEN_ORIENTATION(ROT180)

	MCFG_GFXDECODE_ADD("gfxdecode", "palette", battlex)
	MCFG_PALETTE_ADD("palette", 64)

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")
	MCFG_SOUND_ADD("aysnd", AY8910, XTAL_10MHz/8)   // ?
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.40)
MACHINE_CONFIG_END


/*************************************
 *
 *  ROM definition
 *
 *************************************/

ROM_START( battlex )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "p-rom1.6",    0x0000, 0x1000, CRC(b00ae551) SHA1(32a963fea23ea58fc3aab93cc814784a932f045e) )
	ROM_LOAD( "p-rom2.5",    0x1000, 0x1000, CRC(e765bb11) SHA1(99671e63f4c7d3d8754277451f0b35cba03b532d) )
	ROM_LOAD( "p-rom3.4",    0x2000, 0x1000, CRC(21675a91) SHA1(5bbd5b53b1a1b7aaed5d8c7b09b57f35e4a774dc) )
	ROM_LOAD( "p-rom4.3",    0x3000, 0x1000, CRC(fff1ccc4) SHA1(2cb9b096b30e441559e57992df8f30aee46b1f1c) )
	ROM_LOAD( "p-rom5.2",    0x4000, 0x1000, CRC(ceb63d38) SHA1(92cab905d009c59115f52172ba7d01c8ff8991d7) )
	ROM_LOAD( "p-rom6.1",    0x5000, 0x1000, CRC(6923f601) SHA1(e6c33cbd8d8679299d7b2c568d56f96ed3073971) )

	ROM_REGION( 0x3000, "gfx2", 0 )
	ROM_LOAD( "1a_f.6f",     0x0000, 0x1000, CRC(2b69287a) SHA1(30c0edaec44118b95ec390bd41c1bd49a2802451) )
	ROM_LOAD( "1a_h.6h",     0x1000, 0x1000, CRC(9f4c3bdd) SHA1(e921ecafefe54c033d05d9cd289808e971ac7940) )
	ROM_LOAD( "1a_j.6j",     0x2000, 0x1000, CRC(c1345b05) SHA1(17194c8ec961990222bd295ff1d036a64f497b0e) )

	ROM_REGION( 0x3000, "gfx1", ROMREGION_ERASE00 ) // filled in later

	ROM_REGION( 0x1000, "user2", 0 )                // gfx1 1bpp gfxdata
	ROM_LOAD( "1a_e.6e",     0x0000, 0x1000, CRC(126842b7) SHA1(2da4f64e077232c1dd0853d07d801f9781517850) )

	ROM_REGION( 0x1000, "user1", 0 )                // gfx1 colormask, bad?
	ROM_LOAD( "1a_d.6d",     0x0000, 0x1000, CRC(750a46ef) SHA1(b6ab93e084ab0b7c6ad90ee6431bc1b7ab9ed46d) )
ROM_END


/*************************************
 *
 *  Driver initialization
 *
 *************************************/

DRIVER_INIT_MEMBER(battlex_state,battlex)
{
	UINT8 *colormask = memregion("user1")->base();
	UINT8 *gfxdata = memregion("user2")->base();
	UINT8 *dest = memregion("gfx1")->base();

	int tile, line, bit;

	/* convert gfx data from 1bpp + color block mask to straight 3bpp */
	for (tile = 0; tile < 512; tile++)
	{
		for (line = 0; line < 8; line ++)
		{
			for (bit = 0; bit < 8 ; bit ++)
			{
				int plane;
				int color = colormask[tile << 3 | line];
				int data = gfxdata[tile << 3 | line] >> bit & 1;
				if (!data) color >>= 4;

				for (plane = 2; plane >= 0; plane--)
				{
					dest[tile << 3 | line | (plane << 12)] |= (color & 1) << bit;
					color >>= 1;
				}
			}
		}
	}
}


/*************************************
 *
 *  Game driver
 *
 *************************************/

GAME( 1982, battlex, 0, battlex, battlex, battlex_state, battlex, ROT180, "Omori Electric Co., Ltd.", "Battle Cross", MACHINE_IMPERFECT_GRAPHICS | MACHINE_IMPERFECT_COLORS | MACHINE_SUPPORTS_SAVE )
