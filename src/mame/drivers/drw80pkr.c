/**********************************************************************************


    DRAW 80 POKER

    Driver by Jim Stolis.


    --- Technical Notes ---

    Name:    Draw 80 Poker
    Company: IGT - International Gaming Technology
    Year:    1983

    Hardware:

    CPU =  INTEL 8039       ; I8039 compatible
    VIDEO = SYS 6545        ; CRTC6845 compatible
    SND =  AY-3-8912        ; AY8910 compatible

    History:

    This is one of the first video machines produced by IGT.  Originally, the
    company was called SIRCOMA and was founded in 1979.  It became a public
    company in 1981 and changed its name to IGT.  The motherboard has a date
    of 1982 (c) IGT, but the game rom labels and backglass are dated 1983.

***********************************************************************************/
#include "driver.h"
#include "sound/ay8910.h"
#include "cpu/mcs48/mcs48.h"

#define CPU_CLOCK XTAL_7_8643MHz

static tilemap *bg_tilemap;

static UINT8 p1, p2, prog, bus;

static UINT8 *pkr_io_ram;
static UINT8 active_bank = 0;

static UINT16 video_ram[0x0400];
static UINT8 color_ram[0x0400];


/*****************
* Write Handlers *
******************/

static WRITE8_HANDLER( p1_w )
{
	p1 = data;
}

static WRITE8_HANDLER( p2_w )
{
	p2 = data;
}

static WRITE8_HANDLER( prog_w )
{
	prog = data;

	// Bankswitch Program Memory
	if (prog == 0x01)
	{
		active_bank++;
		if (active_bank == 2)
			active_bank = 0;

		memory_set_bank(space->machine, 1, active_bank);
	}
}

static WRITE8_HANDLER( bus_w )
{
	bus = data;
}

static WRITE8_HANDLER( drw80pkr_io_w )
{
	static UINT16 n_offs;
	static UINT16 n_data;
	static UINT16 add;


	if (p2 == 0x3f) // write cg address
	{
		if (p1 == 0xbf || p1 == 0x3f)
		{
			n_data = data;
		}
		if (p1 == 0x7f && data != 0x0f)
		{
			n_data = data + 0x100;
		}

		add = ((p1 & 0xc0) << 2);
		if (p1 == 0x3f && offset >= 0xf0)
		{
			add = 0x200;
		}
		n_offs = (add) + (0xff-offset);

		video_ram[n_offs] = n_data;

		tilemap_mark_tile_dirty(bg_tilemap, n_offs);
	}

	if (p2 == 0x7f) // write palette
	{
		n_offs = ((p1 & 0xc0) << 2 ) + (0xff-offset);

		color_ram[n_offs] = 0;//data & 0x0f;

		if (data < 0x10)
			video_ram[n_offs] = video_ram[n_offs] + 0x100;

		tilemap_mark_tile_dirty(bg_tilemap, n_offs);
	}

	// ay8910 control port
	if (p1 == 0xfc && p2 == 0xff && offset == 0x00)
		ay8910_address_w(devtag_get_device(space->machine, "ay"), 0, data);

	// ay8910_write_port_0_w
	if (p1 == 0xfe && p2 == 0xff && offset == 0x00)
		ay8910_data_w(devtag_get_device(space->machine, "ay"), 0, data);

	// CRTC Register
	// R0 = 0x1f(31)    Horizontal Total
	// R1 = 0x18(24)    Horizontal Displayed
	// R2 = 0x1a(26)    Horizontal Sync Position
	// R3 = 0x34(52)    HSYNC/VSYNC Widths
	// R4 = 0x1f(31)    Vertical Total
	// R5 = 0x01(01)    Vertical Total Adjust
	// R6 = 0x1b(27)    Vertical Displayed
	// R7 = 0x1c(28)    Vertical Sync Position
	// R8 = 0x10        Mode Control
	//                  Non-interlace
	//                  Straight Binary - Ram Addressing
	//                  Shared Memory - Ram Access
	//                  Delay Display Enable one character time
	//                  No Delay Cursor Skew
	// R9 = 0x07(07)    Scan Line
	// R10 = 0x00       Cursor Start
	// R11 = 0x00       Cursor End
	// R12 = 0x00       Display Start Address (High)
	// R13 = 0x00       Display Start Address (Low)
    //if (p1 == 0xff && p2 == 0xc7)

	// CRTC Address
    //if (p1 == 0xff && p2 == 0xd7)

	pkr_io_ram[offset] = data;
}

/****************
* Read Handlers *
****************/

static READ8_HANDLER( p1_r )
{
    return p1;
}

static READ8_HANDLER( p2_r )
{
    return p2;
}

static READ8_HANDLER( bus_r )
{
    return bus;
}

static READ8_HANDLER( drw80pkr_io_r )
{
    return pkr_io_ram[offset];
}


/****************************
* Video/Character functions *
****************************/

static TILE_GET_INFO( get_bg_tile_info )
{
	int color = color_ram[tile_index];
	int code = video_ram[tile_index];

	SET_TILE_INFO(0, code, color, 0);
}

static VIDEO_START( drw80pkr )
{
	bg_tilemap = tilemap_create(machine, get_bg_tile_info, tilemap_scan_rows, 8, 8, 24, 27);
}

static VIDEO_UPDATE( drw80pkr )
{
	tilemap_draw(bitmap, cliprect, bg_tilemap, 0, 0);

	return 0;
}

static PALETTE_INIT( drw80pkr )
{
/*  prom bits
    7654 3210
    ---- -xxx   red component.
    --xx x---   green component.
    xx-- ----   blue component.
*/
	int i;

	for (i = 0;i < machine->config->total_colors;i++)
	{
		int bit0, bit1, bit2, r, g, b;

		/* red component */
		bit0 = (~color_prom[i] >> 0) & 0x01;
		bit1 = (~color_prom[i] >> 1) & 0x01;
		bit2 = (~color_prom[i] >> 2) & 0x01;
		r = 0x21 * bit2 + 0x47 * bit1 + 0x97 * bit0;

		/* green component */
		bit0 = (~color_prom[i] >> 3) & 0x01;
		bit1 = (~color_prom[i] >> 4) & 0x01;
		bit2 = (~color_prom[i] >> 5) & 0x01;
		g = 0x21 * bit2 + 0x47 * bit1 + 0x97 * bit0;

		/* blue component */
		bit0 = (~color_prom[i] >> 6) & 0x01;
		bit1 = (~color_prom[i] >> 7) & 0x01;
		bit2 = 0;
		b = 0x21 * bit2 + 0x47 * bit1 + 0x97 * bit0;

		palette_set_color(machine, i, MAKE_RGB(r, g, b));
	}
}


/*************************
*    Graphics Layouts    *
*************************/

static const gfx_layout charlayout =
{
	8,8,    /* 8x8 characters */
	RGN_FRAC(1,2), /* 512 characters */
	2,  /* 2 bitplanes */
	{ RGN_FRAC(1,2), 0 },
	{ STEP8(0,1) },
	{ STEP8(0,8) },
	8*8
};


/******************************
* Graphics Decode Information *
******************************/

static GFXDECODE_START( drw80pkr )
	GFXDECODE_ENTRY( "gfx1", 0x00000, charlayout, 0, 16 )
GFXDECODE_END


/**************
* Driver Init *
***************/

static DRIVER_INIT( drw80pkr )
{
	memory_configure_bank(machine, 1, 0, 2, memory_region(machine, "maincpu"), 0x1000);
}


/*************************
* Memory map information *
*************************/

static ADDRESS_MAP_START( drw80pkr_map, ADDRESS_SPACE_PROGRAM, 8 )
	AM_RANGE(0x0000, 0x1fff) AM_ROMBANK(1)
ADDRESS_MAP_END

static ADDRESS_MAP_START( drw80pkr_io_map, ADDRESS_SPACE_IO, 8 )
	AM_RANGE(0x00, 0xff) AM_READWRITE(drw80pkr_io_r, drw80pkr_io_w) AM_BASE(&pkr_io_ram)
	AM_RANGE(MCS48_PORT_T1,   MCS48_PORT_T1) AM_RAM
	AM_RANGE(MCS48_PORT_P1,   MCS48_PORT_P1) AM_READWRITE(p1_r, p1_w)
	AM_RANGE(MCS48_PORT_P2,   MCS48_PORT_P2) AM_READWRITE(p2_r, p2_w)
    AM_RANGE(MCS48_PORT_PROG, MCS48_PORT_PROG) AM_RAM_WRITE(prog_w)
    AM_RANGE(MCS48_PORT_BUS,  MCS48_PORT_BUS) AM_READWRITE(bus_r, bus_w)
ADDRESS_MAP_END

/*************************
*      Input ports       *
*************************/

static INPUT_PORTS_START( drw80pkr )
	// Unknown at this time
INPUT_PORTS_END

/*************************
*     Machine Driver     *
*************************/

static MACHINE_DRIVER_START( drw80pkr )
	// basic machine hardware
	MDRV_CPU_ADD("maincpu", I8039, CPU_CLOCK)
	MDRV_CPU_PROGRAM_MAP(drw80pkr_map)
    MDRV_CPU_IO_MAP(drw80pkr_io_map)
	MDRV_CPU_VBLANK_INT("screen", irq0_line_hold)

	// video hardware

	MDRV_SCREEN_ADD("screen", RASTER)
	MDRV_SCREEN_REFRESH_RATE(60)
	MDRV_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MDRV_SCREEN_FORMAT(BITMAP_FORMAT_INDEXED16)
	MDRV_SCREEN_SIZE((31+1)*8, (31+1)*8)
	MDRV_SCREEN_VISIBLE_AREA(0*8, 24*8-1, 0*8, 27*8-1)

	MDRV_GFXDECODE(drw80pkr)
	MDRV_PALETTE_LENGTH(16*16)

	MDRV_PALETTE_INIT(drw80pkr)
	MDRV_VIDEO_START(drw80pkr)
	MDRV_VIDEO_UPDATE(drw80pkr)

	// sound hardware
	MDRV_SPEAKER_STANDARD_MONO("mono")

	MDRV_SOUND_ADD("ay", AY8912, 20000000/12)
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.75)
MACHINE_DRIVER_END

/*************************
*        Rom Load        *
*************************/

ROM_START( drw80pkr )
	ROM_REGION( 0x2000, "maincpu", 0 )
	ROM_LOAD( "pm0.u81",   0x0000, 0x1000, CRC(0f3e97d2) SHA1(aa9e4015246284f32435d7320de667e075412e5b) )
	ROM_LOAD( "pm1.u82",   0x1000, 0x1000, CRC(5a6ad467) SHA1(0128bd70b65244a0f68031d5f451bf115eeb7609) )


	ROM_REGION( 0x002000, "gfx1", 0 )
	ROM_LOAD( "cg0-a.u74",	 0x0000, 0x1000, CRC(97f5eb92) SHA1(f6c7bb42ccef8a78e8d56104ad942ae5b8e5b0df) )
	ROM_LOAD( "cg1-a.u76",	 0x1000, 0x1000, CRC(2a3a750d) SHA1(db6183d11b2865b011c3748dc472cf5858dde78f) )

	ROM_REGION( 0x100, "proms", 0 ) // WRONG CAP
	ROM_LOAD( "cap13.u92", 0x0000, 0x0100, CRC(42d6e973) SHA1(5c2983d5c80333ca45033070a2296fe58c339ee1) )
ROM_END


/*************************
*      Game Drivers      *
*************************/

/*    YEAR  NAME      PARENT  MACHINE   INPUT     INIT      ROT    COMPANY                                  FULLNAME              FLAGS   */
GAME( 1983, drw80pkr, 0,      drw80pkr, drw80pkr, drw80pkr, ROT0,  "IGT - International Gaming Technology", "Draw 80 Poker",      GAME_NOT_WORKING )
