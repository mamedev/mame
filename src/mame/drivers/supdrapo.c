/*
 Super Draw Poker (c) Stern 1983

 driver by Pierpaolo Prazzoli


A2-1C   8910
A2-1D   Z80
A1-1E
A1-1H
A3-1J

        A1-4K
        A1-4L
        A1-4N
        A1-4P           A1-9N (6301)
                        A1-9P

 Notes:
 - According to some original screenshots floating around the net, this game uses a weird coloring scheme for
   the char text that dynamically changes the color of an entire column. For now I can only guess about how
   it truly works.
   An original snap of these can be seen at -> http://mamedev.emulab.it/kale/fast/files/A00000211628-007.jpg

 To do:
 - Needs schematics to check if the current implementation of the "global column coloring" is correct.
 - Check unknown read/writes, too many of them.

*/

#include "emu.h"
#include "cpu/z80/z80.h"
#include "sound/ay8910.h"


typedef struct _supdrapo_state supdrapo_state;
struct _supdrapo_state
{
	UINT8 *char_bank;
	UINT8 *col_line;
	UINT8 *videoram;
};

static READ8_HANDLER( sdpoker_rng_r )
{
	return mame_rand(space->machine);
}

static ADDRESS_MAP_START( sdpoker_mem, ADDRESS_SPACE_PROGRAM, 8 )
	AM_RANGE(0x0000, 0x4fff) AM_ROM
	AM_RANGE(0x5000, 0x50ff) AM_RAM AM_SHARE("share1")
	AM_RANGE(0x57ff, 0x57ff) AM_RAM AM_SHARE("share1")
	AM_RANGE(0x5800, 0x58ff) AM_RAM AM_SHARE("share1") AM_BASE_MEMBER(supdrapo_state,col_line)
	AM_RANGE(0x6000, 0x67ff) AM_RAM //work ram
	AM_RANGE(0x6800, 0x6bff) AM_RAM AM_BASE_MEMBER(supdrapo_state,videoram)
	AM_RANGE(0x6c00, 0x6fff) AM_RAM AM_BASE_MEMBER(supdrapo_state,char_bank)
	AM_RANGE(0x7000, 0x7bff) AM_RAM //$7600 seems watchdog
	AM_RANGE(0x7c00, 0x7c00) AM_WRITENOP //?
	AM_RANGE(0x8000, 0x8000) AM_READ_PORT("IN4") AM_WRITENOP
	AM_RANGE(0x8001, 0x8001) AM_READ_PORT("IN0")
	AM_RANGE(0x8002, 0x8002) AM_READ_PORT("IN1")
	AM_RANGE(0x8003, 0x8003) AM_READ_PORT("IN2") AM_WRITENOP
	AM_RANGE(0x8004, 0x8004) AM_READ_PORT("IN3") AM_WRITENOP
	AM_RANGE(0x8005, 0x8005) AM_READ_PORT("IN6")
	AM_RANGE(0x8006, 0x8006) AM_READ_PORT("IN5") //dips?
	AM_RANGE(0x9000, 0x90ff) AM_RAM
	AM_RANGE(0x9400, 0x9400) AM_READ(sdpoker_rng_r)
	AM_RANGE(0x9800, 0x9801) AM_DEVWRITE("aysnd", ay8910_data_address_w)
ADDRESS_MAP_END

static INPUT_PORTS_START( supdrapo )
	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_BUTTON9 ) PORT_NAME("P1 Win") PORT_CODE(KEYCODE_D) PORT_PLAYER(1)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_BUTTON8 ) PORT_NAME("P1 Cancel") PORT_CODE(KEYCODE_S) PORT_PLAYER(1)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_START1 )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_BUTTON11 ) PORT_NAME("P1 Bet") PORT_CODE(KEYCODE_A) PORT_PLAYER(1)
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_COIN4 ) PORT_NAME("Coin : 10")
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_COIN3 ) PORT_NAME("Coin :  5")
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_COIN2 ) PORT_NAME("Coin :  2")
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_COIN1 ) PORT_NAME("Coin :  1")

	PORT_START("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_BUTTON5 ) PORT_NAME("P1 Hold 5") PORT_CODE(KEYCODE_B) PORT_PLAYER(1)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_BUTTON4 ) PORT_NAME("P1 Hold 4") PORT_CODE(KEYCODE_V) PORT_PLAYER(1)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_BUTTON3 ) PORT_NAME("P1 Hold 3") PORT_CODE(KEYCODE_C) PORT_PLAYER(1)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_NAME("P1 Hold 2") PORT_CODE(KEYCODE_X) PORT_PLAYER(1)
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_NAME("P1 Hold 1") PORT_CODE(KEYCODE_Z) PORT_PLAYER(1)
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_BUTTON7 ) PORT_NAME("P1 Black") PORT_CODE(KEYCODE_Q) PORT_PLAYER(1)
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_BUTTON6 ) PORT_NAME("P1 Red") PORT_CODE(KEYCODE_W) PORT_PLAYER(1)
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_BUTTON10 ) PORT_NAME("P1 Double Up") PORT_CODE(KEYCODE_E) PORT_PLAYER(1)

	PORT_START("IN2")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("P2 Win") PORT_PLAYER(2)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("P2 Cancel") PORT_PLAYER(2)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_START2 )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("P2 Bet") PORT_PLAYER(2)
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("P2 Coin : 10")
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("P2 Coin :  5")
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("P2 Coin :  2")
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("P2 Coin :  1")

	PORT_START("IN3")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("P2 Hold 5") PORT_PLAYER(2)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("P2 Hold 4") PORT_PLAYER(2)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("P2 Hold 3") PORT_PLAYER(2)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("P2 Hold 2") PORT_PLAYER(2)
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("P2 Hold 1") PORT_PLAYER(2)
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("P2 Black") PORT_PLAYER(2)
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("P2 Red") PORT_PLAYER(2)
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("P2 Double Up") PORT_PLAYER(2)

	PORT_START("IN4")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("P1 Flip") PORT_PLAYER(1)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("P2 Flip") PORT_PLAYER(2)
	PORT_DIPNAME( 0x04, 0x00, "4-3" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x04, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x00, "4-4" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x08, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x00, "4-5" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x10, DEF_STR( On ) )
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_SERVICE1 ) PORT_NAME("Credit Clear")
	PORT_SERVICE_NO_TOGGLE( 0x40, IP_ACTIVE_HIGH )
	PORT_DIPNAME( 0x80, 0x00, DEF_STR( Cabinet ) )
	PORT_DIPSETTING(    0x00, "1 Player" )
	PORT_DIPSETTING(    0x80, "2 Players" )

	PORT_START("IN5")
	PORT_DIPNAME( 0x01, 0x00, "5-1" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x01, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x00, "5-2" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x02, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x00, "5-3" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x04, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x00, "5-4" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x08, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x00, "5-5" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x10, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x00, "5-6" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x20, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x00, DEF_STR( Language ) )
	PORT_DIPSETTING(    0x00, DEF_STR( English ) )
	PORT_DIPSETTING(    0x40, DEF_STR( French ) )
	PORT_DIPNAME( 0x80, 0x00, "5-8" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x80, DEF_STR( On ) )

	PORT_START("IN6")
	PORT_DIPNAME( 0x01, 0x00, "6-1" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x01, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x00, "6-2" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x02, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x00, "6-3" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x04, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x00, "6-4" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x08, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x00, "6-5" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x10, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x00, "6-6" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x20, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x00, "6-7" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x40, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x00, "6-8" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x80, DEF_STR( On ) )
INPUT_PORTS_END

static const gfx_layout charlayout =
{
	8,8,
	RGN_FRAC(1,4),
	4,
	{ 0, RGN_FRAC(1,4), RGN_FRAC(2,4), RGN_FRAC(3,4) },
	{ 0, 1, 2, 3, 4, 5, 6, 7 },
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8 },
	8*8
};

static GFXDECODE_START( supdrapo )
	GFXDECODE_ENTRY( "gfx1", 0, charlayout,   0, 16 )
GFXDECODE_END

static VIDEO_START( supdrapo )
{
}

static VIDEO_UPDATE( supdrapo )
{
	supdrapo_state *state = (supdrapo_state *)screen->machine->driver_data;
	int x,y;
	int count;
	int color;

	count = 0;

	for(y=0;y<32;y++)
	{
		for(x=0;x<32;x++)
		{
			int tile = state->videoram[count] + state->char_bank[count] * 0x100;
			/* Global Column Coloring, GUESS! */
			color = state->col_line[(x*2)+1] ? (state->col_line[(x*2)+1]-1) & 0x7 : 0;

			drawgfx_opaque(bitmap,cliprect,screen->machine->gfx[0],tile,color,0,0,x*8,y*8);

			count++;
		}
	}

	return 0;
}


/*Maybe bit 2 & 3 of the second color prom are intensity bits? */
static PALETTE_INIT( sdpoker )
{
	int	bit0, bit1, bit2 , r, g, b;
	int	i;

	for (i = 0; i < 0x100; ++i)
	{
		bit0 = 0;//(color_prom[0] >> 0) & 0x01;
		bit1 = (color_prom[0] >> 0) & 0x01;
		bit2 = (color_prom[0] >> 1) & 0x01;
		g = 0x21 * bit0 + 0x47 * bit1 + 0x97 * bit2;
		bit0 = 0;//(color_prom[0] >> 3) & 0x01;
		bit1 = (color_prom[0] >> 2) & 0x01;
		bit2 = (color_prom[0] >> 3) & 0x01;
		b = 0x21 * bit0 + 0x47 * bit1 + 0x97 * bit2;
		bit0 = 0;//(color_prom[0] >> 0) & 0x01;
		bit1 = (color_prom[0x100] >> 0) & 0x01;
		bit2 = (color_prom[0x100] >> 1) & 0x01;
		r = 0x21 * bit0 + 0x47 * bit1 + 0x97 * bit2;

		palette_set_color(machine, i, MAKE_RGB(r, g, b));
		color_prom++;
	}
}

static MACHINE_DRIVER_START( supdrapo )

	MDRV_DRIVER_DATA( supdrapo_state )

	MDRV_CPU_ADD("maincpu", Z80,8000000/2) /* ??? */
	MDRV_CPU_PROGRAM_MAP(sdpoker_mem)
	MDRV_CPU_VBLANK_INT("screen", irq0_line_hold)

	/* video hardware */
	MDRV_SCREEN_ADD("screen", RASTER)
	MDRV_SCREEN_REFRESH_RATE(60)
	MDRV_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MDRV_SCREEN_FORMAT(BITMAP_FORMAT_INDEXED16)
	MDRV_SCREEN_SIZE(256, 256)
	MDRV_SCREEN_VISIBLE_AREA(0*8, 32*8-1, 0*8, 32*8-1)

	MDRV_GFXDECODE(supdrapo)
	MDRV_PALETTE_LENGTH(0x100)
	MDRV_PALETTE_INIT(sdpoker)

	MDRV_VIDEO_START(supdrapo)
	MDRV_VIDEO_UPDATE(supdrapo)

	MDRV_SPEAKER_STANDARD_MONO("mono")

	MDRV_SOUND_ADD("aysnd", AY8910, 8000000/2) /* ??? */
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.50)
MACHINE_DRIVER_END

ROM_START( supdrapo )
	ROM_REGION( 0x010000, "maincpu", 0 )
	ROM_LOAD( "a2-1c",        0x0000, 0x1000, CRC(b65af689) SHA1(b45cd15ca8f9c931d83a90f3cdbebf218070b195) )
	ROM_LOAD( "a2-1d",        0x1000, 0x1000, CRC(9ccc4347) SHA1(ea8f4d17aaacc7091ca0a66247b55eb12155c9d7) )
	ROM_LOAD( "a1-1e",        0x2000, 0x1000, CRC(44f2b75d) SHA1(615d0acd3f8a109334f415732b6b4fe7b810d91c) ) //a2-1e
	ROM_LOAD( "a1-1h",        0x3000, 0x1000, CRC(9c1a10ff) SHA1(243dd64f0b29f9bed4cfa8ecb801ddd973d9e3c3) )
	ROM_LOAD( "a3-1j",        0x4000, 0x1000, CRC(71c2bf1c) SHA1(cb98bbf88b8a410075a074ec8619c6e703c6c582) )

	ROM_REGION( 0x04000, "gfx1", 0 )
	ROM_LOAD( "a1-4p",        0x0000, 0x1000, CRC(5ac096cc) SHA1(60173a83d0e60fd4d0eb40b7b4e80a74ac5fb23d) )
	ROM_LOAD( "a1-4n",        0x1000, 0x1000, CRC(6985fac9) SHA1(c6357fe0f042b67f8559ec9da03106d1ff08dc66) )
	ROM_LOAD( "a1-4l",        0x2000, 0x1000, CRC(534f7b94) SHA1(44b83053827b27b9e45f6fc50d3878984ef5c5cc) )
	ROM_LOAD( "a1-4k",        0x3000, 0x1000, CRC(3d881f5b) SHA1(53d8800a098e4393224de0b82f8e516f73fd6b62) )

	ROM_REGION( 0x00200, "proms", 0 )
	ROM_LOAD( "a1-9n",        0x0000, 0x0100, CRC(e62529e3) SHA1(176f2069b0c06c1d088909e81658652af06c8eec) )
	ROM_LOAD( "a1-9p",        0x0100, 0x0100, CRC(a0547746) SHA1(747c8aef5afa26124fe0763e7f96c4ff6be31863) )
ROM_END


GAME( 1983, supdrapo, 0, supdrapo, supdrapo, 0, ROT90, "Valadon Automation (Stern license)", "Super Draw Poker", 0 )
