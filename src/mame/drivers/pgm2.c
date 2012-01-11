

/* PGM 2 hardware.

 Motherboard is bare bones stuff, probably only contains the video processor, no ROMs.

 Makes use of internal ROM ASICS, newer than those found on the best protected PGM games.  Games actually boot to a warning screen even if you remove all program roms!

 Encrypted.

 I'm guessing it's ARM based, but not sure.

*/

#include "emu.h"
#include "cpu/arm7/arm7.h"
#include "cpu/arm7/arm7core.h"
#include "sound/ymz770.h"

class pgm2_state : public driver_device
{
public:
	pgm2_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		{ }

};

static ADDRESS_MAP_START( pgm2_map, AS_PROGRAM, 32 )
	AM_RANGE(0x00000000, 0x00003fff) AM_ROM
ADDRESS_MAP_END

static INPUT_PORTS_START( pgm2 )
INPUT_PORTS_END

static SCREEN_UPDATE(pgm2)
{
	return 0;
}

static SCREEN_EOF(pgm2)
{

}

static VIDEO_START(pgm2)
{

}

static MACHINE_START( pgm2 )
{

}

static MACHINE_RESET( pgm2 )
{

}

static const gfx_layout tiles8x8_layout =
{
	8,8,
	RGN_FRAC(1,1),
	4,
	{ 0, 1, 2, 3 },
	{ 4, 0, 12, 8, 20, 16, 28, 24 },
	{ 0*32, 1*32, 2*32, 3*32, 4*32, 5*32, 6*32, 7*32 },
	32*8
};

/* slightly odd decode, and ends up with multiple letters in a single 32x32 tile, can probably specify corners, or needs an address line swap before decode */
/* actually 7bpp? roms report fixed bits, but should be good dumps */
static const gfx_layout tiles32x32x8_layout =
{
	32,32,
	RGN_FRAC(1,1),
	8,
	{ 0, 1, 2, 3, 4, 5, 6, 7 },
	{ 0*8, 2*8, 1*8, 3*8, 4*8, 6*8, 5*8, 7*8, 8*8, 10*8, 9*8, 11*8, 12*8, 14*8, 13*8, 15*8,
	 16*8, 18*8, 17*8, 19*8, 20*8, 22*8, 21*8, 23*8, 24*8, 26*8, 25*8, 27*8, 28*8, 30*8, 29*8, 31*8 },
	{ 0*256, 1*256, 2*256, 3*256, 4*256, 5*256, 6*256, 7*256, 8*256, 9*256, 10*256, 11*256, 12*256, 13*256, 14*256, 15*256,
	 16*256, 17*256, 18*256, 19*256, 20*256, 21*256, 22*256, 23*256, 24*256, 25*256, 26*256, 27*256, 28*256, 29*256, 30*256, 31*256
	},
	256*32
};

/* sprites aren't tile based, this is variable width 1bpp data, colour data is almost certainly in sprites b */
/* there don't seem to be any indexes into the colour data, probably provided by the program, or the colour data references the bitmaps (reverse of PGM) */
static const gfx_layout tiles32x8x1_layout =
{
	32,8,
	RGN_FRAC(1,1),
	1,
	{ 0 },
	{ 0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,21,22,23,24,25,26,27,28,29,30,31 },
	{ 0*32,1*32,2*32,3*32,4*32,5*32,6*32,7*32 },
	8*32
};




static GFXDECODE_START( pgm2 )
	GFXDECODE_ENTRY( "tiles", 0, tiles8x8_layout, 0, 16 )
	GFXDECODE_ENTRY( "bgtile", 0, tiles32x32x8_layout, 0, 16 )
	// not tile based
	GFXDECODE_ENTRY( "spritesa", 0, tiles32x8x1_layout, 0, 16 )
	GFXDECODE_ENTRY( "spritesb", 0, tiles32x8x1_layout, 0, 16 )
GFXDECODE_END



static MACHINE_CONFIG_START( pgm2, pgm2_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", ARM7, 20000000)	// ?? unknown CPU, has internal ROM.
	MCFG_CPU_PROGRAM_MAP(pgm2_map)
//	MCFG_DEVICE_DISABLE()

	MCFG_MACHINE_START( pgm2 )
	MCFG_MACHINE_RESET( pgm2 )

	MCFG_GFXDECODE(pgm2)

	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MCFG_SCREEN_FORMAT(BITMAP_FORMAT_INDEXED16)
	MCFG_SCREEN_SIZE(64*8, 64*8)
	MCFG_SCREEN_VISIBLE_AREA(0*8, 56*8-1, 0*8, 28*8-1)
	MCFG_SCREEN_UPDATE(pgm2)
	MCFG_SCREEN_EOF(pgm2)

	MCFG_PALETTE_LENGTH(0x1000)

	MCFG_VIDEO_START(pgm2)

	MCFG_SPEAKER_STANDARD_STEREO("lspeaker", "rspeaker")
    MCFG_YMZ770_ADD("ymz770", 16384000)
	MCFG_SOUND_ROUTE(0, "lspeaker", 1.0)
	MCFG_SOUND_ROUTE(1, "rspeaker", 1.0)
MACHINE_CONFIG_END

ROM_START( orleg2 )
	ROM_REGION( 0x800000, "maincpu", 0 ) /* External Code (Internal is missing) */
	ROM_LOAD16_WORD_SWAP( "xyj2_v104cn.u7",  0x000000, 0x800000, CRC(7c24a4f5) SHA1(3cd9f9264ef2aad0869afdf096e88eb8d74b2570) )

	ROM_REGION( 0x0200000, "tiles", ROMREGION_ERASEFF )
	ROM_LOAD( "ig-a.u4",   0x000000, 0x0200000, CRC(fa444c32) SHA1(31e5e3efa92d52bf9ab97a0ece51e3b77f52ce8a) )

	ROM_REGION( 0x1000000, "bgtile", 0 )
	ROM_LOAD16_BYTE( "ig-a.u35",  0x000000, 0x0800000, CRC(083a8315) SHA1(0dba25e132fbb12faa59ced648c27b881dc73478) )
	ROM_LOAD16_BYTE( "ig-a.u36",  0x000001, 0x0800000, CRC(e197221d) SHA1(5574b1e3da4b202db725be906dd868edc2fd4634) )
	
	ROM_REGION( 0x2000000, "spritesa", 0 ) // 1bpp sprite mask data
	ROM_LOAD32_WORD( "ig-a.u12",  0x000000, 0x1000000, CRC(113a331c) SHA1(ee6b31bb2b052cc8799573de0d2f0a83f0ab4f6a) )
	ROM_LOAD32_WORD( "ig-a.u16",  0x000002, 0x1000000, CRC(fbf411c8) SHA1(5089b5cc9bbf6496ef1367c6255e63e9ab895117) )

	ROM_REGION( 0x4000000, "spritesb", 0 ) // sprite colour data?
	ROM_LOAD16_BYTE( "ig-a.u18",  0x000000, 0x2000000, CRC(43501fa6) SHA1(58ccce6d393964b771fec3f5c583e3ede57482a3) )
	ROM_LOAD16_BYTE( "ig-a.u26",  0x000001, 0x2000000, CRC(7051d020) SHA1(3d9b24c6fda4c9699bb9f00742e0888059b623e1) )

	ROM_REGION( 0x1000000, "ymz770", ROMREGION_ERASEFF ) /* ymz770? */
	ROM_LOAD16_WORD_SWAP( "ig-a.u2",   0x000000, 0x1000000, CRC(8250688c) SHA1(d2488477afc528aeee96826065deba2bce4f0a7d) )
ROM_END

ROM_START( orleg2o )
	ROM_REGION( 0x800000, "maincpu", 0 ) /* External Code (Internal is missing) */
	ROM_LOAD16_WORD_SWAP( "xyj2_v103cn.u7",  0x000000, 0x800000, CRC(21c1fae8) SHA1(36eeb7a5e8dc8ee7c834f3ff1173c28cf6c2f1a3) )

	ROM_REGION( 0x0200000, "tiles", ROMREGION_ERASEFF )
	ROM_LOAD( "ig-a.u4",   0x000000, 0x0200000, CRC(fa444c32) SHA1(31e5e3efa92d52bf9ab97a0ece51e3b77f52ce8a) )

	ROM_REGION( 0x1000000, "bgtile", 0 )
	ROM_LOAD16_BYTE( "ig-a.u35",  0x000000, 0x0800000, CRC(083a8315) SHA1(0dba25e132fbb12faa59ced648c27b881dc73478) )
	ROM_LOAD16_BYTE( "ig-a.u36",  0x000001, 0x0800000, CRC(e197221d) SHA1(5574b1e3da4b202db725be906dd868edc2fd4634) )
	
	ROM_REGION( 0x2000000, "spritesa", 0 ) // 1bpp sprite mask data
	ROM_LOAD32_WORD( "ig-a.u12",  0x000000, 0x1000000, CRC(113a331c) SHA1(ee6b31bb2b052cc8799573de0d2f0a83f0ab4f6a) )
	ROM_LOAD32_WORD( "ig-a.u16",  0x000002, 0x1000000, CRC(fbf411c8) SHA1(5089b5cc9bbf6496ef1367c6255e63e9ab895117) )

	ROM_REGION( 0x4000000, "spritesb", 0 ) // sprite colour data?
	ROM_LOAD16_BYTE( "ig-a.u18",  0x000000, 0x2000000, CRC(43501fa6) SHA1(58ccce6d393964b771fec3f5c583e3ede57482a3) )
	ROM_LOAD16_BYTE( "ig-a.u26",  0x000001, 0x2000000, CRC(7051d020) SHA1(3d9b24c6fda4c9699bb9f00742e0888059b623e1) )

	ROM_REGION( 0x1000000, "ymz770", ROMREGION_ERASEFF ) /* ymz770? */
	ROM_LOAD16_WORD_SWAP( "ig-a.u2",   0x000000, 0x1000000, CRC(8250688c) SHA1(d2488477afc528aeee96826065deba2bce4f0a7d) )
ROM_END

ROM_START( kov2nl )
	ROM_REGION( 0x800000, "maincpu", 0 ) /* External Code (Internal is missing) */
	ROM_LOAD16_WORD_SWAP( "gsyx_v302cn.u7",  0x000000, 0x800000, CRC(b19cf540) SHA1(25da5804bbfd7ef2cdf5cc5aabaa803d18b98929) )

	ROM_REGION( 0x0200000, "tiles", ROMREGION_ERASEFF )
	ROM_LOAD( "ig-a3.u4",   0x000000, 0x0200000, CRC(214530ff) SHA1(4231a02054b0345392a077042b95779fd45d6c22) )

	ROM_REGION( 0x1000000, "bgtile", 0 )
	ROM_LOAD16_BYTE( "ig-a3.u35",  0x000000, 0x0800000, CRC(2d46b1f6) SHA1(ea8c805eda6292e86a642e9633d8fee7054d10b1) )
	ROM_LOAD16_BYTE( "ig-a3.u36",  0x000001, 0x0800000, CRC(df710c36) SHA1(f826c3f496c4f17b46d18af1d8e02cac7b7027ac) )
	
	ROM_REGION( 0x2000000, "spritesa", 0 ) // 1bpp sprite mask data
	ROM_LOAD32_WORD( "ig-a3.u12",  0x000000, 0x1000000, CRC(0bf63836) SHA1(b8e4f1951f8074b475b795bd7840c5a375b6f5ef) )
	ROM_LOAD32_WORD( "ig-a3.u16",  0x000002, 0x1000000, CRC(4a378542) SHA1(5d06a8a8796285a786ebb690c34610f923ef5570) )

	ROM_REGION( 0x4000000, "spritesb", 0 ) // sprite colour data?
	ROM_LOAD16_BYTE( "ig-a3.u18",  0x000000, 0x2000000, CRC(8d923e1f) SHA1(14371cf385dd8857017d3111cd4710f4291b1ae2) )
	ROM_LOAD16_BYTE( "ig-a3.u26",  0x000001, 0x2000000, CRC(5b6fbf3f) SHA1(d1f52e230b91ee6cde939d7c2b74da7fd6527e73) )

	ROM_REGION( 0x2000000, "ymz770", ROMREGION_ERASEFF ) /* ymz770? */
	ROM_LOAD16_WORD_SWAP( "ig-a3.u37",   0x000000, 0x2000000, CRC(45cdf422) SHA1(8005d284bcee73cff37a147fcd1c3e9f039a7203) )
ROM_END

ROM_START( kov2nlo )
	ROM_REGION( 0x800000, "maincpu", 0 ) /* External Code (Internal is missing) */
	ROM_LOAD16_WORD_SWAP( "gsyx_v301cn.u7",  0x000000, 0x800000, CRC(c4595c2c) SHA1(09e379556ef76f81a63664f46d3f1415b315f384) )

	ROM_REGION( 0x0200000, "tiles", ROMREGION_ERASEFF )
	ROM_LOAD( "ig-a3.u4",   0x000000, 0x0200000, CRC(214530ff) SHA1(4231a02054b0345392a077042b95779fd45d6c22) )

	ROM_REGION( 0x1000000, "bgtile", 0 )
	ROM_LOAD16_BYTE( "ig-a3.u35",  0x000000, 0x0800000, CRC(2d46b1f6) SHA1(ea8c805eda6292e86a642e9633d8fee7054d10b1) )
	ROM_LOAD16_BYTE( "ig-a3.u36",  0x000001, 0x0800000, CRC(df710c36) SHA1(f826c3f496c4f17b46d18af1d8e02cac7b7027ac) )
	
	ROM_REGION( 0x2000000, "spritesa", 0 ) // 1bpp sprite mask data
	ROM_LOAD32_WORD( "ig-a3.u12",  0x000000, 0x1000000, CRC(0bf63836) SHA1(b8e4f1951f8074b475b795bd7840c5a375b6f5ef) )
	ROM_LOAD32_WORD( "ig-a3.u16",  0x000002, 0x1000000, CRC(4a378542) SHA1(5d06a8a8796285a786ebb690c34610f923ef5570) )

	ROM_REGION( 0x4000000, "spritesb", 0 ) // sprite colour data?
	ROM_LOAD16_BYTE( "ig-a3.u18",  0x000000, 0x2000000, CRC(8d923e1f) SHA1(14371cf385dd8857017d3111cd4710f4291b1ae2) )
	ROM_LOAD16_BYTE( "ig-a3.u26",  0x000001, 0x2000000, CRC(5b6fbf3f) SHA1(d1f52e230b91ee6cde939d7c2b74da7fd6527e73) )

	ROM_REGION( 0x2000000, "ymz770", ROMREGION_ERASEFF ) /* ymz770? */
	ROM_LOAD16_WORD_SWAP( "ig-a3.u37",   0x000000, 0x2000000, CRC(45cdf422) SHA1(8005d284bcee73cff37a147fcd1c3e9f039a7203) )
ROM_END

static void iga_u16_decode(UINT16 *rom, int len, int ixor)
{
	int i;

	for (i = 1; i < len / 2; i+=2)
	{
		UINT16 x = ixor; 

		if ( (i>>1) & 0x000001) x ^= 0x0010;
		if ( (i>>1) & 0x000002) x ^= 0x2004;
		if ( (i>>1) & 0x000004) x ^= 0x0801;
		if ( (i>>1) & 0x000008) x ^= 0x0300;
		if ( (i>>1) & 0x000010) x ^= 0x0080;
		if ( (i>>1) & 0x000020) x ^= 0x0020;
		if ( (i>>1) & 0x000040) x ^= 0x4008;
		if ( (i>>1) & 0x000080) x ^= 0x1002;
		if ( (i>>1) & 0x000100) x ^= 0x0400;
		if ( (i>>1) & 0x000200) x ^= 0x0040;
		if ( (i>>1) & 0x000400) x ^= 0x8000;

		rom[i] ^= x;
		rom[i] = BITSWAP16(rom[i], 8,9,10,11,12,13,14,15,0,1,2,3,4,5,6,7);
	}
}

static void iga_u12_decode(UINT16* rom, int len, int ixor)
{
	int i;

	for (i = 0; i < len / 2; i+=2)
	{
		UINT16 x = ixor; 

		if ( (i>>1) & 0x000001) x ^= 0x9004;
		if ( (i>>1) & 0x000002) x ^= 0x0028;
		if ( (i>>1) & 0x000004) x ^= 0x0182;
		if ( (i>>1) & 0x000008) x ^= 0x0010;
		if ( (i>>1) & 0x000010) x ^= 0x2040;
		if ( (i>>1) & 0x000020) x ^= 0x0801;
		if ( (i>>1) & 0x000040) x ^= 0x0000;
		if ( (i>>1) & 0x000080) x ^= 0x0000;
		if ( (i>>1) & 0x000100) x ^= 0x4000;
		if ( (i>>1) & 0x000200) x ^= 0x0600;
		if ( (i>>1) & 0x000400) x ^= 0x0000;

		rom[i] ^= x;
		rom[i] = BITSWAP16(rom[i], 8,9,10,11,12,13,14,15,0,1,2,3,4,5,6,7);
	}
}

static DRIVER_INIT( orleg2 )
{
	UINT16 *src = (UINT16 *)machine.region("spritesa")->base();

	iga_u12_decode(src, 0x2000000, 0x4761);
	iga_u16_decode(src, 0x2000000, 0xc79f);
}

static DRIVER_INIT( kov2nl )
{
	UINT16 *src = (UINT16 *)machine.region("spritesa")->base();

	iga_u12_decode(src, 0x2000000, 0xa193);
	iga_u16_decode(src, 0x2000000, 0xb780);
}


/* PGM2 */
GAME( 2007, orleg2,       0,         pgm2,    pgm2,     orleg2,       ROT0, "IGS", "Oriental Legend 2 (V104, China)", GAME_IS_SKELETON )
GAME( 2007, orleg2o,      orleg2,    pgm2,    pgm2,     orleg2,       ROT0, "IGS", "Oriental Legend 2 (V103, China)", GAME_IS_SKELETON )

GAME( 2008, kov2nl,       0,         pgm2,    pgm2,     kov2nl,       ROT0, "IGS", "Knights of Valour 2 New Legend (V302, China)", GAME_IS_SKELETON )
GAME( 2008, kov2nlo,      kov2nl,    pgm2,    pgm2,     kov2nl,       ROT0, "IGS", "Knights of Valour 2 New Legend (V301, China)", GAME_IS_SKELETON )
