/***************************************************************************

Template for skeleton drivers

***************************************************************************/

#include "emu.h"
#include "cpu/z80/z80.h"
//#include "sound/ay8910.h"

#define MAIN_CLOCK XTAL_8MHz

class xxx_state : public driver_device
{
public:
	xxx_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag) { }
};

static VIDEO_START( xxx )
{

}

static SCREEN_UPDATE( xxx )
{
	return 0;
}

static ADDRESS_MAP_START( xxx_map, AS_PROGRAM, 8 )
	AM_RANGE(0x0000, 0x7fff) AM_ROM
ADDRESS_MAP_END

static ADDRESS_MAP_START( xxx_io, AS_IO, 8 )
//	ADDRESS_MAP_GLOBAL_MASK(0xff)
ADDRESS_MAP_END

static INPUT_PORTS_START( xxx )
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

static GFXDECODE_START( xxx )
	GFXDECODE_ENTRY( "gfx1", 0, charlayout,     0, 1 )
GFXDECODE_END

static MACHINE_START( xxx )
{
//	xxx_state *state = machine.driver_data<xxx_state>();

}

static MACHINE_RESET( xxx )
{

}

static PALETTE_INIT( xxx )
{
}

static MACHINE_CONFIG_START( xxx, xxx_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu",Z80,MAIN_CLOCK/2)
	MCFG_CPU_PROGRAM_MAP(xxx_map)
	MCFG_CPU_IO_MAP(xxx_io)

	MCFG_MACHINE_START(xxx)
	MCFG_MACHINE_RESET(xxx)

	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MCFG_SCREEN_FORMAT(BITMAP_FORMAT_INDEXED16)
	MCFG_SCREEN_SIZE(32*8, 32*8)
	MCFG_SCREEN_VISIBLE_AREA(0*8, 32*8-1, 2*8, 30*8-1)
	MCFG_SCREEN_UPDATE(xxx)

	MCFG_GFXDECODE(xxx)

	MCFG_PALETTE_INIT(xxx)
	MCFG_PALETTE_LENGTH(8)

	MCFG_VIDEO_START(xxx)

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")
//	MCFG_SOUND_ADD("aysnd", AY8910, MAIN_CLOCK/4)
//	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.30)
MACHINE_CONFIG_END


/***************************************************************************

  Game driver(s)

***************************************************************************/

ROM_START( xxx )
	ROM_REGION( 0x10000, "maincpu", ROMREGION_ERASE00 )
	ROM_REGION( 0x10000, "gfx1", ROMREGION_ERASE00 )
ROM_END

GAME( 198?, xxx,  0,   xxx,  xxx,  0,       ROT0, "<template_manufacturer>",      "<template_gamename>", GAME_NOT_WORKING | GAME_NO_SOUND )
