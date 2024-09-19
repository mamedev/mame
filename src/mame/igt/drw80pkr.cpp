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
#include "cpu/mcs48/mcs48.h"
#include "machine/nvram.h"
#include "video/mc6845.h"
#include "sound/ay8910.h"
#include "emupal.h"
#include "screen.h"
#include "speaker.h"
#include "tilemap.h"


namespace {

class drw80pkr_state : public driver_device
{
public:
	drw80pkr_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_gfxdecode(*this, "gfxdecode"),
		m_crtc(*this, "crtc"),
		m_aysnd(*this, "aysnd"),
		m_mainbank(*this, "mainbank")
	{ }

	void init_drw80pkr();
	void drw80pkr(machine_config &config);

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void video_start() override ATTR_COLD;

private:
	tilemap_t *m_bg_tilemap;
	uint8_t m_t0;
	uint8_t m_t1;
	uint8_t m_p1;
	uint8_t m_p2;
	uint8_t m_prog;
	uint8_t m_bus;
	uint8_t m_attract_mode;
	uint8_t m_active_bank;
	uint8_t m_pkr_io_ram[0x100];
	uint16_t m_video_ram[0x0400];
	uint8_t m_color_ram[0x0400];

	required_device<i8039_device> m_maincpu;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<mc6845_device> m_crtc;
	required_device<ay8912_device> m_aysnd;
	required_memory_bank m_mainbank;

	void p1_w(uint8_t data);
	void p2_w(uint8_t data);
	void prog_w(int state);
	void bus_w(uint8_t data);
	void io_w(offs_t offset, uint8_t data);
	int t0_r();
	int t1_r();
	uint8_t p1_r();
	uint8_t p2_r();
	uint8_t bus_r();
	uint8_t io_r(offs_t offset);

	TILE_GET_INFO_MEMBER(get_bg_tile_info);
	void drw80pkr_palette(palette_device &palette) const;
	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

	void io_map(address_map &map) ATTR_COLD;
	void map(address_map &map) ATTR_COLD;
};


#define DATA_NVRAM_SIZE     0x100


void drw80pkr_state::machine_start()
{
	subdevice<nvram_device>("nvram")->set_base(m_pkr_io_ram, sizeof(m_pkr_io_ram));

	m_active_bank = 0;
}

/*****************
* Write Handlers *
******************/

void drw80pkr_state::p1_w(uint8_t data)
{
	m_p1 = data;
}

void drw80pkr_state::p2_w(uint8_t data)
{
	m_p2 = data;
}

void drw80pkr_state::prog_w(int state)
{
	m_prog = state;

	// Bankswitch Program Memory
	if (m_prog == 0x01)
	{
		m_active_bank = m_active_bank ^ 0x01;

		m_mainbank->set_entry(m_active_bank);
	}
}

void drw80pkr_state::bus_w(uint8_t data)
{
	m_bus = data;
}

void drw80pkr_state::io_w(offs_t offset, uint8_t data)
{
	uint16_t n_offs;

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
		m_crtc->address_w(data);

	if (m_p2 == 0xd7)
		m_crtc->register_w(data);

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
			m_aysnd->address_w(data);

		// ay8910_write_port_0_w
		if (m_p1 == 0xfe)
			m_aysnd->data_w(data);
	}
}

/****************
* Read Handlers *
****************/

int drw80pkr_state::t0_r()
{
	return m_t0;
}

int drw80pkr_state::t1_r()
{
	return m_t1;
}

uint8_t drw80pkr_state::p1_r()
{
	return m_p1;
}

uint8_t drw80pkr_state::p2_r()
{
	return m_p2;
}

uint8_t drw80pkr_state::bus_r()
{
	return m_bus;
}

uint8_t drw80pkr_state::io_r(offs_t offset)
{
	uint8_t ret;
	uint16_t kbdin;

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

	tileinfo.set(0, code, color, 0);
}

void drw80pkr_state::video_start()
{
	m_bg_tilemap = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(drw80pkr_state::get_bg_tile_info)), TILEMAP_SCAN_ROWS, 8, 8, 24, 27);
}

uint32_t drw80pkr_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	m_bg_tilemap->draw(screen, bitmap, cliprect, 0, 0);

	return 0;
}

void drw80pkr_state::drw80pkr_palette(palette_device &palette) const
{
	uint8_t const *const color_prom = memregion("proms")->base();
	for (int j = 0; j < palette.entries(); j++)
	{
		int const i = BIT(color_prom[j], 3);

		// red component
		int const tr = 0xf0 - (0xf0 * BIT(color_prom[j], 0));
		int const r = tr - (i * (tr / 5));

		// green component
		int const tg = 0xf0 - (0xf0 * BIT(color_prom[j], 1));
		int const g = tg - (i * (tg / 5));

		// blue component
		int const tb = 0xf0 - (0xf0 * BIT(color_prom[j], 2));
		int const b = tb - (i * (tb / 5));

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

static GFXDECODE_START( gfx_drw80pkr )
	GFXDECODE_ENTRY( "gfx1", 0x00000, charlayout, 0, 16 )
GFXDECODE_END


/**************
* Driver Init *
***************/

void drw80pkr_state::init_drw80pkr()
{
	m_mainbank->configure_entries(0, 2, memregion("maincpu")->base(), 0x1000);
}


/*************************
* Memory map information *
*************************/

void drw80pkr_state::map(address_map &map)
{
	map(0x0000, 0x0fff).bankr("mainbank");
}

void drw80pkr_state::io_map(address_map &map)
{
	map(0x00, 0xff).rw(FUNC(drw80pkr_state::io_r), FUNC(drw80pkr_state::io_w));
}

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

void drw80pkr_state::drw80pkr(machine_config &config)
{
	// basic machine hardware
	I8039(config, m_maincpu, 8_MHz_XTAL);
	m_maincpu->set_addrmap(AS_PROGRAM, &drw80pkr_state::map);
	m_maincpu->set_addrmap(AS_IO, &drw80pkr_state::io_map);
	m_maincpu->t0_in_cb().set(FUNC(drw80pkr_state::t0_r));
	m_maincpu->t1_in_cb().set(FUNC(drw80pkr_state::t1_r));
	m_maincpu->p1_in_cb().set(FUNC(drw80pkr_state::p1_r));
	m_maincpu->p1_out_cb().set(FUNC(drw80pkr_state::p1_w));
	m_maincpu->p2_in_cb().set(FUNC(drw80pkr_state::p2_r));
	m_maincpu->p2_out_cb().set(FUNC(drw80pkr_state::p2_w));
	m_maincpu->prog_out_cb().set(FUNC(drw80pkr_state::prog_w));
	m_maincpu->bus_in_cb().set(FUNC(drw80pkr_state::bus_r));
	m_maincpu->bus_out_cb().set(FUNC(drw80pkr_state::bus_w));

	NVRAM(config, "nvram", nvram_device::DEFAULT_ALL_0);

	// video hardware
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_raw(8_MHz_XTAL / 2, 256, 0, 192, 257, 0, 216); // 4 MHz?
	screen.set_screen_update(FUNC(drw80pkr_state::screen_update));
	screen.set_palette("palette");

	GFXDECODE(config, m_gfxdecode, "palette", gfx_drw80pkr);

	PALETTE(config, "palette", FUNC(drw80pkr_state::drw80pkr_palette), 16 * 16);

	MC6845(config, m_crtc, 8_MHz_XTAL / 16); // 0.5 MHz?
	m_crtc->set_screen("screen");
	m_crtc->set_show_border_area(false);
	m_crtc->set_char_width(8);
	m_crtc->out_vsync_callback().set_inputline("maincpu", INPUT_LINE_IRQ0);

	// sound hardware
	SPEAKER(config, "mono").front_center();

	AY8912(config, m_aysnd, 20000000/12).add_route(ALL_OUTPUTS, "mono", 0.75);
}

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

} // Anonymous namespace

/*************************
*      Game Drivers      *
*************************/

//    YEAR  NAME      PARENT  MACHINE   INPUT     STATE           INIT           ROT    COMPANY                                FULLNAME                FLAGS
GAME( 1982, drw80pkr, 0,      drw80pkr, drw80pkr, drw80pkr_state, init_drw80pkr, ROT0,  "IGT - International Game Technology", "Draw 80 Poker",        MACHINE_NOT_WORKING )
GAME( 1983, drw80pk2, 0,      drw80pkr, drw80pkr, drw80pkr_state, init_drw80pkr, ROT0,  "IGT - International Game Technology", "Draw 80 Poker - Minn", MACHINE_NOT_WORKING )
