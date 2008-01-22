/* Igrosoft hardware platform driver

  four ALTERA chips - they are probably video processors
  main CPU is z80 for sure, ROM is placed near it
  there are banked ROMs 8kb each, consisted of several banks
  RAM has E000 address
  the sound is PSG - AY3-8910 analog (http://pt.wikipedia.org/wiki/KC89C72)
  z80 CPU is complete with indexed registers
  video - VGA

  This looks like late 1980s hardware, but some of the games have revisions
  as recent as 2007 (!)

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

*/

#include "driver.h"
#include "sound/ay8910.h"


#define IGRO_VIDRAM_SIZE (0x2000*0x10)
#define IGRO_BRAM_SIZE (0x2000*0x10)

static UINT8* igro_vid;
static UINT8* igro_bram;

static int igrosoft_disp_enable;

/* Video Part */
static VIDEO_START(igrosoft)
{
	igro_vid = auto_malloc(IGRO_VIDRAM_SIZE);
	memset(igro_vid,0x00,IGRO_VIDRAM_SIZE);
	state_save_register_global_pointer(igro_vid, IGRO_VIDRAM_SIZE);

	igro_bram = auto_malloc(IGRO_BRAM_SIZE);
	memset(igro_bram,0x00,IGRO_BRAM_SIZE);
	state_save_register_global_pointer(igro_bram, IGRO_BRAM_SIZE);
}

static VIDEO_UPDATE(igrosoft)
{
	int y,x,count;
	gfx_element* gfx = machine->gfx[0];
	fillbitmap(bitmap, get_black_pen(machine), cliprect);

	if (!igrosoft_disp_enable) return 0;

	/* Draw lower part of static tilemap (low pri tiles) */
	count = 0x0000;
	for (y=0;y<64;y++)
	{
		for (x=0;x<64;x++)
		{
			int tile, pal;

			tile = igro_vid[count*2+0] | (igro_vid[count*2+1] << 8);
			pal = igro_vid[count*2+0+0x1000] | (igro_vid[count*2+1+0x1000] << 8);
			if ((pal & 0x0100))
			{
				tile &=0x1fff;
				pal &=0x7;
				drawgfx(bitmap,gfx,tile,pal,0,0,x*16,y*16,cliprect,TRANSPARENCY_PEN,255);
			}
			count++;
		}
	}

	/* Draw scrollable tilemap (used for reels etc.) */
	count = 0x1000;
	for (y=0;y<64;y++)
	{
		for (x=0;x<64;x++)
		{
			int tile, pal;
			int colscroll;
			colscroll = (igro_vid[x*2] | igro_vid[x*2+1] << 8);
			tile = igro_vid[count*2+0] | (igro_vid[count*2+1] << 8);
			pal = tile>>14;

			tile &=0x1fff;
			tile |=0x2000;
			drawgfx(bitmap,gfx,tile,pal+8,0,0,x*16,(y*16-colscroll)&0x3ff,cliprect,TRANSPARENCY_PEN,255);

			count++;

		}
	}

	/* Draw upper part of static tilemap (high pri tiles) */
	count = 0x0000;
	for (y=0;y<64;y++)
	{
		for (x=0;x<64;x++)
		{
			int tile, pal;

			tile = igro_vid[count*2+0] | (igro_vid[count*2+1] << 8);
			pal = igro_vid[count*2+0+0x1000] | (igro_vid[count*2+1+0x1000] << 8);
			if (!(pal & 0x0100))
			{
				tile &=0x1fff;
				pal &=0x7;
				drawgfx(bitmap,gfx,tile,pal,0,0,x*16,y*16,cliprect,TRANSPARENCY_PEN,255);
			}
			count++;
		}
	}

	/* set palette */
	{
		int z;
		int c = 0x4000;

		for (z=0;z<0x1000;z++)
		{
			int r,g,b;
			int coldat;

			coldat = igro_vid[c+z*2+0] | (igro_vid[c+z*2+1] << 8);

			r = ( (coldat &0x001f)>> 0);
			g = ( (coldat &0x1f00)>> 8);
			b = ( (coldat &0x00e0)>> (5));
			b|= ( (coldat &0xe000)>> (8+5-3));

			palette_set_color_rgb(Machine, z, r<<3, g<<3, b<<2);

		}
	}
	return 0;
}

static WRITE8_HANDLER( igrosoft_vid_w )
{
	igro_vid[offset]=data;
}

static WRITE8_HANDLER( igrosoft_bank_w )
{
	memory_set_bank(1, data & 0x0f);
}

static UINT8 rambk = 0;
static UINT8 otherrambk = 0;

static READ8_HANDLER( bankedram_r )
{
	if ((otherrambk & 0x80) == 0x00)
	{
		return igro_bram[offset+0x2000*rambk];
	}
	else
	{
		return igro_vid[offset+0x2000*rambk];
	}

}

static WRITE8_HANDLER( bankedram_w )
{
	if ((otherrambk & 0x80) == 0x00)
	{
		igro_bram[offset+0x2000*rambk] = data;
	}
	else
	{
		igro_vid[offset+0x2000*rambk] = data;
	}
}

static WRITE8_HANDLER( igrosoft_rambank_w )
{
	rambk = data & 0x0f;
	otherrambk = data & 0xf0;
}


static READ8_HANDLER( ray_r )
{
	// the games read the raster beam position as part of the hardware checks..
	// with a 6mhz clock and 640x480 resolution this seems to give the right results.
	return video_screen_get_vpos(0);
}



static ADDRESS_MAP_START( igrosoft_map, ADDRESS_SPACE_PROGRAM, 8 )
	AM_RANGE(0x0000, 0x7fff) AM_READ(MRA8_ROM)
	AM_RANGE(0x0000, 0x7fff) AM_WRITE( igrosoft_vid_w )
	AM_RANGE(0x8000, 0xbfff) AM_READWRITE(MRA8_BANK1, MWA8_ROM )
	AM_RANGE(0xc000, 0xdfff) AM_RAM AM_BASE(&generic_nvram) AM_SIZE(&generic_nvram_size)
	AM_RANGE(0xe000, 0xffff) AM_READWRITE(bankedram_r, bankedram_w)
ADDRESS_MAP_END

// According to the self test the 7 user buttons are arranged as
// Bet/Cancel  |  1 Line  |  3 Lines  |  5 Lines  | 7 Lines  | 9 Lines  | Start


static INPUT_PORTS_START( igrosoft )
	PORT_START_TAG("IN0")
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

	PORT_START_TAG("IN1")
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


	PORT_START_TAG("IN2")
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


	PORT_START_TAG("IN3")
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


	PORT_START_TAG("IN4")
	PORT_DIPNAME(     0x01, 0x01, "S Reserve (35 B)" )
	PORT_DIPSETTING(  0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(  0x00, DEF_STR( On ) )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_COIN3 ) // COIN C (19 A)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON9 ) PORT_NAME("Help") PORT_CODE(KEYCODE_S)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_BUTTON6 ) PORT_NAME("9 Lines") PORT_CODE(KEYCODE_N)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_NAME("1 Line") PORT_CODE(KEYCODE_X)
	PORT_BIT( 0xe0, IP_ACTIVE_LOW, IPT_UNUSED ) // unused?


	PORT_START_TAG("IN5")
	PORT_SERVICE(     0x01, IP_ACTIVE_LOW )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_COIN4 ) // COIN D (19 B)
	PORT_DIPNAME(     0x04, 0x04, "S Reserve (16 B)" )
	PORT_DIPSETTING(  0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(  0x00, DEF_STR( On ) )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_NAME("Bet / Double / Cancel") PORT_CODE(KEYCODE_Z)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_COIN6 ) // BILL 2 (05 A)
	PORT_BIT( 0xe0, IP_ACTIVE_LOW, IPT_UNUSED ) // unused?

	PORT_START_TAG("IN6")
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

	PORT_START_TAG("IN7")
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


WRITE8_HANDLER( igrosoft_f3_w )
{
	//popmessage("Igrosoft_f3_w %02x",data);
}


WRITE8_HANDLER( igrosoft_f4_w )
{
	//popmessage("Igrosoft_f4_w %02x",data); // display enable?
	igrosoft_disp_enable = data;
}

static ADDRESS_MAP_START( igrosoft_portmap, ADDRESS_SPACE_IO, 8 )
	ADDRESS_MAP_FLAGS( AMEF_ABITS(8) )
	AM_RANGE(0x10, 0x10) AM_READ(input_port_0_r)
	AM_RANGE(0x11, 0x11) AM_READ(input_port_1_r)
	AM_RANGE(0x12, 0x12) AM_READ(input_port_2_r)
	AM_RANGE(0x13, 0x13) AM_READ(input_port_3_r)
	AM_RANGE(0x14, 0x14) AM_READ(input_port_4_r)
	AM_RANGE(0x15, 0x15) AM_READ(input_port_5_r)
	AM_RANGE(0x16, 0x16) AM_READ(input_port_6_r)
	AM_RANGE(0x17, 0x17) AM_READ(input_port_7_r)

	/* Write ports not hooked up yet (lights etc.) */
//	AM_RANGE(0x30, 0x30) AM_WRITE(igrosoft_port30_w)
//	AM_RANGE(0x31, 0x31) AM_WRITE(igrosoft_port31_w)
//	AM_RANGE(0x32, 0x32) AM_WRITE(igrosoft_port32_w)
//	AM_RANGE(0x33, 0x33) AM_WRITE(igrosoft_port33_w)
//	AM_RANGE(0x34, 0x34) AM_WRITE(igrosoft_port34_w)
//	AM_RANGE(0x35, 0x35) AM_WRITE(igrosoft_port35_w)
//	AM_RANGE(0x36, 0x36) AM_WRITE(igrosoft_port36_w)
//	AM_RANGE(0x37, 0x37) AM_WRITE(igrosoft_watchdog_reset_w)

	AM_RANGE(0x90, 0x90) AM_READ(ray_r)

	AM_RANGE(0xe1, 0xe1)  AM_WRITE(igrosoft_bank_w)
	AM_RANGE(0xe5, 0xe5)  AM_WRITE(igrosoft_bank_w)

	AM_RANGE(0xf1, 0xf1)  AM_WRITE(igrosoft_rambank_w)
	AM_RANGE(0xf3, 0xf3)  AM_WRITE(igrosoft_f3_w) // from 00->01 at startup, irq enable maybe?
	AM_RANGE(0xf4, 0xf4)  AM_WRITE(igrosoft_f4_w) // display enable?

	/* other mirrors of the rom banking, used by various games / sets */
	AM_RANGE(0xf8, 0xf8)  AM_WRITE(igrosoft_bank_w)
	AM_RANGE(0xf9, 0xf9)  AM_WRITE(igrosoft_bank_w)
	AM_RANGE(0xfa, 0xfa)  AM_WRITE(igrosoft_bank_w)
	AM_RANGE(0xfb, 0xfb)  AM_WRITE(igrosoft_bank_w)
	AM_RANGE(0xfc, 0xfc)  AM_WRITE(igrosoft_bank_w)
	AM_RANGE(0xfd, 0xfd)  AM_WRITE(igrosoft_bank_w)





	AM_RANGE(0x38, 0x38) AM_WRITE(AY8910_control_port_0_w)
	AM_RANGE(0x39, 0x39) AM_WRITE(AY8910_write_port_0_w)
	AM_RANGE(0x3A, 0x3A) AM_READ(AY8910_read_port_0_r)


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



static GFXDECODE_START( igrosoft )
	GFXDECODE_ENTRY( REGION_GFX1, 0, tiles16x16_layout, 0, 16 )
GFXDECODE_END

MACHINE_RESET( igrosoft )
{
	memory_configure_bank(1, 0, 16, memory_region(REGION_CPU1), 0x4000);
	memory_set_bank(1, 0);
}

static struct AY8910interface ay8910_interface =
{
0, 0, 0, 0	/* no ports used */
};


static MACHINE_DRIVER_START( igrosoft )
	/* basic machine hardware */
	MDRV_CPU_ADD(Z80,6000000) /* 6 MHz? */
	MDRV_CPU_PROGRAM_MAP(igrosoft_map,0)
	MDRV_CPU_IO_MAP(igrosoft_portmap,0)
	MDRV_CPU_VBLANK_INT(irq0_line_hold,1)

	MDRV_SCREEN_REFRESH_RATE(60)
	MDRV_SCREEN_VBLANK_TIME(DEFAULT_60HZ_VBLANK_DURATION)
	MDRV_MACHINE_RESET( igrosoft )
	/* video hardware */
	MDRV_VIDEO_ATTRIBUTES(VIDEO_TYPE_RASTER )
	MDRV_SCREEN_FORMAT(BITMAP_FORMAT_INDEXED16)
	MDRV_SCREEN_SIZE(64*16, 32*16)
	MDRV_SCREEN_VISIBLE_AREA(17*16, 1024-16*7-1, 1*16, 32*16-1*16-1)
	MDRV_GFXDECODE(igrosoft)
	MDRV_PALETTE_LENGTH(0x1000)

	MDRV_NVRAM_HANDLER(generic_0fill)

	MDRV_VIDEO_START(igrosoft)
	MDRV_VIDEO_UPDATE(igrosoft)

	MDRV_SPEAKER_STANDARD_MONO("mono")
	MDRV_SOUND_ADD(AY8910,6000000/4)	/* 1.5 MHz? */
	MDRV_SOUND_CONFIG(ay8910_interface)
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.30)
MACHINE_DRIVER_END


#include "isdrvr.c"
















