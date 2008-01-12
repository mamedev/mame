/*
 'Swinging Singles' by Ent. Ent. Ltd
 driver by Tomasz Slanina

 Crap XXX game.
 Three roms contains text "BY YACHIYO"

 Upper half of 7.bin = upper half of 8.bin = intentional or bad dump ?

 TODO:
 - colors (proms ? one of unk writes?)
 - samples (at least two of unused roms contains samples (unkn. format , adpcm ?)
 - dips (one is tested in game (difficulty related?), another 2 are tested at start)

 Unknown reads/writes:
 - AY i/o ports (writes)
 - mem $c000, $c001 = protection device ? if tests fails, game crashes (problems with stack - skipped code with "pop af")
 - i/o port $8 = data read used for  $e command arg for one of AY chips (volume? - could be a sample player (based on volume changes?)
 - i/o port $1a = 1 or 0, rarely accessed, related to crt  writes
*/

#include "driver.h"
#include "sound/ay8910.h"
#include "video/crtc6845.h"

static tilemap *ssingles_tilemap;
static UINT8 *ssingles_vram;
static UINT8 prot_data;

static const crtc6845_interface crtc6845_intf =
{
		0,
		1000000, /* the clock of the chip  - guess */
		8,
		NULL,             
		NULL,            
		NULL,             
		NULL              
};

static TILE_GET_INFO( get_tile_info )
{
	int code = ssingles_vram[tile_index]+256*(ssingles_vram[tile_index+0x800]);
	SET_TILE_INFO(
		0,
		code,
		0,
		0);
}

static WRITE8_HANDLER(ssingles_vram_w)
{
	ssingles_vram[offset]=data;
}

static VIDEO_START(ssingles)
{
	ssingles_tilemap = tilemap_create( get_tile_info,tilemap_scan_rows,TILEMAP_TYPE_PEN,16,16,18,113 ); //smaller than 113
	crtc6845_config(0, &crtc6845_intf);
}

static VIDEO_UPDATE(ssingles)
{
	tilemap_mark_all_tiles_dirty(ssingles_tilemap);
	tilemap_draw(bitmap,cliprect,ssingles_tilemap,TILEMAP_DRAW_OPAQUE,0);
	return 0;
}

static READ8_HANDLER(c000_r)
{
	return prot_data;
}

static READ8_HANDLER(c001_r)
{
	prot_data=0xc4;
	return 0;
}

static WRITE8_HANDLER(c001_w)
{
	prot_data^=data^0x11;
}

static READ8_HANDLER(controls_r)
{
	int data=7;
	switch(input_port_1_r(0)) //multiplexed
	{
		case 0x01: data=1; break;
		case 0x02: data=2; break;
		case 0x04: data=3; break;
		case 0x08: data=4; break;
		case 0x10: data=5; break;
		case 0x20: data=6; break;
		case 0x40: data=0; break;
	}
	return (input_port_0_r(0)&(~0x1c))|(data<<2);
}

static ADDRESS_MAP_START( ssingles_map, ADDRESS_SPACE_PROGRAM, 8 )
	AM_RANGE(0x0000, 0x1fff) AM_READ( MRA8_ROM ) AM_WRITE(ssingles_vram_w)
	AM_RANGE(0xc000, 0xc000) AM_READ( c000_r )
	AM_RANGE(0xc001, 0xc001) AM_READWRITE( c001_r, c001_w )
	AM_RANGE(0x6000, 0xbfff) AM_ROM
	AM_RANGE(0xf800, 0xffff) AM_RAM
ADDRESS_MAP_END

static ADDRESS_MAP_START( ssingles_io_map, ADDRESS_SPACE_IO, 8 )
	ADDRESS_MAP_FLAGS( AMEF_ABITS(8) )
	AM_RANGE(0x00, 0x00) AM_WRITE(AY8910_control_port_0_w)
	AM_RANGE(0x04, 0x04) AM_WRITE(AY8910_write_port_0_w)
	AM_RANGE(0x06, 0x06) AM_WRITE(AY8910_control_port_1_w)
	AM_RANGE(0x08, 0x08) AM_READNOP
	AM_RANGE(0x0a, 0x0a) AM_WRITE(AY8910_write_port_1_w)
	AM_RANGE(0x16, 0x16) AM_READ(input_port_2_r)
	AM_RANGE(0x18, 0x18) AM_READ(input_port_3_r)
	AM_RANGE(0x1c, 0x1c) AM_READ(controls_r)
	AM_RANGE(0x1a, 0x1a) AM_WRITENOP //video related
	AM_RANGE(0xfe, 0xfe) AM_WRITE(crtc6845_address_w)
	AM_RANGE(0xff, 0xff) AM_WRITE(crtc6845_register_w)

ADDRESS_MAP_END

static INPUT_PORTS_START( ssingles )
PORT_START_TAG("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_UNKNOWN ) //must be LOW

	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_BUTTON4 )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_BUTTON2 )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_BUTTON3 )

PORT_START_TAG("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP )
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_START1 )
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_START2 )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_BUTTON1 )

PORT_START_TAG("DSW0")
PORT_DIPNAME( 0x03, 0x00, DEF_STR( Coin_A ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x01, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x02, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0x03, DEF_STR( Free_Play ) )
PORT_DIPNAME( 0x20, 0x20, DEF_STR( Difficulty ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Easy ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Hard ) )

PORT_DIPNAME( 0x04, 0x04, "Unk1" )
	PORT_DIPSETTING(    0x00, DEF_STR( No ) )
	PORT_DIPSETTING(    0x04, DEF_STR( Yes ) )
PORT_DIPNAME( 0x08, 0x08, "Unk2" )
	PORT_DIPSETTING(    0x00, DEF_STR( No ) )
	PORT_DIPSETTING(    0x08, DEF_STR( Yes ) )
PORT_DIPNAME( 0x10, 0x10, "Unk3" )
	PORT_DIPSETTING(    0x00, DEF_STR( No ) )
	PORT_DIPSETTING(    0x10, DEF_STR( Yes ) )

PORT_DIPNAME( 0x40, 0x40, "Unk4" ) //tested in game, every frame, could be difficulty related
	PORT_DIPSETTING(    0x00, DEF_STR( No ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Yes ) )
PORT_DIPNAME( 0x80, 0x80, "Unk5" )
	PORT_DIPSETTING(    0x00, DEF_STR( No ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Yes ) )

PORT_START_TAG("DSW1")
PORT_DIPNAME( 0x03, 0x03, "Unk 6" )
	PORT_DIPSETTING(    0x01, "Pos 1" )
	PORT_DIPSETTING(    0x03, "Pos 2" )
	PORT_DIPSETTING(    0x00, "Pos 3" )
	PORT_DIPSETTING(    0x02, "Pos 4" )
PORT_DIPNAME( 0x04, 0x04, "Unk7" )
	PORT_DIPSETTING(    0x00, DEF_STR( No ) )
	PORT_DIPSETTING(    0x04, DEF_STR( Yes ) )
PORT_DIPNAME( 0x08, 0x08, "Unk8" )
	PORT_DIPSETTING(    0x00, DEF_STR( No ) )
	PORT_DIPSETTING(    0x08, DEF_STR( Yes ) )
PORT_DIPNAME( 0x10, 0x10, "Unk9" )
	PORT_DIPSETTING(    0x00, DEF_STR( No ) )
	PORT_DIPSETTING(    0x10, DEF_STR( Yes ) )
PORT_DIPNAME( 0x20, 0x20, "UnkA" )
	PORT_DIPSETTING(    0x00, DEF_STR( No ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Yes ) )
PORT_DIPNAME( 0x40, 0x40, "UnkB" )
	PORT_DIPSETTING(    0x00, DEF_STR( No ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Yes ) )
PORT_DIPNAME( 0x80, 0x80, "UnkC" )
	PORT_DIPSETTING(    0x00, DEF_STR( No ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Yes ) )
INPUT_PORTS_END

static const gfx_layout charlayout =
{
	16,16,
	RGN_FRAC(1,4),
	2,
	{ 0, RGN_FRAC(1,2) },
	{ RGN_FRAC(1,4),RGN_FRAC(1,4)+ 1,RGN_FRAC(1,4)+ 2,RGN_FRAC(1,4)+ 3,RGN_FRAC(1,4)+ 4,RGN_FRAC(1,4)+ 5,RGN_FRAC(1,4) + 6,RGN_FRAC(1,4) + 7,0,1,2,3,4,5,6,7 },
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8,8*8,9*8,10*8,11*8,12*8,13*8,14*8,15*8},
	8*8*2
};

static GFXDECODE_START( ssingles )
	GFXDECODE_ENTRY( REGION_GFX1, 0, charlayout,   0x0000, 1 )
GFXDECODE_END


static MACHINE_DRIVER_START( ssingles )
	MDRV_CPU_ADD(Z80,4000000)		 /* ? MHz */
	MDRV_CPU_PROGRAM_MAP(ssingles_map,0)
	MDRV_CPU_IO_MAP(ssingles_io_map,0)
	MDRV_CPU_VBLANK_INT(nmi_line_pulse,1)
	
	MDRV_SCREEN_REFRESH_RATE(60)
	MDRV_SCREEN_VBLANK_TIME(DEFAULT_60HZ_VBLANK_DURATION)

	MDRV_VIDEO_ATTRIBUTES(VIDEO_TYPE_RASTER )
	MDRV_SCREEN_FORMAT(BITMAP_FORMAT_INDEXED16)
	MDRV_SCREEN_SIZE(512, 256)

	MDRV_SCREEN_VISIBLE_AREA(0, 511, 0, 255)

	MDRV_GFXDECODE(ssingles)
	MDRV_PALETTE_LENGTH(16*4) //guess

	MDRV_VIDEO_START(ssingles)
	MDRV_VIDEO_UPDATE(ssingles)

	/* sound hardware */
	MDRV_SPEAKER_STANDARD_MONO("mono")

	MDRV_SOUND_ADD(AY8910, 1500000) /* ? MHz */
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.5)

	MDRV_SOUND_ADD(AY8910, 1500000) /* ? MHz */
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.5)

MACHINE_DRIVER_END

ROM_START( ssingles )
	ROM_REGION( 0x10000, REGION_CPU1, 0 ) /* Z80 main CPU  */
	ROM_LOAD( "1.bin", 0x00000, 0x2000, CRC(43f02215) SHA1(9f04a7d4671ff39fd2bd8ec7afced4981ee7be05) )
	ROM_LOAD( "2.bin", 0x06000, 0x2000, CRC(281f27e4) SHA1(cef28717ab2ed991a5709464c01490f0ab1dc17c) )
	ROM_LOAD( "3.bin", 0x08000, 0x2000, CRC(14fdcb65) SHA1(70f7fcb46e74937de0e4037c9fe79349a30d0d07) )
	ROM_LOAD( "4.bin", 0x0a000, 0x2000, CRC(acb44685) SHA1(d68aab8b7e68d842a350d3fb76985ac857b1d972) )

	ROM_REGION( 0x10000, REGION_GFX1, 0 )
	ROM_LOAD( "9.bin",  0x0000, 0x4000, CRC(57fac6f9) SHA1(12f6695c9831399e599a95008ebf9db943725437) )
	ROM_LOAD( "10.bin", 0x4000, 0x4000, CRC(cd3ba260) SHA1(2499ad9982cc6356e2eb3a0f10d77886872a0c9f) )
	ROM_LOAD( "11.bin", 0x8000, 0x4000, CRC(f7107b29) SHA1(a405926fd3cb4b3d2a1c705dcde25d961dba5884) )
	ROM_LOAD( "12.bin", 0xc000, 0x4000, CRC(e5585a93) SHA1(04d55699b56d869066f2be2c6ac48042aa6c3108) )

	ROM_REGION( 0x010000, REGION_USER1, 0) /* samples ? */
	ROM_LOAD( "5.bin", 0x00000, 0x2000, CRC(242a8dda) SHA1(e140893cc05fb8cee75904d98b02626f2565ed1b) )
	ROM_LOAD( "6.bin", 0x02000, 0x2000, CRC(85ab8aab) SHA1(566f034e1ba23382442f27457447133a0e0f1cfc) )
	ROM_LOAD( "7.bin", 0x04000, 0x2000, CRC(57cc112d) SHA1(fc861c58ae39503497f04d302a9f16fca19b37fb) )
	ROM_LOAD( "8.bin", 0x06000, 0x2000, CRC(52de717a) SHA1(e60399355165fb46fac862fb7fcdff16ff351631) )

ROM_END

static DRIVER_INIT(ssingles)
{
	ssingles_vram=auto_malloc(0x2000);
	memset(ssingles_vram,0,0x2000);
	state_save_register_global_pointer(ssingles_vram, 0x2000);
}

GAME ( 1983, ssingles, 0, ssingles, ssingles, ssingles, ROT90, "Ent. Ent. Ltd", "Swinging Singles", GAME_SUPPORTS_SAVE | GAME_WRONG_COLORS | GAME_IMPERFECT_SOUND )
