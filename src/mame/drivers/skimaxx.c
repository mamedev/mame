/***************************************************************************

    Skimaxx

    Preliminary driver by Phil Bennett

    68030-40 * 2
    TMS34010-50
    MSM6295 * 4

    Video hardware consists of scaling sprites with a Mode 7-style ground?
    TMS34010 seems to draws text, score etc.

    It looks like a bistream for a Xilinx FPGA is stored in ROM. The sub
    CPU bit-bangs 30864 bits worth of data to 0x40000003 twice.

***************************************************************************/

#include "driver.h"
#include "cpu/m68000/m68000.h"
#include "cpu/tms34010/tms34010.h"


/*************************************
 *
 *  Statics
 *
 *************************************/

static UINT32 *sprite_buffer;
static UINT16 *frame_buffer;


/*************************************
 *
 *  Video emulation
 *
 *************************************/

static VIDEO_START( skimaxx )
{

}


/*************************************
 *
 *  VRAM shift register callbacks
 *
 *************************************/

// TODO: Might not be used
static void skimaxx_to_shiftreg(const address_space *space, UINT32 address, UINT16 *shiftreg)
{
	memcpy(shiftreg, &frame_buffer[TOWORD(address)], 512 * sizeof(UINT16));
}

static void skimaxx_from_shiftreg(const address_space *space, UINT32 address, UINT16 *shiftreg)
{
	memcpy(&frame_buffer[TOWORD(address)], shiftreg, 512 * sizeof(UINT16));
}


/*************************************
 *
 *  Main video refresh
 *
 *************************************/

static void skimaxx_scanline_update(const device_config *screen, bitmap_t *bitmap, int scanline, const tms34010_display_params *params)
{
	// TODO: This isn't correct. I just hacked it together quickly so I could see something!
	if (params->rowaddr >= 0x220)
	{
		UINT32 rowaddr = (params->rowaddr - 0x220);
		UINT16 *vram1 = &frame_buffer[rowaddr << 8];
		UINT16 *dest = BITMAP_ADDR16(bitmap, scanline, 0);
		int coladdr = params->coladdr;
		int x;
		coladdr = 0;

		if (scanline >= 236)
			return;

		for (x = params->heblnk; x < params->hsblnk; ++x)
		{
			UINT16 tmspix = vram1[coladdr++] & 0x7fff;
			dest[x] = tmspix;
		}
	}
}


/*************************************
 *
 *  TMS34010 host interface
 *
 *************************************/

static WRITE32_HANDLER( m68k_tms_w )
{
	tms34010_host_w(cputag_get_cpu(space->machine, "tms"), offset, data);
}

static READ32_HANDLER( m68k_tms_r )
{
	return tms34010_host_r(cputag_get_cpu(space->machine, "tms"), offset);
}


/*************************************
 *
 *  Handlers
 *
 *************************************/

static READ32_HANDLER( unk_r )
{
	return 0x80;
}


/*************************************
 *
 *  Main CPU memory map
 *
 *************************************/

static ADDRESS_MAP_START( 68030_1_map, ADDRESS_SPACE_PROGRAM, 32 )
	AM_RANGE(0x00000000, 0x001fffff) AM_ROM
	AM_RANGE(0x10000000, 0x10000003) AM_RAM
	AM_RANGE(0x10100000, 0x1010000f) AM_READWRITE(m68k_tms_r, m68k_tms_w)
	AM_RANGE(0x10180000, 0x10187fff) AM_RAM AM_SHARE(1)
	AM_RANGE(0x20000000, 0x20000003) AM_RAM
	AM_RANGE(0x20000020, 0x20000023) AM_RAM
	AM_RANGE(0x20000024, 0x20000027) AM_RAM
	AM_RANGE(0x20000040, 0x20000043) AM_RAM
	AM_RANGE(0x20000044, 0x20000047) AM_RAM
	AM_RANGE(0x20000048, 0x2000004b) AM_RAM
	AM_RANGE(0x2000004c, 0x2000004f) AM_READ(unk_r)
	AM_RANGE(0x20000050, 0x20000053) AM_RAM
	AM_RANGE(0x20000054, 0x20000057) AM_RAM
	AM_RANGE(0xfffc0000, 0xfffdffff) AM_RAM AM_MIRROR(0x00020000)
ADDRESS_MAP_END


/*************************************
 *
 *  Sub CPU memory map
 *
 *************************************/

static ADDRESS_MAP_START( 68030_2_map, ADDRESS_SPACE_PROGRAM, 32 )
	AM_RANGE(0x00000000, 0x003fffff) AM_ROM
	AM_RANGE(0x40000000, 0x40000003) AM_RAM // Xilinx FPGA configuration write port?
	AM_RANGE(0x50000000, 0x5007ffff) AM_RAM AM_BASE(&sprite_buffer) // Part of this is mixed with the TMS frame buffer
	AM_RANGE(0xfffc0000, 0xfffc7fff) AM_RAM AM_SHARE(1)
	AM_RANGE(0xfffe0000, 0xffffffff) AM_RAM // I think this is banked with the shared RAM? (see CPU sync routines)
ADDRESS_MAP_END


/*************************************
 *
 *  Video CPU memory map
 *
 *************************************/

static ADDRESS_MAP_START( tms_program_map, ADDRESS_SPACE_PROGRAM, 16 )
	AM_RANGE(0x00000000, 0x000100ff) AM_RAM
	AM_RANGE(0x00008000, 0x0003ffff) AM_RAM
	AM_RANGE(0x00050000, 0x0005ffff) AM_RAM
	AM_RANGE(0x00220000, 0x003fffff) AM_RAM AM_BASE(&frame_buffer)
	AM_RANGE(0x02000000, 0x0200000f) AM_RAM
	AM_RANGE(0x02100000, 0x0210000f) AM_RAM
	AM_RANGE(0x04000000, 0x047fffff) AM_ROM AM_REGION("tmsgfx", 0)
	AM_RANGE(0xc0000000, 0xc00001ff) AM_READWRITE(tms34010_io_register_r, tms34010_io_register_w)
	AM_RANGE(0xff800000, 0xffffffff) AM_ROM AM_REGION("tms", 0)
ADDRESS_MAP_END


/*************************************
 *
 *  Input definitions
 *
 *************************************/

static INPUT_PORTS_START( skimaxx )
	PORT_START("DSW")
	// TODO
INPUT_PORTS_END


/*************************************
 *
 *  TMS34010 configuration
 *
 *************************************/

static void skimaxx_tms_irq(const device_config *device, int state)
{
	// TODO
}

static const tms34010_config tms_config =
{
	FALSE,                     /* halt on reset */
	"screen",                  /* the screen operated on */
	50000000/8,                /* pixel clock */
	1,                         /* pixels per clock */
	skimaxx_scanline_update,   /* scanline updater */
	skimaxx_tms_irq,           /* generate interrupt */
	skimaxx_to_shiftreg,       /* write to shiftreg function */
	skimaxx_from_shiftreg      /* read from shiftreg function */
};


/*************************************
 *
 *  Initialisation
 *
 *************************************/

static MACHINE_RESET( skimaxx )
{

}


/*************************************
 *
 *  Machine driver
 *
 *************************************/

static MACHINE_DRIVER_START( skimaxx )
	MDRV_CPU_ADD("maincpu", M68020, XTAL_40MHz)	// TODO: Should be a 68030!
	MDRV_CPU_PROGRAM_MAP(68030_1_map)
//  MDRV_CPU_VBLANK_INT("screen", irq3_line_hold)

	MDRV_CPU_ADD("subcpu", M68020, XTAL_40MHz)
	MDRV_CPU_PROGRAM_MAP(68030_2_map)		// TODO: Should be a 68030!

	MDRV_CPU_ADD("tms", TMS34010, XTAL_50MHz)
	MDRV_CPU_CONFIG(tms_config)
	MDRV_CPU_PROGRAM_MAP(tms_program_map)

	MDRV_MACHINE_RESET(skimaxx)

	MDRV_SCREEN_ADD("screen", RASTER)
	MDRV_SCREEN_FORMAT(BITMAP_FORMAT_INDEXED16)
	MDRV_SCREEN_RAW_PARAMS(40000000/4, 156*4, 0, 100*4, 328, 0, 300) // TODO - Wrong but TMS overrides it anyway

	MDRV_PALETTE_INIT(RRRRR_GGGGG_BBBBB)
	MDRV_PALETTE_LENGTH(32768)

	MDRV_SPEAKER_STANDARD_MONO("mono")

	MDRV_VIDEO_START(skimaxx)
	MDRV_VIDEO_UPDATE(tms340x0)
MACHINE_DRIVER_END


/*************************************
 *
 *  ROM definition(s)
 *
 *************************************/

ROM_START( skimaxx )
	ROM_REGION( 0x200000, "maincpu", 0 )
	ROM_LOAD32_BYTE( "main2_0.ve2", 0x000000, 0x80000, CRC(0e139138) SHA1(b28b0f5dba7810712cee99a9a9748ca2dabddf15) )
	ROM_LOAD32_BYTE( "main2_0.ve4", 0x000001, 0x80000, CRC(4e0502cf) SHA1(451efd82656dd32e696f572d848b2717453f05c8) )
	ROM_LOAD32_BYTE( "main2_0.ve3", 0x000002, 0x80000, CRC(53887cfd) SHA1(79d9547e0e9c4c41913279f977bf25cdbe800356) )
	ROM_LOAD32_BYTE( "main2_0.ve5", 0x000003, 0x80000, CRC(6317f54d) SHA1(057141acd2a8a804d8cc0487d5cacc286c344866) )

	ROM_REGION( 0x400000, "subcpu", 0 )
	ROM_LOAD32_BYTE( "ve1v-2_0", 0x000003, 0x100000, CRC(871e16d2) SHA1(9318ebcff4032fffcbe53c8fb805b14578eaebac) )
	ROM_LOAD32_BYTE( "ve2v-2_0", 0x000002, 0x100000, CRC(17b01a96) SHA1(79ef3bd6a22abf774095741c29deeb6af53d1585) )
	ROM_LOAD32_BYTE( "ve3v-2_0", 0x000001, 0x100000, CRC(c1491795) SHA1(3a527ea4bf8cfdab072153b1cd66b0d090ef4d31) )
	ROM_LOAD32_BYTE( "ve4v-2_0", 0x000000, 0x100000, CRC(950773bb) SHA1(68e4a5780701488a934104eb87d10c36d736f049) )

	ROM_REGION16_LE( 0x100000, "tms", 0 )
	ROM_LOAD16_BYTE( "vc8v-2_0", 0x000000, 0x80000, CRC(a6e9ef81) SHA1(59a0fb149e17d3773adb980428a0b107647bd4fa) )
	ROM_LOAD16_BYTE( "vc9v-2_0", 0x000001, 0x80000, CRC(b1e8ba65) SHA1(b91cf93ecd9b6067664780ab2c1b69f632c7ae05) )

	ROM_REGION( 0x600000, "tmsgfx", 0 )
	ROM_LOAD16_BYTE( "vc10v2_0", 0x000000, 0x80000, CRC(433651cd) SHA1(9b9801703d16adbbca2b03e1714490fb166d48a0) )
	ROM_LOAD16_BYTE( "vc11v2_0", 0x000001, 0x80000, CRC(a906fc72) SHA1(c61ad560f203c7f507cc7b7dc8834f529a6501a7) )

	ROM_REGION( 0x600000, "gfx", 0 )
	ROM_LOAD16_BYTE( "ve5v-2_0", 0x000000, 0x100000, CRC(b95544a3) SHA1(1ad49da54d36543229e241887c3dd8411ef5c89a) )
	ROM_LOAD16_BYTE( "ve6v-2_0", 0x000001, 0x100000, CRC(d98bf89a) SHA1(f7fd66487f7696b53ee90a4cf45a810ec791df59) )
	ROM_LOAD16_BYTE( "ve7v-2_0", 0x200000, 0x100000, CRC(a927716b) SHA1(0189d7b08b65b6a8a8d4c3e2affe93612db38155) )
	ROM_LOAD16_BYTE( "ve8v-2_0", 0x200001, 0x100000, CRC(ad8ed4a4) SHA1(5d1339b3d6ce59fea062273fa2da35988ba94a80) )
	ROM_LOAD16_BYTE( "ve9v-2_0", 0x400000, 0x100000, CRC(587b364d) SHA1(b53412bdcc804727990d959e5b2399a7e5e7fbf3) )
	ROM_LOAD16_BYTE( "ve10v2_0", 0x400001, 0x100000, CRC(c2a16a62) SHA1(3663c19a6517b0a01fb454523b995be3cdf2e4b3) )

	ROM_REGION( 0x80000, "oki1", 0 )
	ROM_LOAD( "main2_0.u2", 0x000000, 0x80000, CRC(c84b3c46) SHA1(b956358518495aa822a5b699cbad1abac212dd09) )

	ROM_REGION( 0x80000, "oki2", 0 )
	ROM_LOAD( "main2_0.u4", 0x000000, 0x80000, CRC(c84b3c46) SHA1(b956358518495aa822a5b699cbad1abac212dd09) )

	ROM_REGION( 0x80000, "oki3", 0 )
	ROM_LOAD( "main2_0.u3", 0x000000, 0x80000, CRC(24d8c6ad) SHA1(06f51a4c380c91c930d646826246f62c4e1f9cda) )

	ROM_REGION( 0x80000, "oki4", 0 )
	ROM_LOAD( "main2_0.u5", 0x000000, 0x80000, CRC(e2ba07ad) SHA1(cf82753975f7b6756cca4e10b5372e00135440bf) )
ROM_END


/*************************************
 *
 *  Game driver
 *
 *************************************/

GAME( 1996, skimaxx, 0, skimaxx, skimaxx, 0, ROT0, "Kyle Hodgetts/ICE", "Skimaxx", GAME_NOT_WORKING | GAME_NO_SOUND )
