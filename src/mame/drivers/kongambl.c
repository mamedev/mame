/*
  Konami Gambling Games

  68000 (?) hardware
  unknown custom chips

*/

#include "emu.h"
#include "cpu/m68000/m68000.h"

static ADDRESS_MAP_START( kongambl_map, ADDRESS_SPACE_PROGRAM, 16 )
	AM_RANGE(0x000000, 0x007fff) AM_ROM
	AM_RANGE(0x008000, 0x01ffff) AM_ROM // tests this area even if the 'bios' isn't this big..

	AM_RANGE(0x100000, 0x10ffff) AM_RAM
ADDRESS_MAP_END


static INPUT_PORTS_START( kongambl )
INPUT_PORTS_END


static const gfx_layout tiles16x16_layout =
{
	16,16,
	RGN_FRAC(1,2),
	8,
	{ 0,8,16,24, RGN_FRAC(1,2)+0,RGN_FRAC(1,2)+8,RGN_FRAC(1,2)+16,RGN_FRAC(1,2)+24 },
	{ 0,1,2,3,4,5,6,7, 32,33,34,35,36,37,38,39 },
	{ 0*64, 1*64, 2*64, 3*64, 4*64, 5*64, 6*64, 7*64, 8*64, 9*64, 10*64, 11*64, 12*64, 13*64, 14*64, 15*64 },
	16*64
};

static const gfx_layout tiles8x8_layout =
{
	8,8,
	RGN_FRAC(1,1),
	8,
	{ 0,8,16,24,32,40,48,56 },
	{ 0,1,2,3,4,5,6,7 },
	{ 0*64, 1*64, 2*64, 3*64, 4*64, 5*64, 6*64, 7*64},
	8*64
};

static GFXDECODE_START( kongambl )
	GFXDECODE_ENTRY( "gfx1", 0, tiles16x16_layout, 0, 1 )
	GFXDECODE_ENTRY( "gfx2", 0, tiles8x8_layout, 0, 1 )
GFXDECODE_END


static VIDEO_START(kongambl)
{
	int i;

	for (i=0;i<256;i++)
	{
		palette_set_color(machine,i,MAKE_RGB(i,i,i));
	}

}

static VIDEO_UPDATE(kongambl)
{
	return 0;
}

static MACHINE_DRIVER_START( kongambl )
	MDRV_CPU_ADD("maincpu", M68000, 16000000)	 // ?
	MDRV_CPU_PROGRAM_MAP(kongambl_map)
	//MDRV_CPU_VBLANK_INT("screen", irq1_line_hold)

	MDRV_GFXDECODE(kongambl)

	MDRV_SCREEN_ADD("screen", RASTER)
	MDRV_SCREEN_REFRESH_RATE(60)
	MDRV_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MDRV_SCREEN_FORMAT(BITMAP_FORMAT_INDEXED16)
	MDRV_SCREEN_SIZE(64*8, 32*8)
	MDRV_SCREEN_VISIBLE_AREA(0*8, 64*8-1, 0*8, 32*8-1)

	MDRV_PALETTE_LENGTH(0x200)

	MDRV_VIDEO_START(kongambl)
	MDRV_VIDEO_UPDATE(kongambl)

	MDRV_SPEAKER_STANDARD_STEREO("lspeaker", "rspeaker")
MACHINE_DRIVER_END


ROM_START( kingtut )
	ROM_REGION( 0x40000, "maincpu", 0 ) /* 68000 Code */
	ROM_LOAD16_WORD_SWAP( "n12prog_ifu.41", 0x00000, 0x08000, CRC(dbb8a7e8) SHA1(9662b34e9332385d20e17ee1c92fd91935d4c3b2) ) // some kind of bios? same on both games

	ROM_REGION( 0x80000, "prog", 0 ) // main program? (how / where does it map?)
	ROM_LOAD16_BYTE( "kitp1b37_l.02", 0x000000, 0x40000, CRC(95c6da28) SHA1(3ef33f5d0748c80be82d33c21f0f8bb71909884e) )
	ROM_LOAD16_BYTE( "kitp1b37_h.01", 0x000001, 0x40000, CRC(16709625) SHA1(6b818a85724f87fed23a26978dd26b079f814134) )

	ROM_REGION( 0x200000, "gfx1", 0 ) // 16x16x8 sprites
	ROM_LOAD16_BYTE( "kit11hh1_obj.11", 0x000000, 0x80000, CRC(a64d2382) SHA1(bb745a26ef6c076f3aa3ec476589a95915b359ed) )
	ROM_LOAD16_BYTE( "kit11hm1_obj.13", 0x000001, 0x80000, CRC(21cc4e40) SHA1(9e3735fc8cd53f7e831dc76697911216bd8bbc70) )
	ROM_LOAD16_BYTE( "kit11ll1_obj.17", 0x100000, 0x80000, CRC(a19338b8) SHA1(1aa68596e5bf493cb360495f1174dc1323086ad2) )
	ROM_LOAD16_BYTE( "kit11lm1_obj.15", 0x100001, 0x80000, CRC(1aea3f4d) SHA1(52fd1a7ffeeb3acce176ad3812a2ca146e02c324) )

	ROM_REGION( 0x100000, "gfx2", 0 ) // 8x8x8 tiles
	ROM_LOAD16_BYTE( "kit11_l1_vrm.21", 0x000000, 0x80000, CRC(431eb89f) SHA1(377c96f615b4b76314aeecad4e868edb66c72f33) )
	ROM_LOAD16_BYTE( "kit11_h1_vrm.23", 0x000001, 0x80000, CRC(7aa2f1bc) SHA1(d8aead9dedcc83d3dc574122103aaa2074011197) )

	ROM_REGION( 0x100000, "snd", 0 )
	ROM_LOAD( "snd12sd1_snd.31", 0x000000, 0x80000, CRC(f4121baa) SHA1(723c6d96ecef5ef510d085f443d44bad07aa19e5) )
	ROM_LOAD( "kit11sd2_snd.32", 0x080000, 0x80000, CRC(647c6e2e) SHA1(e013239a73553e2993adabeda103f5b1cfee0f6c) )
ROM_END

ROM_START( moneybnk )
	ROM_REGION( 0x40000, "maincpu", 0 ) /* 68000 Code */
	ROM_LOAD16_WORD_SWAP( "n12prog_ifu.41", 0x00000, 0x08000, CRC(dbb8a7e8) SHA1(9662b34e9332385d20e17ee1c92fd91935d4c3b2) ) // some kind of bios? same on both games

	ROM_REGION( 0x80000, "prog", 0 ) // main program? (how / where does it map?)
	ROM_LOAD16_BYTE( "mobn6l29_l.02", 0x000000, 0x40000, CRC(9cd2754a) SHA1(2eb695cb4abab4a448711b8acf3f5b1bb169eb6f) )
	ROM_LOAD16_BYTE( "mobn6l29_h.01", 0x000001, 0x40000, CRC(952c376b) SHA1(0fc0499f5570b920c600ddd6a15751d72345c83e) )

	ROM_REGION( 0x200000, "gfx1", 0 ) // 16x16x8 sprites
	ROM_LOAD16_BYTE( "mob11hh1_obj.11", 0x000000, 0x80000, CRC(fc2ebc0a) SHA1(7c61d05ae1644a2aafc2f81725658b29ce69a091) )
	ROM_LOAD16_BYTE( "mob11hm1_obj.13", 0x000001, 0x80000, CRC(6f84c287) SHA1(edccefa96d97c6f67a9cd02f70cf61385d70daae) )
	ROM_LOAD16_BYTE( "mob11ll1_obj.17", 0x100000, 0x80000, CRC(5c5959a3) SHA1(1eea6bf4c34aa05f45b2737eb6035f2762277cfb) )
	ROM_LOAD16_BYTE( "mob11lm1_obj.15", 0x100001, 0x80000, CRC(0b0e4e9b) SHA1(cbbbde7470f96e9f93fa848371e19ebfeea7fe4d) )

	ROM_REGION( 0x100000, "gfx2", 0 ) // 8x8x8 tiles
	ROM_LOAD16_BYTE( "mob11_l1_vrm.21", 0x000000, 0x80000, CRC(926fbd3b) SHA1(4f85ea63faff1508d5abf0ca0ebd16e802f8f45c) )
	ROM_LOAD16_BYTE( "mob11_h1_vrm.23", 0x000001, 0x80000, CRC(a119feaa) SHA1(567e319dfddb9ec04b9302af782e9baccab4f5a6) )

	ROM_REGION( 0x100000, "snd", 0 )
	ROM_LOAD( "snd11sd1_snd.31", 0x000000, 0x80000, CRC(cce53e79) SHA1(970507fcef309c6c81f7e1a8e90afa64f3f6e2ae) )
	ROM_LOAD( "mob11sd2_snd.32", 0x080000, 0x80000, CRC(71ecc441) SHA1(4c94fa3a4ab872b2b841d98b73da89eaec0f46f0) )
ROM_END


GAME( 199?, kingtut,    0,        kongambl,    kongambl,    0, ROT0,  "Konami", "King Tut (Australia)", GAME_NOT_WORKING | GAME_NO_SOUND )
GAME( 199?, moneybnk,   0,        kongambl,    kongambl,    0, ROT0,  "Konami", "Money Bank (Australia)", GAME_NOT_WORKING | GAME_NO_SOUND )
