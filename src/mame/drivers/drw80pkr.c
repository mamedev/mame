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

static tilemap *bg_tilemap;

static UINT8 p1, p2, prog, bus;

static UINT8 *pkr_cmos_ram;


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
	/* this is written via an out to port 4, but unless there is an 8243 port expander,
       it is more likely that the port 4 output is used to toggle the PROG line; see
       videopkr */
	prog = data;
}

static WRITE8_HANDLER( bus_w )
{
	bus = data;
}

static WRITE8_HANDLER( drw80pkr_cmos_w )
{
    //if (p2 == 0xc7) CRTC Register
    //if (p2 == 0xd7) CRTC Address
    pkr_cmos_ram[offset] = data;
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

static READ8_HANDLER( drw80pkr_cmos_r )
{
    return pkr_cmos_ram[offset];
}

/****************************
* Video/Character functions *
****************************/

static TILE_GET_INFO( get_bg_tile_info )
{
    int vr = 0;
	int code = vr;
	int color = 0;

	SET_TILE_INFO(0, code, color, 0);
}

static VIDEO_START( drw80pkr )
{
	bg_tilemap = tilemap_create(machine, get_bg_tile_info, tilemap_scan_rows, 8, 8, 40, 25);
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
	0x200, /* 512 characters */
	2,  /* 2 bitplanes */
	{ 0x200*8*8*1, 0x200*8*8*0 }, /* bitplane offsets */
	{ 0, 1, 2, 3, 4, 5, 6, 7 },
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8 },
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

}


/*************************
* Memory map information *
*************************/

static ADDRESS_MAP_START( drw80pkr_map, ADDRESS_SPACE_PROGRAM, 8 )
	AM_RANGE(0x0000, 0x1fff) AM_READWRITE(SMH_ROM, SMH_ROM)
ADDRESS_MAP_END

static ADDRESS_MAP_START( drw80pkr_io_map, ADDRESS_SPACE_IO, 8 )
	AM_RANGE(0x00, 0xff) AM_READWRITE(drw80pkr_cmos_r, drw80pkr_cmos_w) AM_BASE(&pkr_cmos_ram)
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
	MDRV_CPU_ADD("main", I8039, 7864300)
	MDRV_CPU_PROGRAM_MAP(drw80pkr_map, 0)
    MDRV_CPU_IO_MAP(drw80pkr_io_map, 0)
	MDRV_CPU_VBLANK_INT("main", irq0_line_hold)

	// video hardware

	MDRV_SCREEN_ADD("main", RASTER)
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
	MDRV_SOUND_ADD("ay", AY8912, 20000000/12)
	MDRV_SPEAKER_STANDARD_MONO("mono")
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.75)
MACHINE_DRIVER_END

/*************************
*        Rom Load        *
*************************/

ROM_START( drw80pkr )
	ROM_REGION( 0x2000, "main", 0 )
	ROM_LOAD( "pm0.u81",   0x0000, 0x1000, CRC(0f3e97d2) SHA1(aa9e4015246284f32435d7320de667e075412e5b) )
    ROM_LOAD( "pm1.u82",   0x1000, 0x1000, CRC(5a6ad467) SHA1(0128bd70b65244a0f68031d5f451bf115eeb7609) )

	ROM_REGION( 0x100000, "gfx1", ROMREGION_DISPOSE )
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
