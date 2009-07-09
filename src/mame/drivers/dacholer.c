/******************************************************************************

    Dacholer    (c) 1983 Nichibutsu
    Kick Boy    (c) 1983 Nichibutsu

    Driver by Pierpaolo Prazzoli

    TODO:
    - Add colors when proms are dumped

Mods by Tomasz Slanina (2008.06.12):

- fixed sound cpu interrupts (mode 2 (two vectors)+ nmi)
- added sound and music.
- ay/msm clocks are arbitrary
- just a guess - upper nibble of byte from port 3 _probably_
  contains sound command (sound cpu writes it to port c)

******************************************************************************/

#include "driver.h"
#include "cpu/z80/z80.h"
#include "sound/dac.h"
#include "sound/msm5205.h"
#include "sound/ay8910.h"

static int msm_data = 0;
static int msm_toggle=0;

static UINT8 *bgvideoram,*fgvideoram;
static int bg_bank = 0;
static UINT8 snd_interrupt_enable=0;
static UINT8 music_interrupt_enable=0;
static UINT8 snd_ack;

static tilemap *bg_tilemap,*fg_tilemap;

static WRITE8_HANDLER( background_w )
{
	bgvideoram[offset] = data;
	tilemap_mark_tile_dirty(bg_tilemap,offset);
}

static WRITE8_HANDLER( foreground_w )
{
	fgvideoram[offset] = data;
	tilemap_mark_tile_dirty(fg_tilemap,offset);
}

static WRITE8_HANDLER( bg_bank_w )
{
	if((data & 3) != bg_bank)
	{
		bg_bank = data & 3;
		tilemap_mark_all_tiles_dirty(bg_tilemap);
	}

	flip_screen_set(space->machine, data & 0xc); // probably one bit for flipx and one for flipy

}

static WRITE8_HANDLER( coins_w )
{
	coin_counter_w(0, data & 1);
	coin_counter_w(1, data & 2);

	set_led_status(0, data & 4);
	set_led_status(1, data & 8);
}

static WRITE8_HANDLER(snd_w)
{
	soundlatch_w(space, offset, data);
	cputag_set_input_line(space->machine, "audiocpu", INPUT_LINE_NMI, PULSE_LINE);
}

static ADDRESS_MAP_START( main_map, ADDRESS_SPACE_PROGRAM, 8 )
	AM_RANGE(0x0000, 0x7fff) AM_ROM
	AM_RANGE(0x8800, 0x97ff) AM_RAM
	AM_RANGE(0xc000, 0xc3ff) AM_RAM_WRITE(background_w) AM_BASE(&bgvideoram)
	AM_RANGE(0xd000, 0xd3ff) AM_RAM_WRITE(foreground_w) AM_BASE(&fgvideoram)
	AM_RANGE(0xe000, 0xe0ff) AM_RAM AM_BASE(&spriteram) AM_SIZE(&spriteram_size)
ADDRESS_MAP_END

static ADDRESS_MAP_START( main_io_map, ADDRESS_SPACE_IO, 8 )
	ADDRESS_MAP_GLOBAL_MASK(0xff)
	AM_RANGE(0x00, 0x00) AM_READ_PORT("P1")
	AM_RANGE(0x01, 0x01) AM_READ_PORT("P2")
	AM_RANGE(0x02, 0x02) AM_READ_PORT("SYSTEM")
	AM_RANGE(0x03, 0x03) AM_READ_PORT("DSWA")
	AM_RANGE(0x04, 0x04) AM_READ_PORT("DSWB")
	AM_RANGE(0x20, 0x20) AM_WRITE(coins_w)
	AM_RANGE(0x21, 0x21) AM_WRITE(bg_bank_w)
	AM_RANGE(0x22, 0x22) AM_WRITENOP
	AM_RANGE(0x23, 0x23) AM_WRITENOP
	AM_RANGE(0x24, 0x24) AM_WRITENOP
	AM_RANGE(0x27, 0x27) AM_WRITE(snd_w)
ADDRESS_MAP_END


static ADDRESS_MAP_START( snd_map, ADDRESS_SPACE_PROGRAM, 8 )
	AM_RANGE(0x0000, 0x5fff) AM_ROM
	AM_RANGE(0xd000, 0xe7ff) AM_RAM
ADDRESS_MAP_END

static WRITE8_HANDLER( adpcm_w )
{
	msm_data = data;
	msm_toggle = 0;
}

static WRITE8_HANDLER( snd_ack_w )
{
	snd_ack = data;
}

static CUSTOM_INPUT( snd_ack_r )
{
	return snd_ack;		//guess ...
}

static WRITE8_HANDLER( snd_irq_w )
{
	snd_interrupt_enable = data;
}

static WRITE8_HANDLER( music_irq_w )
{
	music_interrupt_enable = data;
}

static ADDRESS_MAP_START( snd_io_map, ADDRESS_SPACE_IO, 8 )
	ADDRESS_MAP_GLOBAL_MASK(0xff)
	AM_RANGE(0x00, 0x00) AM_READWRITE(soundlatch_r, soundlatch_clear_w )
	AM_RANGE(0x04, 0x04) AM_WRITE(music_irq_w)
	AM_RANGE(0x08, 0x08) AM_WRITE(snd_irq_w)
	AM_RANGE(0x0c, 0x0c) AM_WRITE(snd_ack_w)
	AM_RANGE(0x80, 0x80) AM_WRITE(adpcm_w)
	AM_RANGE(0x86, 0x87) AM_DEVWRITE("ay1", ay8910_data_address_w)
	AM_RANGE(0x8a, 0x8b) AM_DEVWRITE("ay2", ay8910_data_address_w)
	AM_RANGE(0x8e, 0x8f) AM_DEVWRITE("ay3", ay8910_data_address_w)
ADDRESS_MAP_END


static INPUT_PORTS_START( dacholer )
	PORT_START("P1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_UP )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON2 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("P2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_COCKTAIL
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_COCKTAIL
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_COCKTAIL
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_COCKTAIL
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_COCKTAIL
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_COCKTAIL
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("SYSTEM")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_SERVICE( 0x08, IP_ACTIVE_LOW )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("DSWA")
	PORT_DIPNAME( 0x03, 0x03, DEF_STR( Coin_A ) )           /* table at 0x0a8c */
	PORT_DIPSETTING(    0x01, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x03, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 2C_3C ) )
	PORT_DIPSETTING(    0x02, DEF_STR( 1C_2C ) )
	PORT_DIPNAME( 0x0c, 0x0c, DEF_STR( Coin_B ) )           /* table at 0x0a94 */
	PORT_DIPSETTING(    0x00, DEF_STR( 6C_1C ) )
	PORT_DIPSETTING(    0x04, DEF_STR( 5C_1C ) )
	PORT_DIPSETTING(    0x08, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(    0x0c, DEF_STR( 1C_1C ) )
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_SPECIAL ) PORT_CUSTOM(snd_ack_r, NULL)

	PORT_START("DSWB")
	PORT_DIPNAME( 0x03, 0x03, DEF_STR( Lives ) )            /* table at 0x0a9c */
	PORT_DIPSETTING(    0x03, "1" )
	PORT_DIPSETTING(    0x02, "2" )
	PORT_DIPSETTING(    0x01, "3" )
	PORT_DIPSETTING(    0x00, "4" )
//  PORT_DIPNAME( 0x0c, 0x0c, "1st Bonus Life" )            /* table at 0x0aa0 */
//  PORT_DIPSETTING(    0x0c, "20k" )
//  PORT_DIPSETTING(    0x08, "30k" )
//  PORT_DIPSETTING(    0x04, "40k" )
//  PORT_DIPSETTING(    0x00, DEF_STR( None ) )
//  PORT_DIPNAME( 0x30, 0x30, "Next Bonus Lifes" )          /* table at 0x0aa8 */
//  PORT_DIPSETTING(    0x30, "+50k" )
//  PORT_DIPSETTING(    0x20, "+70k" )
//  PORT_DIPSETTING(    0x10, "+100k" )
//  PORT_DIPSETTING(    0x00, DEF_STR( None ) )
	PORT_DIPNAME( 0x3c, 0x3c, DEF_STR( Bonus_Life ) )
	PORT_DIPSETTING(    0x3c, "20k 70k then every 50k" )
	PORT_DIPSETTING(    0x38, "30k 80k then every 50k" )
	PORT_DIPSETTING(    0x34, "40k 90k then every 50k" )
//  PORT_DIPSETTING(    0x30, DEF_STR( None ) )
	PORT_DIPSETTING(    0x2c, "20k 90k then every 70k" )
	PORT_DIPSETTING(    0x28, "30k 100k then every 70k" )
	PORT_DIPSETTING(    0x24, "40k 110k then every 70k" )
//  PORT_DIPSETTING(    0x20, DEF_STR( None ) )
	PORT_DIPSETTING(    0x1c, "20k 120k then every 100k" )
	PORT_DIPSETTING(    0x18, "30k 130k then every 100k" )
	PORT_DIPSETTING(    0x14, "40k 140k then every 100k" )
//  PORT_DIPSETTING(    0x10, DEF_STR( None ) )
	PORT_DIPSETTING(    0x0c, "20k only" )
	PORT_DIPSETTING(    0x08, "30k only" )
	PORT_DIPSETTING(    0x04, "40k only" )
	PORT_DIPSETTING(    0x00, DEF_STR( None ) )
	PORT_DIPNAME( 0x40, 0x00, DEF_STR( Demo_Sounds ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x00, DEF_STR( Cabinet ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Upright ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Cocktail ) )
INPUT_PORTS_END

static INPUT_PORTS_START( kickboy )
	PORT_INCLUDE(dacholer)

	PORT_MODIFY("DSWA")
	PORT_DIPNAME( 0x03, 0x03, DEF_STR( Coin_A ) )           /* table at 0x0f71 - same as in 'dacholer' */
	PORT_DIPSETTING(    0x01, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x03, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 2C_3C ) )
	PORT_DIPSETTING(    0x02, DEF_STR( 1C_2C ) )
	PORT_DIPNAME( 0x0c, 0x0c, DEF_STR( Coin_B ) )           /* table at 0x0f79 */
	PORT_DIPSETTING(    0x04, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x0c, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x08, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Free_Play ) )

	PORT_MODIFY("DSWB")
	PORT_DIPNAME( 0x03, 0x01, DEF_STR( Lives ) )            /* table at 0x0f81 */
	PORT_DIPSETTING(    0x03, "3" )
	PORT_DIPSETTING(    0x02, "4" )
	PORT_DIPSETTING(    0x01, "5" )
	PORT_DIPSETTING(    0x00, "99 (Cheat)" )
//  PORT_DIPNAME( 0x0c, 0x0c, "1st Bonus Life" )            /* table at 0x0f85 - same as in 'dacholer' */
//  PORT_DIPSETTING(    0x0c, "20k" )
//  PORT_DIPSETTING(    0x08, "30k" )
//  PORT_DIPSETTING(    0x04, "40k" )
//  PORT_DIPSETTING(    0x00, DEF_STR( None ) )
//  PORT_DIPNAME( 0x30, 0x30, "Next Bonus Lifes" )          /* table at 0x0f8d */
//  PORT_DIPSETTING(    0x30, "+30k" )
//  PORT_DIPSETTING(    0x20, "+50k" )
//  PORT_DIPSETTING(    0x10, "+70k" )
//  PORT_DIPSETTING(    0x00, DEF_STR( None ) )
	PORT_DIPNAME( 0x3c, 0x3c, DEF_STR( Bonus_Life ) )
	PORT_DIPSETTING(    0x3c, "20k 50k then every 30k" )
	PORT_DIPSETTING(    0x38, "30k 60k then every 30k" )
	PORT_DIPSETTING(    0x34, "40k 70k then every 30k" )
//  PORT_DIPSETTING(    0x30, DEF_STR( None ) )
	PORT_DIPSETTING(    0x2c, "20k 70k then every 50k" )
	PORT_DIPSETTING(    0x28, "30k 80k then every 50k" )
	PORT_DIPSETTING(    0x24, "40k 90k then every 50k" )
//  PORT_DIPSETTING(    0x20, DEF_STR( None ) )
	PORT_DIPSETTING(    0x1c, "20k 90k then every 70k" )
	PORT_DIPSETTING(    0x18, "30k 100k then every 70k" )
	PORT_DIPSETTING(    0x14, "40k 110k then every 70k" )
//  PORT_DIPSETTING(    0x10, DEF_STR( None ) )
	PORT_DIPSETTING(    0x0c, "20k only" )
	PORT_DIPSETTING(    0x08, "30k only" )
	PORT_DIPSETTING(    0x04, "40k only" )
	PORT_DIPSETTING(    0x00, DEF_STR( None ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Difficulty ) )       /* stored at 0x920f */
	PORT_DIPSETTING(    0x40, DEF_STR( Easy ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Hard ) )
INPUT_PORTS_END


static TILE_GET_INFO( get_bg_tile_info )
{
	SET_TILE_INFO(1,bgvideoram[tile_index] + bg_bank * 0x100,0,0);
}

static TILE_GET_INFO( get_fg_tile_info )
{
	SET_TILE_INFO(0,fgvideoram[tile_index],0,0);
}

static VIDEO_START( dacholer )
{
	bg_tilemap = tilemap_create(machine, get_bg_tile_info,tilemap_scan_rows,8,8,32,32);
	fg_tilemap = tilemap_create(machine, get_fg_tile_info,tilemap_scan_rows,8,8,32,32);

	tilemap_set_transparent_pen(fg_tilemap,0);
}

static void draw_sprites(running_machine *machine, bitmap_t *bitmap, const rectangle *cliprect )
{
	int offs,code,attr,sx,sy,flipx,flipy;

	for (offs = 0;offs < spriteram_size;offs += 4)
	{
		code = spriteram[offs+1];
		attr = spriteram[offs+2];

		flipx = attr & 0x10;
		flipy = attr & 0x20;

		sx = (spriteram[offs+3] - 128) + 256 * (attr & 0x01);
		sy = 248 - spriteram[offs];

		if (flip_screen_get(machine))
		{
			sx = 240 - sx;
			sy = 240 - sy;
			flipx = !flipx;
			flipy = !flipy;
		}

		drawgfx_transpen(bitmap,cliprect,machine->gfx[2],
				code,
				0,
				flipx,flipy,
				sx,sy,0);
	}
}

static VIDEO_UPDATE(dacholer)
{
	tilemap_draw(bitmap,cliprect,bg_tilemap,0,0);
	tilemap_draw(bitmap,cliprect,fg_tilemap,0,0);
	draw_sprites(screen->machine,bitmap,cliprect);
	return 0;
}

static const gfx_layout charlayout =
{
	8,8,
	RGN_FRAC(1,1),
	4,
	{ 0, 1, 2, 3 },
	{ 1*4, 0*4, 3*4, 2*4, 5*4, 4*4, 7*4, 6*4 },
	{ 0*32, 1*32, 2*32, 3*32, 4*32, 5*32, 6*32, 7*32 },
	32*8
};

static const gfx_layout spritelayout =
{
	16,16,
	RGN_FRAC(1,1),
	4,
	{ 0, 1, 2, 3 },
	{ 4,0,12,8,20,16,28,24,36,32,44,40,52,48,60,56 },
	{ 0*64, 1*64, 2*64, 3*64, 4*64, 5*64, 6*64, 7*64,
	  8*64, 9*64, 10*64, 11*64, 12*64, 13*64, 14*64, 15*64 },
	16*16*4
};

static GFXDECODE_START( dacholer )
	GFXDECODE_ENTRY( "gfx1", 0, charlayout,   0, 16 )
	GFXDECODE_ENTRY( "gfx2", 0, charlayout,   0, 16 )
	GFXDECODE_ENTRY( "gfx3", 0, spritelayout, 0, 16 )
GFXDECODE_END


static INTERRUPT_GEN( sound_irq )
{
	if(music_interrupt_enable == 1)
	{
		cpu_set_input_line_and_vector(device, 0, HOLD_LINE, 0x30);
	}
}

static void adpcm_int(const device_config *device)
{
	if(snd_interrupt_enable == 1 || (snd_interrupt_enable == 0 && msm_toggle == 1))
	{
		msm5205_data_w(device, msm_data >> 4);
		msm_data <<= 4;
		msm_toggle ^= 1;
		if (msm_toggle == 0)
		{
			cputag_set_input_line_and_vector(device->machine, "audiocpu", 0, HOLD_LINE, 0x38);
		}
	}
}

static const msm5205_interface msm_interface =
{
	adpcm_int,			/* interrupt function */
	MSM5205_S96_4B 	/* 1 / 96 = 3906.25Hz playback  - guess */
};

static MACHINE_DRIVER_START( dacholer )

	/* basic machine hardware */
	MDRV_CPU_ADD("maincpu", Z80, 4000000)	/* ? */
	MDRV_CPU_PROGRAM_MAP(main_map)
	MDRV_CPU_IO_MAP(main_io_map)
	MDRV_CPU_VBLANK_INT("screen", irq0_line_hold)

	MDRV_CPU_ADD("audiocpu", Z80, 4000000)	/* ? */
	MDRV_CPU_PROGRAM_MAP(snd_map)
	MDRV_CPU_IO_MAP(snd_io_map)
	MDRV_CPU_VBLANK_INT("screen",sound_irq)

	/* video hardware */
	MDRV_SCREEN_ADD("screen", RASTER)
	MDRV_SCREEN_REFRESH_RATE(60)
	MDRV_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MDRV_SCREEN_FORMAT(BITMAP_FORMAT_INDEXED16)
	MDRV_SCREEN_SIZE(256, 256)
	MDRV_SCREEN_VISIBLE_AREA(0, 256-1, 16, 256-1-16)

	MDRV_PALETTE_LENGTH(16)
	MDRV_GFXDECODE(dacholer)

	MDRV_VIDEO_START(dacholer)
	MDRV_VIDEO_UPDATE(dacholer)

	/* sound hardware */
	MDRV_SPEAKER_STANDARD_MONO("mono")

	MDRV_SOUND_ADD("ay1", AY8910, 1500000)
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.15)

	MDRV_SOUND_ADD("ay2", AY8910, 1500000)
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.15)

	MDRV_SOUND_ADD("ay3", AY8910, 1500000)
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.15)

	MDRV_SOUND_ADD("msm", MSM5205, 375000)
	MDRV_SOUND_CONFIG(msm_interface)
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.30)

MACHINE_DRIVER_END

ROM_START( dacholer )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "dacholer8.rom",  0x0000, 0x2000, CRC(8b73a441) SHA1(6de9e4845b9063af8df42aa82ad536737190582c) )
	ROM_LOAD( "dacholer9.rom",  0x2000, 0x2000, CRC(9499289f) SHA1(bcfe554eb1f8e686d193050c18278b6bf93f179f) )
	ROM_LOAD( "dacholer10.rom", 0x4000, 0x2000, CRC(39d37281) SHA1(daaf84079dd18dd854946e066e2dcde994bcbba4) )
	ROM_LOAD( "dacholer11.rom", 0x6000, 0x2000, CRC(bb781ea4) SHA1(170966c4bcd0246968850d908a69f81ea1e136d5) )

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "dacholer12.rom", 0x0000, 0x2000, CRC(cc3a4b68) SHA1(29344dc10c5d236f9a452196b3809565b4101327) )
	ROM_LOAD( "dacholer13.rom", 0x2000, 0x2000, CRC(aa18e126) SHA1(e6af334188d0edbc37a7fb4a00a325b2039172b7) )
	ROM_LOAD( "dacholer14.rom", 0x4000, 0x2000, CRC(3b0131c7) SHA1(338ca2c2c7480e1cd0bb15ee6b90d683ce06f0fd) )

	ROM_REGION( 0x2000, "gfx1", 0 )
	ROM_LOAD( "dacholer7.rom", 0x0000, 0x2000, CRC(fd649d36) SHA1(77d78eef44f348b635dbc0711e662a5236c00d51) )

	ROM_REGION( 0x6000, "gfx2", 0 )
	ROM_LOAD( "dacholer1.rom", 0x0000, 0x2000, CRC(9cca0fd2) SHA1(3ca1b4cca9611232df1195ae6ac172a79c8368c3) )
	ROM_LOAD( "dacholer2.rom", 0x2000, 0x2000, CRC(c1322b27) SHA1(8022f59b8ae10a7a911563b01bffc2d5646108a5) )
	ROM_LOAD( "dacholer3.rom", 0x4000, 0x2000, CRC(9e1e7198) SHA1(7a75da1ae09f6cf095976b48f462ede42625b244) )

	ROM_REGION( 0x6000, "gfx3", 0 )
	ROM_LOAD( "dacholer5.rom", 0x0000, 0x2000, CRC(dd4818f0) SHA1(718236932248512f8779f640e0367b5d92e6497e) )
	ROM_LOAD( "dacholer4.rom", 0x2000, 0x2000, CRC(7f338ae0) SHA1(9206ed044feb44c55990803cdf608dd899e976ff) )
	ROM_LOAD( "dacholer6.rom", 0x4000, 0x2000, CRC(0a6d4ec4) SHA1(419ea1f6ead3afb2de98432d9f8ead7447842a1e) )

	ROM_REGION( 0x0020, "proms", 0 )
	ROM_LOAD( "proms", 0x0000, 0x0020, NO_DUMP )
ROM_END

ROM_START( kickboy )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "kickboy1.rom", 0x0000, 0x2000, CRC(525746f1) SHA1(4044f880f271f77b56b2d8964ab97d34fb507c7a) )
	ROM_LOAD( "kickboy2.rom", 0x2000, 0x2000, CRC(9d091725) SHA1(827cea1c371094720b47fda271945cee20c9d956) )
	ROM_LOAD( "kickboy3.rom", 0x4000, 0x2000, CRC(d61b6ff6) SHA1(071ab4c05ed54526144f2ba751c111e8c4bdc61a) )
	ROM_LOAD( "kickboy4.rom", 0x6000, 0x2000, CRC(a8985bfe) SHA1(a8e466a7df381dfc8dd2e3483eba0215bfec7551) )

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "kickboy5.rom", 0x0000, 0x2000, CRC(cc3a4b68) SHA1(29344dc10c5d236f9a452196b3809565b4101327) )
	ROM_LOAD( "kickboy6.rom", 0x2000, 0x2000, CRC(aa18e126) SHA1(e6af334188d0edbc37a7fb4a00a325b2039172b7) )
	ROM_LOAD( "kickboy7.rom", 0x4000, 0x2000, CRC(3b0131c7) SHA1(338ca2c2c7480e1cd0bb15ee6b90d683ce06f0fd) )

	ROM_REGION( 0x2000, "gfx1", 0 )
	ROM_LOAD( "kickboy13.rom", 0x0000, 0x2000, CRC(22be46e8) SHA1(d92b3913d8eba881c69acd1d85ca73ee58489fae) )

	ROM_REGION( 0x4000, "gfx2", 0 )
	ROM_LOAD( "kickboy9.rom",  0x0000, 0x2000, CRC(7eac2a64) SHA1(b4a44770bbded59cd572ac5d0ae178affc8cdab8) )
	ROM_LOAD( "kickboy8.rom",  0x2000, 0x2000, CRC(b8829572) SHA1(01009ec63449c809608923fd9dcecd82b29c5d6d) )

	ROM_REGION( 0x6000, "gfx3", 0 )
	ROM_LOAD( "kickboy11.rom", 0x0000, 0x2000, CRC(4b769a1c) SHA1(fde17dcd4b7cda9cc54572e81bc2f0e48c19277d) )
	ROM_LOAD( "kickboy10.rom", 0x2000, 0x2000, CRC(45199750) SHA1(a04b4d6d0defa613d269625b089d28dc68d5b73a) )
	ROM_LOAD( "kickboy12.rom", 0x4000, 0x2000, CRC(d1795506) SHA1(e0f7a64e301cf43c4739031461dba16aa44100a1) )

	ROM_REGION( 0x0020, "proms", 0 )
	ROM_LOAD( "proms", 0x0000, 0x0020, NO_DUMP )
ROM_END

GAME( 1983, dacholer, 0, dacholer, dacholer, 0, ROT0, "Nichibutsu", "Dacholer", GAME_WRONG_COLORS )
GAME( 1983, kickboy,  0, dacholer, kickboy,  0, ROT0, "Nichibutsu", "Kick Boy", GAME_WRONG_COLORS )
