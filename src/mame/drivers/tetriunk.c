/*
 Tetris -  Unknown manufacturer

  PC based hardware

  David Haywood
  Tomasz Slanina

*/

#include "driver.h"
#include "deprecat.h"

static int crtindex;
static int crtdata[256];

static UINT8 * tetriunk_videoram;
static UINT8 * tetriunk_attribram;

static int bitmap_offset=0;

static READ8_HANDLER( tetriunk_random_r )
{
	return mame_rand(Machine);
}

static WRITE8_HANDLER(crt_index_w)
{
	crtindex=data;
}

static WRITE8_HANDLER(crt_data_w)
{
	crtdata[crtindex]=data;
}

static READ8_HANDLER(crt_index_r)
{
	return crtindex;
}

static READ8_HANDLER(crt_data_r)
{
	return crtdata[crtindex];
}

static WRITE8_HANDLER(txtram_w)
{
	int address=(crtdata[0x0e]<<8)+crtdata[0x0f];
	if(offset&1)
	{
		tetriunk_attribram[((offset>>1)+address)&0x1fff]=data;
	}
	else
	{
		tetriunk_videoram[((offset>>1)+address)&0x1fff]=data;
	}
}

static ADDRESS_MAP_START( tetriunk_io, ADDRESS_SPACE_IO, 8 )
	AM_RANGE(0x000, 0x00f) AM_RAM //AM_RANGE(0x000, 0x00f) AM_READWRITE(dma8237_0_r, dma8237_0_w)
	AM_RANGE(0x020, 0x021) AM_RAM //AM_RANGE(0x020, 0x021) AM_READWRITE(pic8259_0_r, pic8259_0_w)
	AM_RANGE(0x040, 0x043) AM_READ(tetriunk_random_r) AM_WRITENOP //AM_RANGE(0x040, 0x043) AM_READWRITE(pit8253_0_r, pit8253_0_w)
	AM_RANGE(0x061, 0x061) AM_READ(tetriunk_random_r) AM_WRITENOP //AM_RANGE(0x060, 0x063) AM_READWRITE(ppi8255_0_r, ppi8255_0_w)
	AM_RANGE(0x062, 0x063) AM_RAM
	AM_RANGE(0x0a0, 0x0a0) AM_RAM
	AM_RANGE(0x200, 0x207) AM_RAM //AM_READWRITE(pc_JOY_r, pc_JOY_w)
 	AM_RANGE(0x3b8, 0x3cf) AM_READ(tetriunk_random_r) AM_WRITENOP
	AM_RANGE(0x3c0, 0x3c0) AM_RAM
	AM_RANGE(0x3d4, 0x3d4) AM_READWRITE(crt_index_r, crt_index_w)
	AM_RANGE(0x3d5, 0x3d5) AM_READWRITE(crt_data_r, crt_data_w)
	AM_RANGE(0x3d8, 0x3d8) AM_NOP //AM_WRITE(crt_control_w)
	AM_RANGE(0x3d9, 0x3d9) AM_NOP //AM_WRITE(color_select_w)
ADDRESS_MAP_END

static ADDRESS_MAP_START( tetriunk_mem, ADDRESS_SPACE_PROGRAM, 8 )
	AM_RANGE(0x00000, 0x0ffff) AM_RAM
	AM_RANGE(0xb0000, 0xbffff) AM_READ(SMH_RAM) AM_WRITE(txtram_w)
	AM_RANGE(0xf0000, 0xfffff) AM_ROM
ADDRESS_MAP_END

static VIDEO_START(tetriunk)
{
}

static VIDEO_UPDATE(tetriunk)
{
	int x,y,z;
	int count = 0;
	UINT8 *region = memory_region(REGION_USER1);

	if(input_code_pressed_once(KEYCODE_Z))
	{
		bitmap_offset+=0x10000;
		bitmap_offset&=0xf0000;
	}

	if(input_code_pressed_once(KEYCODE_X))
	{
		bitmap_offset-=0x10000;
		bitmap_offset&=0xf0000;
	}

	for(y=0;y<200;y+=8)
	{
		for(z=0;z<8;z++)
		for(x=0;x<320;x++)
		{
			*BITMAP_ADDR16(bitmap, y+z, x) = region[y*320/8+x+z*0x2000+bitmap_offset+8];
		}
	}

	for (y=0;y<32;y++)
	{
		for (x=0;x<40;x++)
		{
			int tile = tetriunk_videoram[count<<1];
			int color = tetriunk_attribram[count<<1]&7;
			int color2 = (tetriunk_attribram[count<<1]>>4)&7;
			if(color2!=0)
			{
				drawgfx(bitmap,screen->machine->gfx[0],0x100,color2,0,0,x<<3,y<<3,cliprect,TRANSPARENCY_NONE,0);
			}
			drawgfx(bitmap,screen->machine->gfx[0],tile&0xff,color,0,0,x<<3,y<<3,cliprect,TRANSPARENCY_PEN,0);
			count++;
		}
	}
	return 0;
}

static const rgb_t tmpcolors[]= //fake colors
{
	MAKE_RGB(0x00,0x00,0x00),
	MAKE_RGB(0x00,0x00,0xaa),
	MAKE_RGB(0x00,0xaa,0x00),
	MAKE_RGB(0x00,0xaa,0xaa),
	MAKE_RGB(0xaa,0x00,0x00),
	MAKE_RGB(0xaa,0x00,0xaa),
	MAKE_RGB(0xaa,0xaa,0x00),
	MAKE_RGB(0xaa,0xaa,0xaa),
	MAKE_RGB(0x55,0x55,0x55),
	MAKE_RGB(0x55,0x55,0xff),
	MAKE_RGB(0x55,0xff,0x55),
	MAKE_RGB(0x55,0xff,0xff),
	MAKE_RGB(0xff,0x55,0x55),
	MAKE_RGB(0xff,0x55,0xff),
	MAKE_RGB(0xff,0xff,0x55),
	MAKE_RGB(0xff,0xff,0xff)
};

static PALETTE_INIT(tetrisunk)
{
	int i;
	for(i=0;i<16;i++)
	{
		palette_set_color(machine,i,tmpcolors[i]);
	}
}

static const gfx_layout tiles8x8_layout =
{
	8,8,
	RGN_FRAC(1,1),
	1,
	{ 0  },
	{ 0, 1, 2, 3, 4, 5, 6, 7 },
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8 },
	8*8
};

static GFXDECODE_START( tetriunk )
	GFXDECODE_ENTRY( REGION_GFX1, 0, tiles8x8_layout, 0, 16 )
GFXDECODE_END

static INPUT_PORTS_START( tetriunk )
INPUT_PORTS_END

static MACHINE_DRIVER_START( tetriunk )
	/* basic machine hardware */
	MDRV_CPU_ADD(I8088, 8000000)
	MDRV_CPU_PROGRAM_MAP(tetriunk_mem, 0)
	MDRV_CPU_IO_MAP(tetriunk_io, 0)

	/* video hardware */
	MDRV_SCREEN_ADD("main", RASTER)
	MDRV_SCREEN_REFRESH_RATE(60)
	MDRV_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MDRV_SCREEN_FORMAT(BITMAP_FORMAT_INDEXED16)
	MDRV_SCREEN_SIZE(320,200)
	MDRV_SCREEN_VISIBLE_AREA(0, 320-1, 0, 200-1)

	MDRV_GFXDECODE(tetriunk)
	MDRV_PALETTE_LENGTH(256)
	MDRV_PALETTE_INIT(tetrisunk)

	MDRV_VIDEO_START(tetriunk)
	MDRV_VIDEO_UPDATE(tetriunk)
MACHINE_DRIVER_END

ROM_START( tetriunk )
	ROM_REGION( 0x100000, REGION_CPU1, 0 ) /* code */
	ROM_LOAD( "b-10.u10", 0xf0000, 0x10000, CRC(efc2a0f6) SHA1(5f0f1e90237bee9b78184035a32055b059a91eb3) )

	ROM_REGION( 0x10000, REGION_GFX1,0 ) /* gfx - 1bpp font*/
	ROM_LOAD( "b-3.u36", 0x00000, 0x2000, CRC(1a636f9a) SHA1(a356cc57914d0c9b9127670b55d1f340e64b1ac9) )

	ROM_REGION( 0x80000, REGION_GFX2,ROMREGION_INVERT )
	ROM_LOAD( "b-1.u59", 0x00000, 0x10000, CRC(4719d986) SHA1(6e0499944b968d96fbbfa3ead6237d69c769d634))
	ROM_LOAD( "b-2.u58", 0x10000, 0x10000, CRC(599e1154) SHA1(14d99f90b4fedeab0ac24ffa9b1fd9ad0f0ba699))
	ROM_LOAD( "b-4.u54", 0x20000, 0x10000, CRC(e112c450) SHA1(dfdecfc6bd617ec520b7563b7caf44b79d498bd3))
	ROM_LOAD( "b-5.u53", 0x30000, 0x10000, CRC(050b7650) SHA1(5981dda4ed43b6e81fbe48bfba90a8775d5ecddf))
	ROM_LOAD( "b-6.u49", 0x40000, 0x10000, CRC(d596ceb0) SHA1(8c82fb638688971ef11159a6b240253e63f0949d))
	ROM_LOAD( "b-7.u48", 0x50000, 0x10000, CRC(79336b6c) SHA1(7a95875f3071bdc3ee25c0e6a5a3c00ef02dc977))
	ROM_LOAD( "b-8.u44", 0x60000, 0x10000, CRC(1f82121a) SHA1(106da0f39f1260d0761217ed0a24c1611bfd7f05))
	ROM_LOAD( "b-9.u43", 0x70000, 0x10000, CRC(4ea22349) SHA1(14dfd3dbd51f8bd6f3290293b8ea1c165e8cf7fd))

	ROM_REGION( 0x180000, REGION_USER1, ROMREGION_ERASEFF )
ROM_END

static DRIVER_INIT (tetriunk)
{
	int i,j,k;
	int index=0;
	UINT8 *region = memory_region(REGION_USER1);
	for(i=0;i<0x20000;i++)
	{
		//8 pixels/byte
		for(j=0;j<8;j++)
		{
			int mask=(1<<(7-j));
			int pixel=0;
			for(k=0;k<4;k++)
			{
				if(memory_region(REGION_GFX2)[k*0x20000+i]&mask)
				{
					pixel|=(1<<k);
				}
			}
			region[index++]=pixel;
		}
	}

	tetriunk_videoram=auto_malloc(0x2000);
	tetriunk_attribram=auto_malloc(0x2000);
}

GAME( 1989, tetriunk, 0,        tetriunk, tetriunk, tetriunk, ROT0,  "Unknown",      "Tetris (Unknown Manufacturer)"       , GAME_NOT_WORKING | GAME_NO_SOUND)
