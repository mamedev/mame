/* 'Multifish' hardware

  four ALTERA chips - they are probably video processors
  main CPU is z80 for sure, ROM is placed near it
  there are banked ROMs 8kb each, consisted of several banks
  RAM has E000 address
  the sound is PSG - AY3-8910 analog (http://pt.wikipedia.org/wiki/KC89C72)
  z80 CPU is complete with indexed registers
  video - VGA

  To Init the games

  Turn Service Mode ON (press 'F2')
  Reset the game (press 'F3')
  Use 'C' to move pointer to INIT
  Press '1' (Start) to enter INIT menu
  Hold 'Z' (Bet/Double) for 5 seconds while counter counts down
  Turn Service Mode OFF (press 'F2')
  Reset the game (press 'F3')


  Todo:
  -------------------------------------------------------------------------
  Payout doesn't currently work and causes 'Call Attendant' to be displayed
  Hook up Lamps
  Emulate the RTC (real time clock)


  NOTE:
  Revision information comes from dat files, not all of them can be tested
  becuase some program rom revisions don't seem to be dumped.

  For sets where the program rom hasn't been verified, the MD5 hash is given
  but not the CRC32 hash. These sets are excluded using the 'ALL_REVISIONS'
  "#if" statements but listed in the driver below.

*/

#define ALL_REVISIONS 0

#include "driver.h"
#include "sound/ay8910.h"
#include "cpu/z80/z80.h"

#define multfish_VIDRAM_SIZE (0x2000*0x10)
#define multfish_BRAM_SIZE (0x2000*0x10)

static UINT8* multfish_vid;
static UINT8* multfish_bram;

static int multfish_disp_enable;

/* Video Part */

static tilemap *multfish_tilemap;
static tilemap *multfish_reel_tilemap;


static TILE_GET_INFO( get_multfish_tile_info )
{
	int code = multfish_vid[tile_index*2+0x0000] | (multfish_vid[tile_index*2+0x0001] << 8);
	int attr = multfish_vid[tile_index*2+0x1000] | (multfish_vid[tile_index*2+0x1001] << 8);

	tileinfo->category = (attr&0x100)>>8;

	SET_TILE_INFO(
			0,
			code&0x1fff,
			attr&0x7,
			0);
}

static TILE_GET_INFO( get_multfish_reel_tile_info )
{
	int code = multfish_vid[tile_index*2+0x2000] | (multfish_vid[tile_index*2+0x2001] << 8);

	SET_TILE_INFO(
			0,
			(code&0x1fff)+0x2000,
			(code>>14)+0x8,
			0);
}


static VIDEO_START(multfish)
{
	multfish_vid = auto_alloc_array(machine, UINT8, multfish_VIDRAM_SIZE);
	memset(multfish_vid,0x00,multfish_VIDRAM_SIZE);
	state_save_register_global_pointer(machine, multfish_vid, multfish_VIDRAM_SIZE);

	multfish_bram = auto_alloc_array(machine, UINT8, multfish_BRAM_SIZE);
	memset(multfish_bram,0x00,multfish_BRAM_SIZE);
	state_save_register_global_pointer(machine, multfish_bram, multfish_BRAM_SIZE);

	multfish_tilemap = tilemap_create(machine,get_multfish_tile_info,tilemap_scan_rows,16,16, 64, 32);
	tilemap_set_transparent_pen(multfish_tilemap,255);

	multfish_reel_tilemap = tilemap_create(machine,get_multfish_reel_tile_info,tilemap_scan_rows,16,16, 64, 64);
	tilemap_set_transparent_pen(multfish_reel_tilemap,255);
	tilemap_set_scroll_cols(multfish_reel_tilemap, 64);
}





static VIDEO_UPDATE(multfish)
{
	int i;
	bitmap_fill(bitmap, cliprect, get_black_pen(screen->machine));

	if (!multfish_disp_enable) return 0;

	/* Draw lower part of static tilemap (low pri tiles) */
	tilemap_draw(bitmap,cliprect,multfish_tilemap,TILEMAP_DRAW_CATEGORY(1),0);

	/* Setup the column scroll and draw the reels */
	for (i=0;i<64;i++)
	{
		int colscroll = (multfish_vid[i*2] | multfish_vid[i*2+1] << 8);
		tilemap_set_scrolly(multfish_reel_tilemap, i, colscroll );
	}
	tilemap_draw(bitmap,cliprect,multfish_reel_tilemap,0,0);

	/* Draw upper part of static tilemap (high pri tiles) */
	tilemap_draw(bitmap,cliprect,multfish_tilemap,TILEMAP_DRAW_CATEGORY(0),0);

	return 0;
}

static WRITE8_HANDLER( multfish_vid_w )
{
	multfish_vid[offset]=data;

	// 0x0000 - 0x1fff is normal tilemap
	if (offset < 0x2000)
	{
		tilemap_mark_tile_dirty(multfish_tilemap,(offset&0xfff)/2);

	}
	// 0x2000 - 0x2fff is for the reels
	else if (offset < 0x4000)
	{
		tilemap_mark_tile_dirty(multfish_reel_tilemap,(offset&0x1fff)/2);
	}
	else if (offset < 0x6000)
	{
		int r,g,b;
		int coldat;

		coldat = multfish_vid[(offset&0xfffe)] | (multfish_vid[(offset&0xfffe)^1] << 8);

		r = ( (coldat &0x001f)>> 0);
		g = ( (coldat &0x1f00)>> 8);
		b = ( (coldat &0x00e0)>> (5));
		b|= ( (coldat &0xe000)>> (8+5-3));

		palette_set_color_rgb(space->machine, (offset-0x4000)/2, r<<3, g<<3, b<<2);
	}
	else
	{
		// probably just work ram
	}
}

static WRITE8_HANDLER( multfish_bank_w )
{
	memory_set_bank(space->machine, 1, data & 0x0f);
}

static UINT8 rambk = 0;
static UINT8 otherrambk = 0;

static READ8_HANDLER( bankedram_r )
{
	if ((otherrambk & 0x80) == 0x00)
	{
		return multfish_bram[offset+0x2000*rambk];
	}
	else
	{
		return multfish_vid[offset+0x2000*rambk];
	}

}

static WRITE8_HANDLER( bankedram_w )
{
	if ((otherrambk & 0x80) == 0x00)
	{
		multfish_bram[offset+0x2000*rambk] = data;
	}
	else
	{
		multfish_vid_w(space, offset+0x2000*rambk, data);
	}
}

static WRITE8_HANDLER( multfish_rambank_w )
{
	rambk = data & 0x0f;
	otherrambk = data & 0xf0;
}


static READ8_HANDLER( ray_r )
{
	// the games read the raster beam position as part of the hardware checks..
	// with a 6mhz clock and 640x480 resolution this seems to give the right results.
	return video_screen_get_vpos(space->machine->primary_screen);
}



static ADDRESS_MAP_START( multfish_map, ADDRESS_SPACE_PROGRAM, 8 )
	AM_RANGE(0x0000, 0x7fff) AM_READWRITE(SMH_ROM, multfish_vid_w)
	AM_RANGE(0x8000, 0xbfff) AM_READWRITE(SMH_BANK(1), SMH_ROM )
	AM_RANGE(0xc000, 0xdfff) AM_RAM AM_BASE(&generic_nvram) AM_SIZE(&generic_nvram_size)
	AM_RANGE(0xe000, 0xffff) AM_READWRITE(bankedram_r, bankedram_w)
ADDRESS_MAP_END

// According to the self test the 7 user buttons are arranged as
// Bet/Cancel  |  1 Line  |  3 Lines  |  5 Lines  | 7 Lines  | 9 Lines  | Start


static INPUT_PORTS_START( multfish )
	PORT_START("IN0")
	PORT_DIPNAME(     0x01, 0x01, "Key In (35 A)" ) // Key In ( 35 A )
	PORT_DIPSETTING(  0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(  0x00, DEF_STR( On ) )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_COIN2 ) // COIN B (18 B)
	PORT_DIPNAME(     0x04, 0x04, "S Reserve (14 B)" ) // S Reserve ( 14 B )
	PORT_DIPSETTING(  0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(  0x00, DEF_STR( On ) )
	PORT_DIPNAME(     0x08, 0x08, "Hopper 3 ( 10 B )" ) // Hooper 3 ( 10 B )
	PORT_DIPSETTING(  0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(  0x00, DEF_STR( On ) )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON5 ) PORT_NAME("7 Lines") PORT_CODE(KEYCODE_B)
	PORT_BIT( 0xe0, IP_ACTIVE_LOW, IPT_UNUSED ) // unused?

	PORT_START("IN1")
	PORT_DIPNAME(     0x01, 0x01, "Hopper SW (22 B)" )
	PORT_DIPSETTING(  0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(  0x00, DEF_STR( On ) )
	PORT_DIPNAME(     0x02, 0x02, "BK Door (17 A)"  )
	PORT_DIPSETTING(  0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(  0x00, DEF_STR( On ) )
	PORT_DIPNAME(     0x04, 0x04, "P Reserve (13 A)" )
	PORT_DIPSETTING(  0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(  0x00, DEF_STR( On ) )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_START1 ) PORT_NAME("Start")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_COIN8 ) // BILL 4 (07 A)
	PORT_BIT( 0xe0, IP_ACTIVE_LOW, IPT_UNUSED ) // unused?


	PORT_START("IN2")
	PORT_DIPNAME(     0x01, 0x01, "Unused??" ) // unused?
	PORT_DIPSETTING(  0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(  0x00, DEF_STR( On ) )
	PORT_DIPNAME(     0x02, 0x02, "Call Att (17 A)" )
	PORT_DIPSETTING(  0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(  0x00, DEF_STR( On ) )
	PORT_DIPNAME(     0x04, 0x04, "S Reserve (13 B)" )
	PORT_DIPSETTING(  0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(  0x00, DEF_STR( On ) )
	PORT_DIPNAME(     0x08, 0x08, "Hopper 2 (09 B)" )
	PORT_DIPSETTING(  0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(  0x00, DEF_STR( On ) )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_NAME("5 Lines") PORT_CODE(KEYCODE_V)
	PORT_BIT( 0xe0, IP_ACTIVE_LOW, IPT_UNUSED ) // unused?


	PORT_START("IN3")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON10 ) PORT_NAME("Pay Out") PORT_CODE(KEYCODE_D) // not currently working!
	PORT_DIPNAME(     0x02, 0x02, "S Reserve (16 B)" )
	PORT_DIPSETTING(  0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(  0x00, DEF_STR( On ) )
	PORT_DIPNAME(     0x04, 0x04, "Ticket (12 B)" )
	PORT_DIPSETTING(  0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(  0x00, DEF_STR( On ) )
	PORT_DIPNAME(     0x08, 0x08, "Hopper 1 (08 B)" )
	PORT_DIPSETTING(  0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(  0x00, DEF_STR( On ) )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_COIN5 ) // BILL 1 (04 B)
	PORT_BIT( 0xe0, IP_ACTIVE_LOW, IPT_UNUSED ) // unused?


	PORT_START("IN4")
	PORT_DIPNAME(     0x01, 0x01, "S Reserve (35 B)" )
	PORT_DIPSETTING(  0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(  0x00, DEF_STR( On ) )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_COIN3 ) // COIN C (19 A)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON9 ) PORT_NAME("Help") PORT_CODE(KEYCODE_S)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_BUTTON6 ) PORT_NAME("9 Lines") PORT_CODE(KEYCODE_N)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_NAME("1 Line") PORT_CODE(KEYCODE_X)
	PORT_BIT( 0xe0, IP_ACTIVE_LOW, IPT_UNUSED ) // unused?


	PORT_START("IN5")
	PORT_SERVICE(     0x01, IP_ACTIVE_LOW )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_COIN4 ) // COIN D (19 B)
	PORT_DIPNAME(     0x04, 0x04, "S Reserve (16 B)" )
	PORT_DIPSETTING(  0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(  0x00, DEF_STR( On ) )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_NAME("Bet / Double / Cancel") PORT_CODE(KEYCODE_Z)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_COIN6 ) // BILL 2 (05 A)
	PORT_BIT( 0xe0, IP_ACTIVE_LOW, IPT_UNUSED ) // unused?

	PORT_START("IN6")
	PORT_DIPNAME(     0x01, 0x01, "Short St (20 A)")
	PORT_DIPSETTING(  0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(  0x00, DEF_STR( On ) )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_COIN1 ) // COIN A (18 A)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON8 ) PORT_NAME("Max Bet") PORT_CODE(KEYCODE_A)
	PORT_DIPNAME(     0x08, 0x08, "Hopper 4 (11 A)" )
	PORT_DIPSETTING(  0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(  0x00, DEF_STR( On ) )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_NAME("3 Lines") PORT_CODE(KEYCODE_C)
	PORT_BIT( 0xe0, IP_ACTIVE_LOW, IPT_UNUSED ) // unused?

	PORT_START("IN7")
	PORT_DIPNAME(     0x01, 0x01, "Key Out (21 B)" )
	PORT_DIPSETTING(  0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(  0x00, DEF_STR( On ) )
	PORT_DIPNAME(     0x02, 0x02, "Fr Door (16 A)" )
	PORT_DIPSETTING(  0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(  0x00, DEF_STR( On ) )
	PORT_DIPNAME(     0x04, 0x04, "P Reserve (12 A)" )
	PORT_DIPSETTING(  0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(  0x00, DEF_STR( On ) )
	PORT_DIPNAME(     0x08, 0x08, "P Reserve (11 A)" )
	PORT_DIPSETTING(  0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(  0x00, DEF_STR( On ) )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_COIN7 ) // BILL 3 (06 A)
	PORT_BIT( 0xe0, IP_ACTIVE_LOW, IPT_UNUSED ) // unused?
INPUT_PORTS_END


static WRITE8_HANDLER( multfish_f3_w )
{
	//popmessage("multfish_f3_w %02x",data);
}


static WRITE8_HANDLER( multfish_f4_w )
{
	//popmessage("multfish_f4_w %02x",data); // display enable?
	multfish_disp_enable = data;
}

static ADDRESS_MAP_START( multfish_portmap, ADDRESS_SPACE_IO, 8 )
	ADDRESS_MAP_GLOBAL_MASK(0xff)
	AM_RANGE(0x10, 0x10) AM_READ_PORT("IN0")
	AM_RANGE(0x11, 0x11) AM_READ_PORT("IN1")
	AM_RANGE(0x12, 0x12) AM_READ_PORT("IN2")
	AM_RANGE(0x13, 0x13) AM_READ_PORT("IN3")
	AM_RANGE(0x14, 0x14) AM_READ_PORT("IN4")
	AM_RANGE(0x15, 0x15) AM_READ_PORT("IN5")
	AM_RANGE(0x16, 0x16) AM_READ_PORT("IN6")
	AM_RANGE(0x17, 0x17) AM_READ_PORT("IN7")

	/* Write ports not hooked up yet (lights etc.) */
//  AM_RANGE(0x30, 0x30) AM_WRITE(multfish_port30_w)
//  AM_RANGE(0x31, 0x31) AM_WRITE(multfish_port31_w)
//  AM_RANGE(0x32, 0x32) AM_WRITE(multfish_port32_w)
//  AM_RANGE(0x33, 0x33) AM_WRITE(multfish_port33_w)
//  AM_RANGE(0x34, 0x34) AM_WRITE(multfish_port34_w)
//  AM_RANGE(0x35, 0x35) AM_WRITE(multfish_port35_w)
//  AM_RANGE(0x36, 0x36) AM_WRITE(multfish_port36_w)
//  AM_RANGE(0x37, 0x37) AM_WRITE(multfish_watchdog_reset_w)
	AM_RANGE(0x38, 0x38) AM_DEVWRITE("ay", ay8910_address_w)
	AM_RANGE(0x39, 0x39) AM_DEVWRITE("ay", ay8910_data_w)
	AM_RANGE(0x3a, 0x3a) AM_DEVREAD("ay", ay8910_r)

	AM_RANGE(0x90, 0x90) AM_READ(ray_r)

	AM_RANGE(0xe1, 0xe1)  AM_WRITE(multfish_bank_w)
	AM_RANGE(0xe5, 0xe5)  AM_WRITE(multfish_bank_w)
	AM_RANGE(0xe8, 0xe8)  AM_WRITE(multfish_bank_w) // mirror banking for some games
	AM_RANGE(0xea, 0xea)  AM_WRITE(multfish_bank_w) // mirror banking for some games

	AM_RANGE(0xf1, 0xf1)  AM_WRITE(multfish_rambank_w)
	AM_RANGE(0xf3, 0xf3)  AM_WRITE(multfish_f3_w) // from 00->01 at startup, irq enable maybe?
	AM_RANGE(0xf4, 0xf4)  AM_WRITE(multfish_f4_w) // display enable?

	/* mirrors of the rom banking */
	AM_RANGE(0xf8, 0xfd)  AM_WRITE(multfish_bank_w)
ADDRESS_MAP_END




static const gfx_layout tiles16x16_layout =
{
	16,16,
	RGN_FRAC(1,4),
	8,
	{ RGN_FRAC(2,4)+0, RGN_FRAC(2,4)+1,RGN_FRAC(2,4)+2, RGN_FRAC(2,4)+3,0,1,2,3 },
	{ 0,4,
	  RGN_FRAC(1,4)+0, RGN_FRAC(1,4)+4,
	  8, 12,
	  RGN_FRAC(1,4)+8, RGN_FRAC(1,4)+12,
	  16, 20,
	  RGN_FRAC(1,4)+16, RGN_FRAC(1,4)+20,
	  24, 28,
	  RGN_FRAC(1,4)+24,RGN_FRAC(1,4)+28 },
	{ 0*32, 1*32, 2*32, 3*32, 4*32, 5*32, 6*32, 7*32,8*32,9*32,10*32,11*32,12*32,13*32,14*32,15*32 },
	8*64
};



static GFXDECODE_START( multfish )
	GFXDECODE_ENTRY( "gfx", 0, tiles16x16_layout, 0, 16 )
GFXDECODE_END

static MACHINE_RESET( multfish )
{
	memory_configure_bank(machine, 1, 0, 16, memory_region(machine, "maincpu"), 0x4000);
	memory_set_bank(machine, 1, 0);
}

static const ay8910_interface ay8910_config =
{
	AY8910_LEGACY_OUTPUT,
	AY8910_DEFAULT_LOADS,
	DEVCB_NULL,
	DEVCB_NULL,
	DEVCB_NULL,
	DEVCB_NULL
};


static MACHINE_DRIVER_START( multfish )
	/* basic machine hardware */
	MDRV_CPU_ADD("maincpu", Z80,6000000) /* 6 MHz? */
	MDRV_CPU_PROGRAM_MAP(multfish_map)
	MDRV_CPU_IO_MAP(multfish_portmap)
	MDRV_CPU_VBLANK_INT("screen",irq0_line_hold)

	MDRV_MACHINE_RESET( multfish )
	/* video hardware */
	MDRV_SCREEN_ADD("screen", RASTER)
	MDRV_SCREEN_REFRESH_RATE(60)
	MDRV_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MDRV_SCREEN_FORMAT(BITMAP_FORMAT_INDEXED16)
	MDRV_SCREEN_SIZE(64*16, 32*16)
	MDRV_SCREEN_VISIBLE_AREA(17*16, 1024-16*7-1, 1*16, 32*16-1*16-1)
	MDRV_GFXDECODE(multfish)
	MDRV_PALETTE_LENGTH(0x1000)

	MDRV_NVRAM_HANDLER(generic_0fill)

	MDRV_VIDEO_START(multfish)
	MDRV_VIDEO_UPDATE(multfish)

	MDRV_SPEAKER_STANDARD_MONO("mono")
	MDRV_SOUND_ADD("ay", AY8910, 6000000/4)
	MDRV_SOUND_CONFIG(ay8910_config)
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.30)
MACHINE_DRIVER_END



/*********************************************************
   Multifish
**********************************************************/

#if ALL_REVISIONS
ROM_START( mfish ) // 021120
	ROM_REGION( 0x40000, "maincpu", 0 )
	ROM_LOAD( "mf021120.rom", 0x00000, 0x40000, MD5(6021e2bfa67abdfc0beb7f291fdc9d9c) SHA1(eb7eb5aae00a77edcf328f460970eb180d86d058) )

	ROM_REGION( 0x400000, "gfx", 0 )
	ROM_LOAD( "1", 0x000000, 0x80000, CRC(2f2a7367) SHA1(ce7ee9ca4f374ec61edc3b89d4752f0edb64a910) )
	ROM_LOAD( "2", 0x100000, 0x80000, CRC(606acd73) SHA1(ce5f7b1366dbb16d57fe4b7f395f08725e3cf756) )
	ROM_LOAD( "3", 0x200000, 0x80000, CRC(33759c2a) SHA1(6afcee2e00a27542fc9751702abcc84cd7d3a2a8) )
	ROM_LOAD( "4", 0x300000, 0x80000, CRC(d0053546) SHA1(01c69be0c594947d57648f491904a3b6938a5570) )
	ROM_LOAD( "5", 0x080000, 0x80000, CRC(6f632872) SHA1(949661cb234855a9c86403e9893c5d9f465ddd79) )
	ROM_LOAD( "6", 0x180000, 0x80000, CRC(023c1193) SHA1(98cf2732f9542b0bb3bee324611f6d3143ef1dc4) )
	ROM_LOAD( "7", 0x280000, 0x80000, CRC(9afdc2d3) SHA1(b112fd2005354c9f97d77030bdb6f99d7b5c8050) )
	ROM_LOAD( "8", 0x380000, 0x80000, CRC(29f1a326) SHA1(5e268411cab888c0727aaf8ae7d0b435d2efd189) )
ROM_END

ROM_START( mfish_2 ) // 021121
	ROM_REGION( 0x40000, "maincpu", 0 )
	ROM_LOAD( "mf021121.rom", 0x00000, 0x40000, MD5(727dc01459f6745caa2b19fbd4432055) SHA1(87a1fb81330cf4b66e17702c22fda694ebff58eb) )

	ROM_REGION( 0x400000, "gfx", 0 )
	ROM_LOAD( "1", 0x000000, 0x80000, CRC(2f2a7367) SHA1(ce7ee9ca4f374ec61edc3b89d4752f0edb64a910) )
	ROM_LOAD( "2", 0x100000, 0x80000, CRC(606acd73) SHA1(ce5f7b1366dbb16d57fe4b7f395f08725e3cf756) )
	ROM_LOAD( "3", 0x200000, 0x80000, CRC(33759c2a) SHA1(6afcee2e00a27542fc9751702abcc84cd7d3a2a8) )
	ROM_LOAD( "4", 0x300000, 0x80000, CRC(d0053546) SHA1(01c69be0c594947d57648f491904a3b6938a5570) )
	ROM_LOAD( "5", 0x080000, 0x80000, CRC(6f632872) SHA1(949661cb234855a9c86403e9893c5d9f465ddd79) )
	ROM_LOAD( "6", 0x180000, 0x80000, CRC(023c1193) SHA1(98cf2732f9542b0bb3bee324611f6d3143ef1dc4) )
	ROM_LOAD( "7", 0x280000, 0x80000, CRC(9afdc2d3) SHA1(b112fd2005354c9f97d77030bdb6f99d7b5c8050) )
	ROM_LOAD( "8", 0x380000, 0x80000, CRC(29f1a326) SHA1(5e268411cab888c0727aaf8ae7d0b435d2efd189) )
ROM_END
#endif

ROM_START( mfish_3 ) // 021124
	ROM_REGION( 0x40000, "maincpu", 0 )
	ROM_LOAD( "mf021124.rom", 0x00000, 0x40000, CRC(59fd16f5) SHA1(ea132f68e9c09c40369d4cc02c670ee6e26bdcbe) )

	ROM_REGION( 0x400000, "gfx", 0 )
	ROM_LOAD( "1", 0x000000, 0x80000, CRC(2f2a7367) SHA1(ce7ee9ca4f374ec61edc3b89d4752f0edb64a910) )
	ROM_LOAD( "2", 0x100000, 0x80000, CRC(606acd73) SHA1(ce5f7b1366dbb16d57fe4b7f395f08725e3cf756) )
	ROM_LOAD( "3", 0x200000, 0x80000, CRC(33759c2a) SHA1(6afcee2e00a27542fc9751702abcc84cd7d3a2a8) )
	ROM_LOAD( "4", 0x300000, 0x80000, CRC(d0053546) SHA1(01c69be0c594947d57648f491904a3b6938a5570) )
	ROM_LOAD( "5", 0x080000, 0x80000, CRC(6f632872) SHA1(949661cb234855a9c86403e9893c5d9f465ddd79) )
	ROM_LOAD( "6", 0x180000, 0x80000, CRC(023c1193) SHA1(98cf2732f9542b0bb3bee324611f6d3143ef1dc4) )
	ROM_LOAD( "7", 0x280000, 0x80000, CRC(9afdc2d3) SHA1(b112fd2005354c9f97d77030bdb6f99d7b5c8050) )
	ROM_LOAD( "8", 0x380000, 0x80000, CRC(29f1a326) SHA1(5e268411cab888c0727aaf8ae7d0b435d2efd189) )
ROM_END

ROM_START( mfish_3a ) // 021124
	ROM_REGION( 0x40000, "maincpu", 0 )
	ROM_LOAD( "mf021124a.rom", 0x00000, 0x40000, CRC(31344b4e) SHA1(33c7f30b55d22c087a02e840456d475177df8bf1) ) /* Not officially listed on Igrosoft's web site hash page */

	ROM_REGION( 0x400000, "gfx", 0 )
	ROM_LOAD( "1", 0x000000, 0x80000, CRC(2f2a7367) SHA1(ce7ee9ca4f374ec61edc3b89d4752f0edb64a910) )
	ROM_LOAD( "2", 0x100000, 0x80000, CRC(606acd73) SHA1(ce5f7b1366dbb16d57fe4b7f395f08725e3cf756) )
	ROM_LOAD( "3", 0x200000, 0x80000, CRC(33759c2a) SHA1(6afcee2e00a27542fc9751702abcc84cd7d3a2a8) )
	ROM_LOAD( "4", 0x300000, 0x80000, CRC(d0053546) SHA1(01c69be0c594947d57648f491904a3b6938a5570) )
	ROM_LOAD( "5", 0x080000, 0x80000, CRC(6f632872) SHA1(949661cb234855a9c86403e9893c5d9f465ddd79) )
	ROM_LOAD( "6", 0x180000, 0x80000, CRC(023c1193) SHA1(98cf2732f9542b0bb3bee324611f6d3143ef1dc4) )
	ROM_LOAD( "7", 0x280000, 0x80000, CRC(9afdc2d3) SHA1(b112fd2005354c9f97d77030bdb6f99d7b5c8050) )
	ROM_LOAD( "8", 0x380000, 0x80000, CRC(29f1a326) SHA1(5e268411cab888c0727aaf8ae7d0b435d2efd189) )
ROM_END

#if ALL_REVISIONS
ROM_START( mfish_4 ) // 021219
	ROM_REGION( 0x40000, "maincpu", 0 )
	ROM_LOAD( "mf021219.rom", 0x00000, 0x40000, MD5(c8810d803d320d9fefa46588c8ef28c0) SHA1(887d456b2ba89560329457d9eaea26fb72223a38) )

	ROM_REGION( 0x400000, "gfx", 0 )
	ROM_LOAD( "1", 0x000000, 0x80000, CRC(2f2a7367) SHA1(ce7ee9ca4f374ec61edc3b89d4752f0edb64a910) )
	ROM_LOAD( "2", 0x100000, 0x80000, CRC(606acd73) SHA1(ce5f7b1366dbb16d57fe4b7f395f08725e3cf756) )
	ROM_LOAD( "3", 0x200000, 0x80000, CRC(33759c2a) SHA1(6afcee2e00a27542fc9751702abcc84cd7d3a2a8) )
	ROM_LOAD( "4", 0x300000, 0x80000, CRC(d0053546) SHA1(01c69be0c594947d57648f491904a3b6938a5570) )
	ROM_LOAD( "5", 0x080000, 0x80000, CRC(6f632872) SHA1(949661cb234855a9c86403e9893c5d9f465ddd79) )
	ROM_LOAD( "6", 0x180000, 0x80000, CRC(023c1193) SHA1(98cf2732f9542b0bb3bee324611f6d3143ef1dc4) )
	ROM_LOAD( "7", 0x280000, 0x80000, CRC(9afdc2d3) SHA1(b112fd2005354c9f97d77030bdb6f99d7b5c8050) )
	ROM_LOAD( "8", 0x380000, 0x80000, CRC(29f1a326) SHA1(5e268411cab888c0727aaf8ae7d0b435d2efd189) )
ROM_END

ROM_START( mfish_5 ) // 021227
	ROM_REGION( 0x40000, "maincpu", 0 )
	ROM_LOAD( "mf021227.rom", 0x00000, 0x40000, MD5(1bbdff5bd2b89a0c9c474286c55d16db) SHA1(58b74c41a88a781da01dba52744dc74e41deae70) )

	ROM_REGION( 0x400000, "gfx", 0 )
	ROM_LOAD( "1", 0x000000, 0x80000, CRC(2f2a7367) SHA1(ce7ee9ca4f374ec61edc3b89d4752f0edb64a910) )
	ROM_LOAD( "2", 0x100000, 0x80000, CRC(606acd73) SHA1(ce5f7b1366dbb16d57fe4b7f395f08725e3cf756) )
	ROM_LOAD( "3", 0x200000, 0x80000, CRC(33759c2a) SHA1(6afcee2e00a27542fc9751702abcc84cd7d3a2a8) )
	ROM_LOAD( "4", 0x300000, 0x80000, CRC(d0053546) SHA1(01c69be0c594947d57648f491904a3b6938a5570) )
	ROM_LOAD( "5", 0x080000, 0x80000, CRC(6f632872) SHA1(949661cb234855a9c86403e9893c5d9f465ddd79) )
	ROM_LOAD( "6", 0x180000, 0x80000, CRC(023c1193) SHA1(98cf2732f9542b0bb3bee324611f6d3143ef1dc4) )
	ROM_LOAD( "7", 0x280000, 0x80000, CRC(9afdc2d3) SHA1(b112fd2005354c9f97d77030bdb6f99d7b5c8050) )
	ROM_LOAD( "8", 0x380000, 0x80000, CRC(29f1a326) SHA1(5e268411cab888c0727aaf8ae7d0b435d2efd189) )
ROM_END

ROM_START( mfish_6 ) // 030124
	ROM_REGION( 0x40000, "maincpu", 0 )
	ROM_LOAD( "mf030124.rom", 0x00000, 0x40000, MD5(4772becb7c2b3220492c690501e174a7) SHA1(b119b086bad3f6f8acc64a5809ce449800615406) )

	ROM_REGION( 0x400000, "gfx", 0 )
	ROM_LOAD( "1", 0x000000, 0x80000, CRC(2f2a7367) SHA1(ce7ee9ca4f374ec61edc3b89d4752f0edb64a910) )
	ROM_LOAD( "2", 0x100000, 0x80000, CRC(606acd73) SHA1(ce5f7b1366dbb16d57fe4b7f395f08725e3cf756) )
	ROM_LOAD( "3", 0x200000, 0x80000, CRC(33759c2a) SHA1(6afcee2e00a27542fc9751702abcc84cd7d3a2a8) )
	ROM_LOAD( "4", 0x300000, 0x80000, CRC(d0053546) SHA1(01c69be0c594947d57648f491904a3b6938a5570) )
	ROM_LOAD( "5", 0x080000, 0x80000, CRC(6f632872) SHA1(949661cb234855a9c86403e9893c5d9f465ddd79) )
	ROM_LOAD( "6", 0x180000, 0x80000, CRC(023c1193) SHA1(98cf2732f9542b0bb3bee324611f6d3143ef1dc4) )
	ROM_LOAD( "7", 0x280000, 0x80000, CRC(9afdc2d3) SHA1(b112fd2005354c9f97d77030bdb6f99d7b5c8050) )
	ROM_LOAD( "8", 0x380000, 0x80000, CRC(29f1a326) SHA1(5e268411cab888c0727aaf8ae7d0b435d2efd189) )
ROM_END

ROM_START( mfish_7 ) // 030511
	ROM_REGION( 0x40000, "maincpu", 0 )
	ROM_LOAD( "mf030511.rom", 0x00000, 0x40000, MD5(a910910ce7963a4385e31769789842f7) SHA1(06b3e3875f036782983e29e305f67a36f78a4f06) )

	ROM_REGION( 0x400000, "gfx", 0 )
	ROM_LOAD( "1", 0x000000, 0x80000, CRC(2f2a7367) SHA1(ce7ee9ca4f374ec61edc3b89d4752f0edb64a910) )
	ROM_LOAD( "2", 0x100000, 0x80000, CRC(606acd73) SHA1(ce5f7b1366dbb16d57fe4b7f395f08725e3cf756) )
	ROM_LOAD( "3", 0x200000, 0x80000, CRC(33759c2a) SHA1(6afcee2e00a27542fc9751702abcc84cd7d3a2a8) )
	ROM_LOAD( "4", 0x300000, 0x80000, CRC(d0053546) SHA1(01c69be0c594947d57648f491904a3b6938a5570) )
	ROM_LOAD( "5", 0x080000, 0x80000, CRC(6f632872) SHA1(949661cb234855a9c86403e9893c5d9f465ddd79) )
	ROM_LOAD( "6", 0x180000, 0x80000, CRC(023c1193) SHA1(98cf2732f9542b0bb3bee324611f6d3143ef1dc4) )
	ROM_LOAD( "7", 0x280000, 0x80000, CRC(9afdc2d3) SHA1(b112fd2005354c9f97d77030bdb6f99d7b5c8050) )
	ROM_LOAD( "8", 0x380000, 0x80000, CRC(29f1a326) SHA1(5e268411cab888c0727aaf8ae7d0b435d2efd189) )
ROM_END

ROM_START( mfish_8 ) // 030522
	ROM_REGION( 0x40000, "maincpu", 0 )
	ROM_LOAD( "mf030522.rom", 0x00000, 0x40000, MD5(17dc6bf0308a4ac53bdff0ade1216235) SHA1(fa80e12275b960374c84518bcaa1e32d0a4ff437) )

	ROM_REGION( 0x400000, "gfx", 0 )
	ROM_LOAD( "1", 0x000000, 0x80000, CRC(2f2a7367) SHA1(ce7ee9ca4f374ec61edc3b89d4752f0edb64a910) )
	ROM_LOAD( "2", 0x100000, 0x80000, CRC(606acd73) SHA1(ce5f7b1366dbb16d57fe4b7f395f08725e3cf756) )
	ROM_LOAD( "3", 0x200000, 0x80000, CRC(33759c2a) SHA1(6afcee2e00a27542fc9751702abcc84cd7d3a2a8) )
	ROM_LOAD( "4", 0x300000, 0x80000, CRC(d0053546) SHA1(01c69be0c594947d57648f491904a3b6938a5570) )
	ROM_LOAD( "5", 0x080000, 0x80000, CRC(6f632872) SHA1(949661cb234855a9c86403e9893c5d9f465ddd79) )
	ROM_LOAD( "6", 0x180000, 0x80000, CRC(023c1193) SHA1(98cf2732f9542b0bb3bee324611f6d3143ef1dc4) )
	ROM_LOAD( "7", 0x280000, 0x80000, CRC(9afdc2d3) SHA1(b112fd2005354c9f97d77030bdb6f99d7b5c8050) )
	ROM_LOAD( "8", 0x380000, 0x80000, CRC(29f1a326) SHA1(5e268411cab888c0727aaf8ae7d0b435d2efd189) )
ROM_END

ROM_START( mfish_9 ) // 031026
	ROM_REGION( 0x40000, "maincpu", 0 )
	ROM_LOAD( "mf031026.rom", 0x00000, 0x40000, MD5(a0a31829705ad78786f7c1bd36cee0cf) SHA1(451b390793f89188afe2b6e82fc02b474fb97a7c) )

	ROM_REGION( 0x400000, "gfx", 0 )
	ROM_LOAD( "1", 0x000000, 0x80000, CRC(2f2a7367) SHA1(ce7ee9ca4f374ec61edc3b89d4752f0edb64a910) )
	ROM_LOAD( "2", 0x100000, 0x80000, CRC(606acd73) SHA1(ce5f7b1366dbb16d57fe4b7f395f08725e3cf756) )
	ROM_LOAD( "3", 0x200000, 0x80000, CRC(33759c2a) SHA1(6afcee2e00a27542fc9751702abcc84cd7d3a2a8) )
	ROM_LOAD( "4", 0x300000, 0x80000, CRC(d0053546) SHA1(01c69be0c594947d57648f491904a3b6938a5570) )
	ROM_LOAD( "5", 0x080000, 0x80000, CRC(6f632872) SHA1(949661cb234855a9c86403e9893c5d9f465ddd79) )
	ROM_LOAD( "6", 0x180000, 0x80000, CRC(023c1193) SHA1(98cf2732f9542b0bb3bee324611f6d3143ef1dc4) )
	ROM_LOAD( "7", 0x280000, 0x80000, CRC(9afdc2d3) SHA1(b112fd2005354c9f97d77030bdb6f99d7b5c8050) )
	ROM_LOAD( "8", 0x380000, 0x80000, CRC(29f1a326) SHA1(5e268411cab888c0727aaf8ae7d0b435d2efd189) )
ROM_END

ROM_START( mfish_10 ) // 031117
	ROM_REGION( 0x40000, "maincpu", 0 )
	ROM_LOAD( "mf031117.rom", 0x00000, 0x40000, MD5(531a3e63c46be33a151c06bdd9479655) SHA1(1d244a332af0fb6aa593a246211ff2b6d2c48a59) )

	ROM_REGION( 0x400000, "gfx", 0 )
	ROM_LOAD( "1", 0x000000, 0x80000, CRC(2f2a7367) SHA1(ce7ee9ca4f374ec61edc3b89d4752f0edb64a910) )
	ROM_LOAD( "2", 0x100000, 0x80000, CRC(606acd73) SHA1(ce5f7b1366dbb16d57fe4b7f395f08725e3cf756) )
	ROM_LOAD( "3", 0x200000, 0x80000, CRC(33759c2a) SHA1(6afcee2e00a27542fc9751702abcc84cd7d3a2a8) )
	ROM_LOAD( "4", 0x300000, 0x80000, CRC(d0053546) SHA1(01c69be0c594947d57648f491904a3b6938a5570) )
	ROM_LOAD( "5", 0x080000, 0x80000, CRC(6f632872) SHA1(949661cb234855a9c86403e9893c5d9f465ddd79) )
	ROM_LOAD( "6", 0x180000, 0x80000, CRC(023c1193) SHA1(98cf2732f9542b0bb3bee324611f6d3143ef1dc4) )
	ROM_LOAD( "7", 0x280000, 0x80000, CRC(9afdc2d3) SHA1(b112fd2005354c9f97d77030bdb6f99d7b5c8050) )
	ROM_LOAD( "8", 0x380000, 0x80000, CRC(29f1a326) SHA1(5e268411cab888c0727aaf8ae7d0b435d2efd189) )
ROM_END

ROM_START( mfish_11 ) // 031124
	ROM_REGION( 0x40000, "maincpu", 0 )
	ROM_LOAD( "mf031124.rom", 0x00000, 0x40000, MD5(9cd800719c6e4a2267e3c140467238d3) SHA1(c0d1b541c4b076bbc810ad637acb4a2663a919ba) )

	ROM_REGION( 0x400000, "gfx", 0 )
	ROM_LOAD( "2", 0x100000, 0x80000, CRC(606acd73) SHA1(ce5f7b1366dbb16d57fe4b7f395f08725e3cf756) )
	ROM_LOAD( "3", 0x200000, 0x80000, CRC(33759c2a) SHA1(6afcee2e00a27542fc9751702abcc84cd7d3a2a8) )
	ROM_LOAD( "4", 0x300000, 0x80000, CRC(d0053546) SHA1(01c69be0c594947d57648f491904a3b6938a5570) )
	ROM_LOAD( "5", 0x080000, 0x80000, CRC(6f632872) SHA1(949661cb234855a9c86403e9893c5d9f465ddd79) )
	ROM_LOAD( "6", 0x180000, 0x80000, CRC(023c1193) SHA1(98cf2732f9542b0bb3bee324611f6d3143ef1dc4) )
	ROM_LOAD( "7", 0x280000, 0x80000, CRC(9afdc2d3) SHA1(b112fd2005354c9f97d77030bdb6f99d7b5c8050) )
	ROM_LOAD( "8", 0x380000, 0x80000, CRC(29f1a326) SHA1(5e268411cab888c0727aaf8ae7d0b435d2efd189) )
ROM_END
#endif

ROM_START( mfish_12 ) // 040308
	ROM_REGION( 0x40000, "maincpu", 0 )
	ROM_LOAD( "mf040308.rom", 0x00000, 0x40000, CRC(adb9c1d9) SHA1(88c69f48766dc7c98a6f03c1a0a4aa63b76560b6) )

	ROM_REGION( 0x400000, "gfx", 0 )
	ROM_LOAD( "2", 0x100000, 0x80000, CRC(606acd73) SHA1(ce5f7b1366dbb16d57fe4b7f395f08725e3cf756) )
	ROM_LOAD( "3", 0x200000, 0x80000, CRC(33759c2a) SHA1(6afcee2e00a27542fc9751702abcc84cd7d3a2a8) )
	ROM_LOAD( "4", 0x300000, 0x80000, CRC(d0053546) SHA1(01c69be0c594947d57648f491904a3b6938a5570) )
	ROM_LOAD( "5", 0x080000, 0x80000, CRC(6f632872) SHA1(949661cb234855a9c86403e9893c5d9f465ddd79) )
	ROM_LOAD( "6", 0x180000, 0x80000, CRC(023c1193) SHA1(98cf2732f9542b0bb3bee324611f6d3143ef1dc4) )
	ROM_LOAD( "7", 0x280000, 0x80000, CRC(9afdc2d3) SHA1(b112fd2005354c9f97d77030bdb6f99d7b5c8050) )
	ROM_LOAD( "8", 0x380000, 0x80000, CRC(29f1a326) SHA1(5e268411cab888c0727aaf8ae7d0b435d2efd189) )
ROM_END

ROM_START( mfish_12a ) // 040308
	ROM_REGION( 0x40000, "maincpu", 0 )
	ROM_LOAD( "mf040308a.rom", 0x00000, 0x40000, CRC(44537648) SHA1(7bce6085778ff0b21c052ae91703de3b78b8eed0) ) /* Not officially listed on Igrosoft's web site hash page */

	ROM_REGION( 0x400000, "gfx", 0 )
	ROM_LOAD( "2", 0x100000, 0x80000, CRC(606acd73) SHA1(ce5f7b1366dbb16d57fe4b7f395f08725e3cf756) )
	ROM_LOAD( "3", 0x200000, 0x80000, CRC(33759c2a) SHA1(6afcee2e00a27542fc9751702abcc84cd7d3a2a8) )
	ROM_LOAD( "4", 0x300000, 0x80000, CRC(d0053546) SHA1(01c69be0c594947d57648f491904a3b6938a5570) )
	ROM_LOAD( "5", 0x080000, 0x80000, CRC(6f632872) SHA1(949661cb234855a9c86403e9893c5d9f465ddd79) )
	ROM_LOAD( "6", 0x180000, 0x80000, CRC(023c1193) SHA1(98cf2732f9542b0bb3bee324611f6d3143ef1dc4) )
	ROM_LOAD( "7", 0x280000, 0x80000, CRC(9afdc2d3) SHA1(b112fd2005354c9f97d77030bdb6f99d7b5c8050) )
	ROM_LOAD( "8", 0x380000, 0x80000, CRC(29f1a326) SHA1(5e268411cab888c0727aaf8ae7d0b435d2efd189) )
ROM_END

#if ALL_REVISIONS
ROM_START( mfish_13 ) // 040316
	ROM_REGION( 0x40000, "maincpu", 0 )
	ROM_LOAD( "mf040316.rom", 0x00000, 0x40000, MD5(66019927201954518261652147b05e43) SHA1(c1f4d1c51632a45b533d19c8b6f63d337d84d9cd) )

	ROM_REGION( 0x400000, "gfx", 0 )
	ROM_LOAD( "2", 0x100000, 0x80000, CRC(606acd73) SHA1(ce5f7b1366dbb16d57fe4b7f395f08725e3cf756) )
	ROM_LOAD( "3", 0x200000, 0x80000, CRC(33759c2a) SHA1(6afcee2e00a27542fc9751702abcc84cd7d3a2a8) )
	ROM_LOAD( "4", 0x300000, 0x80000, CRC(d0053546) SHA1(01c69be0c594947d57648f491904a3b6938a5570) )
	ROM_LOAD( "5", 0x080000, 0x80000, CRC(6f632872) SHA1(949661cb234855a9c86403e9893c5d9f465ddd79) )
	ROM_LOAD( "6", 0x180000, 0x80000, CRC(023c1193) SHA1(98cf2732f9542b0bb3bee324611f6d3143ef1dc4) )
	ROM_LOAD( "7", 0x280000, 0x80000, CRC(9afdc2d3) SHA1(b112fd2005354c9f97d77030bdb6f99d7b5c8050) )
	ROM_LOAD( "8", 0x380000, 0x80000, CRC(29f1a326) SHA1(5e268411cab888c0727aaf8ae7d0b435d2efd189) )
ROM_END
#endif

/*********************************************************
   Crazy Monkey

    Roms 1-4 were changed after the 070315 update.
        The official list of hashes shows the 070315 updated roms.

**********************************************************/

#if ALL_REVISIONS
ROM_START( crzmon ) // 030217
	ROM_REGION( 0x40000, "maincpu", 0 ) // z80 code, banked
	ROM_LOAD( "cm030217.rom", 0x00000, 0x40000, MD5(5e2e4eec4cb20437a3e389003f8e2bb7) SHA1(75787f32aa4c8e8ff7bc11c57a37ad5a65f71c52) )

	ROM_REGION( 0x400000, "gfx", 0 )
	ROM_LOAD( "1a", 0x000000, 0x80000, CRC(683f2be3) SHA1(6fdba4ec07752bf049787a11638895352e9d5f10) )
	ROM_LOAD( "2a", 0x100000, 0x80000, CRC(e21ce6a4) SHA1(942ffe323ddbcaaad887cb5bc9f356550926083b) )
	ROM_LOAD( "3a", 0x200000, 0x80000, CRC(c3d0e3d5) SHA1(5b0cb436c6b0bac1213c1df56702fa7f16856106) )
	ROM_LOAD( "4a", 0x300000, 0x80000, CRC(f79df52c) SHA1(b99fa9f61849b62668bf9edff1c80212a9108b15) )
	ROM_LOAD( "5",  0x080000, 0x80000, CRC(9d4d2a94) SHA1(c714e110de628b343dfc7fff23befaa1276056a9) )
	ROM_LOAD( "6",  0x180000, 0x80000, CRC(a15f0fee) SHA1(3f06d5a1a41e1335bcc7586a5ea95b9b734155c0) )
	ROM_LOAD( "7",  0x280000, 0x80000, CRC(715a2528) SHA1(6c4c72592568ecbaa9518fb7271d2714dd22dbbb) )
	ROM_LOAD( "8",  0x380000, 0x80000, CRC(6fdb6fd5) SHA1(f40916112365de258956ec033aff79aae1f58690) )
ROM_END

ROM_START( crzmon_2 ) // 030225
	ROM_REGION( 0x40000, "maincpu", 0 ) // z80 code, banked
	ROM_LOAD( "cm030225.rom", 0x00000, 0x40000, MD5(8f8d0ca97b5a2ad255d36094a6b30e0e) SHA1(3627a3d6a4a50ed8544456d53ab5a489af389a19) )

	ROM_REGION( 0x400000, "gfx", 0 )
	ROM_LOAD( "1a", 0x000000, 0x80000, CRC(683f2be3) SHA1(6fdba4ec07752bf049787a11638895352e9d5f10) )
	ROM_LOAD( "2a", 0x100000, 0x80000, CRC(e21ce6a4) SHA1(942ffe323ddbcaaad887cb5bc9f356550926083b) )
	ROM_LOAD( "3a", 0x200000, 0x80000, CRC(c3d0e3d5) SHA1(5b0cb436c6b0bac1213c1df56702fa7f16856106) )
	ROM_LOAD( "4a", 0x300000, 0x80000, CRC(f79df52c) SHA1(b99fa9f61849b62668bf9edff1c80212a9108b15) )
	ROM_LOAD( "5",  0x080000, 0x80000, CRC(9d4d2a94) SHA1(c714e110de628b343dfc7fff23befaa1276056a9) )
	ROM_LOAD( "6",  0x180000, 0x80000, CRC(a15f0fee) SHA1(3f06d5a1a41e1335bcc7586a5ea95b9b734155c0) )
	ROM_LOAD( "7",  0x280000, 0x80000, CRC(715a2528) SHA1(6c4c72592568ecbaa9518fb7271d2714dd22dbbb) )
	ROM_LOAD( "8",  0x380000, 0x80000, CRC(6fdb6fd5) SHA1(f40916112365de258956ec033aff79aae1f58690) )
ROM_END

ROM_START( crzmon_3 ) // 030227
	ROM_REGION( 0x40000, "maincpu", 0 ) // z80 code, banked
	ROM_LOAD( "cm030227.rom", 0x00000, 0x40000, MD5(e2704337640db28a92b9946209eec2b1) SHA1(4f8cd68dd2b6abeaabc9b45da18469cc6e7ac74d) )

	ROM_REGION( 0x400000, "gfx", 0 )
	ROM_LOAD( "1a", 0x000000, 0x80000, CRC(683f2be3) SHA1(6fdba4ec07752bf049787a11638895352e9d5f10) )
	ROM_LOAD( "2a", 0x100000, 0x80000, CRC(e21ce6a4) SHA1(942ffe323ddbcaaad887cb5bc9f356550926083b) )
	ROM_LOAD( "3a", 0x200000, 0x80000, CRC(c3d0e3d5) SHA1(5b0cb436c6b0bac1213c1df56702fa7f16856106) )
	ROM_LOAD( "4a", 0x300000, 0x80000, CRC(f79df52c) SHA1(b99fa9f61849b62668bf9edff1c80212a9108b15) )
	ROM_LOAD( "5",  0x080000, 0x80000, CRC(9d4d2a94) SHA1(c714e110de628b343dfc7fff23befaa1276056a9) )
	ROM_LOAD( "6",  0x180000, 0x80000, CRC(a15f0fee) SHA1(3f06d5a1a41e1335bcc7586a5ea95b9b734155c0) )
	ROM_LOAD( "7",  0x280000, 0x80000, CRC(715a2528) SHA1(6c4c72592568ecbaa9518fb7271d2714dd22dbbb) )
	ROM_LOAD( "8",  0x380000, 0x80000, CRC(6fdb6fd5) SHA1(f40916112365de258956ec033aff79aae1f58690) )
ROM_END

ROM_START( crzmon_4 ) // 030404
	ROM_REGION( 0x40000, "maincpu", 0 ) // z80 code, banked
	ROM_LOAD( "cm030404.rom", 0x00000, 0x40000, MD5(a3a9fce896f75cf9f8046c68a70a098d) SHA1(fd99caa2b6ef7218563db4f3b755e34dd551e05f) )

	ROM_REGION( 0x400000, "gfx", 0 )
	ROM_LOAD( "1a", 0x000000, 0x80000, CRC(683f2be3) SHA1(6fdba4ec07752bf049787a11638895352e9d5f10) )
	ROM_LOAD( "2a", 0x100000, 0x80000, CRC(e21ce6a4) SHA1(942ffe323ddbcaaad887cb5bc9f356550926083b) )
	ROM_LOAD( "3a", 0x200000, 0x80000, CRC(c3d0e3d5) SHA1(5b0cb436c6b0bac1213c1df56702fa7f16856106) )
	ROM_LOAD( "4a", 0x300000, 0x80000, CRC(f79df52c) SHA1(b99fa9f61849b62668bf9edff1c80212a9108b15) )
	ROM_LOAD( "5",  0x080000, 0x80000, CRC(9d4d2a94) SHA1(c714e110de628b343dfc7fff23befaa1276056a9) )
	ROM_LOAD( "6",  0x180000, 0x80000, CRC(a15f0fee) SHA1(3f06d5a1a41e1335bcc7586a5ea95b9b734155c0) )
	ROM_LOAD( "7",  0x280000, 0x80000, CRC(715a2528) SHA1(6c4c72592568ecbaa9518fb7271d2714dd22dbbb) )
	ROM_LOAD( "8",  0x380000, 0x80000, CRC(6fdb6fd5) SHA1(f40916112365de258956ec033aff79aae1f58690) )
ROM_END

ROM_START( crzmon_5 ) // 030421
	ROM_REGION( 0x40000, "maincpu", 0 ) // z80 code, banked
	ROM_LOAD( "cm030421.rom", 0x00000, 0x40000, MD5(2b30aabb2c1becc3209018580ebe0086) SHA1(6559e45e3ec39c1d201ed54a10fdb5c6aeff6582) )

	ROM_REGION( 0x400000, "gfx", 0 )
	ROM_LOAD( "1a", 0x000000, 0x80000, CRC(683f2be3) SHA1(6fdba4ec07752bf049787a11638895352e9d5f10) )
	ROM_LOAD( "2a", 0x100000, 0x80000, CRC(e21ce6a4) SHA1(942ffe323ddbcaaad887cb5bc9f356550926083b) )
	ROM_LOAD( "3a", 0x200000, 0x80000, CRC(c3d0e3d5) SHA1(5b0cb436c6b0bac1213c1df56702fa7f16856106) )
	ROM_LOAD( "4a", 0x300000, 0x80000, CRC(f79df52c) SHA1(b99fa9f61849b62668bf9edff1c80212a9108b15) )
	ROM_LOAD( "5",  0x080000, 0x80000, CRC(9d4d2a94) SHA1(c714e110de628b343dfc7fff23befaa1276056a9) )
	ROM_LOAD( "6",  0x180000, 0x80000, CRC(a15f0fee) SHA1(3f06d5a1a41e1335bcc7586a5ea95b9b734155c0) )
	ROM_LOAD( "7",  0x280000, 0x80000, CRC(715a2528) SHA1(6c4c72592568ecbaa9518fb7271d2714dd22dbbb) )
	ROM_LOAD( "8",  0x380000, 0x80000, CRC(6fdb6fd5) SHA1(f40916112365de258956ec033aff79aae1f58690) )
ROM_END

ROM_START( crzmon_6 ) // 031016
	ROM_REGION( 0x40000, "maincpu", 0 ) // z80 code, banked
	ROM_LOAD( "cm031016.rom", 0x00000, 0x40000, MD5(36901e462cd3a9d221a9cefcdf377cbe) SHA1(2f2a5ecbb311ade75f8fdc322c6e63836d4119c3) )

	ROM_REGION( 0x400000, "gfx", 0 )
	ROM_LOAD( "1a", 0x000000, 0x80000, CRC(683f2be3) SHA1(6fdba4ec07752bf049787a11638895352e9d5f10) )
	ROM_LOAD( "2a", 0x100000, 0x80000, CRC(e21ce6a4) SHA1(942ffe323ddbcaaad887cb5bc9f356550926083b) )
	ROM_LOAD( "3a", 0x200000, 0x80000, CRC(c3d0e3d5) SHA1(5b0cb436c6b0bac1213c1df56702fa7f16856106) )
	ROM_LOAD( "4a", 0x300000, 0x80000, CRC(f79df52c) SHA1(b99fa9f61849b62668bf9edff1c80212a9108b15) )
	ROM_LOAD( "5",  0x080000, 0x80000, CRC(9d4d2a94) SHA1(c714e110de628b343dfc7fff23befaa1276056a9) )
	ROM_LOAD( "6",  0x180000, 0x80000, CRC(a15f0fee) SHA1(3f06d5a1a41e1335bcc7586a5ea95b9b734155c0) )
	ROM_LOAD( "7",  0x280000, 0x80000, CRC(715a2528) SHA1(6c4c72592568ecbaa9518fb7271d2714dd22dbbb) )
	ROM_LOAD( "8",  0x380000, 0x80000, CRC(6fdb6fd5) SHA1(f40916112365de258956ec033aff79aae1f58690) )
ROM_END
#endif

ROM_START( crzmon_7 ) // 031110
	ROM_REGION( 0x40000, "maincpu", 0 ) // z80 code, banked
	ROM_LOAD( "cm031110.rom", 0x00000, 0x40000, CRC(d3e67980) SHA1(f0daa91abdde211a2ff61414d84386b763c30949) )

	ROM_REGION( 0x400000, "gfx", 0 )
	ROM_LOAD( "1a", 0x000000, 0x80000, CRC(683f2be3) SHA1(6fdba4ec07752bf049787a11638895352e9d5f10) )
	ROM_LOAD( "2a", 0x100000, 0x80000, CRC(e21ce6a4) SHA1(942ffe323ddbcaaad887cb5bc9f356550926083b) )
	ROM_LOAD( "3a", 0x200000, 0x80000, CRC(c3d0e3d5) SHA1(5b0cb436c6b0bac1213c1df56702fa7f16856106) )
	ROM_LOAD( "4a", 0x300000, 0x80000, CRC(f79df52c) SHA1(b99fa9f61849b62668bf9edff1c80212a9108b15) )
	ROM_LOAD( "5",  0x080000, 0x80000, CRC(9d4d2a94) SHA1(c714e110de628b343dfc7fff23befaa1276056a9) )
	ROM_LOAD( "6",  0x180000, 0x80000, CRC(a15f0fee) SHA1(3f06d5a1a41e1335bcc7586a5ea95b9b734155c0) )
	ROM_LOAD( "7",  0x280000, 0x80000, CRC(715a2528) SHA1(6c4c72592568ecbaa9518fb7271d2714dd22dbbb) )
	ROM_LOAD( "8",  0x380000, 0x80000, CRC(6fdb6fd5) SHA1(f40916112365de258956ec033aff79aae1f58690) )
ROM_END

ROM_START( crzmon_7a ) // 031110
	ROM_REGION( 0x40000, "maincpu", 0 ) // z80 code, banked
	ROM_LOAD( "cm031110a.rom", 0x00000, 0x40000, CRC(80666246) SHA1(e15a210b11ba769ca4fd637c962932417555dc0e) ) /* Not officially listed on Igrosoft's web site hash page */

	ROM_REGION( 0x400000, "gfx", 0 )
	ROM_LOAD( "1a", 0x000000, 0x80000, CRC(683f2be3) SHA1(6fdba4ec07752bf049787a11638895352e9d5f10) )
	ROM_LOAD( "2a", 0x100000, 0x80000, CRC(e21ce6a4) SHA1(942ffe323ddbcaaad887cb5bc9f356550926083b) )
	ROM_LOAD( "3a", 0x200000, 0x80000, CRC(c3d0e3d5) SHA1(5b0cb436c6b0bac1213c1df56702fa7f16856106) )
	ROM_LOAD( "4a", 0x300000, 0x80000, CRC(f79df52c) SHA1(b99fa9f61849b62668bf9edff1c80212a9108b15) )
	ROM_LOAD( "5",  0x080000, 0x80000, CRC(9d4d2a94) SHA1(c714e110de628b343dfc7fff23befaa1276056a9) )
	ROM_LOAD( "6",  0x180000, 0x80000, CRC(a15f0fee) SHA1(3f06d5a1a41e1335bcc7586a5ea95b9b734155c0) )
	ROM_LOAD( "7",  0x280000, 0x80000, CRC(715a2528) SHA1(6c4c72592568ecbaa9518fb7271d2714dd22dbbb) )
	ROM_LOAD( "8",  0x380000, 0x80000, CRC(6fdb6fd5) SHA1(f40916112365de258956ec033aff79aae1f58690) )
ROM_END

ROM_START( crzmon_7b ) // 031110
	ROM_REGION( 0x40000, "maincpu", 0 ) // z80 code, banked
	ROM_LOAD( "cm031110b.rom", 0x00000, 0x40000, CRC(bb6f4f85) SHA1(a2f44632f857392eb422412b55a19decae4c8620) ) /* Not officially listed on Igrosoft's web site hash page */

	ROM_REGION( 0x400000, "gfx", 0 )
	ROM_LOAD( "1a", 0x000000, 0x80000, CRC(683f2be3) SHA1(6fdba4ec07752bf049787a11638895352e9d5f10) )
	ROM_LOAD( "2a", 0x100000, 0x80000, CRC(e21ce6a4) SHA1(942ffe323ddbcaaad887cb5bc9f356550926083b) )
	ROM_LOAD( "3a", 0x200000, 0x80000, CRC(c3d0e3d5) SHA1(5b0cb436c6b0bac1213c1df56702fa7f16856106) )
	ROM_LOAD( "4a", 0x300000, 0x80000, CRC(f79df52c) SHA1(b99fa9f61849b62668bf9edff1c80212a9108b15) )
	ROM_LOAD( "5",  0x080000, 0x80000, CRC(9d4d2a94) SHA1(c714e110de628b343dfc7fff23befaa1276056a9) )
	ROM_LOAD( "6",  0x180000, 0x80000, CRC(a15f0fee) SHA1(3f06d5a1a41e1335bcc7586a5ea95b9b734155c0) )
	ROM_LOAD( "7",  0x280000, 0x80000, CRC(715a2528) SHA1(6c4c72592568ecbaa9518fb7271d2714dd22dbbb) )
	ROM_LOAD( "8",  0x380000, 0x80000, CRC(6fdb6fd5) SHA1(f40916112365de258956ec033aff79aae1f58690) )
ROM_END

ROM_START( crzmon_8 ) // 050120
	ROM_REGION( 0x40000, "maincpu", 0 ) // z80 code, banked
	ROM_LOAD( "cm050120.rom", 0x00000, 0x40000, CRC(9af1e03f) SHA1(caadf48a36da48f4e126b286f6f5498005d8182a) )

	ROM_REGION( 0x400000, "gfx", 0 )
	ROM_LOAD( "1a", 0x000000, 0x80000, CRC(683f2be3) SHA1(6fdba4ec07752bf049787a11638895352e9d5f10) )
	ROM_LOAD( "2a", 0x100000, 0x80000, CRC(e21ce6a4) SHA1(942ffe323ddbcaaad887cb5bc9f356550926083b) )
	ROM_LOAD( "3a", 0x200000, 0x80000, CRC(c3d0e3d5) SHA1(5b0cb436c6b0bac1213c1df56702fa7f16856106) )
	ROM_LOAD( "4a", 0x300000, 0x80000, CRC(f79df52c) SHA1(b99fa9f61849b62668bf9edff1c80212a9108b15) )
	ROM_LOAD( "5",  0x080000, 0x80000, CRC(9d4d2a94) SHA1(c714e110de628b343dfc7fff23befaa1276056a9) )
	ROM_LOAD( "6",  0x180000, 0x80000, CRC(a15f0fee) SHA1(3f06d5a1a41e1335bcc7586a5ea95b9b734155c0) )
	ROM_LOAD( "7",  0x280000, 0x80000, CRC(715a2528) SHA1(6c4c72592568ecbaa9518fb7271d2714dd22dbbb) )
	ROM_LOAD( "8",  0x380000, 0x80000, CRC(6fdb6fd5) SHA1(f40916112365de258956ec033aff79aae1f58690) )
ROM_END

ROM_START( crzmon_8a ) // 050120
	ROM_REGION( 0x40000, "maincpu", 0 ) // z80 code, banked
	ROM_LOAD( "cm050120a.rom", 0x00000, 0x40000, CRC(e20a6997) SHA1(50e0f0f354dd6db2be64d42e36b4043915c4276b) ) /* Not officially listed on Igrosoft's web site hash page */

	ROM_REGION( 0x400000, "gfx", 0 )
	ROM_LOAD( "1a", 0x000000, 0x80000, CRC(683f2be3) SHA1(6fdba4ec07752bf049787a11638895352e9d5f10) )
	ROM_LOAD( "2a", 0x100000, 0x80000, CRC(e21ce6a4) SHA1(942ffe323ddbcaaad887cb5bc9f356550926083b) )
	ROM_LOAD( "3a", 0x200000, 0x80000, CRC(c3d0e3d5) SHA1(5b0cb436c6b0bac1213c1df56702fa7f16856106) )
	ROM_LOAD( "4a", 0x300000, 0x80000, CRC(f79df52c) SHA1(b99fa9f61849b62668bf9edff1c80212a9108b15) )
	ROM_LOAD( "5",  0x080000, 0x80000, CRC(9d4d2a94) SHA1(c714e110de628b343dfc7fff23befaa1276056a9) )
	ROM_LOAD( "6",  0x180000, 0x80000, CRC(a15f0fee) SHA1(3f06d5a1a41e1335bcc7586a5ea95b9b734155c0) )
	ROM_LOAD( "7",  0x280000, 0x80000, CRC(715a2528) SHA1(6c4c72592568ecbaa9518fb7271d2714dd22dbbb) )
	ROM_LOAD( "8",  0x380000, 0x80000, CRC(6fdb6fd5) SHA1(f40916112365de258956ec033aff79aae1f58690) )
ROM_END

/*********************************************************
   Fruit Cocktail

    All the roms were changed after the 070305 update.
        The official list of hashes shows the 070305 updated roms.

**********************************************************/

#if ALL_REVISIONS
ROM_START( fcockt ) // 030505
	ROM_REGION( 0x40000, "maincpu", 0 ) // z80 code, banked
	ROM_LOAD( "fc030505.rom", 0x00000, 0x40000, MD5(5ee5ad269498787e5eb69194874b6544) SHA1(cc65334e8dfae5ffef1d73bd5085e3555905e259) )

	ROM_REGION( 0x400000, "gfx", 0 )
	ROM_LOAD( "1b", 0x000000, 0x80000, CRC(da72c0bb) SHA1(41c9eebccce82520dfe41d6a3a574b2890945ffa) )
	ROM_LOAD( "2b", 0x100000, 0x80000, CRC(6239ba9d) SHA1(22486ad30c28341784e7e490255247b82782b72e) )
	ROM_LOAD( "3b", 0x200000, 0x80000, CRC(2c14a464) SHA1(5fce2f4ef95c5054b055db94399946257bc7321f) )
	ROM_LOAD( "4b", 0x300000, 0x80000, CRC(115898f4) SHA1(55b93bddaeede1c2f6b18083a6a2e6329af087cc) )
	ROM_LOAD( "5b", 0x080000, 0x80000, CRC(f0176b60) SHA1(f764aea00ed306a28cacc62f8d2db9cc42895db8) )
	ROM_LOAD( "6b", 0x180000, 0x80000, CRC(ef24f255) SHA1(6ff924627c179868a25f180f79cd57182b72d9d4) )
	ROM_LOAD( "7",  0x280000, 0x80000, CRC(20f87a15) SHA1(cb60866a3543668f3592c270b445dee881d78128) ) // the same as newer sets?
	ROM_LOAD( "8b", 0x380000, 0x80000, CRC(d282e42e) SHA1(eac9c3eaef39b1805f863ade5da47d6274d20a55) )
ROM_END

ROM_START( fcockt_2 ) // 030512
	ROM_REGION( 0x40000, "maincpu", 0 ) // z80 code, banked
	ROM_LOAD( "fc030512.rom", 0x00000, 0x40000, MD5(e94917c5935641601f17a3fe56dedb17) SHA1(c23ebcf64609a56a029f05101185f3adf73cdadd) )

	ROM_REGION( 0x400000, "gfx", 0 )
	ROM_LOAD( "1b", 0x000000, 0x80000, CRC(da72c0bb) SHA1(41c9eebccce82520dfe41d6a3a574b2890945ffa) )
	ROM_LOAD( "2b", 0x100000, 0x80000, CRC(6239ba9d) SHA1(22486ad30c28341784e7e490255247b82782b72e) )
	ROM_LOAD( "3b", 0x200000, 0x80000, CRC(2c14a464) SHA1(5fce2f4ef95c5054b055db94399946257bc7321f) )
	ROM_LOAD( "4b", 0x300000, 0x80000, CRC(115898f4) SHA1(55b93bddaeede1c2f6b18083a6a2e6329af087cc) )
	ROM_LOAD( "5b", 0x080000, 0x80000, CRC(f0176b60) SHA1(f764aea00ed306a28cacc62f8d2db9cc42895db8) )
	ROM_LOAD( "6b", 0x180000, 0x80000, CRC(ef24f255) SHA1(6ff924627c179868a25f180f79cd57182b72d9d4) )
	ROM_LOAD( "7",  0x280000, 0x80000, CRC(20f87a15) SHA1(cb60866a3543668f3592c270b445dee881d78128) ) // the same as newer sets?
	ROM_LOAD( "8b", 0x380000, 0x80000, CRC(d282e42e) SHA1(eac9c3eaef39b1805f863ade5da47d6274d20a55) )
ROM_END

ROM_START( fcockt_3 ) // 030623
	ROM_REGION( 0x40000, "maincpu", 0 ) // z80 code, banked
	ROM_LOAD( "fc030623.rom", 0x00000, 0x40000, MD5(f787356afecbe02dcfcd89060cc30daf) SHA1(b95c5e06cf41762802199e1b55a5eda2243c9af7) )

	ROM_REGION( 0x400000, "gfx", 0 )
	ROM_LOAD( "1b", 0x000000, 0x80000, CRC(da72c0bb) SHA1(41c9eebccce82520dfe41d6a3a574b2890945ffa) )
	ROM_LOAD( "2b", 0x100000, 0x80000, CRC(6239ba9d) SHA1(22486ad30c28341784e7e490255247b82782b72e) )
	ROM_LOAD( "3b", 0x200000, 0x80000, CRC(2c14a464) SHA1(5fce2f4ef95c5054b055db94399946257bc7321f) )
	ROM_LOAD( "4b", 0x300000, 0x80000, CRC(115898f4) SHA1(55b93bddaeede1c2f6b18083a6a2e6329af087cc) )
	ROM_LOAD( "5b", 0x080000, 0x80000, CRC(f0176b60) SHA1(f764aea00ed306a28cacc62f8d2db9cc42895db8) )
	ROM_LOAD( "6b", 0x180000, 0x80000, CRC(ef24f255) SHA1(6ff924627c179868a25f180f79cd57182b72d9d4) )
	ROM_LOAD( "7",  0x280000, 0x80000, CRC(20f87a15) SHA1(cb60866a3543668f3592c270b445dee881d78128) ) // the same as newer sets?
	ROM_LOAD( "8b", 0x380000, 0x80000, CRC(d282e42e) SHA1(eac9c3eaef39b1805f863ade5da47d6274d20a55) )
ROM_END

ROM_START( fcockt_4 ) // 031028
	ROM_REGION( 0x40000, "maincpu", 0 ) // z80 code, banked
	ROM_LOAD( "fc031028.rom", 0x00000, 0x40000, MD5(ffab543f86538d4717da7cf20d97e3d1) SHA1(18a0ac6e3c6f1d6ae7aeae5322e6b6617923cfdf) )

	ROM_REGION( 0x400000, "gfx", 0 )
	ROM_LOAD( "1b", 0x000000, 0x80000, CRC(da72c0bb) SHA1(41c9eebccce82520dfe41d6a3a574b2890945ffa) )
	ROM_LOAD( "2b", 0x100000, 0x80000, CRC(6239ba9d) SHA1(22486ad30c28341784e7e490255247b82782b72e) )
	ROM_LOAD( "3b", 0x200000, 0x80000, CRC(2c14a464) SHA1(5fce2f4ef95c5054b055db94399946257bc7321f) )
	ROM_LOAD( "4b", 0x300000, 0x80000, CRC(115898f4) SHA1(55b93bddaeede1c2f6b18083a6a2e6329af087cc) )
	ROM_LOAD( "5b", 0x080000, 0x80000, CRC(f0176b60) SHA1(f764aea00ed306a28cacc62f8d2db9cc42895db8) )
	ROM_LOAD( "6b", 0x180000, 0x80000, CRC(ef24f255) SHA1(6ff924627c179868a25f180f79cd57182b72d9d4) )
	ROM_LOAD( "7",  0x280000, 0x80000, CRC(20f87a15) SHA1(cb60866a3543668f3592c270b445dee881d78128) ) // the same as newer sets?
	ROM_LOAD( "8b", 0x380000, 0x80000, CRC(d282e42e) SHA1(eac9c3eaef39b1805f863ade5da47d6274d20a55) )
ROM_END

ROM_START( fcockt_5 ) // 031111
	ROM_REGION( 0x40000, "maincpu", 0 ) // z80 code, banked
	ROM_LOAD( "fc031111.rom", 0x00000, 0x40000, MD5(6df6a06bf0c4df16b6f4d76493d31a39) SHA1(7cc9aeb88a2923f6c5c176abcd6c6b241b353eab) )

	ROM_REGION( 0x400000, "gfx", 0 )
	ROM_LOAD( "1b", 0x000000, 0x80000, CRC(da72c0bb) SHA1(41c9eebccce82520dfe41d6a3a574b2890945ffa) )
	ROM_LOAD( "2b", 0x100000, 0x80000, CRC(6239ba9d) SHA1(22486ad30c28341784e7e490255247b82782b72e) )
	ROM_LOAD( "3b", 0x200000, 0x80000, CRC(2c14a464) SHA1(5fce2f4ef95c5054b055db94399946257bc7321f) )
	ROM_LOAD( "4b", 0x300000, 0x80000, CRC(115898f4) SHA1(55b93bddaeede1c2f6b18083a6a2e6329af087cc) )
	ROM_LOAD( "5b", 0x080000, 0x80000, CRC(f0176b60) SHA1(f764aea00ed306a28cacc62f8d2db9cc42895db8) )
	ROM_LOAD( "6b", 0x180000, 0x80000, CRC(ef24f255) SHA1(6ff924627c179868a25f180f79cd57182b72d9d4) )
	ROM_LOAD( "7",  0x280000, 0x80000, CRC(20f87a15) SHA1(cb60866a3543668f3592c270b445dee881d78128) ) // the same as newer sets?
	ROM_LOAD( "8b", 0x380000, 0x80000, CRC(d282e42e) SHA1(eac9c3eaef39b1805f863ade5da47d6274d20a55) )
ROM_END
#endif

ROM_START( fcockt_6 ) // 040216
	ROM_REGION( 0x40000, "maincpu", 0 ) // z80 code, banked
	ROM_LOAD( "fc040216.rom", 0x00000, 0x40000, CRC(d12b0201) SHA1(09f4b0b5239609ebf13e643782d1881920a1203d) )

	ROM_REGION( 0x400000, "gfx", 0 )
	ROM_LOAD( "1b", 0x000000, 0x80000, CRC(da72c0bb) SHA1(41c9eebccce82520dfe41d6a3a574b2890945ffa) )
	ROM_LOAD( "2b", 0x100000, 0x80000, CRC(6239ba9d) SHA1(22486ad30c28341784e7e490255247b82782b72e) )
	ROM_LOAD( "3b", 0x200000, 0x80000, CRC(2c14a464) SHA1(5fce2f4ef95c5054b055db94399946257bc7321f) )
	ROM_LOAD( "4b", 0x300000, 0x80000, CRC(115898f4) SHA1(55b93bddaeede1c2f6b18083a6a2e6329af087cc) )
	ROM_LOAD( "5b", 0x080000, 0x80000, CRC(f0176b60) SHA1(f764aea00ed306a28cacc62f8d2db9cc42895db8) )
	ROM_LOAD( "6b", 0x180000, 0x80000, CRC(ef24f255) SHA1(6ff924627c179868a25f180f79cd57182b72d9d4) )
	ROM_LOAD( "7",  0x280000, 0x80000, CRC(20f87a15) SHA1(cb60866a3543668f3592c270b445dee881d78128) ) // the same as newer sets?
	ROM_LOAD( "8b", 0x380000, 0x80000, CRC(d282e42e) SHA1(eac9c3eaef39b1805f863ade5da47d6274d20a55) )
ROM_END

ROM_START( fcockt_6a ) // 040216
	ROM_REGION( 0x40000, "maincpu", 0 ) // z80 code, banked
	ROM_LOAD( "fc040216a.rom", 0x00000, 0x40000, CRC(58e7a0c6) SHA1(8022f92af05e9ff6999ff936bad6048d6c264086) ) /* Not officially listed on Igrosoft's web site hash page */

	ROM_REGION( 0x400000, "gfx", 0 )
	ROM_LOAD( "1b", 0x000000, 0x80000, CRC(da72c0bb) SHA1(41c9eebccce82520dfe41d6a3a574b2890945ffa) )
	ROM_LOAD( "2b", 0x100000, 0x80000, CRC(6239ba9d) SHA1(22486ad30c28341784e7e490255247b82782b72e) )
	ROM_LOAD( "3b", 0x200000, 0x80000, CRC(2c14a464) SHA1(5fce2f4ef95c5054b055db94399946257bc7321f) )
	ROM_LOAD( "4b", 0x300000, 0x80000, CRC(115898f4) SHA1(55b93bddaeede1c2f6b18083a6a2e6329af087cc) )
	ROM_LOAD( "5b", 0x080000, 0x80000, CRC(f0176b60) SHA1(f764aea00ed306a28cacc62f8d2db9cc42895db8) )
	ROM_LOAD( "6b", 0x180000, 0x80000, CRC(ef24f255) SHA1(6ff924627c179868a25f180f79cd57182b72d9d4) )
	ROM_LOAD( "7",  0x280000, 0x80000, CRC(20f87a15) SHA1(cb60866a3543668f3592c270b445dee881d78128) ) // the same as newer sets?
	ROM_LOAD( "8b", 0x380000, 0x80000, CRC(d282e42e) SHA1(eac9c3eaef39b1805f863ade5da47d6274d20a55) )
ROM_END

ROM_START( fcockt_6b ) // 040216
	ROM_REGION( 0x40000, "maincpu", 0 ) // z80 code, banked
	ROM_LOAD( "fc040216b.rom", 0x00000, 0x40000, CRC(0f6bcf03) SHA1(6c8765f836f1d899aec3be9c842d5064fd70a435) ) /* Not officially listed on Igrosoft's web site hash page */

	ROM_REGION( 0x400000, "gfx", 0 )
	ROM_LOAD( "1b", 0x000000, 0x80000, CRC(da72c0bb) SHA1(41c9eebccce82520dfe41d6a3a574b2890945ffa) )
	ROM_LOAD( "2b", 0x100000, 0x80000, CRC(6239ba9d) SHA1(22486ad30c28341784e7e490255247b82782b72e) )
	ROM_LOAD( "3b", 0x200000, 0x80000, CRC(2c14a464) SHA1(5fce2f4ef95c5054b055db94399946257bc7321f) )
	ROM_LOAD( "4b", 0x300000, 0x80000, CRC(115898f4) SHA1(55b93bddaeede1c2f6b18083a6a2e6329af087cc) )
	ROM_LOAD( "5b", 0x080000, 0x80000, CRC(f0176b60) SHA1(f764aea00ed306a28cacc62f8d2db9cc42895db8) )
	ROM_LOAD( "6b", 0x180000, 0x80000, CRC(ef24f255) SHA1(6ff924627c179868a25f180f79cd57182b72d9d4) )
	ROM_LOAD( "7",  0x280000, 0x80000, CRC(20f87a15) SHA1(cb60866a3543668f3592c270b445dee881d78128) ) // the same as newer sets?
	ROM_LOAD( "8b", 0x380000, 0x80000, CRC(d282e42e) SHA1(eac9c3eaef39b1805f863ade5da47d6274d20a55) )
ROM_END

ROM_START( fcockt_7 ) // 050118
	ROM_REGION( 0x40000, "maincpu", 0 ) // z80 code, banked
	ROM_LOAD( "fc050118.rom", 0x00000, 0x40000, CRC(356b140a) SHA1(d6e671b5c7fa6592f80b90b289cce0afe1a9cea3) )

	ROM_REGION( 0x400000, "gfx", 0 )
	ROM_LOAD( "1b", 0x000000, 0x80000, CRC(da72c0bb) SHA1(41c9eebccce82520dfe41d6a3a574b2890945ffa) )
	ROM_LOAD( "2b", 0x100000, 0x80000, CRC(6239ba9d) SHA1(22486ad30c28341784e7e490255247b82782b72e) )
	ROM_LOAD( "3b", 0x200000, 0x80000, CRC(2c14a464) SHA1(5fce2f4ef95c5054b055db94399946257bc7321f) )
	ROM_LOAD( "4b", 0x300000, 0x80000, CRC(115898f4) SHA1(55b93bddaeede1c2f6b18083a6a2e6329af087cc) )
	ROM_LOAD( "5b", 0x080000, 0x80000, CRC(f0176b60) SHA1(f764aea00ed306a28cacc62f8d2db9cc42895db8) )
	ROM_LOAD( "6b", 0x180000, 0x80000, CRC(ef24f255) SHA1(6ff924627c179868a25f180f79cd57182b72d9d4) )
	ROM_LOAD( "7",  0x280000, 0x80000, CRC(20f87a15) SHA1(cb60866a3543668f3592c270b445dee881d78128) ) // the same as newer sets?
	ROM_LOAD( "8b", 0x380000, 0x80000, CRC(d282e42e) SHA1(eac9c3eaef39b1805f863ade5da47d6274d20a55) )
ROM_END

ROM_START( fcockt_7a ) // 050118
	ROM_REGION( 0x40000, "maincpu", 0 ) // z80 code, banked
	ROM_LOAD( "fc050118a.rom", 0x00000, 0x40000, CRC(eb2bd908) SHA1(b8e9ef469767fb9e95ff181876ffeaee4b7b9361) ) /* Not officially listed on Igrosoft's web site hash page */

	ROM_REGION( 0x400000, "gfx", 0 )
	ROM_LOAD( "1b", 0x000000, 0x80000, CRC(da72c0bb) SHA1(41c9eebccce82520dfe41d6a3a574b2890945ffa) )
	ROM_LOAD( "2b", 0x100000, 0x80000, CRC(6239ba9d) SHA1(22486ad30c28341784e7e490255247b82782b72e) )
	ROM_LOAD( "3b", 0x200000, 0x80000, CRC(2c14a464) SHA1(5fce2f4ef95c5054b055db94399946257bc7321f) )
	ROM_LOAD( "4b", 0x300000, 0x80000, CRC(115898f4) SHA1(55b93bddaeede1c2f6b18083a6a2e6329af087cc) )
	ROM_LOAD( "5b", 0x080000, 0x80000, CRC(f0176b60) SHA1(f764aea00ed306a28cacc62f8d2db9cc42895db8) )
	ROM_LOAD( "6b", 0x180000, 0x80000, CRC(ef24f255) SHA1(6ff924627c179868a25f180f79cd57182b72d9d4) )
	ROM_LOAD( "7",  0x280000, 0x80000, CRC(20f87a15) SHA1(cb60866a3543668f3592c270b445dee881d78128) ) // the same as newer sets?
	ROM_LOAD( "8b", 0x380000, 0x80000, CRC(d282e42e) SHA1(eac9c3eaef39b1805f863ade5da47d6274d20a55) )
ROM_END

ROM_START( fcockt_8 ) // 060111
	ROM_REGION( 0x40000, "maincpu", 0 ) // z80 code, banked
	ROM_LOAD( "fc060111.rom", 0x00000, 0x40000, CRC(a4af79e3) SHA1(28f40573d6c61e1937b8d05da94e197da5236f57) )

	ROM_REGION( 0x400000, "gfx", 0 )
	ROM_LOAD( "1b", 0x000000, 0x80000, CRC(da72c0bb) SHA1(41c9eebccce82520dfe41d6a3a574b2890945ffa) )
	ROM_LOAD( "2b", 0x100000, 0x80000, CRC(6239ba9d) SHA1(22486ad30c28341784e7e490255247b82782b72e) )
	ROM_LOAD( "3b", 0x200000, 0x80000, CRC(2c14a464) SHA1(5fce2f4ef95c5054b055db94399946257bc7321f) )
	ROM_LOAD( "4b", 0x300000, 0x80000, CRC(115898f4) SHA1(55b93bddaeede1c2f6b18083a6a2e6329af087cc) )
	ROM_LOAD( "5b", 0x080000, 0x80000, CRC(f0176b60) SHA1(f764aea00ed306a28cacc62f8d2db9cc42895db8) )
	ROM_LOAD( "6b", 0x180000, 0x80000, CRC(ef24f255) SHA1(6ff924627c179868a25f180f79cd57182b72d9d4) )
	ROM_LOAD( "7",  0x280000, 0x80000, CRC(20f87a15) SHA1(cb60866a3543668f3592c270b445dee881d78128) ) // the same as newer sets?
	ROM_LOAD( "8b", 0x380000, 0x80000, CRC(d282e42e) SHA1(eac9c3eaef39b1805f863ade5da47d6274d20a55) )
ROM_END

/*********************************************************
   Lucky Haunter

    Roms 1-4 were changed after the 070402 update.
        The official list of hashes shows the 070402 updated roms.

**********************************************************/

#if ALL_REVISIONS
ROM_START( lhaunt ) // 030707
	ROM_REGION( 0x40000, "maincpu", 0 ) // z80 code, banked
	ROM_LOAD( "lh030707.rom", 0x00000, 0x40000, MD5(f2e42fb479888c21351e3029025e6700) SHA1(c7b8e1b98cd0aa665d62c1652716993539c9f3ef) )

	ROM_REGION( 0x400000, "gfx", 0 )
	ROM_LOAD( "1a", 0x000000, 0x80000, CRC(5f1000c6) SHA1(05154d786caf7f5fb9ed90c7d3391cec42e871f8) )
	ROM_LOAD( "2a", 0x100000, 0x80000, CRC(b18abebc) SHA1(66c28fd3f338360b9236dcf414548bfb53655951) )
	ROM_LOAD( "3a", 0x200000, 0x80000, CRC(2e67a1d9) SHA1(a3eff78f25e4e4878706d7c9e4ca71f6914006c6) )
	ROM_LOAD( "4a", 0x300000, 0x80000, CRC(90c963f6) SHA1(2a9c689315cdfb67425f0710511dc0e0241741a7) )
	ROM_LOAD( "5",  0x080000, 0x80000, CRC(b50c90a3) SHA1(74749f4ffb5b0630631b511fc3230c6e7b50dc3b) )
	ROM_LOAD( "6",  0x180000, 0x80000, CRC(4eaaab64) SHA1(9fed16f8e0308200fd16c4b1e511e1bf6c22ae66) )
	ROM_LOAD( "7",  0x280000, 0x80000, CRC(64d16ba9) SHA1(3b897183d6e0f1256be7657441f234fc72077682) )
	ROM_LOAD( "8",  0x380000, 0x80000, CRC(1bdf6252) SHA1(7b5ae82a95a744b236e109024d47b526dccf9c14) )
ROM_END

ROM_START( lhaunt_2 ) // 030804
	ROM_REGION( 0x40000, "maincpu", 0 ) // z80 code, banked
	ROM_LOAD( "lh030804.rom", 0x00000, 0x40000, MD5(01352f3dd0e7d301f8edfb170600e2a1) SHA1(b75702a678d716cd0ccb1f2d1e58c1d3e9f7ca98) )

	ROM_REGION( 0x400000, "gfx", 0 )
	ROM_LOAD( "1a", 0x000000, 0x80000, CRC(5f1000c6) SHA1(05154d786caf7f5fb9ed90c7d3391cec42e871f8) )
	ROM_LOAD( "2a", 0x100000, 0x80000, CRC(b18abebc) SHA1(66c28fd3f338360b9236dcf414548bfb53655951) )
	ROM_LOAD( "3a", 0x200000, 0x80000, CRC(2e67a1d9) SHA1(a3eff78f25e4e4878706d7c9e4ca71f6914006c6) )
	ROM_LOAD( "4a", 0x300000, 0x80000, CRC(90c963f6) SHA1(2a9c689315cdfb67425f0710511dc0e0241741a7) )
	ROM_LOAD( "5",  0x080000, 0x80000, CRC(b50c90a3) SHA1(74749f4ffb5b0630631b511fc3230c6e7b50dc3b) )
	ROM_LOAD( "6",  0x180000, 0x80000, CRC(4eaaab64) SHA1(9fed16f8e0308200fd16c4b1e511e1bf6c22ae66) )
	ROM_LOAD( "7",  0x280000, 0x80000, CRC(64d16ba9) SHA1(3b897183d6e0f1256be7657441f234fc72077682) )
	ROM_LOAD( "8",  0x380000, 0x80000, CRC(1bdf6252) SHA1(7b5ae82a95a744b236e109024d47b526dccf9c14) )
ROM_END

ROM_START( lhaunt_3 ) // 031027
	ROM_REGION( 0x40000, "maincpu", 0 ) // z80 code, banked
	ROM_LOAD( "lh031027.rom", 0x00000, 0x40000, MD5(49b1cf6f89d03b36c5f80d0ccfc23aa2) SHA1(caec736dde2878588ab197ba37801cf7a9ed975b) )

	ROM_REGION( 0x400000, "gfx", 0 )
	ROM_LOAD( "1a", 0x000000, 0x80000, CRC(5f1000c6) SHA1(05154d786caf7f5fb9ed90c7d3391cec42e871f8) )
	ROM_LOAD( "2a", 0x100000, 0x80000, CRC(b18abebc) SHA1(66c28fd3f338360b9236dcf414548bfb53655951) )
	ROM_LOAD( "3a", 0x200000, 0x80000, CRC(2e67a1d9) SHA1(a3eff78f25e4e4878706d7c9e4ca71f6914006c6) )
	ROM_LOAD( "4a", 0x300000, 0x80000, CRC(90c963f6) SHA1(2a9c689315cdfb67425f0710511dc0e0241741a7) )
	ROM_LOAD( "5",  0x080000, 0x80000, CRC(b50c90a3) SHA1(74749f4ffb5b0630631b511fc3230c6e7b50dc3b) )
	ROM_LOAD( "6",  0x180000, 0x80000, CRC(4eaaab64) SHA1(9fed16f8e0308200fd16c4b1e511e1bf6c22ae66) )
	ROM_LOAD( "7",  0x280000, 0x80000, CRC(64d16ba9) SHA1(3b897183d6e0f1256be7657441f234fc72077682) )
	ROM_LOAD( "8",  0x380000, 0x80000, CRC(1bdf6252) SHA1(7b5ae82a95a744b236e109024d47b526dccf9c14) )
ROM_END
#endif

ROM_START( lhaunt_4 ) // 031111
	ROM_REGION( 0x40000, "maincpu", 0 ) // z80 code, banked
	ROM_LOAD( "lh031111.rom", 0x00000, 0x40000, CRC(fc357b75) SHA1(512e4f57612851284bb93ba97c276cbc7cb758d9) )

	ROM_REGION( 0x400000, "gfx", 0 )
	ROM_LOAD( "1a", 0x000000, 0x80000, CRC(5f1000c6) SHA1(05154d786caf7f5fb9ed90c7d3391cec42e871f8) )
	ROM_LOAD( "2a", 0x100000, 0x80000, CRC(b18abebc) SHA1(66c28fd3f338360b9236dcf414548bfb53655951) )
	ROM_LOAD( "3a", 0x200000, 0x80000, CRC(2e67a1d9) SHA1(a3eff78f25e4e4878706d7c9e4ca71f6914006c6) )
	ROM_LOAD( "4a", 0x300000, 0x80000, CRC(90c963f6) SHA1(2a9c689315cdfb67425f0710511dc0e0241741a7) )
	ROM_LOAD( "5",  0x080000, 0x80000, CRC(b50c90a3) SHA1(74749f4ffb5b0630631b511fc3230c6e7b50dc3b) )
	ROM_LOAD( "6",  0x180000, 0x80000, CRC(4eaaab64) SHA1(9fed16f8e0308200fd16c4b1e511e1bf6c22ae66) )
	ROM_LOAD( "7",  0x280000, 0x80000, CRC(64d16ba9) SHA1(3b897183d6e0f1256be7657441f234fc72077682) )
	ROM_LOAD( "8",  0x380000, 0x80000, CRC(1bdf6252) SHA1(7b5ae82a95a744b236e109024d47b526dccf9c14) )
ROM_END

ROM_START( lhaunt_4a ) // 031111
	ROM_REGION( 0x40000, "maincpu", 0 ) // z80 code, banked
	ROM_LOAD( "lh031111a.rom", 0x00000, 0x40000, CRC(83d487c9) SHA1(5b88745d06acba542e2d0660298c9058f2bdfa3f) ) /* Not officially listed on Igrosoft's web site hash page */

	ROM_REGION( 0x400000, "gfx", 0 )
	ROM_LOAD( "1a", 0x000000, 0x80000, CRC(5f1000c6) SHA1(05154d786caf7f5fb9ed90c7d3391cec42e871f8) )
	ROM_LOAD( "2a", 0x100000, 0x80000, CRC(b18abebc) SHA1(66c28fd3f338360b9236dcf414548bfb53655951) )
	ROM_LOAD( "3a", 0x200000, 0x80000, CRC(2e67a1d9) SHA1(a3eff78f25e4e4878706d7c9e4ca71f6914006c6) )
	ROM_LOAD( "4a", 0x300000, 0x80000, CRC(90c963f6) SHA1(2a9c689315cdfb67425f0710511dc0e0241741a7) )
	ROM_LOAD( "5",  0x080000, 0x80000, CRC(b50c90a3) SHA1(74749f4ffb5b0630631b511fc3230c6e7b50dc3b) )
	ROM_LOAD( "6",  0x180000, 0x80000, CRC(4eaaab64) SHA1(9fed16f8e0308200fd16c4b1e511e1bf6c22ae66) )
	ROM_LOAD( "7",  0x280000, 0x80000, CRC(64d16ba9) SHA1(3b897183d6e0f1256be7657441f234fc72077682) )
	ROM_LOAD( "8",  0x380000, 0x80000, CRC(1bdf6252) SHA1(7b5ae82a95a744b236e109024d47b526dccf9c14) )
ROM_END

ROM_START( lhaunt_5 ) // 040216
	ROM_REGION( 0x40000, "maincpu", 0 ) // z80 code, banked
	ROM_LOAD( "lh040216.rom", 0x00000, 0x40000, CRC(558d8345) SHA1(30a87902b291413b1e6eaad6bf4964c54e391e23) )

	ROM_REGION( 0x400000, "gfx", 0 )
	ROM_LOAD( "1a", 0x000000, 0x80000, CRC(5f1000c6) SHA1(05154d786caf7f5fb9ed90c7d3391cec42e871f8) )
	ROM_LOAD( "2a", 0x100000, 0x80000, CRC(b18abebc) SHA1(66c28fd3f338360b9236dcf414548bfb53655951) )
	ROM_LOAD( "3a", 0x200000, 0x80000, CRC(2e67a1d9) SHA1(a3eff78f25e4e4878706d7c9e4ca71f6914006c6) )
	ROM_LOAD( "4a", 0x300000, 0x80000, CRC(90c963f6) SHA1(2a9c689315cdfb67425f0710511dc0e0241741a7) )
	ROM_LOAD( "5",  0x080000, 0x80000, CRC(b50c90a3) SHA1(74749f4ffb5b0630631b511fc3230c6e7b50dc3b) )
	ROM_LOAD( "6",  0x180000, 0x80000, CRC(4eaaab64) SHA1(9fed16f8e0308200fd16c4b1e511e1bf6c22ae66) )
	ROM_LOAD( "7",  0x280000, 0x80000, CRC(64d16ba9) SHA1(3b897183d6e0f1256be7657441f234fc72077682) )
	ROM_LOAD( "8",  0x380000, 0x80000, CRC(1bdf6252) SHA1(7b5ae82a95a744b236e109024d47b526dccf9c14) )
ROM_END

ROM_START( lhaunt_5a ) // 040216
	ROM_REGION( 0x40000, "maincpu", 0 ) // z80 code, banked
	ROM_LOAD( "lh040216a.rom", 0x00000, 0x40000, CRC(2a6c7ff9) SHA1(4a0137c7df5003e8fd843d5489d416d15f001f46) ) /* Not officially listed on Igrosoft's web site hash page */

	ROM_REGION( 0x400000, "gfx", 0 )
	ROM_LOAD( "1a", 0x000000, 0x80000, CRC(5f1000c6) SHA1(05154d786caf7f5fb9ed90c7d3391cec42e871f8) )
	ROM_LOAD( "2a", 0x100000, 0x80000, CRC(b18abebc) SHA1(66c28fd3f338360b9236dcf414548bfb53655951) )
	ROM_LOAD( "3a", 0x200000, 0x80000, CRC(2e67a1d9) SHA1(a3eff78f25e4e4878706d7c9e4ca71f6914006c6) )
	ROM_LOAD( "4a", 0x300000, 0x80000, CRC(90c963f6) SHA1(2a9c689315cdfb67425f0710511dc0e0241741a7) )
	ROM_LOAD( "5",  0x080000, 0x80000, CRC(b50c90a3) SHA1(74749f4ffb5b0630631b511fc3230c6e7b50dc3b) )
	ROM_LOAD( "6",  0x180000, 0x80000, CRC(4eaaab64) SHA1(9fed16f8e0308200fd16c4b1e511e1bf6c22ae66) )
	ROM_LOAD( "7",  0x280000, 0x80000, CRC(64d16ba9) SHA1(3b897183d6e0f1256be7657441f234fc72077682) )
	ROM_LOAD( "8",  0x380000, 0x80000, CRC(1bdf6252) SHA1(7b5ae82a95a744b236e109024d47b526dccf9c14) )
ROM_END

ROM_START( lhaunt_6 ) // 040825
	ROM_REGION( 0x40000, "maincpu", 0 ) // z80 code, banked
	ROM_LOAD( "lh040825.rom", 0x00000, 0x40000, CRC(f9924fa1) SHA1(57a1730fef4963d30f3991f27021647a8c681952) )

	ROM_REGION( 0x400000, "gfx", 0 )
	ROM_LOAD( "1a", 0x000000, 0x80000, CRC(5f1000c6) SHA1(05154d786caf7f5fb9ed90c7d3391cec42e871f8) )
	ROM_LOAD( "2a", 0x100000, 0x80000, CRC(b18abebc) SHA1(66c28fd3f338360b9236dcf414548bfb53655951) )
	ROM_LOAD( "3a", 0x200000, 0x80000, CRC(2e67a1d9) SHA1(a3eff78f25e4e4878706d7c9e4ca71f6914006c6) )
	ROM_LOAD( "4a", 0x300000, 0x80000, CRC(90c963f6) SHA1(2a9c689315cdfb67425f0710511dc0e0241741a7) )
	ROM_LOAD( "5",  0x080000, 0x80000, CRC(b50c90a3) SHA1(74749f4ffb5b0630631b511fc3230c6e7b50dc3b) )
	ROM_LOAD( "6",  0x180000, 0x80000, CRC(4eaaab64) SHA1(9fed16f8e0308200fd16c4b1e511e1bf6c22ae66) )
	ROM_LOAD( "7",  0x280000, 0x80000, CRC(64d16ba9) SHA1(3b897183d6e0f1256be7657441f234fc72077682) )
	ROM_LOAD( "8",  0x380000, 0x80000, CRC(1bdf6252) SHA1(7b5ae82a95a744b236e109024d47b526dccf9c14) )
ROM_END

ROM_START( lhaunt_6a ) // 040825
	ROM_REGION( 0x40000, "maincpu", 0 ) // z80 code, banked
	ROM_LOAD( "lh040825a.rom", 0x00000, 0x40000, CRC(18ba5704) SHA1(3c77ed129db0e5181217167b76292f8e4ee78728) ) /* Not officially listed on Igrosoft's web site hash page */

	ROM_REGION( 0x400000, "gfx", 0 )
	ROM_LOAD( "1a", 0x000000, 0x80000, CRC(5f1000c6) SHA1(05154d786caf7f5fb9ed90c7d3391cec42e871f8) )
	ROM_LOAD( "2a", 0x100000, 0x80000, CRC(b18abebc) SHA1(66c28fd3f338360b9236dcf414548bfb53655951) )
	ROM_LOAD( "3a", 0x200000, 0x80000, CRC(2e67a1d9) SHA1(a3eff78f25e4e4878706d7c9e4ca71f6914006c6) )
	ROM_LOAD( "4a", 0x300000, 0x80000, CRC(90c963f6) SHA1(2a9c689315cdfb67425f0710511dc0e0241741a7) )
	ROM_LOAD( "5",  0x080000, 0x80000, CRC(b50c90a3) SHA1(74749f4ffb5b0630631b511fc3230c6e7b50dc3b) )
	ROM_LOAD( "6",  0x180000, 0x80000, CRC(4eaaab64) SHA1(9fed16f8e0308200fd16c4b1e511e1bf6c22ae66) )
	ROM_LOAD( "7",  0x280000, 0x80000, CRC(64d16ba9) SHA1(3b897183d6e0f1256be7657441f234fc72077682) )
	ROM_LOAD( "8",  0x380000, 0x80000, CRC(1bdf6252) SHA1(7b5ae82a95a744b236e109024d47b526dccf9c14) )
ROM_END

/*********************************************************
   Garage

    Roms 1-4 were changed after the 070213 update.
        The official list of hashes shows the 070213 updated roms.

**********************************************************/

#if ALL_REVISIONS
ROM_START( garage ) // 040122.
	ROM_REGION( 0x40000, "maincpu", 0 ) // z80 code, banked
	ROM_LOAD( "gg040122.rom", 0x00000, 0x40000, MD5(b1fca0a1293d6891171b168ade2112c0) SHA1(327e55d1f4bdc0ad0556faa2fbdaa05b9a5f1c16) )

	ROM_REGION( 0x400000, "gfx", 0 )
	ROM_LOAD( "1a", 0x000000, 0x80000, CRC(57acd4bc) SHA1(8796f463787c359cce6ac09c1b6895f871bbb7c9) )
	ROM_LOAD( "2a", 0x100000, 0x80000, CRC(6d591fa3) SHA1(ddbdf87e0e88dc848b963fbfcb6e14d7b3b9efdc) )
	ROM_LOAD( "3a", 0x200000, 0x80000, CRC(6a15eeda) SHA1(ac35a20893b0518a159207401f6b7f58e3de45fa) )
	ROM_LOAD( "4a", 0x300000, 0x80000, CRC(38f2cd3c) SHA1(5a4463ac352e4e340c6aaa61102841541e9f4c48) )
	ROM_LOAD( "5",  0x080000, 0x80000, CRC(5bf85bc5) SHA1(ff9d2b9cbcd2af6f5fda972e387820d830c196a9) )
	ROM_LOAD( "6",  0x180000, 0x80000, CRC(e5082b26) SHA1(6547409d39dd51498ce8e3f82ff813a8ac3c6522) )
	ROM_LOAD( "7",  0x280000, 0x80000, CRC(dfa2ceb1) SHA1(ec7de8a8f6e7785a563df973841cc1f1603f79fc) )
	ROM_LOAD( "8",  0x380000, 0x80000, CRC(90c5416b) SHA1(583bfb517bc2e30d7b7903aa19fc3b4b5188d7d2) )
ROM_END

ROM_START( garage_2 ) // 040123.
	ROM_REGION( 0x40000, "maincpu", 0 ) // z80 code, banked
	ROM_LOAD( "gg040123.rom", 0x00000, 0x40000, MD5(7b66e7c7b9ddf74bd344a626c64fce5e) SHA1(3051c99d22cfe46b532fcc59a0b98eec186f4a76) )

	ROM_REGION( 0x400000, "gfx", 0 )
	ROM_LOAD( "1a", 0x000000, 0x80000, CRC(57acd4bc) SHA1(8796f463787c359cce6ac09c1b6895f871bbb7c9) )
	ROM_LOAD( "2a", 0x100000, 0x80000, CRC(6d591fa3) SHA1(ddbdf87e0e88dc848b963fbfcb6e14d7b3b9efdc) )
	ROM_LOAD( "3a", 0x200000, 0x80000, CRC(6a15eeda) SHA1(ac35a20893b0518a159207401f6b7f58e3de45fa) )
	ROM_LOAD( "4a", 0x300000, 0x80000, CRC(38f2cd3c) SHA1(5a4463ac352e4e340c6aaa61102841541e9f4c48) )
	ROM_LOAD( "5",  0x080000, 0x80000, CRC(5bf85bc5) SHA1(ff9d2b9cbcd2af6f5fda972e387820d830c196a9) )
	ROM_LOAD( "6",  0x180000, 0x80000, CRC(e5082b26) SHA1(6547409d39dd51498ce8e3f82ff813a8ac3c6522) )
	ROM_LOAD( "7",  0x280000, 0x80000, CRC(dfa2ceb1) SHA1(ec7de8a8f6e7785a563df973841cc1f1603f79fc) )
	ROM_LOAD( "8",  0x380000, 0x80000, CRC(90c5416b) SHA1(583bfb517bc2e30d7b7903aa19fc3b4b5188d7d2) )
ROM_END

ROM_START( garage_3 ) // 040216.
	ROM_REGION( 0x40000, "maincpu", 0 ) // z80 code, banked
	ROM_LOAD( "gg040216.rom", 0x00000, 0x40000, MD5(9d89036118ece87c98bb9b64021014ff) SHA1(321c4106ce07e195a05eacdef6387d61d5e58bb9) )

	ROM_REGION( 0x400000, "gfx", 0 )
	ROM_LOAD( "1a", 0x000000, 0x80000, CRC(57acd4bc) SHA1(8796f463787c359cce6ac09c1b6895f871bbb7c9) )
	ROM_LOAD( "2a", 0x100000, 0x80000, CRC(6d591fa3) SHA1(ddbdf87e0e88dc848b963fbfcb6e14d7b3b9efdc) )
	ROM_LOAD( "3a", 0x200000, 0x80000, CRC(6a15eeda) SHA1(ac35a20893b0518a159207401f6b7f58e3de45fa) )
	ROM_LOAD( "4a", 0x300000, 0x80000, CRC(38f2cd3c) SHA1(5a4463ac352e4e340c6aaa61102841541e9f4c48) )
	ROM_LOAD( "5",  0x080000, 0x80000, CRC(5bf85bc5) SHA1(ff9d2b9cbcd2af6f5fda972e387820d830c196a9) )
	ROM_LOAD( "6",  0x180000, 0x80000, CRC(e5082b26) SHA1(6547409d39dd51498ce8e3f82ff813a8ac3c6522) )
	ROM_LOAD( "7",  0x280000, 0x80000, CRC(dfa2ceb1) SHA1(ec7de8a8f6e7785a563df973841cc1f1603f79fc) )
	ROM_LOAD( "8",  0x380000, 0x80000, CRC(90c5416b) SHA1(583bfb517bc2e30d7b7903aa19fc3b4b5188d7d2) )
ROM_END
#endif

ROM_START( garage_4 ) // 040219.
	ROM_REGION( 0x40000, "maincpu", 0 ) // z80 code, banked
	ROM_LOAD( "gg040219.rom", 0x00000, 0x40000, CRC(49fe4a55) SHA1(df55df0065b4718d2b0c7ff3da85f5d66c2dd95f) )

	ROM_REGION( 0x400000, "gfx", 0 )
	ROM_LOAD( "1a", 0x000000, 0x80000, CRC(57acd4bc) SHA1(8796f463787c359cce6ac09c1b6895f871bbb7c9) )
	ROM_LOAD( "2a", 0x100000, 0x80000, CRC(6d591fa3) SHA1(ddbdf87e0e88dc848b963fbfcb6e14d7b3b9efdc) )
	ROM_LOAD( "3a", 0x200000, 0x80000, CRC(6a15eeda) SHA1(ac35a20893b0518a159207401f6b7f58e3de45fa) )
	ROM_LOAD( "4a", 0x300000, 0x80000, CRC(38f2cd3c) SHA1(5a4463ac352e4e340c6aaa61102841541e9f4c48) )
	ROM_LOAD( "5",  0x080000, 0x80000, CRC(5bf85bc5) SHA1(ff9d2b9cbcd2af6f5fda972e387820d830c196a9) )
	ROM_LOAD( "6",  0x180000, 0x80000, CRC(e5082b26) SHA1(6547409d39dd51498ce8e3f82ff813a8ac3c6522) )
	ROM_LOAD( "7",  0x280000, 0x80000, CRC(dfa2ceb1) SHA1(ec7de8a8f6e7785a563df973841cc1f1603f79fc) )
	ROM_LOAD( "8",  0x380000, 0x80000, CRC(90c5416b) SHA1(583bfb517bc2e30d7b7903aa19fc3b4b5188d7d2) )
ROM_END

ROM_START( garage_4a ) // 040219.
	ROM_REGION( 0x40000, "maincpu", 0 ) // z80 code, banked
	ROM_LOAD( "gg040219a.rom", 0x00000, 0x40000, CRC(e16b213a) SHA1(af0d78116d985efe5f09eb86eb67df2535765527) ) /* Not officially listed on Igrosoft's web site hash page */

	ROM_REGION( 0x400000, "gfx", 0 )
	ROM_LOAD( "1a", 0x000000, 0x80000, CRC(57acd4bc) SHA1(8796f463787c359cce6ac09c1b6895f871bbb7c9) )
	ROM_LOAD( "2a", 0x100000, 0x80000, CRC(6d591fa3) SHA1(ddbdf87e0e88dc848b963fbfcb6e14d7b3b9efdc) )
	ROM_LOAD( "3a", 0x200000, 0x80000, CRC(6a15eeda) SHA1(ac35a20893b0518a159207401f6b7f58e3de45fa) )
	ROM_LOAD( "4a", 0x300000, 0x80000, CRC(38f2cd3c) SHA1(5a4463ac352e4e340c6aaa61102841541e9f4c48) )
	ROM_LOAD( "5",  0x080000, 0x80000, CRC(5bf85bc5) SHA1(ff9d2b9cbcd2af6f5fda972e387820d830c196a9) )
	ROM_LOAD( "6",  0x180000, 0x80000, CRC(e5082b26) SHA1(6547409d39dd51498ce8e3f82ff813a8ac3c6522) )
	ROM_LOAD( "7",  0x280000, 0x80000, CRC(dfa2ceb1) SHA1(ec7de8a8f6e7785a563df973841cc1f1603f79fc) )
	ROM_LOAD( "8",  0x380000, 0x80000, CRC(90c5416b) SHA1(583bfb517bc2e30d7b7903aa19fc3b4b5188d7d2) )
ROM_END

ROM_START( garage_5 ) // 050311.
	ROM_REGION( 0x40000, "maincpu", 0 ) // z80 code, banked
	ROM_LOAD( "gg050311.rom", 0x00000, 0x40000, CRC(405aee88) SHA1(356a8c309434ae4ad6b6fab97aeaece8aa60a730) )

	ROM_REGION( 0x400000, "gfx", 0 )
	ROM_LOAD( "1a", 0x000000, 0x80000, CRC(57acd4bc) SHA1(8796f463787c359cce6ac09c1b6895f871bbb7c9) )
	ROM_LOAD( "2a", 0x100000, 0x80000, CRC(6d591fa3) SHA1(ddbdf87e0e88dc848b963fbfcb6e14d7b3b9efdc) )
	ROM_LOAD( "3a", 0x200000, 0x80000, CRC(6a15eeda) SHA1(ac35a20893b0518a159207401f6b7f58e3de45fa) )
	ROM_LOAD( "4a", 0x300000, 0x80000, CRC(38f2cd3c) SHA1(5a4463ac352e4e340c6aaa61102841541e9f4c48) )
	ROM_LOAD( "5",  0x080000, 0x80000, CRC(5bf85bc5) SHA1(ff9d2b9cbcd2af6f5fda972e387820d830c196a9) )
	ROM_LOAD( "6",  0x180000, 0x80000, CRC(e5082b26) SHA1(6547409d39dd51498ce8e3f82ff813a8ac3c6522) )
	ROM_LOAD( "7",  0x280000, 0x80000, CRC(dfa2ceb1) SHA1(ec7de8a8f6e7785a563df973841cc1f1603f79fc) )
	ROM_LOAD( "8",  0x380000, 0x80000, CRC(90c5416b) SHA1(583bfb517bc2e30d7b7903aa19fc3b4b5188d7d2) )
ROM_END

ROM_START( garage_5a ) // 050311.
	ROM_REGION( 0x40000, "maincpu", 0 ) // z80 code, banked
	ROM_LOAD( "gg050311a.rom", 0x00000, 0x40000, CRC(874a2c27) SHA1(9bf586314f375c2c6f7d79557cc777ac3559cb64) ) /* Not officially listed on Igrosoft's web site hash page */

	ROM_REGION( 0x400000, "gfx", 0 )
	ROM_LOAD( "1a", 0x000000, 0x80000, CRC(57acd4bc) SHA1(8796f463787c359cce6ac09c1b6895f871bbb7c9) )
	ROM_LOAD( "2a", 0x100000, 0x80000, CRC(6d591fa3) SHA1(ddbdf87e0e88dc848b963fbfcb6e14d7b3b9efdc) )
	ROM_LOAD( "3a", 0x200000, 0x80000, CRC(6a15eeda) SHA1(ac35a20893b0518a159207401f6b7f58e3de45fa) )
	ROM_LOAD( "4a", 0x300000, 0x80000, CRC(38f2cd3c) SHA1(5a4463ac352e4e340c6aaa61102841541e9f4c48) )
	ROM_LOAD( "5",  0x080000, 0x80000, CRC(5bf85bc5) SHA1(ff9d2b9cbcd2af6f5fda972e387820d830c196a9) )
	ROM_LOAD( "6",  0x180000, 0x80000, CRC(e5082b26) SHA1(6547409d39dd51498ce8e3f82ff813a8ac3c6522) )
	ROM_LOAD( "7",  0x280000, 0x80000, CRC(dfa2ceb1) SHA1(ec7de8a8f6e7785a563df973841cc1f1603f79fc) )
	ROM_LOAD( "8",  0x380000, 0x80000, CRC(90c5416b) SHA1(583bfb517bc2e30d7b7903aa19fc3b4b5188d7d2) )
ROM_END

/*********************************************************
   Rock Clibmer

    Roms 1-4 were changed after the 070322 update.
        The official list of hashes shows the 070322 updated roms.

**********************************************************/

#if ALL_REVISIONS
ROM_START( rclimb ) // 040815
	ROM_REGION( 0x40000, "maincpu", 0 ) // z80 code, banked
	ROM_LOAD( "rc040815.rom", 0x00000, 0x40000, MD5(82c3ee54f8112c0d0f8007c7e87bb8a2) SHA1(593e64bfe57ba271c04bdd2a35c9484c4efaaa00))

	ROM_REGION( 0x400000, "gfx", 0 )
	ROM_LOAD( "1a", 0x000000, 0x80000, CRC(934f18c7) SHA1(da3a7cddc68e104d415d947e89c0e7f0d067c056) )
	ROM_LOAD( "2a", 0x100000, 0x80000, CRC(7364bd2b) SHA1(c0edfd3b8de813c95fe5d6072662fa0e39fec89e) )
	ROM_LOAD( "3a", 0x200000, 0x80000, CRC(e7befb17) SHA1(8a214680142cd657784a667ab3f6422165fea224) )
	ROM_LOAD( "4a", 0x300000, 0x80000, CRC(dc6d43a0) SHA1(62fc47136775f3fa9369857ec91fe897a1f1ebd6) )
	ROM_LOAD( "5",  0x080000, 0x80000, CRC(ea127c3d) SHA1(a6391eed69a4723b68d727f59b6baebe51633e66) )
	ROM_LOAD( "6",  0x180000, 0x80000, CRC(277fa273) SHA1(6320e6c5b5e48dc451cc48189054c42d85e8ccc1) )
	ROM_LOAD( "7",  0x280000, 0x80000, CRC(3ca7f69a) SHA1(878cca181d915dc3548d5285a4bbb51aef31a64e) )
	ROM_LOAD( "8",  0x380000, 0x80000, CRC(8cf6b4c2) SHA1(4c36c217b83c82acfdd615f5547bf597af7b8833) )
ROM_END

ROM_START( rclimb_2 ) // 040823
	ROM_REGION( 0x40000, "maincpu", 0 ) // z80 code, banked
	ROM_LOAD( "rc040823.rom", 0x00000, 0x40000, MD5(f3f5edf6f838f07f4848fbcf9e566a38) SHA1(31cf4d7f50102d35556817273893182e30c9a70c) )

	ROM_REGION( 0x400000, "gfx", 0 )
	ROM_LOAD( "1a", 0x000000, 0x80000, CRC(934f18c7) SHA1(da3a7cddc68e104d415d947e89c0e7f0d067c056) )
	ROM_LOAD( "2a", 0x100000, 0x80000, CRC(7364bd2b) SHA1(c0edfd3b8de813c95fe5d6072662fa0e39fec89e) )
	ROM_LOAD( "3a", 0x200000, 0x80000, CRC(e7befb17) SHA1(8a214680142cd657784a667ab3f6422165fea224) )
	ROM_LOAD( "4a", 0x300000, 0x80000, CRC(dc6d43a0) SHA1(62fc47136775f3fa9369857ec91fe897a1f1ebd6) )
	ROM_LOAD( "5",  0x080000, 0x80000, CRC(ea127c3d) SHA1(a6391eed69a4723b68d727f59b6baebe51633e66) )
	ROM_LOAD( "6",  0x180000, 0x80000, CRC(277fa273) SHA1(6320e6c5b5e48dc451cc48189054c42d85e8ccc1) )
	ROM_LOAD( "7",  0x280000, 0x80000, CRC(3ca7f69a) SHA1(878cca181d915dc3548d5285a4bbb51aef31a64e) )
	ROM_LOAD( "8",  0x380000, 0x80000, CRC(8cf6b4c2) SHA1(4c36c217b83c82acfdd615f5547bf597af7b8833) )
ROM_END
#endif

ROM_START( rclimb_3 ) // 040827
	ROM_REGION( 0x40000, "maincpu", 0 ) // z80 code, banked
	ROM_LOAD( "rc040827.rom", 0x00000, 0x40000, CRC(3ba55647) SHA1(56e96be0d9782da4b3d5d911ea67962257626ae0) )

	ROM_REGION( 0x400000, "gfx", 0 )
	ROM_LOAD( "1a", 0x000000, 0x80000, CRC(934f18c7) SHA1(da3a7cddc68e104d415d947e89c0e7f0d067c056) )
	ROM_LOAD( "2a", 0x100000, 0x80000, CRC(7364bd2b) SHA1(c0edfd3b8de813c95fe5d6072662fa0e39fec89e) )
	ROM_LOAD( "3a", 0x200000, 0x80000, CRC(e7befb17) SHA1(8a214680142cd657784a667ab3f6422165fea224) )
	ROM_LOAD( "4a", 0x300000, 0x80000, CRC(dc6d43a0) SHA1(62fc47136775f3fa9369857ec91fe897a1f1ebd6) )
	ROM_LOAD( "5",  0x080000, 0x80000, CRC(ea127c3d) SHA1(a6391eed69a4723b68d727f59b6baebe51633e66) )
	ROM_LOAD( "6",  0x180000, 0x80000, CRC(277fa273) SHA1(6320e6c5b5e48dc451cc48189054c42d85e8ccc1) )
	ROM_LOAD( "7",  0x280000, 0x80000, CRC(3ca7f69a) SHA1(878cca181d915dc3548d5285a4bbb51aef31a64e) )
	ROM_LOAD( "8",  0x380000, 0x80000, CRC(8cf6b4c2) SHA1(4c36c217b83c82acfdd615f5547bf597af7b8833) )
ROM_END

ROM_START( rclimb_3a ) // 040827
	ROM_REGION( 0x40000, "maincpu", 0 ) // z80 code, banked
	ROM_LOAD( "rc040827a.rom", 0x00000, 0x40000, CRC(6420f8b4) SHA1(64e4018dbea245ddc06a65fb2f8cf38e77f60999) ) /* Not officially listed on Igrosoft's web site hash page */

	ROM_REGION( 0x400000, "gfx", 0 )
	ROM_LOAD( "1a", 0x000000, 0x80000, CRC(934f18c7) SHA1(da3a7cddc68e104d415d947e89c0e7f0d067c056) )
	ROM_LOAD( "2a", 0x100000, 0x80000, CRC(7364bd2b) SHA1(c0edfd3b8de813c95fe5d6072662fa0e39fec89e) )
	ROM_LOAD( "3a", 0x200000, 0x80000, CRC(e7befb17) SHA1(8a214680142cd657784a667ab3f6422165fea224) )
	ROM_LOAD( "4a", 0x300000, 0x80000, CRC(dc6d43a0) SHA1(62fc47136775f3fa9369857ec91fe897a1f1ebd6) )
	ROM_LOAD( "5",  0x080000, 0x80000, CRC(ea127c3d) SHA1(a6391eed69a4723b68d727f59b6baebe51633e66) )
	ROM_LOAD( "6",  0x180000, 0x80000, CRC(277fa273) SHA1(6320e6c5b5e48dc451cc48189054c42d85e8ccc1) )
	ROM_LOAD( "7",  0x280000, 0x80000, CRC(3ca7f69a) SHA1(878cca181d915dc3548d5285a4bbb51aef31a64e) )
	ROM_LOAD( "8",  0x380000, 0x80000, CRC(8cf6b4c2) SHA1(4c36c217b83c82acfdd615f5547bf597af7b8833) )
ROM_END

ROM_START( rclimb_3b ) // 040827 - new service menu
	ROM_REGION( 0x40000, "maincpu", 0 ) // z80 code, banked
	ROM_LOAD( "rc040827.rom", 0x00000, 0x40000, CRC(3ba55647) SHA1(56e96be0d9782da4b3d5d911ea67962257626ae0) ) /* Not officially listed on Igrosoft's web site hash page */

	ROM_REGION( 0x400000, "gfx", 0 )
	ROM_LOAD( "rc1_.bin", 0x000000, 0x80000, CRC(4bc00c66) SHA1(e4d89c2b188f253b642ae341a1a4c04af33024c8) ) /* is this set a total hack??? */
	ROM_LOAD( "rc2_.bin", 0x100000, 0x80000, CRC(89237f10) SHA1(a493a03f79656332089f2794872be44e62d7e306) )
	ROM_LOAD( "rc3_.bin", 0x200000, 0x80000, CRC(c4147d05) SHA1(8497fca6e64896cf8f03877c3455ee7bf9965b60) )
	ROM_LOAD( "rc4_.bin", 0x300000, 0x80000, CRC(06176cb1) SHA1(0644861042ca4be2d459b31870369a2e46d80aa4) )
	ROM_LOAD( "rc5_.bin", 0x080000, 0x80000, CRC(fb5f2036) SHA1(5f8885332a2d9249d34a0a50ac464a5637b9bc95) )
	ROM_LOAD( "rc6_.bin", 0x180000, 0x80000, CRC(36a8148f) SHA1(57d84e44c77f2bb1e97ec8a1acd3ef71246e3274) )
	ROM_LOAD( "rc7_.bin", 0x280000, 0x80000, CRC(26c620e3) SHA1(b14f2cfc0e5b740360e6ecca97f6bf941136141f) )
	ROM_LOAD( "rc8_.bin", 0x380000, 0x80000, CRC(8cf6b4c2) SHA1(4c36c217b83c82acfdd615f5547bf597af7b8833) )
ROM_END

/*********************************************************
   Sweet Life
**********************************************************/

ROM_START( sweetl ) // 041220
	ROM_REGION( 0x40000, "maincpu", 0 ) // z80 code, banked
	ROM_LOAD( "sl041220.rom", 0x00000, 0x40000, CRC(851b85c6) SHA1(a5db94d94fe82d06f3fac1c16aed5358fcb92f29) )

	ROM_REGION( 0x400000, "gfx", 0 )
	ROM_LOAD( "1", 0x000000, 0x80000, CRC(a096c786) SHA1(81f6b083cb089e9412a8506889196354c670d945) )
	ROM_LOAD( "2", 0x100000, 0x80000, CRC(c5e1e22c) SHA1(973ad27681a0f3beee7084b1b85fc9deb79d638e) )
	ROM_LOAD( "3", 0x200000, 0x80000, CRC(af335323) SHA1(b8afdce231a8ec0f313cc47e00a27f05461bbbc4) )
	ROM_LOAD( "4", 0x300000, 0x80000, CRC(a35c7503) SHA1(78f7a868660bbaa066e8e9e341db52018aaf3af1) )
	ROM_LOAD( "5", 0x080000, 0x80000, CRC(e2d6b632) SHA1(65d05e55671b8c335cae2dfbf6a6f5bd8cc90e2c) )
	ROM_LOAD( "6", 0x180000, 0x80000, CRC(d34e0905) SHA1(cc4afe64fb9052a31f759be41ff07a727e0a9093) )
	ROM_LOAD( "7", 0x280000, 0x80000, CRC(978b67bb) SHA1(87357d5832588f00272bd76df736c06c599f3853) )
	ROM_LOAD( "8", 0x380000, 0x80000, CRC(75954355) SHA1(e6ef2b70d859b61e8e3d1751de8558b8778e502d) )
ROM_END

ROM_START( sweetla ) // 041220
	ROM_REGION( 0x40000, "maincpu", 0 ) // z80 code, banked
	ROM_LOAD( "sl041220a.rom", 0x00000, 0x40000, CRC(920fd9fe) SHA1(0b5ad099ae4c8e3ba0f99baf8fc9322cae24e9d2) ) /* Not officially listed on Igrosoft's web site hash page */

	ROM_REGION( 0x400000, "gfx", 0 )
	ROM_LOAD( "1", 0x000000, 0x80000, CRC(a096c786) SHA1(81f6b083cb089e9412a8506889196354c670d945) )
	ROM_LOAD( "2", 0x100000, 0x80000, CRC(c5e1e22c) SHA1(973ad27681a0f3beee7084b1b85fc9deb79d638e) )
	ROM_LOAD( "3", 0x200000, 0x80000, CRC(af335323) SHA1(b8afdce231a8ec0f313cc47e00a27f05461bbbc4) )
	ROM_LOAD( "4", 0x300000, 0x80000, CRC(a35c7503) SHA1(78f7a868660bbaa066e8e9e341db52018aaf3af1) )
	ROM_LOAD( "5", 0x080000, 0x80000, CRC(e2d6b632) SHA1(65d05e55671b8c335cae2dfbf6a6f5bd8cc90e2c) )
	ROM_LOAD( "6", 0x180000, 0x80000, CRC(d34e0905) SHA1(cc4afe64fb9052a31f759be41ff07a727e0a9093) )
	ROM_LOAD( "7", 0x280000, 0x80000, CRC(978b67bb) SHA1(87357d5832588f00272bd76df736c06c599f3853) )
	ROM_LOAD( "8", 0x380000, 0x80000, CRC(75954355) SHA1(e6ef2b70d859b61e8e3d1751de8558b8778e502d) )
ROM_END

/*********************************************************
   Resident

    Roms 1-4 were changed after the 070222 update.
        The official list of hashes shows the 070222 updated roms.

**********************************************************/

#if ALL_REVISIONS
ROM_START( resdnt ) // 040415
	ROM_REGION( 0x40000, "maincpu", 0 ) // z80 code, banked
	ROM_LOAD( "rs040415.rom", 0x00000, 0x40000, MD5(a46e993839a7ce5c4a3d90ba0e961e69) SHA1(b9f07bc2765d4f366e548007e51b9f605c884ba1) )

	ROM_REGION( 0x400000, "gfx", 0 )
	ROM_LOAD( "1a", 0x000000, 0x80000, CRC(e0645da6) SHA1(dd72f4830d8011f603aa6d430f34ac2598005281) )
	ROM_LOAD( "2a", 0x100000, 0x80000, CRC(dd8de247) SHA1(498c5b931ce65e289f52d8864b603166f81e3dc4) )
	ROM_LOAD( "3a", 0x200000, 0x80000, CRC(0d346ec2) SHA1(e2456b28825c54c5e16829525627c40611c0083d) )
	ROM_LOAD( "4a", 0x300000, 0x80000, CRC(1f95aad9) SHA1(51d003288d5ff23b3c981fbaa99d29b66dd2c101) )
	ROM_LOAD( "5",  0x080000, 0x80000, CRC(0cfe7d44) SHA1(9f0e4925e815ff9f79188f18e78c0a7b377daa3f) )
	ROM_LOAD( "6",  0x180000, 0x80000, CRC(7437904f) SHA1(630c79cd6a990ce7658a1ffabba5a27efba985a1) )
	ROM_LOAD( "7",  0x280000, 0x80000, CRC(6e94728a) SHA1(ab414879cb957d9bc8d653b5e3bb2bbf91139ec0) )
	ROM_LOAD( "8",  0x380000, 0x80000, CRC(a9f55043) SHA1(4771df3d45bdc0a21b1c479f45e09ac5bab6c94f) )
ROM_END
#endif

ROM_START( resdnt_2 ) // 040513
	ROM_REGION( 0x40000, "maincpu", 0 ) // z80 code, banked
	ROM_LOAD( "rs040513.rom", 0x00000, 0x40000, CRC(95f74cb3) SHA1(2e4862ac0ad86899b8ce12580ebd217dfb74f6a2) )

	ROM_REGION( 0x400000, "gfx", 0 )
	ROM_LOAD( "1a", 0x000000, 0x80000, CRC(e0645da6) SHA1(dd72f4830d8011f603aa6d430f34ac2598005281) )
	ROM_LOAD( "2a", 0x100000, 0x80000, CRC(dd8de247) SHA1(498c5b931ce65e289f52d8864b603166f81e3dc4) )
	ROM_LOAD( "3a", 0x200000, 0x80000, CRC(0d346ec2) SHA1(e2456b28825c54c5e16829525627c40611c0083d) )
	ROM_LOAD( "4a", 0x300000, 0x80000, CRC(1f95aad9) SHA1(51d003288d5ff23b3c981fbaa99d29b66dd2c101) )
	ROM_LOAD( "5",  0x080000, 0x80000, CRC(0cfe7d44) SHA1(9f0e4925e815ff9f79188f18e78c0a7b377daa3f) )
	ROM_LOAD( "6",  0x180000, 0x80000, CRC(7437904f) SHA1(630c79cd6a990ce7658a1ffabba5a27efba985a1) )
	ROM_LOAD( "7",  0x280000, 0x80000, CRC(6e94728a) SHA1(ab414879cb957d9bc8d653b5e3bb2bbf91139ec0) )
	ROM_LOAD( "8",  0x380000, 0x80000, CRC(a9f55043) SHA1(4771df3d45bdc0a21b1c479f45e09ac5bab6c94f) )
ROM_END

ROM_START( resdnt_2a ) // 040513
	ROM_REGION( 0x40000, "maincpu", 0 ) // z80 code, banked
	ROM_LOAD( "rs040513a.rom", 0x00000, 0x40000, CRC(5b6480d7) SHA1(e54ddc822819136687d613ce4f38dd98f3e01bb5) ) /* Not officially listed on Igrosoft's web site hash page */

	ROM_REGION( 0x400000, "gfx", 0 )
	ROM_LOAD( "1a", 0x000000, 0x80000, CRC(e0645da6) SHA1(dd72f4830d8011f603aa6d430f34ac2598005281) )
	ROM_LOAD( "2a", 0x100000, 0x80000, CRC(dd8de247) SHA1(498c5b931ce65e289f52d8864b603166f81e3dc4) )
	ROM_LOAD( "3a", 0x200000, 0x80000, CRC(0d346ec2) SHA1(e2456b28825c54c5e16829525627c40611c0083d) )
	ROM_LOAD( "4a", 0x300000, 0x80000, CRC(1f95aad9) SHA1(51d003288d5ff23b3c981fbaa99d29b66dd2c101) )
	ROM_LOAD( "5",  0x080000, 0x80000, CRC(0cfe7d44) SHA1(9f0e4925e815ff9f79188f18e78c0a7b377daa3f) )
	ROM_LOAD( "6",  0x180000, 0x80000, CRC(7437904f) SHA1(630c79cd6a990ce7658a1ffabba5a27efba985a1) )
	ROM_LOAD( "7",  0x280000, 0x80000, CRC(6e94728a) SHA1(ab414879cb957d9bc8d653b5e3bb2bbf91139ec0) )
	ROM_LOAD( "8",  0x380000, 0x80000, CRC(a9f55043) SHA1(4771df3d45bdc0a21b1c479f45e09ac5bab6c94f) )
ROM_END

/*********************************************************
   Roll Fruit

    Roms 4-8 were changed after the 080327 update.
        The official list of hashes shows both set of roms.

**********************************************************/

#if ALL_REVISIONS
ROM_START( rollfr ) // 030821
	ROM_REGION( 0x40000, "maincpu", 0 ) // z80 code, banked
	ROM_LOAD( "rf5-030821.rom", 0x00000, 0x40000, MD5(ef4e9d1845676fe655d2f415ca7bd953) SHA1(5e9c2235ea4207086db23870993d8e28356c9eb8) )

	ROM_REGION( 0x400000, "gfx", 0 ) // no gfx roms listed in dat..
	ROM_LOAD( "1",  0x000000, 0x80000, CRC(caeb1fc3) SHA1(14b9f99f892849faecb3327e572dc134e1065463) )
	ROM_LOAD( "2",  0x100000, 0x80000, CRC(f017c200) SHA1(a247bbbd1c4ca99978dcc705bd62590815a891f2) )
	ROM_LOAD( "3",  0x200000, 0x80000, CRC(a2d6df11) SHA1(c2553136252aebe3b3ce0b5c33e740d0e27fb7b2) )
	ROM_LOAD( "4",  0x300000, 0x80000, CRC(cd3c928a) SHA1(4c50ce17bd5714149eae91279a0133059397b776) )
	ROM_LOAD( "5a", 0x080000, 0x80000, CRC(24c7362a) SHA1(684b7b370fcad07bf74bddffaf432bd52e5d29e2) )
	ROM_LOAD( "6a", 0x180000, 0x80000, CRC(d6a61904) SHA1(73700e88358ed9bccbb63643b7daaff416737e43) )
	ROM_LOAD( "7a", 0x280000, 0x80000, CRC(81e3480b) SHA1(c0f006cf2a4747359cb79f14976ac3411951af1c) )
	ROM_LOAD( "8a", 0x380000, 0x80000, CRC(ed3558b8) SHA1(8ec808069053f0c07d81c45090b2ba22ef8e9c32) )
ROM_END
#endif

ROM_START( rollfr_2 ) // 040318
	ROM_REGION( 0x40000, "maincpu", 0 ) // z80 code, banked
	ROM_LOAD( "rf5-040318.rom", 0x00000, 0x40000, CRC(d8efd395) SHA1(71edd1541df400fef97abacabb10d882ace4c8b0) )

	ROM_REGION( 0x400000, "gfx", 0 )
	ROM_LOAD( "1",  0x000000, 0x80000, CRC(caeb1fc3) SHA1(14b9f99f892849faecb3327e572dc134e1065463) )
	ROM_LOAD( "2",  0x100000, 0x80000, CRC(f017c200) SHA1(a247bbbd1c4ca99978dcc705bd62590815a891f2) )
	ROM_LOAD( "3",  0x200000, 0x80000, CRC(a2d6df11) SHA1(c2553136252aebe3b3ce0b5c33e740d0e27fb7b2) )
	ROM_LOAD( "4",  0x300000, 0x80000, CRC(cd3c928a) SHA1(4c50ce17bd5714149eae91279a0133059397b776) )
	ROM_LOAD( "5a", 0x080000, 0x80000, CRC(24c7362a) SHA1(684b7b370fcad07bf74bddffaf432bd52e5d29e2) )
	ROM_LOAD( "6a", 0x180000, 0x80000, CRC(d6a61904) SHA1(73700e88358ed9bccbb63643b7daaff416737e43) )
	ROM_LOAD( "7a", 0x280000, 0x80000, CRC(81e3480b) SHA1(c0f006cf2a4747359cb79f14976ac3411951af1c) )
	ROM_LOAD( "8a", 0x380000, 0x80000, CRC(ed3558b8) SHA1(8ec808069053f0c07d81c45090b2ba22ef8e9c32) )
ROM_END

/*********************************************************
   Island
**********************************************************/

ROM_START( island ) // 050713
	ROM_REGION( 0x40000, "maincpu", 0 ) // z80 code, banked
	ROM_LOAD( "is050713.rom", 0x00000, 0x40000, CRC(26c7013e) SHA1(5d604f5b4859e9e82830424a1e21f32a9e49bf34) )

	ROM_REGION( 0x400000, "gfx", 0 )
	ROM_LOAD( "1", 0x000000, 0x80000, CRC(dbe8cdda) SHA1(4747cf0d85afdef22d3ba9fa5e75b39548725745) )
	ROM_LOAD( "2", 0x100000, 0x80000, CRC(64064745) SHA1(91a7bc7204a8f7a7512eeaf4906da20a9f587565) )
	ROM_LOAD( "3", 0x200000, 0x80000, CRC(1d993f68) SHA1(b0459d3941d50668f7533909e3f3da91453d3efd) )
	ROM_LOAD( "4", 0x300000, 0x80000, CRC(a4739404) SHA1(8f7ffcc13dcb35adfa8060ab1930d07195b6110c) )
	ROM_LOAD( "5", 0x080000, 0x80000, CRC(d016eb31) SHA1(a84f18af470f72730b241b9031cd6131c8a03db2) )
	ROM_LOAD( "6", 0x180000, 0x80000, CRC(0faaa968) SHA1(0f05546e6e0559e24c6afdde65b3feeb66b6adff) )
	ROM_LOAD( "7", 0x280000, 0x80000, CRC(d7277a6c) SHA1(d96a0befc965ad22087381982305d68208978a7e) )
	ROM_LOAD( "8", 0x380000, 0x80000, CRC(ac6fba48) SHA1(64dd03d624f16da52bc7fa0702246e91ae39a806) )
ROM_END

ROM_START( islanda ) // 050713
	ROM_REGION( 0x40000, "maincpu", 0 ) // z80 code, banked
	ROM_LOAD( "is050713a.rom", 0x00000, 0x40000, CRC(d3d62cb3) SHA1(2ceb83ac9d59a570435220f06e8317057bb46608) ) /* Not officially listed on Igrosoft's web site hash page */

	ROM_REGION( 0x400000, "gfx", 0 )
	ROM_LOAD( "1", 0x000000, 0x80000, CRC(dbe8cdda) SHA1(4747cf0d85afdef22d3ba9fa5e75b39548725745) )
	ROM_LOAD( "2", 0x100000, 0x80000, CRC(64064745) SHA1(91a7bc7204a8f7a7512eeaf4906da20a9f587565) )
	ROM_LOAD( "3", 0x200000, 0x80000, CRC(1d993f68) SHA1(b0459d3941d50668f7533909e3f3da91453d3efd) )
	ROM_LOAD( "4", 0x300000, 0x80000, CRC(a4739404) SHA1(8f7ffcc13dcb35adfa8060ab1930d07195b6110c) )
	ROM_LOAD( "5", 0x080000, 0x80000, CRC(d016eb31) SHA1(a84f18af470f72730b241b9031cd6131c8a03db2) )
	ROM_LOAD( "6", 0x180000, 0x80000, CRC(0faaa968) SHA1(0f05546e6e0559e24c6afdde65b3feeb66b6adff) )
	ROM_LOAD( "7", 0x280000, 0x80000, CRC(d7277a6c) SHA1(d96a0befc965ad22087381982305d68208978a7e) )
	ROM_LOAD( "8", 0x380000, 0x80000, CRC(ac6fba48) SHA1(64dd03d624f16da52bc7fa0702246e91ae39a806) )
ROM_END

/*********************************************************
   Island 2
**********************************************************/

ROM_START( island2 ) // 060529
	ROM_REGION( 0x40000, "maincpu", 0 ) // z80 code, banked
	ROM_LOAD( "is2_060529.rom", 0x00000, 0x40000, CRC(4ccddabd) SHA1(ae5902734488b7ddfa0f7bbf9b800d25b2b657b5) )

	ROM_REGION( 0x400000, "gfx", 0 )
	ROM_LOAD( "1", 0x000000, 0x80000, CRC(f8dd9fe9) SHA1(0cf67fbca107b255011fded6390507d12cbac514) )
	ROM_LOAD( "2", 0x100000, 0x80000, CRC(4f9607c0) SHA1(cd3e7b4a88f46231a115c9d18a26b5e30fea74e4) )
	ROM_LOAD( "3", 0x200000, 0x80000, CRC(bceccdba) SHA1(5cf3b51ccfe317ca57d770bff0204b0ee83d1173) )
	ROM_LOAD( "4", 0x300000, 0x80000, CRC(15fdecc7) SHA1(6882ea10f117c85544df51d9abd67ef52db91d95) )
	ROM_LOAD( "5", 0x080000, 0x80000, CRC(2d4905aa) SHA1(c4a1e4db61e8af6cf0fb70aabe5e3896ab5227ca) )
	ROM_LOAD( "6", 0x180000, 0x80000, CRC(55e285d9) SHA1(ba58963441c65220700cd8057e6afe3f5f8faa4f) )
	ROM_LOAD( "7", 0x280000, 0x80000, CRC(edd72be6) SHA1(fb1e63f59e8565c23ae43630fa572fbc022c878f) )
	ROM_LOAD( "8", 0x380000, 0x80000, CRC(c336d608) SHA1(55391183c6d95ecea81354efa70641350860d1f5) )
ROM_END

ROM_START( island2a ) // 060529
	ROM_REGION( 0x40000, "maincpu", 0 ) // z80 code, banked
	ROM_LOAD( "is2_060529a.rom", 0x00000, 0x40000, CRC(4341d65c) SHA1(e7120c805d7dbf0fee5d18243ddf2cfa19a0d88c) ) /* Not officially listed on Igrosoft's web site hash page */

	ROM_REGION( 0x400000, "gfx", 0 )
	ROM_LOAD( "1", 0x000000, 0x80000, CRC(f8dd9fe9) SHA1(0cf67fbca107b255011fded6390507d12cbac514) )
	ROM_LOAD( "2", 0x100000, 0x80000, CRC(4f9607c0) SHA1(cd3e7b4a88f46231a115c9d18a26b5e30fea74e4) )
	ROM_LOAD( "3", 0x200000, 0x80000, CRC(bceccdba) SHA1(5cf3b51ccfe317ca57d770bff0204b0ee83d1173) )
	ROM_LOAD( "4", 0x300000, 0x80000, CRC(15fdecc7) SHA1(6882ea10f117c85544df51d9abd67ef52db91d95) )
	ROM_LOAD( "5", 0x080000, 0x80000, CRC(2d4905aa) SHA1(c4a1e4db61e8af6cf0fb70aabe5e3896ab5227ca) )
	ROM_LOAD( "6", 0x180000, 0x80000, CRC(55e285d9) SHA1(ba58963441c65220700cd8057e6afe3f5f8faa4f) )
	ROM_LOAD( "7", 0x280000, 0x80000, CRC(edd72be6) SHA1(fb1e63f59e8565c23ae43630fa572fbc022c878f) )
	ROM_LOAD( "8", 0x380000, 0x80000, CRC(c336d608) SHA1(55391183c6d95ecea81354efa70641350860d1f5) )
ROM_END

/*********************************************************
   Pirate
**********************************************************/

#if ALL_REVISIONS
ROM_START( pirate ) // 051229
	ROM_REGION( 0x40000, "maincpu", 0 ) // z80 code, banked
	ROM_LOAD( "pr051229.rom", 0x00000, 0x40000, MD5(d813af59a7a356800470b109a41b978f) SHA1(d1286cba474ccbbff8358ba2fd6917d43d101674) )

	ROM_REGION( 0x400000, "gfx", 0 )
	ROM_LOAD( "1", 0x000000, 0x80000, CRC(d2199619) SHA1(8c67ef7d0e305ae0783302ba9a1cb56cdcf4bc09) )
	ROM_LOAD( "2", 0x100000, 0x80000, CRC(ce5c6548) SHA1(ef1cd6ae36cc1abcf010762dc89a255cd817d016) )
	ROM_LOAD( "3", 0x200000, 0x80000, CRC(d6a8338d) SHA1(6a0e41309dc909decf8bd49cf13cbeca95f0314a) )
	ROM_LOAD( "4", 0x300000, 0x80000, CRC(590b8cf6) SHA1(b2778f6e1b7bcf7f33ced43f999eff983e5a6af4) )
	ROM_LOAD( "5", 0x080000, 0x80000, CRC(bf9f1267) SHA1(b0947bd7d31301ffbe80cbaf1e96c3476f6f9ca3) )
	ROM_LOAD( "6", 0x180000, 0x80000, CRC(b0cdf7eb) SHA1(cf6bd20fb40cf0d87eeb6f1502fb73d9760c9140) )
	ROM_LOAD( "7", 0x280000, 0x80000, CRC(6c4a9510) SHA1(e10bf8475ff7c73ba90b904b9214b285a5b2669f) )
	ROM_LOAD( "8", 0x380000, 0x80000, CRC(cc2edac2) SHA1(24bacd9e092a83945a8def3a254ec66758d71ff5) )
ROM_END
#endif

ROM_START( pirate_1 ) // 060210
	ROM_REGION( 0x40000, "maincpu", 0 ) // z80 code, banked
	ROM_LOAD( "pr060210.rom", 0x00000, 0x40000, CRC(5684d67d) SHA1(4cbd103bcd071df26830d56760ef477b9a652857) )

	ROM_REGION( 0x400000, "gfx", 0 )
	ROM_LOAD( "1", 0x000000, 0x80000, CRC(d2199619) SHA1(8c67ef7d0e305ae0783302ba9a1cb56cdcf4bc09) )
	ROM_LOAD( "2", 0x100000, 0x80000, CRC(ce5c6548) SHA1(ef1cd6ae36cc1abcf010762dc89a255cd817d016) )
	ROM_LOAD( "3", 0x200000, 0x80000, CRC(d6a8338d) SHA1(6a0e41309dc909decf8bd49cf13cbeca95f0314a) )
	ROM_LOAD( "4", 0x300000, 0x80000, CRC(590b8cf6) SHA1(b2778f6e1b7bcf7f33ced43f999eff983e5a6af4) )
	ROM_LOAD( "5", 0x080000, 0x80000, CRC(bf9f1267) SHA1(b0947bd7d31301ffbe80cbaf1e96c3476f6f9ca3) )
	ROM_LOAD( "6", 0x180000, 0x80000, CRC(b0cdf7eb) SHA1(cf6bd20fb40cf0d87eeb6f1502fb73d9760c9140) )
	ROM_LOAD( "7", 0x280000, 0x80000, CRC(6c4a9510) SHA1(e10bf8475ff7c73ba90b904b9214b285a5b2669f) )
	ROM_LOAD( "8", 0x380000, 0x80000, CRC(cc2edac2) SHA1(24bacd9e092a83945a8def3a254ec66758d71ff5) )
ROM_END

/*********************************************************
   Keks

    Roms 1-4 were changed after the 070119 update.
        The official list of hashes shows the 070119 updated roms.

**********************************************************/

ROM_START( keks ) // 060328
	ROM_REGION( 0x40000, "maincpu", 0 ) // z80 code, banked
	ROM_LOAD( "ks060328.rom", 0x00000, 0x40000, CRC(bcf77f77) SHA1(26b09994907c41be957a0b7442cfb1807b27d7be) )

	ROM_REGION( 0x400000, "gfx", 0 )
	ROM_LOAD( "1", 0x000000, 0x80000, CRC(f4c20f66) SHA1(bed42ef01dfaa9d5d6ebb703e44ce7c11b8a373c) )
	ROM_LOAD( "2", 0x100000, 0x80000, CRC(b7ec3fac) SHA1(c3c62690487a6056415c46888bde8254efca836f) )
	ROM_LOAD( "3", 0x200000, 0x80000, CRC(5b6e8568) SHA1(003297e9cd080d91fe6751286eabd3a2f37ceb76) )
	ROM_LOAD( "4", 0x300000, 0x80000, CRC(9dc32736) SHA1(7b2091ae802431d1c959b859a58e0076d32abef0) )
	ROM_LOAD( "5", 0x080000, 0x80000, CRC(c5b09267) SHA1(7fd0988e63752fdbb31fde60b4726cfd63149622) )
	ROM_LOAD( "6", 0x180000, 0x80000, CRC(583da5fd) SHA1(645228db20cdaacb53bfc68731fd1a66a6a8cf56) )
	ROM_LOAD( "7", 0x280000, 0x80000, CRC(311c166a) SHA1(5f0ad8d755a6141964d818b98b3f156cbda8fb0d) )
	ROM_LOAD( "8", 0x380000, 0x80000, CRC(f69b0831) SHA1(75392349ef02a39cf883206938e2c615445065fc) )
ROM_END

ROM_START( keksa ) // 060328
	ROM_REGION( 0x40000, "maincpu", 0 ) // z80 code, banked
	ROM_LOAD( "ks060328a.rom", 0x00000, 0x40000, CRC(7b387386) SHA1(d6bfc3b0d1f74723902d96dbcb69865cb5274cd0) ) /* Not officially listed on Igrosoft's web site hash page */

	ROM_REGION( 0x400000, "gfx", 0 )
	ROM_LOAD( "1", 0x000000, 0x80000, CRC(f4c20f66) SHA1(bed42ef01dfaa9d5d6ebb703e44ce7c11b8a373c) )
	ROM_LOAD( "2", 0x100000, 0x80000, CRC(b7ec3fac) SHA1(c3c62690487a6056415c46888bde8254efca836f) )
	ROM_LOAD( "3", 0x200000, 0x80000, CRC(5b6e8568) SHA1(003297e9cd080d91fe6751286eabd3a2f37ceb76) )
	ROM_LOAD( "4", 0x300000, 0x80000, CRC(9dc32736) SHA1(7b2091ae802431d1c959b859a58e0076d32abef0) )
	ROM_LOAD( "5", 0x080000, 0x80000, CRC(c5b09267) SHA1(7fd0988e63752fdbb31fde60b4726cfd63149622) )
	ROM_LOAD( "6", 0x180000, 0x80000, CRC(583da5fd) SHA1(645228db20cdaacb53bfc68731fd1a66a6a8cf56) )
	ROM_LOAD( "7", 0x280000, 0x80000, CRC(311c166a) SHA1(5f0ad8d755a6141964d818b98b3f156cbda8fb0d) )
	ROM_LOAD( "8", 0x380000, 0x80000, CRC(f69b0831) SHA1(75392349ef02a39cf883206938e2c615445065fc) )
ROM_END

ROM_START( keksb ) // 060328
	ROM_REGION( 0x40000, "maincpu", 0 ) // z80 code, banked
	ROM_LOAD( "ks060328b.rom", 0x00000, 0x40000, CRC(661c7ee9) SHA1(f07902c4a3ba5fce5bc7fe666d90deb852e40b4c) ) /* Not officially listed on Igrosoft's web site hash page */

	ROM_REGION( 0x400000, "gfx", 0 )
	ROM_LOAD( "1", 0x000000, 0x80000, CRC(f4c20f66) SHA1(bed42ef01dfaa9d5d6ebb703e44ce7c11b8a373c) )
	ROM_LOAD( "2", 0x100000, 0x80000, CRC(b7ec3fac) SHA1(c3c62690487a6056415c46888bde8254efca836f) )
	ROM_LOAD( "3", 0x200000, 0x80000, CRC(5b6e8568) SHA1(003297e9cd080d91fe6751286eabd3a2f37ceb76) )
	ROM_LOAD( "4", 0x300000, 0x80000, CRC(9dc32736) SHA1(7b2091ae802431d1c959b859a58e0076d32abef0) )
	ROM_LOAD( "5", 0x080000, 0x80000, CRC(c5b09267) SHA1(7fd0988e63752fdbb31fde60b4726cfd63149622) )
	ROM_LOAD( "6", 0x180000, 0x80000, CRC(583da5fd) SHA1(645228db20cdaacb53bfc68731fd1a66a6a8cf56) )
	ROM_LOAD( "7", 0x280000, 0x80000, CRC(311c166a) SHA1(5f0ad8d755a6141964d818b98b3f156cbda8fb0d) )
	ROM_LOAD( "8", 0x380000, 0x80000, CRC(f69b0831) SHA1(75392349ef02a39cf883206938e2c615445065fc) )
ROM_END

ROM_START( keks_2 ) // 060403
	ROM_REGION( 0x40000, "maincpu", 0 ) // z80 code, banked
	ROM_LOAD( "ks060403.rom", 0x00000, 0x40000, CRC(7abb9392) SHA1(f7a0ba5bcc7566f706e911486fa9cf3e62b86b8b) )

	ROM_REGION( 0x400000, "gfx", 0 )
	ROM_LOAD( "1", 0x000000, 0x80000, CRC(f4c20f66) SHA1(bed42ef01dfaa9d5d6ebb703e44ce7c11b8a373c) )
	ROM_LOAD( "2", 0x100000, 0x80000, CRC(b7ec3fac) SHA1(c3c62690487a6056415c46888bde8254efca836f) )
	ROM_LOAD( "3", 0x200000, 0x80000, CRC(5b6e8568) SHA1(003297e9cd080d91fe6751286eabd3a2f37ceb76) )
	ROM_LOAD( "4", 0x300000, 0x80000, CRC(9dc32736) SHA1(7b2091ae802431d1c959b859a58e0076d32abef0) )
	ROM_LOAD( "5", 0x080000, 0x80000, CRC(c5b09267) SHA1(7fd0988e63752fdbb31fde60b4726cfd63149622) )
	ROM_LOAD( "6", 0x180000, 0x80000, CRC(583da5fd) SHA1(645228db20cdaacb53bfc68731fd1a66a6a8cf56) )
	ROM_LOAD( "7", 0x280000, 0x80000, CRC(311c166a) SHA1(5f0ad8d755a6141964d818b98b3f156cbda8fb0d) )
	ROM_LOAD( "8", 0x380000, 0x80000, CRC(f69b0831) SHA1(75392349ef02a39cf883206938e2c615445065fc) )
ROM_END

ROM_START( keks_2a ) // 060403
	ROM_REGION( 0x40000, "maincpu", 0 ) // z80 code, banked
	ROM_LOAD( "ks060403a.rom", 0x00000, 0x40000, CRC(bd749f63) SHA1(dc3ba624b186370896d3ecf5968a82a17aa019d0) ) /* Not officially listed on Igrosoft's web site hash page */

	ROM_REGION( 0x400000, "gfx", 0 )
	ROM_LOAD( "1", 0x000000, 0x80000, CRC(f4c20f66) SHA1(bed42ef01dfaa9d5d6ebb703e44ce7c11b8a373c) )
	ROM_LOAD( "2", 0x100000, 0x80000, CRC(b7ec3fac) SHA1(c3c62690487a6056415c46888bde8254efca836f) )
	ROM_LOAD( "3", 0x200000, 0x80000, CRC(5b6e8568) SHA1(003297e9cd080d91fe6751286eabd3a2f37ceb76) )
	ROM_LOAD( "4", 0x300000, 0x80000, CRC(9dc32736) SHA1(7b2091ae802431d1c959b859a58e0076d32abef0) )
	ROM_LOAD( "5", 0x080000, 0x80000, CRC(c5b09267) SHA1(7fd0988e63752fdbb31fde60b4726cfd63149622) )
	ROM_LOAD( "6", 0x180000, 0x80000, CRC(583da5fd) SHA1(645228db20cdaacb53bfc68731fd1a66a6a8cf56) )
	ROM_LOAD( "7", 0x280000, 0x80000, CRC(311c166a) SHA1(5f0ad8d755a6141964d818b98b3f156cbda8fb0d) )
	ROM_LOAD( "8", 0x380000, 0x80000, CRC(f69b0831) SHA1(75392349ef02a39cf883206938e2c615445065fc) )
ROM_END

/*

Note:

   Only the first set of a given revision is listed in Igrosoft's official hashes list.

   It is not known if the alternate sets are hacks or were intended for different regions.
   It looks like the difference are in the payout percentage / odds

Most games had a revision in early 2007 to meet the standards of the "Government gambling control"
   law of The Russian Federation No 244-03 of Dec 29, 2006

From Igrosoft's web site about version types (IE: some version have "M" in them):

   * Two software versions are shown, one of them corresponds to Russian legislation,
     the other one (with the letter m) is for the countries without such restrictions.

*/

GAME( 2002, mfish_3,     0,        multfish, multfish,  0, ROT0, "Igro", "Multi Fish (021124, set 1)",  0 )
GAME( 2002, mfish_3a,    mfish_3,  multfish, multfish,  0, ROT0, "Igro", "Multi Fish (021124, set 2)",  0 )
GAME( 2002, mfish_12,    mfish_3,  multfish, multfish,  0, ROT0, "Igro", "Multi Fish (040308, set 1)",  0 )
GAME( 2002, mfish_12a,   mfish_3,  multfish, multfish,  0, ROT0, "Igro", "Multi Fish (040308, set 2)",  0 )
#if ALL_REVISIONS
GAME( 2002, mfish,       0,        multfish, multfish,  0, ROT0, "Igro", "Multi Fish (021120)",  0 )
GAME( 2002, mfish_2,     mfish,    multfish, multfish,  0, ROT0, "Igro", "Multi Fish (021121)",  0 )
GAME( 2002, mfish_4,     mfish,    multfish, multfish,  0, ROT0, "Igro", "Multi Fish (021219)",  0 )
GAME( 2002, mfish_5,     mfish,    multfish, multfish,  0, ROT0, "Igro", "Multi Fish (021227)",  0 )
GAME( 2002, mfish_6,     mfish,    multfish, multfish,  0, ROT0, "Igro", "Multi Fish (030124)",  0 )
GAME( 2002, mfish_7,     mfish,    multfish, multfish,  0, ROT0, "Igro", "Multi Fish (030511)",  0 )
GAME( 2002, mfish_8,     mfish,    multfish, multfish,  0, ROT0, "Igro", "Multi Fish (030522)",  0 )
GAME( 2002, mfish_9,     mfish,    multfish, multfish,  0, ROT0, "Igro", "Multi Fish (031026)",  0 )
GAME( 2002, mfish_10,    mfish,    multfish, multfish,  0, ROT0, "Igro", "Multi Fish (031117)",  0 )
GAME( 2002, mfish_11,    mfish,    multfish, multfish,  0, ROT0, "Igro", "Multi Fish (031124)",  0 )
GAME( 2002, mfish_13,    mfish,    multfish, multfish,  0, ROT0, "Igro", "Multi Fish (040316)",  0 )
#endif

GAME( 2003, crzmon_7,    0,        multfish, multfish,  0, ROT0, "Igrosoft", "Crazy Monkey (031110, set 1)",  0 )
GAME( 2003, crzmon_7a,   crzmon_7, multfish, multfish,  0, ROT0, "Igrosoft", "Crazy Monkey (031110, set 2)",  0 )
GAME( 2003, crzmon_7b,   crzmon_7, multfish, multfish,  0, ROT0, "Igrosoft", "Crazy Monkey (031110, set 3)",  0 )
GAME( 2003, crzmon_8,    crzmon_7, multfish, multfish,  0, ROT0, "Igrosoft", "Crazy Monkey (050120, set 1)",  0 )
GAME( 2003, crzmon_8a,   crzmon_7, multfish, multfish,  0, ROT0, "Igrosoft", "Crazy Monkey (050120, set 2)",  0 )
#if ALL_REVISIONS
GAME( 2003, crzmon,      0,        multfish, multfish,  0, ROT0, "Igrosoft", "Crazy Monkey (030217)",  0 )
GAME( 2003, crzmon_2,    crzmon,   multfish, multfish,  0, ROT0, "Igrosoft", "Crazy Monkey (030225)",  0 )
GAME( 2003, crzmon_3,    crzmon,   multfish, multfish,  0, ROT0, "Igrosoft", "Crazy Monkey (030227)",  0 )
GAME( 2003, crzmon_4,    crzmon,   multfish, multfish,  0, ROT0, "Igrosoft", "Crazy Monkey (030404)",  0 )
GAME( 2003, crzmon_5,    crzmon,   multfish, multfish,  0, ROT0, "Igrosoft", "Crazy Monkey (030421)",  0 )
GAME( 2003, crzmon_6,    crzmon,   multfish, multfish,  0, ROT0, "Igrosoft", "Crazy Monkey (031016)",  0 )
#endif

GAME( 2003, fcockt_6,    0,        multfish, multfish,  0, ROT0, "Igrosoft", "Fruit Cocktail (040216, set 1)",  0 )
GAME( 2003, fcockt_6a,   fcockt_6, multfish, multfish,  0, ROT0, "Igrosoft", "Fruit Cocktail (040216, set 2)",  0 )
GAME( 2003, fcockt_6b,   fcockt_6, multfish, multfish,  0, ROT0, "Igrosoft", "Fruit Cocktail (040216, set 3)",  0 )
GAME( 2003, fcockt_7,    fcockt_6, multfish, multfish,  0, ROT0, "Igrosoft", "Fruit Cocktail (050118, set 1)",  0 )
GAME( 2003, fcockt_7a,   fcockt_6, multfish, multfish,  0, ROT0, "Igrosoft", "Fruit Cocktail (050118, set 2)",  0 )
GAME( 2003, fcockt_8,    fcockt_6, multfish, multfish,  0, ROT0, "Igrosoft", "Fruit Cocktail (060111)",  0 )
#if ALL_REVISIONS
GAME( 2003, fcockt,      0,        multfish, multfish,  0, ROT0, "Igrosoft", "Fruit Cocktail (030505)",  0 )
GAME( 2003, fcockt_2,    fcockt,   multfish, multfish,  0, ROT0, "Igrosoft", "Fruit Cocktail (030512)",  0 )
GAME( 2003, fcockt_3,    fcockt,   multfish, multfish,  0, ROT0, "Igrosoft", "Fruit Cocktail (030623)",  0 )
GAME( 2003, fcockt_4,    fcockt,   multfish, multfish,  0, ROT0, "Igrosoft", "Fruit Cocktail (031028)",  0 )
GAME( 2003, fcockt_5,    fcockt,   multfish, multfish,  0, ROT0, "Igrosoft", "Fruit Cocktail (031111)",  0 )
#endif

GAME( 2003, lhaunt_4,    0,        multfish, multfish,  0, ROT0, "Igrosoft", "Lucky Haunter (031111, set 1)",  0 )
GAME( 2003, lhaunt_4a,   lhaunt_4, multfish, multfish,  0, ROT0, "Igrosoft", "Lucky Haunter (031111, set 2)",  0 )
GAME( 2003, lhaunt_5,    lhaunt_4, multfish, multfish,  0, ROT0, "Igrosoft", "Lucky Haunter (040216. set 1)",  0 )
GAME( 2003, lhaunt_5a,   lhaunt_4, multfish, multfish,  0, ROT0, "Igrosoft", "Lucky Haunter (040216, set 2)",  0 )
GAME( 2003, lhaunt_6,    lhaunt_4, multfish, multfish,  0, ROT0, "Igrosoft", "Lucky Haunter (040825, set 1)",  0 )
GAME( 2003, lhaunt_6a,   lhaunt_4, multfish, multfish,  0, ROT0, "Igrosoft", "Lucky Haunter (040825, set 2)",  0 )
#if ALL_REVISIONS
GAME( 2003, lhaunt,      0,        multfish, multfish,  0, ROT0, "Igrosoft", "Lucky Haunter (030707)",  0 )
GAME( 2003, lhaunt_2,    lhaunt,   multfish, multfish,  0, ROT0, "Igrosoft", "Lucky Haunter (030804)",  0 )
GAME( 2003, lhaunt_3,    lhaunt,   multfish, multfish,  0, ROT0, "Igrosoft", "Lucky Haunter (031027)",  0 )
#endif

#if ALL_REVISIONS
GAME( 2003, rollfr,      0,        multfish, multfish,  0, ROT0, "Igrosoft", "Roll Fruit (030821)",  0 )
#endif
GAME( 2003, rollfr_2,    0,        multfish, multfish,  0, ROT0, "Igrosoft", "Roll Fruit (040318)",  0 )

GAME( 2004, garage_4,    0,        multfish, multfish,  0, ROT0, "Igrosoft", "Garage (040219, set 1)",  0 )
GAME( 2004, garage_4a,   garage_4, multfish, multfish,  0, ROT0, "Igrosoft", "Garage (040219, set 2)",  0 )
GAME( 2004, garage_5,    garage_4, multfish, multfish,  0, ROT0, "Igrosoft", "Garage (050311, set 1)",  0 )
GAME( 2004, garage_5a,   garage_4, multfish, multfish,  0, ROT0, "Igrosoft", "Garage (050311, set 2)",  0 )
#if ALL_REVISIONS
GAME( 2004, garage,      0,        multfish, multfish,  0, ROT0, "Igrosoft", "Garage (040122)",  0 )
GAME( 2004, garage_2,    garage,   multfish, multfish,  0, ROT0, "Igrosoft", "Garage (040123)",  0 )
GAME( 2004, garage_3,    garage,   multfish, multfish,  0, ROT0, "Igrosoft", "Garage (040216)",  0 )
#endif

GAME( 2004, rclimb_3,    0,        multfish, multfish,  0, ROT0, "Igrosoft", "Rock Climber (040827, set 1)", 0 )
GAME( 2004, rclimb_3a,   rclimb_3, multfish, multfish,  0, ROT0, "Igrosoft", "Rock Climber (040827, set 2)", 0 )
GAME( 2004, rclimb_3b,   rclimb_3, multfish, multfish,  0, ROT0, "Igrosoft", "Rock Climber (040827, set 3)", 0 ) // new service menu
#if ALL_REVISIONS
GAME( 2004, rclimb,      0,        multfish, multfish,  0, ROT0, "Igrosoft", "Rock Climber (040815)", 0 )
GAME( 2004, rclimb_2,    rclimb,   multfish, multfish,  0, ROT0, "Igrosoft", "Rock Climber (040823)", 0 )
#endif

GAME( 2004, sweetl,      0,        multfish, multfish,  0, ROT0, "Igrosoft", "Sweet Life (041220, set 1)",  0 )
GAME( 2004, sweetla,     sweetl,   multfish, multfish,  0, ROT0, "Igrosoft", "Sweet Life (041220, set 2)",  0 )

#if ALL_REVISIONS
GAME( 2004, resdnt,      0,        multfish, multfish,  0, ROT0, "Igrosoft", "Resident (040415)",  0 )
#endif
GAME( 2004, resdnt_2,    0,        multfish, multfish,  0, ROT0, "Igrosoft", "Resident (040513, set 1)",  0 )
GAME( 2004, resdnt_2a,   resdnt_2, multfish, multfish,  0, ROT0, "Igrosoft", "Resident (040513, set 2)",  0 )

GAME( 2005, island,      0,        multfish, multfish,  0, ROT0, "Igrosoft", "Island (050713, set 1)",  0 )
GAME( 2005, islanda,     island,   multfish, multfish,  0, ROT0, "Igrosoft", "Island (050713, set 2)",  0 )

GAME( 2005, pirate_1,    0,        multfish, multfish,  0, ROT0, "Igrosoft", "Pirate (060210)",  0 )
#if ALL_REVISIONS
GAME( 2005, pirate,      0,        multfish, multfish,  0, ROT0, "Igrosoft", "Pirate (051229)",  0 )
GAME( 2005, pirate_2,    pirate,   multfish, multfish,  0, ROT0, "Igrosoft", "Pirate (060803)",  0 )
#endif

GAME( 2006, island2,     0,        multfish, multfish,  0, ROT0, "Igrosoft", "Island 2 (060529, set 1)",  0 )
GAME( 2006, island2a,    island2,  multfish, multfish,  0, ROT0, "Igrosoft", "Island 2 (060529, set 2)",  0 )

GAME( 2006, keks,        0,        multfish, multfish,  0, ROT0, "Igrosoft", "Keks (060328, set 1)",  0 )
GAME( 2006, keksa,       keks,     multfish, multfish,  0, ROT0, "Igrosoft", "Keks (060328, set 2)",  0 )
GAME( 2006, keksb,       keks,     multfish, multfish,  0, ROT0, "Igrosoft", "Keks (060328, set 3)",  0 )
GAME( 2006, keks_2,      keks,     multfish, multfish,  0, ROT0, "Igrosoft", "Keks (060403. set 1)",  0 )
GAME( 2006, keks_2a,     keks,     multfish, multfish,  0, ROT0, "Igrosoft", "Keks (060403, set 2)",  0 )
