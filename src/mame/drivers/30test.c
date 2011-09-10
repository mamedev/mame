/***************************************************************************

30-TEST (Remake)
NAMCO 1997
GAME CODE M125

MC68HC11K1
M6295
X1 1.056MHz
OSC1 16.000MHz


cabinet photo
http://blogs.yahoo.co.jp/nadegatayosoyuki/59285865.html

***************************************************************************/

#include "emu.h"
#include "cpu/mc68hc11/mc68hc11.h"
#include "sound/okim6295.h"
#include "30test.lh"

#define MAIN_CLOCK XTAL_16MHz

class namco_30test_state : public driver_device
{
public:
	namco_30test_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag) { }

	UINT8 mux_data;
};


static VIDEO_START( 30test )
{

}

static SCREEN_UPDATE( 30test )
{
	return 0;
}

static READ8_HANDLER( unk_r )
{
	return 1;
}

static READ8_HANDLER(hc11_mux_r)
{
	namco_30test_state *state = space->machine().driver_data<namco_30test_state>();

	return state->mux_data;
}

static WRITE8_HANDLER(hc11_mux_w)
{
	namco_30test_state *state = space->machine().driver_data<namco_30test_state>();

	state->mux_data = data;
}

static READ8_HANDLER(namco_30test_mux_r)
{
	namco_30test_state *state = space->machine().driver_data<namco_30test_state>();

	switch(state->mux_data)
	{
		default:
			return 0xff;
	}

	return 0;
}

static const UINT8 led_map[16] =
	{ 0x3f,0x06,0x5b,0x4f,0x66,0x6d,0x7c,0x07,0x7f,0x67,0x77,0x7c,0x39,0x5e,0x79,0x00 };

static WRITE8_HANDLER( namco_30test_led_w )
{
	output_set_digit_value(0 + offset * 2, led_map[(data & 0xf0) >> 4]);
	output_set_digit_value(1 + offset * 2, led_map[(data & 0x0f) >> 0]);
}

static ADDRESS_MAP_START( namco_30test_map, AS_PROGRAM, 8 )
	AM_RANGE(0x0000, 0x003f) AM_RAM // internal I/O
	AM_RANGE(0x007c, 0x007c) AM_READWRITE(hc11_mux_r,hc11_mux_w)
	AM_RANGE(0x0040, 0x007f) AM_RAM // more internal I/O, HC11 change pending
	AM_RANGE(0x0080, 0x037f) AM_RAM // internal RAM
	AM_RANGE(0x0d80, 0x0dbf) AM_RAM	// EEPROM read-back data goes there
	AM_RANGE(0x2000, 0x2000) AM_DEVREADWRITE_MODERN("oki", okim6295_device, read, write)
	AM_RANGE(0x4000, 0x401f) AM_WRITE(namco_30test_led_w) // 7-seg leds
	AM_RANGE(0x6000, 0x6004) AM_RAM
	AM_RANGE(0x8000, 0xffff) AM_ROM
ADDRESS_MAP_END

static ADDRESS_MAP_START( namco_30test_io, AS_IO, 8 )
	AM_RANGE(MC68HC11_IO_PORTA,MC68HC11_IO_PORTA) AM_READ(namco_30test_mux_r)
	AM_RANGE(MC68HC11_IO_PORTD,MC68HC11_IO_PORTD) AM_RAM//AM_READ_PORT("SYSTEM")
	AM_RANGE(MC68HC11_IO_PORTE,MC68HC11_IO_PORTE) AM_READ(unk_r)
ADDRESS_MAP_END


static INPUT_PORTS_START( 30test )
	PORT_START("SYSTEM")
	PORT_DIPNAME( 0x01, 0x01, "SYSTEM" )
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Unknown ) )
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
INPUT_PORTS_END


static MACHINE_START( 30test )
{

}

static MACHINE_RESET( 30test )
{

}

static const hc11_config namco_30test_config =
{
	0, 	   //has extended internal I/O
	768,   //internal RAM size
	0x00   //registers are at 0-0x100
};


static MACHINE_CONFIG_START( 30test, namco_30test_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", MC68HC11,MAIN_CLOCK/4)
	MCFG_CPU_PROGRAM_MAP(namco_30test_map)
	MCFG_CPU_IO_MAP(namco_30test_io)
	MCFG_CPU_CONFIG(namco_30test_config)
//	MCFG_CPU_PERIODIC_INT(irq0_line_hold,4*60) // unknown timing

	MCFG_MACHINE_START(30test)
	MCFG_MACHINE_RESET(30test)

	/* video hardware */
	/* TODO: NOT raster! */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MCFG_SCREEN_FORMAT(BITMAP_FORMAT_INDEXED16)
	MCFG_SCREEN_SIZE(32*8, 32*8)
	MCFG_SCREEN_VISIBLE_AREA(0*8, 32*8-1, 2*8, 30*8-1)
	MCFG_SCREEN_UPDATE(30test)

//	MCFG_PALETTE_INIT(30test)
	MCFG_PALETTE_LENGTH(8)

	MCFG_VIDEO_START(30test)

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")

	MCFG_OKIM6295_ADD("oki", 1056000, OKIM6295_PIN7_HIGH) // pin 7 not verified
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 1.0)
MACHINE_CONFIG_END

/***************************************************************************

  Game driver(s)

***************************************************************************/

ROM_START( 30test )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "tt1-mpr0b.8p",   0x0000, 0x10000, CRC(455043d5) SHA1(46b15324d193ee621beabce92c0dc493b608b8dd) )

	ROM_REGION( 0x80000, "oki", 0 )
	ROM_LOAD( "tt1-voi0.7p",   0x0000, 0x80000, CRC(b4fc5921) SHA1(92a88d5adb50dae48715847f12e88a35e37ef78c) )
ROM_END

GAMEL( 1997, 30test,  0,   30test,  30test,  0, ROT0, "Namco", "30 Test (Remake)", GAME_NOT_WORKING | GAME_NO_SOUND, layout_30test )
