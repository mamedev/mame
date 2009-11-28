/*

Sega System H1
preliminary

22 Aug 2004 - Basic skeleton driver, loads some roms doesn't run the right code yet
2 Dec 2008 - Added an hack for the SH-2,fixed some irqs and some memory maps/ram sharing.
             Got to the point that area 0x03e00000 on the SH-2 loads some DMA-style tables.

Known Games on this Platform
Cool Riders

-- readme --

Cool Riders by SEGA 1995
SYSTEM H1 CPU Board
-------------------
Processors :
Hitachi SH2 HD6417095
Toshiba TMP68HC000N-16
Hitachi SH7032 HD6417032F20

Eprom :
Ep17662.12

SEGA CUSTOM IC :
315-5687 (x2)
315-5757
315-5758
315-5849
315-5800 GAL16V8B
315-5801 GAL16V8B
315-5802 GAL16V8B

SYSTEM H1 VIDEO BOARD
---------------------
SEGA CUSTOM IC :
315-5648 (x4)
315-5691
315-5692
315-5693 (x2)
315-5694
315-5695 (x2)
315-5696 (x2)
315-5697
315-5698
315-5803 GAL16V8B
315-5864 GAL16V8B

*/

#include "driver.h"
#include "cpu/sh2/sh2.h"
#include "cpu/m68000/m68000.h"
#include "deprecat.h"
#include "sound/scsp.h"

static UINT32* sysh1_workram_h,*h1_ioga,*framebuffer_data,*framebuffer_vram;

/* video */

static VIDEO_START(coolridr)
{
}

static VIDEO_UPDATE(coolridr)
{
	return 0;
}

/* end video */

static READ32_HANDLER(sysh1_ioga_r)
{
	switch(offset)
	{
		case 0x08/4:
		{
			static UINT8 vblank = 0;

			vblank^=1;

			return (h1_ioga[offset] & 0xfdffffff) | (vblank<<25);
		}
	}

	return h1_ioga[offset];
}

static WRITE32_HANDLER(sysh1_ioga_w)
{
	COMBINE_DATA(&h1_ioga[offset]);
}

#ifdef UNUSED_FUNCTION
static WRITE32_HANDLER( paletteram32_sysh1_w )
{
	int r,g,b;
	COMBINE_DATA(&space->machine->generic.paletteram.u32[offset]);

	b = ((space->machine->generic.paletteram.u32[offset] & 0x00007c00) >> 10);
	g = ((space->machine->generic.paletteram.u32[offset] & 0x000003e0) >> 5);
	r = ((space->machine->generic.paletteram.u32[offset] & 0x0000001f) >> 0);
	palette_set_color_rgb(space->machine,(offset*2)+1,pal5bit(r),pal5bit(g),pal5bit(b));
	b = ((space->machine->generic.paletteram.u32[offset] & 0x7c000000) >> 26);
	g = ((space->machine->generic.paletteram.u32[offset] & 0x03e00000) >> 21);
	r = ((space->machine->generic.paletteram.u32[offset] & 0x001f0000) >> 16);
	palette_set_color_rgb(space->machine,offset*2,pal5bit(r),pal5bit(g),pal5bit(b));
}

static WRITE32_HANDLER( sysh1_dmac_w )
{
	static UINT32 src,dst,size;
	COMBINE_DATA(&dma_data[offset]);

	if(offset == 0x10/4)
		src = dma_data[offset] & 0x7fffff0;
	if(offset == 0x14/4)
	{
		dst = dma_data[offset] & 0x7fffff0;
		if(dma_data[offset] & 0x40000000)
		{
			for(size=0;size<0x800;size+=4)
			{
				program_write_dword(dst,program_read_dword(src));
				dst+=4;
				src+=4;
			}
			popmessage("!");
		}
	}
}
#endif

//UINT16* sysh1_soundram;

// what's wrong:
//
// SH-1 waits for "SEGA" at 0530008c after writing it to 6000020 and 600010c
// SH-2 writes 0x01 to 060d88a5, expects something (SH-1?) to change that

static ADDRESS_MAP_START( system_h1_map, ADDRESS_SPACE_PROGRAM, 32 )
	AM_RANGE(0x00000000, 0x000fffff) AM_ROM AM_SHARE(1)
	/*WARNING: boundaries of these two are WRONG!*/
	AM_RANGE(0x03e00000, 0x03e0ffff) AM_RAM AM_BASE(&framebuffer_vram)/*Buffer VRAM Chains*/
	AM_RANGE(0x03e10000, 0x03e11fff) AM_RAM AM_BASE(&framebuffer_data)/*Buffer data should go here*/

	AM_RANGE(0x03f00000, 0x03f0ffff) AM_RAM AM_SHARE(3) /*Communication area RAM*/
//  AM_RANGE(0x04000000, 0x0400001f) AM_RAM /*???*/
	AM_RANGE(0x04000000, 0x0400ffff) AM_RAM /*dunno what it is,might be palette RAM*/
	AM_RANGE(0x06000000, 0x060fffff) AM_RAM AM_BASE(&sysh1_workram_h)
	AM_RANGE(0x20000000, 0x200fffff) AM_ROM AM_SHARE(1)

	AM_RANGE(0x60000000, 0x600003ff) AM_WRITENOP
ADDRESS_MAP_END

static ADDRESS_MAP_START( coolridr_submap, ADDRESS_SPACE_PROGRAM, 32 )
	AM_RANGE(0x00000000, 0x0001ffff) AM_ROM AM_SHARE(2)

	AM_RANGE(0x01000000, 0x0100ffff) AM_RAM

	AM_RANGE(0x03008900, 0x03008903) AM_RAM /*???*/
	AM_RANGE(0x03100400, 0x03100403) AM_RAM /*irq enable?*/
	AM_RANGE(0x03208900, 0x03208903) AM_RAM /*???*/
	AM_RANGE(0x03300400, 0x03300403) AM_RAM /*irq enable?*/

	AM_RANGE(0x04000000, 0x0400003f) AM_READWRITE(sysh1_ioga_r,sysh1_ioga_w) AM_BASE(&h1_ioga) /*input area?*/
	AM_RANGE(0x04200000, 0x0420003f) AM_RAM /*???*/

	AM_RANGE(0x05000000, 0x05000fff) AM_RAM
	AM_RANGE(0x05200000, 0x052001ff) AM_RAM
	AM_RANGE(0x05300000, 0x0530ffff) AM_RAM AM_SHARE(3) /*Communication area RAM*/
	AM_RANGE(0x05ff0000, 0x05ffffff) AM_RAM /*???*/
	AM_RANGE(0x06000000, 0x06000fff) AM_RAM
	AM_RANGE(0x06100000, 0x06100fff) AM_RAM
	AM_RANGE(0x06200000, 0x06200fff) AM_RAM
	AM_RANGE(0x07fff000, 0x07ffffff) AM_RAM
	AM_RANGE(0x20000000, 0x2001ffff) AM_ROM AM_SHARE(2)

	AM_RANGE(0x60000000, 0x600003ff) AM_WRITENOP
ADDRESS_MAP_END

// SH-1 or SH-2 almost certainly copies the program down to here: the ROM containing the program is 32-bit wide and the 68000 is 16-bit
// the SCSP is believed to be hardcoded to decode the first 4 MB like this for a master/slave config
// (see also Model 3):
static ADDRESS_MAP_START( system_h1_sound_map, ADDRESS_SPACE_PROGRAM, 16 )
	AM_RANGE(0x000000, 0x07ffff) AM_RAM
//  AM_RANGE(0x100000, 0x100fff) AM_DEVREADWRITE("scsp1", scsp_r, scsp_w)
	AM_RANGE(0x200000, 0x27ffff) AM_RAM
//  AM_RANGE(0x300000, 0x300fff) AM_DEVREADWRITE("scsp2", scsp_r, scsp_w)
ADDRESS_MAP_END



static const gfx_layout tiles8x8_layout =
{
	16,16,
	RGN_FRAC(1,1),
	8,
	{ 0, 1, 2, 3,4,5,6,7 },
	{ 16, 24, 0, 8,
	  48,56,32,40,
	  80,88,64,72,
	  112,120,96,104 },
	{ 0*128, 1*128, 2*128, 3*128, 4*128, 5*128, 6*128, 7*128, 8*128, 9*128, 10*128, 11*128, 12*128, 13*128, 14*128, 15*128 },
	16*128
};

static GFXDECODE_START( coolridr )
	GFXDECODE_ENTRY( "gfx1", 0, tiles8x8_layout, 0, 16 )
	GFXDECODE_ENTRY( "gfx2", 0, tiles8x8_layout, 0, 16 )
	GFXDECODE_ENTRY( "gfx3", 0, tiles8x8_layout, 0, 16 )
	GFXDECODE_ENTRY( "gfx4", 0, tiles8x8_layout, 0, 16 )
	GFXDECODE_ENTRY( "gfx5", 0, tiles8x8_layout, 0, 16 )

GFXDECODE_END


// IRQs 4 & 6 are valid on SH-2
static INTERRUPT_GEN( system_h1 )
{
//  if (cpu_getiloops(device))
//      cpu_set_input_line(device, 4, HOLD_LINE);
//  else
//      cpu_set_input_line(device, 6, HOLD_LINE);
}

//IRQs 10,12 and 14 are valid on SH-1 instead
static INTERRUPT_GEN( system_h1_sub )
{
	switch(cpu_getiloops(device))
	{
      	case 0:cpu_set_input_line(device, 0xa, HOLD_LINE); break;
        case 1:cpu_set_input_line(device, 0xc, HOLD_LINE); break;
        case 2:cpu_set_input_line(device, 0xe, HOLD_LINE); break;
	}
}

static MACHINE_RESET ( coolridr )
{

//  cputag_set_input_line(machine, "maincpu", INPUT_LINE_HALT, ASSERT_LINE);
	cputag_set_input_line(machine, "soundcpu", INPUT_LINE_HALT, ASSERT_LINE);

}

static MACHINE_DRIVER_START( coolridr )
	MDRV_CPU_ADD("maincpu", SH2, 28000000)	// ?? mhz
	MDRV_CPU_PROGRAM_MAP(system_h1_map)
	MDRV_CPU_VBLANK_INT_HACK(system_h1, 2)

	MDRV_CPU_ADD("soundcpu", M68000, 12000000)	// ?? mhz
	MDRV_CPU_PROGRAM_MAP(system_h1_sound_map)

	MDRV_CPU_ADD("sub", SH1, 8000000)	// SH7032 HD6417032F20!! ?? mhz
	MDRV_CPU_PROGRAM_MAP(coolridr_submap)
	MDRV_CPU_VBLANK_INT_HACK(system_h1_sub, 3)

	MDRV_GFXDECODE(coolridr)

	MDRV_SCREEN_ADD("screen", RASTER)
	MDRV_SCREEN_REFRESH_RATE(60)
	MDRV_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MDRV_SCREEN_FORMAT(BITMAP_FORMAT_RGB32)
	MDRV_SCREEN_SIZE(64*8, 64*8)
	MDRV_SCREEN_VISIBLE_AREA(0*8, 64*8-1, 0*8, 64*8-1)

	MDRV_PALETTE_LENGTH(0x10000)
	MDRV_MACHINE_RESET(coolridr)

	MDRV_VIDEO_START(coolridr)
	MDRV_VIDEO_UPDATE(coolridr)
MACHINE_DRIVER_END

ROM_START( coolridr )
	ROM_REGION( 0x200000, "maincpu", 0 ) /* SH2 code */
	ROM_LOAD32_WORD_SWAP( "ep17659.30", 0x0000000, 0x080000, CRC(473027b0) SHA1(acaa212869dd79550235171b9f054e82750f74c3) )
	ROM_LOAD32_WORD_SWAP( "ep17658.29", 0x0000002, 0x080000, CRC(7ecfdfcc) SHA1(97cb3e6cf9764c8db06de12e4e958148818ef737) )

	ROM_REGION( 0x100000, "soundcpu", 0 )	/* 68000 */
	ROM_LOAD32_WORD_SWAP( "ep17661.32", 0x0000000, 0x080000, CRC(81a7d90b) SHA1(99f8c3e75b94dd1b60455c26dc38ce08db82fe32) )
	ROM_LOAD32_WORD_SWAP( "ep17660.31", 0x0000002, 0x080000, CRC(27b7a507) SHA1(4c28b1d18d75630a73194b5d4fd166f3b647c595) )

	ROM_REGION( 0x100000, "sub", 0 ) /* SH1 */
	ROM_LOAD16_WORD_SWAP( "ep17662.12", 0x000000, 0x020000,  CRC(50d66b1f) SHA1(f7b7f2f5b403a13b162f941c338a3e1207762a0b) )

	ROM_REGION( 0x0400000, "gfx1", 0 ) /* Tiles. . at least 1 format */
	ROM_LOAD32_WORD_SWAP( "mp17650.11",0x000000, 0x0200000, CRC(0ccc84a1) SHA1(65951685b0c8073f6bd1cf9959e1b4d0fc6031d8) )
	ROM_LOAD32_WORD_SWAP( "mp17651.12",0x000002, 0x0200000, CRC(25fd7dde) SHA1(a1c3f3d947ce20fbf61ea7ab235259be9b7d35a8) )

	ROM_REGION( 0x0400000, "gfx2", 0 ) /* Tiles. . at least 1 format */
	ROM_LOAD32_WORD_SWAP( "mp17652.13",0x000000, 0x0200000, CRC(be9b4d05) SHA1(0252ba647434f69d6eacb4efc6f55e6af534c7c5) )
	ROM_LOAD32_WORD_SWAP( "mp17653.14",0x000002, 0x0200000, CRC(64d1406d) SHA1(779dbbf42a14a6be1de9afbae5bbb18f8f36ceb3) )

	ROM_REGION( 0x0400000, "gfx3", 0 ) /*Tiles. . at least 1 format + sprites (non-tile data)? */
	ROM_LOAD32_WORD_SWAP( "mp17654.15",0x000000, 0x0200000, CRC(5dee5cba) SHA1(6e6ec8574bdd35cc27903fc45f0d4a36ce9df103) )
	ROM_LOAD32_WORD_SWAP( "mp17655.16",0x000002, 0x0200000, CRC(02903cf2) SHA1(16d555fda144e0f1b62b428e9158a0e8ebf7084e) )

	ROM_REGION( 0x0400000, "gfx4", 0 ) /* sprites (non-tile data?) */
	ROM_LOAD32_WORD_SWAP( "mp17656.17",0x000000, 0x0200000, CRC(945c89e3) SHA1(8776d74f73898d948aae3c446d7c710ad0407603) )
	ROM_LOAD32_WORD_SWAP( "mp17657.18",0x000002, 0x0200000, CRC(74676b1f) SHA1(b4a9003a052bde93bebfa4bef9e8dff65003c3b2) )

	/* these 10 interleave somehow?? */
	ROM_REGION( 0x1600000, "gfx5", ROMREGION_ERASEFF ) /* Other Roms */
	ROMX_LOAD( "mp17640.1", 0x0000000, 0x0200000, CRC(5ecd98c7) SHA1(22027c1e9e6195d27f29a5779695d8597f68809e), ROM_SKIP(9) )
	ROMX_LOAD( "mp17641.2", 0x0000001, 0x0200000, CRC(a59b0605) SHA1(c93f84fd58f1942b40b7a55058e02a18a3dec3af), ROM_SKIP(9) )
	ROMX_LOAD( "mp17642.3", 0x0000002, 0x0200000, CRC(5f8a1827) SHA1(23179d751777436f2a4f652132001d5e425d8cd5), ROM_SKIP(9) )
	ROMX_LOAD( "mp17643.4", 0x0000003, 0x0200000, CRC(44a05dd0) SHA1(32aa86f8761ec6ffceb63979c44828603c244e7d), ROM_SKIP(9) )
	ROMX_LOAD( "mp17644.5", 0x0000004, 0x0200000, CRC(be2763c5) SHA1(1044b0a73e334337b0b9ac958df59480aedfb942), ROM_SKIP(9) )
	ROMX_LOAD( "mp17645.6", 0x0000005, 0x0200000, CRC(00954173) SHA1(863f32565296448ef10992dc3c0480411eb2b193), ROM_SKIP(9) )
	ROMX_LOAD( "mp17646.7", 0x0000006, 0x0200000, CRC(7ae4d92e) SHA1(8a0eaa5dce112289ac5d16ad5dc7f5895e71e87b), ROM_SKIP(9) )
	ROMX_LOAD( "mp17647.8", 0x0000007, 0x0200000, CRC(082faee8) SHA1(c047b8475517f96f481c09471a77aa0d103631d6), ROM_SKIP(9) )
	ROMX_LOAD( "mp17648.9", 0x0000008, 0x0200000, CRC(0791802f) SHA1(acad55bbd22c7e955a729c8abed9509fc6f10927), ROM_SKIP(9) )
	ROMX_LOAD( "mp17649.10",0x0000009, 0x0200000, CRC(567fbc0a) SHA1(3999c99b26f13d97ac1c58de00a44049ee7775fd), ROM_SKIP(9) )
ROM_END

/*TODO: there must be an irq line with custom vector located somewhere that writes to here...*/
static READ32_HANDLER( coolridr_hack_r )
{
	return 0;
}

static DRIVER_INIT( coolridr )
{
	memory_install_read32_handler(cputag_get_address_space(machine, "maincpu", ADDRESS_SPACE_PROGRAM), 0x60d88a4, 0x060d88a7, 0, 0, coolridr_hack_r );
}

GAME( 1995, coolridr,    0, coolridr,    0,    coolridr, ROT0,  "Sega", "Cool Riders",GAME_NOT_WORKING|GAME_NO_SOUND )
