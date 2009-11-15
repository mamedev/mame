/****************************************************************************************
Chanbara
Data East, 1985

PCB Layout
----------

DE-0207-0
|-----------------------------------------------------------|
|             CP12.17H                     CP00-2.17C       |
|                          2016                       6809  |
|             CP13.15H                     CP01.15C         |
|                          2016                             |
|1  RCDM-I4   CP14.13H                     CP02.14C         |
|8  RCDM-I1                                2016             |
|W  RCDM-I1                                CP03.11C   2016  |
|A  RCDM-I1                         2148   CP04.10C         |
|Y  RCDM-I1                                CP05.9C          |
|   RCDM-I1                2148            CP06.8C          |
|  RM-C2                   2148                   TC15G032AY|
|  PM-C1                            2148   CP07.6C          |
|       DSW1                               CP08.5C          |
|CP15.6K                   DECO                     DECO    |
|CP16.5K                   VSC30           CP09.3C  HMC20   |
|CP17.4K                                   CP10.2C          |
|MB3730  558  558  YM3014  YM2203          CP11.1C   12MHz  |
|-----------------------------------------------------------|
Notes:
      6809   - clock 1.500MHz [12/8]
      YM2203 - clock 1.500MHz [12/8]
      VSC30  - clock 3.000MHz [12/4, pin 7), custom DECO DIP40 IC
      HMC20  - DECO HMC20 custom DIP28 IC. Provides many clocks each divided by 2
               (i.e. 12MHz, 6MHz, 3MHz, 1.5MHz, 750kHz etc)
               HSync is generated on pins 12 and 13 with 12/256/3. Actual xtal measures
               11.9931MHz, which accounts for the measured HSync error (12/256/3 = 15.625kHz)
      VSync  - 57.4122Hz
      HSync  - 15.6161kHz

------------------------

 Driver by Tomasz Slanina & David Haywood

ToDo:
 there might still be some sprite banking issues


****************************************************************************************/

#include "driver.h"
#include "cpu/m6809/m6809.h"
#include "sound/2203intf.h"

static tilemap *bg_tilemap;
static tilemap *bg2_tilemap;

static UINT8 *videoram2, *colorram2;

static PALETTE_INIT( chanbara )
{
	int i, red, green, blue;

	for (i = 0;i < machine->config->total_colors;i++)
	{
		red = color_prom[i];
		green = color_prom[machine->config->total_colors+i];
		blue = color_prom[2*machine->config->total_colors+i];

		palette_set_color_rgb(machine,i,pal4bit(red<<1),pal4bit(green<<1),pal4bit(blue<<1));
	}
}

static WRITE8_HANDLER( chanbara_videoram_w )
{
	videoram[offset] = data;
	tilemap_mark_tile_dirty(bg_tilemap, offset);
}

static WRITE8_HANDLER( chanbara_colorram_w )
{
	colorram[offset] = data;
	tilemap_mark_tile_dirty(bg_tilemap, offset);
}

static WRITE8_HANDLER( chanbara_videoram2_w )
{
	videoram2[offset] = data;
	tilemap_mark_tile_dirty(bg2_tilemap, offset);
}

static WRITE8_HANDLER( chanbara_colorram2_w )
{
	colorram2[offset] = data;
	tilemap_mark_tile_dirty(bg2_tilemap, offset);
}


static TILE_GET_INFO( get_bg_tile_info )
{
	int code = videoram[tile_index] + ((colorram[tile_index] & 1) << 8);
	int color = (colorram[tile_index] >> 1) & 0x1f;

	SET_TILE_INFO(0, code, color, 0);
}

static TILE_GET_INFO( get_bg2_tile_info )
{
	int code = videoram2[tile_index];
	int color = (colorram2[tile_index] >> 1) & 0x1f;

	SET_TILE_INFO(2, code, color, 0);
}

static VIDEO_START(chanbara )
{
	bg_tilemap = tilemap_create(machine, get_bg_tile_info, tilemap_scan_rows,8, 8, 32, 32);
	bg2_tilemap = tilemap_create(machine, get_bg2_tile_info, tilemap_scan_rows,16, 16, 16, 32);
	tilemap_set_transparent_pen(bg_tilemap,0);
}

static void draw_sprites(running_machine *machine, bitmap_t *bitmap, const rectangle *cliprect)
{
	int offs;

    for (offs = 0; offs < 0x80; offs += 4)
	{
		if(spriteram[offs + 0x80]&0x80)
		{
			int attr = spriteram[offs + 0];
			int code = spriteram[offs + 1];
			int color = spriteram[offs + 0x80]&0x1f;
			int flipx = 0;
			int flipy = attr & 2;
			int sx = 240-spriteram[offs + 3];
			int sy = 232-spriteram[offs+2];

			sy+=16;

			if (spriteram[offs + 0x80]&0x10) code += 0x200;
			if (spriteram[offs + 0x80]&0x20) code += 0x400;
			if (spriteram[offs + 0x80]&0x40) code += 0x100;

			if(attr&0x10)
			{
				if(!flipy)
				{

					drawgfx_transpen(bitmap, cliprect, machine->gfx[1], code, color, flipx, flipy, sx, sy-16, 0);
					drawgfx_transpen(bitmap, cliprect, machine->gfx[1], code+1, color, flipx, flipy, sx, sy, 0);
				}
				else
				{
					drawgfx_transpen(bitmap, cliprect, machine->gfx[1], code, color, flipx, flipy, sx, sy, 0);
					drawgfx_transpen(bitmap, cliprect, machine->gfx[1], code+1, color, flipx, flipy, sx, sy-16, 0);
				}
			}
			else
			{
				drawgfx_transpen(bitmap, cliprect, machine->gfx[1], code, color, flipx, flipy, sx, sy, 0);
			}
		}
	}
}

static UINT8 scroll;
static UINT8 scrollhi;

static VIDEO_UPDATE( chanbara )
{
	tilemap_set_scrolly(bg2_tilemap,0,scroll | (scrollhi << 8));
	tilemap_draw(bitmap, cliprect, bg2_tilemap, 0, 0);
	draw_sprites(screen->machine, bitmap, cliprect);
	tilemap_draw(bitmap, cliprect, bg_tilemap, 0, 0);
	return 0;
}

static ADDRESS_MAP_START( memmap, ADDRESS_SPACE_PROGRAM, 8 )
	AM_RANGE(0x0000, 0x07ff) AM_RAM
	AM_RANGE(0x0800, 0x0bff) AM_READ(SMH_RAM) AM_WRITE(chanbara_videoram_w) AM_BASE(&videoram)
 	AM_RANGE(0x0c00, 0x0fff) AM_READ(SMH_RAM) AM_WRITE(chanbara_colorram_w) AM_BASE(&colorram)
 	AM_RANGE(0x1000, 0x10ff) AM_RAM AM_BASE(&spriteram)
 	AM_RANGE(0x1800, 0x19ff) AM_READ(SMH_RAM) AM_WRITE(chanbara_videoram2_w) AM_BASE(&videoram2)
 	AM_RANGE(0x1a00, 0x1bff) AM_READ(SMH_RAM) AM_WRITE(chanbara_colorram2_w) AM_BASE(&colorram2)
	AM_RANGE(0x2000, 0x2000) AM_READ_PORT("DSW0")
	AM_RANGE(0x2001, 0x2001) AM_READ_PORT("SYSTEM")
	AM_RANGE(0x2003, 0x2003) AM_READ_PORT("JOY")
	AM_RANGE(0x3800, 0x3801) AM_DEVREADWRITE("ymsnd", ym2203_r, ym2203_w)
	AM_RANGE(0x4000, 0x7fff) AM_READ(SMH_BANK(1))
	AM_RANGE(0x8000, 0xffff) AM_READ(SMH_ROM)
ADDRESS_MAP_END

/***************************************************************************/

static INPUT_PORTS_START( chanbara )
	PORT_START ("DSW0")
	PORT_DIPNAME( 0x01,   0x01, DEF_STR( Unused ) )
	PORT_DIPSETTING(      0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02,   0x02, DEF_STR( Unused ) )
	PORT_DIPSETTING(      0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04,   0x04, DEF_STR( Unused ) )
	PORT_DIPSETTING(      0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08,   0x08, DEF_STR( Unused ) )
	PORT_DIPSETTING(      0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10,   0x10, DEF_STR( Unused ) )
	PORT_DIPSETTING(      0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20,   0x20, DEF_STR( Unused ) )
	PORT_DIPSETTING(      0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40,   0x40, DEF_STR( Unused ) )
	PORT_DIPSETTING(      0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80,   0x80, DEF_STR( Unused ) )
	PORT_DIPSETTING(      0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x00, DEF_STR( On ) )

	PORT_START ("SYSTEM")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON2	 )
	PORT_DIPNAME( 0x04,   0x04, "2" )
	PORT_DIPSETTING(      0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08,   0x08, DEF_STR( Unused ) )
	PORT_DIPSETTING(      0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x00, DEF_STR( On ) )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_VBLANK )

	/* System Port */
	PORT_START ("JOY")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICKLEFT_DOWN )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICKLEFT_UP )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICKLEFT_LEFT )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICKLEFT_RIGHT )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_JOYSTICKRIGHT_DOWN )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_JOYSTICKRIGHT_UP )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_JOYSTICKRIGHT_LEFT )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_JOYSTICKRIGHT_RIGHT )
INPUT_PORTS_END

/***************************************************************************/

static const gfx_layout tilelayout =
{
	8,8,	/* tile size */
	RGN_FRAC(1,2),	/* number of tiles */
	2,	/* bits per pixel */
	{ 0, 4 }, /* plane offsets */
	{ RGN_FRAC(1,2)+0,  RGN_FRAC(1,2)+1, RGN_FRAC(1,2)+2, RGN_FRAC(1,2)+3, 0,1,2,3 }, /* x offsets */
	{ 0*8,1*8,2*8,3*8, 4*8, 5*8, 6*8, 7*8 }, /* y offsets */
	8*8	/* offset to next tile */
};

static const gfx_layout tile16layout =
{
	16,16,	/* tile size */
	RGN_FRAC(1,4),	/* number of tiles */
	3,	/* bits per pixel */
	{ RGN_FRAC(1,2),0,4 }, /* plane offsets */
	{ 16*8+RGN_FRAC(1,4)+0,16*8+ RGN_FRAC(1,4)+1,16*8+ RGN_FRAC(1,4)+2,16*8+ RGN_FRAC(1,4)+3,
       0,1,2,3,
		RGN_FRAC(1,4)+0,  RGN_FRAC(1,4)+1, RGN_FRAC(1,4)+2, RGN_FRAC(1,4)+3,
      16*8+0, 16*8+1, 16*8+2, 16*8+3,

	}, /* x offsets */
	{ 0*8,1*8,2*8,3*8, 4*8, 5*8, 6*8, 7*8,8*8,9*8,10*8,11*8,12*8,13*8,14*8,15*8 }, /* y offsets */
	32*8	/* offset to next tile */
};


static const gfx_layout spritelayout =
{
	16,16,
	RGN_FRAC(1,3),
	3,
	{  RGN_FRAC(2,3),RGN_FRAC(1,3), 0},
	{ 2*8*8+0,2*8*8+1,2*8*8+2,2*8*8+3,2*8*8+4,2*8*8+5,2*8*8+6,2*8*8+7,
	0,1,2,3,4,5,6,7 },
	{ 0*8,1*8,2*8,3*8,4*8,5*8,6*8,7*8,
	1*8*8+0*8,1*8*8+1*8,1*8*8+2*8,1*8*8+3*8,1*8*8+4*8,1*8*8+5*8,1*8*8+6*8,1*8*8+7*8 },
	16*16
};

static GFXDECODE_START( chanbara )
	GFXDECODE_ENTRY( "gfx1", 0x00000, tilelayout,	0x40, 32 )
	GFXDECODE_ENTRY( "gfx2", 0x00000, spritelayout,	0x80, 16 )

	GFXDECODE_ENTRY( "gfx3", 0x00000, tile16layout,	0, 32 )
GFXDECODE_END
/***************************************************************************/


static WRITE8_DEVICE_HANDLER(chanbara_ay_out_0_w)
{
//  printf("chanbara_ay_out_0_w %02x\n",data);
	scroll=data;
}

static WRITE8_DEVICE_HANDLER(chanbara_ay_out_1_w)
{
//  printf("chanbara_ay_out_1_w %02x\n",data);
	memory_set_bankptr(device->machine, 1, memory_region(device->machine, "user1") + ((data&4)?0x4000:0x0000) );
	scrollhi = data & 0x03;

	//if (data&0xf8)    printf("chanbara_ay_out_1_w unused bits set %02x\n",data&0xf8);
}

static void sound_irq(const device_config *device, int linestate)
{
	cputag_set_input_line(device->machine, "maincpu", 0, linestate);
}


static const ym2203_interface ym2203_config =
{
	{
			AY8910_LEGACY_OUTPUT,
			AY8910_DEFAULT_LOADS,
			DEVCB_NULL,
			DEVCB_NULL,
			DEVCB_HANDLER(chanbara_ay_out_0_w),
			DEVCB_HANDLER(chanbara_ay_out_1_w),
	},
	sound_irq
};

static MACHINE_DRIVER_START( chanbara )
	MDRV_CPU_ADD("maincpu", M6809, 12000000/8)
	MDRV_CPU_PROGRAM_MAP(memmap)

	/* video hardware */
	MDRV_SCREEN_ADD("screen", RASTER)
	MDRV_SCREEN_REFRESH_RATE(57.4122)
	MDRV_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(2500) /* not accurate */)
	MDRV_SCREEN_FORMAT(BITMAP_FORMAT_INDEXED16)
	MDRV_SCREEN_SIZE(32*8, 32*8)
	MDRV_SCREEN_VISIBLE_AREA(0, 32*8-1, 2*8, 30*8-1)

	MDRV_GFXDECODE(chanbara)
	MDRV_PALETTE_LENGTH(256)
	MDRV_PALETTE_INIT(chanbara)

	MDRV_VIDEO_START(chanbara)
	MDRV_VIDEO_UPDATE(chanbara)

	MDRV_SPEAKER_STANDARD_MONO("mono")

	MDRV_SOUND_ADD("ymsnd", YM2203, 12000000/8)
	MDRV_SOUND_CONFIG(ym2203_config)
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "mono", 1.0)
MACHINE_DRIVER_END


ROM_START( chanbara )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "cp01.16c",     0x08000, 0x4000, CRC(a0c3c24c) SHA1(8445dc39dd763187a2d66c6165b487f146e7d474))
	ROM_LOAD( "cp00-2.17c",   0x0c000, 0x4000, CRC(a045e463) SHA1(2eb546e16f163be6ed72238f2f0203527a957efd) )

	ROM_REGION( 0x8000, "user1", 0 ) // background data
	ROM_LOAD( "cp02.14c",     0x00000, 0x8000, CRC(c2b66cea) SHA1(f72f57add5f38313a72f5c521dce157edf49f70e) )

	ROM_REGION( 0x02000, "gfx1", 0 ) // text layer
	ROM_LOAD( "cp12.17h",     	0x00000, 0x2000, CRC(b87b96de) SHA1(f8bb9f094917df305c4fed071edaa775071e40fd) )

	ROM_REGION( 0x08000, "gfx3", 0 ) // bg layer
	ROM_LOAD( "cp13.15h",     	0x00000, 0x4000, CRC(2dc38c3d) SHA1(4bb1335b8285e91b51c28e74d8de11a8d6df0486) )
	/* rom cp14.13h is expanded at 0x4000 - 0x8000 */

	ROM_REGION( 0x08000, "gfx4", 0 )
	ROM_LOAD( "cp14.13h",     	0x00000, 0x2000, CRC(d31db368) SHA1(b62834137bfe4ac2013d2d16b0ead10bf2a2df83) )

	ROM_REGION( 0x24000, "gfx2", 0 )
	ROM_LOAD( "cp03.12c",     0x08000, 0x4000, CRC(dea247fb) SHA1(d54fa30813613ef6c3b5f86b563e9ab618a9f627))
	ROM_LOAD( "cp04.10c",     0x04000, 0x4000, CRC(f7dce87b) SHA1(129ae41d70d96720e020ec1bc1d3f2d9e87ebf47) )
	ROM_LOAD( "cp05.9c",      0x00000, 0x4000, CRC(df2dc3cb) SHA1(3505042c91566bb09fcd2102fecbe2034551b8eb) )

	ROM_LOAD( "cp06.7c",     0x14000, 0x4000,  CRC(2f337c08) SHA1(657ee6776780fa0a979a278ff27a49b459232cad) )
	ROM_LOAD( "cp07.6c",     0x10000, 0x4000, CRC(0e3727f2) SHA1(d177651bc20a56f5651ae5ce6f3d3ff7ad0e2053) )
	ROM_LOAD( "cp08.5c",     0x0c000, 0x4000, CRC(4cf35192) SHA1(1891dcc412caf72ba5a2ea56c1cab35cb3ae6123) )

	ROM_LOAD( "cp09.4c",     0x20000, 0x4000, CRC(3f58b647) SHA1(4eb212667aedd7c397a4911ac7f1b542c5c0a70d) )
	ROM_LOAD( "cp10.2c",     0x1c000, 0x4000, CRC(bfa324c0) SHA1(c7ff09bb5f1dd2d3707970fae1fd60b6004250c0) )
	ROM_LOAD( "cp11.1c",     0x18000, 0x4000, CRC(33e6160a) SHA1(b0171b554825072eebe935d12a6085d158b87bdc) )

	ROM_REGION( 0x0300, "proms", 0 )
	ROM_LOAD( "cp17.4k", 0x0000, 0x0100, CRC(cf03706e) SHA1(2dd2b29067f418ec590c56a38cc64d09d8dc8e09) ) /* red */
	ROM_LOAD( "cp16.5k", 0x0100, 0x0100, CRC(5fedc8ba) SHA1(8b685ce71d833fefb3e4502d1dd0cca96ba9162a) ) /* green */
	ROM_LOAD( "cp15.6k", 0x0200, 0x0100, CRC(655936eb) SHA1(762b419c0571fafd8e1c5e96d0d94999768ba325) ) /* blue */
ROM_END


static DRIVER_INIT(chanbara )
{
	UINT8	*src = memory_region(machine, "gfx4");
	UINT8	*dst = memory_region(machine, "gfx3")+0x4000;

	int i;
	for (i=0;i<0x1000;i++)
	{
		dst[i+0x1000] = src[i]&0xf0;
		dst[i+0x0000] = (src[i]&0x0f)<<4;
		dst[i+0x3000] = src[i+0x1000]&0xf0;
		dst[i+0x2000] = (src[i+0x1000]&0x0f)<<4;
	}
}

GAME( 1985, chanbara, 0,		chanbara, chanbara, chanbara, ROT270, "Data East", "Chanbara", 0 )
