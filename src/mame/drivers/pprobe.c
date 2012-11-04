/*

Planet Probe -  unknown manufacture  -  Copyright 1985
Game probably programmed by same people behind some Kyugo/Orca/Komax games (see hiscore table, Gyrodine pinout, clocks etc.)

Upper board marked DVL/A-V
Bottom Bord DVL/B-V

The pcb seems a bootleg/prototype:
On the upper board there are some pads for jumpers , some empty spaces left unpopulated for additional TTLs and an XTAL.
All 5 sockets for 2732 eproms were modified to accept 2764 eproms.
The AY8910 pin 26 (TEST 2) is grounded with a flying wire

Upper board chips:
5x 2764 eproms
1x 2128 static ram (2k ram)
2x z80B
1x AY8910
2x 8 positions dipswitches

Bottom Board chips:
5x 2764 eproms
2x 2128 static ram (2kx8 ram)
4x 93422 DRAM (256x4 dram)
1x 6301 PROM (probably used for background ?)
3x 82s129 Colour PROMS (connected to resistors) 

Clocks measured:

Main XTAL 18.432mhz
2x z80 : 18.432 / 6 
AY8910 : 18.432 / 12
Vsync : 60.58hz

*/



#include "emu.h"
#include "cpu/z80/z80.h"
#include "sound/ay8910.h"

class pprobe_state : public driver_device
{
public:
	pprobe_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)	
	{ }

	DECLARE_DRIVER_INIT(pprobe);
	UINT32 screen_update_pprobe(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
};



UINT32 pprobe_state::screen_update_pprobe(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	return 0;
}


static ADDRESS_MAP_START( pprobe_main_map, AS_PROGRAM, 8, pprobe_state )
	AM_RANGE(0x0000, 0x7fff) AM_ROM
ADDRESS_MAP_END

static ADDRESS_MAP_START( pprobe_main_portmap, AS_IO, 8, pprobe_state )
//	ADDRESS_MAP_GLOBAL_MASK(0xff)
ADDRESS_MAP_END


static const gfx_layout fg_tilelayout =
{
	8,8,
	RGN_FRAC(1,1),
	2,
	{ 0, 4 },
	{ 0, 1, 2, 3, 8*8+0, 8*8+1, 8*8+2, 8*8+3 },
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8 },
	8*8*2
};

static const gfx_layout sp_tilelayout =
{
	16,16,
	RGN_FRAC(1,1),
	2,
	{ 0, 4 },
	{ 0, 1, 2, 3, 8*8+0, 8*8+1, 8*8+2, 8*8+3, 128+0, 128+1, 128+2, 128+3, 128+8*8+0, 128+8*8+1, 128+8*8+2, 128+8*8+3 },
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8, 256+0*8, 256+1*8, 256+2*8, 256+3*8, 256+4*8, 256+5*8, 256+6*8, 256+7*8 },
	8*8*8
};




static GFXDECODE_START( pprobe )
	GFXDECODE_ENTRY( "gfx1", 0, fg_tilelayout, 0, 64 )
	GFXDECODE_ENTRY( "gfx2", 0, fg_tilelayout, 0, 64 )
	GFXDECODE_ENTRY( "gfx3", 0, fg_tilelayout, 0, 64 )
	GFXDECODE_ENTRY( "gfx4", 0, sp_tilelayout, 0, 64 )
	GFXDECODE_ENTRY( "gfx5", 0, sp_tilelayout, 0, 64 )
GFXDECODE_END


static INPUT_PORTS_START( pprobe )
INPUT_PORTS_END


static MACHINE_CONFIG_START( pprobe, pprobe_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", Z80, XTAL_18_432MHz/6)	/* verified on pcb */
	MCFG_CPU_PROGRAM_MAP(pprobe_main_map)
	MCFG_CPU_IO_MAP(pprobe_main_portmap)

//	MCFG_CPU_ADD("subz80", Z80, XTAL_18_432MHz/6)	/* verified on pcb */

	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(60.58)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(2500) /* not accurate */)
	MCFG_SCREEN_SIZE(64*8, 32*8)
	MCFG_SCREEN_VISIBLE_AREA(0*8, 64*8-1, 0*8, 32*8-1)
	MCFG_SCREEN_UPDATE_DRIVER(pprobe_state, screen_update_pprobe)

	MCFG_GFXDECODE(pprobe)
	MCFG_PALETTE_LENGTH(256)

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")

	MCFG_SOUND_ADD("ay1", AY8910, XTAL_18_432MHz/12)  /* verified on pcb */
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.30)
MACHINE_CONFIG_END


ROM_START( pprobe )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "pb2.bin",   0x0000, 0x2000, CRC(a88592aa) SHA1(98e8e6233b85e678718f532708d57ec946b9fd88) )
	ROM_LOAD( "pb3.bin",   0x2000, 0x2000, CRC(e4e20f74) SHA1(53b4d0499127cca149a3dd03af4f05de552cff57) )
	ROM_LOAD( "pb4.bin",   0x4000, 0x2000, CRC(4e40e3fe) SHA1(ccb3c5828508efc9f0df44bf3408e807d5ef58a0) )
	ROM_LOAD( "pb5.bin",   0x6000, 0x2000, CRC(b26ff0fd) SHA1(c64966ee91557f8982b9b7fd17306508228f1e15) )

	ROM_REGION( 0x10000, "subz80", 0 )
	ROM_LOAD( "pb1.bin",   0x0000, 0x2000, BAD_DUMP CRC(6e340426) SHA1(e6fc227d22f9b769955127d308dfb96ed85ba40c) ) // corrupt :-(

	ROM_REGION( 0x02000, "gfx1", 0 ) // bg tiles
	ROM_LOAD( "pb6.bin",  0x0000, 0x2000, CRC(ff309239) SHA1(4e52833fafd54d4502ad09091fbfb1a8a2ff8828) )
	
	ROM_REGION( 0x02000, "gfx2", 0 ) // bg tiles
	ROM_LOAD( "pb7.bin",  0x0000, 0x2000, BAD_DUMP CRC(1defb6fc) SHA1(f0d57cf8a92c29fef52c8437d0be6edecaf9c5c9) ) // some bad bytes

	ROM_REGION( 0x02000, "gfx3", 0 ) // tx tiles
	ROM_LOAD( "pb9.bin",  0x0000, 0x2000, CRC(82294dd6) SHA1(24b8eac3d476d4a4d91dd169e26bd075b0d1bf45) )

	ROM_REGION( 0x02000, "gfx4", 0 ) // sprites
	ROM_LOAD( "pb8.bin",  0x0000, 0x2000, CRC(8d809e45) SHA1(70f99626acdceaadbe03de49bcf778266ddff893) )

	ROM_REGION( 0x02000, "gfx5", 0 ) // sprites
	ROM_LOAD( "pb10.bin", 0x0000, 0x2000, CRC(895f9dd3) SHA1(919861482598aa35a9ad476da19f9efa30904cd4) )

	ROM_REGION( 0x0320, "proms", 0 )
	ROM_LOAD( "82s129.r",   0x0000, 0x0100, NO_DUMP ) /* red */
	ROM_LOAD( "82s129.g",   0x0100, 0x0100, NO_DUMP ) /* green */
	ROM_LOAD( "82s129.b",   0x0200, 0x0100, NO_DUMP ) /* blue */
	ROM_LOAD( "6301.prom",   0x0300, 0x0020, NO_DUMP ) /* unk */
ROM_END

DRIVER_INIT_MEMBER(pprobe_state,pprobe)
{
}

GAME( 1984, pprobe,  0,        pprobe,  pprobe, pprobe_state,   pprobe, ROT270, "Kyugo?", "Planet Probe", GAME_IS_SKELETON )
