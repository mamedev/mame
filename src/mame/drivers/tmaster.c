/***************************************************************************

                          -=  Touch Master Games =-

                    driver by   Luca Elia (l.elia@tin.it)


CPU:    68000
Video:  Blitter, double framebuffer
Sound:  OKI6295
Input:  Pressure sensitive touch screen
Other:  Dallas NVRAM + optional RTC


To Do:

- Proper touchscreen controller emulation. Currently it's flakey and
  tends to stop registering user input (try coining up in that case)
- Protection in tm4k
- Coin optics
- RTC emulation (there's code to check the upper bytes of NVRAM to see if
  the real time clock is present, and to only use it in that case)


***************************************************************************/

#include "driver.h"
#include "deprecat.h"
#include "sound/okim6295.h"

/***************************************************************************

                                   Sound

***************************************************************************/

static int okibank;
static WRITE16_HANDLER( tmaster_oki_bank_w )
{
	if (ACCESSING_MSB)
	{
		// data & 0x0800?
		okibank = ((data >> 8) & 3);
		OKIM6295_set_bank_base(0, okibank * 0x40000);
	}

	if (ACCESSING_LSB)
	{
		// data & 0x0002?
	}
}

/***************************************************************************

                     Touch Screen Controller - PRELIMINARY

***************************************************************************/

static int touchscreen;

static void show_touchscreen(void)
{
#ifdef MAME_DEBUG
	popmessage("% d] %03x %03x - %d",touchscreen,readinputportbytag("TSCREEN_X")&0x1ff,readinputportbytag("TSCREEN_Y"),okibank);
#endif
}

static WRITE16_HANDLER( tmaster_tscreen_reset_w )
{
	if (ACCESSING_LSB && data == 0x05)
	{
		touchscreen = 0;
		show_touchscreen();
	}
}

static READ16_HANDLER( tmaster_tscreen_next_r )
{
	if (touchscreen != -1)
		touchscreen++;
	if (touchscreen == 6)
		touchscreen = -1;
	show_touchscreen();

	return 0;
}

static READ16_HANDLER( tmaster_tscreen_x_hi_r )
{
	switch (touchscreen)
	{
		case -1:	return 0xf1;
	}
	return 0x01;
}

static READ16_HANDLER( tmaster_tscreen_x_lo_r )
{
	UINT16 val = 0;

	int press1 = readinputportbytag("TSCREEN_X") & 0x4000;
	int press2 = readinputportbytag("TSCREEN_X") & 0x8000;
	if (press1)	press2 = 1;

	switch (touchscreen)
	{
		case 1:	val = press1 ? 0 : 1<<6;	break;	// press
		case 2:	val = readinputportbytag("TSCREEN_X") & 0x003;	break;
		case 3:	val = (readinputportbytag("TSCREEN_X") >> 2) & 0x7f;	break;
		case 4:	val = 0;	break;
		case 5:	val = ((readinputportbytag("TSCREEN_Y")^0xff) >> 1) & 0x7f;	break;

		default:
			return 0;
	}
	return val | (press2 ? 0x80 : 0);	// away : hover
}

static READ16_HANDLER( tmaster_tscreen_y_hi_r )	{	return 0x01;	}
static READ16_HANDLER( tmaster_tscreen_y_lo_r )	{	return 0x00;	}



/***************************************************************************

                                Video & Blitter

***************************************************************************/

static mame_bitmap *tmaster_bitmap[2];
static UINT16 *tmaster_regs;
static UINT16 tmaster_color;
static UINT16 tmaster_addr;

static VIDEO_START( tmaster )
{
	int i;
	for (i = 0; i < 2; i++)
		tmaster_bitmap[i] = auto_bitmap_alloc(machine->screen[0].width,machine->screen[0].height,machine->screen[0].format);
}

static VIDEO_UPDATE( tmaster )
{
	// double buffering
	copybitmap(bitmap,tmaster_bitmap[(tmaster_regs[0x02/2]>>8)&1],0,0,0,0,cliprect);

	show_touchscreen();
	return 0;
}

static WRITE16_HANDLER( tmaster_color_w )
{
	COMBINE_DATA( &tmaster_color );
	if (tmaster_color & ~7)
		logerror("%06x: color %04x\n", activecpu_get_pc(), tmaster_color);
}

static WRITE16_HANDLER( tmaster_addr_w )
{
	COMBINE_DATA( &tmaster_addr );
}

static void tmaster_draw(void)
{
	int x,y,x0,x1,y0,y1,dx,dy,flipx,flipy,sx,sy,sw,sh, addr, mode, dest;

	UINT8 *gfxdata	=	memory_region( REGION_GFX1 );
	size_t size		=	memory_region_length( REGION_GFX1 );

	UINT16 data;

	mame_bitmap *bitmap;

	dest	=	 (tmaster_regs[0x02/2] >> 8) & 1;
 	sw		=	  tmaster_regs[0x04/2];
	sx		=	  tmaster_regs[0x06/2];
	sh		=	  tmaster_regs[0x08/2] + 1;
	sy		=	  tmaster_regs[0x0a/2];
	addr	=	( tmaster_regs[0x0c/2] & 0xff) |
				((tmaster_regs[0x0e/2] & 0x1ff) << 8) | (tmaster_addr << 17);
	mode	=	  tmaster_regs[0x10/2];

	dest	^=	(mode >> 6) & 1;
	bitmap	=	tmaster_bitmap[dest];

	addr <<= 1;

#if 0
	logerror("%06x: blit w %03x, h %02x, x %03x, y %02x, addr %06x, mode %02x\n", activecpu_get_pc(),
			sw,sh,sx,sy, addr, mode
	);
#endif

	flipx = mode & 1;
	flipy = mode & 2;

	if (flipx)	{ x0 = sw-1;	x1 = -1;	dx = -1;	sx -= sw-1;	}
	else		{ x0 = 0;		x1 = sw;	dx = +1;	}

	if (flipy)	{ y0 = sh-1;	y1 = -1;	dy = -1;	sy -= sh-1;	}
	else		{ y0 = 0;		y1 = sh;	dy = +1;	}

	switch (mode & 0x20)
	{
		case 0x00:							// blit with transparency
			if (addr > (size - sw*sh))
			{
				logerror("%06x: blit error, addr %06x out of bounds\n", activecpu_get_pc(),addr);
				return;
			}

			for (y = y0; y != y1; y += dy)
			{
				for (x = x0; x != x1; x += dx)
				{
					data = gfxdata[addr++];

					if ((data != 0xff) && (sx + x >= 0) && (sx + x < 400) && (sy + y >= 0) && (sy + y < 256))
						*BITMAP_ADDR16(bitmap, sy + y, sx + x) = data + ((tmaster_color & 7) << 8);
				}
			}
			break;

		case 0x20:							// solid fill
			data = tmaster_addr & 0xff;
			for (y = y0; y != y1; y += dy)
			{
				for (x = x0; x != x1; x += dx)
				{
					if ((sx + x >= 0) && (sx + x < 400) && (sy + y >= 0) && (sy + y < 256))
						*BITMAP_ADDR16(bitmap, sy + y, sx + x) = data + ((tmaster_color & 7) << 8);
				}
			}
			break;

	}
}

static WRITE16_HANDLER( tmaster_blitter_w )
{
	COMBINE_DATA( tmaster_regs + offset );
	switch (offset*2)
	{
		case 0x0e:
			tmaster_draw();
			cpunum_set_input_line(Machine, 0, 2, HOLD_LINE);
			break;
	}
}

static READ16_HANDLER( tmaster_blitter_r )
{
	return 0x0000;	// bit 7 = 1 -> blitter busy
}

/***************************************************************************

                                Memory Maps

***************************************************************************/

static READ16_HANDLER( tmaster_coins_r )
{
	return readinputportbytag("COIN")|(mame_rand(Machine)&0x0800);
}

static ADDRESS_MAP_START( tmaster_map, ADDRESS_SPACE_PROGRAM, 16 )
	AM_RANGE( 0x000000, 0x1fffff ) AM_ROM
	AM_RANGE( 0x200000, 0x27ffff ) AM_RAM
	AM_RANGE( 0x280000, 0x28ffff ) AM_RAM AM_BASE(&generic_nvram16) AM_SIZE(&generic_nvram_size)

	AM_RANGE( 0x300010, 0x300011 ) AM_READ( tmaster_coins_r )

	AM_RANGE( 0x300022, 0x300023 ) AM_READ ( tmaster_tscreen_x_hi_r )
	AM_RANGE( 0x300024, 0x300025 ) AM_WRITE( tmaster_tscreen_reset_w )
	AM_RANGE( 0x300026, 0x300027 ) AM_READ ( tmaster_tscreen_x_lo_r )
	AM_RANGE( 0x300028, 0x300029 ) AM_READ ( tmaster_tscreen_next_r )
	AM_RANGE( 0x300032, 0x300033 ) AM_READ ( tmaster_tscreen_y_hi_r )
	AM_RANGE( 0x300036, 0x300037 ) AM_READ ( tmaster_tscreen_y_lo_r )

	AM_RANGE( 0x300040, 0x300041 ) AM_WRITE( tmaster_oki_bank_w )

	AM_RANGE( 0x300070, 0x300071 ) AM_WRITE( tmaster_addr_w )

	AM_RANGE( 0x500000, 0x500011 ) AM_WRITE( tmaster_blitter_w ) AM_BASE( &tmaster_regs )
	AM_RANGE( 0x500010, 0x500011 ) AM_READ ( tmaster_blitter_r )

	AM_RANGE( 0x580000, 0x580001 ) AM_WRITE(MWA16_NOP) // often

	AM_RANGE( 0x600000, 0x600fff ) AM_READWRITE( MRA16_RAM, paletteram16_xBBBBBGGGGGRRRRR_word_w ) AM_BASE(&paletteram16) // looks like palettes, maybe

	AM_RANGE( 0x800000, 0x800001 ) AM_READWRITE( OKIM6295_status_0_lsb_r, OKIM6295_data_0_lsb_w )

	AM_RANGE( 0x800010, 0x800011 ) AM_WRITE( tmaster_color_w )
ADDRESS_MAP_END


/***************************************************************************

                                Input Ports

***************************************************************************/

static INPUT_PORTS_START( tmaster )
	PORT_START_TAG("TSCREEN_X")
	PORT_BIT( 0x01ff, 0x100, IPT_LIGHTGUN_X ) PORT_CROSSHAIR(X, 1, 0, 0) PORT_SENSITIVITY(35) PORT_KEYDELTA(3) PORT_PLAYER(1)
	PORT_BIT( 0x4000, IP_ACTIVE_HIGH, IPT_BUTTON1 )	PORT_IMPULSE(5)	// press
	PORT_BIT( 0x8000, IP_ACTIVE_HIGH, IPT_BUTTON2 )					// hover

	PORT_START_TAG("TSCREEN_Y")
	PORT_BIT( 0x0ff, 0x80, IPT_LIGHTGUN_Y ) PORT_CROSSHAIR(Y, 1, 0, 0) PORT_SENSITIVITY(35) PORT_KEYDELTA(3) PORT_PLAYER(1)

	PORT_START_TAG("COIN") // IN3
	PORT_BIT( 0x0001, IP_ACTIVE_LOW,  IPT_COIN1		)	PORT_IMPULSE(2)	// m. coin 1 (coin optics?)
	PORT_BIT( 0x0002, IP_ACTIVE_LOW,  IPT_COIN1		)	PORT_IMPULSE(4)	// m. coin 2 (coin optics?)
	PORT_BIT( 0x0004, IP_ACTIVE_LOW,  IPT_COIN1		)	PORT_IMPULSE(6)	// dbv input (coin optics?)
	PORT_BIT( 0x0008, IP_ACTIVE_LOW,  IPT_UNKNOWN	)
	PORT_BIT( 0x0010, IP_ACTIVE_LOW,  IPT_COIN1		)	// (service coin?)
	PORT_SERVICE_NO_TOGGLE( 0x0020, IP_ACTIVE_LOW	)
	PORT_BIT( 0x0040, IP_ACTIVE_LOW,  IPT_SERVICE1	)	// calibrate
	PORT_BIT( 0x0080, IP_ACTIVE_LOW,  IPT_UNKNOWN	)
	PORT_BIT( 0x0100, IP_ACTIVE_LOW,  IPT_UNKNOWN	)
	PORT_BIT( 0x0200, IP_ACTIVE_LOW,  IPT_UNKNOWN	)
	PORT_BIT( 0x0400, IP_ACTIVE_LOW,  IPT_UNKNOWN	)
	PORT_BIT( 0x0800, IP_ACTIVE_HIGH, IPT_SPECIAL	)
	PORT_BIT( 0x1000, IP_ACTIVE_LOW,  IPT_COIN1		)	// e. coin 1
	PORT_BIT( 0x2000, IP_ACTIVE_LOW,  IPT_COIN2		)	// e. coin 2
	PORT_BIT( 0x4000, IP_ACTIVE_LOW,  IPT_COIN3		)	// e. coin 3
	PORT_BIT( 0x8000, IP_ACTIVE_LOW,  IPT_COIN4		)	// e. coin 4
INPUT_PORTS_END


/***************************************************************************

                               Machine Drivers

***************************************************************************/

static MACHINE_RESET( tmaster )
{
	touchscreen = -1;
}

static INTERRUPT_GEN( tm3k_interrupt )
{
	switch (cpu_getiloops())
	{
		case 0:		cpunum_set_input_line(machine, 0, 2, HOLD_LINE);	break;
		case 1:		cpunum_set_input_line(machine, 0, 3, HOLD_LINE);	break;

		case 2:
		case 3:
		case 4:
		case 5:
		case 6:		cpunum_set_input_line_and_vector(machine, 0, 4, HOLD_LINE, 0x100/4);	break;	// touch screen controller

		default:	cpunum_set_input_line(machine, 0, 1, HOLD_LINE);	break;
	}
}

static MACHINE_DRIVER_START( tm3k )
	MDRV_CPU_ADD_TAG("main", M68000, 12000000)
	MDRV_CPU_PROGRAM_MAP(tmaster_map,0)
	MDRV_CPU_VBLANK_INT(tm3k_interrupt,2+5+20) // ??

	MDRV_SCREEN_REFRESH_RATE(60)
	MDRV_SCREEN_VBLANK_TIME(DEFAULT_60HZ_VBLANK_DURATION)

	MDRV_MACHINE_RESET(tmaster)
	MDRV_NVRAM_HANDLER(generic_0fill)

	MDRV_VIDEO_ATTRIBUTES(VIDEO_TYPE_RASTER)
	MDRV_SCREEN_FORMAT(BITMAP_FORMAT_INDEXED16)
	MDRV_SCREEN_SIZE(400, 256)
	MDRV_SCREEN_VISIBLE_AREA(0, 400-1, 0, 256-1)
	MDRV_PALETTE_LENGTH(0x800)

	MDRV_VIDEO_START(tmaster)
	MDRV_VIDEO_UPDATE(tmaster)

	MDRV_SPEAKER_STANDARD_MONO("mono")

	MDRV_SOUND_ADD_TAG("OKI",OKIM6295, 2000000)
	MDRV_SOUND_CONFIG(okim6295_interface_region_1_pin7high) // clock frequency & pin 7 not verified
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "mono", 1.0)
MACHINE_DRIVER_END


static INTERRUPT_GEN( tm_interrupt )
{
	switch (cpu_getiloops())
	{
		case 0:		cpunum_set_input_line(machine, 0, 2, HOLD_LINE);	break;
		case 1:		cpunum_set_input_line(machine, 0, 3, HOLD_LINE);	break;
		case 2:		cpunum_set_input_line_and_vector(machine, 0, 4, HOLD_LINE, 0x100/4);	break;	// touch screen controller
		default:	cpunum_set_input_line(machine, 0, 1, HOLD_LINE);	break;
	}
}

static MACHINE_DRIVER_START( tm )
	MDRV_IMPORT_FROM(tm3k)
	MDRV_CPU_MODIFY("main")
	MDRV_CPU_VBLANK_INT(tm_interrupt,3+20) // ??

	MDRV_SOUND_REPLACE("OKI",OKIM6295, 1122000)
	MDRV_SOUND_CONFIG(okim6295_interface_region_1_pin7high) // clock frequency & pin 7 not verified
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "mono", 1.0)
MACHINE_DRIVER_END


/***************************************************************************

                               ROMs Loading

***************************************************************************/

/***************************************************************************

Touch Master
1996, Midway

68000 @ 12MHz or 6MHz
u51 - u52 program code
u36 -> u39 gfx
u8 sound
OKI6295
NVSRAM DS1225a
Philips SCN68681
Xlinx XC3042a

Dumped by ANY

***************************************************************************/

ROM_START( tm )
	ROM_REGION( 0x200000, REGION_CPU1, 0 ) // 68000 Code
	ROM_LOAD16_BYTE( "tmaster.u51", 0x000000, 0x080000, CRC(edaa5874) SHA1(48b99bc7f5a6453def265967ca7d8eefdf9dc97b) )
	ROM_LOAD16_BYTE( "tmaster.u52", 0x000001, 0x080000, CRC(e9fd30fc) SHA1(d91ea05d5f574603883336729fb9df705688945d) )

	ROM_REGION( 0x400000, REGION_GFX1, ROMREGION_ERASE )	// Blitter gfx
	ROM_LOAD16_BYTE( "tmaster.u38", 0x100000, 0x080000, CRC(68885ef6) SHA1(010602b59c33c3e490491a296ddaf8952e315b83) )
	ROM_LOAD16_BYTE( "tmaster.u36", 0x100001, 0x080000, CRC(204096ec) SHA1(9239923b7eedb6003c63ef2e8ff224edee657bbc) )
	// unused gap
	ROM_LOAD16_BYTE( "tmaster.u39", 0x300000, 0x080000, CRC(cbb716cb) SHA1(4e8d8f6cbfb25a8161ff8fe7505d6b209650dd2b) )
	ROM_LOAD16_BYTE( "tmaster.u37", 0x300001, 0x080000, CRC(e0b6a9f7) SHA1(7e057ca87833c682e5be03668469259bbdefbf20) )

	ROM_REGION( 0x100000, REGION_SOUND1, 0 ) // Samples
	ROM_LOAD( "tmaster.u8", 0x40000, 0x040000, CRC(f39ad4cf) SHA1(9bcb9a5dd3636d6541eeb3e737c7253ab0ed4e8d) )
	ROM_CONTINUE(           0xc0000, 0x040000 )
ROM_END

/***************************************************************************

Touchmaster 3000
by Midway
touchscreen game
Dumped BY: N?Z!

All chips are SGS 27C801
---------------------------

Name_Board Location        Version               Use                      Checksum
-----------------------------------------------------------------------------------
TM3K_u8.bin                5.0  Audio Program & sounds          64d5
TM3K_u51.bin               5.01 Game Program & Cpu instructions 0c6c
TM3K_u52.bin               5.01 Game Program & Cpu instructions b2d8
TM3K_u36.bin               5.0  Video Images & Graphics         54f1
TM3K_u37.bin               5.0  Video Images & Graphics         4856
TM3K_u38.bin               5.0  Video Images & Graphics         5493
TM3K_u39.bin               5.0  Video Images & Graphics         6029
TM3K_u40.bin               5.0  Video Images & Graphics         ccb4
TM3K_u41.bin               5.0  Video Images & Graphics         54a7
u62 (NOT INCLUDED)         N/A  Battery Memory Module           N/A
J12 (NOT INCLUDED)         N/A  Security Key(not required for this Version)
-----------------------------------------------------------------------------------

SCN68681c1n40
xc3042A      www.xilinx.com

***************************************************************************/

ROM_START( tm3k )
	ROM_REGION( 0x200000, REGION_CPU1, 0 ) // 68000 Code
	ROM_LOAD16_BYTE( "tm3k_u52.bin", 0x000001, 0x100000, CRC(8c6a0db7) SHA1(6b0eae60ea471cd8c4001749ac2677d8d4532567) )
	ROM_LOAD16_BYTE( "tm3k_u51.bin", 0x000000, 0x100000, CRC(c9522279) SHA1(e613b791f831271722f05b7e96c35519fa9fc174) )

	ROM_REGION( 0x600000, REGION_GFX1, 0 )	// Blitter gfx
	ROM_LOAD16_BYTE( "tm3k_u38.bin", 0x000000, 0x100000, CRC(a6683899) SHA1(d05024390917cdb1871d030996da8e1eb6460918) )
	ROM_LOAD16_BYTE( "tm3k_u36.bin", 0x000001, 0x100000, CRC(7bde520d) SHA1(77750b689e2f0d47804042456e54bbd9c28deeac) )
	ROM_LOAD16_BYTE( "tm3k_u39.bin", 0x200000, 0x100000, CRC(206b56a6) SHA1(09e5e05bffd0a09abd24d668e2c59b56f2c79134) )
	ROM_LOAD16_BYTE( "tm3k_u37.bin", 0x200001, 0x100000, CRC(18f50eb3) SHA1(a7c9d3b24b5fd110380ec87d9200d55cad473efc) )
	ROM_LOAD16_BYTE( "tm3k_u41.bin", 0x400000, 0x100000, BAD_DUMP CRC(74a36bca) SHA1(7ad594daa156dea40a25b390f26c2fd0550e66ff) )
	ROM_LOAD16_BYTE( "tm3k_u40.bin", 0x400001, 0x100000, CRC(353df7ca) SHA1(d6c5d5449af6b6a3acee219778583904c5b554b4) )

	ROM_REGION( 0x100000, REGION_SOUND1, 0 ) // Samples
	ROM_LOAD( "tm3k_u8.bin", 0x00000, 0x100000, CRC(d0ae33c1) SHA1(a079def9a086a091fcc4493a44fec756d2470415) )
ROM_END

/***************************************************************************

Touchmaster 4000
by Midway
touchscreen game
Dumped BY: N?Z!

All chips are SGS 27C801
---------------------------

Name_Board Location        Version               Use                      Checksum
-----------------------------------------------------------------------------------
TM4K_u8.bin                6.0  Audio Program & sounds          DE0B
TM4K_u51.bin               6.02 Game Program & Cpu instructions FEA0
TM4K_u52.bin               6.02 Game Program & Cpu instructions 9A71
TM4K_u36.bin               6.0  Video Images & Graphics         54f1
TM4K_u37.bin               6.0  Video Images & Graphics         609E
TM4K_u38.bin               6.0  Video Images & Graphics         5493
TM4K_u39.bin               6.0  Video Images & Graphics         CB90
TM4K_u40.bin               6.0  Video Images & Graphics         208A
TM4K_u41.bin               6.0  Video Images & Graphics         385D
u62 (NOT INCLUDED)         N/A  Battery Memory Module           N/A
J12 (NOT INCLUDED)         N/A  Security Key(required for this Version)
-----------------------------------------------------------------------------------

SCN68681c1n40
xc3042A      www.xilinx.com

***************************************************************************/

ROM_START( tm4k )
	ROM_REGION( 0x200000, REGION_CPU1, 0 ) // 68000 Code
	ROM_LOAD16_BYTE( "tm4k_u51.bin", 0x000000, 0x100000, CRC(3d8d7848) SHA1(31638f23cdd5e6cfbb2270e953f84fe1bd437950) )
	ROM_LOAD16_BYTE( "tm4k_u52.bin", 0x000001, 0x100000, CRC(6d412871) SHA1(ae27c7723b292daf6682c53bafac22e4a3cd1ece) )

	ROM_REGION( 0x600000, REGION_GFX1, 0 )	// Blitter gfx
	ROM_LOAD16_BYTE( "tm4k_u38.bin", 0x000000, 0x100000, CRC(a6683899) SHA1(d05024390917cdb1871d030996da8e1eb6460918) )
	ROM_LOAD16_BYTE( "tm4k_u36.bin", 0x000001, 0x100000, CRC(7bde520d) SHA1(77750b689e2f0d47804042456e54bbd9c28deeac) )
	ROM_LOAD16_BYTE( "tm4k_u39.bin", 0x200000, 0x100000, CRC(bac88cfb) SHA1(26ed169296b890c5f5b50c418c15299355a6592f) )
	ROM_LOAD16_BYTE( "tm4k_u37.bin", 0x200001, 0x100000, CRC(bf49fafa) SHA1(b400667bf654dc9cd01a85c8b99670459400fd60) )
	ROM_LOAD16_BYTE( "tm4k_u41.bin", 0x400000, 0x100000, CRC(e97edb1e) SHA1(75510676cf1692ad03efd4ccd57d25af1cc8ef2a) )
	ROM_LOAD16_BYTE( "tm4k_u40.bin", 0x400001, 0x100000, CRC(f6771a09) SHA1(74f71d5e910006c83a38170f24aa811c38a3e020) )

	ROM_REGION( 0x100000, REGION_SOUND1, 0 ) // Samples
	ROM_LOAD( "tm4k_u8.bin", 0x00000, 0x100000, CRC(48c3782b) SHA1(bfe105ddbde8bbbd84665dfdd565d6d41926834a) )
ROM_END

static DRIVER_INIT( tm3k )
{
	// try this if you need to calibrate
#if 0
	UINT16 *ROM = (UINT16 *)memory_region( REGION_CPU1 );
	// tscreen test
	ROM[0x75e3c/2] = 0x4ef9;
	ROM[0x75e3e/2] = 0x0007;
	ROM[0x75e40/2] = 0x5e4a;

	// tscreen test
	ROM[0x765ca/2] = 0x7001;
#endif
}

static DRIVER_INIT( tm4k )
{
	UINT16 *ROM = (UINT16 *)memory_region( REGION_CPU1 );

	// protection
	ROM[0x83476/2] = 0x4e75;

	ROM[0x8342C/2] = 0x601a;
	ROM[0x8346C/2] = 0x6002;
}

GAME( 1996, tm,   0, tm,   tmaster, 0,    ROT0, "Midway", "Touchmaster",              GAME_NOT_WORKING )
GAME( 1997, tm3k, 0, tm3k, tmaster, tm3k, ROT0, "Midway", "Touchmaster 3000 (v5.01)", GAME_NOT_WORKING | GAME_IMPERFECT_GRAPHICS)
GAME( 1998, tm4k, 0, tm3k, tmaster, tm4k, ROT0, "Midway", "Touchmaster 4000 (v6.02)", GAME_NOT_WORKING )
