// license:BSD-3-Clause
// copyright-holders:AJR
/****************************************************************************

    Skeleton driver for TeleVideo Model 924 terminal.

****************************************************************************/

#include "emu.h"

//#include "bus/rs232/rs232.h"
#include "cpu/m6502/m6502.h"
#include "cpu/mcs48/mcs48.h"
#include "machine/mc68681.h"
//#include "machine/nvram.h"
#include "video/scn2674.h"
#include "screen.h"


namespace {

class tv924_state : public driver_device
{
public:
	tv924_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_pvtc(*this, "pvtc")
	{
	}

	void tv924(machine_config &config);

private:
	SCN2672_DRAW_CHARACTER_MEMBER(draw_character);

	void mem_map(address_map &map) ATTR_COLD;
	void char_map(address_map &map) ATTR_COLD;

	required_device<cpu_device> m_maincpu;
	required_device<scn2672_device> m_pvtc;
};

SCN2672_DRAW_CHARACTER_MEMBER(tv924_state::draw_character)
{
}

void tv924_state::mem_map(address_map &map)
{
	map(0x0000, 0x07ff).ram();
	map(0x2020, 0x202f).rw("duart", FUNC(scn2681_device::read), FUNC(scn2681_device::write));
	map(0x2030, 0x2037).rw(m_pvtc, FUNC(scn2672_device::read), FUNC(scn2672_device::write));
	map(0x8000, 0x9fff).ram();
	map(0xa000, 0xbfff).rom().region("program", 0x0000);
	map(0xe000, 0xffff).rom().region("program", 0x2000);
}

void tv924_state::char_map(address_map &map)
{
	map(0x0000, 0x3fff).ram();
}


static INPUT_PORTS_START(tv924)
INPUT_PORTS_END

void tv924_state::tv924(machine_config &config)
{
	M6502(config, m_maincpu, 1'723'560); // R6502AP (clock guessed)
	m_maincpu->set_addrmap(AS_PROGRAM, &tv924_state::mem_map);

	I8049(config, "kbdc", 5.7143_MHz_XTAL).set_disable();

	scn2681_device &duart(SCN2681(config, "duart", 3.6864_MHz_XTAL)); // SCN2681A
	duart.irq_cb().set_inputline(m_maincpu, m6502_device::IRQ_LINE);

	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_raw(27'576'960, 848 * 2, 0, 640 * 2, 271, 0, 250);
	screen.set_screen_update("pvtc", FUNC(scn2672_device::screen_update));

	SCN2672(config, m_pvtc, 1'723'560); // SCN2672A (with SCB2673B + AMI gate arrays 130170-00 and 130180-00)
	m_pvtc->set_screen("screen");
	m_pvtc->set_character_width(16); // nominally 8, but with half-dot shifting
	m_pvtc->set_addrmap(0, &tv924_state::char_map);
	m_pvtc->set_display_callback(FUNC(tv924_state::draw_character));
	m_pvtc->intr_callback().set_inputline(m_maincpu, m6502_device::NMI_LINE);
}

ROM_START(tv924)
	ROM_REGION(0x4000, "program", 0)
	ROM_LOAD("180001-53e_924_u14_12_18_84_e.bin", 0x0000, 0x2000, CRC(9129e555) SHA1(2977f5a8153b474d1fff4bec17952562277f86a4))
	ROM_LOAD("180001-54e_924_u21_12_18_84_e.bin", 0x2000, 0x2000, CRC(fa2cf28d) SHA1(9cb461b2ba2a7bf44467d5fbc5358c9caaa60024))

	ROM_REGION(0x1000, "chargen", 0)
	ROM_LOAD("350_rn_118_2333-5006_8000142.u10", 0x0000, 0x1000, NO_DUMP) // VTI 24-pin mask ROM

	ROM_REGION(0x800, "kbdc", 0)
	ROM_LOAD("d8049hc.bin", 0x000, 0x800, NO_DUMP)
ROM_END

} // anonymous namespace


COMP(1984, tv924, 0, 0, tv924, tv924, tv924_state, empty_init, "TeleVideo Systems", "TeleVideo 924 Video Display Terminal", MACHINE_IS_SKELETON)
