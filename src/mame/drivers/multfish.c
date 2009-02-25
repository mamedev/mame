/* 'Multifish' hardware

  four ALTERA chips - they are probably video processors
  main CPU is z80 for sure, ROM is placed near it
  there are banked ROMs 8kb each, consisted of several banks
  RAM has E000 address
  the sound is PSG - AY3-8910 analog (http://pt.wikipedia.org/wiki/KC89C72)
  z80 CPU is complete with indexed registers
  video - VGA

  Note,

  Payout doesn't currently work and causes 'Call Attendant' to be displayed
  Lamps not hooked up

  To Init the games

  Press 'F2' (Turn Service Mode ON)
  Press 'F3' (Reset)
  Use 'C' (3 Lines) to move pointer to INIT
  press 'M' (Start) to enter INIT menu
  hold 'Z' (Bet/Double) for 5 seconds while counter counts down
  press 'F2' (Turn Service Mode OFF)
  Press 'F3' (Reset)

  RTC not emulated?
  Lights not emulated?
  Payout not emulated


*/

// revision information comes from dat files, not all of them can be tested
//  for some only MD5 and CRC information is present
#define ALL_REVISIONS 1

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
	multfish_vid = auto_malloc(multfish_VIDRAM_SIZE);
	memset(multfish_vid,0x00,multfish_VIDRAM_SIZE);
	state_save_register_global_pointer(machine, multfish_vid, multfish_VIDRAM_SIZE);

	multfish_bram = auto_malloc(multfish_BRAM_SIZE);
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
	AM_RANGE(0x8000, 0xbfff) AM_READWRITE(SMH_BANK1, SMH_ROM )
	AM_RANGE(0xc000, 0xdfff) AM_RAM AM_BASE(&generic_nvram) AM_SIZE(&generic_nvram_size)
	AM_RANGE(0xe000, 0xffff) AM_READWRITE(bankedram_r, bankedram_w)
ADDRESS_MAP_END

// According to the self test the 7 user buttons are arranged as
// Bet/Cancel  |  1 Line  |  3 Lines  |  5 Lines  | 7 Lines  | 9 Lines  | Start


INPUT_PORTS_START( multfish )
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
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_BUTTON7 ) PORT_NAME("Start") PORT_CODE(KEYCODE_M)
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


WRITE8_HANDLER( multfish_f3_w )
{
	//popmessage("multfish_f3_w %02x",data);
}


WRITE8_HANDLER( multfish_f4_w )
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
//	AM_RANGE(0x30, 0x30) AM_WRITE(multfish_port30_w)
//	AM_RANGE(0x31, 0x31) AM_WRITE(multfish_port31_w)
//	AM_RANGE(0x32, 0x32) AM_WRITE(multfish_port32_w)
//	AM_RANGE(0x33, 0x33) AM_WRITE(multfish_port33_w)
//	AM_RANGE(0x34, 0x34) AM_WRITE(multfish_port34_w)
//	AM_RANGE(0x35, 0x35) AM_WRITE(multfish_port35_w)
//	AM_RANGE(0x36, 0x36) AM_WRITE(multfish_port36_w)
//	AM_RANGE(0x37, 0x37) AM_WRITE(multfish_watchdog_reset_w)
	AM_RANGE(0x38, 0x38) AM_DEVWRITE(SOUND, "ay", ay8910_address_w)
	AM_RANGE(0x39, 0x39) AM_DEVWRITE(SOUND, "ay", ay8910_data_w)
	AM_RANGE(0x3a, 0x3a) AM_DEVREAD(SOUND, "ay", ay8910_r)
	
	AM_RANGE(0x90, 0x90) AM_READ(ray_r)

	AM_RANGE(0xe1, 0xe1)  AM_WRITE(multfish_bank_w)
	AM_RANGE(0xe5, 0xe5)  AM_WRITE(multfish_bank_w)

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

MACHINE_RESET( multfish )
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


MACHINE_DRIVER_START( multfish )
	/* basic machine hardware */
	MDRV_CPU_ADD("maincpu", Z80,6000000) /* 6 MHz? */
	MDRV_CPU_PROGRAM_MAP(multfish_map,0)
	MDRV_CPU_IO_MAP(multfish_portmap,0)
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
   Multifish (14 sets)
**********************************************************/

#if ALL_REVISIONS
ROM_START( mfish ) // 040316
	ROM_REGION( 0x40000, "maincpu", 0 )
	ROM_LOAD( "mf040316.rom", 0x00000, 0x40000, MD5(66019927201954518261652147b05e43) SHA1(c1f4d1c51632a45b533d19c8b6f63d337d84d9cd) )

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

ROM_START( mfish_2 ) // 040308
	ROM_REGION( 0x40000, "maincpu", 0 )
	ROM_LOAD( "mf040308.rom", 0x00000, 0x40000, CRC(adb9c1d9) SHA1(88c69f48766dc7c98a6f03c1a0a4aa63b76560b6) )

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

ROM_START( mfish_2a ) // 040308
	ROM_REGION( 0x40000, "maincpu", 0 )
	ROM_LOAD( "mf040308_alt.rom", 0x00000, 0x40000, CRC(44537648) SHA1(7bce6085778ff0b21c052ae91703de3b78b8eed0))

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
ROM_START( mfish_3 ) // 031124
	ROM_REGION( 0x40000, "maincpu", 0 )
	ROM_LOAD( "mf031124.rom", 0x00000, 0x40000, MD5(9cd800719c6e4a2267e3c140467238d3) SHA1(c0d1b541c4b076bbc810ad637acb4a2663a919ba) )

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

ROM_START( mfish_4 ) // 031117
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

ROM_START( mfish_5 ) // 031026
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

ROM_START( mfish_6 ) // 030522
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

ROM_START( mfish_8 ) // 030124
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

ROM_START( mfish_9 ) // 021227
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

ROM_START( mfish_10 ) // 021219
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

ROM_START( mfish_11 ) // 021124
	ROM_REGION( 0x40000, "maincpu", 0 )
	ROM_LOAD( "mf021124.rom", 0x00000, 0x40000, MD5(a3bfe235ca1d2efa51cc55465a8161e4) SHA1(ea132f68e9c09c40369d4cc02c670ee6e26bdcbe) )

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

ROM_START( mfish_12 ) // 021121
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

ROM_START( mfish_13 ) // 021120
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
#endif

#if ALL_REVISIONS /* Multifish sets verified against dat */
GAME(   2004, mfish,      0,       multfish, multfish,  0, ROT0, "Igro", "Multi Fish (040316)",  0 )
GAME(   2004, mfish_2,    mfish,    multfish, multfish,  0, ROT0, "Igro", "Multi Fish (040308, set 1)",  0 )
GAME(   2004, mfish_2a,   mfish,    multfish, multfish,  0, ROT0, "Igro", "Multi Fish (040308, set 2, hack?)",  0 )
GAME(   2003, mfish_3,    mfish,    multfish, multfish,  0, ROT0, "Igro", "Multi Fish (031124)",  0 )
GAME(   2003, mfish_4,    mfish,    multfish, multfish,  0, ROT0, "Igro", "Multi Fish (031117)",  0 )
GAME(   2003, mfish_5,    mfish,    multfish, multfish,  0, ROT0, "Igro", "Multi Fish (031026)",  0 )
GAME(   2003, mfish_6,    mfish,    multfish, multfish,  0, ROT0, "Igro", "Multi Fish (030522)",  0 )
GAME(   2003, mfish_7,    mfish,    multfish, multfish,  0, ROT0, "Igro", "Multi Fish (030511)",  0 )
GAME(   2003, mfish_8,    mfish,    multfish, multfish,  0, ROT0, "Igro", "Multi Fish (030124)",  0 )
GAME(   2002, mfish_9,    mfish,    multfish, multfish,  0, ROT0, "Igro", "Multi Fish (021227)",  0 )
GAME(   2002, mfish_10,   mfish,    multfish, multfish,  0, ROT0, "Igro", "Multi Fish (021219)",  0 )
GAME(   2002, mfish_11,   mfish,    multfish, multfish,  0, ROT0, "Igro", "Multi Fish (021124)",  0 )
GAME(   2002, mfish_12,   mfish,    multfish, multfish,  0, ROT0, "Igro", "Multi Fish (021121)",  0 )
GAME(   2002, mfish_13,   mfish,    multfish, multfish,  0, ROT0, "Igro", "Multi Fish (021120)",  0 )
#else /* Multifish sets for which SHA1 information is available */
GAME(   2004, mfish_2,    0,       multfish, multfish,  0, ROT0, "Igro", "Multi Fish (040308, set 1)",  0 )
GAME(   2004, mfish_2a,   mfish_2,  multfish, multfish,  0, ROT0, "Igro", "Multi Fish (040308, set 2, hack?)",  0 )
#endif





