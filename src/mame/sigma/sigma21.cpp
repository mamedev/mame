// license:BSD-3-Clause
// copyright-holders:AJR
/****************************************************************************

    Skeleton driver for "21" game by Sigma (possibly "Fortune 21").

    No real progress is to be expected until the program ROMs are redumped.

****************************************************************************/

#include "emu.h"
#include "cpu/m6809/m6809.h"
#include "machine/6821pia.h"
#include "video/mc6845.h"
#include "emupal.h"
#include "screen.h"


namespace {

class sigma21_state : public driver_device
{
public:
	sigma21_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
	{
	}

	void sigma21(machine_config &config);

private:
	MC6845_UPDATE_ROW(update_row);
	void sigma21_palette(palette_device &palette);

	void mem_map(address_map &map) ATTR_COLD;

	required_device<cpu_device> m_maincpu;
};


MC6845_UPDATE_ROW(sigma21_state::update_row)
{
}

void sigma21_state::sigma21_palette(palette_device &palette)
{
	palette.set_pen_color(0, rgb_t::black());
	palette.set_pen_color(1, rgb_t::white());
	palette.set_pen_color(2, rgb_t(0x00, 0x00, 0x55));
	palette.set_pen_color(3, rgb_t(0xff, 0x00, 0x00));
}


void sigma21_state::mem_map(address_map &map)
{
	map(0xc000, 0xc0ff).ram(); // NVRAM, maybe?
	map(0xd800, 0xd800).w("crtc", FUNC(mc6845_device::address_w));
	map(0xd801, 0xd801).w("crtc", FUNC(mc6845_device::register_w));
	map(0xd804, 0xd807).rw("pia", FUNC(pia6821_device::read), FUNC(pia6821_device::write));
	map(0xe800, 0xffff).rom().region("program", 0);
}


static INPUT_PORTS_START(sigma21)
INPUT_PORTS_END


static const gfx_layout gfx_layout =
{
	8, 8,
	RGN_FRAC(1,2),
	2,
	{ 0, RGN_FRAC(1,2) },
	{ 0, 1, 2, 3, 4, 5, 6, 7 },
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8 },
	8*8,
};

static GFXDECODE_START(gfx_sigma21)
	GFXDECODE_ENTRY("gfx", 0, gfx_layout, 0, 1)
GFXDECODE_END

void sigma21_state::sigma21(machine_config &config)
{
	MC6809(config, m_maincpu, 5600000); // exact type and clock unknown
	m_maincpu->set_addrmap(AS_PROGRAM, &sigma21_state::mem_map);

	PIA6821(config, "pia");

	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_raw(5600000, 360, 0, 256, 276, 0, 224); // copied from r2dtank; actual params are in badly dumped ROM
	screen.set_screen_update("crtc", FUNC(mc6845_device::screen_update));

	mc6845_device &crtc(MC6845(config, "crtc", 700000));
	crtc.set_screen("screen");
	crtc.set_show_border_area(false);
	crtc.set_char_width(8);
	crtc.set_update_row_callback(FUNC(sigma21_state::update_row));

	GFXDECODE(config, "gfxdecode", "palette", gfx_sigma21);
	PALETTE(config, "palette", FUNC(sigma21_state::sigma21_palette), 4);
}


// 21 (TWENTY-ONE)
// (C)197? SIGMA
ROM_START(sigma21)
	ROM_REGION(0x1800, "program", 0)
	ROM_LOAD("21_108-.i35", 0x0000, 0x0800, CRC(06fde41e) SHA1(b2088c38b4455102bbe2edf6137d066a69bfd05d))
	ROM_LOAD("21_108-.i36", 0x0800, 0x0800, BAD_DUMP CRC(6bfce933) SHA1(038f5b6a3b09506f9dc32933fb1dd284124a09aa)) // FIXED BITS (xxx1xxxx)
	ROM_LOAD("21_108-.i37", 0x1000, 0x0800, BAD_DUMP CRC(79fb8f3e) SHA1(6245da1b26633a02cbbf19b23213096533840041)) // FIXED BITS (11xxxxxx)
	ROM_FILL(0x17ff, 1, 0x00) // make at least the reset vector logical

	ROM_REGION(0x1000, "gfx", 0)
	ROM_LOAD("ic23.rom", 0x0000, 0x0800, CRC(ce045516) SHA1(439e82ccacd4c802bc4e6d72a7756933924ba5f5))
	ROM_LOAD("ic25.rom", 0x0800, 0x0800, CRC(01e31462) SHA1(27bf1c18a6d28739649b693c66212c81e8cda297))

	ROM_REGION(0x100, "proms", 0)
	ROM_LOAD("bprom.bin", 0x000, 0x100, CRC(c0e5e4e0) SHA1(c42636c4723fe8b2b00868af5dd33c0608b3fecc))
ROM_END

} // anonymous namespace


GAME(197?, sigma21, 0, sigma21, sigma21, sigma21_state, empty_init, ROT0, "Sigma Enterprises", "21 (Sigma)", MACHINE_IS_SKELETON)
