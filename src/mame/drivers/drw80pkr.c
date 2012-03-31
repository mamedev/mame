/**********************************************************************************


    DRAW 80 POKER

    Driver by Jim Stolis.


    --- Technical Notes ---

    Name:    Draw 80 Poker
    Company: IGT - International Gaming Technology
    Year:    1982

    Hardware:

    CPU =  INTEL 8039       ; I8039 compatible
    VIDEO = SYS 6545        ; CRTC6845 compatible
    SND =  AY-3-8912        ; AY8910 compatible

    History:

    This is one of the first video machines produced by IGT.  Originally, the
    company was called SIRCOMA and was founded in 1979.  It became a public
    company in 1981 and changed its name to IGT.

***********************************************************************************/
#include "emu.h"
#include "machine/nvram.h"
#include "sound/ay8910.h"
#include "cpu/mcs48/mcs48.h"


class drw80pkr_state : public driver_device
{
public:
	drw80pkr_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag) { }

	tilemap_t *m_bg_tilemap;
	UINT8 m_t0;
	UINT8 m_t1;
	UINT8 m_p0;
	UINT8 m_p1;
	UINT8 m_p2;
	UINT8 m_prog;
	UINT8 m_bus;
	UINT8 m_attract_mode;
	UINT8 m_active_bank;
	UINT8 m_pkr_io_ram[0x100];
	UINT16 m_video_ram[0x0400];
	UINT8 m_color_ram[0x0400];
};


#define CPU_CLOCK			XTAL_8MHz
#define DATA_NVRAM_SIZE     0x100


static MACHINE_START( drw80pkr )
{
	drw80pkr_state *state = machine.driver_data<drw80pkr_state>();
	machine.device<nvram_device>("nvram")->set_base(state->m_pkr_io_ram, sizeof(state->m_pkr_io_ram));
}

/*****************
* Write Handlers *
******************/

static WRITE8_HANDLER( t0_w )
{
	drw80pkr_state *state = space->machine().driver_data<drw80pkr_state>();
	state->m_t0 = data;
}

static WRITE8_HANDLER( t1_w )
{
	drw80pkr_state *state = space->machine().driver_data<drw80pkr_state>();
	state->m_t1 = data;
}

static WRITE8_HANDLER( p0_w )
{
	drw80pkr_state *state = space->machine().driver_data<drw80pkr_state>();
	state->m_p0 = data;
}

static WRITE8_HANDLER( p1_w )
{
	drw80pkr_state *state = space->machine().driver_data<drw80pkr_state>();
	state->m_p1 = data;
}

static WRITE8_HANDLER( p2_w )
{
	drw80pkr_state *state = space->machine().driver_data<drw80pkr_state>();
	state->m_p2 = data;
}

static WRITE8_HANDLER( prog_w )
{
	drw80pkr_state *state = space->machine().driver_data<drw80pkr_state>();
	state->m_prog = data;

	// Bankswitch Program Memory
	if (state->m_prog == 0x01)
	{
		state->m_active_bank = state->m_active_bank ^ 0x01;

		memory_set_bank(space->machine(), "bank1", state->m_active_bank);
	}
}

static WRITE8_HANDLER( bus_w )
{
	drw80pkr_state *state = space->machine().driver_data<drw80pkr_state>();
	state->m_bus = data;
}

static WRITE8_HANDLER( drw80pkr_io_w )
{
	drw80pkr_state *state = space->machine().driver_data<drw80pkr_state>();
	UINT16 n_offs;

	if (state->m_p2 == 0x3f || state->m_p2 == 0x7f)
	{
		n_offs = ((state->m_p1 & 0xc0) << 2 ) + offset;

		if (state->m_p2 == 0x3f)
		{
			state->m_video_ram[n_offs] = data; // low address
		} else {
			state->m_color_ram[n_offs] = data & 0x0f; // color palette
			state->m_video_ram[n_offs] += ((data & 0xf0) << 4 ); // high address
		}

		state->m_bg_tilemap->mark_tile_dirty(n_offs);
	}

	if (state->m_p2 == 0xc7)
	{
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
	}

	if (state->m_p2 == 0xd7)
	{
		// CRTC Address
	}

	if (state->m_p2 == 0xfb) {
		state->m_pkr_io_ram[offset] = data;
	}

	if (state->m_p2 == 0xff)
	{
		if (state->m_p1 == 0xdf)
		{
			state->m_attract_mode = data; // Latch this for use in input reads (0x01 = attract mode, 0x00 = game in progress)
		}

		if (state->m_p1 == 0xdb || state->m_p1 == 0xef || state->m_p1 == 0xf7 || state->m_p1 == 0xfb)
		{
			// unknown, most likely lamps, meters, hopper etc.
		}

		// ay8910 control port
		if (state->m_p1 == 0xfc)
			ay8910_address_w(space->machine().device("aysnd"), 0, data);

		// ay8910_write_port_0_w
		if (state->m_p1 == 0xfe)
			ay8910_data_w(space->machine().device("aysnd"), 0, data);
	}
}

/****************
* Read Handlers *
****************/

static READ8_HANDLER( t0_r )
{
	drw80pkr_state *state = space->machine().driver_data<drw80pkr_state>();
    return state->m_t0;
}

static READ8_HANDLER( t1_r )
{
	drw80pkr_state *state = space->machine().driver_data<drw80pkr_state>();
    return state->m_t1;
}

static READ8_HANDLER( p0_r )
{
	drw80pkr_state *state = space->machine().driver_data<drw80pkr_state>();
    return state->m_p0;
}

static READ8_HANDLER( p1_r )
{
	drw80pkr_state *state = space->machine().driver_data<drw80pkr_state>();
    return state->m_p1;
}

static READ8_HANDLER( p2_r )
{
	drw80pkr_state *state = space->machine().driver_data<drw80pkr_state>();
    return state->m_p2;
}

static READ8_HANDLER( bus_r )
{
	drw80pkr_state *state = space->machine().driver_data<drw80pkr_state>();
    return state->m_bus;
}

static READ8_HANDLER( drw80pkr_io_r )
{
	drw80pkr_state *state = space->machine().driver_data<drw80pkr_state>();
	UINT8 ret;
	UINT16 kbdin;

	ret = 0x00;

	if (state->m_p2 == 0x3b)
	{
		// unknown
	}

	if (state->m_p2 == 0x7b)
	{
		ret = state->m_pkr_io_ram[offset];
	}

	if (state->m_p2 == 0xf7)
	{
		// unknown
	}

	if (state->m_p2 == 0xfb)
	{
		ret = state->m_pkr_io_ram[offset];
	}

	if (state->m_p2 == 0xff)
	{
		if (state->m_p1 == 0x5f || state->m_p1 == 0x9f || state->m_p1 == 0xdb)
		{
			// unknown
		}

		if (state->m_p1 == 0xfe)
		{
			// Dip switches tied to sound chip
			//
			// TODO: Unknown switch positions, but found the following flipping bits:
			//      SW.? = Double Up Option
			//      SW.? = Coin Denomination
			//      SW.4 = Payout Type (0=cash, 1=credit)
			//      SW.? = Use Joker in Deck
			//
			ret = 0x77; // double-up with credit payout
		}

		if ((state->m_attract_mode == 0x01 && state->m_p1 == 0xef) || state->m_p1 == 0xf7)
		{

			// TODO: Get Input Port Values
			kbdin = ((input_port_read(space->machine(), "IN1") & 0xaf ) << 8) + input_port_read(space->machine(), "IN0");

			switch (kbdin)
			{
				// The following is very incorrect, but does allow you to
				// play slightly with very messed up hold buttons etc.
				//
				// Open/Close the door with 'O'
				// Press '5' (twice) with door open to play credit
				// Press '1' to draw/deal
				//
				case 0x0000: ret = 0x00; break;
				case 0x0001: ret = 0x01; break;	/* Door */
				case 0x4000: ret = 0x00; break;
				case 0x8000: ret = 0x00; break;	/* Hand Pay */
				case 0x0002: ret = 0x00; break;	/* Books */
				case 0x0004: ret = 0x0e; break;	/* Coin In */
				case 0x0008: ret = 0x0d; break;	/* Start */
				case 0x0010: ret = 0x00; break;	/* Discard */
				case 0x0020: ret = 0x00; break;	/* Cancel */
				case 0x0040: ret = 0x01; break;	/* Hold 1 */
				case 0x0080: ret = 0x02; break;	/* Hold 2 */
				case 0x0100: ret = 0x03; break;	/* Hold 3 */
				case 0x0200: ret = 0x04; break;	/* Hold 4 */
				case 0x0400: ret = 0x05; break;	/* Hold 5 */
				case 0x0800: ret = 0x00; break;	/* Bet */
			}
		}
	}

    return ret;
}


/****************************
* Video/Character functions *
****************************/

static TILE_GET_INFO( get_bg_tile_info )
{
	drw80pkr_state *state = machine.driver_data<drw80pkr_state>();
	int color = state->m_color_ram[tile_index];
	int code = state->m_video_ram[tile_index];

	SET_TILE_INFO(0, code, color, 0);
}

static VIDEO_START( drw80pkr )
{
	drw80pkr_state *state = machine.driver_data<drw80pkr_state>();
	state->m_bg_tilemap = tilemap_create(machine, get_bg_tile_info, tilemap_scan_rows, 8, 8, 24, 27);
}

static SCREEN_UPDATE_IND16( drw80pkr )
{
	drw80pkr_state *state = screen.machine().driver_data<drw80pkr_state>();
	state->m_bg_tilemap->draw(bitmap, cliprect, 0, 0);

	return 0;
}

static PALETTE_INIT( drw80pkr )
{
	int j;

	for (j = 0; j < machine.total_colors(); j++)
	{
		int r, g, b, tr, tg, tb, i;

		i = (color_prom[j] >> 3) & 0x01;
		//i = color_prom[j];

		/* red component */
		tr = 0xf0 - (0xf0 * ((color_prom[j] >> 0) & 0x01));
		r = tr - (i * (tr / 5));

		/* green component */
		tg = 0xf0 - (0xf0 * ((color_prom[j] >> 1) & 0x01));
		g = tg - (i * (tg / 5));

		/* blue component */
		tb = 0xf0 - (0xf0 * ((color_prom[j] >> 2) & 0x01));
		b = tb - (i * (tb / 5));

		palette_set_color(machine, j, MAKE_RGB(r, g, b));
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
	{ 0, RGN_FRAC(1,2) },
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
	memory_configure_bank(machine, "bank1", 0, 2, machine.region("maincpu")->base(), 0x1000);
}


/*************************
* Memory map information *
*************************/

static ADDRESS_MAP_START( drw80pkr_map, AS_PROGRAM, 8, drw80pkr_state )
	AM_RANGE(0x0000, 0x1fff) AM_ROMBANK("bank1")
ADDRESS_MAP_END

static ADDRESS_MAP_START( drw80pkr_io_map, AS_IO, 8, drw80pkr_state )
	AM_RANGE(0x00, 0xff) AM_READWRITE(drw80pkr_io_r, drw80pkr_io_w)
	AM_RANGE(MCS48_PORT_T0,   MCS48_PORT_T0) AM_READWRITE(t0_r, t0_w)
	AM_RANGE(MCS48_PORT_T1,   MCS48_PORT_T1) AM_READWRITE(t1_r, t1_w)
	AM_RANGE(MCS48_PORT_P0,   MCS48_PORT_P0) AM_READWRITE(p0_r, p0_w)
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
	// These are temporary buttons for testing only
	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_GAMBLE_DOOR ) PORT_TOGGLE
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_GAMBLE_BOOK ) PORT_NAME("Books")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_COIN1 ) PORT_IMPULSE(2)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_START1 ) PORT_NAME("Start")
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("Discard") PORT_CODE(KEYCODE_2)
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_POKER_CANCEL )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_POKER_HOLD1 )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_POKER_HOLD2 )

	PORT_START("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_POKER_HOLD3 )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_POKER_HOLD4 )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_POKER_HOLD5 )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("Hopper") PORT_TOGGLE PORT_CODE(KEYCODE_H)
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_GAMBLE_PAYOUT)
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNUSED )
INPUT_PORTS_END

/*************************
*     Machine Driver     *
*************************/

static MACHINE_CONFIG_START( drw80pkr, drw80pkr_state )
	// basic machine hardware
	MCFG_CPU_ADD("maincpu", I8039, CPU_CLOCK)
	MCFG_CPU_PROGRAM_MAP(drw80pkr_map)
    MCFG_CPU_IO_MAP(drw80pkr_io_map)
	MCFG_CPU_VBLANK_INT("screen", irq0_line_hold)

	MCFG_MACHINE_START(drw80pkr)

	// video hardware

	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MCFG_SCREEN_SIZE((31+1)*8, (31+1)*8)
	MCFG_SCREEN_VISIBLE_AREA(0*8, 24*8-1, 0*8, 27*8-1)
	MCFG_SCREEN_UPDATE_STATIC(drw80pkr)

	MCFG_GFXDECODE(drw80pkr)
	MCFG_PALETTE_LENGTH(16*16)

	MCFG_PALETTE_INIT(drw80pkr)
	MCFG_VIDEO_START(drw80pkr)

	MCFG_NVRAM_ADD_0FILL("nvram")

	// sound hardware
	MCFG_SPEAKER_STANDARD_MONO("mono")

	MCFG_SOUND_ADD("aysnd", AY8912, 20000000/12)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.75)
MACHINE_CONFIG_END

/*************************
*        Rom Load        *
*************************/

ROM_START( drw80pkr )
	ROM_REGION( 0x2000, "maincpu", 0 )
	ROM_LOAD( "pm0.u81",   0x0000, 0x1000, CRC(73223555) SHA1(229999ec00a1353f0d4928c65c8975079060c5af) )
	ROM_LOAD( "pm1.u82",   0x1000, 0x1000, CRC(f8158f2b) SHA1(da3b30cfd49cd0e8a48d78fd3f82b2b4ab33670c) )

	ROM_REGION( 0x002000, "gfx1", 0 )
	ROM_LOAD( "cg0-a.u74",	 0x0000, 0x1000, CRC(0eefe598) SHA1(ed10aac345b10e35fb15babdd3ac30ebe2b8fc0f) )
	ROM_LOAD( "cg1-a.u76",	 0x1000, 0x1000, CRC(522a96d0) SHA1(48f855a132413493353fbf6a44a1feb34ae6726d) )

	ROM_REGION( 0x100, "proms", 0 )
	ROM_LOAD( "cap13.u92", 0x0000, 0x0100, CRC(be67a8d9) SHA1(24b8cd19a5ec09779a737f6fc8c07b44f1226c8f) )
ROM_END

ROM_START( drw80pk2 )
	ROM_REGION( 0x2000, "maincpu", 0 )
	ROM_LOAD( "pm0.u81",   0x0000, 0x1000, CRC(0f3e97d2) SHA1(aa9e4015246284f32435d7320de667e075412e5b) )
	ROM_LOAD( "pm1.u82",   0x1000, 0x1000, CRC(5a6ad467) SHA1(0128bd70b65244a0f68031d5f451bf115eeb7609) )

	ROM_REGION( 0x002000, "gfx1", 0 )
	ROM_LOAD( "cg0-a.u74",	 0x0000, 0x1000, CRC(97f5eb92) SHA1(f6c7bb42ccef8a78e8d56104ad942ae5b8e5b0df) )
	ROM_LOAD( "cg1-a.u76",	 0x1000, 0x1000, CRC(2a3a750d) SHA1(db6183d11b2865b011c3748dc472cf5858dde78f) )

	ROM_REGION( 0x100, "proms", 0 )
	ROM_LOAD( "cap13.u92", 0x0000, 0x0100, CRC(be67a8d9) SHA1(24b8cd19a5ec09779a737f6fc8c07b44f1226c8f) )
ROM_END

/*************************
*      Game Drivers      *
*************************/

/*    YEAR  NAME      PARENT  MACHINE   INPUT     INIT      ROT    COMPANY                                  FULLNAME                FLAGS   */
GAME( 1982, drw80pkr, 0,      drw80pkr, drw80pkr, drw80pkr, ROT0,  "IGT - International Gaming Technology", "Draw 80 Poker",        GAME_NOT_WORKING )
GAME( 1983, drw80pk2, 0,      drw80pkr, drw80pkr, drw80pkr, ROT0,  "IGT - International Gaming Technology", "Draw 80 Poker - Minn", GAME_NOT_WORKING )
