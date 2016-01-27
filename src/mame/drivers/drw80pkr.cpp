// license:BSD-3-Clause
// copyright-holders:Jim Stolis
/**********************************************************************************


    DRAW 80 POKER

    Driver by Jim Stolis.


    --- Technical Notes ---

    Name:    Draw 80 Poker
    Company: IGT - International Game Technology
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
		: driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_gfxdecode(*this, "gfxdecode") { }

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
	DECLARE_WRITE8_MEMBER(t0_w);
	DECLARE_WRITE8_MEMBER(t1_w);
	DECLARE_WRITE8_MEMBER(p0_w);
	DECLARE_WRITE8_MEMBER(p1_w);
	DECLARE_WRITE8_MEMBER(p2_w);
	DECLARE_WRITE8_MEMBER(prog_w);
	DECLARE_WRITE8_MEMBER(bus_w);
	DECLARE_WRITE8_MEMBER(drw80pkr_io_w);
	DECLARE_READ8_MEMBER(t0_r);
	DECLARE_READ8_MEMBER(t1_r);
	DECLARE_READ8_MEMBER(p0_r);
	DECLARE_READ8_MEMBER(p1_r);
	DECLARE_READ8_MEMBER(p2_r);
	DECLARE_READ8_MEMBER(bus_r);
	DECLARE_READ8_MEMBER(drw80pkr_io_r);
	DECLARE_DRIVER_INIT(drw80pkr);
	TILE_GET_INFO_MEMBER(get_bg_tile_info);
	virtual void machine_start() override;
	virtual void video_start() override;
	DECLARE_PALETTE_INIT(drw80pkr);
	UINT32 screen_update_drw80pkr(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	required_device<cpu_device> m_maincpu;
	required_device<gfxdecode_device> m_gfxdecode;
};


#define CPU_CLOCK           XTAL_8MHz
#define DATA_NVRAM_SIZE     0x100


void drw80pkr_state::machine_start()
{
	machine().device<nvram_device>("nvram")->set_base(m_pkr_io_ram, sizeof(m_pkr_io_ram));
}

/*****************
* Write Handlers *
******************/

WRITE8_MEMBER(drw80pkr_state::t0_w)
{
	m_t0 = data;
}

WRITE8_MEMBER(drw80pkr_state::t1_w)
{
	m_t1 = data;
}

WRITE8_MEMBER(drw80pkr_state::p0_w)
{
	m_p0 = data;
}

WRITE8_MEMBER(drw80pkr_state::p1_w)
{
	m_p1 = data;
}

WRITE8_MEMBER(drw80pkr_state::p2_w)
{
	m_p2 = data;
}

WRITE8_MEMBER(drw80pkr_state::prog_w)
{
	m_prog = data;

	// Bankswitch Program Memory
	if (m_prog == 0x01)
	{
		m_active_bank = m_active_bank ^ 0x01;

		membank("bank1")->set_entry(m_active_bank);
	}
}

WRITE8_MEMBER(drw80pkr_state::bus_w)
{
	m_bus = data;
}

WRITE8_MEMBER(drw80pkr_state::drw80pkr_io_w)
{
	UINT16 n_offs;

	if (m_p2 == 0x3f || m_p2 == 0x7f)
	{
		n_offs = ((m_p1 & 0xc0) << 2 ) + offset;

		if (m_p2 == 0x3f)
		{
			m_video_ram[n_offs] = data; // low address
		} else {
			m_color_ram[n_offs] = data & 0x0f; // color palette
			m_video_ram[n_offs] += ((data & 0xf0) << 4 ); // high address
		}

		m_bg_tilemap->mark_tile_dirty(n_offs);
	}

	if (m_p2 == 0xc7)
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

	if (m_p2 == 0xd7)
	{
		// CRTC Address
	}

	if (m_p2 == 0xfb) {
		m_pkr_io_ram[offset] = data;
	}

	if (m_p2 == 0xff)
	{
		if (m_p1 == 0xdf)
		{
			m_attract_mode = data; // Latch this for use in input reads (0x01 = attract mode, 0x00 = game in progress)
		}

		if (m_p1 == 0xdb || m_p1 == 0xef || m_p1 == 0xf7 || m_p1 == 0xfb)
		{
			// unknown, most likely lamps, meters, hopper etc.
		}

		// ay8910 control port
		if (m_p1 == 0xfc)
			machine().device<ay8910_device>("aysnd")->address_w(space, 0, data);

		// ay8910_write_port_0_w
		if (m_p1 == 0xfe)
			machine().device<ay8910_device>("aysnd")->data_w(space, 0, data);
	}
}

/****************
* Read Handlers *
****************/

READ8_MEMBER(drw80pkr_state::t0_r)
{
	return m_t0;
}

READ8_MEMBER(drw80pkr_state::t1_r)
{
	return m_t1;
}

READ8_MEMBER(drw80pkr_state::p0_r)
{
	return m_p0;
}

READ8_MEMBER(drw80pkr_state::p1_r)
{
	return m_p1;
}

READ8_MEMBER(drw80pkr_state::p2_r)
{
	return m_p2;
}

READ8_MEMBER(drw80pkr_state::bus_r)
{
	return m_bus;
}

READ8_MEMBER(drw80pkr_state::drw80pkr_io_r)
{
	UINT8 ret;
	UINT16 kbdin;

	ret = 0x00;

	if (m_p2 == 0x3b)
	{
		// unknown
	}

	if (m_p2 == 0x7b)
	{
		ret = m_pkr_io_ram[offset];
	}

	if (m_p2 == 0xf7)
	{
		// unknown
	}

	if (m_p2 == 0xfb)
	{
		ret = m_pkr_io_ram[offset];
	}

	if (m_p2 == 0xff)
	{
		if (m_p1 == 0x5f || m_p1 == 0x9f || m_p1 == 0xdb)
		{
			// unknown
		}

		if (m_p1 == 0xfe)
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

		if ((m_attract_mode == 0x01 && m_p1 == 0xef) || m_p1 == 0xf7)
		{
			// TODO: Get Input Port Values
			kbdin = ((ioport("IN1")->read() & 0xaf ) << 8) + ioport("IN0")->read();

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
				case 0x0001: ret = 0x01; break; /* Door */
				case 0x4000: ret = 0x00; break;
				case 0x8000: ret = 0x00; break; /* Hand Pay */
				case 0x0002: ret = 0x00; break; /* Books */
				case 0x0004: ret = 0x0e; break; /* Coin In */
				case 0x0008: ret = 0x0d; break; /* Start */
				case 0x0010: ret = 0x00; break; /* Discard */
				case 0x0020: ret = 0x00; break; /* Cancel */
				case 0x0040: ret = 0x01; break; /* Hold 1 */
				case 0x0080: ret = 0x02; break; /* Hold 2 */
				case 0x0100: ret = 0x03; break; /* Hold 3 */
				case 0x0200: ret = 0x04; break; /* Hold 4 */
				case 0x0400: ret = 0x05; break; /* Hold 5 */
				case 0x0800: ret = 0x00; break; /* Bet */
			}
		}
	}

	return ret;
}


/****************************
* Video/Character functions *
****************************/

TILE_GET_INFO_MEMBER(drw80pkr_state::get_bg_tile_info)
{
	int color = m_color_ram[tile_index];
	int code = m_video_ram[tile_index];

	SET_TILE_INFO_MEMBER(0, code, color, 0);
}

void drw80pkr_state::video_start()
{
	m_bg_tilemap = &machine().tilemap().create(m_gfxdecode, tilemap_get_info_delegate(FUNC(drw80pkr_state::get_bg_tile_info),this), TILEMAP_SCAN_ROWS, 8, 8, 24, 27);
}

UINT32 drw80pkr_state::screen_update_drw80pkr(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	m_bg_tilemap->draw(screen, bitmap, cliprect, 0, 0);

	return 0;
}

PALETTE_INIT_MEMBER(drw80pkr_state, drw80pkr)
{
	const UINT8 *color_prom = memregion("proms")->base();
	int j;

	for (j = 0; j < palette.entries(); j++)
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

		palette.set_pen_color(j, rgb_t(r, g, b));
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

DRIVER_INIT_MEMBER(drw80pkr_state,drw80pkr)
{
	membank("bank1")->configure_entries(0, 2, memregion("maincpu")->base(), 0x1000);
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
	MCFG_CPU_VBLANK_INT_DRIVER("screen", drw80pkr_state,  irq0_line_hold)


	// video hardware

	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MCFG_SCREEN_SIZE((31+1)*8, (31+1)*8)
	MCFG_SCREEN_VISIBLE_AREA(0*8, 24*8-1, 0*8, 27*8-1)
	MCFG_SCREEN_UPDATE_DRIVER(drw80pkr_state, screen_update_drw80pkr)
	MCFG_SCREEN_PALETTE("palette")

	MCFG_GFXDECODE_ADD("gfxdecode", "palette", drw80pkr)
	MCFG_PALETTE_ADD("palette", 16*16)
	MCFG_PALETTE_INIT_OWNER(drw80pkr_state, drw80pkr)

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
	ROM_LOAD( "cg0-a.u74",   0x0000, 0x1000, CRC(0eefe598) SHA1(ed10aac345b10e35fb15babdd3ac30ebe2b8fc0f) )
	ROM_LOAD( "cg1-a.u76",   0x1000, 0x1000, CRC(522a96d0) SHA1(48f855a132413493353fbf6a44a1feb34ae6726d) )

	ROM_REGION( 0x100, "proms", 0 )
	ROM_LOAD( "cap13.u92", 0x0000, 0x0100, CRC(be67a8d9) SHA1(24b8cd19a5ec09779a737f6fc8c07b44f1226c8f) )
ROM_END

ROM_START( drw80pk2 )
	ROM_REGION( 0x2000, "maincpu", 0 )
	ROM_LOAD( "pm0.u81",   0x0000, 0x1000, CRC(0f3e97d2) SHA1(aa9e4015246284f32435d7320de667e075412e5b) )
	ROM_LOAD( "pm1.u82",   0x1000, 0x1000, CRC(5a6ad467) SHA1(0128bd70b65244a0f68031d5f451bf115eeb7609) )

	ROM_REGION( 0x002000, "gfx1", 0 )
	ROM_LOAD( "cg0-a.u74",   0x0000, 0x1000, CRC(97f5eb92) SHA1(f6c7bb42ccef8a78e8d56104ad942ae5b8e5b0df) )
	ROM_LOAD( "cg1-a.u76",   0x1000, 0x1000, CRC(2a3a750d) SHA1(db6183d11b2865b011c3748dc472cf5858dde78f) )

	ROM_REGION( 0x100, "proms", 0 )
	ROM_LOAD( "cap13.u92", 0x0000, 0x0100, CRC(be67a8d9) SHA1(24b8cd19a5ec09779a737f6fc8c07b44f1226c8f) )
ROM_END

/*************************
*      Game Drivers      *
*************************/

/*    YEAR  NAME      PARENT  MACHINE   INPUT     INIT      ROT    COMPANY                                  FULLNAME                FLAGS   */
GAME( 1982, drw80pkr, 0,      drw80pkr, drw80pkr, drw80pkr_state, drw80pkr, ROT0,  "IGT - International Game Technology", "Draw 80 Poker",        MACHINE_NOT_WORKING )
GAME( 1983, drw80pk2, 0,      drw80pkr, drw80pkr, drw80pkr_state, drw80pkr, ROT0,  "IGT - International Game Technology", "Draw 80 Poker - Minn", MACHINE_NOT_WORKING )
