/* IGS ARM7 (IGS027A) based Mahjong platform
 Driver by Xing Xing

 These games use the IGS027A processor.

 This is an ARM7 with Internal ROM. (Also used on later PGM games)

 In some cases the first part of the Internal ROM is excute only, and
 cannot be read out with a trojan.  It hasn't been confirmed if these
 games make use of that feature.

 To emulate these games the Internal ROM will need dumping
 There are at least 5 other games on the platform.

*/

#include "driver.h"
#include "cpu/arm7/arm7.h"
#include "cpu/arm7/arm7core.h"


/***************************************************************************

    Video

    0x38001000, 0x380017ff          CG_CONTROL,8 byte per object, 0x100 in total
    0x38001800, 0x380019ff      PALLETE RAM,2 byte per color, 0x100 in total
    0x38004000, 0x38005FFF      TX Video RAM????????1E00??????512x240??????
    0x38006000, 0x38007FFF      BG Video RAM????????1E00??????512x240??????


***************************************************************************/
static UINT32 *igs_mainram,*igs_cg_videoram,*igs_tx_videoram,*igs_bg_videoram;
static UINT32 *igs_pallete32;
static tilemap *igs_tx_tilemap,*igs_bg_tilemap;



/* CGLayer */
static WRITE32_HANDLER( igs_cg_videoram_w )
{
	COMBINE_DATA(&igs_cg_videoram[offset]);
	//if(data!=0)
	logerror("PC(%08X) CG @%x = %x!\n",cpu_get_pc(space->cpu),offset ,igs_cg_videoram[offset]);




	/*
    ROM:08020520                 DCW 0x3E                                           ddd1        y
    ROM:08020522                 DCW 0x29                                           ddd2        x
    ROM:08020524                 DCD 0x190BB6                                   ddd3        n
    ROM:08020528                 DCW 0xC                                            ddd4        Y
    ROM:0802052A                 DCW 0xA6                                           ddd5        X

    (ddd5+?)??10bit
    ddd2??9bit
    (ddd4+?)??11bit
    ddd1??8bit
    ddd3??10bit

    8060a4a6 2642ed8f
    A6A46080 8FED4226

    XXXX-XXXX
    XXxx-xxxx
    xxxY-YYYY
    YYYY-YYyy

    yyyy-yynn
    nnnn-nnnn



    */



}


/* TX Layer */
static WRITE32_HANDLER( igs_tx_videoram_w )
{
	COMBINE_DATA(&igs_tx_videoram[offset]);
	tilemap_mark_tile_dirty(igs_tx_tilemap,offset);
	//if(data!=0)
	//logerror( "TX VIDEO RAM OFFSET %x ,data %x!\n",offset ,igs_tx_videoram[offset]);
}

static TILE_GET_INFO( get_tx_tilemap_tile_info )
{
	//ppppppppNNNNNNNN
	int tileno,colour;
	tileno = igs_tx_videoram[tile_index] & 0xffff;
	colour = (igs_tx_videoram[tile_index]>>0x10) & 0xffff;

	SET_TILE_INFO(0,tileno,colour,0);
}

/* BG Layer */
static WRITE32_HANDLER( igs_bg_videoram_w )
{
	COMBINE_DATA(&igs_bg_videoram[offset]);
	tilemap_mark_tile_dirty(igs_bg_tilemap,offset);
	//if(data!=0)
	logerror("BG VIDEO RAM OFFSET %x ,data %x!\n",offset ,igs_bg_videoram[offset]);
}

static TILE_GET_INFO( get_bg_tilemap_tile_info )
{
	//ppppppppNNNNNNNN
	int tileno,colour;
	tileno = igs_bg_videoram[tile_index] & 0xffff;
	colour = (igs_bg_videoram[tile_index]>>0x10) & 0xffff;

	SET_TILE_INFO(0,tileno,colour,0);
}


/* Pallete Layer */
static WRITE32_HANDLER( igs_pallete32_w )
{
	space->machine->generic.paletteram.u16=(UINT16 *)igs_pallete32;
	COMBINE_DATA(&igs_pallete32[offset]);
	//paletteram16_xGGGGGRRRRRBBBBB_word_w(offset*2,space->machine->generic.paletteram.u16[offset*2],0);
	//paletteram16_xGGGGGRRRRRBBBBB_word_w(offset*2+1,space->machine->generic.paletteram.u16[offset*2+1],0);
	//if(data!=0)
	//fprintf(stdout,"PALLETE RAM OFFSET %x ,data %x!\n",offset ,igs_pallete32[offset]);
}



static VIDEO_START(igs_majhong)
{
	igs_tx_tilemap= tilemap_create(machine, get_tx_tilemap_tile_info,tilemap_scan_rows, 8, 8,64,32);
	tilemap_set_transparent_pen(igs_tx_tilemap,15);
	igs_bg_tilemap= tilemap_create(machine, get_bg_tilemap_tile_info,tilemap_scan_rows, 8, 8,64,32);
	//igs_bg_tilemap= tilemap_create(machine, get_bg_tilemap_tile_info,tilemap_scan_rows, 8, 8,64,32);
	//tilemap_set_transparent_pen(igs_bg_tilemap,15);
	logerror("Video START OK!\n");
}

static VIDEO_UPDATE(igs_majhong)
{
	//??????????
	bitmap_fill(bitmap,cliprect,get_black_pen(screen->machine));

	//??????
	tilemap_draw(bitmap,cliprect,igs_bg_tilemap,0,0);

	//CG??????

	//??????
	tilemap_draw(bitmap,cliprect,igs_tx_tilemap,0,0);
	//fprintf(stdout,"Video UPDATE OK!\n");
	return 0;
}

/***************************************************************************

    Blitter

***************************************************************************/
/***************************************************************************

    Memory Maps

***************************************************************************/

static ADDRESS_MAP_START( igs_majhong_map, ADDRESS_SPACE_PROGRAM, 32 )
	AM_RANGE(0x00000000, 0x00003fff) AM_ROM /* Internal ROM */
	AM_RANGE(0x08000000, 0x0807ffff) AM_ROM AM_REGION("user1", 0)/* Game ROM */
	AM_RANGE(0x10000000, 0x100003ff) AM_RAM AM_BASE(&igs_mainram)// main ram for asic?
	AM_RANGE(0x18000000, 0x18007fff) AM_RAM

	AM_RANGE(0x38001000, 0x380017ff) AM_RAM_WRITE(igs_cg_videoram_w) AM_BASE(&igs_cg_videoram)		//0x200 * 1   CG PALLETE?
	AM_RANGE(0x38001800, 0x38001fff) AM_RAM_WRITE(igs_pallete32_w) AM_BASE(&igs_pallete32)		//0x200 * 1

	AM_RANGE(0x38004000, 0x38005FFF) AM_RAM_WRITE(igs_tx_videoram_w) AM_BASE(&igs_tx_videoram) /* Text Layer */
	AM_RANGE(0x38006000, 0x38007FFF) AM_RAM_WRITE(igs_bg_videoram_w) AM_BASE(&igs_bg_videoram) /* CG Layer */


	AM_RANGE(0x38002010, 0x38002017) AM_RAM		//??????????????
	AM_RANGE(0x38009000, 0x38009003) AM_RAM		//??????????????
	AM_RANGE(0x70000200, 0x70000203) AM_RAM		//??????????????
	AM_RANGE(0x50000000, 0x500003ff) AM_WRITENOP // uploads xor table to external rom here
	AM_RANGE(0xf0000000, 0xF000000f) AM_WRITENOP // magic registers
ADDRESS_MAP_END


/***************************************************************************

    Common functions

***************************************************************************/

/***************************************************************************

    Code Decryption

***************************************************************************/
static const UINT8 sdwx_tab[] =
{
	0x49,0x47,0x53,0x30,0x30,0x35,0x35,0x52,0x44,0x34,0x30,0x32,0x30,0x36,0x32,0x31,
	0x8A,0xBB,0x20,0x67,0x97,0xA5,0x20,0x45,0x6B,0xC0,0xE8,0x0C,0x80,0xFB,0x49,0xAA,
	0x1E,0xAC,0x29,0xF2,0xB9,0x9F,0x01,0x4A,0x8D,0x5F,0x95,0x96,0x78,0xC3,0xF6,0x65,
	0x17,0xBD,0xB6,0x5B,0x25,0x5F,0x6B,0xDE,0x10,0x2E,0x67,0x05,0xDC,0xAC,0xB6,0xBD,
	0x3D,0x20,0x58,0x3D,0xF0,0xA8,0xC0,0xAD,0x5B,0x82,0x8D,0x12,0x65,0x97,0x87,0x7D,
	0x97,0x49,0xDD,0x74,0x74,0x7E,0x9D,0xA1,0x15,0xED,0x75,0xB9,0x09,0xA8,0xA8,0xB0,
	0x6B,0xEA,0x54,0x1B,0x45,0x23,0xE2,0xE5,0x25,0x42,0xCE,0x36,0xFE,0x42,0x99,0xA0,
	0x41,0xF8,0x0B,0x8C,0x3C,0x1B,0xAE,0xE4,0xB2,0x94,0x87,0x02,0xBC,0x08,0x17,0xD9,
	0xE0,0xA4,0x93,0x63,0x6F,0x28,0x5F,0x4A,0x24,0x36,0xD1,0xDA,0xFA,0xDD,0x23,0x26,
	0x4E,0x61,0xB9,0x7A,0x36,0x4D,0x95,0x01,0x20,0xBC,0x18,0xB7,0xAF,0xE4,0xFB,0x92,
	0xD2,0xE3,0x8E,0xEC,0x26,0xCE,0x2F,0x34,0x8F,0xF7,0x0D,0xD6,0x11,0x7F,0x1F,0x68,
	0xF4,0x1D,0x5F,0x16,0x19,0x2D,0x4C,0x4F,0x96,0xFC,0x9F,0xB0,0x99,0x53,0x4C,0x32,
	0x7B,0x41,0xBC,0x90,0x23,0x2E,0x4A,0xFC,0x9E,0x1D,0xFC,0x02,0xFC,0x41,0x83,0xBC,
	0x6D,0xC4,0x75,0x37,0x9D,0xD3,0xC9,0x26,0x4D,0xED,0x93,0xC6,0x32,0x6D,0x02,0x11,
	0x12,0x56,0x97,0x26,0x1D,0x5F,0xA7,0xF8,0x89,0x3F,0x14,0x36,0x72,0x3B,0x48,0x7B,
	0xF1,0xED,0x72,0xB7,0x7A,0x56,0x05,0xDE,0x7B,0x27,0x6D,0xCF,0x33,0x4C,0x14,0x86,
};
static void sdwx_decrypt(running_machine *machine)
{

	int i;
	UINT16 *src = (UINT16 *) memory_region(machine, "user1");

	int rom_size = 0x80000;

	for(i=0; i<rom_size/2; i++) {
		UINT16 x = src[i];

    if((i & 0x00480) != 0x00080)
      x ^= 0x0001;


    if((i & 0x004008) == 0x004008)
      x ^= 0x0002;


 //   if((i & 0x000030) == 0x000010)
 //     x ^= 0x0004;

    if((i & 0x000242) != 0x000042)
      x ^= 0x0008;

    if((i & 0x08100) == 0x08000)
      x ^= 0x0010;

    if((i & 0x022004) != 0x000004)
      x ^= 0x0020;

    if((i & 0x11800) != 0x10000)
      x ^= 0x0040;

    if((i & 0x004820) == 0x004820)
      x ^= 0x0080;

    x ^= sdwx_tab[(i >> 1) & 0xff] << 8;

		src[i] = x;
	}
}



static void sdwx_gfx_decrypt(running_machine *machine)
{
	int i;
	unsigned rom_size = 0x80000;
	UINT8 *src = (UINT8 *) (memory_region(machine, "gfx1"));
	UINT8 *result_data = alloc_array_or_die(UINT8, rom_size);

	for (i=0; i<rom_size; i++)
    	result_data[i] = src[BITSWAP24(i, 23,22,21,20,19,18,17,16,15,14,13,12,11,8,7,6,10,9,5,4,3,2,1,0)];

	for (i=0; i<rom_size; i+=0x200)
	{
		memcpy(src+i+0x000,result_data+i+0x000,0x80);
		memcpy(src+i+0x080,result_data+i+0x100,0x80);
		memcpy(src+i+0x100,result_data+i+0x080,0x80);
		memcpy(src+i+0x180,result_data+i+0x180,0x80);
	}
	free(result_data);
}

/***************************************************************************

    Protection & I/O

***************************************************************************/







/***************************************************************************

    Input Ports

***************************************************************************/

static INPUT_PORTS_START( sdwx )
INPUT_PORTS_END


/***************************************************************************

    Machine Drivers

***************************************************************************/


// for debugging
#if 0
static const gfx_layout charlayout =
{
	8,8,			/* 8 x 8 chars */
	RGN_FRAC(1,1),
	4,				/* 4 bits per pixel */
	{ 0, 1, 2, 3 },    /* planes are packed in a nibble */
	{ 33*4, 32*4, 49*4, 48*4, 1*4, 0*4, 17*4, 16*4 },
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8 },
	32*8	/* 32 bytes per char */
};
#endif

static const gfx_layout gfxlayout_8x8x4 =
{
    8,8,
    RGN_FRAC(1,1),
    4,
    //{ STEP4(0,8) },
    { 24,8,16,0 },
		{ STEP8(7,-1) },
    { STEP8(0,4*8) },
    8*8*4
};

#if 0
static const gfx_layout gfxlayout_16x16x16 =
{
    16,16,
    RGN_FRAC(1,1),
    16,
    { STEP16(0,0) },	// >8planes not supported
    { STEP16(15,-1) },
    { STEP16(0,16*1) },
    16*16*16
};
#endif

static GFXDECODE_START( igs_m027 )
    GFXDECODE_ENTRY( "gfx1", 0, gfxlayout_8x8x4,   0, 16  )
   // GFXDECODE_ENTRY( "gfx2", 0, gfxlayout_16x16x16, 0, 16  )
GFXDECODE_END


static INTERRUPT_GEN( igs_majhong_interrupt )
{
	generic_pulse_irq_line(device, ARM7_FIRQ_LINE);
}


static MACHINE_DRIVER_START( igs_majhong )
	MDRV_CPU_ADD("maincpu",ARM7, 20000000)

	MDRV_CPU_PROGRAM_MAP(igs_majhong_map)

	MDRV_CPU_VBLANK_INT("screen", igs_majhong_interrupt)
	//MDRV_NVRAM_HANDLER(generic_0fill)

	MDRV_GFXDECODE(igs_m027)


	MDRV_SCREEN_ADD("screen", RASTER)
	MDRV_SCREEN_REFRESH_RATE(60)
	MDRV_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MDRV_SCREEN_FORMAT(BITMAP_FORMAT_INDEXED16)
	MDRV_SCREEN_SIZE(512, 256)
	MDRV_SCREEN_VISIBLE_AREA(0, 512-1, 0, 256-1)

	MDRV_PALETTE_LENGTH(0x200)

	MDRV_VIDEO_START( igs_majhong )
	MDRV_VIDEO_UPDATE( igs_majhong )

	/* sound hardware */

MACHINE_DRIVER_END




static DRIVER_INIT( sdwx )
{
	sdwx_decrypt(machine);
	sdwx_gfx_decrypt(machine);
	logerror("init OK!\n");
}



/***************************************************************************

    ROMs Loading

***************************************************************************/
ROM_START( sdwx )
	ROM_REGION( 0x04000, "maincpu", 0 )
	/* Internal rom of IGS027A ARM based MCU */
	ROM_LOAD( "sdwx_igs027a", 0x00000, 0x4000, NO_DUMP )

	ROM_REGION( 0x80000, "user1", 0 ) // external ARM data / prg
	ROM_LOAD( "prg.u16", 0x000000, 0x80000, CRC(c94ef6a8) SHA1(69f2f356e05206b0866a9020253d9a112b56316c) )

	ROM_REGION( 0x80000, "gfx1", 0 )
	ROM_LOAD( "text.u24", 0x000000, 0x80000, CRC(60b415ac) SHA1(b4475b0ba1e70504cac9ac05078873df0b16495b) )

	ROM_REGION( 0x200000, "gfx2", 0 )
	ROM_LOAD( "cg.u25", 0x000000, 0x200000, CRC(709b9a42) SHA1(18c4b8e159b29c168f5cafb437fe6eb123672471) )

	ROM_REGION( 0x80000, "unknown", 0 )
	ROM_LOAD( "sp.u2", 0x00000, 0x80000, CRC(216b5418) SHA1(b7bc24ced0ccb5476c974420aa506c13b971fc9f) )
ROM_END




ROM_START( sddz )
	ROM_REGION( 0x04000, "maincpu", 0 )
	/* Internal rom of IGS027A ARM based MCU */
	ROM_LOAD( "sddz_igs027a", 0x00000, 0x4000, NO_DUMP )

	ROM_REGION( 0x80000, "user1", 0 ) // external ARM data / prg
	ROM_LOAD( "ddz_218cn.u17", 0x000000, 0x80000, CRC(3cfe38d5) SHA1(9c7f82ecffbc22879583519d5f753bb35e973ee3) )

	ROM_REGION( 0x80000, "gfx1", 0 )
	ROM_LOAD( "ddz_text.u27", 0x000000, 0x80000, CRC(520dc392) SHA1(0ab2620f20af8253806b6ff4e1d9d77a694da17c) )

	ROM_REGION( 0x400000, "gfx2", 0 )
	ROM_LOAD( "ddz_ani.u28", 0x000000, 0x400000, CRC(72487508) SHA1(9f4bbc858960ddaae403e4a3330b2345f6fd6cb3))

	ROM_REGION( 0x200000, "unknown", 0 )
	ROM_LOAD( "ddz_sp.u4", 0x00000, 0x200000, CRC(7ef65d95) SHA1(345c587cd449d6d06908e9687480be76b2cb2d28) )
ROM_END



ROM_START( lhzb3 )
	ROM_REGION( 0x04000, "maincpu", 0 )
	/* Internal rom of IGS027A ARM based MCU */
	ROM_LOAD( "lhzb3_igs027a", 0x00000, 0x4000, NO_DUMP )

	ROM_REGION( 0x80000, "user1", 0 ) // external ARM data / prg
	ROM_LOAD( "lhzb3_104.u9", 0x000000, 0x80000, CRC(70d61846) SHA1(662b59702ef6f26129de6b16346786df92f99097) )

	ROM_REGION( 0x80000, "gfx1", 0 )
	ROM_LOAD( "lhzb3_text.u17", 0x000000, 0x80000,CRC(a82398a9) SHA1(4d2987f57096b7f24ce6571ed3be6dcb33bce88d) )

	ROM_REGION( 0x400000, "gfx2", 0 )
	ROM_LOAD( "m2401.u18", 0x000000, 0x400000,  CRC(81428f18) SHA1(9fb19c8a79cc3443642f4b044e04735df2cb45be) )

	ROM_REGION( 0x200000, "unknown", 0 )
	ROM_LOAD( "s2402.u14", 0x00000, 0x100000, CRC(56083fe2) SHA1(62afd651809bf5e639bfda6e5579dbf4b903b664) )
ROM_END


ROM_START( lhzb4 )
	ROM_REGION( 0x04000, "maincpu", 0 )
	/* Internal rom of IGS027A ARM based MCU */
	ROM_LOAD( "lhzb4_igs027a", 0x00000, 0x4000, NO_DUMP )

	ROM_REGION( 0x80000, "user1", 0 ) // external ARM data / prg
	ROM_LOAD( "lhzb4_104.u17", 0x000000, 0x80000, CRC(6f349bbb) SHA1(54cf895889ef0f208637ba732ede696ca3603ee0) )

	ROM_REGION( 0x80000, "gfx1", 0 )
	ROM_LOAD( "lhzb4_text.u27", 0x000000, 0x80000, CRC(8488b039) SHA1(59bc9eccba810fcac2a53866b2da1e71bfd8a6e7) )

	ROM_REGION( 0x400000, "gfx2", 0 )
	ROM_LOAD( "a05501.u28", 0x000000, 0x400000, CRC(f78b3714) SHA1(c73d8e50b04126bc4f91783384713624ed133ee2) )

	ROM_REGION( 0x200000, "unknown", 0 )
	ROM_LOAD( "w05502.u5", 0x00000, 0x200000, CRC(467f677e) SHA1(63927c0d606176c0e22db89ea3a9777ed702abbd) )
ROM_END



ROM_START( klxyj )
	ROM_REGION( 0x04000, "maincpu", 0 )
	/* Internal rom of IGS027A ARM based MCU */
	ROM_LOAD( "klxyj_igs027a", 0x00000, 0x4000, NO_DUMP )

	ROM_REGION( 0x80000, "user1", 0 ) // external ARM data / prg
	ROM_LOAD( "klxyj_104.u16", 0x000000, 0x80000, CRC(8cb9bdc2) SHA1(5a13d0ff6488a938617a9ea89e7cf607539a1f49) )

	ROM_REGION( 0x80000, "gfx1", 0 )
	ROM_LOAD( "klxyj_text.u24", 0x000000, 0x80000, CRC(22dcebd0) SHA1(0383f017135230d020d12c8c6cc3aeb136fe9106) )

	ROM_REGION( 0x400000, "gfx2", 0 )
	ROM_LOAD( "a4202.u25", 0x000000, 0x400000, CRC(97a68f85) SHA1(177c8c23fd0d585b24a71359ede005ac9a2e4d4d) )

	ROM_REGION( 0x200000, "unknown", 0 )
	ROM_LOAD( "w4201.u2", 0x00000, 0x100000, CRC(464f11ab) SHA1(56e45bd31f667fc30387fcd4c940a94819b7ef0f) )
ROM_END


ROM_START( mgfx )
	ROM_REGION( 0x04000, "maincpu", 0 )
	/* Internal rom of IGS027A ARM based MCU */
	ROM_LOAD( "mgfx_igs027a", 0x00000, 0x4000, NO_DUMP )

	ROM_REGION( 0x80000, "user1", 0 ) // external ARM data / prg
	ROM_LOAD( "mgfx_101.u10", 0x000000, 0x80000, CRC(897c88a1) SHA1(0f7a7808b9503ff28ad32c0b8e071cb24cff59b1) )

	ROM_REGION( 0x80000, "gfx1", 0 )
	ROM_LOAD( "mgfx_text.u9", 0x000000, 0x80000, CRC(e41e7768) SHA1(3d0add7c75c23533309e799fd8853c815e6f811c) )

	ROM_REGION( 0x400000, "gfx2", 0 )
	ROM_LOAD( "mgfx_ani.u17", 0x000000, 0x400000, CRC(9fc75f4d) SHA1(acb600739dcf252a5210e28ec96d749573061b27) )

	ROM_REGION( 0x200000, "unknown", 0 )
	ROM_LOAD( "mgfx_sp.u14", 0x00000, 0x100000, CRC(9bb28fc8) SHA1(6368753c29607f2d212d68c5cca3f10aa069649b) )
ROM_END


/*

Big D2
IGS, 2000

PCB Layout
----------

IGS PCB NO-0267
|------------------------------------------|
|M2601.U17  PAL |-------|           RESET  |
|    M2603.U18  |       |                  |
|               |IGS027A|                  |
|               |       |         BATT_3.6V|
|               |-------|                  |
|                          W24257          |
|J                                S2602.U14|
|A         |-------|                       |
|M T2604.U9|       |                       |
|M         |IGS031 |  P2600.U10            |
|A         |       |                 M6295 |
|          |-------|                       |
|                                          |
|       22MHz     W24257                   |
|                                          |
|                        8255         VOL  |
|    DSW1(8)                               |
|        DSW2(8)             LM7805        |
|                                 UPC1242H |
|------------------------------------------|
Notes:
      W24257     - Winbond 32kx8 SRAM (SOJ28)
      Custom ICs -
                  IGS027A - ARM7/9? based CPU (QFP120, labelled 'J8')
                  IGS033  - likey GFX processor. Appears to be linked to the 3.6V battery. However,
                  the battery was dead and the PCB still works, so maybe the battery is not used? (QFP208)
      ROMs -
            P2600.U10 - 27C4096 EPROM, Main program
            M2601.U17 - 32MBit DIP42 MaskROM, read as 27C322, GFX (stamped 'IMAGE')
            M2603.U18 - 4MBit DIP40 EPROM, read as 27C4096, GFX (stamped 'IMAGE')
            S2602.U14 - 8MBit DIP32 MaskROM, read as MX27C8000, Oki M6295 sound data (stamped 'SPEECH')
            T2604.U9  - 4MBit DIP40 MaskROM, read as 27C4096, GFX (stamped 'TEXT')

*/

ROM_START( bigd2 )
	ROM_REGION( 0x04000, "maincpu", 0 )
	/* Internal rom of IGS027A ARM based MCU */
	ROM_LOAD( "bigd2_igs027a", 0x00000, 0x4000, NO_DUMP )

	ROM_REGION( 0x80000, "user1", 0 ) // external ARM data / prg
	ROM_LOAD( "p2600.u10", 0x000000, 0x80000, CRC(9ad34135) SHA1(54717753d1296efe49946369fd4a27181f19dbc0) )

	ROM_REGION( 0x80000, "gfx1", 0 )
	ROM_LOAD( "t2604.u9", 0x000000, 0x80000, CRC(5401a52d) SHA1(05b47a4b39939c1d5904e3fbd5cc56d6ee9b7953) )

	ROM_REGION( 0x400000, "gfx2", 0 )
	ROM_LOAD( "m2601.u17", 0x000000, 0x400000, CRC(89736e3f) SHA1(6a22e2eb10d2c740cf21640c43a8caf4c72d3be7) )
	ROM_LOAD( "m2603.u18", 0x000000, 0x080000, CRC(fb2e91a8) SHA1(29b2f0ce3749539cbe4cfb5c40b240cc7f6147f1) )

	ROM_REGION( 0x200000, "unknown", 0 )
	ROM_LOAD( "s2602.u14", 0x00000, 0x100000, CRC(f137028c) SHA1(0e4114222820bca2f7026fa653e2b96a489a0183) )
ROM_END


/***************************************************************************

    Game Drivers

***************************************************************************/

GAME( 2002,  sdwx, 0, igs_majhong, sdwx, sdwx, ROT0, "IGS", "Sheng Dan Wu Xian", GAME_NO_SOUND | GAME_NOT_WORKING ) // aka Christmas 5 Line?
GAME( 200?,  sddz, 0, igs_majhong, sdwx, 0,    ROT0, "IGS", "Super Dou Di Zhu",  GAME_NO_SOUND | GAME_NOT_WORKING )
GAME( 2000,  bigd2,0, igs_majhong, sdwx, 0,    ROT0, "IGS", "Big D2",  GAME_NO_SOUND | GAME_NOT_WORKING )
GAME( 200?,  lhzb3,0, igs_majhong, sdwx, 0,    ROT0, "IGS", "Long Hu Zheng Ba 3", GAME_NO_SOUND | GAME_NOT_WORKING ) // ????????3
GAME( 200?,  lhzb4,0, igs_majhong, sdwx, 0,    ROT0, "IGS", "Long Hu Zheng Ba 4", GAME_NO_SOUND | GAME_NOT_WORKING ) // ????????4
GAME( 200?,  klxyj,0, igs_majhong, sdwx, 0,    ROT0, "IGS", "Kuai Le Xi You Ji",  GAME_NO_SOUND | GAME_NOT_WORKING ) // ??????????
GAME( 2000,  mgfx, 0, igs_majhong, sdwx, 0,    ROT0, "IGS", "Man Guan Fu Xing",   GAME_NO_SOUND | GAME_NOT_WORKING ) // ????????
