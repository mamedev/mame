/* Galxaia

Galaxia by Zaccaria (1979)

Taken from an untested board.

1K byte files were 2708 or equivalent.
512 byte file is the 82S130 colour PROM.

This is not a direct pirate of Galaxians as you might think from the name.
The game uses a Signetics 2650A CPU with three 40-pin 2636 chips. I have
no idea what 2636's are but I am hoping they are something to do with the
sound since the board has no apparent sound circuitry. The video hardware
looks like it's similar to Galaxians (2 x 2114, 2 x 2101, 2 x EPROM) but
there is no attack RAM and the graphics EPROMS are 2708. The graphics EPROMS
do contain Galaxian-like graphics...

---

TS 2008.08.12:
- fixed rom loading
- added preliminary video emulation

*/

#include "emu.h"
#include "video/s2636.h"
#include "cpu/s2650/s2650.h"

static UINT8 *galaxia_video;
static UINT8 *galaxia_color;

static VIDEO_UPDATE( galaxia )
{
	int x,y, count;

	bitmap_t *s2636_0_bitmap;
	bitmap_t *s2636_1_bitmap;
	bitmap_t *s2636_2_bitmap;

	running_device *s2636_0 = devtag_get_device(screen->machine, "s2636_0");
	running_device *s2636_1 = devtag_get_device(screen->machine, "s2636_1");
	running_device *s2636_2 = devtag_get_device(screen->machine, "s2636_2");

	count = 0;

	for (y=0;y<256/8;y++)
	{
		for (x=0;x<256/8;x++)
		{
			int tile = galaxia_video[count];
			drawgfx_opaque(bitmap,cliprect,screen->machine->gfx[0],tile,0,0,0,x*8,y*8);
			count++;
		}
	}

	s2636_0_bitmap = s2636_update(s2636_0, cliprect);
	s2636_1_bitmap = s2636_update(s2636_1, cliprect);
	s2636_2_bitmap = s2636_update(s2636_2, cliprect);

	/* copy the S2636 images into the main bitmap */
	{
		int y;

		for (y = cliprect->min_y; y <= cliprect->max_y; y++)
		{
			int x;

			for (x = cliprect->min_x; x <= cliprect->max_x; x++)
			{
				int pixel0 = *BITMAP_ADDR16(s2636_0_bitmap, y, x);
				int pixel1 = *BITMAP_ADDR16(s2636_1_bitmap, y, x);
				int pixel2 = *BITMAP_ADDR16(s2636_2_bitmap, y, x);

				if (S2636_IS_PIXEL_DRAWN(pixel0))
					*BITMAP_ADDR16(bitmap, y, x) = S2636_PIXEL_COLOR(pixel0);

				if (S2636_IS_PIXEL_DRAWN(pixel1))
					*BITMAP_ADDR16(bitmap, y, x) = S2636_PIXEL_COLOR(pixel1);

				if (S2636_IS_PIXEL_DRAWN(pixel2))
					*BITMAP_ADDR16(bitmap, y, x) = S2636_PIXEL_COLOR(pixel2);
			}
		}
	}
	return 0;
}

static WRITE8_HANDLER(galaxia_video_w)
{
	if (cpu_get_reg(space->cpu, S2650_FO))
	{
		galaxia_video[offset]=data;
	}
	else
	{
		galaxia_color[offset]=data;
	}
}

static READ8_HANDLER(galaxia_video_r)
{
	return galaxia_video[offset];
}

static ADDRESS_MAP_START( mem_map, ADDRESS_SPACE_PROGRAM, 8 )
	AM_RANGE(0x0000, 0x13ff) AM_ROM
	AM_RANGE(0x1400, 0x14ff) AM_MIRROR(0x6000) AM_RAM
	AM_RANGE(0x1500, 0x15ff) AM_MIRROR(0x6000) AM_DEVREADWRITE("s2636_0", s2636_work_ram_r, s2636_work_ram_w)
	AM_RANGE(0x1600, 0x16ff) AM_MIRROR(0x6000) AM_DEVREADWRITE("s2636_1", s2636_work_ram_r, s2636_work_ram_w)
	AM_RANGE(0x1700, 0x17ff) AM_MIRROR(0x6000) AM_DEVREADWRITE("s2636_2", s2636_work_ram_r, s2636_work_ram_w)
	AM_RANGE(0x1800, 0x1bff) AM_MIRROR(0x6000) AM_READWRITE(galaxia_video_r, galaxia_video_w)  AM_BASE(&galaxia_video)
	AM_RANGE(0x1c00, 0x1fff) AM_MIRROR(0x6000) AM_RAM
	AM_RANGE(0x2000, 0x33ff) AM_ROM
	AM_RANGE(0x7214, 0x7214) AM_READ_PORT("IN0")
ADDRESS_MAP_END

static ADDRESS_MAP_START( io_map, ADDRESS_SPACE_IO, 8 )
	ADDRESS_MAP_GLOBAL_MASK(0xff)
	AM_RANGE(0x00, 0x00) AM_READ_PORT("IN7")
	AM_RANGE(0x01, 0x01) AM_READ_PORT("IN1")
	AM_RANGE(0x02, 0x02) AM_READ_PORT("IN2")
	AM_RANGE(0x05, 0x05) AM_READ_PORT("IN5")
	AM_RANGE(0x06, 0x06) AM_READ_PORT("IN6")
	AM_RANGE(0xac, 0xac) AM_READ_PORT("IN3")
ADDRESS_MAP_END


static ADDRESS_MAP_START( astrowar_mem, ADDRESS_SPACE_PROGRAM, 8 )
	AM_RANGE(0x0000, 0x13ff) AM_ROM
	AM_RANGE(0x1400, 0x14ff) AM_MIRROR(0x6000) AM_RAM
	AM_RANGE(0x1500, 0x15ff) AM_MIRROR(0x6000) AM_DEVREADWRITE("s2636_0", s2636_work_ram_r, s2636_work_ram_w)
	AM_RANGE(0x1600, 0x16ff) AM_MIRROR(0x6000) AM_DEVREADWRITE("s2636_1", s2636_work_ram_r, s2636_work_ram_w)
	AM_RANGE(0x1700, 0x17ff) AM_MIRROR(0x6000) AM_DEVREADWRITE("s2636_2", s2636_work_ram_r, s2636_work_ram_w)
	AM_RANGE(0x1800, 0x1bff) AM_MIRROR(0x6000) AM_READWRITE(galaxia_video_r, galaxia_video_w)  AM_BASE(&galaxia_video)
	AM_RANGE(0x1c00, 0x1fff) AM_MIRROR(0x6000) AM_RAM
	AM_RANGE(0x2000, 0x33ff) AM_ROM
ADDRESS_MAP_END

static ADDRESS_MAP_START( astrowar_io, ADDRESS_SPACE_IO, 8 )
	ADDRESS_MAP_GLOBAL_MASK(0xff)
	AM_RANGE(0x00, 0x00) AM_READ_PORT("IN0")
	AM_RANGE(0x01, 0x01) AM_READ_PORT("IN1")
	AM_RANGE(0x02, 0x02) AM_READ_PORT("IN2")
	AM_RANGE(0x06, 0x06) AM_READ_PORT("IN6")
	AM_RANGE(0x07, 0x07) AM_READ_PORT("IN7")
ADDRESS_MAP_END

static INPUT_PORTS_START( galaxia )
	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_START1 )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_START2 )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_COIN2 ) PORT_IMPULSE(1)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_COIN1 ) PORT_IMPULSE(1)
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_COCKTAIL
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_BUTTON1 )
	PORT_BIT( 0xc0, IP_ACTIVE_HIGH, IPT_UNKNOWN )

	PORT_START("IN1")
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("IN2")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_2WAY PORT_COCKTAIL
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_2WAY PORT_COCKTAIL
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_2WAY
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_2WAY
	PORT_DIPNAME( 0x80, 0x80, "Freeze" )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_BIT( 0x43, IP_ACTIVE_HIGH, IPT_UNKNOWN )

	PORT_START("IN3")
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("IN5")
	PORT_BIT( 0xff, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("IN6")
	PORT_DIPNAME( 0x07, 0x00, DEF_STR( Coinage ) )
	PORT_DIPSETTING(    0x00, "A 1C_1C B 2C_1C" )
	PORT_DIPSETTING(    0x01, "A 1C_2C B 2C_1C" )
	PORT_DIPSETTING(    0x02, "A 1C_3C B 2C_1C" )
	PORT_DIPSETTING(    0x03, "A 1C_5C B 2C_1C" )
	PORT_DIPSETTING(    0x04, "A 1C_1C B 1C_1C" )
	PORT_DIPSETTING(    0x05, "A 1C_2C B 1C_1C" )
	PORT_DIPSETTING(    0x06, "A 1C_3C B 1C_1C" )
	PORT_DIPSETTING(    0x07, "A 1C_5C B 1C_1C" )
	PORT_DIPNAME( 0x08, 0x00, DEF_STR( Lives ) )
	PORT_DIPSETTING(    0x00, "3" )
	PORT_DIPSETTING(    0x08, "5" )
	PORT_BIT( 0xf0, IP_ACTIVE_HIGH, IPT_UNKNOWN )

	PORT_START("IN7")
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNUSED )
INPUT_PORTS_END

static const gfx_layout tiles8x8x1_layout =
{
	8,8,
	RGN_FRAC(1,1),
	1,
	{ RGN_FRAC(0,1) },
	{ 0, 1, 2, 3, 4, 5, 6, 7 },
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8 },
	8*8
};

static const gfx_layout tiles8x8x2_layout =
{
	8,8,
	RGN_FRAC(1,2),
	2,
	{ RGN_FRAC(0,2), RGN_FRAC(1,2) },
	{ 0, 1, 2, 3, 4, 5, 6, 7 },
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8 },
	8*8
};

static GFXDECODE_START( galaxia )
	GFXDECODE_ENTRY( "gfx1", 0, tiles8x8x2_layout, 0, 16 )
GFXDECODE_END

static GFXDECODE_START( astrowar )
	GFXDECODE_ENTRY( "gfx1", 0, tiles8x8x1_layout, 0, 16 )
GFXDECODE_END


static INTERRUPT_GEN( galaxia_interrupt )
{
	cpu_set_input_line_and_vector(device, 0, HOLD_LINE, 0x03);
}


static const s2636_interface s2636_0_config =
{
	"screen",
	0x100,
	3, -27
};

static const s2636_interface s2636_1_config =
{
	"screen",
	0x100,
	3, -27
};

static const s2636_interface s2636_2_config =
{
	"screen",
	0x100,
	3, -27
};

static MACHINE_DRIVER_START( galaxia )
	/* basic machine hardware */
	MDRV_CPU_ADD("maincpu", S2650,2000000)		 /* ? MHz */
	MDRV_CPU_PROGRAM_MAP(mem_map)
	MDRV_CPU_IO_MAP(io_map)
	MDRV_CPU_VBLANK_INT("screen", galaxia_interrupt)

	/* video hardware */
	MDRV_SCREEN_ADD("screen", RASTER)
	MDRV_SCREEN_REFRESH_RATE(60)
	MDRV_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MDRV_SCREEN_FORMAT(BITMAP_FORMAT_INDEXED16)
	MDRV_SCREEN_SIZE(256, 256)
	MDRV_SCREEN_VISIBLE_AREA(0, 256-1, 0, 256-1)

	MDRV_GFXDECODE(galaxia)
	MDRV_PALETTE_LENGTH(0x100)

	MDRV_S2636_ADD("s2636_0", s2636_0_config)
	MDRV_S2636_ADD("s2636_1", s2636_1_config)
	MDRV_S2636_ADD("s2636_2", s2636_2_config)

	MDRV_VIDEO_UPDATE(galaxia)
MACHINE_DRIVER_END

static MACHINE_DRIVER_START( astrowar )
	MDRV_IMPORT_FROM( galaxia )
	MDRV_CPU_MODIFY("maincpu")
	MDRV_CPU_PROGRAM_MAP(astrowar_mem)
	MDRV_CPU_IO_MAP(astrowar_io)
	MDRV_GFXDECODE(astrowar)
MACHINE_DRIVER_END

ROM_START( galaxia )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "08h.bin", 0x00000, 0x0400, CRC(f3b4ffde) SHA1(15b004e7821bfc145158b1e9435f061c524f6b86) )
	ROM_LOAD( "10h.bin", 0x00400, 0x0400, CRC(6d07fdd4) SHA1(d7d4b345a055275d59951788569db370bccd5195) )
	ROM_LOAD( "11h.bin", 0x00800, 0x0400, CRC(1520eb3d) SHA1(3683174da701e1124af0f9c2ee4a9a84f3fea33a) )
	ROM_LOAD( "13h.bin", 0x00c00, 0x0400, CRC(c4482770) SHA1(aee983cc3d80989f49aea4138961bb623039484a) )
	ROM_LOAD( "08i.bin", 0x01000, 0x0400, CRC(45b88599) SHA1(3b79c21db1aa9d80fac81ac5a554e438805febd1) )
	ROM_LOAD( "10i.bin", 0x02000, 0x0400, CRC(c0baa654) SHA1(80e0880c32ad285fbce0f7f552268b964b97cab3) )
	ROM_LOAD( "11i.bin", 0x02400, 0x0400, CRC(4456808a) SHA1(f9e8cfdde0e17f13f1be297b2b4503ccc959b33c) )
	ROM_LOAD( "13i.bin", 0x02800, 0x0400, CRC(cf653b9a) SHA1(fef5943de60cb5ba2459fc6ae7419e29c96a76cd) )
	ROM_LOAD( "11l.bin", 0x02c00, 0x0400, CRC(50c6a645) SHA1(46638907bc393df6be25fc7461d73047d1746ffc) )
	ROM_LOAD( "13l.bin", 0x03000, 0x0400, CRC(3a9c38c7) SHA1(d1e934092b69c0f3f9636eba05a1d8a6d9588e6b) )

	ROM_REGION( 0x0800, "gfx1", 0 )
	ROM_LOAD( "01d.bin", 0x00000, 0x0400, CRC(2dd50aab) SHA1(758d7a5383c9a1ee134d99e3f7025819cfbe0e0f) )
	ROM_LOAD( "03d.bin", 0x00400, 0x0400, CRC(1dc30185) SHA1(e3c75eecb80b376ece98f602e1b9587487841824) )

	ROM_REGION( 0x80000, "proms", 0 )
	ROM_LOAD( "11o", 0x00000, 0x0200, CRC(ae816417) SHA1(9497857d13c943a2735c3b85798199054e613b2c) )
ROM_END


ROM_START( astrowar )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "astro8h.rom",  0x00000, 0x0400, CRC(b0ec246c) SHA1(f9123b5e317938655f5e8b3f8a5810d0b2b7c7af) )
	ROM_LOAD( "astro10h.rom", 0x00400, 0x0400, CRC(090d360f) SHA1(528ddcdc30a5a291bd8850ff6f134fcc19af562f) )
	ROM_LOAD( "astro11h.rom", 0x00800, 0x0400, CRC(72ab1378) SHA1(50743c64c4775076aa6f1d8ab2e05c14884bf0ba) )
	ROM_LOAD( "astro13h.rom", 0x00c00, 0x0400, CRC(2dc4c895) SHA1(831afbfd4ebfd6522ab0758222bc6f9826148a5d) )
	ROM_LOAD( "astro8i.rom",  0x01000, 0x0400, CRC(ab87fbfc) SHA1(34b670f96c260f186c643e588995ae5d80377784) )
	ROM_LOAD( "astro10i.rom", 0x02000, 0x0400, CRC(533675c1) SHA1(69cc066e1874a135a53a21b7b2461bda456504f1) )
	ROM_LOAD( "astro11i.rom", 0x02400, 0x0400, CRC(59cf8901) SHA1(e849d4c99350b7e3453c156d91618b71b5be1163) )
	ROM_LOAD( "astro13i.rom", 0x02800, 0x0400, CRC(5149c121) SHA1(232ba594e283fb25c31d8ae0b7d8315a81852a71) )
	ROM_LOAD( "astro11l.rom", 0x02c00, 0x0400, CRC(29f52f57) SHA1(5cb50b82e09c537eeaeae167351fca686fde8228) )
	ROM_LOAD( "astro13l.rom", 0x03000, 0x0400, CRC(882cdb87) SHA1(062ee8d296316cbce2eb62e72774aa4181e9847d) )

	ROM_REGION( 0x0800, "gfx1", 0 )
	ROM_LOAD( "astro1d.rom",  0x00000, 0x0400, CRC(6053f834) SHA1(e0b76800c241b3c8010c09869cecbc109b25310a) )
	ROM_LOAD( "astro3d.rom",  0x00400, 0x0400, CRC(822505aa) SHA1(f9d3465e14bb850a286f8b4f42aa0a4044413b67) )

	ROM_REGION( 0x80000, "proms", 0 )
	ROM_LOAD( "11o.rom", 0x00000, 0x0200, NO_DUMP ) /* a rom is missing */
ROM_END


static DRIVER_INIT(galaxia)
{
	galaxia_color=auto_alloc_array(machine, UINT8, 0x400);
}

GAME( 1979, galaxia, 0, galaxia, galaxia, galaxia, ROT90, "Zaccaria", "Galaxia", GAME_NOT_WORKING|GAME_NO_SOUND )
GAME( 1980, astrowar, 0, astrowar, galaxia, galaxia, ROT90, "Zaccaria", "Astro Wars", GAME_NOT_WORKING|GAME_NO_SOUND|GAME_WRONG_COLORS )
