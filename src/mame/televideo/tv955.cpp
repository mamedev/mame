// license:BSD-3-Clause
// copyright-holders:AJR
/***********************************************************************************************************************************

Skeleton driver for "third generation" TeleVideo terminals (905, 955, 9220).

************************************************************************************************************************************/

#include "emu.h"
#include "tv955kb.h"
#include "bus/rs232/rs232.h"
#include "cpu/m6502/m65c02.h"
#include "machine/input_merger.h"
#include "machine/mos6551.h"
#include "machine/nvram.h"
#include "video/scn2674.h"
#include "screen.h"


namespace {

class tv955_state : public driver_device
{
public:
	tv955_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_crtc(*this, "crtc")
		, m_hostuart(*this, "hostuart")
		, m_printuart(*this, "printuart")
		, m_keybuart(*this, "keybuart")
		, m_mainport(*this, "mainport")
		, m_printer(*this, "printer")
		, m_chargen(*this, "chargen")
	{ }

	void tv955(machine_config &config);

protected:
	virtual void machine_reset() override ATTR_COLD;

private:
	SCN2674_DRAW_CHARACTER_MEMBER(draw_character);

	void control_latch_w(u8 data);
	void system_reset_w(int state);

	void mem_map(address_map &map) ATTR_COLD;
	void char_map(address_map &map) ATTR_COLD;
	void attr_map(address_map &map) ATTR_COLD;

	required_device<cpu_device> m_maincpu;
	required_device<scn2674_device> m_crtc;
	required_device<mos6551_device> m_hostuart;
	required_device<mos6551_device> m_printuart;
	required_device<mos6551_device> m_keybuart;
	required_device<rs232_port_device> m_mainport;
	required_device<rs232_port_device> m_printer;
	required_region_ptr<u8> m_chargen;
};

void tv955_state::machine_reset()
{
	m_printer->write_rts(0);
	m_printer->write_dtr(0);

	m_printuart->write_cts(0);
	m_keybuart->write_cts(0);
}

SCN2674_DRAW_CHARACTER_MEMBER(tv955_state::draw_character)
{
	u16 dots = m_chargen[charcode << 4 | linecount] << 1;
	if (BIT(dots, 1) && BIT(charcode, 7))
		dots |= 1;

	// TODO: attribute logic
	if (cursor)
		dots = ~dots;

	for (int i = 0; i < 9; i++)
	{
		bitmap.pix(y, x++) = BIT(dots, 8) ? rgb_t::white() : rgb_t::black();
		dots <<= 1;
	}
}

void tv955_state::control_latch_w(u8 data)
{
	m_mainport->write_dtr(BIT(data, 0));
	m_hostuart->set_xtal(3.6864_MHz_XTAL / (BIT(data, 1) ? 1 : 2));

	// CPU clock is inverted relative to character clock (and divided by two for 132-column mode)
	if (BIT(data, 7))
	{
		// 132-column mode
		m_maincpu->set_unscaled_clock(31.684_MHz_XTAL / 18);
		m_crtc->set_unscaled_clock(31.684_MHz_XTAL / 9);
	}
	else
	{
		// 80-column mode
		m_maincpu->set_unscaled_clock(19.3396_MHz_XTAL / 9);
		m_crtc->set_unscaled_clock(19.3396_MHz_XTAL / 9);
	}
}

void tv955_state::system_reset_w(int state)
{
	m_maincpu->set_input_line(INPUT_LINE_RESET, state ? CLEAR_LINE : ASSERT_LINE);
	if (!state)
	{
		m_keybuart->reset();
		m_printuart->reset();
		m_hostuart->reset();
	}
}

void tv955_state::mem_map(address_map &map)
{
	// verified from maintenance manual (131968-00-C)
	map(0x0000, 0x07ff).mirror(0x0800).ram().share("nvram");
	map(0x1100, 0x1100).mirror(0x00ff).w(FUNC(tv955_state::control_latch_w));
	map(0x1200, 0x1203).mirror(0x00fc).rw(m_keybuart, FUNC(mos6551_device::read), FUNC(mos6551_device::write));
	map(0x1400, 0x1403).mirror(0x00fc).rw(m_printuart, FUNC(mos6551_device::read), FUNC(mos6551_device::write));
	map(0x1800, 0x1803).mirror(0x00fc).rw(m_hostuart, FUNC(mos6551_device::read), FUNC(mos6551_device::write));
	map(0x2000, 0x2007).mirror(0x0ff8).rw("crtc", FUNC(scn2674_device::read), FUNC(scn2674_device::write));
	map(0x3000, 0x3fff).rom().region("option", 0);
	map(0x4000, 0x7fff).ram().share("attrram");
	map(0x8000, 0xbfff).ram().share("charram");
	map(0xc000, 0xffff).rom().region("system", 0);
}

void tv955_state::char_map(address_map &map)
{
	map(0x0000, 0x3fff).ram().share("charram");
}

void tv955_state::attr_map(address_map &map)
{
	map(0x0000, 0x3fff).ram().share("attrram");
}

static INPUT_PORTS_START( tv955 )
INPUT_PORTS_END

void tv955_state::tv955(machine_config &config)
{
	M65C02(config, m_maincpu, 19.3396_MHz_XTAL / 9);
	m_maincpu->set_addrmap(AS_PROGRAM, &tv955_state::mem_map);

	INPUT_MERGER_ANY_HIGH(config, "mainirq").output_handler().set_inputline(m_maincpu, m6502_device::IRQ_LINE);

	tv955kb_device &keyboard(TV955_KEYBOARD(config, "keyboard"));
	keyboard.txd_cb().set("keybuart", FUNC(mos6551_device::write_rxd));
	keyboard.reset_cb().set(FUNC(tv955_state::system_reset_w));

	NVRAM(config, "nvram", nvram_device::DEFAULT_ALL_0); // HM6116LP-4 + 3.2V battery

	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_color(rgb_t::green());
	screen.set_raw(19.3396_MHz_XTAL, 846, 0, 720, 381, 0, 364);
	//screen.set_raw(31.684_MHz_XTAL, 1386, 0, 1188, 381, 0, 364);
	screen.set_screen_update("crtc", FUNC(scn2674_device::screen_update));

	SCN2674(config, m_crtc, 19.3396_MHz_XTAL / 9);
	// Character clock is 31.684_MHz_XTAL / 9 in 132-column mode
	// Character cells are 9 pixels wide by 14 pixels high
	m_crtc->set_character_width(9);
	m_crtc->set_addrmap(0, &tv955_state::char_map);
	m_crtc->set_addrmap(1, &tv955_state::attr_map);
	m_crtc->set_display_callback(FUNC(tv955_state::draw_character));
	m_crtc->intr_callback().set_inputline(m_maincpu, m6502_device::NMI_LINE);
	m_crtc->set_screen("screen");

	MOS6551(config, m_hostuart, 0);
	m_hostuart->set_xtal(3.6864_MHz_XTAL);
	m_hostuart->irq_handler().set("mainirq", FUNC(input_merger_device::in_w<0>));
	m_hostuart->txd_handler().set(m_mainport, FUNC(rs232_port_device::write_txd));
	m_hostuart->rts_handler().set(m_mainport, FUNC(rs232_port_device::write_rts));
	m_hostuart->dtr_handler().set(m_mainport, FUNC(rs232_port_device::write_dtr));

	MOS6551(config, m_printuart, 0);
	m_printuart->set_xtal(3.6864_MHz_XTAL / 2);
	m_printuart->irq_handler().set("mainirq", FUNC(input_merger_device::in_w<1>));
	m_printuart->txd_handler().set(m_printer, FUNC(rs232_port_device::write_txd));

	MOS6551(config, m_keybuart, 0);
	m_keybuart->set_xtal(3.6864_MHz_XTAL / 2);
	m_keybuart->irq_handler().set("mainirq", FUNC(input_merger_device::in_w<2>));
	m_keybuart->txd_handler().set("keyboard", FUNC(tv955kb_device::write_rxd));

	RS232_PORT(config, m_mainport, default_rs232_devices, "loopback"); // DTE
	m_mainport->rxd_handler().set(m_hostuart, FUNC(mos6551_device::write_rxd));
	m_mainport->cts_handler().set(m_hostuart, FUNC(mos6551_device::write_cts));
	m_mainport->dsr_handler().set(m_hostuart, FUNC(mos6551_device::write_dsr));
	m_mainport->dcd_handler().set(m_hostuart, FUNC(mos6551_device::write_dcd));

	RS232_PORT(config, m_printer, default_rs232_devices, nullptr); // DCE
	m_printer->rxd_handler().set("printuart", FUNC(mos6551_device::write_rxd)); // pin 2
	m_printer->dsr_handler().set("printuart", FUNC(mos6551_device::write_dsr)); // pin 20 or pin 11
}

/**************************************************************************************************************

Televideo TVI-955 (132160-00 Rev. M)
Chips: G65SC02P-3, 3x S6551AP, SCN2674B, AMI 131406-00 (unknown 40-pin DIL), 2x TMM2064P-10 (near two similar empty sockets), HM6116LP-4, round silver battery
Crystals: 19.3396, 31.684, 3.6864
Keyboard: M5L8049-230P-6, 5.7143, Beeper

***************************************************************************************************************/

ROM_START( tv955 )
	ROM_REGION(0x4000, "system", 0)
	ROM_LOAD( "t180002-88d_955.u4",  0x0000, 0x4000, CRC(5767fbe7) SHA1(49a2241612af5c3af09778ffa541ac0bc186e05a) )

	ROM_REGION(0x1000, "option", 0)
	ROM_LOAD( "t180002-91a_calc.u5", 0x0000, 0x1000, CRC(f86c103a) SHA1(fa3ada3a5d8913e519e2ea4817e96166c1fedd32) )
	ROM_CONTINUE( 0x0000, 0x1000 ) // first half is all FF (and not addressable)

	ROM_REGION(0x1000, "chargen", 0)
	ROM_LOAD( "t180002-26b.u45",     0x0000, 0x1000, CRC(69c9ebc7) SHA1(32282c816ec597a7c45e939acb7a4155d35ea584) )
ROM_END

} // anonymous namespace


COMP( 1985, tv955, 0, 0, tv955, tv955, tv955_state, empty_init, "TeleVideo Systems", "TeleVideo 955", MACHINE_NOT_WORKING | MACHINE_IMPERFECT_GRAPHICS )
