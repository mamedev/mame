/*
 'Swinging Singles' by Ent. Ent. Ltd
 driver by Tomasz Slanina


 Crap XXX game.
 Three roms contains text "BY YACHIYO"

 Upper half of 7.bin = upper half of 8.bin = intentional or bad dump ?

 TODO:
 - colors (missing prom(s) ?)
 - samples (at least two of unused roms contains samples (unkn. format , adpcm ?)
 - dips (one is tested in game (difficulty related?), another 2 are tested at start)

 Unknown reads/writes:
 - AY i/o ports (writes)
 - mem $c000, $c001 = protection device ? if tests fails, game crashes (problems with stack - skipped code with "pop af")
 - i/o port $8 = data read used for  $e command arg for one of AY chips (volume? - could be a sample player (based on volume changes?)
 - i/o port $1a = 1 or 0, rarely accessed, related to crt  writes

*/

#include "driver.h"
#include "cpu/z80/z80.h"
#include "sound/ay8910.h"
#include "video/mc6845.h"

static UINT8 *ssingles_videoram;
static UINT8 *ssingles_colorram;
static UINT8 prot_data;

#define NUM_PENS (4*8)
#define VMEM_SIZE 0x100
static pen_t pens[NUM_PENS];

//fake palette
static const UINT8 ssingles_colors[NUM_PENS*3]=
{
	0x00,0x00,0x00,	0xff,0xff,0xff, 0xff,0x00,0x00,	0x80,0x00,0x00,
	0x00,0x00,0x00,	0xf0,0xf0,0xf0,	0xff,0xff,0x00, 0x40,0x40,0x40,
	0x00,0x00,0x00,	0xff,0xff,0xff,	0xff,0x00,0x00,	0xff,0xff,0x00,
	0x00,0x00,0x00,	0xff,0xff,0x00,	0xd0,0x00,0x00,	0x80,0x00,0x00,
	0x00,0x00,0x00,	0xff,0x00,0x00,	0xff,0xff,0x00,	0x80,0x80,0x00,
	0x00,0x00,0x00,	0xff,0x00,0x00,	0x40,0x40,0x40,	0xd0,0xd0,0xd0,
	0x00,0x00,0x00,	0x00,0x00,0xff,	0x60,0x40,0x30,	0xff,0xff,0x00,
	0x00,0x00,0x00,	0xff,0x00,0xff,	0x80,0x00,0x80,	0x40,0x00,0x40
};

static MC6845_UPDATE_ROW( update_row )
{
	int cx,x;
	UINT32 tile_address;
	UINT16 cell,palette;
	UINT8 b0,b1;
	const UINT8 *gfx = memory_region(device->machine, "gfx1");

	for(cx=0;cx<x_count;++cx)
	{
		int address=((ma>>1)+(cx>>1))&0xff;

		cell=ssingles_videoram[address]+(ssingles_colorram[address]<<8);

		tile_address=((cell&0x3ff)<<4)+ra;
		palette=(cell>>10)&0x1c;

		if(cx&1)
		{
			b0=gfx[tile_address+0x0000]; /*  9.bin */
			b1=gfx[tile_address+0x8000]; /* 11.bin */
		}
		else
		{
			b0=gfx[tile_address+0x4000]; /* 10.bin */
			b1=gfx[tile_address+0xc000]; /* 12.bin */
		}

		for(x=7;x>=0;--x)
		{
			*BITMAP_ADDR32(bitmap, y, (cx<<3)|(x)) = pens[palette+((b1&1)|((b0&1)<<1))];
			b0>>=1;
			b1>>=1;
		}
	}
}

static const mc6845_interface mc6845_intf =
{
	"screen",
	8,
	NULL,						/* before pixel update callback */
	update_row,					/* row update callback */
	NULL,						/* after pixel update callback */
	DEVCB_NULL,					/* callback for display state changes */
	DEVCB_NULL,					/* callback for cursor state changes */
	DEVCB_NULL,					/* HSYNC callback */
	DEVCB_NULL,					/* VSYNC callback */
	NULL						/* update address callback */
};

static WRITE8_HANDLER(ssingles_videoram_w)
{
	ssingles_videoram[offset]=data;
}

static WRITE8_HANDLER(ssingles_colorram_w)
{
	ssingles_colorram[offset]=data;
}


static VIDEO_START(ssingles)
{
	{
		int i;
		for(i=0;i<NUM_PENS;++i)
		{
			pens[i]=MAKE_RGB(ssingles_colors[3*i], ssingles_colors[3*i+1], ssingles_colors[3*i+2]);
		}
	}
}


static VIDEO_UPDATE( ssingles )
{
	const device_config *mc6845 = devtag_get_device(screen->machine, "crtc");
	mc6845_update(mc6845, bitmap, cliprect);

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

static CUSTOM_INPUT(controls_r)
{
	int data = 7;
	switch(input_port_read(field->port->machine, "EXTRA"))		//multiplexed
	{
		case 0x01: data = 1; break;
		case 0x02: data = 2; break;
		case 0x04: data = 3; break;
		case 0x08: data = 4; break;
		case 0x10: data = 5; break;
		case 0x20: data = 6; break;
		case 0x40: data = 0; break;
	}

	return data;
}

static ADDRESS_MAP_START( ssingles_map, ADDRESS_SPACE_PROGRAM, 8 )
	AM_RANGE(0x0000, 0x00ff) AM_WRITE(ssingles_videoram_w)
	AM_RANGE(0x0800, 0x08ff) AM_WRITE(ssingles_colorram_w)
	AM_RANGE(0x0000, 0x1fff) AM_ROM
	AM_RANGE(0xc000, 0xc000) AM_READ( c000_r )
	AM_RANGE(0xc001, 0xc001) AM_READWRITE( c001_r, c001_w )
	AM_RANGE(0x6000, 0xbfff) AM_ROM
	AM_RANGE(0xf800, 0xffff) AM_RAM
ADDRESS_MAP_END

static ADDRESS_MAP_START( ssingles_io_map, ADDRESS_SPACE_IO, 8 )
	ADDRESS_MAP_GLOBAL_MASK(0xff)
	AM_RANGE(0x00, 0x00) AM_DEVWRITE("ay1", ay8910_address_w)
	AM_RANGE(0x04, 0x04) AM_DEVWRITE("ay1", ay8910_data_w)
	AM_RANGE(0x06, 0x06) AM_DEVWRITE("ay2", ay8910_address_w)
	AM_RANGE(0x08, 0x08) AM_READNOP
	AM_RANGE(0x0a, 0x0a) AM_DEVWRITE("ay2", ay8910_data_w)
	AM_RANGE(0x16, 0x16) AM_READ_PORT("DSW0")
	AM_RANGE(0x18, 0x18) AM_READ_PORT("DSW1")
	AM_RANGE(0x1c, 0x1c) AM_READ_PORT("INPUTS")
	AM_RANGE(0x1a, 0x1a) AM_WRITENOP //video/crt related
	AM_RANGE(0xfe, 0xfe) AM_DEVWRITE("crtc", mc6845_address_w)
	AM_RANGE(0xff, 0xff) AM_DEVWRITE("crtc", mc6845_register_w)

ADDRESS_MAP_END

static INPUT_PORTS_START( ssingles )
	PORT_START("INPUTS")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_UNKNOWN ) //must be LOW
	PORT_BIT( 0x1c, IP_ACTIVE_HIGH, IPT_SPECIAL ) PORT_CUSTOM(controls_r, NULL)
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_BUTTON4 )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_BUTTON2 )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_BUTTON3 )

	PORT_START("EXTRA")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP )
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_START1 )
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_START2 )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_BUTTON1 )

	PORT_START("DSW0")
	PORT_DIPNAME( 0x03, 0x00, DEF_STR( Coin_A ) )
	PORT_DIPSETTING(	0x00, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(	0x01, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(	0x02, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(	0x03, DEF_STR( Free_Play ) )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Difficulty ) )
	PORT_DIPSETTING(	0x20, DEF_STR( Easy ) )
	PORT_DIPSETTING(	0x00, DEF_STR( Hard ) )
	PORT_DIPNAME( 0x04, 0x04, "Unk1" )
	PORT_DIPSETTING(	0x00, DEF_STR( No ) )
	PORT_DIPSETTING(	0x04, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x08, 0x08, "Unk2" )
	PORT_DIPSETTING(	0x00, DEF_STR( No ) )
	PORT_DIPSETTING(	0x08, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x10, 0x10, "Unk3" )
	PORT_DIPSETTING(	0x00, DEF_STR( No ) )
	PORT_DIPSETTING(	0x10, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x40, 0x40, "Unk4" ) //tested in game, every frame, could be difficulty related
	PORT_DIPSETTING(	0x00, DEF_STR( No ) )
	PORT_DIPSETTING(	0x40, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x80, 0x80, "Unk5" )
	PORT_DIPSETTING(	0x00, DEF_STR( No ) )
	PORT_DIPSETTING(	0x80, DEF_STR( Yes ) )

	PORT_START("DSW1")
	PORT_DIPNAME( 0x03, 0x03, "Unk 6" )
	PORT_DIPSETTING(	0x01, "Pos 1" )
	PORT_DIPSETTING(	0x03, "Pos 2" )
	PORT_DIPSETTING(	0x00, "Pos 3" )
	PORT_DIPSETTING(	0x02, "Pos 4" )
	PORT_DIPNAME( 0x04, 0x04, "Unk7" )
	PORT_DIPSETTING(	0x00, DEF_STR( No ) )
	PORT_DIPSETTING(	0x04, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x08, 0x08, "Unk8" )
	PORT_DIPSETTING(	0x00, DEF_STR( No ) )
	PORT_DIPSETTING(	0x08, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x10, 0x10, "Unk9" )
	PORT_DIPSETTING(	0x00, DEF_STR( No ) )
	PORT_DIPSETTING(	0x10, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x20, 0x20, "UnkA" )
	PORT_DIPSETTING(	0x00, DEF_STR( No ) )
	PORT_DIPSETTING(	0x20, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x40, 0x40, "UnkB" )
	PORT_DIPSETTING(	0x00, DEF_STR( No ) )
	PORT_DIPSETTING(	0x40, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x80, 0x80, "UnkC" )
	PORT_DIPSETTING(	0x00, DEF_STR( No ) )
	PORT_DIPSETTING(	0x80, DEF_STR( Yes ) )
INPUT_PORTS_END

static MACHINE_DRIVER_START( ssingles )
	MDRV_CPU_ADD("maincpu", Z80,4000000)		 /* ? MHz */
	MDRV_CPU_PROGRAM_MAP(ssingles_map)
	MDRV_CPU_IO_MAP(ssingles_io_map)
	MDRV_CPU_VBLANK_INT("screen", nmi_line_pulse)

	MDRV_SCREEN_ADD("screen", RASTER)
	MDRV_SCREEN_FORMAT(BITMAP_FORMAT_RGB32)
	MDRV_SCREEN_RAW_PARAMS(4000000, 256, 0, 256, 256, 0, 256)	/* temporary, CRTC will configure screen */

	MDRV_PALETTE_LENGTH(4) //guess

	MDRV_VIDEO_START(ssingles)
	MDRV_VIDEO_UPDATE(ssingles)

	MDRV_MC6845_ADD("crtc", MC6845, 1000000 /* ? MHz */, mc6845_intf)

	/* sound hardware */
	MDRV_SPEAKER_STANDARD_MONO("mono")

	MDRV_SOUND_ADD("ay1", AY8910, 1500000) /* ? MHz */
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.5)

	MDRV_SOUND_ADD("ay2", AY8910, 1500000) /* ? MHz */
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.5)

MACHINE_DRIVER_END

ROM_START( ssingles )
	ROM_REGION( 0x10000, "maincpu", 0 ) /* Z80 main CPU  */
	ROM_LOAD( "1.bin", 0x00000, 0x2000, CRC(43f02215) SHA1(9f04a7d4671ff39fd2bd8ec7afced4981ee7be05) )
	ROM_LOAD( "2.bin", 0x06000, 0x2000, CRC(281f27e4) SHA1(cef28717ab2ed991a5709464c01490f0ab1dc17c) )
	ROM_LOAD( "3.bin", 0x08000, 0x2000, CRC(14fdcb65) SHA1(70f7fcb46e74937de0e4037c9fe79349a30d0d07) )
	ROM_LOAD( "4.bin", 0x0a000, 0x2000, CRC(acb44685) SHA1(d68aab8b7e68d842a350d3fb76985ac857b1d972) )

	ROM_REGION( 0x10000, "gfx1", 0 )
	ROM_LOAD( "9.bin",  0x0000, 0x4000, CRC(57fac6f9) SHA1(12f6695c9831399e599a95008ebf9db943725437) )
	ROM_LOAD( "10.bin", 0x4000, 0x4000, CRC(cd3ba260) SHA1(2499ad9982cc6356e2eb3a0f10d77886872a0c9f) )
	ROM_LOAD( "11.bin", 0x8000, 0x4000, CRC(f7107b29) SHA1(a405926fd3cb4b3d2a1c705dcde25d961dba5884) )
	ROM_LOAD( "12.bin", 0xc000, 0x4000, CRC(e5585a93) SHA1(04d55699b56d869066f2be2c6ac48042aa6c3108) )

	ROM_REGION( 0x08000, "user1", 0) /* samples ? data ?*/
	ROM_LOAD( "5.bin", 0x00000, 0x2000, CRC(242a8dda) SHA1(e140893cc05fb8cee75904d98b02626f2565ed1b) )
	ROM_LOAD( "6.bin", 0x02000, 0x2000, CRC(85ab8aab) SHA1(566f034e1ba23382442f27457447133a0e0f1cfc) )
	ROM_LOAD( "7.bin", 0x04000, 0x2000, CRC(57cc112d) SHA1(fc861c58ae39503497f04d302a9f16fca19b37fb) )
	ROM_LOAD( "8.bin", 0x06000, 0x2000, CRC(52de717a) SHA1(e60399355165fb46fac862fb7fcdff16ff351631) )

ROM_END

static DRIVER_INIT(ssingles)
{
	ssingles_videoram=auto_alloc_array_clear(machine, UINT8, VMEM_SIZE);
	ssingles_colorram=auto_alloc_array_clear(machine, UINT8, VMEM_SIZE);
	state_save_register_global_pointer(machine, ssingles_videoram, VMEM_SIZE);
	state_save_register_global_pointer(machine, ssingles_colorram, VMEM_SIZE);
}

GAME ( 1983, ssingles, 0, ssingles, ssingles, ssingles, ROT90, "Ent. Ent. Ltd", "Swinging Singles", GAME_SUPPORTS_SAVE | GAME_WRONG_COLORS | GAME_IMPERFECT_SOUND )
