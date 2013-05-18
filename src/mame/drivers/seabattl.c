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

static const gfx_layout tiles32x16x3_layout =
{
	32,16,
	RGN_FRAC(1,3),
	3,
	{ RGN_FRAC(0,3),RGN_FRAC(1,3),RGN_FRAC(2,3) },
	{ 384,385,386,387,388,389,390,391, 0, 1, 2, 3, 4, 5, 6, 7, 128,129,130,131,132,133,134,135, 256,257,258,259,260,261,262,263 },
	{ 0*8,1*8,2*8,3*8,4*8,5*8,6*8,7*8,8*8,9*8,10*8,11*8,12*8,13*8,14*8,15*8 },
	16*8*4
};


static const gfx_layout tiles8x8_layout =
{
	8,8,
	RGN_FRAC(1,1),
	1,
	{ 0 },
	{ 0, 1, 2, 3, 4, 5, 6, 7 },
	{ 0*8,1*8,2*8,3*8,4*8,5*8,6*8,7*8 },
	8*8
};

static GFXDECODE_START( seabattl )
	GFXDECODE_ENTRY( "gfx1", 0, tiles32x16x3_layout, 0, 1 )
	GFXDECODE_ENTRY( "gfx2", 0, tiles8x8_layout, 0, 1 )
	GFXDECODE_ENTRY( "gfx3", 0, tiles8x8_layout, 0, 1 )
GFXDECODE_END


static MACHINE_CONFIG_START( seabattl, seabattl_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", S2650, 14318180/4) // ???
	MCFG_CPU_PROGRAM_MAP(seabattl_map)
//	MCFG_CPU_IO_MAP(seabattl_io_map)
//	MCFG_CPU_VBLANK_INT_DRIVER("screen", seabattl_state,  seabattl_interrupt)

	MCFG_PALETTE_LENGTH(256)

	MCFG_GFXDECODE(seabattl)

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

	ROM_REGION( 0x1800, "gfx1", 0 ) // first half of each of these is empty, is that correct?
	ROM_LOAD( "sea b red.prg",      0x0000, 0x0800, CRC(fe7192df) SHA1(0b262bc1ac959d8dd79d71780e16237075f4a099) )
	ROM_LOAD( "sea b green.prg",    0x0800, 0x0800, CRC(cea4c0c9) SHA1(697c136ef363676b346692740d3c3a482dde6207) )
	ROM_LOAD( "sea b blu.prg",      0x1000, 0x0800, CRC(cd972c4a) SHA1(fcb8149bc462912c8393431ccb792ea4b1b1109d) )

	ROM_REGION( 0x0800, "gfx2", 0 )
	ROM_LOAD( "sea b screen.prg",     0x0000, 0x0800, CRC(4e98f719) SHA1(2cdbc23aed790807b2dc730258916cc32dab1a31) )

	ROM_REGION( 0x0800, "gfx3", 0 )
	ROM_LOAD( "sea b wawe.prg",     0x0000, 0x0800, CRC(7e356dc5) SHA1(71d34fa39ff0b7d0fa6d32ba2b9dc0006a03d1bb) )
ROM_END

ROM_START( seabattla ) // this was a very different looking PCB (bootleg called armada maybe?) most parts had been stripped
	ROM_REGION( 0x8000, "maincpu", 0 )
	ROM_LOAD( "program roms",      0x0000, 0x0400, NO_DUMP )

	ROM_REGION( 0xc00, "gfx1", 0 ) // probably the same as above without the blank data at the start
	ROM_LOAD( "armadared.ic26",      0x0000, 0x0400, CRC(b588f509) SHA1(073f9dc584aba1351969ef597cd80a0037938dfb) )
	ROM_LOAD( "armadagreen.ic25",    0x0400, 0x0400, CRC(3cc861c9) SHA1(d9159ee045cc0994f468035ae28cd8b79b5985ee) )
	ROM_LOAD( "armadablu.ic24",      0x0800, 0x0400, CRC(3689e530) SHA1(b30ab0d5ddc9b296437aa1bc2887f1416eb69f9c) )

	ROM_REGION( 0x0800, "gfx2", 0 )
	ROM_LOAD( "greenobj.ic38",     0x0000, 0x0800, CRC(81a9a741) SHA1(b2725c320a232d4abf6e6fc58ccf6a5edb8dd9a0) )

	ROM_REGION( 0x0800, "gfx3", 0 )
	ROM_LOAD( "seawawe.ic9",     0x0000, 0x0800, CRC(7e356dc5) SHA1(71d34fa39ff0b7d0fa6d32ba2b9dc0006a03d1bb) ) // identical to above set
ROM_END

GAME( 1980, seabattl,  0,               seabattl, seabattl, driver_device, 0, ROT0,  "Zaccaria", "Sea Battle (set 1)",                    GAME_NO_SOUND | GAME_NOT_WORKING )
GAME( 1980, seabattla, seabattl,        seabattl, seabattl, driver_device, 0, ROT0,  "Zaccaria", "Sea Battle (set 2)",                    GAME_NO_SOUND | GAME_NOT_WORKING )
