// license:BSD-3-Clause
// copyright-holders:Carl

#include "emu.h"
#include "cpu/i86/i186.h"
#include "machine/wd_fdc.h"
#include "video/mc6845.h"
#include "screen.h"


namespace {

class yes_state : public driver_device
{
public:
	yes_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_fdc(*this, "fdc"),
		m_crtc(*this, "crtc")
		{ }

	void yes(machine_config &config);

private:
	required_device<i80186_cpu_device> m_maincpu;
	required_device<wd2793_device> m_fdc;
	required_device<hd6845s_device> m_crtc;
	void io_map(address_map &map) ATTR_COLD;
	void program_map(address_map &map) ATTR_COLD;
	MC6845_UPDATE_ROW(crtc_update_row);
};

void yes_state::program_map(address_map &map)
{
	map(0x00000, 0xaffff).ram();
	map(0xf0000, 0xfffff).rom().region("bios", 0);
}

void yes_state::io_map(address_map &map)
{
	//map(0x0400, 0x041f).i8256
	map(0x0480, 0x0487).rw(m_fdc, FUNC(wd2793_device::read), FUNC(wd2793_device::write)).umask16(0x00ff);
	map(0x0500, 0x0500).w(m_crtc, FUNC(hd6845s_device::address_w));
	map(0x0502, 0x0502).rw(m_crtc, FUNC(hd6845s_device::register_r), FUNC(mc6845_device::register_w));
}

MC6845_UPDATE_ROW( yes_state::crtc_update_row )
{
}

void yes_state::yes(machine_config &config)
{
	/* basic machine hardware */
	I80186(config, m_maincpu, 16_MHz_XTAL);
	m_maincpu->set_addrmap(AS_PROGRAM, &yes_state::program_map);
	m_maincpu->set_addrmap(AS_IO, &yes_state::io_map);

	WD2793(config, m_fdc, 16_MHz_XTAL / 8);
	//m_fdc->intrq_wr_callback().set(m_muart, FUNC(i8256_device::ir));
	m_fdc->drq_wr_callback().set(m_maincpu, FUNC(i80186_cpu_device::drq1_w));

	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_raw(16_MHz_XTAL, 1016, 0, 640, 314, 0, 240);
	screen.set_screen_update("crtc", FUNC(hd6845s_device::screen_update));

	HD6845S(config, m_crtc, 16_MHz_XTAL / 8); //clock?
	m_crtc->set_screen("screen");
	m_crtc->set_show_border_area(false);
	m_crtc->set_char_width(8);
	m_crtc->set_update_row_callback(FUNC(yes_state::crtc_update_row));
}

ROM_START(yes)
	ROM_REGION16_LE(0x10000,"bios", 0)
	ROMX_LOAD("yes_23484.bin", 0x0001, 0x8000, CRC(3530cce3) SHA1(f888f3c291c405f6cb873b5f8ba00eb638947cbb), ROM_SKIP(1))
	ROMX_LOAD("yes_23494.bin", 0x0000, 0x8000, CRC(6d58b50f) SHA1(63af83c4055395a086c014f66dabbac1a728fb14), ROM_SKIP(1))

	ROM_REGION(0x2000, "chargen", 0)
	ROM_LOAD( "yes_23502.bin", 0x0000, 0x2000, CRC(e3324683) SHA1(87c3a6cb7fbe982f88abb85426785228c2b33bb7))
ROM_END

} // anonymous namespace


COMP( 1985, yes, 0, 0, yes, 0, yes_state, empty_init, "Philips", ":YES", MACHINE_NOT_WORKING | MACHINE_NO_SOUND )
