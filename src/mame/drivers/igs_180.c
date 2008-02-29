/* Some IGS games with a HD64180 */
/* also see iqblock.c and tarzan.c */

#include "driver.h"
#include "deprecat.h"
#include "cpu/z180/z180.h"
#include "sound/2413intf.h"
#include "sound/okim6295.h"

static UINT8 *fg_videoram;
static tilemap *fg_tilemap;

static TILE_GET_INFO( get_fg_tile_info )
{
	int code = fg_videoram[tile_index*4+0] + ((fg_videoram[tile_index*4+1] & 0xf) << 8);
	SET_TILE_INFO(1, code, 0, 0);
}

static VIDEO_START(igs_180)
{
	fg_tilemap = tilemap_create(get_fg_tile_info,tilemap_scan_rows,8,8,64,32);
}

static VIDEO_UPDATE(igs_180)
{
	tilemap_mark_all_tiles_dirty(fg_tilemap);
	tilemap_draw(bitmap,cliprect,fg_tilemap,0,0);

	return 0;
}

static INPUT_PORTS_START( igs_180 )
INPUT_PORTS_END

static ADDRESS_MAP_START( igs_180_map, ADDRESS_SPACE_PROGRAM, 8 )
	AM_RANGE(0x00000, 0x0dfff) AM_ROM
	AM_RANGE(0x0e000, 0x0efff) AM_RAM
	AM_RANGE(0x0f000, 0x0ffff) AM_RAM
	AM_RANGE(0x10000, 0x3ffff) AM_ROM
ADDRESS_MAP_END

/*
#define W(n) \
    static WRITE8_HANDLER( w##n )\
    {\
        static int old = -1;\
        if(data != old)\
            printf("w"#n" = %02X\n",old=data);\
    }\

W(1)
W(2)
W(3)
W(4)
W(5)
W(6)
W(7)
W(8)
*/

static WRITE8_HANDLER( nmi_ack_w )
{
//  cpunum_set_input_line(Machine, 0, INPUT_LINE_NMI, CLEAR_LINE);
}

static WRITE8_HANDLER( irq0_ack_w )
{
//  cpunum_set_input_line(Machine, 0, 0, CLEAR_LINE);
}

static READ8_HANDLER( ff_r )
{
	return 0xff;
}

static ADDRESS_MAP_START( igs_180_portmap, ADDRESS_SPACE_IO, 8 )
	AM_RANGE(0x0000, 0x003f) AM_RAM // internal regs

	AM_RANGE(0x1000, 0x17ff) AM_RAM
	AM_RANGE(0x1800, 0x19ff) AM_RAM
	AM_RANGE(0x1a00, 0x1bff) AM_RAM
	AM_RANGE(0x1c00, 0x1fff) AM_RAM
	AM_RANGE(0x200a, 0x200a) AM_WRITENOP
	AM_RANGE(0x2010, 0x2010) AM_READ(ff_r) AM_WRITENOP
	AM_RANGE(0x2011, 0x2011) AM_READ(ff_r) AM_WRITENOP
	AM_RANGE(0x2012, 0x2012) AM_READ(ff_r) AM_WRITENOP
	AM_RANGE(0x2014, 0x2014) AM_WRITE(nmi_ack_w)
	AM_RANGE(0x2015, 0x2015) AM_WRITE(irq0_ack_w)

	AM_RANGE(0x4000, 0x4fff) AM_RAM AM_BASE(&fg_videoram)
	AM_RANGE(0x5000, 0x5fff) AM_RAM
	AM_RANGE(0x6000, 0x7fff) AM_RAM

	AM_RANGE(0x8000, 0x8000) AM_WRITE(YM2413_register_port_0_w)
	AM_RANGE(0x8001, 0x8001) AM_READ(ff_r) AM_WRITE(YM2413_data_port_0_w)
	AM_RANGE(0xa000, 0xa000) AM_READ(ff_r)
	AM_RANGE(0xb000, 0xb000) AM_WRITENOP
	AM_RANGE(0xb001, 0xb001) AM_WRITENOP
ADDRESS_MAP_END

static const gfx_layout tilelayout =
{
	8,8,
	RGN_FRAC(1,4), //?
	6, //?
	{ 8, RGN_FRAC(0,4), RGN_FRAC(1,4)+8, RGN_FRAC(1,4), RGN_FRAC(2,4)+8, RGN_FRAC(2,4) }, //?
	{ 0, 1, 2, 3, 4, 5, 6, 7 },
	{ 0*32, 1*32, 2*32, 3*32, 4*32, 5*32, 6*32, 7*32 },
	32*8
};

static GFXDECODE_START( igs_180 )
	GFXDECODE_ENTRY( REGION_GFX1, 0, tilelayout, 0, 1 )
	GFXDECODE_ENTRY( REGION_GFX2, 0, tilelayout, 0, 1 )
GFXDECODE_END


static INTERRUPT_GEN( igs_180_interrupt )
{
	if (cpu_getiloops() & 1)
		cpunum_set_input_line(machine, 0, INPUT_LINE_NMI, PULSE_LINE);//ASSERT_LINE);    /* ???? */
	else
		cpunum_set_input_line(machine, 0, 0, HOLD_LINE);//ASSERT_LINE);          /* ???? */
}

static MACHINE_DRIVER_START( igs_180 )

	/* basic machine hardware */
	MDRV_CPU_ADD(Z180,16000000)	/* 16 MHz? */
	MDRV_CPU_PROGRAM_MAP(igs_180_map,0)
	MDRV_CPU_IO_MAP(igs_180_portmap,0)
	MDRV_CPU_VBLANK_INT_HACK(igs_180_interrupt,2)
	//MDRV_CPU_VBLANK_INT("main", irq0_line_hold)

	/* video hardware */
	MDRV_SCREEN_ADD("main", RASTER)
	MDRV_SCREEN_REFRESH_RATE(60)
	MDRV_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MDRV_SCREEN_FORMAT(BITMAP_FORMAT_INDEXED16)
	MDRV_SCREEN_SIZE(64*8, 32*8)
	MDRV_SCREEN_VISIBLE_AREA(0*8, 64*8-1, 0*8, 32*8-1)

	MDRV_GFXDECODE(igs_180)
	MDRV_PALETTE_LENGTH(1024)

	MDRV_VIDEO_START(igs_180)
	MDRV_VIDEO_UPDATE(igs_180)

	/* sound hardware */
	MDRV_SPEAKER_STANDARD_MONO("mono")
	MDRV_SOUND_ADD(YM2413, 3579545)
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.5)

	MDRV_SOUND_ADD(OKIM6295, 16000000/16)
	MDRV_SOUND_CONFIG(okim6295_interface_region_1_pin7high)
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.5)
MACHINE_DRIVER_END


/*

IQ Block (alt hardware)
IGS, 1996

PCB Layout
----------

IGS PCB N0- 0131-4
|---------------------------------------|
|uPD1242H     VOL    U3567   3.579545MHz|
|                               AR17961 |
|   HD64180RP8                          |
|  16MHz                         BATTERY|
|                                       |
|                         SPEECH.U17    |
|                                       |
|J                        6264          |
|A                                      |
|M      8255              V.U18         |
|M                                      |
|A                                      |
|                                       |
|                                       |
|                      |-------|        |
|                      |       |        |
|       CG.U7          |IGS017 |        |
|                      |       |        |
|       TEXT.U8        |-------|   PAL  |
|            22MHz               61256  |
|                   DSW1  DSW2  DSW3    |
|---------------------------------------|
Notes:
      HD64180RP8 - Hitachi HD64180 CPU. Clocks 16MHz (pins 2 & 3), 8MHz (pin 64)
      61256   - 32k x8 SRAM (DIP28)
      6264    - 8k x8 SRAM (DIP28)
      IGS017  - Custom IGS IC (QFP208)
      AR17961 - == Oki M6295 (QFP44). Clock 1.000MHz [16/16]. pin 7 = high
      U3567   - == YM2413. Clock 3.579545MHz
      VSync   - 60Hz
      HSync   - 15.31kHz

  */

ROM_START( iqblocka )
	ROM_REGION( 0x40000, REGION_CPU1, 0 ) /* DIP28 Code */
	ROM_LOAD( "v.u18", 0x00000, 0x40000, CRC(2e2b7d43) SHA1(cc73f4c8f9a6e2219ee04c9910725558a80b4eb2) )

	ROM_REGION( 0x40000, REGION_SOUND1, 0 ) /* Samples */
	ROM_LOAD( "speech.u17", 0x00000, 0x40000, CRC(d9e3d39f) SHA1(bec85d1ac2dfca77453cbca0e7dd53fee8fb438b) )

	ROM_REGION( 0x80000, REGION_GFX1, ROMREGION_INVERT )
	ROM_LOAD( "cg.u7",   0x000000, 0x080000, CRC(cb48a66e) SHA1(6d597193d1333a97957d5ceec8179a24bedfd928) )

	ROM_REGION( 0x80000, REGION_GFX2, ROMREGION_INVERT )
	ROM_LOAD( "text.u8", 0x000000, 0x080000, CRC(48c4f4e6) SHA1(b1e1ca62cf6a99c11a5cc56705eef7e22a3b2740) )
ROM_END

ROM_START( iqblockf )
	ROM_REGION( 0x40000, REGION_CPU1, 0 ) /* DIP28 Code */
	ROM_LOAD( "v113fr.u18", 0x00000, 0x40000, CRC(346c68af) SHA1(ceae4c0143c288dc9c1dd1e8a51f1e3371ffa439) )

	ROM_REGION( 0x40000, REGION_SOUND1, 0 ) /* Samples */
	ROM_LOAD( "sp.u17", 0x00000, 0x40000, CRC(71357845) SHA1(25f4f7aebdcc0706018f041d3696322df569b0a3) )

	ROM_REGION( 0x80000, REGION_GFX1, ROMREGION_INVERT )
	ROM_LOAD( "cg.u7",   0x000000, 0x080000, CRC(cb48a66e) SHA1(6d597193d1333a97957d5ceec8179a24bedfd928) )

	ROM_REGION( 0x80000, REGION_GFX2, ROMREGION_INVERT )
	ROM_LOAD( "text.u8", 0x000000, 0x080000, CRC(48c4f4e6) SHA1(b1e1ca62cf6a99c11a5cc56705eef7e22a3b2740) )
ROM_END

/*

Mahjong Tian Jiang Shen Bing
IGS, 1997

This PCB is almost the same as IQBlock (IGS, 1996)
but the 8255 has been replaced with the IGS025 IC

PCB Layout
----------

IGS PCB N0- 0157-2
|---------------------------------------|
|uPD1242H     VOL    U3567   3.579545MHz|
|                         AR17961       |
|   HD64180RP8                   SPDT_SW|
|  16MHz                         BATTERY|
|                                       |
|                         S0703.U15     |
|                                       |
|J     |-------|          6264          |
|A     |       |                        |
|M     |IGS025 |          P0700.U16     |
|M     |       |                        |
|A     |-------|                        |
|                                       |
|                                       |
|                      |-------|        |
|                      |       |   PAL  |
|       A0701.U3       |IGS017 |        |
|                      |       |   PAL  |
|       TEXT.U6        |-------|        |
|            22MHz               61256  |
|                   DSW1  DSW2  DSW3    |
|---------------------------------------|
Notes:
      HD64180RP8 - Hitachi HD64180 CPU. Clocks 16MHz (pins 2 & 3), 8MHz (pin 64)
      61256   - 32k x8 SRAM (DIP28)
      6264    - 8k x8 SRAM (DIP28)
      IGS017  - Custom IGS IC (QFP208)
      IGS025  - Custom IGS IC (PLCC68)
      AR17961 - == Oki M6295 (QFP44). Clock 1.000MHz [16/16]. pin 7 = high
      U3567   - == YM2413. Clock 3.579545MHz
      VSync   - 60Hz
      HSync   - 15.30kHz

*/

ROM_START( tjsb )
	ROM_REGION( 0x40000, REGION_CPU1, 0 ) /* DIP28 Code */
	ROM_LOAD( "p0700.u16", 0x00000, 0x40000,CRC(1b2a50df) SHA1(95a272e624f727df9523667864f933118d9e633c) )

	ROM_REGION( 0x80000, REGION_SOUND1, 0 ) /* Samples */
	ROM_LOAD( "s0703.u15", 0x00000, 0x80000,  CRC(c6f94d29) SHA1(ec413580240711fc4977dd3c96c288501aa7ef6c) )

	ROM_REGION( 0x200000, REGION_GFX1, ROMREGION_INVERT )
	ROM_LOAD( "a0701.u3", 0x000000, 0x200000, CRC(aa182140) SHA1(37c2053386c183ff726ba417d13f2063cf9a22df) )

	ROM_REGION( 0x80000, REGION_GFX2, ROMREGION_INVERT )
	ROM_LOAD( "text.u6", 0x000000, 0x080000,  CRC(3be886b8) SHA1(15b3624ed076640c1828d065b01306a8656f5a9b) )
ROM_END

static void decrypt_program_rom(int mask, int a7, int a6, int a5, int a4, int a3, int a2, int a1, int a0)
{
	int length = memory_region_length(REGION_CPU1);
	UINT8 *rom = memory_region(REGION_CPU1);
	UINT8 *tmp = auto_malloc(length);
	int i;

	/* decrypt the program ROM */

	/* XOR layer */
	for (i = 0;i < length;i++)
	{
		if(i & 0x2000)
		{
			if((i & mask) == mask)
				rom[i] ^= 0x01;
		}
		else
		{
			if(i & 0x0100)
			{
				if((i & mask) == mask)
					rom[i] ^= 0x01;
			}
			else
			{
				if(i & 0x0080)
				{
					if((i & mask) == mask)
						rom[i] ^= 0x01;
				}
				else
				{
					if((i & mask) != mask)
						rom[i] ^= 0x01;
				}
			}
		}
	}

	memcpy(tmp,rom,length);

	/* address line swap */
	for (i = 0;i < length;i++)
	{
		int addr = (i & ~0xff) | BITSWAP8(i,a7,a6,a5,a4,a3,a2,a1,a0);
		rom[i] = tmp[addr];
	}
}

static DRIVER_INIT( iqblock )
{

	decrypt_program_rom(0x11, 7, 6, 5, 4, 3, 2, 1, 0);
}

static DRIVER_INIT( tjsb )
{
	decrypt_program_rom(0x05, 7, 6, 3, 2, 5, 4, 1, 0);
}

GAME( 1996, iqblocka, iqblock, igs_180, igs_180, iqblock, ROT0, "IGS", "IQ-Block (V127M)", GAME_NOT_WORKING )
GAME( 1996, iqblockf, iqblock, igs_180, igs_180, iqblock, ROT0, "IGS", "IQ-Block (V113FR)", GAME_NOT_WORKING )
GAME( 1997, tjsb,     0,       igs_180, igs_180, tjsb,    ROT0, "IGS", "Mahjong Tian Jiang Shen Bing", GAME_NOT_WORKING )
