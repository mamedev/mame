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

    Notes from Tomasz Slanina:

    Tile decoding:

    Each 8x8 BG tile is defined by:
    - 1 bit  8x8 mask  (one tile - 8 consecutive bytes - user2 region)
    - 4+4  bits of color ( one tile - 8 consecutive bytes - user1 region)
    - bit 3 of color  = brightness ?

    Single mask byte defines one row of tile pixels (FG or BG)
    Single color byte defines color of FG (4 bits) and color of BG (4 bits)
    of high (odd address in user1) or low (even address in user1)
    nibbles of two tile pixels rows.

    Here's an example (single tile):

     user2      user1   colors
    ----------------------------
    00011100    0x32   33321144
    00111100    0x41   33221144

    00111100    0x32   33227744
    00011000    0x47   33327444

    00011000    0x56   55566555
    00011000    0x56   55566555

    00011000    0x84   88844777
    00011000    0x74   88844777





    TO DO :

    - missing starfield

    - game speed, its seems to be controlled by the IRQ's, how fast should it
      be? firing seems frustratingly inconsistant (better with PORT_IMPULSE)

    - BG tilemap palette bits (in most cases paltte 0 is used,
      only highlights ( battlex logo, hiscore table) uses different palettes(?).
      Current implementation gives different highlight colors than on real
      hardware (i.e. battlex logo should have yellow highights)

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
#include "includes/battlex.h"

#include "cpu/z80/z80.h"
#include "sound/ay8910.h"
#include "screen.h"
#include "speaker.h"


INTERRUPT_GEN_MEMBER(battlex_state::battlex_interrupt)
{
	m_in0_b4 = 1;
	device.execute().set_input_line(0, ASSERT_LINE);
}

CUSTOM_INPUT_MEMBER(battlex_state::battlex_in0_b4_r)
{
	uint32_t ret = m_in0_b4;
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

void battlex_state::battlex_map(address_map &map)
{
	map(0x0000, 0x5fff).rom();
	map(0x8000, 0x8fff).ram().w(FUNC(battlex_state::battlex_videoram_w)).share("videoram");
	map(0x9000, 0x91ff).ram().share("spriteram");
	map(0xa000, 0xa3ff).ram();
	map(0xe000, 0xe03f).ram().w(FUNC(battlex_state::battlex_palette_w));
}


void battlex_state::io_map(address_map &map)
{
	map.global_mask(0xff);
	map(0x00, 0x00).portr("DSW1");
	map(0x01, 0x01).portr("SYSTEM");
	map(0x02, 0x02).portr("INPUTS");
	map(0x03, 0x03).portr("DSW2");
	map(0x10, 0x10).w(FUNC(battlex_state::battlex_flipscreen_w));

	/* verify all of these */
	map(0x22, 0x23).w("ay1", FUNC(ay8910_device::data_address_w));
	map(0x30, 0x30).w(FUNC(battlex_state::battlex_scroll_starfield_w));
	map(0x32, 0x32).w(FUNC(battlex_state::battlex_scroll_x_lsb_w));
	map(0x33, 0x33).w(FUNC(battlex_state::battlex_scroll_x_msb_w));
}

void battlex_state::dodgeman_io_map(address_map &map)
{
	io_map(map);
	map(0x26, 0x27).w("ay2", FUNC(ay8910_device::data_address_w));
}

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
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_CUSTOM_MEMBER(DEVICE_SELF, battlex_state,battlex_in0_b4_r, nullptr)
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
	// TODO: PORT_IMPULSE?
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

static INPUT_PORTS_START( dodgeman )
	PORT_INCLUDE( battlex )

	PORT_MODIFY("SYSTEM")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_BUTTON1 )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_COCKTAIL
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_BUTTON2 )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_COCKTAIL
INPUT_PORTS_END


/*************************************
 *
 *  Graphics definitions
 *
 *************************************/

static const gfx_layout battlex_charlayout =
{
	8,8,
	RGN_FRAC(1,1),
	4,
	{ 0,1,2,3 },
	{ 0, 4, 8, 12, 16, 20, 24, 28  },
	{ 0*32, 1*32, 2*32, 3*32, 4*32, 5*32, 6*32, 7*32 },
	8*8*4
};

static const gfx_layout battlex_spritelayout =
{
	16,16,
	RGN_FRAC(1,3),
	3,
	{ RGN_FRAC(0,3), RGN_FRAC(1,3), RGN_FRAC(2,3) },
	{ 7,6,5,4,3,2,1,0,
		15,14,13,12,11,10,9,8 },
	{ 0*16, 1*16, 2*16, 3*16, 4*16, 5*16, 6*16, 7*16,
		8*16, 9*16, 10*16, 11*16, 12*16, 13*16, 14*16, 15*16 },
	16*16
};

static GFXDECODE_START( gfx_battlex )
	GFXDECODE_ENTRY( "gfx1", 0, battlex_charlayout,   64, 8 )
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

void battlex_state::battlex(machine_config &config)
{
	/* basic machine hardware */
	Z80(config, m_maincpu, XTAL(10'000'000)/4 );      // ?
	m_maincpu->set_addrmap(AS_PROGRAM, &battlex_state::battlex_map);
	m_maincpu->set_addrmap(AS_IO, &battlex_state::io_map);
	m_maincpu->set_periodic_int(FUNC(battlex_state::battlex_interrupt), attotime::from_hz(400)); /* controls game speed? */


	/* video hardware */
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_refresh_hz(60);
	screen.set_vblank_time(ATTOSECONDS_IN_USEC(0));
	screen.set_size(32*8, 32*8);
	screen.set_visarea(0*8, 32*8-1, 2*8, 30*8-1);
	screen.set_screen_update(FUNC(battlex_state::screen_update_battlex));
	screen.set_palette(m_palette);

	GFXDECODE(config, m_gfxdecode, m_palette, gfx_battlex);
	PALETTE(config, m_palette).set_entries(64 + 128);

	/* sound hardware */
	SPEAKER(config, "mono").front_center();
	AY8910(config, "ay1", XTAL(10'000'000)/8).add_route(ALL_OUTPUTS, "mono", 0.40);   // ?
}

void battlex_state::dodgeman(machine_config &config)
{
	battlex(config);

	m_maincpu->set_addrmap(AS_IO, &battlex_state::dodgeman_io_map);

	MCFG_VIDEO_START_OVERRIDE(battlex_state, dodgeman)

	AY8910(config, "ay2", XTAL(10'000'000)/8).add_route(ALL_OUTPUTS, "mono", 0.40);   // ?
}


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

	ROM_REGION( 0x4000, "gfx1", ROMREGION_ERASE00 ) // filled in later

	ROM_REGION( 0x3000, "gfx2", 0 )
	ROM_LOAD( "1a_f.6f",     0x0000, 0x1000, CRC(2b69287a) SHA1(30c0edaec44118b95ec390bd41c1bd49a2802451) )
	ROM_LOAD( "1a_h.6h",     0x1000, 0x1000, CRC(9f4c3bdd) SHA1(e921ecafefe54c033d05d9cd289808e971ac7940) )
	ROM_LOAD( "1a_j.6j",     0x2000, 0x1000, CRC(c1345b05) SHA1(17194c8ec961990222bd295ff1d036a64f497b0e) )

	ROM_REGION( 0x1000, "user1", 0 )                // gfx1 colormask, bad?
	ROM_LOAD( "1a_d.6d",     0x0000, 0x1000, CRC(750a46ef) SHA1(b6ab93e084ab0b7c6ad90ee6431bc1b7ab9ed46d) )

	ROM_REGION( 0x1000, "user2", 0 )                // gfx1 1bpp gfxdata
	ROM_LOAD( "1a_e.6e",     0x0000, 0x1000, CRC(126842b7) SHA1(2da4f64e077232c1dd0853d07d801f9781517850) )
ROM_END

ROM_START( dodgeman )
	ROM_REGION( 0x10000, "maincpu", ROMREGION_ERASE00 )
	ROM_LOAD( "dg0.7f",       0x0000, 0x001000, CRC(1219b5db) SHA1(e4050a5e52f7b125f317b6e2ef615774c81cf679) )
	ROM_LOAD( "dg1.5f",       0x1000, 0x001000, CRC(fff9a086) SHA1(e4e528789d07755cf999054c191e242ea3ebe55f) )
	ROM_LOAD( "dg2.4f",       0x2000, 0x001000, CRC(2ca9ac99) SHA1(7fe81626ab2b5c01189fbb578999157863fcc29f) )
	ROM_LOAD( "dg3.3f",       0x3000, 0x001000, CRC(55a51c0e) SHA1(a5b253f096e1fe85ee391ad2aa0373809a3b48c2) )
	ROM_LOAD( "dg4.2f",       0x4000, 0x001000, CRC(14169361) SHA1(86d3cd1fa0aa4f21029daea2eba99bdaa34372e8) )
	ROM_LOAD( "dg5.1f",       0x5000, 0x001000, CRC(8f83ae2f) SHA1(daad41b61ba3d55531021d444bbe4acfc275cfc9) )

	ROM_REGION( 0x8000, "gfx1", ROMREGION_ERASE00 ) // filled in later

	ROM_REGION( 0x6000, "gfx2", ROMREGION_ERASE00 )
	ROM_LOAD( "f.6f",         0x0000, 0x002000, CRC(dfaaf4c8) SHA1(1e09f1d72e7e5e6782d73ae60bca7982fc04df0e) )
	ROM_LOAD( "h.6h",         0x2000, 0x002000, CRC(e2525ffe) SHA1(a17b608b4089014be381b26f16597b83d4a66ebd) )
	ROM_LOAD( "j.6j",         0x4000, 0x002000, CRC(2731ee46) SHA1(15b9350e19f31b1cea99deb9935543777644e6a8) )

	ROM_REGION( 0x2000, "user1", 0 )                // gfx1 1bpp gfxdata
	ROM_LOAD( "d.6d",         0x0000, 0x002000, CRC(451c1c3a) SHA1(214f775e242f7f29ac799cdd554708acddd1e34f) )

	ROM_REGION( 0x2000, "user2", 0 )                // gfx1 colormask, bad?
	ROM_LOAD( "e.6e",         0x0000, 0x002000, CRC(c9a515df) SHA1(5232d2d1bd02b89cb651d817995daf33469f0e2f) )
ROM_END


/*************************************
 *
 *  Driver initialization
 *
 *************************************/

void battlex_state::init_battlex()
{
	uint8_t *colormask = memregion("user1")->base();
	uint8_t *gfxdata = memregion("user2")->base();
	uint8_t *dest = memregion("gfx1")->base();
	int tile_size = memregion("gfx1")->bytes() / 32;

	int offset = 0;
	for (int tile = 0; tile < tile_size; tile++)
	{
		for (int line = 0; line < 8; line ++)
		{
			for (int bit = 0; bit < 8 ; bit ++)
			{
				int color = colormask[(tile << 3) | ((line & 0x6) + (bit > 3 ? 1 : 0))];
				int data = BIT(gfxdata[(tile << 3) | line], bit);

				if (!data)
					color >>= 4;

				color &= 0x0f;

				if (offset&1)
					dest[offset >> 1] |= color;
				else
					dest[offset >> 1] = color<<4;
				++offset;
			}
		}
	}
}

/*************************************
 *
 *  Game driver
 *
 *************************************/

GAME( 1982, battlex,  0, battlex,  battlex,  battlex_state, init_battlex, ROT180, "Omori Electric Co., Ltd.", "Battle Cross", MACHINE_IMPERFECT_GRAPHICS | MACHINE_IMPERFECT_COLORS | MACHINE_SUPPORTS_SAVE | MACHINE_NO_COCKTAIL )
GAME( 1983, dodgeman, 0, dodgeman, dodgeman, battlex_state, init_battlex, ROT180, "Omori Electric Co., Ltd.", "Dodge Man",    MACHINE_IMPERFECT_COLORS | MACHINE_SUPPORTS_SAVE | MACHINE_NO_COCKTAIL )
