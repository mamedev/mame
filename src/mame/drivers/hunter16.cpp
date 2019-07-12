// license:BSD-3-Clause
// copyright-holders:Barry Rodewald
/**********************************************************************

    Husky Hunter 16, Hunter 16/80

**********************************************************************/


#include "emu.h"

#include "cpu/nec/v25.h"
#include "video/hd61830.h"
#include "video/mc6845.h"

#include "screen.h"
#include "emupal.h"

class hunter16_state : public driver_device
{
public:
	hunter16_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_screen(*this, "screen"),
		m_palette(*this, "palette"),
		m_lcdc(*this, "lcdc"),
		m_cga(*this, "cga")
		{}

	void hunter16(machine_config &config);
	void hunter1680(machine_config &config);

protected:
	uint32_t screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);

	void mem_map(address_map &map);
	void io_16_map(address_map &map);
	void io_1680_map(address_map &map);

	required_device<cpu_device> m_maincpu;
	required_device<screen_device> m_screen;
	optional_device<palette_device> m_palette;
	optional_device<hd61830_device> m_lcdc;
	optional_device<mc6845_device> m_cga;

private:
	void palette_init_hunter16(palette_device &palette);
};


void hunter16_state::mem_map(address_map &map)
{
	map(0x00000,0xbffff).ram();
	map(0xf0000,0xfffff).rom().region("bios",0);
}

void hunter16_state::io_16_map(address_map &map)
{
	map(0x430, 0x430).w(m_lcdc, FUNC(hd61830_device::data_w));
	map(0x431, 0x431).rw(m_lcdc, FUNC(hd61830_device::status_r), FUNC(hd61830_device::control_w));
//  map(0x43e, 0x43e).r(m_lcdc, FUNC(hd61830_device::data_r));
}

void hunter16_state::io_1680_map(address_map &map)
{
}

static INPUT_PORTS_START( hunter16 )
INPUT_PORTS_END

void hunter16_state::palette_init_hunter16(palette_device &palette)
{
	palette.set_pen_color(0, rgb_t(138, 146, 148));
	palette.set_pen_color(1, rgb_t(92, 83, 88));
}

void hunter16_state::hunter16(machine_config &config)
{
	V25(config, m_maincpu, 16_MHz_XTAL / 2);
	m_maincpu->set_addrmap(AS_PROGRAM, &hunter16_state::mem_map);
	m_maincpu->set_addrmap(AS_IO, &hunter16_state::io_16_map);

	HD61830(config, m_lcdc, 16_MHz_XTAL / 8);  // unknown clock

	SCREEN(config, m_screen, SCREEN_TYPE_LCD);
	m_screen->set_refresh_hz(60);
	m_screen->set_size(240, 128);
	m_screen->set_visarea(0, 239, 0, 63);
	m_screen->set_palette(m_palette);
	m_screen->set_screen_update("lcdc", FUNC(hd61830_device::screen_update));

	PALETTE(config, m_palette, FUNC(hunter16_state::palette_init_hunter16), 2);
	//m_palette->set_init(DEVICE_SELF_OWNER, FUNC(hunter16_state::palette_init_hunter16));
}

void hunter16_state::hunter1680(machine_config &config)
{
	V25(config, m_maincpu, 16_MHz_XTAL / 2);
	m_maincpu->set_addrmap(AS_PROGRAM, &hunter16_state::mem_map);
	m_maincpu->set_addrmap(AS_IO, &hunter16_state::io_1680_map);

	MC6845(config, m_cga, 16_MHz_XTAL / 2);   // Chips 82C426 CGA, exact clock unknown
	m_cga->set_screen("screen");
	m_cga->set_show_border_area(false);

	SCREEN(config, m_screen, SCREEN_TYPE_LCD);
	m_screen->set_refresh_hz(60);
	m_screen->set_size(640, 400);
	m_screen->set_visarea(0, 639, 0, 399);
	m_screen->set_screen_update("cga", FUNC(mc6845_device::screen_update));
}

ROM_START( hunter16 )
	ROM_REGION(0x20000, "bios", 0)
	ROM_LOAD( "v3.03i.bin", 0x0000, 0x20000, CRC(8b3ee7fd) SHA1(8bf962358a316c78ba91658c21e45e1e6cda9605) )
ROM_END

ROM_START( hunter1680 )
	ROM_REGION(0x20000, "bios", 0)
	ROM_LOAD( "v4.05.bin", 0x0000, 0x20000, CRC(e0cfabf4) SHA1(183d5bf7553404302697ac89eed25f2a8bb7695c) )
ROM_END

/*    YEAR  NAME        PARENT    COMPAT  MACHINE     INPUT     CLASS          INIT        COMPANY    FULLNAME                FLAGS */
COMP( 1989, hunter16,   0,        0,      hunter16,   hunter16, hunter16_state, empty_init, "Husky Computers Ltd", "Hunter 16", MACHINE_IS_SKELETON )
COMP( 1989, hunter1680, hunter16, 0,      hunter1680, hunter16, hunter16_state, empty_init, "Husky Computers Ltd", "Hunter 16/80", MACHINE_IS_SKELETON )
