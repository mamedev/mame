// license:BSD-3-Clause
// copyright-holders:David Haywood
/* Xyonix *********************************************************************

driver by David Haywood and Stephh

Notes about the board:

Ram is 2x 6264 (near Z80) and 1x 6264 near UM6845. Xtal is verified 16.000MHz,
I can also see another special chip. PHILKO PK8801. Chip looks about the same as a
TMS3615 (though I have no idea what the chip actually is). It's located next to the
PROM, the 2x 256k ROMs, and the 1x 6264 RAM.
Dip SW is 1 x 8-position

On the PCB is an empty socket. Written next to the socket is 68705P3. "oh no" you
say..... well, it's unpopulated, so maybe it was never used? (another PCB was
found with the 68705 populated)


TODO:
- there are some more unknown commands for the I/O chip

******************************************************************************/

#include "emu.h"

#include "cpu/z80/z80.h"
#include "sound/sn76496.h"
#include "video/mc6845.h"
#include "screen.h"
#include "speaker.h"
#include "emupal.h"
#include "tilemap.h"


// configurable logging
#define LOG_IO (1U << 1)

//#define VERBOSE (LOG_GENERAL | LOG_IO)

#include "logmacro.h"

#define LOGIO(...)     LOGMASKED(LOG_IO,     __VA_ARGS__)


namespace {

class xyonix_state : public driver_device
{
public:
	xyonix_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_crtc(*this, "crtc"),
		m_palette(*this, "palette"),
		m_gfxdecode(*this, "gfxdecode"),
		m_tiles(*this, "tiles"),
		m_vidram(*this, "vidram"),
		m_dsw(*this, "DSW"),
		m_player(*this, "P%u", 1U)
	{ }

	void xyonix(machine_config &config);

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;
	virtual void video_start() override ATTR_COLD;

private:
	required_device<cpu_device> m_maincpu;
	required_device<mc6845_device> m_crtc;
	required_device<palette_device> m_palette;
	required_device<gfxdecode_device> m_gfxdecode;
	required_region_ptr<uint8_t> m_tiles;

	required_shared_ptr<uint8_t> m_vidram;

	required_ioport m_dsw;
	required_ioport_array<2> m_player;

	tilemap_t *m_tilemap;

	uint8_t m_e0_data;
	uint8_t m_credits;
	uint8_t m_coins;
	uint8_t m_prev_coin;
	bool m_nmi_mask;

	void nmiclk_w(int state);
	void irqack_w(uint8_t data);
	void nmiack_w(uint8_t data);
	uint8_t io_r();
	void io_w(uint8_t data);
	void vidram_w(offs_t offset, uint8_t data);

	TILE_GET_INFO_MEMBER(get_tile_info);
	void palette(palette_device &palette) const;
	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

	void handle_coins(int coin);
	void main_map(address_map &map) ATTR_COLD;
	void port_map(address_map &map) ATTR_COLD;

	[[maybe_unused]] MC6845_UPDATE_ROW(crtc_update_row);
};

void xyonix_state::machine_start()
{
	save_item(NAME(m_e0_data));
	save_item(NAME(m_credits));
	save_item(NAME(m_coins));
	save_item(NAME(m_prev_coin));
	save_item(NAME(m_nmi_mask));
}

void xyonix_state::machine_reset()
{
	m_nmi_mask = false;
}

void xyonix_state::irqack_w(uint8_t data)
{
	m_maincpu->set_input_line(0, CLEAR_LINE);
}

void xyonix_state::nmiclk_w(int state)
{
	if (state && m_nmi_mask)
		m_maincpu->set_input_line(INPUT_LINE_NMI, ASSERT_LINE);
}

void xyonix_state::nmiack_w(uint8_t data)
{
	m_nmi_mask = BIT(data, 0);
	if (!m_nmi_mask)
		m_maincpu->set_input_line(INPUT_LINE_NMI, CLEAR_LINE);
}

void xyonix_state::palette(palette_device &palette) const
{
	const uint8_t *color_prom = memregion("proms")->base();

	for (int i = 0; i < palette.entries(); i++)
	{
		// red component
		int bit0 = BIT(color_prom[i], 0);
		int bit1 = BIT(color_prom[i], 1);
		int bit2 = BIT(color_prom[i], 2);
		int const r = 0x21 * bit0 + 0x47 * bit1 + 0x97 * bit2;

		// green component
		bit0 = BIT(color_prom[i], 5);
		bit1 = BIT(color_prom[i], 6);
		bit2 = BIT(color_prom[i], 7);
		int const g = 0x21 * bit0 + 0x47 * bit1 + 0x97 * bit2;

		// blue component
		bit0 = BIT(color_prom[i], 3);
		bit1 = BIT(color_prom[i], 4);
		int const b = 0x4f * bit0 + 0xa8 * bit1;

		palette.set_pen_color(i, rgb_t(r, g, b));
	}
}


TILE_GET_INFO_MEMBER(xyonix_state::get_tile_info)
{
	int attr = m_vidram[tile_index + 0x1000 + 1];

	int tileno = (m_vidram[tile_index + 1] << 0) | ((attr & 0x0f) << 8);

	tileinfo.set(0, tileno, attr >> 4, 0);
}

void xyonix_state::vidram_w(offs_t offset, uint8_t data)
{
	m_vidram[offset] = data;
	m_tilemap->mark_tile_dirty((offset - 1) & 0x0fff);
}

void xyonix_state::video_start()
{
	m_tilemap = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(xyonix_state::get_tile_info)), TILEMAP_SCAN_ROWS, 4, 8, 80, 32);
}

uint32_t xyonix_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	m_tilemap->draw(screen, bitmap, cliprect, 0, 0);
	return 0;
}


// not used because the tilemap renderer is much simpler

MC6845_UPDATE_ROW( xyonix_state::crtc_update_row )
{
	const pen_t *pen = m_palette->pens();

	for (int i = 0; i < x_count; i++)
	{
		uint8_t code = m_vidram[(0x0000 | (ma + i)) + 1];
		uint8_t attr = m_vidram[(0x1000 | (ma + i)) + 1];

		// tile offset into the gfx rom
		uint16_t tile = ((((attr & 0x0f) << 8) | code) << 3) | ra;

		// tile data (4 pixels with 4 bit color code)
		uint16_t data = m_tiles[0x8000 | tile] << 8 | m_tiles[tile];

		// draw 4 pixels
		bitmap.pix(y, i * 4 + 0) = pen[(attr & 0xf0) | bitswap<4>(data, 4, 0, 12, 8)];
		bitmap.pix(y, i * 4 + 1) = pen[(attr & 0xf0) | bitswap<4>(data, 5, 1, 13, 9)];
		bitmap.pix(y, i * 4 + 2) = pen[(attr & 0xf0) | bitswap<4>(data, 6, 2, 14, 10)];
		bitmap.pix(y, i * 4 + 3) = pen[(attr & 0xf0) | bitswap<4>(data, 7, 3, 15, 11)];
	}
}


/* Inputs ********************************************************************/

void xyonix_state::handle_coins(int coin)
{
	static const int coinage_table[4][2] = {{2,3},{2,1},{1,2},{1,1}};
	int tmp = 0;

	//popmessage("Coin %d", m_coin);

	if (coin & 1)   // Coin 2 !
	{
		tmp = (m_dsw->read() & 0xc0) >> 6;
		m_coins++;
		if (m_coins >= coinage_table[tmp][0])
		{
			m_credits += coinage_table[tmp][1];
			m_coins -= coinage_table[tmp][0];
		}
		machine().bookkeeping().coin_lockout_global_w(0); // Unlock all coin slots
		machine().bookkeeping().coin_counter_w(1, 1); machine().bookkeeping().coin_counter_w(1, 0); // Count slot B
	}

	if (coin & 2)   // Coin 1 !
	{
		tmp = (m_dsw->read() & 0x30) >> 4;
		m_coins++;
		if (m_coins >= coinage_table[tmp][0])
		{
			m_credits += coinage_table[tmp][1];
			m_coins -= coinage_table[tmp][0];
		}
		machine().bookkeeping().coin_lockout_global_w(0); // Unlock all coin slots
		machine().bookkeeping().coin_counter_w(0, 1); machine().bookkeeping().coin_counter_w(0, 0); // Count slot A
	}

	if (m_credits >= 9)
		m_credits = 9;
}


uint8_t xyonix_state::io_r()
{
	int regPC = m_maincpu->pc();

	if (regPC == 0x27ba)
		return 0x88;

	if (regPC == 0x27c2)
		return m_e0_data;

	if (regPC == 0x27c7)
	{
		int coin;

		switch (m_e0_data)
		{
			case 0x81:
				return m_player[0]->read() & 0x7f;
			case 0x82:
				return m_player[1]->read() & 0x7f;
			case 0x91:
				// check coin inputs
				coin = ((m_player[0]->read() & 0x80) >> 7) | ((m_player[1]->read() & 0x80) >> 6);
				if (coin ^ m_prev_coin && coin != 3)
				{
					if (m_credits < 9) handle_coins(coin);
				}
				m_prev_coin = coin;
				return m_credits;
			case 0x92:
				return ((m_player[0]->read() & 0x80) >> 7) | ((m_player[1]->read() & 0x80) >> 6);
			case 0xe0:  // reset?
				m_coins = 0;
				m_credits = 0;
				return 0xff;
			case 0xe1:
				m_credits--;
				return 0xff;
			case 0xfe:  // Dip Switches 1 to 4
				return m_dsw->read() & 0x0f;
			case 0xff:  // Dip Switches 5 to 8
				return m_dsw->read() >> 4;
		}
	}

	LOGIO("io_r - PC = %04x - port = %02x\n", regPC, m_e0_data);
	//popmessage("%02x", m_e0_data);

	return 0xff;
}

void xyonix_state::io_w(uint8_t data)
{
	LOGIO("io_w %02x - PC = %04x\n", data, m_maincpu->pc());
	m_e0_data = data;
}

/* Mem / Port Maps ***********************************************************/

void xyonix_state::main_map(address_map &map)
{
	map(0x0000, 0xbfff).rom();
	map(0xc000, 0xdfff).ram();
	map(0xe000, 0xffff).ram().w(FUNC(xyonix_state::vidram_w)).share(m_vidram);
}

void xyonix_state::port_map(address_map &map)
{
	map.global_mask(0xff);
	map(0x20, 0x20).nopr().w("sn1", FUNC(sn76496_device::write));   // SN76496 ready signal
	map(0x21, 0x21).nopr().w("sn2", FUNC(sn76496_device::write));
	map(0x40, 0x40).w(FUNC(xyonix_state::nmiack_w));
	map(0x50, 0x50).w(FUNC(xyonix_state::irqack_w));
	map(0x60, 0x60).w(m_crtc, FUNC(mc6845_device::address_w));
	map(0x61, 0x61).rw(m_crtc, FUNC(mc6845_device::register_r), FUNC(mc6845_device::register_w));
	map(0xe0, 0xe0).rw(FUNC(xyonix_state::io_r), FUNC(xyonix_state::io_w));
}

/* Inputs Ports **************************************************************/

static INPUT_PORTS_START( xyonix )
	PORT_START("P1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(1)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_COIN1 )      // handled by io_r()

	PORT_START("P2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_COIN2 )      // handled by io_r()

	PORT_START("DSW")
	PORT_DIPNAME( 0x03, 0x03, DEF_STR( Difficulty ) )
	PORT_DIPSETTING(    0x03, DEF_STR( Easy ) )
	PORT_DIPSETTING(    0x02, DEF_STR( Normal ) )
	PORT_DIPSETTING(    0x01, DEF_STR( Hard ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Hardest ) )          // DEF_STR( Very_Hard )
	PORT_DIPNAME( 0x04, 0x00, DEF_STR( Allow_Continue ) )
	PORT_DIPSETTING(    0x04, DEF_STR( No ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Yes ) )
	PORT_SERVICE( 0x08, IP_ACTIVE_LOW )
	PORT_DIPNAME( 0x30, 0x30, DEF_STR( Coin_A ) )
	PORT_DIPSETTING(    0x10, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x30, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 2C_3C ) )
	PORT_DIPSETTING(    0x20, DEF_STR( 1C_2C ) )
	PORT_DIPNAME( 0xc0, 0xc0, DEF_STR( Coin_B ) )
	PORT_DIPSETTING(    0x40, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0xc0, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 2C_3C ) )
	PORT_DIPSETTING(    0x80, DEF_STR( 1C_2C ) )
INPUT_PORTS_END

/* GFX Decode ****************************************************************/

static const gfx_layout charlayout =
{
	4,8,
	RGN_FRAC(1,2),
	4,
	{ 0, 4, RGN_FRAC(1,2)+0, RGN_FRAC(1,2)+4 },
	{ 3, 2, 1, 0 },
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8 },
	4*16
};

static GFXDECODE_START( gfx_xyonix )
	GFXDECODE_ENTRY( "tiles", 0, charlayout, 0, 16 )
GFXDECODE_END


/* MACHINE driver *************************************************************/

void xyonix_state::xyonix(machine_config &config)
{
	// basic machine hardware
	Z80(config, m_maincpu, 16000000 / 4);        // 4 MHz ?
	m_maincpu->set_addrmap(AS_PROGRAM, &xyonix_state::main_map);
	m_maincpu->set_addrmap(AS_IO, &xyonix_state::port_map);
	m_maincpu->set_periodic_int(FUNC(xyonix_state::irq0_line_assert), attotime::from_hz(4*60));  // ?? controls music tempo

	// video hardware
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_raw(16_MHz_XTAL / 2, 508, 0, 320, 256, 0, 224); // 8 MHz?
	screen.set_screen_update(FUNC(xyonix_state::screen_update));
//  screen.set_screen_update(m_crtc, FUNC(mc6845_device::screen_update));
	screen.set_palette("palette");

	GFXDECODE(config, m_gfxdecode, "palette", gfx_xyonix);

	PALETTE(config, "palette", FUNC(xyonix_state::palette), 256);

	MC6845(config, m_crtc, 16_MHz_XTAL / 8); // 2 MHz?
	m_crtc->set_screen("screen");
	m_crtc->set_show_border_area(false);
	m_crtc->set_char_width(4);
//  m_crtc->set_update_row_callback(FUNC(xyonix_state::crtc_update_row));
	m_crtc->out_vsync_callback().set(FUNC(xyonix_state::nmiclk_w));

	// sound hardware
	SPEAKER(config, "mono").front_center();

	SN76496(config, "sn1", 16000000/4).add_route(ALL_OUTPUTS, "mono", 0.5);
	SN76496(config, "sn2", 16000000/4).add_route(ALL_OUTPUTS, "mono", 0.5);
}

/* ROM Loading ***************************************************************/

ROM_START( xyonix )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "xyonix3.bin", 0x00000, 0x10000, CRC(1960a74e) SHA1(5fd7bc31ca2f5f1e114d3d0ccf6554ebd712cbd3) )

	ROM_REGION( 0x10000, "mcu", 0 )
	ROM_LOAD( "mc68705p3s.e7", 0x00000, 0x780, BAD_DUMP CRC(f60cdd86) SHA1(e18cc598153b3e108942328ee9c5b9f83b034c41) ) // FIXED BITS (xxxxxx0x)

	ROM_REGION( 0x10000, "tiles", 0 )
	ROM_LOAD( "xyonix1.bin", 0x00000, 0x08000, CRC(3dfa9596) SHA1(52cdbbe18f83cea7248c29588ea3a18c4bb7984f) )
	ROM_LOAD( "xyonix2.bin", 0x08000, 0x08000, CRC(db87343e) SHA1(62bc30cd65b2f8976cd73a0b349a9ccdb3faaad2) )

	ROM_REGION( 0x0100, "proms", 0 )
	ROM_LOAD( "xyonix.pr",   0x0000, 0x0100, CRC(0012cfc9) SHA1(c7454107a1a8083a370b662c617117b769c0dc1c) )
ROM_END

} // Anonymous namespace


/* GAME drivers **************************************************************/

GAME( 1989, xyonix, 0, xyonix, xyonix, xyonix_state, empty_init, ROT0, "Philko", "Xyonix", MACHINE_SUPPORTS_SAVE )
