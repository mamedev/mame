// license:BSD-3-Clause
// copyright-holders:AJR
/*******************************************************************************

    Skeleton driver for Alcatel 7100 terminal.

*******************************************************************************/

#include "emu.h"
#include "cpu/z80/z80.h"
#include "machine/am9517a.h"
#include "machine/ay31015.h"
#include "machine/z80ctc.h"
#include "machine/z80pio.h"
#include "video/mc6845.h"
#include "screen.h"


namespace {

class alcat7100_state : public driver_device
{
public:
	alcat7100_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
	{ }

	void alcat7100(machine_config &config);

private:
	MC6845_UPDATE_ROW(update_row);

	void mem_map(address_map &map) ATTR_COLD;
	void io_map(address_map &map) ATTR_COLD;

	required_device<z80_device> m_maincpu;
};


MC6845_UPDATE_ROW(alcat7100_state::update_row)
{
}


void alcat7100_state::mem_map(address_map &map)
{
	map(0x0000, 0x0fff).rom().region("maincpu", 0);
	map(0x7800, 0x7fff).ram();
	map(0xc800, 0xcfff).ram();
	map(0xf800, 0xffff).ram();
}

void alcat7100_state::io_map(address_map &map)
{
	map.global_mask(0xff);
	map(0x09, 0x09).nopw();
	map(0x0c, 0x0f).rw("pio1", FUNC(z80pio_device::read), FUNC(z80pio_device::write));
	map(0x20, 0x23).rw("pio2", FUNC(z80pio_device::read), FUNC(z80pio_device::write));
	map(0x24, 0x24).w("crtc", FUNC(mc6845_device::address_w));
	map(0x28, 0x2b).rw("ctc", FUNC(z80ctc_device::read), FUNC(z80ctc_device::write));
	map(0x25, 0x25).w("crtc", FUNC(mc6845_device::register_w));
	map(0x30, 0x3f).rw("dmac", FUNC(am9517a_device::read), FUNC(am9517a_device::write));
}


static INPUT_PORTS_START(alcat7100)
INPUT_PORTS_END


static const z80_daisy_config daisy_chain[] =
{
	{ "pio1" },
	{ "pio2" },
	{ "ctc" },
	{ nullptr }
};

void alcat7100_state::alcat7100(machine_config &config)
{
	Z80(config, m_maincpu, 24_MHz_XTAL / 6); // MK3880N-4 (clock not verified)
	m_maincpu->set_addrmap(AS_PROGRAM, &alcat7100_state::mem_map);
	m_maincpu->set_addrmap(AS_IO, &alcat7100_state::io_map);
	m_maincpu->set_daisy_config(daisy_chain);

	AM9517A(config, "dmac", 24_MHz_XTAL / 6); // P8237A-5

	z80pio_device &pio1(Z80PIO(config, "pio1", 24_MHz_XTAL / 6)); // Z8420A
	pio1.out_int_callback().set_inputline(m_maincpu, INPUT_LINE_IRQ0);

	z80pio_device &pio2(Z80PIO(config, "pio2", 24_MHz_XTAL / 6)); // Z8420A
	pio2.out_int_callback().set_inputline(m_maincpu, INPUT_LINE_IRQ0);
	pio2.in_pb_callback().set_constant(0x80); // ?

	z80ctc_device &ctc(Z80CTC(config, "ctc", 24_MHz_XTAL / 6));
	ctc.intr_callback().set_inputline(m_maincpu, INPUT_LINE_IRQ0);

	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_raw(15'900'000, 1200, 0, 960, 265, 0, 240);
	screen.set_screen_update("crtc", FUNC(mc6845_device::screen_update));

	mc6845_device &crtc(MC6845(config, "crtc", 15'900'000 / 12));
	crtc.set_screen("screen");
	crtc.set_char_width(12);
	crtc.set_show_border_area(false);
	crtc.set_update_row_callback(FUNC(alcat7100_state::update_row));

	AY31015(config, "uart"); // AMI S1602P
}


ROM_START(alcat7100) // Z80  // 256k ram // b&w  // looks like it needs a boot floppy to start
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("rom.u117",               0x0000, 0x0800, CRC(9c0debf7) SHA1(a042db34090656224ede41d8190f22f719d1a634))
	ROM_LOAD("906_513601_012gd2.u110", 0x0800, 0x0800, CRC(9346a41c) SHA1(6f7a2946494adac4d34874da9d5e475c99457000)) // keyboard?

	ROM_REGION(0x1000, "chargen", 0) // first half blank
	ROM_LOAD("906_513301_rev00_ba6d.u20", 0x0000, 0x1000, CRC(143cfdfc) SHA1(4d924d1f16c30d72e1fdbb786488156bb9961442))
ROM_END

} // anonymous namespace


COMP(1984, alcat7100, 0, 0, alcat7100, alcat7100, alcat7100_state, empty_init, "Alcatel", "Terminal 7100", MACHINE_IS_SKELETON)
