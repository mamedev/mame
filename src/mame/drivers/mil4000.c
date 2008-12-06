/*******************************************************************************************

Millenium Nuovo 4000 / Nuovo Millenium 4000 (c) 2000 Sure Milano

driver by David Haywood and Angelo Salese

Notes:
-At first start-up,an Italian msg pops up: "(translated) pcb has been hacked from external
agent,it's advised to add an anti-spark device". Press F2 to enter Service Mode,then press
Hold 5 to exit Service Mode.
-HW name (stamped on the pcb) is "CHP4";

TODO:
-Add Touch Screen support;
-H/V-blank bits emulation;
-Inputs/outputs are grossly mapped;
-Protection PIC is unused?

============================================================================================

Manufacturer: Sure
Revision number: CHP4 1.5

CPU
1x PIC16C65B (u60)(read protected)
1x MC68HC000FN12 (u61)
1x U6295 (u53)(equivalent to M6295)
1x resonator 1000j (close to 6295)
1x oscillator 12.000MHz
1x oscillator 14.31818MHz

ROMs
1x 27C020 (1)
7x V29C51001T (2,3,4,5,6,27,28)
1x PALCE22V10H (u74)(read protected)
2x A40MX04-PL840010 (u2,u3)(read protected)

Note
1x 28x2 edge connector
1x RS232 9pins connector
1x trimmer (volume)
1x 2positon jumper
1x pushbutton (reset)

*******************************************************************************************/

#include "driver.h"
#include "cpu/m68000/m68000.h"
#include "sound/okim6295.h"

static UINT16 *sc0_vram,*sc1_vram,*sc2_vram,*sc3_vram;
static tilemap *sc0_tilemap,*sc1_tilemap,*sc2_tilemap,*sc3_tilemap;

static TILE_GET_INFO( get_sc0_tile_info )
{
	UINT32 data = (sc0_vram[tile_index*2]<<16) | sc0_vram[tile_index*2+1];
	int tile = data >> 14;
	int color = (sc0_vram[tile_index*2+1] & 0x1f)+0;

	SET_TILE_INFO(
			0,
			tile,
			color,
			0);
}

static TILE_GET_INFO( get_sc1_tile_info )
{
	UINT32 data = (sc1_vram[tile_index*2]<<16) | sc1_vram[tile_index*2+1];
	int tile = data >> 14;
	int color = (sc1_vram[tile_index*2+1] & 0x1f)+0x10;

	SET_TILE_INFO(
			0,
			tile,
			color,
			0);
}

static TILE_GET_INFO( get_sc2_tile_info )
{
	UINT32 data = (sc2_vram[tile_index*2]<<16) | sc2_vram[tile_index*2+1];
	int tile = data >> 14;
	int color = (sc2_vram[tile_index*2+1] & 0x1f)+0x20;

	SET_TILE_INFO(
			0,
			tile,
			color,
			0);
}

static TILE_GET_INFO( get_sc3_tile_info )
{
	UINT32 data = (sc3_vram[tile_index*2]<<16) | sc3_vram[tile_index*2+1];
	int tile = data >> 14;
	int color = (sc3_vram[tile_index*2+1] & 0x1f)+0x30;

	SET_TILE_INFO(
			0,
			tile,
			color,
			0);
}

VIDEO_START(mil4000)
{
	const address_space *space = cpu_get_address_space(machine->cpu[0], ADDRESS_SPACE_PROGRAM);
	int i;

	// game doesn't clear the palette, so to avoid seeing mame defaults we clear it
	for (i=0;i<0x800;i++)
		palette_set_color(space->machine, i, MAKE_RGB(0, 0, 0));

	sc0_tilemap = tilemap_create(get_sc0_tile_info,tilemap_scan_rows,8,8,64,64);
	sc1_tilemap = tilemap_create(get_sc1_tile_info,tilemap_scan_rows,8,8,64,64);
	sc2_tilemap = tilemap_create(get_sc2_tile_info,tilemap_scan_rows,8,8,64,64);
	sc3_tilemap = tilemap_create(get_sc3_tile_info,tilemap_scan_rows,8,8,64,64);

	tilemap_set_transparent_pen(sc1_tilemap,0);
	tilemap_set_transparent_pen(sc2_tilemap,0);
	tilemap_set_transparent_pen(sc3_tilemap,0);
}

VIDEO_UPDATE(mil4000)
{
	tilemap_draw(bitmap,cliprect,sc0_tilemap,0,0);
	tilemap_draw(bitmap,cliprect,sc1_tilemap,0,0);
	tilemap_draw(bitmap,cliprect,sc2_tilemap,0,0);
	tilemap_draw(bitmap,cliprect,sc3_tilemap,0,0);
	return 0;
}

/*TODO*/
static READ16_HANDLER( hvretrace_r )
{
	static UINT16 res;
	static UINT16 vblank = 0,hblank = 0;

	res = 0;

	vblank^=1;
	hblank^=1;

	/*V-Blank*/
	if (vblank)
		res|= 0x80;

	/*H-Blank*/
	if (hblank)
		res|= 0x40;

	return res;
}


static WRITE16_HANDLER( sc0_vram_w )
{
	sc0_vram[offset] = data;
	tilemap_mark_tile_dirty(sc0_tilemap,offset/2);
}

static WRITE16_HANDLER( sc1_vram_w )
{
	sc1_vram[offset] = data;
	tilemap_mark_tile_dirty(sc1_tilemap,offset/2);
}

static WRITE16_HANDLER( sc2_vram_w )
{
	sc2_vram[offset] = data;
	tilemap_mark_tile_dirty(sc2_tilemap,offset/2);
}

static WRITE16_HANDLER( sc3_vram_w )
{
	sc3_vram[offset] = data;
	tilemap_mark_tile_dirty(sc3_tilemap,offset/2);
}

/*end of video stuff*/

/*
	--x- ---- ---- ---- Coin Counter
	---- ---- -x-- ---- Prize
	---- ---- --x- ---- Start
	---- ---- ---x ---- Hold 5
	---- ---- ---- x--- Hold 4
	---- ---- ---- -x-- Hold 3
	---- ---- ---- --x- Hold 2
	---- ---- ---- ---x Hold 1
*/
static WRITE16_HANDLER( output_w )
{
	static int i;

	for(i=0;i<3;i++)
		coin_counter_w(i,data & 0x2000);

//	popmessage("%04x\n",data);
}

static ADDRESS_MAP_START( mil4000_map, ADDRESS_SPACE_PROGRAM, 16 )
	AM_RANGE(0x000000, 0x03ffff) AM_ROM
	AM_RANGE(0x500000, 0x503fff) AM_RAM_WRITE(sc0_vram_w) AM_BASE(&sc0_vram)
	AM_RANGE(0x504000, 0x507fff) AM_RAM_WRITE(sc1_vram_w) AM_BASE(&sc1_vram)
	AM_RANGE(0x508000, 0x50bfff) AM_RAM_WRITE(sc2_vram_w) AM_BASE(&sc2_vram)
	AM_RANGE(0x50c000, 0x50ffff) AM_RAM_WRITE(sc3_vram_w) AM_BASE(&sc3_vram)

	AM_RANGE(0x708000, 0x708001) AM_READ_PORT("IN0")
	AM_RANGE(0x708002, 0x708003) AM_READ_PORT("IN1")
	AM_RANGE(0x708004, 0x708005) AM_READ(hvretrace_r)
	AM_RANGE(0x708006, 0x708007) AM_READ_PORT("IN2")
	AM_RANGE(0x708008, 0x708009) AM_WRITE(output_w)
	AM_RANGE(0x708010, 0x708011) AM_NOP //touch screen
	AM_RANGE(0x70801e, 0x70801f) AM_READWRITE(okim6295_status_0_lsb_r, okim6295_data_0_lsb_w)

	AM_RANGE(0x780000, 0x780fff) AM_RAM_WRITE(paletteram16_RRRRRGGGGGBBBBBx_word_w) AM_BASE(&paletteram16)
	AM_RANGE(0xf00000, 0xffffff) AM_RAM AM_BASE(&generic_nvram16) AM_SIZE(&generic_nvram_size) //probably not all of it.

ADDRESS_MAP_END

static INPUT_PORTS_START( mil4000 )
	PORT_START("IN0")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_NAME("Hold 1") PORT_CODE(KEYCODE_Z)
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_NAME("Hold 2") PORT_CODE(KEYCODE_X)
	PORT_DIPNAME( 0x0004, 0x0004, "IN0" )
	PORT_DIPSETTING(      0x0004, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0008, 0x0008, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0008, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_BUTTON5 ) PORT_NAME("Hold 5") PORT_CODE(KEYCODE_B)
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_START1 )
	PORT_DIPNAME( 0x0040, 0x0040, "Key out" ) //key out
	PORT_DIPSETTING(      0x0040, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_NAME("Hold 3") PORT_CODE(KEYCODE_C)
	PORT_DIPNAME( 0x0100, 0x0100, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0100, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0200, 0x0200, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0200, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0400, 0x0400, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0400, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0800, 0x0800, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0800, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x1000, 0x1000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x1000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x2000, 0x2000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x2000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x4000, 0x4000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x4000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x8000, 0x8000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x8000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_START("IN1")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_DIPNAME( 0x0004, 0x0004, "IN1" )
	PORT_DIPSETTING(      0x0004, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0008, 0x0008, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0008, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0010, 0x0010, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0010, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0020, 0x0020, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0020, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_SERVICE_NO_TOGGLE( 0x0040, IP_ACTIVE_LOW )
	PORT_DIPNAME( 0x0080, 0x0080, "Prize" )//premio / prize (ticket?)
	PORT_DIPSETTING(      0x0080, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0100, 0x0100, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0100, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0200, 0x0200, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0200, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0400, 0x0400, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0400, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0800, 0x0800, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0800, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x1000, 0x1000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x1000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x2000, 0x2000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x2000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x4000, 0x4000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x4000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x8000, 0x8000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x8000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_START("IN2")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_COIN3 )//key in
	PORT_DIPNAME( 0x0002, 0x0002, "IN2" )
	PORT_DIPSETTING(      0x0002, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0004, 0x0004, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0004, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0008, 0x0008, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0008, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0010, 0x0010, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0010, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0020, 0x0020, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0020, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_NAME("Hold 4") PORT_CODE(KEYCODE_V)
	PORT_DIPNAME( 0x0080, 0x0080, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0080, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0100, 0x0100, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0100, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0200, 0x0200, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0200, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0400, 0x0400, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0400, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0800, 0x0800, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0800, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x1000, 0x1000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x1000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x2000, 0x2000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x2000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x4000, 0x4000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x4000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x8000, 0x8000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x8000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
INPUT_PORTS_END

static const gfx_layout tilelayout =
{
	8,8,    /* 8*8 characters */
	RGN_FRAC(1,5),
	5,
	{  RGN_FRAC(0,5), RGN_FRAC(1,5), RGN_FRAC(2,5),RGN_FRAC(3,5),RGN_FRAC(4,5) },
	{ 0, 1, 2, 3, 4, 5, 6, 7 },
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8 },
	8*8
};


static GFXDECODE_START( mil4000 )
	GFXDECODE_ENTRY( "gfx1", 0, tilelayout,     0, 0x800/32 )
GFXDECODE_END


static MACHINE_DRIVER_START( mil4000 )
	MDRV_CPU_ADD("main", M68000, 8000000 )	// ?
	MDRV_CPU_PROGRAM_MAP(mil4000_map,0)
	// irq 2/4/5 point to the same place, others invalid
	MDRV_CPU_VBLANK_INT("main", irq5_line_hold)

	MDRV_NVRAM_HANDLER(generic_0fill)

	MDRV_SCREEN_ADD("main", RASTER)
	MDRV_SCREEN_REFRESH_RATE(60)
	MDRV_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MDRV_SCREEN_FORMAT(BITMAP_FORMAT_RGB32)
	MDRV_SCREEN_SIZE(320, 240)
	MDRV_SCREEN_VISIBLE_AREA(0, 320-1, 0, 240-1)

	MDRV_PALETTE_LENGTH(0x800)
	MDRV_GFXDECODE(mil4000)
	MDRV_VIDEO_START(mil4000)
	MDRV_VIDEO_UPDATE(mil4000)

	MDRV_SPEAKER_STANDARD_MONO("mono")
	MDRV_SOUND_ADD("oki", OKIM6295, 2500000) //pin 7 high & frequency not verified
	MDRV_SOUND_CONFIG(okim6295_interface_pin7high)
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "mono", 1.0)
MACHINE_DRIVER_END




ROM_START( mil4000 )
	ROM_REGION( 0x100000, "main", 0 ) // 68000 code
	ROM_LOAD16_BYTE( "9.u75", 0x000001, 0x20000, CRC(e3e520df) SHA1(16ee86deb75bd711c846a647e3a0a4293b5685a8) )
	ROM_LOAD16_BYTE( "10.u76", 0x000000, 0x20000, CRC(9020e19a) SHA1(e9ba0b69e8cb1fc35d024ae702d4670d78bf5cc8) )

	ROM_REGION( 0xa0000, "gfx1", 0 ) // 5bpp?
	ROM_LOAD( "2.u36",     0x000000, 0x20000, CRC(bb4fcfde) SHA1(7e19722ce42b9ec86faac32a526429b0e56639b5) )
	ROM_LOAD( "3.u35_alt", 0x020000, 0x20000, CRC(3fd93c2f) SHA1(5217e328e51a2e00dc85a662dab6e339bd7f336f) ) // one of these is probably bad
	ROM_LOAD( "4.u34",     0x040000, 0x20000, CRC(372a67a4) SHA1(c1c1352dd3152603827224d8970e6cb04aa1e858) )
	ROM_LOAD( "5.u33",     0x060000, 0x20000, CRC(8058882e) SHA1(2de7b1e6e39d89913b2d6c1290d3cf326d2527d4) )
	ROM_LOAD( "6.u32",     0x080000, 0x20000, CRC(7217a8c2) SHA1(275c2d5a128960dd6cd56d5e3647354b17129a12) )

	ROM_REGION( 0x40000, "oki", 0 ) // 6295 samples
	ROM_LOAD( "1.u54",   0x000000, 0x40000, CRC(e4a89163) SHA1(c0622c4e97b23daf9775137a2754bf9c47a29385) )

	ROM_REGION( 0x4d4c, "mcu", 0 ) // MCU code
	ROM_LOAD( "pic16c65a.u60.bad.dump", 0x000, 0x4d4c, BAD_DUMP CRC(c5e260ec) SHA1(d6e41de8a7db27382757ed7edfd7985090896e39) )

// palce22v10h.u74.bad.dump= palce22v10h-ch-jin-u27.u27  Jingle Bell (Italy, V133I)
ROM_END

ROM_START( mil4000a )
	ROM_REGION( 0x100000, "main", 0 ) // 68000 code
	ROM_LOAD16_BYTE( "27.u75", 0x000001, 0x20000, CRC(2a090f82) SHA1(c70295de25a99ec78752f2bd63e6ef0714141c84) )
	ROM_LOAD16_BYTE( "28.u76", 0x000000, 0x20000, CRC(009e1f16) SHA1(33014ccd33abf2de8e83ec964192ebb9cbda8a08) )

	ROM_REGION( 0xa0000, "gfx1", 0 ) // 5bpp?
	ROM_LOAD( "2.u36",   0x000000, 0x20000, CRC(bb4fcfde) SHA1(7e19722ce42b9ec86faac32a526429b0e56639b5) )
	ROM_LOAD( "3.u35",   0x020000, 0x20000, CRC(21c43d81) SHA1(a266b85378723ad8e219dd63a639add64624de13) )
	ROM_LOAD( "4.u34",   0x040000, 0x20000, CRC(372a67a4) SHA1(c1c1352dd3152603827224d8970e6cb04aa1e858) )
	ROM_LOAD( "5.u33",   0x060000, 0x20000, CRC(8058882e) SHA1(2de7b1e6e39d89913b2d6c1290d3cf326d2527d4) )
	ROM_LOAD( "6.u32",   0x080000, 0x20000, CRC(7217a8c2) SHA1(275c2d5a128960dd6cd56d5e3647354b17129a12) )

	ROM_REGION( 0x40000, "oki", 0 ) // 6295 samples
	ROM_LOAD( "1.u54",   0x000000, 0x40000, CRC(e4a89163) SHA1(c0622c4e97b23daf9775137a2754bf9c47a29385) )

	ROM_REGION( 0x4d4c, "mcu", 0 ) // MCU code
	ROM_LOAD( "pic16c65b_millennium4000.u60", 0x000, 0x4d4c, BAD_DUMP CRC(4f3f7b90) SHA1(fdf689dda57960820315dcf0138d2ade28248681) )

// palce22v10h.u74.bad.dump= palce22v10h-ch-jin-u27.u27  Jingle Bell (Italy, V133I)
ROM_END


GAME( 2000, mil4000,    0,        mil4000,    mil4000,    0, ROT0,  "Sure Milano", "Millennium Nuovo 4000 (Version 2.0)", 0 )
GAME( 2000, mil4000a,   mil4000,  mil4000,    mil4000,    0, ROT0,  "Sure Milano", "Millennium Nuovo 4000 (Version 1.8)", 0 )
