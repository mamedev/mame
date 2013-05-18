/*

Sea Battle by Zaccaria

Memory map in pics...


2650 + 2636

sea b b_1 *.prg are 2650 progamm

sea b blu.prg is blue data?
sea b red.prg is red data?
sea b green.prg is green data?	for video?

sea b wawe.prg is sea wave data?

sea b screen.prg ???


the sound board should be fully discrete.


DS0		1	2	3
PLAY TIME	ON	ON	ON	free game
		ON	OFF	ON	75 seconds
		OFF 	OFF	ON	90 seconds
		OFF	ON	ON	105 seconds

SHIP NUMBER	ON	ON	OFF	free game
		ON	OFF	OFF	3 ships
		OFF	OFF	OFF	4 ships
		OFF	ON	OFF	5 ships
	I don't forget anything, this is a copy of the manual
	DS0-3	seem to select from time based games to ships based game.


			
DS0		4	5	6
COIN SLOT 2	ON	ON	ON	2 coin 1 play
		ON	OFF	ON	1 coin 1 play
		ON	ON	OFF	1 coin 2 plays
		ON	OFF	OFF	1 coin 3 plays
		OFF	ON	ON	1 coin 4 plays	
		OFF	OFF	ON	1 coin 5 plays
		OFF	ON	OFF	1 coin 6 plays
		OFF	OFF	OFF	1 coin 7 plays


DS0		7	8	DS1-1
COIN SLOT 1	ON	ON	ON	2 coin 1 play
		ON	OFF	ON	1 coin 1 play
		ON	ON	OFF	1 coin 2 plays
		ON	OFF	OFF	1 coin 3 plays
		OFF	ON	ON	1 coin 4 plays	
		OFF	OFF	ON	1 coin 5 plays
		OFF	ON	OFF	1 coin 6 plays
		OFF	OFF	OFF	1 coin 7 plays

DS1		2
SHIP SPEED	ON	fast
		OFF	slow


DS1		3	4
EXTEND PLAY	OFF	OFF	no extended
		ON	OFF	2000 points
		OFF	ON	3000 points
		ON	ON	4000 points

DS1		5
GRID		ON	game
		OFF	grid

DS1 6-7-8 not used

*/

#include "emu.h"
#include "cpu/s2650/s2650.h"


class seabattl_state : public driver_device
{
public:
	seabattl_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu")
	{
	}

	required_device<cpu_device> m_maincpu;

	virtual void machine_start();
	virtual void machine_reset();
	virtual void video_start();
	UINT32 screen_update_seabattl(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
};



UINT32 seabattl_state::screen_update_seabattl(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	return 0;
}

void seabattl_state::video_start()
{

}

static ADDRESS_MAP_START( seabattl_map, AS_PROGRAM, 8, seabattl_state )
	AM_RANGE(0x0000, 0x13ff) AM_ROM
	AM_RANGE(0x1400, 0x17ff) AM_RAM
	AM_RANGE(0x2000, 0x33ff) AM_ROM
	AM_RANGE(0x3400, 0x37ff) AM_RAM
ADDRESS_MAP_END

static INPUT_PORTS_START( seabattl )
INPUT_PORTS_END

void seabattl_state::machine_start()
{

}

void seabattl_state::machine_reset()
{

}

static MACHINE_CONFIG_START( seabattl, seabattl_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", S2650, 14318180/4) // ???
	MCFG_CPU_PROGRAM_MAP(seabattl_map)
//	MCFG_CPU_IO_MAP(seabattl_io_map)
//	MCFG_CPU_VBLANK_INT_DRIVER("screen", seabattl_state,  seabattl_interrupt)

	MCFG_PALETTE_LENGTH(256)

	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(50)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(2500) /* not accurate */)
	MCFG_SCREEN_SIZE(256, 256)
	MCFG_SCREEN_VISIBLE_AREA(1*8, 29*8-1, 2*8, 32*8-1)
	MCFG_SCREEN_UPDATE_DRIVER(seabattl_state, screen_update_seabattl)
MACHINE_CONFIG_END


ROM_START( seabattl )
	ROM_REGION( 0x8000, "maincpu", 0 )
	ROM_LOAD( "sea b b_1 1.prg",      0x0000, 0x0400, CRC(16a475c0) SHA1(5380d3be39c421227e52012d1bcf0516e99f6a3f) )
	ROM_CONTINUE(                     0x2000, 0x0400 )
	ROM_LOAD( "sea b b_1 2.prg",      0x0400, 0x0400, CRC(4bd73a82) SHA1(9ab4edf24fcd437ecd8e9e551ce0ed33be3bbad7) )
	ROM_CONTINUE(                     0x2400, 0x0400 )
	ROM_LOAD( "sea b b_1 3.prg",      0x0800, 0x0400, CRC(e251492b) SHA1(a152f9b6f189909ff478b4d95ee764f1898405b5) )
	ROM_CONTINUE(                     0x2800, 0x0400 )
	ROM_LOAD( "sea b b_1 4.prg",      0x0c00, 0x0400, CRC(6012b83f) SHA1(57de9e45253609b71f14fb3541760fd33647a651) )
	ROM_CONTINUE(                     0x2c00, 0x0400 )
	ROM_LOAD( "sea b b_1 5.prg",      0x1000, 0x0400, CRC(55c263f6) SHA1(33eba61cb8c9318cf19b771c93a14397b4ee0ace) )
	ROM_CONTINUE(                     0x3000, 0x0400 )

	ROM_REGION( 0x1800, "gfx1", 0 )
	ROM_LOAD( "sea b red.prg",      0x0000, 0x0800, CRC(fe7192df) SHA1(0b262bc1ac959d8dd79d71780e16237075f4a099) )
	ROM_LOAD( "sea b green.prg",    0x0800, 0x0800, CRC(cea4c0c9) SHA1(697c136ef363676b346692740d3c3a482dde6207) )
	ROM_LOAD( "sea b blu.prg",      0x1000, 0x0800, CRC(cd972c4a) SHA1(fcb8149bc462912c8393431ccb792ea4b1b1109d) )

	ROM_REGION( 0x0800, "gfx2", 0 )
	ROM_LOAD( "sea b screen.prg",     0x0000, 0x0800, CRC(4e98f719) SHA1(2cdbc23aed790807b2dc730258916cc32dab1a31) )

	ROM_REGION( 0x0800, "gfx3", 0 )
	ROM_LOAD( "sea b wawe.prg",     0x0000, 0x0800, CRC(7e356dc5) SHA1(71d34fa39ff0b7d0fa6d32ba2b9dc0006a03d1bb) )
ROM_END

GAME( 1980, seabattl, 0,        seabattl, seabattl, driver_device, 0, ROT0,  "Zaccaria", "Sea Battle",                    GAME_NO_SOUND | GAME_NOT_WORKING )
