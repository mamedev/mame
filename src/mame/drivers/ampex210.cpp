// license:BSD-3-Clause
// copyright-holders:AJR
/***************************************************************************

    Skeleton driver for Ampex 210/210+ video display terminals.

****************************************************************************/

#include "emu.h"
#include "cpu/z80/z80.h"
#include "machine/mos6551.h"
#include "video/scn2674.h"
#include "screen.h"


class ampex210_state : public driver_device
{
public:
	ampex210_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_chargen(*this, "chargen")
	{ }

	void ampex210p(machine_config &config);

private:
	SCN2672_DRAW_CHARACTER_MEMBER(draw_character);

	void mem_map(address_map &map);
	void io_map(address_map &map);
	required_device<cpu_device> m_maincpu;
	required_region_ptr<u8> m_chargen;
};


SCN2672_DRAW_CHARACTER_MEMBER(ampex210_state::draw_character)
{
}

void ampex210_state::mem_map(address_map &map)
{
	map(0x0000, 0x7fff).rom().region("maincpu", 0);
	map(0x8000, 0x87ff).ram();
	map(0x8800, 0x9fff).ram();
}

void ampex210_state::io_map(address_map &map)
{
	map.global_mask(0xff);
	map(0x00, 0x07).rw("pvtc", FUNC(scn2672_device::read), FUNC(scn2672_device::write));
	map(0x44, 0x47).rw("acia", FUNC(mos6551_device::read), FUNC(mos6551_device::write));
	map(0x80, 0x80).nopw();
}


static INPUT_PORTS_START(ampex210p)
INPUT_PORTS_END


void ampex210_state::ampex210p(machine_config &config)
{
	Z80(config, m_maincpu, 4'000'000);
	m_maincpu->set_addrmap(AS_PROGRAM, &ampex210_state::mem_map);
	m_maincpu->set_addrmap(AS_IO, &ampex210_state::io_map);

	mos6551_device &acia(MOS6551(config, "acia", 1'843'200));
	acia.irq_handler().set_inputline("maincpu", INPUT_LINE_IRQ0);

	scn2672_device &pvtc(SCN2672(config, "pvtc", 2'178'000));
	pvtc.intr_callback().set_inputline("maincpu", INPUT_LINE_NMI);
	pvtc.set_character_width(10);
	pvtc.set_display_callback(FUNC(ampex210_state::draw_character));
	pvtc.set_screen("screen");

	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_raw(21'780'000, 1000, 0, 800, 363, 0, 300); // clock guessed
	screen.set_screen_update("pvtc", FUNC(scn2672_device::screen_update));
}


ROM_START(ampex210p) // Z80 (+6551,MC2672,3515260-01, 3 xtals, speaker) // 8k ram // amber
	ROM_REGION(0x8000, "maincpu", 0)
	ROM_LOAD("35-5960-03.u30", 0x0000, 0x8000, CRC(d3f86117) SHA1(f8a9b66899117b362b195bfc94c75bc902fb1376))

	ROM_REGION(0x1000, "chargen", 0)
	ROM_LOAD("35-526-01.u3", 0x0000, 0x1000, CRC(4659bcd2) SHA1(554574f55ed875baba0a6133648c44df763cc5c4))
ROM_END

COMP(1989, ampex210p, 0, 0, ampex210p, ampex210p, ampex210_state, empty_init, "Ampex",              "210+", MACHINE_IS_SKELETON)
