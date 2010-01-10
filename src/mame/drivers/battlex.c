/***************************************************************************

    Omori Battle Cross

    driver by David Haywood

    Stephh's notes :

    - I don't know exactly how to call the "Free Play" Dip Switch 8(
      It's effect is the following :
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
      be? firing seems frustratingly inconsistant

    - colors match Tim's screen shots, but there's no guarantee RGB are in the
      correct order.

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
#include "deprecat.h"
#include "sound/ay8910.h"
#include "includes/battlex.h"


/*************************************
 *
 *  Memory maps
 *
 *************************************/

static ADDRESS_MAP_START( battlex_map, ADDRESS_SPACE_PROGRAM, 8 )
	AM_RANGE(0x0000, 0x5fff) AM_ROM
	AM_RANGE(0x8000, 0x8fff) AM_RAM_WRITE(battlex_videoram_w) AM_BASE_MEMBER(battlex_state, videoram)
	AM_RANGE(0x9000, 0x91ff) AM_RAM AM_BASE_MEMBER(battlex_state, spriteram)
	AM_RANGE(0xa000, 0xa3ff) AM_RAM
	AM_RANGE(0xe000, 0xe03f) AM_RAM_WRITE(battlex_palette_w)
ADDRESS_MAP_END


static ADDRESS_MAP_START( io_map, ADDRESS_SPACE_IO, 8 )
	ADDRESS_MAP_GLOBAL_MASK(0xff)
	AM_RANGE(0x00, 0x00) AM_READ_PORT("DSW1")
	AM_RANGE(0x01, 0x01) AM_READ_PORT("SYSTEM")
	AM_RANGE(0x02, 0x02) AM_READ_PORT("INPUTS")
	AM_RANGE(0x03, 0x03) AM_READ_PORT("DSW2")
	AM_RANGE(0x10, 0x10) AM_WRITE(battlex_flipscreen_w)

	/* verify all of these */
	AM_RANGE(0x22, 0x23) AM_DEVWRITE("aysnd", ay8910_data_address_w)

	/* 0x30 looks like scroll, but can't be ? changes (increases or decreases)
        depending on the direction your ship is facing on lev 2. at least */
	AM_RANGE(0x30, 0x30) AM_WRITENOP

	AM_RANGE(0x32, 0x32) AM_WRITE(battlex_scroll_x_lsb_w)
	AM_RANGE(0x33, 0x33) AM_WRITE(battlex_scroll_x_msb_w)
ADDRESS_MAP_END

/*************************************
 *
 *  Input ports
 *
 *************************************/

static INPUT_PORTS_START( battlex )
	PORT_START("DSW1")	/* IN0 */
	PORT_DIPNAME( 0x03, 0x00, DEF_STR( Coin_A ) )
	PORT_DIPSETTING(    0x02, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x03, DEF_STR( 2C_3C ) )
	PORT_DIPSETTING(    0x01, DEF_STR( 1C_2C ) )
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Allow_Continue ) )			// Not on 1st stage
	PORT_DIPSETTING(    0x00, DEF_STR( No ) )
	PORT_DIPSETTING(    0x04, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Demo_Sounds ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x08, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x00, "Freeze" )				// VBLANK ?
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x10, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Cabinet ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Upright ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Cocktail ) )
	PORT_DIPNAME( 0x40, 0x00, DEF_STR( Flip_Screen ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x40, DEF_STR( On ) )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("SYSTEM")	/* IN1 */
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_COIN1 )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_COIN2 )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_BUTTON1 )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_COCKTAIL
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_START1 )
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_START2 )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNKNOWN )

	PORT_START("INPUTS")	/* IN2 */
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP ) PORT_8WAY
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_8WAY
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN ) PORT_8WAY
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_8WAY
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_COCKTAIL

	PORT_START("DSW2")	/* IN3 */
	PORT_DIPNAME( 0x07, 0x00, DEF_STR( Coin_A ) )
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
	PORT_DIPNAME( 0x80, 0x00, DEF_STR( Free_Play ) )		// See notes
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
	RGN_FRAC(1,1),
	4,
	{ 0,1,2,3 },
	{ 0*4, 1*4, 2*4, 3*4, 4*4, 5*4, 6*4, 7*4 },
	{ 0*8*4, 1*8*4, 2*8*4, 3*8*4, 4*8*4, 5*8*4, 6*8*4, 7*8*4 },
	8*8*4
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
	GFXDECODE_ENTRY( "gfx1", 0, battlex_charlayout,      0, 8 )
	GFXDECODE_ENTRY( "gfx2", 0, battlex_spritelayout, 16*8, 8 )
GFXDECODE_END


/*************************************
 *
 *  Machine driver
 *
 *************************************/

static MACHINE_RESET( battlex )
{
	battlex_state *state = (battlex_state *)machine->driver_data;

	state->scroll_lsb = 0;
	state->scroll_msb = 0;
}

static MACHINE_DRIVER_START( battlex )

	/* driver data */
	MDRV_DRIVER_DATA(battlex_state)

	/* basic machine hardware */
	MDRV_CPU_ADD("maincpu", Z80,10000000/2 )		 /* 10 MHz, divided ? (Z80A CPU) */
	MDRV_CPU_PROGRAM_MAP(battlex_map)
	MDRV_CPU_IO_MAP(io_map)
	MDRV_CPU_VBLANK_INT_HACK(irq0_line_hold,8) /* controls game speed? */

	MDRV_MACHINE_RESET(battlex)

	/* video hardware */
	MDRV_SCREEN_ADD("screen", RASTER)
	MDRV_SCREEN_REFRESH_RATE(56) /* The video syncs at 15.8k H and 56 V (www.klov.com) */
	MDRV_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MDRV_SCREEN_FORMAT(BITMAP_FORMAT_INDEXED16)
	MDRV_SCREEN_SIZE(32*8, 32*8)
	MDRV_SCREEN_VISIBLE_AREA(0*8, 32*8-1, 2*8, 30*8-1)
	MDRV_GFXDECODE(battlex)
	MDRV_PALETTE_LENGTH(16*8+64)

	MDRV_PALETTE_INIT(battlex)
	MDRV_VIDEO_START(battlex)
	MDRV_VIDEO_UPDATE(battlex)

	/* sound hardware */
	MDRV_SPEAKER_STANDARD_MONO("mono")
	MDRV_SOUND_ADD("aysnd", AY8910, 10000000/8)
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.40)
MACHINE_DRIVER_END


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

	ROM_REGION( 0x4000, "gfx1", ROMREGION_ERASE00 )
	/* filled in later */
//  ROM_LOAD( "2732.e",    0x0000, 0x1000, CRC(126842b7) SHA1(2da4f64e077232c1dd0853d07d801f9781517850) )

	ROM_REGION( 0x3000, "gfx2", 0 )
	ROM_LOAD( "2732.f",    0x0000, 0x1000, CRC(2b69287a) SHA1(30c0edaec44118b95ec390bd41c1bd49a2802451) )
	ROM_LOAD( "2732.h",    0x1000, 0x1000, CRC(9f4c3bdd) SHA1(e921ecafefe54c033d05d9cd289808e971ac7940) )
	ROM_LOAD( "2732.j",    0x2000, 0x1000, CRC(c1345b05) SHA1(17194c8ec961990222bd295ff1d036a64f497b0e) )

	ROM_REGION( 0x1000, "user1", 0 ) /* line colours? or bad? */
	ROM_LOAD( "2732.d",    0x0000, 0x1000, CRC(750a46ef) SHA1(b6ab93e084ab0b7c6ad90ee6431bc1b7ab9ed46d) )

	ROM_REGION( 0x1000, "user2", 0 )
	ROM_LOAD( "2732.e",    0x0000, 0x1000, CRC(126842b7) SHA1(2da4f64e077232c1dd0853d07d801f9781517850) )
ROM_END


/*************************************
 *
 *  Driver initialization
 *
 *************************************/

static DRIVER_INIT( battlex )
{
	UINT8 *cold = memory_region(machine, "user1");
	UINT8 *mskd = memory_region(machine, "user2");
	UINT8 *dest = memory_region(machine, "gfx1");

	int outcount;

	/* convert gfx data from 1bpp + color block mask to straight 4bpp */
	for (outcount = 0; outcount < (0x1000/8); outcount++)
	{
		int linecount;
		for (linecount = 0; linecount < 8; linecount ++)
		{
			int bitmask = 0x01;
			int bitcount;

			for (bitcount = 0; bitcount < 8 ; bitcount ++)
			{
				int bit, col;
				bit = (mskd[outcount * 8 + linecount] & bitmask) >> bitcount;

				if (bit)
					col = (cold[outcount * 8 + (linecount & ~1) + (bitcount / 4)] & 0x0f) << 4;
				else
					col = (cold[outcount * 8 + (linecount & ~1) + (bitcount / 4)] & 0xf0);

				dest[outcount * 32 + linecount * 4 + bitcount /2] |= (col >> (4 * (bitcount & 1)));
				bitmask = bitmask << 1;
			}
		}
	}
}


/*************************************
 *
 *  Game driver
 *
 *************************************/

GAME( 1982, battlex, 0, battlex, battlex, battlex, ROT180, "Omori Electric", "Battle Cross", GAME_IMPERFECT_GRAPHICS | GAME_SUPPORTS_SAVE )
