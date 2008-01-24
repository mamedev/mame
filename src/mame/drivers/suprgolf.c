/***************************************************************************

 Super Golf
 preliminary WIP driver
 by Tomasz Slanina

 maybe vidram banking
 plenty of things unknown still

 sometimes extra bits are written to bank registers..

 its a z80 game.. i wonder if there is a palette or it should have proms
               -- the rest of the hardware makes me fear so

***************************************************************************/

#include "driver.h"
#include "sound/2203intf.h"

static tilemap *suprgolf_tilemap;

static int suprgolf_rom_bank;

static TILE_GET_INFO( get_tile_info )
{
	int code = videoram[tile_index*2]+256*(videoram[tile_index*2+1]);
	SET_TILE_INFO(
		0,
		code,
		0,
		0);
}

/*
static READ8_HANDLER( rom_bank_select_r )
{
    return suprgolf_rom_bank;
}
*/
static WRITE8_HANDLER( rom_bank_select_w )
{
	UINT8 *region_base = memory_region(REGION_USER1);

	suprgolf_rom_bank = data;

	mame_printf_debug("ROM_BANK 0x8000 - %X @%X\n",data,activecpu_get_previouspc());
	memory_set_bankptr(2, region_base + (data&0x3f ) * 0x4000);
}

static WRITE8_HANDLER( rom2_bank_select_w )
{
	UINT8 *region_base = memory_region(REGION_USER2);
	mame_printf_debug("ROM_BANK 0x4000 - %X @%X\n",data,activecpu_get_previouspc());
	memory_set_bankptr(1, region_base + (data&0x3f ) * 0x4000);
}

static MACHINE_RESET( suprgolf )
{

}

static VIDEO_START( suprgolf )
{
	suprgolf_tilemap = tilemap_create( get_tile_info,tilemap_scan_rows,TILEMAP_TYPE_PEN,8,8,32,32 );
}

static VIDEO_UPDATE( suprgolf )
{
	tilemap_mark_all_tiles_dirty(suprgolf_tilemap);
	tilemap_draw(bitmap,cliprect,suprgolf_tilemap,TILEMAP_DRAW_OPAQUE,0);
	return 0;
}


// vidram is banked?

static ADDRESS_MAP_START( main_map, ADDRESS_SPACE_PROGRAM, 8 )
	AM_RANGE(0x0000, 0x3fff) AM_ROM
	AM_RANGE(0x4000, 0x7fff) AM_READ(MRA8_BANK1)
	AM_RANGE(0x4000, 0x4000) AM_WRITE( rom2_bank_select_w )

	AM_RANGE(0x8000, 0xbfff) AM_READ(MRA8_BANK2)

	AM_RANGE(0xc000, 0xcfff) AM_RAM
	AM_RANGE(0xd000, 0xdfff) AM_RAM
	AM_RANGE(0xe000, 0xe7ff) AM_RAM AM_BASE(&videoram)
	AM_RANGE(0xe800, 0xefff) AM_RAM
//  AM_RANGE(0xf000, 0xffff) AM_RAM
	AM_RANGE(0xf800, 0xffff) AM_RAM
ADDRESS_MAP_END

static READ8_HANDLER( suprgolf_random )
{
	return mame_rand(Machine);
}

static ADDRESS_MAP_START( io_map, ADDRESS_SPACE_IO, 8 )
	ADDRESS_MAP_FLAGS( AMEF_ABITS(8) )
	AM_RANGE(0x00, 0x00) AM_READ(input_port_0_r) // Player 1 controls
	AM_RANGE(0x01, 0x01) AM_READ(input_port_1_r) // Player 2 controls
	AM_RANGE(0x02, 0x02) AM_READ(input_port_2_r) // ??
	AM_RANGE(0x04, 0x04) AM_READ(input_port_3_r) // extra controls + coin + test
	AM_RANGE(0x05, 0x05) AM_READ(input_port_4_r) AM_WRITE(rom_bank_select_w)
	AM_RANGE(0x06, 0x06) AM_READ(suprgolf_random) // game locks up or crashes? if this doesn't return right values?

	AM_RANGE(0x08, 0x08) AM_READ(YM2203_status_port_0_r) AM_WRITE(YM2203_control_port_0_w)
	AM_RANGE(0x09, 0x09) AM_READ(YM2203_read_port_0_r) AM_WRITE(YM2203_write_port_0_w)
 ADDRESS_MAP_END

static INPUT_PORTS_START( suprgolf )
	PORT_START	/* PLAY1 */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(1)	    /* D.L */
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_PLAYER(1)	    /* D.R */
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1)			/* CNT */
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(1)			/* SEL */

	PORT_START	/* PLAY2 */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(2)	    /* D.L */
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_PLAYER(2)	    /* D.R */
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)			/* CNT */
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2)			/* SEL */


	PORT_START	/* 8bit */
	PORT_DIPNAME( 0x01, 0x01, "2" )
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START	/* 8bit */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_START1 )               			/* 1P */
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(1)			/* POW */
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_START2 )  	                	/* 1P */
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(2)			/* POW */
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, "TST" )
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_COIN2 )

	PORT_START	/* 8bit */
	PORT_DIPNAME( 0x01, 0x01, "4" )
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START	/* 8bit */
	PORT_DIPNAME( 0x01, 0x01, "DSW0" )
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START	/* 8bit */
	PORT_SERVICE( 0x01, IP_ACTIVE_LOW )
	PORT_DIPNAME( 0x02, 0x02, "DSW1" )
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
INPUT_PORTS_END

static WRITE8_HANDLER( suprgolf_writeA )
{
	mame_printf_debug("ymwA\n");
}

static WRITE8_HANDLER( suprgolf_writeB )
{
	mame_printf_debug("ymwA\n");
}

static void irqhandler(int irq)
{
//  cpunum_set_input_line(Machine, 1,0,irq ? ASSERT_LINE : CLEAR_LINE);
}

static const struct YM2203interface ym2203_interface =
{
	input_port_5_r,
	input_port_6_r,
	suprgolf_writeA,
	suprgolf_writeB,
	irqhandler
};
static const gfx_layout gfxlayout =
{
   8,8,
   RGN_FRAC(1,1),
   4,
 { 0, 1, 2, 3 },
	{ 1*4, 0*4, 3*4, 2*4, 5*4, 4*4, 7*4, 6*4 },
	{ 0*32, 1*32, 2*32, 3*32, 4*32, 5*32, 6*32, 7*32 },
	8*8*4
};

static GFXDECODE_START( suprgolf )
	GFXDECODE_ENTRY( REGION_GFX1, 0, gfxlayout,   0, 32 )
GFXDECODE_END

static MACHINE_DRIVER_START( suprgolf )

	/* basic machine hardware */
	MDRV_CPU_ADD(Z80,4000000)
	MDRV_CPU_PROGRAM_MAP(main_map,0)
	MDRV_CPU_IO_MAP(io_map,0)
	MDRV_CPU_VBLANK_INT(irq0_line_hold,1)

	MDRV_VIDEO_START(suprgolf)
	MDRV_VIDEO_UPDATE(suprgolf)

	MDRV_SCREEN_REFRESH_RATE(60)
	MDRV_SCREEN_VBLANK_TIME(DEFAULT_REAL_60HZ_VBLANK_DURATION)

	MDRV_MACHINE_RESET(suprgolf)

	/* video hardware */
	MDRV_VIDEO_ATTRIBUTES(VIDEO_TYPE_RASTER)
	MDRV_SCREEN_FORMAT(BITMAP_FORMAT_INDEXED16)
	MDRV_SCREEN_SIZE(256, 256)
	MDRV_SCREEN_VISIBLE_AREA(0, 255, 0, 191)
	MDRV_GFXDECODE(suprgolf)
	MDRV_PALETTE_LENGTH(512)

	/* sound hardware */
	MDRV_SPEAKER_STANDARD_MONO("mono")

	MDRV_SOUND_ADD(YM2203, 3000000)
	MDRV_SOUND_CONFIG(ym2203_interface)
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "mono", 1.0)
MACHINE_DRIVER_END


/*
----------------------
CG24     6K        CONN BD
CG1      6J         "
CG2      6G         "
CG3      6F         "
CG4      6D         "
CG5      6C         "
CG6      6A         "
CG7      5J         "
CG8      5G         "
CG9      5F         "
CG10     5D         "
CG11     5A         "
CG12     6K         "
CG13     6J         "
CG14     5K         "
CG15     5J         "
CG16     5G         "
CG17     5F         "

CG18     3K        DAUGHTER BOARD
CG20     7K         "
CG21     7J         "
CG22     7G         "
CG23     7F         "
*/

ROM_START( suprgolf )
	ROM_REGION( 0x10000, REGION_CPU1, 0 )
	ROM_LOAD( "cg24.6k",0x000000, 0x08000, CRC(de548044) SHA1(f96b4cfcfca4dffabfaf205eb903cbc70972626b) )

	ROM_REGION( 0x100000, REGION_USER1, ROMREGION_ERASEFF )
	ROM_LOAD( "cg1.6j", 0x000000, 0x10000, CRC(ee545c71) SHA1(8ee459a85e52257d3f9a2aa7263b641aad87bafd) )
	ROM_LOAD( "cg2.6g", 0x010000, 0x10000, CRC(a2ed2159) SHA1(5e13b6c4eaba8146a4c6c2ff24197f3ffca29b92) )
	ROM_LOAD( "cg3.6f", 0x020000, 0x10000, CRC(4543334d) SHA1(7ee268ed6d02c78db8c222418313593df37cde4b) )
	ROM_LOAD( "cg4.6d", 0x030000, 0x10000, CRC(85ace664) SHA1(5267406c98e2d124a4985816f8e2e32e74e09614) )
	ROM_LOAD( "cg5.6c", 0x040000, 0x10000, CRC(609d5b37) SHA1(60640a9bd0883bf4dc999077d89ef793e827ac23) )
	ROM_LOAD( "cg6.6a", 0x050000, 0x10000, CRC(5e4a8ddb) SHA1(0c71c7eba9fe79187c4214eb639a481305070dcc) )
	ROM_LOAD( "cg7.5j", 0x060000, 0x10000, CRC(90ac6734) SHA1(2656397fca6dceabf8e35c093c0ba25e08d2ad1e) )
	ROM_LOAD( "cg8.5g", 0x070000, 0x10000, CRC(2e9edece) SHA1(a0961bb23f312ed137134746d2d3d438fe098085) )
	ROM_LOAD( "cg9.5f", 0x080000, 0x10000, CRC(139d71f1) SHA1(756ed068e1e2b76a9d1df95b432976e632edfa77) )
	ROM_LOAD( "cg10.5d",0x090000, 0x10000, CRC(c069e75e) SHA1(77f1b7571e677aef601b8b1c481b352ca6e485d6) )
	/* no 5c? */
	ROM_LOAD( "cg11.5a",0x0b0000, 0x10000, CRC(cfec1a0f) SHA1(c09ece059cb3c456b66c016c6fab3139d3f61c6a) )

	ROM_REGION( 0x100000, REGION_USER2, ROMREGION_ERASEFF )
	ROM_LOAD( "cg20.7k",0x000000, 0x10000, CRC(1e3fa2fd) SHA1(4771b90e40ebfbae4a98ff7ce6db50f635232597) )
	ROM_LOAD( "cg21.7j",0x010000, 0x10000, CRC(0323a2cd) SHA1(d7d4b35ad451acb2fa3d117bb0ae2f8fbd883f17) )
	ROM_LOAD( "cg22.7g",0x020000, 0x10000, CRC(83bcbefd) SHA1(77f29cfd1583d2506e95b8513cb9f87569c31821) )
	ROM_LOAD( "cg23.7f",0x030000, 0x10000, CRC(50191b4d) SHA1(8f74cba2a2b5fd2a03eaf13a6d6b39af8833a4ab) )

	ROM_REGION( 0x70000, REGION_GFX1, ROMREGION_DISPOSE )
	ROM_LOAD( "cg18.3k",0x60000, 0x10000, CRC(36edd88e) SHA1(374c95721198a88831d6f7e0b71d05e2f8465271) )
	ROM_LOAD( "cg17.5f",0x50000, 0x10000, CRC(d27f87b5) SHA1(5b2927e89615589540e3853593aeff517584b6a0))
	ROM_LOAD( "cg16.5g",0x40000, 0x10000, CRC(0498aa2e) SHA1(988965c3a584dac17ad8c7e504fa1f1e49775611) )
	ROM_LOAD( "cg15.5j",0x30000, 0x10000, CRC(0fb88270) SHA1(d85a7f1bc5b3c4b13bbd887cea4c055541cbb737) )
	ROM_LOAD( "cg14.5k",0x20000, 0x10000, CRC(ca12e01d) SHA1(9c627fb527c8966e16dc6bdb99ec0b9728b5c5f9) )
	ROM_LOAD( "cg13.6j",0x10000, 0x10000, CRC(02ff0187) SHA1(aeeb3b2d15c3c8ff4695ecf6cfc0c385295ecce6) )
	ROM_LOAD( "cg12.6k",0x00000, 0x10000, CRC(5707b3d5) SHA1(9102a40fefb6426f2cd9d92d66fdc77e078e3f4c) )
ROM_END

GAME( 19??, suprgolf, 0, suprgolf,  suprgolf,  0, ROT0, "Nasco/Face?", "Super Crown Golf", GAME_NOT_WORKING )

