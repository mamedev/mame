// license:BSD-3-Clause
// copyright-holders:AJR
/****************************************************************************

    Preliminary driver for single-board computers by Synertek/Sym Systems.

    The bottom edge connector is compatible with Motorola's EXORciser/
    Micromodule bus. One of MBC020's other connectors is claimed to allow
    "Direct Attachment to a CRT Monitor"; this was omitted in MBC010.

    MBC010 and MBC020 were available with either a SY6512 CPU (-65) or
    MC6800 CPU (-68), running at either 1 or 2 MHz.

    Though the timing circuit includes dynamic RAM refresh control, the
    onboard RAM is entirely static (6 SY2114s or equivalent, plus an optional
    socketed 6116).

    "Sym Systems Corp." may have been a short-lived spinoff or subsidiary of
    Synertek, named in reference to their SYM-1 SBC. Synertek clearly did not
    insist on in-house sourcing of the 6500-series peripherals, since their
    catalog photo of the MBC020 includes a Motorola-branded MC6845 CRTC, and
    the dumped board has a Rockwell VIA and AMI ACIA.

****************************************************************************/

#include "emu.h"
#include "bus/rs232/rs232.h"
#include "cpu/m6502/m6502.h"
#include "machine/6522via.h"
#include "machine/mos6551.h"
#include "video/mc6845.h"
#include "screen.h"


namespace {

class mbc020_state : public driver_device
{
public:
	mbc020_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_videoram(*this, "videoram")
		, m_chargen(*this, "chargen")
	{
	}

	void mbc020(machine_config &config);

private:
	MC6845_UPDATE_ROW(update_row);
	MC6845_ON_UPDATE_ADDR_CHANGED(update_cb);

	void mem_map(address_map &map) ATTR_COLD;

	required_device<cpu_device> m_maincpu;
	required_shared_ptr<u8> m_videoram;
	required_region_ptr<u8> m_chargen;
};

MC6845_UPDATE_ROW(mbc020_state::update_row)
{
	u32 *pix = &bitmap.pix(y);

	for (int x = 0; x < x_count; x++)
	{
		u8 data = m_videoram[(ma + x) & 0x7ff];

		// No XOR logic evident on PCB
		u8 dots = x == cursor_x ? 0xff : m_chargen[(data & 0x7f) | (ra & 15) << 7];

		// Guess as to how "Dual Intensity Video Levels" works
		rgb_t fg = BIT(data, 7) ? rgb_t(0xc0, 0xc0, 0xc0) : rgb_t::white();
		for (int n = 7; n >= 0; n--)
			*pix++ = BIT(dots, n) ? fg : rgb_t::black();
	}
}

MC6845_ON_UPDATE_ADDR_CHANGED(mbc020_state::update_cb)
{
}


void mbc020_state::mem_map(address_map &map)
{
	map(0x0000, 0x03ff).ram();
	map(0x8000, 0x8fff).rom().region("monitor", 0x2000);
	map(0x9000, 0x900f).m("extvia", FUNC(via6522_device::map));
	map(0x9900, 0x990f).m("via", FUNC(via6522_device::map));
	map(0x9a00, 0x9a03).rw("acia", FUNC(mos6551_device::read), FUNC(mos6551_device::write));
	map(0x9c00, 0x9c00).rw("crtc", FUNC(sy6545_1_device::status_r), FUNC(sy6545_1_device::address_w));
	map(0x9c01, 0x9c01).rw("crtc", FUNC(sy6545_1_device::register_r), FUNC(sy6545_1_device::register_w));
	map(0xa000, 0xa7ff).ram();
	map(0xa800, 0xafff).ram().share("videoram");
	map(0xb000, 0xbfff).rom().region("monitor", 0);
	map(0xe000, 0xefff).rom().region("monitor", 0x1000);
	map(0xf800, 0xffff).rom().region("monitor", 0x2800);
}

static INPUT_PORTS_START(mbc020)
	PORT_START("PA")
	PORT_CONFNAME(0x38, 0x30, "Baud Rate")
	PORT_CONFSETTING(0x00, "110")
	PORT_CONFSETTING(0x08, "300")
	PORT_CONFSETTING(0x10, "600")
	PORT_CONFSETTING(0x18, "1200")
	PORT_CONFSETTING(0x20, "2400")
	PORT_CONFSETTING(0x28, "4800")
	PORT_CONFSETTING(0x30, "9600")
	PORT_CONFSETTING(0x38, "19200")
	PORT_BIT(0xc3, IP_ACTIVE_LOW, IPT_UNUSED)
INPUT_PORTS_END

static DEVICE_INPUT_DEFAULTS_START( keyboard )
	DEVICE_INPUT_DEFAULTS( "RS232_TXBAUD", 0xff, RS232_BAUD_9600 )
	DEVICE_INPUT_DEFAULTS( "RS232_DATABITS", 0xff, RS232_DATABITS_7 )
	DEVICE_INPUT_DEFAULTS( "RS232_PARITY", 0xff, RS232_PARITY_NONE )
	DEVICE_INPUT_DEFAULTS( "RS232_STOPBITS", 0xff, RS232_STOPBITS_2 )
DEVICE_INPUT_DEFAULTS_END

static DEVICE_INPUT_DEFAULTS_START( terminal )
	DEVICE_INPUT_DEFAULTS( "RS232_RXBAUD", 0xff, RS232_BAUD_9600 )
	DEVICE_INPUT_DEFAULTS( "RS232_TXBAUD", 0xff, RS232_BAUD_9600 )
	DEVICE_INPUT_DEFAULTS( "RS232_DATABITS", 0xff, RS232_DATABITS_7 )
	DEVICE_INPUT_DEFAULTS( "RS232_PARITY", 0xff, RS232_PARITY_NONE )
	DEVICE_INPUT_DEFAULTS( "RS232_STOPBITS", 0xff, RS232_STOPBITS_2 )
DEVICE_INPUT_DEFAULTS_END

void mbc020_state::mbc020(machine_config &config)
{
	M6512(config, m_maincpu, 16_MHz_XTAL / 8); // SYU6512A
	m_maincpu->set_addrmap(AS_PROGRAM, &mbc020_state::mem_map);

	MOS6522(config, "via", 16_MHz_XTAL / 8); // R6522AP

	via6522_device &extvia(MOS6522(config, "extvia", 16_MHz_XTAL / 8)); // not on main board
	extvia.readpa_handler().set_ioport("PA");

	mos6551_device &acia(MOS6551(config, "acia", 16_MHz_XTAL / 8)); // S6551AP
	acia.set_xtal(1.8432_MHz_XTAL);
	acia.txd_handler().set("rs232", FUNC(rs232_port_device::write_txd));
	acia.rts_handler().set("rs232", FUNC(rs232_port_device::write_rts));
	acia.dtr_handler().set("rs232", FUNC(rs232_port_device::write_dtr));

	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_raw(16_MHz_XTAL, 1016, 0, 640, 263, 0, 225);
	screen.set_screen_update("crtc", FUNC(sy6545_1_device::screen_update));

	sy6545_1_device &crtc(SY6545_1(config, "crtc", 16_MHz_XTAL / 8)); // SY6545-1
	crtc.set_screen("screen");
	crtc.set_show_border_area(false);
	crtc.set_char_width(8);
	crtc.set_update_row_callback(FUNC(mbc020_state::update_row));
	crtc.set_on_update_addr_change_callback(FUNC(mbc020_state::update_cb));

	rs232_port_device &rs232(RS232_PORT(config, "rs232", default_rs232_devices, "keyboard"));
	rs232.set_option_device_input_defaults("keyboard", DEVICE_INPUT_DEFAULTS_NAME(keyboard));
	rs232.set_option_device_input_defaults("terminal", DEVICE_INPUT_DEFAULTS_NAME(terminal));
	rs232.rxd_handler().set("acia", FUNC(mos6551_device::write_rxd));
	rs232.dsr_handler().set("acia", FUNC(mos6551_device::write_dsr));
	rs232.cts_handler().set("acia", FUNC(mos6551_device::write_cts));
}

ROM_START(mbc020) // Silkscreened on PCB: "© 1980 by SYM Systems Corp."
	ROM_REGION(0x3000, "monitor", 0) // "SERVOMON VER 4.0 COPYRIGHT JUL-1983 TORQUE SYSTEMS INC."
	ROM_LOAD("20013-4.u13", 0x0000, 0x1000, CRC(53cbfc68) SHA1(72834ac1d8e8feed1941c7b7d53b264d8333a496)) // TMS2532JL-35
	ROM_LOAD("20014-4.u14", 0x1000, 0x1000, CRC(f7ed5508) SHA1(a6d644f07c889c24291fe6d64f9ef90ef34324ba)) // TMS2532JL-35
	ROM_LOAD("20015-4.u15", 0x2000, 0x1000, CRC(17485482) SHA1(57a6f684dd5111f2499b655f27794116aef354d7)) // TMS2532JL-35

	ROM_REGION(0x800, "chargen", 0)
	ROM_LOAD("02-0054a.u5", 0x000, 0x800, CRC(3ed97af7) SHA1(26d5a1c96b9896336e7ccf9e66dbeb2733ab4593))

	ROM_REGION(0x20, "mmap", 0) // Memory mapping PROM (decodes A11–A15)
	ROM_LOAD("n82s123n.u17", 0x00, 0x20, CRC(f219550d) SHA1(b149f9872bc9091b28b0da65f0206ce9893083be))
ROM_END

} // anonymous namespace


COMP(1983, mbc020, 0, 0, mbc020, mbc020, mbc020_state, empty_init, "Sym Systems / Torque Systems", "MBC020-65 CPU/Video Board (Torque Systems OEM)", MACHINE_NOT_WORKING | MACHINE_NO_SOUND_HW)
