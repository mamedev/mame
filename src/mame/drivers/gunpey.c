/*
Gunpey
Banpresto, 2000

The hardware looks very Raizing/8ing -ish, especially the ROM stickers and PCB silk-screening
which are identical to those on Brave Blade and Battle Bakraid ;-)

PCB Layout
----------

VG-10
|-------------------------------------|
|        M6295  ROM5                  |
|        YMZ280B-F      ROM4   ROM3   |
|  YAC516      16.93MHz               |
|                       61256  ROM1   |
|                       61256  ROM2   |
|J                                    |
|A             PAL       XILINX       |
|M                       XC95108      |
|M             57.2424MHz             |
|A                            V30     |
|                                     |
|              |-------|              |
|              |AXELL  |              |
|       DSW1   |AG-1   |              |
|       DSW2   |AX51101|  T2316162    |
|              |-------|  T2316162    |
|-------------------------------------|

Notes:
      V30 clock: 14.3106MHz (= 57.2424 / 4)
      YMZ280B clock: 16.93MHz
      OKI M6295 clock: 2.11625MHz (= 16.93 / 8), sample rate = clock / 165
      VSync: 60Hz
      HSync: 15.79kHz

      ROMs:
           GP_ROM1.021 \
           GP_ROM2.022 / 27C040, Main program

           GP_ROM3.025 \
           GP_ROM4.525 / SOP44 32M MASK, Graphics

           GP_ROM5.622   SOP44 32M MASK, OKI samples
*/

#include "driver.h"

static UINT16 *unkram;
static UINT16 *mainram;

static VIDEO_START( gunpey )
{
}

static VIDEO_UPDATE( gunpey )
{

	return 0;
}

static WRITE16_HANDLER(unk_w)
{
	COMBINE_DATA(&unkram[offset]);
}


static READ16_HANDLER(unk_r)
{
	// c9 = status ? //40 10 04=start ?
	// 40,41,44 = ?
	// 4a = flags?
		if(offset==0xc9/2 )//|| offset==0x40 || offset==0x41 || offset==0x44)
			return mame_rand(machine);
	return unkram[offset];
}


static WRITE16_HANDLER(main_w)
{
	COMBINE_DATA(&mainram[offset]);
}


static READ16_HANDLER(main_r)
{
	if(offset>0x502d/2 && offset<0x56c0/2)
		logerror("R %x @%x\n",offset*2,activecpu_get_pc());
	return mainram[offset];
}


/***************************************************************************************/

static ADDRESS_MAP_START( mem_map, ADDRESS_SPACE_PROGRAM, 16 )
	AM_RANGE(0x00000, 0x0ffff) AM_READWRITE(main_r, main_w) AM_BASE(&mainram)
	AM_RANGE(0x50000, 0x500ff) AM_RAM AM_BASE(&unkram)
	AM_RANGE(0x50100, 0x502ff) AM_NOP
	AM_RANGE(0x80000, 0xfffff) AM_ROM
ADDRESS_MAP_END

static ADDRESS_MAP_START( io_map, ADDRESS_SPACE_IO, 16 )
	AM_RANGE(0x7f00, 0x7fff) AM_READWRITE(unk_r, unk_w)
ADDRESS_MAP_END


/***************************************************************************************/



/***************************************************************************************/

static INTERRUPT_GEN( gunpey_interrupt )
{
	cpunum_set_input_line_and_vector(machine, 0,0,HOLD_LINE,0x200/4);
}

/***************************************************************************************/
static MACHINE_DRIVER_START( gunpey )

	/* basic machine hardware */
	MDRV_CPU_ADD_TAG("main", V30, 57242400 / 4)
	MDRV_CPU_PROGRAM_MAP(mem_map,0)
	MDRV_CPU_IO_MAP(io_map,0)
	MDRV_CPU_VBLANK_INT("main", gunpey_interrupt)

	/* video hardware */
	MDRV_SCREEN_ADD("main", RASTER)
	MDRV_SCREEN_REFRESH_RATE(60)
	MDRV_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MDRV_SCREEN_FORMAT(BITMAP_FORMAT_INDEXED16)
	MDRV_SCREEN_SIZE(32*8, 32*8)
	MDRV_SCREEN_VISIBLE_AREA(0*8, 32*8-1, 0*8, 32*8-1)

	MDRV_PALETTE_LENGTH(0x800)

	MDRV_VIDEO_START(gunpey)
	MDRV_VIDEO_UPDATE(gunpey)

MACHINE_DRIVER_END

/***************************************************************************************/

static INPUT_PORTS_START( gunpey )

INPUT_PORTS_END


/***************************************************************************************/


ROM_START( gunpey )
	ROM_REGION( 0x100000, REGION_CPU1, 0 ) /* V30 code */
	ROM_LOAD16_BYTE( "gp_rom1.021",  0x00000, 0x80000, CRC(07a589a7) SHA1(06c4140ffd5f74b3d3ddfc424f43fcd08d903490) )
	ROM_LOAD16_BYTE( "gp_rom2.022",  0x00001, 0x80000, CRC(f66bc4cf) SHA1(54931d878d228c535b9e2bf22a0a3e41756f0fe5) )

	ROM_REGION( 0x400000*2, REGION_GFX1, 0 )
	ROM_LOAD( "gp_rom3.025",  0x00000, 0x400000,  CRC(f2d1f9f0) SHA1(0d20301fd33892074508b9d127456eae80cc3a1c) )
	ROM_LOAD( "gp_rom4.525",  0x400000, 0x400000, CRC(78dd1521) SHA1(91d2046c60e3db348f29f776def02e3ef889f2c1) )


	ROM_REGION( 0x400000, REGION_SOUND1, 0 )
	ROM_LOAD( "gp_rom5.622",  0x00000, 0x400000,  CRC(f79903e0) SHA1(4fd50b4138e64a48ec1504eb8cd172a229e0e965))

ROM_END

GAME( 2000, gunpey, 0, gunpey, gunpey, 0,	ROT0, "Banpresto", "Gunpey",GAME_NO_SOUND|GAME_NOT_WORKING)
