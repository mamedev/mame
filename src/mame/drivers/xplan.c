/***************************************************************************

X-Plan (c) 2006 Subsino

***************************************************************************/

#include "emu.h"
#include "cpu/nec/nec.h"

static UINT16 *vram;

static VIDEO_START( xplan )
{

}

static VIDEO_UPDATE( xplan )
{
	int x,y,count;
	const gfx_element *gfx = screen->machine->gfx[0];

	count = 0;

	for (y=0;y<32;y++)
	{
		for (x=0;x<32;x++)
		{
			UINT16 tile = vram[count];

			drawgfx_opaque(bitmap,cliprect,gfx,tile,0,0,0,x*8,y*8);

			count++;
		}
	}


	return 0;
}

static WRITE8_HANDLER( paletteram_io_w )
{
	static int pal_offs,r,g,b,internal_pal_offs;

	switch(offset)
	{
		case 0:
			pal_offs = data;
			internal_pal_offs = 0;
			break;
		case 2: // RAMDAC MASK
			break;
		case 1:
			switch(internal_pal_offs)
			{
				case 0:
					r = ((data & 0x3f) << 2) | ((data & 0x30) >> 4);
					internal_pal_offs++;
					break;
				case 1:
					g = ((data & 0x3f) << 2) | ((data & 0x30) >> 4);
					internal_pal_offs++;
					break;
				case 2:
					b = ((data & 0x3f) << 2) | ((data & 0x30) >> 4);
					palette_set_color(space->machine, pal_offs, MAKE_RGB(r, g, b));
					internal_pal_offs = 0;
					pal_offs++;
					break;
			}

			break;
	}
}

static READ16_HANDLER( test_r )
{
	return space->machine->rand();
}

static ADDRESS_MAP_START( xplan_map, ADDRESS_SPACE_PROGRAM, 16 )
	AM_RANGE(0x00000, 0x0ffff) AM_RAM

	AM_RANGE(0x30000, 0x30fff) AM_RAM AM_BASE(&vram)
	AM_RANGE(0xc0000, 0xfffff) AM_ROM AM_REGION("boot_code", 0)
ADDRESS_MAP_END

static ADDRESS_MAP_START( xplan_io, ADDRESS_SPACE_IO, 16 )
	AM_RANGE(0x0060, 0x0063) AM_WRITE8( paletteram_io_w, 0xffff )

	AM_RANGE(0x00a0, 0x00a1) AM_WRITENOP

	AM_RANGE(0x0300, 0x0307) AM_READ(test_r) AM_WRITENOP // i/os, mux in there too
ADDRESS_MAP_END


static INPUT_PORTS_START( xplan )

INPUT_PORTS_END

static const gfx_layout charlayout =
{
	8,8,
	RGN_FRAC(1,1),
	8,
	{ 0, 1, 2, 3, 4, 5, 6, 7 },
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8 },
	{ 0*8*8, 1*8*8, 2*8*8, 3*8*8, 4*8*8, 5*8*8, 6*8*8, 7*8*8 },
	8*8*8
};

static GFXDECODE_START( xplan )
	GFXDECODE_ENTRY( "gfx1", 0, charlayout,     0, 1 )
GFXDECODE_END

static MACHINE_START( xplan )
{
}

static MACHINE_RESET( xplan )
{

}


static MACHINE_CONFIG_START( xplan, driver_device )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu",V33,12000000) //unknown CPU type
	MCFG_CPU_PROGRAM_MAP(xplan_map)
	MCFG_CPU_IO_MAP(xplan_io)

	MCFG_MACHINE_START(xplan)
	MCFG_MACHINE_RESET(xplan)

	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MCFG_SCREEN_FORMAT(BITMAP_FORMAT_INDEXED16)
	MCFG_SCREEN_SIZE(32*8, 32*8)
	MCFG_SCREEN_VISIBLE_AREA(0*8, 32*8-1, 0*8, 32*8-1)
	MCFG_GFXDECODE(xplan)

	MCFG_PALETTE_LENGTH(0x100)

	MCFG_VIDEO_START(xplan)
	MCFG_VIDEO_UPDATE(xplan)

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")
MACHINE_CONFIG_END

/***************************************************************************

  Game driver(s)

***************************************************************************/

ROM_START( xplan )
	ROM_REGION( 0x100000, "maincpu", ROMREGION_ERASEFF )

	ROM_REGION( 0x40000, "boot_code", 0 )
	ROM_LOAD( "x-plan_v101.u14",   0x0000, 0x40000, CRC(5a05fcb3) SHA1(9dffffd868e777f9436c38df76fa5247f4dd6daf) )

	ROM_REGION( 0x200000, "gfx1", 0 )
	ROM_LOAD32_BYTE( "x-plan_rom_3_v102b.u20",   0x000000, 0x80000, CRC(a027cbd1) SHA1(dac4226014794ef5bff84ddafee7da6691c00ece) )
	ROM_LOAD32_BYTE( "x-plan_rom_4_v102b.u19",   0x000001, 0x80000, CRC(744be318) SHA1(1c1f2a9e1da77d9bc1bf897072df44a681a53079) )
	ROM_LOAD32_BYTE( "x-plan_rom_5_v102b.u18",   0x000002, 0x80000, CRC(7e89c9b3) SHA1(9e3fea0d74cac48c068a15595f2342a2b0b3f747) )
	ROM_LOAD32_BYTE( "x-plan_rom_6_v102b.u17",   0x000003, 0x80000, CRC(a86ca3b9) SHA1(46aa86b9c62aa0a4e519eb06c72c2d540489afee) )

	ROM_REGION( 0x80000,  "samples", 0 )
	ROM_LOAD( "x-plan_rom_2_v100.u7",   0x000000, 0x80000, CRC(c742b5c8) SHA1(646960508be738824bfc578c1b21355c17e05010) )
ROM_END

GAME( 2006, xplan,  0,   xplan,  xplan,  0, ROT0, "Subsino", "X-Plan (v1.02)", GAME_NOT_WORKING | GAME_NO_SOUND )
