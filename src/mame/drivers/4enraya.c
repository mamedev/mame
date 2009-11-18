/***************************************************************************

Driver by Tomasz Slanina  dox@space.pl

***************************************************************************

RAM :
    1 x GM76c28-10 (6116) RAM
    3 x 2114  - VRAM (only 10 bits are used )

ROM:
  27256 + 27128 for code/data
  3x2764 for gfx

PROM:
 82S123 32x8
 Used for system control
    (d0 - connected to ROM5 /CS , d1 - ROM4 /CS, d2 - RAM /CS , d3 - to some logic(gfx control), and Z80 WAIT )

Memory Map :
  0x0000 - 0xbfff - ROM
  0xc000 - 0xcfff - RAM
  0xd000 - 0xdfff - VRAM mirrored write,
        tilemap offset = address & 0x3ff
        tile number =  bits 0-7 = data, bits 8,9  = address bits 10,11

Video :
    No scrolling , no sprites.
    32x32 Tilemap stored in VRAM (10 bits/tile (tile numebr 0-1023))

    3 gfx ROMS
    ROM1 - R component (ROM ->(parallel in) shift register 74166 (serial out) -> jamma output
    ROM2 - B component
    ROM3 - G component

    Default MAME color palette is used

Sound :
 AY 3 8910

 sound_control :

  bit 0 - BC1
  bit 1 - BC2
  bit 2 - BDIR

  bits 3-7 - not connected

***************************************************************************/

#include "driver.h"
#include "deprecat.h"
#include "cpu/z80/z80.h"
#include "sound/ay8910.h"
#include "includes/4enraya.h"

static WRITE8_HANDLER( sound_data_w )
{
	_4enraya_state *state = (_4enraya_state *)space->machine->driver_data;
	state->soundlatch = data;
}

static WRITE8_DEVICE_HANDLER( sound_control_w )
{
	_4enraya_state *state = (_4enraya_state *)device->machine->driver_data;

	if ((state->last_snd_ctrl & 0x04) == 0x04 && (data & 0x4) == 0x00)
		ay8910_data_address_w(device, state->last_snd_ctrl, state->soundlatch);

	state->last_snd_ctrl = data;
}

static ADDRESS_MAP_START( main_map, ADDRESS_SPACE_PROGRAM, 8 )
	AM_RANGE(0x0000, 0xbfff) AM_ROM
	AM_RANGE(0xc000, 0xcfff) AM_RAM
	AM_RANGE(0xd000, 0xdfff) AM_WRITE(fenraya_videoram_w) AM_BASE_MEMBER(_4enraya_state, videoram) AM_SIZE(&videoram_size)
ADDRESS_MAP_END

static ADDRESS_MAP_START( main_portmap, ADDRESS_SPACE_IO, 8 )
	ADDRESS_MAP_GLOBAL_MASK(0xff)
	AM_RANGE(0x00, 0x00) AM_READ_PORT("DSW")
	AM_RANGE(0x01, 0x01) AM_READ_PORT("INPUTS")
	AM_RANGE(0x02, 0x02) AM_READ_PORT("SYSTEM")
	AM_RANGE(0x23, 0x23) AM_WRITE(sound_data_w)
	AM_RANGE(0x33, 0x33) AM_DEVWRITE("aysnd", sound_control_w)
ADDRESS_MAP_END


static INPUT_PORTS_START( 4enraya )
	PORT_START("DSW")
	PORT_DIPNAME( 0x01, 0x01, DEF_STR( Difficulty ) )	PORT_DIPLOCATION("SW1:1")
	PORT_DIPSETTING(    0x01, DEF_STR( Easy ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Hard ) )
	PORT_DIPNAME( 0x02, 0x00, DEF_STR( Demo_Sounds ) )	PORT_DIPLOCATION("SW1:2")
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, "Pieces" )				PORT_DIPLOCATION("SW1:3")
	PORT_DIPSETTING(    0x04, "30" )
	PORT_DIPSETTING(    0x00, "16" )
	PORT_DIPNAME( 0x08, 0x08, "Speed" )					PORT_DIPLOCATION("SW1:4")
	PORT_DIPSETTING(    0x08, "Slow" )
	PORT_DIPSETTING(    0x00, "Fast" )
	PORT_DIPNAME( 0x30, 0x30, DEF_STR( Coin_B ) )		PORT_DIPLOCATION("SW1:5,6")
	PORT_DIPSETTING(    0x30, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 2C_3C ) )
	PORT_DIPSETTING(    0x10, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0x20, DEF_STR( 1C_4C ) )
	PORT_DIPNAME( 0xc0, 0xc0, DEF_STR( Coin_A ) )		PORT_DIPLOCATION("SW1:7,8")
	PORT_DIPSETTING(    0x40, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0xc0, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 2C_3C ) )
	PORT_DIPSETTING(    0x80, DEF_STR( 1C_2C ) )

	PORT_START("INPUTS")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_2WAY PORT_PLAYER(2)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_2WAY PORT_PLAYER(1)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1) PORT_NAME("P1 Down")			// "drop" ("down")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_2WAY PORT_PLAYER(2)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)	PORT_NAME("P2 Down")			// "drop" ("down")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_2WAY PORT_PLAYER(1)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(1) PORT_NAME("P1 Shot")			// "fire" ("shot")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2)	PORT_NAME("P2 Shot")			// "fire" ("shot")

	PORT_START("SYSTEM")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_COIN2 )
INPUT_PORTS_END

static const gfx_layout charlayout =
{
	8,8,	/* 8*8 characters */
	RGN_FRAC(1,3),	/* 1024 characters */
	3,	/* 3 bits per pixel */
	{ RGN_FRAC(1,3), RGN_FRAC(2,3), RGN_FRAC(0,3) },	/* the bitplanes are separated */
	{ 0, 1, 2, 3, 4, 5, 6, 7 },
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8 },
	8*8	/* every char takes 8 consecutive bytes */
};

static GFXDECODE_START( 4enraya )
	GFXDECODE_ENTRY( "gfx1", 0, charlayout,     0, 8 )
GFXDECODE_END

static MACHINE_START( 4enraya )
{
	_4enraya_state *state = (_4enraya_state *)machine->driver_data;

	state_save_register_global(machine, state->soundlatch);
	state_save_register_global(machine, state->last_snd_ctrl);
}

static MACHINE_RESET( 4enraya )
{
	_4enraya_state *state = (_4enraya_state *)machine->driver_data;

	state->soundlatch = 0;
	state->last_snd_ctrl = 0;
}

static MACHINE_DRIVER_START( 4enraya )

	MDRV_DRIVER_DATA(_4enraya_state)

	/* basic machine hardware */
	MDRV_CPU_ADD("maincpu",Z80,8000000/2)
	MDRV_CPU_PROGRAM_MAP(main_map)
	MDRV_CPU_IO_MAP(main_portmap)
	MDRV_CPU_VBLANK_INT_HACK(irq0_line_hold,4)

	MDRV_MACHINE_START(4enraya)
	MDRV_MACHINE_RESET(4enraya)

	/* video hardware */
	MDRV_SCREEN_ADD("screen", RASTER)
	MDRV_SCREEN_REFRESH_RATE(60)
	MDRV_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MDRV_SCREEN_FORMAT(BITMAP_FORMAT_INDEXED16)
	MDRV_SCREEN_SIZE(32*8, 32*8)
	MDRV_SCREEN_VISIBLE_AREA(0*8, 32*8-1, 2*8, 30*8-1)
	MDRV_GFXDECODE(4enraya)
	MDRV_PALETTE_LENGTH(512)

	MDRV_VIDEO_START(4enraya)
	MDRV_VIDEO_UPDATE(4enraya)

	/* sound hardware */
	MDRV_SPEAKER_STANDARD_MONO("mono")
	MDRV_SOUND_ADD("aysnd", AY8910, 8000000/4 /* guess */)
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.30)
MACHINE_DRIVER_END

/***************************************************************************

  Game driver(s)

***************************************************************************/

ROM_START( 4enraya )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "5.bin",   0x0000, 0x8000, CRC(cf1cd151) SHA1(3920b0a6ed5798859158871b578b01ec742b0d13) )
	ROM_LOAD( "4.bin",   0x8000, 0x4000, CRC(f9ec1be7) SHA1(189159129ecbc4f6909c086867b0e02821f5b976) )

	ROM_REGION( 0x6000, "gfx1", 0 )
	ROM_LOAD( "1.bin",   0x0000, 0x2000, CRC(87f92552) SHA1(d16afd963c30f2e60951876b843e5c1dcbee1cfc) )
	ROM_LOAD( "2.bin",   0x2000, 0x2000, CRC(2b0a3793) SHA1(2c3d224251557824bb9641dc2f98a000ab72c4a2) )
	ROM_LOAD( "3.bin",   0x4000, 0x2000, CRC(f6940836) SHA1(afde21ffa0c141cf73243e50da62ecfd474aaac2) )

	ROM_REGION( 0x0020,  "proms", 0 )
	ROM_LOAD( "1.bpr",   0x0000, 0x0020, CRC(dcbd2352) SHA1(ce72e84129ed1b455aaf648e1dfaa4333e7e7628) )	/* not used */
ROM_END

GAME( 1990, 4enraya,  0,   4enraya,  4enraya,  0, ROT0, "IDSA", "4 En Raya", GAME_SUPPORTS_SAVE )
