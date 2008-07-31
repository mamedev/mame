/***************************************************************************

                      -= IGS009 Based Games =-

                        driver by Luca Elia

CPU     :   Z180
Sound   :   M6295 + YM2413
Video   :   IGS009
NVRAM   :   Battery for main RAM

- The hardware is similar to other IGS002 + IGS003 based boards.
  The interesting part is the background tilemap, that is designed specifically
  for simulating the nine reels of a slot machine. The way it works is similar,
  but more complicated than what's emulated in iqblock.c or goldstar.c.
  It's not emulated properly yet, hence the game is not playable and marked NOT_WORKING.

***************************************************************************/

#include "driver.h"
#include "cpu/z180/z180.h"
#include "machine/8255ppi.h"
#include "sound/2413intf.h"
#include "sound/okim6295.h"

/***************************************************************************
                                Video Hardware
***************************************************************************/

static UINT8   *fg_tile_ram, *fg_color_ram;
static UINT8   *bg_tile_ram, *bg_scroll, *bg_scroll2;

static tilemap *fg_tilemap;
static tilemap *bg_tilemap;

static int video_enable;

static WRITE8_HANDLER( bg_scroll_w )
{
	bg_scroll[offset] = data;
	tilemap_set_scrolly(bg_tilemap,offset,data);
}


static TILE_GET_INFO( get_bg_tile_info )
{
	int code = bg_tile_ram[tile_index];
	SET_TILE_INFO(0, code + ((tile_index+1) % 4) * 0x100, (code & 0x80) ? 0xc : 0, 0);
}

static WRITE8_HANDLER( bg_tile_w )
{
	bg_tile_ram[offset] = data;
	tilemap_mark_tile_dirty(bg_tilemap,offset);
}


static TILE_GET_INFO( get_fg_tile_info )
{
	int code = fg_tile_ram[tile_index] | (fg_color_ram[tile_index] << 8);
	SET_TILE_INFO(1, code, (4*(code >> 14)+3), 0);
}

static WRITE8_HANDLER( fg_tile_w )
{
	fg_tile_ram[offset] = data;
	tilemap_mark_tile_dirty(fg_tilemap,offset);
}

static WRITE8_HANDLER( fg_color_w )
{
	fg_color_ram[offset] = data;
	tilemap_mark_tile_dirty(fg_tilemap,offset);
}

static VIDEO_START(jingbell)
{
	bg_tilemap = tilemap_create(get_bg_tile_info, tilemap_scan_rows,	8, 32,	0x80,0x20);
	fg_tilemap = tilemap_create(get_fg_tile_info, tilemap_scan_rows,	8,  8,	0x80,0x20);

	tilemap_set_transparent_pen(bg_tilemap, 0);
	tilemap_set_transparent_pen(fg_tilemap, 0);

	tilemap_set_scroll_cols(bg_tilemap, 0x80);
}

static VIDEO_UPDATE(jingbell)
{
	int layers_ctrl = video_enable ? -1 : 0;

#ifdef MAME_DEBUG
	if (input_code_pressed(KEYCODE_Z))
	{
		int mask = 0;
		if (input_code_pressed(KEYCODE_Q))	mask |= 1;
		if (input_code_pressed(KEYCODE_W))	mask |= 2;
		if (input_code_pressed(KEYCODE_A))	mask |= 4;
		if (mask != 0) layers_ctrl &= mask;
	}
#endif

	if (layers_ctrl & 1)
	{
		if (1)
		{
			// ********* experimental version of the bg rendering
			int x,y,z, tmap, ystart = 0;
			rectangle myclip;

			fillbitmap(bitmap, get_black_pen(screen->machine), cliprect);

			memcpy(&myclip,cliprect,sizeof(myclip));

			for (tmap = 0; tmap < 4; tmap++)
			{
				myclip.min_y = ystart;
				myclip.max_y = ystart+0x0100/4 - 1;
				for (y = 0; y < 0x8; y++)
				{
					for (x = 0; x < 0x80; x++)
					{
						int code = bg_tile_ram[x + y * 0x80 + bg_scroll2[tmap*0x20 + 4] * 0x80 * 0x8];
						for (z = 0; z < 2; z++)
						{
							drawgfx(bitmap,screen->machine->gfx[0],
									code + ((x+1) % 4) * 0x100, (code & 0x80) ? 0xc : 0,
									0, 0,
									x * 8, ystart + (y * 32 - ((bg_scroll[x + tmap * 0x80] + tmap*0x100/4) & 0xff)) + z * 0x8 * 0x20,
	//								&myclip, TRANSPARENCY_PEN, 0);
									&myclip, TRANSPARENCY_NONE, 0);
						}
					}
				}
				ystart += 0x100/4;
			}
			// *********
		}
		else				tilemap_draw(bitmap, cliprect, bg_tilemap, TILEMAP_DRAW_OPAQUE, 0);
	}
	else					fillbitmap(bitmap, get_black_pen(screen->machine), cliprect);


	if (layers_ctrl & 2)	tilemap_draw(bitmap, cliprect, fg_tilemap, 0, 0);

	return 0;
}

/***************************************************************************
                                Memory Maps
***************************************************************************/

static int nmi_enable, hopper;

static CUSTOM_INPUT( hopper_r )
{
	return hopper && !(video_screen_get_frame_number(field->port->machine->primary_screen)%10);
}

static UINT8 out[3];

static void show_out(void)
{
#ifdef MAME_DEBUG
	popmessage("%02x %02x %02x", out[0], out[1], out[2]);
#endif
}

static WRITE8_HANDLER( jingbell_nmi_and_coins_w )
{
	if ((nmi_enable ^ data) & (~0xdd))
	{
		logerror("PC %06X: nmi_and_coins = %02x\n",activecpu_get_pc(),data);
//		popmessage("%02x",data);
	}

	coin_counter_w(0,		data & 0x01);	// coin_a
	coin_counter_w(1,		data & 0x04);	// coin_c
	coin_counter_w(2,		data & 0x08);	// key in
	coin_counter_w(3,		data & 0x10);	// coin out mech

	set_led_status(6,		data & 0x40);	// led for coin out / hopper active

	nmi_enable = data;	//	data & 0x80		// nmi enable?

	out[0] = data;
	show_out();
}

static WRITE8_HANDLER( jingbell_video_and_leds_w )
{
	set_led_status(4,	  data & 0x01);	// start?
	set_led_status(5,	  data & 0x04);	// l_bet?

	video_enable	=	  data & 0x40;
	hopper			=	(~data)& 0x80;

	out[1] = data;
	show_out();
}

static WRITE8_HANDLER( jingbell_leds_w )
{
	set_led_status(0, data & 0x01);	// stop_1
	set_led_status(1, data & 0x02);	// stop_2
	set_led_status(2, data & 0x04);	// stop_3
	set_led_status(3, data & 0x08);	// stop
	// data & 0x10?

	out[2] = data;
	show_out();
}

static UINT8 igs_magic[2];

static WRITE8_HANDLER( jingbell_magic_w )
{
	igs_magic[offset] = data;

	if (offset == 0)
		return;

	switch(igs_magic[0])
	{
		case 0x01:
			break;

		default:
//          popmessage("magic %x <- %04x",igs_magic[0],data);
			logerror("%06x: warning, writing to igs_magic %02x = %02x\n", activecpu_get_pc(), igs_magic[0], data);
	}
}

static READ8_HANDLER( jingbell_magic_r )
{
	switch(igs_magic[0])
	{
		case 0x00:
			if ( !(igs_magic[1] & 0x01) )	return input_port_read(machine, "DSW1");
			if ( !(igs_magic[1] & 0x02) )	return input_port_read(machine, "DSW2");
			if ( !(igs_magic[1] & 0x04) )	return input_port_read(machine, "DSW3");
			if ( !(igs_magic[1] & 0x08) )	return input_port_read(machine, "DSW4");
			if ( !(igs_magic[1] & 0x10) )	return input_port_read(machine, "DSW5");
			logerror("%06x: warning, reading dsw with igs_magic[1] = %02x\n", activecpu_get_pc(), igs_magic[1]);
			break;

		default:
			logerror("%06x: warning, reading with igs_magic = %02x\n", activecpu_get_pc(), igs_magic[0]);
	}

	return 0;
}

static ADDRESS_MAP_START( jingbell_map, ADDRESS_SPACE_PROGRAM, 8 )
	AM_RANGE( 0x00000, 0x0f3ff ) AM_ROM
	AM_RANGE( 0x0f400, 0x0ffff ) AM_RAM AM_BASE( &generic_nvram ) AM_SIZE( &generic_nvram_size )
ADDRESS_MAP_END

static ADDRESS_MAP_START( jingbell_portmap, ADDRESS_SPACE_IO, 8 )
	AM_RANGE( 0x0000, 0x003f ) AM_RAM // Z180 internal regs

	AM_RANGE( 0x1000, 0x11ff ) AM_RAM_WRITE( bg_scroll_w ) AM_BASE( &bg_scroll )

	AM_RANGE( 0x2000, 0x23ff ) AM_RAM_WRITE( paletteram_xBBBBBGGGGGRRRRR_split1_w ) AM_BASE( &paletteram )
	AM_RANGE( 0x2400, 0x27ff ) AM_RAM_WRITE( paletteram_xBBBBBGGGGGRRRRR_split2_w ) AM_BASE( &paletteram_2 )

	AM_RANGE( 0x3000, 0x3fff ) AM_RAM_WRITE( bg_tile_w )  AM_BASE( &bg_tile_ram )

	AM_RANGE( 0x4000, 0x407f ) AM_RAM AM_BASE( &bg_scroll2 )

	AM_RANGE( 0x5000, 0x5fff ) AM_RAM_WRITE( fg_tile_w )  AM_BASE( &fg_tile_ram )

	AM_RANGE( 0x6480, 0x6480 ) AM_WRITE( jingbell_nmi_and_coins_w )

	AM_RANGE( 0x6481, 0x6481 ) AM_READ_PORT( "SERVICE" )
	AM_RANGE( 0x6482, 0x6482 ) AM_READ_PORT( "COINS" )
	AM_RANGE( 0x6490, 0x6490 ) AM_READ_PORT( "BUTTONS1" )
	AM_RANGE( 0x6491, 0x6491 ) AM_WRITE( jingbell_video_and_leds_w )
	AM_RANGE( 0x6492, 0x6492 ) AM_WRITE( jingbell_leds_w )
	AM_RANGE( 0x64a0, 0x64a0 ) AM_READ_PORT( "BUTTONS2" )

	AM_RANGE( 0x64b0, 0x64b0 ) AM_WRITE( YM2413_register_port_0_w )
	AM_RANGE( 0x64b1, 0x64b1 ) AM_WRITE( YM2413_data_port_0_w )

	AM_RANGE( 0x64c0, 0x64c0 ) AM_READWRITE( OKIM6295_status_0_r, OKIM6295_data_0_w )

	AM_RANGE( 0x64d0, 0x64d1 ) AM_READWRITE( jingbell_magic_r, jingbell_magic_w )	// DSW1-5

	AM_RANGE( 0x7000, 0x7fff ) AM_RAM_WRITE( fg_color_w ) AM_BASE( &fg_color_ram )

	AM_RANGE( 0x8000, 0xffff ) AM_ROM AM_REGION("data", 0)
ADDRESS_MAP_END


/***************************************************************************
                                Input Ports
***************************************************************************/

INPUT_PORTS_START( jingbell )
	PORT_START_TAG("DSW1")
	PORT_DIPNAME( 0x01, 0x00, DEF_STR( Demo_Sounds ) )
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x00, "W-Up Bonus" )
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )	// it's shown in attract mode
	PORT_DIPUNKNOWN( 0x04, 0x04 )
	PORT_DIPUNKNOWN( 0x08, 0x08 )
	PORT_DIPUNKNOWN( 0x10, 0x10 )
	PORT_DIPUNKNOWN( 0x20, 0x20 )
	PORT_DIPUNKNOWN( 0x40, 0x40 )
	PORT_DIPUNKNOWN( 0x80, 0x80 )

	PORT_START_TAG("DSW2")
	PORT_DIPUNKNOWN( 0x01, 0x01 )
	PORT_DIPUNKNOWN( 0x02, 0x02 )
	PORT_DIPUNKNOWN( 0x04, 0x04 )
	PORT_DIPUNKNOWN( 0x08, 0x08 )
	PORT_DIPUNKNOWN( 0x10, 0x10 )
	PORT_DIPUNKNOWN( 0x20, 0x20 )
	PORT_DIPUNKNOWN( 0x40, 0x40 )
	PORT_DIPUNKNOWN( 0x80, 0x80 )

	PORT_START_TAG("DSW3")
	PORT_DIPUNKNOWN( 0x01, 0x01 )
	PORT_DIPUNKNOWN( 0x02, 0x02 )
	PORT_DIPUNKNOWN( 0x04, 0x04 )
	PORT_DIPUNKNOWN( 0x08, 0x08 )
	PORT_DIPUNKNOWN( 0x10, 0x10 )
	PORT_DIPUNKNOWN( 0x20, 0x20 )
	PORT_DIPUNKNOWN( 0x40, 0x40 )
	PORT_DIPUNKNOWN( 0x80, 0x80 )

	PORT_START_TAG("DSW4")
	PORT_DIPUNKNOWN( 0x01, 0x01 )
	PORT_DIPUNKNOWN( 0x02, 0x02 )
	PORT_DIPUNKNOWN( 0x04, 0x04 )
	PORT_DIPUNKNOWN( 0x08, 0x08 )
	PORT_DIPUNKNOWN( 0x10, 0x10 )
	PORT_DIPUNKNOWN( 0x20, 0x20 )
	PORT_DIPUNKNOWN( 0x40, 0x40 )
	PORT_DIPUNKNOWN( 0x80, 0x80 )

	PORT_START_TAG("DSW5")
	PORT_DIPUNKNOWN( 0x01, 0x01 )
	PORT_DIPUNKNOWN( 0x02, 0x02 )
	PORT_DIPUNKNOWN( 0x04, 0x04 )
	PORT_DIPUNKNOWN( 0x08, 0x08 )
	PORT_DIPUNKNOWN( 0x10, 0x10 )
	PORT_DIPUNKNOWN( 0x20, 0x20 )
	PORT_DIPUNKNOWN( 0x40, 0x40 )
	PORT_DIPUNKNOWN( 0x80, 0x80 )

	PORT_START_TAG("SERVICE")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN  )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_SERVICE1 ) PORT_NAME("Memory Clear")	// stats, memory
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_SPECIAL  ) PORT_CUSTOM( hopper_r, (void *)0 )	// hopper sensor
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN  )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_START2   ) PORT_NAME("Pay Out")
	PORT_SERVICE_NO_TOGGLE( 0x20, IP_ACTIVE_LOW )	// test (press during boot)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_SERVICE2 ) PORT_NAME("Records")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN  )

	PORT_START_TAG("COINS")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_COIN1   )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_COIN2   )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_COIN3   ) PORT_NAME("Key In")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_COIN4   ) PORT_NAME("Key Down")	// pays out
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START_TAG("BUTTONS1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_NAME("Stop 1")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_NAME("Stop 2")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_NAME("Stop 3")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_NAME("Stop")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START_TAG("BUTTONS2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_START1  ) PORT_NAME("Start/H_Dup")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON7 ) PORT_NAME("Small")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON5 ) PORT_NAME("Left Bet/H_Dup")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_START4  ) PORT_NAME("Take")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON6 ) PORT_NAME("Right Bet/Dup")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON8 ) PORT_NAME("Big")
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )
INPUT_PORTS_END

/***************************************************************************
                                Graphics Layout
***************************************************************************/

static const gfx_layout layout_8x8x6 =
{
	8, 8,
	RGN_FRAC(1, 3),
	6,
	{ RGN_FRAC(0,3)+8,RGN_FRAC(0,3)+0,
	  RGN_FRAC(1,3)+8,RGN_FRAC(1,3)+0,
	  RGN_FRAC(2,3)+8,RGN_FRAC(2,3)+0 },
	{ STEP8(0,1) },
	{ STEP8(0,2*8) },
	8*8*2
};

static const gfx_layout layout_8x32x6 =
{
	8, 32,
	RGN_FRAC(1, 3),
	6,
	{ RGN_FRAC(0,3)+8,RGN_FRAC(0,3)+0,
	  RGN_FRAC(1,3)+8,RGN_FRAC(1,3)+0,
	  RGN_FRAC(2,3)+8,RGN_FRAC(2,3)+0 },
	{ STEP8(0,1) },
	{ STEP32(0,2*8) },
	8*32*2
};

static GFXDECODE_START( jingbell )
	GFXDECODE_ENTRY( "gfx1", 0, layout_8x32x6, 0, 16 )
	GFXDECODE_ENTRY( "gfx2", 0, layout_8x8x6,  0, 16 )
GFXDECODE_END


/***************************************************************************
                                Machine Drivers
***************************************************************************/

static MACHINE_RESET( jingbell )
{
	nmi_enable		=	0;
	hopper			=	0;
	video_enable	=	1;
}

static INTERRUPT_GEN( jingbell_interrupt )
{
	 if (nmi_enable & 0x80)
		cpunum_set_input_line(machine, 0, INPUT_LINE_NMI, PULSE_LINE);
}

static MACHINE_DRIVER_START( jingbell )
	/* basic machine hardware */
	MDRV_CPU_ADD("main", Z180, XTAL_12MHz / 2)	/* HD64180RP8, 8 MHz? */
	MDRV_CPU_PROGRAM_MAP(jingbell_map,0)
	MDRV_CPU_IO_MAP(jingbell_portmap,0)
	MDRV_CPU_VBLANK_INT("main",jingbell_interrupt)

	MDRV_MACHINE_RESET(jingbell)

	MDRV_NVRAM_HANDLER(generic_0fill)

	/* video hardware */
	MDRV_SCREEN_ADD("main", RASTER)
	MDRV_SCREEN_REFRESH_RATE(60)
	MDRV_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MDRV_SCREEN_FORMAT(BITMAP_FORMAT_INDEXED16)
	MDRV_SCREEN_SIZE(512, 256)
	MDRV_SCREEN_VISIBLE_AREA(0, 512-1, 0, 256-16-1)

	MDRV_GFXDECODE(jingbell)
	MDRV_PALETTE_LENGTH(0x400)

	MDRV_VIDEO_START(jingbell)
	MDRV_VIDEO_UPDATE(jingbell)

	/* sound hardware */
	MDRV_SPEAKER_STANDARD_MONO("mono")
	MDRV_SOUND_ADD("ym", YM2413, XTAL_3_579545MHz)
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "mono", 1.0)

	MDRV_SOUND_ADD("oki", OKIM6295, XTAL_12MHz / 12)
	MDRV_SOUND_CONFIG(okim6295_interface_pin7high)
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "mono", 1.0)
MACHINE_DRIVER_END

/***************************************************************************

Jingle Bell
(C) 1998 IGS

CPU:
    1x HD64180RP8 (u18)(main)
    2x NEC D8255AC (u19,u20)(main)
    1x custom IGS009-F56D246 (u22)
    1x U3567HX881 (u45)(sound equivalent to ym2413)
    1x AR17961-AP0848 (u46)(sound equivalent to m6295)
    1x oscillator 12.000
    1x oscillator 3.579545

ROMs:
    3x M27C512 (1,2,3)
    1x LE27C2001F (4)
    2x MX27C2000 (5,6)
    1x D27256 (7)
    1x MX27C512 (v)
    1x NM27C010 (sp)
    2x PALCE16V8H (read protected)
    1x PALCE22V10H (read protected)
    1x PALCE22V10H (dumped)

Notes:
    1x 38x2 edge connector
    1x 10x2 edge connector
    1x pushbutton
    1x battery
    5x 8x2 switches dip

12/02/2008 f205v

***************************************************************************/

ROM_START( jingbell )
	ROM_REGION( 0x10000, "main", 0 )
	ROM_LOAD( "jinglev133i.u44", 0x00000, 0x10000, CRC(df60dc39) SHA1(ff57afd50c045b621395353fdc50ffd1e1b65e9e) )

	ROM_REGION( 0x8000, "data", 0 )
	ROM_LOAD( "jingle133i7.u43", 0x0000, 0x8000, CRC(a7affa15) SHA1(f9d33e32b57ad267d383e075663994e0af0b3016) )

	ROM_REGION( 0x30000, "gfx1", ROMREGION_DISPOSE )
	ROM_LOAD( "jingle133i1.u17", 0x00000, 0x10000, CRC(cadd7910) SHA1(aa514ddb29c8c9a77478d56bea4ae71995fdd518) )
	ROM_LOAD( "jingle133i2.u16", 0x10000, 0x10000, CRC(a9e1f5aa) SHA1(68d7f4e9e9a5bbce0904e406ee6fe82e9e52a9ba) )
	ROM_LOAD( "jingle133i3.u15", 0x20000, 0x10000, CRC(865b7d3a) SHA1(c1dff3a27d747ee499aaee0c4468534f0249a3e5) )

	ROM_REGION( 0xc0000, "gfx2", ROMREGION_DISPOSE )
	ROM_LOAD( "jingle133i4.u25", 0x00000, 0x40000, CRC(7aa1d344) SHA1(141e27df93cb35ab852d9022e0b08bd596f1186b) )
	ROM_LOAD( "jingle133i5.u24", 0x40000, 0x40000, CRC(021261d1) SHA1(5b23f9bd818193c343f9f4c9317955b17efb8cfa) )
	ROM_LOAD( "jingle133i6.u23", 0x80000, 0x40000, CRC(c40228fd) SHA1(4dc05337d64ed2b8d66fc5f0ca8ffbf96799f768) )

	ROM_REGION( 0x20000, "oki", 0 )
	ROM_LOAD( "jingle133isp.u38", 0x00000, 0x20000, CRC(a42d73b1) SHA1(93157e9630d5c8bb34c71186415d0aa8c5d51951) )

	ROM_REGION( 0x2dd, "plds",0 )
	ROM_LOAD( "palce16v8h-ch-jin-u12v.u12", 0x000, 0x117, CRC(c89d2f52) SHA1(f9d52d9c42ef95b7b85bbf6d09888ebdeac11fd3) )
	ROM_LOAD( "palce16v8h-ch-jin-u33v.u33", 0x000, 0x117, CRC(c89d2f52) SHA1(f9d52d9c42ef95b7b85bbf6d09888ebdeac11fd3) )
	ROM_LOAD( "palce22v10h-ajbu24.u24",     0x000, 0x2dd, CRC(6310f441) SHA1(b610e170ccca1fcb06a57f718ece1408b696ba9c) )
	ROM_LOAD( "palce22v10h-ch-jin-u27.u27", 0x000, 0x2dd, CRC(5c4e9024) SHA1(e9d1e4df3d79c21f4ce053a84bb7b7a43d650f91) )
ROM_END

static DRIVER_INIT( jingbell )
{
	int i;
	UINT8 *rom  = (UINT8 *)memory_region(machine, "main");
	size_t size = memory_region_length(machine, "main");

	for (i=0; i<size; i++)
	{
		UINT8 x = rom[i];
		if (i & 0x0080)
		{
			if ((i & 0x0420) == 0x0420)	x ^= 0x20;
			else						x ^= 0x22;
		}
		else
		{
			if (i & 0x0200)	x ^= 0x02;
			else			x ^= 0x22;
		}

		if ((i & 0x1208) == 0x1208)	x ^= 0x01;

		rom[i] = x;
	}

	// protection patch
	rom[0x01f19] = 0x18;
}

GAME( 1995?, jingbell, 0, jingbell, jingbell, jingbell, ROT0, "IGS", "Jingle Bell (Italy, V133I)", GAME_NOT_WORKING | GAME_IMPERFECT_GRAPHICS | GAME_UNEMULATED_PROTECTION )
