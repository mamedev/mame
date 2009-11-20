/**********************************************************************************************

M14 Hardware (c) 1979 Irem

driver by Angelo Salese

TODO:
- Sound (very likely to be discrete);
- Colors might be not 100% accurate (needs screenshots from the real thing);
- What are the high 4 bits in the colorram for? They are used on the mahjong tiles only,
  left-over or something more important?
- I'm not sure about the hopper hook-up, it could also be that the player should press
  start + button 1 + ron buttons (= 0x43) instead of being "automatic";
- Inputs are grossly mapped;

Notes:
- If you call a ron but you don't have the right hand you'll automatically lose the match;
- If you make the timer to run out, you'll lose the turn but you don't get any visible message
  (presumably signaled by a sound effect);
- As you could expect, the cpu hands are actually pre-determined, so you actually play alone
  against a variable number of available tiles;


==============================================================================================
x (Mystery Rom)
(c)1978-1981? Irem?
PCB No. :M14S-2
    :M14L-2
CPU :NEC D8085AC
Sound   :?
OSC :6MHz x2

mgpb1.bin
mgpa2.bin
mgpa3.bin
mgpa4.bin
mgpa5.bin
mgpb6.bin
mgpa7.bin
mgpb8.bin

mgpa9.bin
mgpa10.bin


--- Team Japump!!! ---
http://japump.i.am/
Dumped by Chackn
01/30/2000

**********************************************************************************************/

#include "driver.h"
#include "cpu/i8085/i8085.h"


typedef struct _m14_state m14_state;
struct _m14_state
{
	/* video-related */
	tilemap  *m14_tilemap;
	UINT8 *  video_ram;
	UINT8 *  color_ram;

	/* input-related */
	UINT8 hop_mux;
};


/*************************************
 *
 *  Video Hardware
 *
 *************************************/

/* guess, might not be 100% accurate. */
static PALETTE_INIT( m14 )
{
	int i;

	for (i = 0; i < 0x20; i++)
	{
		rgb_t color;

		if (i & 0x01)
			color = MAKE_RGB(pal1bit(i >> 1), pal1bit(i >> 2), pal1bit(i >> 3));
		else
			color = (i & 0x10) ? RGB_WHITE : RGB_BLACK;

		palette_set_color(machine, i, color);
	}
}

static TILE_GET_INFO( m14_get_tile_info )
{
	m14_state *state = (m14_state *)machine->driver_data;

	int code = state->video_ram[tile_index];
	int color = state->color_ram[tile_index] & 0x0f;

	/* colorram & 0xf0 used but unknown purpose*/

	SET_TILE_INFO(
			0,
			code,
			color,
			0);
}

static VIDEO_START( m14 )
{
	m14_state *state = (m14_state *)machine->driver_data;

	state->m14_tilemap = tilemap_create(machine, m14_get_tile_info, tilemap_scan_rows, 8, 8, 32, 32);
}

static VIDEO_UPDATE( m14 )
{
	m14_state *state = (m14_state *)screen->machine->driver_data;

	tilemap_draw(bitmap, cliprect, state->m14_tilemap, 0, 0);
	return 0;
}


static WRITE8_HANDLER( m14_vram_w )
{
	m14_state *state = (m14_state *)space->machine->driver_data;

	state->video_ram[offset] = data;
	tilemap_mark_tile_dirty(state->m14_tilemap, offset);
}

static WRITE8_HANDLER( m14_cram_w )
{
	m14_state *state = (m14_state *)space->machine->driver_data;

	state->color_ram[offset] = data;
	tilemap_mark_tile_dirty(state->m14_tilemap, offset);
}

/*************************************
 *
 *  I/O
 *
 *************************************/

static READ8_HANDLER( m14_rng_r )
{
	/* graphic artifacts happens if this doesn't return random values. */
	return (mame_rand(space->machine) & 0x0f) | 0xf0; /* | (input_port_read(space->machine, "IN1") & 0x80)*/;
}

/* Here routes the hopper & the inputs */
static READ8_HANDLER( input_buttons_r )
{
	m14_state *state = (m14_state *)space->machine->driver_data;

	if (state->hop_mux)
	{
		state->hop_mux = 0;
		return 0; //0x43 status bits
	}
	else
		return input_port_read(space->machine, "IN0");
}

#if 0
static WRITE8_HANDLER( test_w )
{
	static UINT8 x[5];

	x[offset] = data;

	popmessage("%02x %02x %02x %02x %02x",x[0],x[1],x[2],x[3],x[4]);
}
#endif

static WRITE8_HANDLER( hopper_w )
{
	m14_state *state = (m14_state *)space->machine->driver_data;

	/* ---- x--- coin out */
	/* ---- --x- hopper/input mux? */
	state->hop_mux = data & 2;
	//popmessage("%02x",data);
}

/*************************************
 *
 *  Memory Map
 *
 *************************************/

static ADDRESS_MAP_START( m14_map, ADDRESS_SPACE_PROGRAM, 8 )
	AM_RANGE(0x0000, 0x1fff) AM_ROM
	AM_RANGE(0x2000, 0x23ff) AM_RAM
	AM_RANGE(0xe000, 0xe3ff) AM_RAM_WRITE(m14_vram_w) AM_BASE_MEMBER(m14_state, video_ram)
	AM_RANGE(0xe400, 0xe7ff) AM_RAM_WRITE(m14_cram_w) AM_BASE_MEMBER(m14_state, color_ram)
ADDRESS_MAP_END

static ADDRESS_MAP_START( m14_io_map, ADDRESS_SPACE_IO, 8 )
	ADDRESS_MAP_GLOBAL_MASK(0xff)
	AM_RANGE(0xf8, 0xf8) AM_READ_PORT("AN_PADDLE") AM_WRITENOP
	AM_RANGE(0xf9, 0xf9) AM_READ(input_buttons_r) AM_WRITENOP
	AM_RANGE(0xfa, 0xfa) AM_READ(m14_rng_r) AM_WRITENOP
	AM_RANGE(0xfb, 0xfb) AM_READ_PORT("DSW") AM_WRITE(hopper_w)
	AM_RANGE(0xf8, 0xfc) AM_WRITENOP
ADDRESS_MAP_END

/*************************************
 *
 *  Input Port Definitions
 *
 *************************************/

static INPUT_CHANGED( left_coin_inserted )
{
	/* left coin insertion causes a rst6.5 (vector 0x34) */
	if (newval)
		cputag_set_input_line(field->port->machine, "maincpu", I8085_RST65_LINE, HOLD_LINE);
}

static INPUT_CHANGED( right_coin_inserted )
{
	/* right coin insertion causes a rst5.5 (vector 0x2c) */
	if (newval)
		cputag_set_input_line(field->port->machine, "maincpu", I8085_RST55_LINE, HOLD_LINE);
}

static INPUT_PORTS_START( m14 )
	PORT_START("AN_PADDLE")
	PORT_BIT( 0xff, 0x00, IPT_PADDLE  ) PORT_MINMAX(0,0xff) PORT_SENSITIVITY(5) PORT_KEYDELTA(1) PORT_CENTERDELTA(0) PORT_REVERSE

	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_MAHJONG_RON ) //could be reach too
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Unknown ) ) //affects medal settings?
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, "Freeze" )
	PORT_DIPSETTING(    0x20, DEF_STR( No ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Yes ) )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_START1 )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("DSW") //this whole port is stored at work ram $2112.
	PORT_DIPNAME( 0x01, 0x00, "Show available tiles" ) // debug mode for the rng?
	PORT_DIPSETTING(    0x00, DEF_STR( No ) )
	PORT_DIPSETTING(    0x01, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x02, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x02, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x04, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x08, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x10, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x20, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x40, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x80, DEF_STR( On ) )

	PORT_START("FAKE")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_COIN1 ) PORT_IMPULSE(1) PORT_CHANGED(left_coin_inserted, 0) //coin x 5
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_COIN2 ) PORT_IMPULSE(1) PORT_CHANGED(right_coin_inserted, 0) //coin x 1
INPUT_PORTS_END

static const gfx_layout charlayout =
{
	8,8,
	RGN_FRAC(1,1),
	1,
	{ RGN_FRAC(0,1) },
	{ 0, 1, 2, 3, 4, 5, 6, 7 },
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8 },
	8*8
};

static GFXDECODE_START( m14 )
	GFXDECODE_ENTRY( "gfx1", 0, charlayout,     0, 0x10 )
GFXDECODE_END

static INTERRUPT_GEN( m14_irq )
{
	cpu_set_input_line(device, I8085_RST75_LINE, ASSERT_LINE);
	cpu_set_input_line(device, I8085_RST75_LINE, CLEAR_LINE);
}

static MACHINE_START( m14 )
{
	m14_state *state = (m14_state *)machine->driver_data;

	state_save_register_global(machine, state->hop_mux);
}

static MACHINE_RESET( m14 )
{
	m14_state *state = (m14_state *)machine->driver_data;

	state->hop_mux = 0;
}


static MACHINE_DRIVER_START( m14 )

	/* driver data */
	MDRV_DRIVER_DATA(m14_state)

	/* basic machine hardware */
	MDRV_CPU_ADD("maincpu",8085A,6000000/2) //guess: 6 Mhz internally divided by 2
	MDRV_CPU_PROGRAM_MAP(m14_map)
	MDRV_CPU_IO_MAP(m14_io_map)
	MDRV_CPU_VBLANK_INT("screen",m14_irq)

	MDRV_MACHINE_START(m14)
	MDRV_MACHINE_RESET(m14)

	/* video hardware */
	MDRV_SCREEN_ADD("screen", RASTER)
	MDRV_SCREEN_REFRESH_RATE(60)
	MDRV_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(2500)) //not accurate
	MDRV_SCREEN_FORMAT(BITMAP_FORMAT_INDEXED16)
	MDRV_SCREEN_SIZE(32*8, 32*8)
	MDRV_SCREEN_VISIBLE_AREA(0*8, 32*8-1, 0*8, 28*8-1)
	MDRV_GFXDECODE(m14)
	MDRV_PALETTE_LENGTH(0x20)
	MDRV_PALETTE_INIT(m14)

	MDRV_VIDEO_START(m14)
	MDRV_VIDEO_UPDATE(m14)

	/* sound hardware */
//  MDRV_SPEAKER_STANDARD_MONO("mono")

//  MDRV_SOUND_ADD("discrete", DISCRETE, 0)
//  MDRV_SOUND_CONFIG_DISCRETE(m14)
//  MDRV_SOUND_ROUTE(ALL_OUTPUTS, "mono", 1.0)
MACHINE_DRIVER_END

/***************************************************************************

  Game driver(s)

***************************************************************************/


ROM_START( ptrmj )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "mgpb1.bin",   0x0000, 0x0400, CRC(47c041b8) SHA1(e834c375e689f99a13964863fc9847a8e148ec91) )
	ROM_LOAD( "mgpa2.bin",   0x0400, 0x0400, CRC(cf8bfa23) SHA1(091055e803255f1b5520f50b31af7135d71d0a40) )
	ROM_LOAD( "mgpa3.bin",   0x0800, 0x0400, CRC(a07a3093) SHA1(5b86bb11e83c06f828956e7db6dd2c105b023b03) )
	ROM_LOAD( "mgpa4.bin",   0x0c00, 0x0400, CRC(a420241c) SHA1(7497d90014dabb49f9db1d5d8e3014c634045725) )
	ROM_LOAD( "mgpa5.bin",   0x1000, 0x0400, CRC(a2df92a3) SHA1(97a3d4b188d26f172881f8cf86bdd83d549f5b74) )
	ROM_LOAD( "mgpb6.bin",   0x1400, 0x0400, CRC(f5c0fcd4) SHA1(14e2d04be105caeb221dfc226f84cb1722ae2627) )
	ROM_LOAD( "mgpa7.bin",   0x1800, 0x0400, CRC(56ed7eb2) SHA1(acf954bf4daadb225475937dd3c36baa996f7b65) )
	ROM_LOAD( "mgpb8.bin",   0x1c00, 0x0400, CRC(3ecb8214) SHA1(8575bcb49f693aa8798b8a7d9a76392bfcc90e0e) )

	ROM_REGION( 0x0800, "gfx1", 0 )
	ROM_LOAD( "mgpa9.bin",   0x0000, 0x0400, CRC(cb68b4ec) SHA1(2cf596affb155ae38729fcd95cae424073faf74d) )
	ROM_LOAD( "mgpa10.bin",  0x0400, 0x0400, CRC(e1a4ebdc) SHA1(d9df42424ede17f0634d8d0a56c0374a33c55333) )
ROM_END

GAME( 1979, ptrmj,  0,       m14,  m14,  0, ROT0, "Irem", "PT Reach Mahjong (Japan)", GAME_NO_SOUND | GAME_SUPPORTS_SAVE )
