// license:BSD-3-Clause
// copyright-holders:Robbbert
/***************************************************************************

        Intel iPDS

        17/12/2009 Skeleton driver.

****************************************************************************/

#include "emu.h"
#include "cpu/i8085/i8085.h"
#include "video/i8275.h"
#include "machine/keyboard.h"
#include "emupal.h"
#include "screen.h"


namespace {

class ipds_state : public driver_device
{
public:
	ipds_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_crtc(*this, "i8275"),
		m_palette(*this, "palette")
	{
	}

	void ipds(machine_config &config);

private:
	required_device<cpu_device> m_maincpu;
	required_device<i8275_device> m_crtc;
	required_device<palette_device> m_palette;
	uint8_t ipds_b0_r();
	uint8_t ipds_b1_r();
	uint8_t ipds_c0_r();
	void ipds_b1_w(uint8_t data);
	void kbd_put(u8 data);
	I8275_DRAW_CHARACTER_MEMBER( crtc_display_pixels );
	uint8_t m_term_data = 0U;
	virtual void machine_reset() override ATTR_COLD;
	void ipds_io(address_map &map) ATTR_COLD;
	void ipds_mem(address_map &map) ATTR_COLD;
};

uint8_t ipds_state::ipds_b0_r()
{
	if (m_term_data)
		return 0xc0;
	else
		return 0x80;
}

uint8_t ipds_state::ipds_b1_r()
{
	uint8_t ret = m_term_data;
	m_term_data = 0;
	return ret;
}

uint8_t ipds_state::ipds_c0_r() { return 0x55; }

void ipds_state::ipds_b1_w(uint8_t data)
{
}

void ipds_state::ipds_mem(address_map &map)
{
	map.unmap_value_high();
	map(0x0000, 0x07ff).rom();
	map(0x0800, 0xffff).ram();
}

void ipds_state::ipds_io(address_map &map)
{
	map.global_mask(0xff);
	map.unmap_value_high();
	map(0xb0, 0xb0).r(FUNC(ipds_state::ipds_b0_r));
	map(0xb1, 0xb1).rw(FUNC(ipds_state::ipds_b1_r), FUNC(ipds_state::ipds_b1_w));
	map(0xc0, 0xc0).r(FUNC(ipds_state::ipds_c0_r));
}

/* Input ports */
static INPUT_PORTS_START( ipds )
INPUT_PORTS_END

void ipds_state::machine_reset()
{
}

I8275_DRAW_CHARACTER_MEMBER( ipds_state::crtc_display_pixels )
{
	rgb_t const *const palette = m_palette->palette()->entry_list_raw();
	uint8_t *charmap = memregion("chargen")->base();
	uint8_t pixels = charmap[(linecount & 7) + (charcode << 3)] ^ 0xff;

	using namespace i8275_attributes;

	if (BIT(attrcode, VSP))
		pixels = 0;

	if (BIT(attrcode, LTEN))
		pixels = 0xff;

	if (BIT(attrcode, RVV))
		pixels ^= 0xff;

	bool hlgt = BIT(attrcode, HLGT);
	for(int i=0;i<6;i++)
		bitmap.pix(y, x + i) = palette[(pixels >> (5-i)) & 1 ? (hlgt ? 2 : 1) : 0];
}

/* F4 Character Displayer */
static const gfx_layout ipds_charlayout =
{
	8, 11,                  /* 8 x 11 characters */
	128,                    /* 128 characters */
	1,                  /* 1 bits per pixel */
	{ 0 },                  /* no bitplanes */
	/* x offsets */
	{ 0, 1, 2, 3, 4, 5, 6, 7 },
	/* y offsets */
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8, 8*8, 9*8, 10*8 },
	8*16                    /* every char takes 16 bytes */
};

static GFXDECODE_START( gfx_ipds )
	GFXDECODE_ENTRY( "chargen", 0x0000, ipds_charlayout, 0, 1 )
GFXDECODE_END


void ipds_state::kbd_put(u8 data)
{
	m_term_data = data;
}

void ipds_state::ipds(machine_config &config)
{
	/* basic machine hardware */
	I8085A(config, m_maincpu, XTAL(19'660'800) / 4);
	m_maincpu->set_addrmap(AS_PROGRAM, &ipds_state::ipds_mem);
	m_maincpu->set_addrmap(AS_IO, &ipds_state::ipds_io);

	/* video hardware */
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER, rgb_t::green()));
	screen.set_screen_update("i8275", FUNC(i8275_device::screen_update));
	screen.set_refresh_hz(50);
	screen.set_vblank_time(ATTOSECONDS_IN_USEC(2500)); /* not accurate */
	screen.set_size(640, 480);
	screen.set_visarea(0, 640-1, 0, 480-1);
	GFXDECODE(config, "gfxdecode", m_palette, gfx_ipds);
	PALETTE(config, m_palette, palette_device::MONOCHROME);

	I8275(config, m_crtc, XTAL(19'660'800) / 4);
	m_crtc->set_character_width(6);
	m_crtc->set_display_callback(FUNC(ipds_state::crtc_display_pixels));

	generic_keyboard_device &keyboard(GENERIC_KEYBOARD(config, "keyboard", 0));
	keyboard.set_keyboard_callback(FUNC(ipds_state::kbd_put));
}

/* ROM definition */
ROM_START( ipds )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "164180.bin", 0x0000, 0x0800, CRC(10ba23ce) SHA1(67a9d03a08cdd7c70a867fb7e069703c7a0b9094) )

	ROM_REGION( 0x10000, "slavecpu", ROMREGION_ERASEFF )
	ROM_LOAD( "164196.bin", 0x0000, 0x1000, CRC(d3e18bc6) SHA1(01bc233be154bdfea9e8015839c5885cdaa08f11) )

	ROM_REGION(0x0800, "chargen", 0)
	ROM_LOAD( "163642.bin", 0x0000, 0x0800, CRC(3d81b2d1) SHA1(262a42920f15f1156ad0717c876cc0b2ed947ec5) )

	ROM_REGION(0x1000, "user1", 0)
	ROM_LOAD( "162806-001.u24", 0x0000, 0x0800, CRC(e6ba41ca) SHA1(a2d3438b728670b75c359ff49146ef57c08bf0f1) )
	ROM_LOAD( "163775-001.u8",  0x0800, 0x0800, CRC(0442163e) SHA1(737732adcfe22823db37112e28758464f5d9af7b) )
ROM_END

} // anonymous namespace


/* Driver */

/*    YEAR  NAME  PARENT  COMPAT  MACHINE  INPUT  CLASS       INIT        COMPANY  FULLNAME  FLAGS */
COMP( 1982, ipds, 0,      0,      ipds,    ipds,  ipds_state, empty_init, "Intel", "iPDS",   MACHINE_NOT_WORKING | MACHINE_NO_SOUND)
