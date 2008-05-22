/*
G4001
Diamond Derby - Electrocoin on an SNK board

this seems to be a gambling game

SNK/Electrocoin 1987

--------------------------------------------
G4001UP01

SWA SWB   C1        DD1
                    DD2

                6116              DD4
                          Z80     DD5
                                  DD6
                          DD3     6116
             8910  Z80    6116    6116

---------------------------------------------
G4001UP02

DD7  DD11  DD15           K1
DD8  DD12  DD16                   DD19
DD9  DD13  DD17                   DD20
DD10 DD14  DD18     H5            DD21


                                  DD22
                                  DD23
 2114                  2148 2148
 2114              H10


*/

#include "driver.h"

static UINT8* dderby_vid;

static READ8_HANDLER( dderby_random_reader )
{
	return mame_rand(machine);
}

static ADDRESS_MAP_START( memmap, ADDRESS_SPACE_PROGRAM, 8 )
	AM_RANGE(0x0000, 0x5fff) AM_ROM
	AM_RANGE(0x8000, 0x8fff) AM_RAM
	AM_RANGE(0xC000, 0xC007) AM_READ(dderby_random_reader)
	AM_RANGE(0xC802, 0xC802) AM_READ(dderby_random_reader)
	AM_RANGE(0xC803, 0xC803) AM_READ(dderby_random_reader)
	AM_RANGE(0xCA01, 0xCA01) AM_WRITE(SMH_NOP)
	AM_RANGE(0xD000, 0xD7ff) AM_RAM AM_BASE(&dderby_vid)
ADDRESS_MAP_END


static INPUT_PORTS_START( dderby )
	PORT_START	/* 8bit */
	PORT_DIPNAME( 0x01, 0x01, DEF_STR( Unknown ) )
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

static const gfx_layout tiles8x8_layout =
{
	8,8,
	RGN_FRAC(1,2),
	4,
	{ 0,4,RGN_FRAC(1,2)+0,RGN_FRAC(1,2)+4 },
	{ 0, 1, 2, 3,8,9,10,11 },
	{ 0*16, 1*16, 2*16, 3*16, 4*16, 5*16, 6*16, 7*16 },
	16*8
};

static const gfx_layout tiles8x8_layout2 =
{
	8,8,
	RGN_FRAC(1,1),
	2,
	{ 0,8 },
	{ 0, 1, 2, 3,4,5,6,7},
	{ 0*16, 1*16, 2*16, 3*16, 4*16, 5*16, 6*16, 7*16 },
	16*8
};

static const UINT32 tiles8x64_layout_yoffset[64] =
{
	0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8,8*8,9*8,10*8,11*8,12*8,13*8,14*8,15*8,16*8,17*8,18*8,19*8,20*8,21*8,22*8,23*8,24*8,25*8,26*8,27*8,28*8,29*8,30*8,31*8,
	32*8,33*8,34*8,35*8,36*8,37*8,38*8,39*8,40*8,41*8,42*8,43*8,44*8,45*8,46*8,47*8,48*8,49*8,50*8,51*8,52*8,53*8,54*8,55*8,56*8,57*8,58*8,59*8,60*8,61*8,62*8,63*8
};

/* i'm not convinced they will be used like this, might just be used as 8x8 tiles .. */
static const gfx_layout tiles8x64_layout =
{
	8,64,
	RGN_FRAC(1,3),
	3,
	{ 0,RGN_FRAC(1,3),RGN_FRAC(2,3) },
	{ 0, 1, 2, 3, 4, 5, 6, 7 },
	EXTENDED_YOFFS,
	64*8,
	NULL,
	tiles8x64_layout_yoffset
};

static GFXDECODE_START( dmndrby )
	GFXDECODE_ENTRY( REGION_GFX1, 0, tiles8x8_layout, 0, 16 )
	GFXDECODE_ENTRY( REGION_GFX2, 0, tiles8x64_layout, 0, 16 )
	GFXDECODE_ENTRY( REGION_GFX3, 0, tiles8x8_layout2, 0, 16 )

GFXDECODE_END

static VIDEO_START(dderby)
{
}

static VIDEO_UPDATE(dderby)
{
	int x,y,count;
	const gfx_element *gfx = screen->machine->gfx[0];

	fillbitmap(bitmap, get_black_pen(screen->machine), cliprect);

	count=0;
	for (y=0;y<32;y++)
	{
		for(x=0;x<32;x++)
		{
			int tileno,bank;

			tileno=dderby_vid[count];
			bank=dderby_vid[count+0x400]&0x20;

			if (bank) tileno+=0x100;

			drawgfx(bitmap,gfx,tileno,0,0,0,x*8,y*8,cliprect,TRANSPARENCY_PEN,0);
			count++;
		}
	}



	return 0;
}

static INTERRUPT_GEN( dderby_interrupt )
{
//  cpunum_set_input_line_and_vector(machine, 0, 0, HOLD_LINE, 0x8);  // almost certainly wrong?
//  cpunum_set_input_line_and_vector(machine, 0, 0, HOLD_LINE, 0x10); // almost certainly wrong?
//  cpunum_set_input_line_and_vector(machine, 0, 0, HOLD_LINE, 0x18); // almost certainly wrong?
//  cpunum_set_input_line_and_vector(machine, 0, 0, HOLD_LINE, 0x20); // almost certainly wrong?
}

static MACHINE_DRIVER_START( dderby )
	/* basic machine hardware */
	MDRV_CPU_ADD(Z80,8000000)		 /* ? MHz */
	MDRV_CPU_PROGRAM_MAP(memmap,0)
	MDRV_CPU_VBLANK_INT("main", dderby_interrupt)

	/* video hardware */
	MDRV_SCREEN_ADD("main", RASTER)
	MDRV_SCREEN_REFRESH_RATE(60)
	MDRV_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MDRV_SCREEN_FORMAT(BITMAP_FORMAT_INDEXED16)
	MDRV_SCREEN_SIZE(256, 256)
	MDRV_SCREEN_VISIBLE_AREA(0, 256-1, 16, 256-16-1)

	MDRV_GFXDECODE(dmndrby)
	MDRV_PALETTE_LENGTH(0x100)

	MDRV_VIDEO_START(dderby)
	MDRV_VIDEO_UPDATE(dderby)
MACHINE_DRIVER_END

ROM_START( dmndrby )
	ROM_REGION( 0x04000, REGION_GFX1, 0 )
	ROM_LOAD( "dd01.e1", 0x00000, 0x02000, CRC(2e120288) SHA1(0ea29aff07e956b19080f05bd18b427195694ce8) )
	ROM_LOAD( "dd02.e2", 0x02000, 0x02000, CRC(ca028c8c) SHA1(f882eea2191cf1f3ea57d49fd6862f98401555be) )

	ROM_REGION( 0x10000, REGION_CPU2, 0 ) /* sound cpu */
	ROM_LOAD( "dd03.j9", 0x00000, 0x01000, CRC(660f0cae) SHA1(b3b414e52342de877a5c20886a87002a63310a94) )

	ROM_REGION( 0x10000, REGION_CPU1, 0 ) /* main cpu */
	ROM_LOAD( "dd04.m6", 0x00000, 0x02000, CRC(a3cfd28e) SHA1(7ba14876fa4409634a699e049bce3bc457522932) )
	ROM_LOAD( "dd05.m7", 0x02000, 0x02000, CRC(16f7ac0b) SHA1(030b8c2b294a0287f3aaf72424304fc191315888) )
	ROM_LOAD( "dd06.m8", 0x04000, 0x02000, CRC(914ba8f5) SHA1(d1b3f3d5d2625e42ea6cb5c777942cec7faea58e) )

	ROM_REGION( 0x18000, REGION_GFX2, 0 )
	ROM_LOAD( "dd07.b1", 0x00000, 0x02000, CRC(207a534a) SHA1(ddbd292f79cc9fb7bd9f0ee9874da87909147789) )
	ROM_LOAD( "dd08.b2", 0x02000, 0x02000, CRC(f380e2c4) SHA1(860a6557ae8b81d310c353f88f9194e1ffd551ec) )
	ROM_LOAD( "dd09.b3", 0x04000, 0x02000, CRC(68ebf74c) SHA1(959ee6c4ce700cff86af39442063dc79b8f8913e) )
	ROM_LOAD( "dd10.b5", 0x06000, 0x01000, CRC(38b1568a) SHA1(f7e04db49708dfc8c8512026d3460af0f3fb6780) )
	ROM_LOAD( "dd11.d1", 0x08000, 0x02000, CRC(fe615561) SHA1(808f703d0ca1576feb78f21c380e4006dd634a9c) )
	ROM_LOAD( "dd12.d2", 0x0a000, 0x02000, CRC(4df63aae) SHA1(a0b224fb1157fc25c21f9f0664bb8385e94e5c77) )
	ROM_LOAD( "dd13.d4", 0x0c000, 0x02000, CRC(cace0dfc) SHA1(41902f3ee2fa18798e3b441ee18f7b953d977b93) )
	ROM_LOAD( "dd14.d5", 0x0e000, 0x01000, CRC(2c602cbe) SHA1(78ffe79e3f2c4a3e9c6adc8f4635ed1a93528dc8) )
	ROM_LOAD( "dd15.e1", 0x10000, 0x02000, CRC(2ce23b64) SHA1(5cbeabc015cb167c7fd485ab4d9f1329bc2e94b3) )
	ROM_LOAD( "dd16.e2", 0x12000, 0x02000, CRC(6af9796c) SHA1(4cd818d488ac85fd6f8732fdca80cc29db86d3f4) )
	ROM_LOAD( "dd17.e4", 0x14000, 0x02000, CRC(b451cde2) SHA1(1c7340cc39d9beca1640c88000112c898d3de941) )
	ROM_LOAD( "dd18.e5", 0x16000, 0x01000, CRC(56228aaf) SHA1(74e96ebefc1b69310b23e47a35affbb7cd7d9acc) )

	ROM_REGION( 0x6000, REGION_GFX3, 0 )
	ROM_LOAD( "dd19.n2", 0x00000, 0x02000, CRC(fd536051) SHA1(556dfe064eeb9be1db751410ec128385c463e706) )
	ROM_LOAD( "dd20.n3", 0x02000, 0x02000, CRC(1497e52f) SHA1(f08c20c97c8d2148fcc705297cf1129bc65c9b83) )
	ROM_LOAD( "dd21.n4", 0x04000, 0x02000, CRC(87605d44) SHA1(c985fb15eac7bcc89e92909cf588b5982d0cabd0) )

	ROM_REGION( 0x4000, REGION_USER1, 0 ) // something else
	ROM_LOAD( "dd22.n6", 0x00000, 0x02000, CRC(db6b13fc) SHA1(3415deb2ffa86679e4f8abb644b75963e5368ba0) )
	ROM_LOAD( "dd23.n7", 0x02000, 0x02000, CRC(595fdb9b) SHA1(133d227bb156be52337da974e37973b049722e49) )

	ROM_REGION( 0x520, REGION_USER2, 0 ) // proms
	ROM_LOAD( "ddprom1.c1", 0x0000, 0x0100, CRC(e1c2fa1b) SHA1(6b8b0b2c1ac4b2796070452c923ba96dd8b29048) )
	ROM_LOAD( "ddprom2.j5", 0x0100, 0x0100, CRC(3e5402dc) SHA1(2f497333f49064c54995cec8919f3aebdc17e977) )
	ROM_LOAD( "ddprom4.h10",0x0200, 0x0100, CRC(7280f000) SHA1(bfad2b547b8abe6c67928bec08e7d00431c539d5) )
	ROM_LOAD( "ddprom5.k1", 0x0300, 0x0100, CRC(15edbdac) SHA1(298640afb24830d32f54c0c7c5960d777f51f2bd) )
	ROM_LOAD( "ddprom6.m12",0x0400, 0x0100, CRC(7f677b7d) SHA1(946014cb01f5954a3cb196796741ee174a0de641) )
	ROM_LOAD( "ddprom3.h5", 0x0500, 0x0020, CRC(aea3cff6) SHA1(1139dd52c127436873a674be8a14527190091a82) )
ROM_END

ROM_START( dmndrbya )
	ROM_REGION( 0x04000, REGION_GFX1, 0 )
	ROM_LOAD( "dd1", 0x00000, 0x02000, CRC(7fe475a6) SHA1(008bbaff87baad7f4c2497e40bf5e3fc95f993b4) )
	ROM_LOAD( "dd2", 0x02000, 0x02000, CRC(54def3ee) SHA1(fb88852ada2b5b555c0e8c0a082ed9978ff27434) )

	ROM_REGION( 0x10000, REGION_CPU2, 0 ) /* sound cpu */
	ROM_LOAD( "dd03.j9", 0x00000, 0x01000, CRC(660f0cae) SHA1(b3b414e52342de877a5c20886a87002a63310a94) )

	ROM_REGION( 0x10000, REGION_CPU1, 0 )
	ROM_LOAD( "dd4", 0x00000, 0x02000, CRC(29b06e0f) SHA1(301fc2fe25ce47c2ad5112f0b795cd6bae605071) )
	ROM_LOAD( "dd5", 0x02000, 0x02000, CRC(5299d020) SHA1(678d338d2cee5250154454be97456d5f80bb4759) )
	ROM_LOAD( "dd6", 0x04000, 0x02000, CRC(f7e30ec0) SHA1(bf898987366ee9def190e3575108395b0aaef2d6) )

	ROM_REGION( 0x18000, REGION_GFX2, 0 )
	ROM_LOAD( "dd07.b1", 0x00000, 0x02000, CRC(207a534a) SHA1(ddbd292f79cc9fb7bd9f0ee9874da87909147789) )
	ROM_LOAD( "dd08.b2", 0x02000, 0x02000, CRC(f380e2c4) SHA1(860a6557ae8b81d310c353f88f9194e1ffd551ec) )
	ROM_LOAD( "dd09.b3", 0x04000, 0x02000, CRC(68ebf74c) SHA1(959ee6c4ce700cff86af39442063dc79b8f8913e) )
	ROM_LOAD( "dd10.b5", 0x06000, 0x01000, CRC(38b1568a) SHA1(f7e04db49708dfc8c8512026d3460af0f3fb6780) )
	ROM_LOAD( "dd11.d1", 0x08000, 0x02000, CRC(fe615561) SHA1(808f703d0ca1576feb78f21c380e4006dd634a9c) )
	ROM_LOAD( "dd12.d2", 0x0a000, 0x02000, CRC(4df63aae) SHA1(a0b224fb1157fc25c21f9f0664bb8385e94e5c77) )
	ROM_LOAD( "dd13.d4", 0x0c000, 0x02000, CRC(cace0dfc) SHA1(41902f3ee2fa18798e3b441ee18f7b953d977b93) )
	ROM_LOAD( "dd14.d5", 0x0e000, 0x01000, CRC(2c602cbe) SHA1(78ffe79e3f2c4a3e9c6adc8f4635ed1a93528dc8) )
	ROM_LOAD( "dd15.e1", 0x10000, 0x02000, CRC(2ce23b64) SHA1(5cbeabc015cb167c7fd485ab4d9f1329bc2e94b3) )
	ROM_LOAD( "dd16.e2", 0x12000, 0x02000, CRC(6af9796c) SHA1(4cd818d488ac85fd6f8732fdca80cc29db86d3f4) )
	ROM_LOAD( "dd17.e4", 0x14000, 0x02000, CRC(b451cde2) SHA1(1c7340cc39d9beca1640c88000112c898d3de941) )
	ROM_LOAD( "dd18.e5", 0x16000, 0x01000, CRC(56228aaf) SHA1(74e96ebefc1b69310b23e47a35affbb7cd7d9acc) )

	ROM_REGION( 0x6000, REGION_GFX3, 0 )
	ROM_LOAD( "dd19.n2", 0x00000, 0x02000, CRC(fd536051) SHA1(556dfe064eeb9be1db751410ec128385c463e706) )
	ROM_LOAD( "dd20.n3", 0x02000, 0x02000, CRC(1497e52f) SHA1(f08c20c97c8d2148fcc705297cf1129bc65c9b83) )
	ROM_LOAD( "dd21.n4", 0x04000, 0x02000, CRC(87605d44) SHA1(c985fb15eac7bcc89e92909cf588b5982d0cabd0) )

	ROM_REGION( 0x4000, REGION_USER1, 0 ) // something else
	ROM_LOAD( "dd22.n6", 0x00000, 0x02000, CRC(db6b13fc) SHA1(3415deb2ffa86679e4f8abb644b75963e5368ba0) )
	ROM_LOAD( "dd23.n7", 0x02000, 0x02000, CRC(595fdb9b) SHA1(133d227bb156be52337da974e37973b049722e49) )

	ROM_REGION( 0x520, REGION_USER2, 0 ) // proms
	ROM_LOAD( "ddprom1.c1", 0x0000, 0x0100, CRC(e1c2fa1b) SHA1(6b8b0b2c1ac4b2796070452c923ba96dd8b29048) )
	ROM_LOAD( "ddprom2.j5", 0x0100, 0x0100, CRC(3e5402dc) SHA1(2f497333f49064c54995cec8919f3aebdc17e977) ) // wasn't in this set
	ROM_LOAD( "ddprom4.h10",0x0200, 0x0100, CRC(7280f000) SHA1(bfad2b547b8abe6c67928bec08e7d00431c539d5) )
	ROM_LOAD( "ddprom5.k1", 0x0300, 0x0100, CRC(15edbdac) SHA1(298640afb24830d32f54c0c7c5960d777f51f2bd) )
	ROM_LOAD( "ddprom6.m12",0x0400, 0x0100, CRC(7f677b7d) SHA1(946014cb01f5954a3cb196796741ee174a0de641) )
	ROM_LOAD( "ddprom3.h5", 0x0500, 0x0020, CRC(aea3cff6) SHA1(1139dd52c127436873a674be8a14527190091a82) )
ROM_END



GAME( 1986, dmndrby,  0,       dderby, dderby, 0, ROT0, "Electrocoin", "Diamond Derby (set 1)",GAME_NOT_WORKING|GAME_NO_SOUND )
GAME( 1986, dmndrbya, dmndrby, dderby, dderby, 0, ROT0, "Electrocoin", "Diamond Derby (set 2)",GAME_NOT_WORKING|GAME_NO_SOUND )
