/* TAS 5 REEL system? by Olympic Video Gaming */

#include "emu.h"
#include "cpu/m68000/m68000.h"

static UINT16* hotstuff_bitmapram;

VIDEO_START( hotstuff )
{

}

/* the first 0x20 bytes in every 0x200 (each line) of video ram are the colour data, providing a palette of 16 RGB444 colours for that line */

VIDEO_UPDATE( hotstuff )
{
	int count, y,yyy,x,xxx;
	UINT16 row_palette_data[0x10];
	rgb_t row_palette_data_as_rgb32_pen_data[0x10];

	yyy=512;xxx=512*2;

	count = 0;
	for (y = 0; y < yyy; y++)
	{
		// the current palette is stored in the first 0x20 bytes of each row!
		int p;

		for (p=0;p<0x10;p++)
		{
			row_palette_data[p] = hotstuff_bitmapram[count+p];

			row_palette_data_as_rgb32_pen_data[p] = MAKE_RGB( (row_palette_data[p] & 0x0f00)>>4, (row_palette_data[p] & 0x00f0)>>0, (row_palette_data[p] & 0x000f)<<4  );

		}

		for(x = 0; x < xxx; x++)
		{
			{
				*BITMAP_ADDR32(bitmap, y, x) = row_palette_data_as_rgb32_pen_data[(hotstuff_bitmapram[count] &0xf000)>>12];
				x++;
				*BITMAP_ADDR32(bitmap, y, x) = row_palette_data_as_rgb32_pen_data[(hotstuff_bitmapram[count] &0x0f00)>>8];
				x++;
				*BITMAP_ADDR32(bitmap, y, x) = row_palette_data_as_rgb32_pen_data[(hotstuff_bitmapram[count] &0x00f0)>>4];
				x++;
				*BITMAP_ADDR32(bitmap, y, x) = row_palette_data_as_rgb32_pen_data[(hotstuff_bitmapram[count] &0x000f)>>0];
			}

			count++;
		}
	}

	return 0;
}

static ADDRESS_MAP_START( hotstuff_map, ADDRESS_SPACE_PROGRAM, 16 )
	AM_RANGE(0x000000, 0x07ffff) AM_ROM

	AM_RANGE(0x400000, 0x40ffff) AM_RAM

	AM_RANGE(0x980000, 0x9bffff) AM_RAM AM_BASE(&hotstuff_bitmapram)
ADDRESS_MAP_END

static INPUT_PORTS_START( hotstuff )
INPUT_PORTS_END

static MACHINE_CONFIG_START( hotstuff, driver_device )

	MDRV_CPU_ADD("maincpu", M68000, 16000000)
	MDRV_CPU_PROGRAM_MAP(hotstuff_map)
	MDRV_CPU_VBLANK_INT("screen", irq1_line_hold)

	MDRV_SCREEN_ADD("screen", RASTER)
	MDRV_SCREEN_REFRESH_RATE(60)
	MDRV_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MDRV_SCREEN_FORMAT(BITMAP_FORMAT_RGB32)
	MDRV_SCREEN_SIZE(128*8, 64*8)
	MDRV_SCREEN_VISIBLE_AREA((0x10*4)+8, 101*8-1, 0*8, 33*8-1)

	MDRV_PALETTE_LENGTH(0x200)

	MDRV_VIDEO_START(hotstuff)
	MDRV_VIDEO_UPDATE(hotstuff)
MACHINE_CONFIG_END



ROM_START( hotstuff )
	ROM_REGION( 0x80000, "maincpu", 0 ) /* 68000 Code */
	ROM_LOAD16_WORD_SWAP( "hot stuff game u6 (68000).bin", 0x00000, 0x80000, CRC(65f6a72f) SHA1(3a6d489ec3bf351018e279605d42f10b0a2c61b1) )

	ROM_REGION( 0x80000, "data", 0 ) /* 68000 Data? */
	ROM_LOAD16_WORD_SWAP( "hot stuff symbol u8 (68000).bin", 0x00000, 0x80000, CRC(f154a157) SHA1(92ae0fb977e2dcc0377487d768f95c6e447e990b) )
ROM_END

GAME( ????, hotstuff,    0,        hotstuff,    hotstuff,    0, ROT0,  "Olympic Video Gaming", "Olympic Hot Stuff (TAS 5 Reel System)", GAME_NOT_WORKING | GAME_NO_SOUND )
