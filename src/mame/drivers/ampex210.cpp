// license:BSD-3-Clause
// copyright-holders:AJR
/***************************************************************************

    Skeleton driver for Ampex 210/210+ video display terminals.

****************************************************************************/

#include "emu.h"
#include "cpu/z80/z80.h"
#include "machine/mos6551.h"
#include "machine/nvram.h"
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
	void vram_map(address_map &map);

	required_device<cpu_device> m_maincpu;
	required_region_ptr<u8> m_chargen;
};


SCN2672_DRAW_CHARACTER_MEMBER(ampex210_state::draw_character)
{
}

void ampex210_state::mem_map(address_map &map)
{
	map(0x0000, 0x7fff).rom().region("maincpu", 0);
	map(0x8000, 0x87ff).ram().share("nvram");
	map(0x8800, 0x9fff).ram();
}

void ampex210_state::io_map(address_map &map)
{
	map.global_mask(0xff);
	map(0x00, 0x07).rw("pvtc", FUNC(scn2672_device::read), FUNC(scn2672_device::write));
	map(0x44, 0x47).rw("acia", FUNC(mos6551_device::read), FUNC(mos6551_device::write));
	map(0x80, 0x80).nopw();
	map(0xc0, 0xc0).w("pvtc", FUNC(scn2672_device::buffer_w));
	map(0xc1, 0xc1).r("pvtc", FUNC(scn2672_device::buffer_r));
	map(0xc7, 0xc7).nopw();
}

void ampex210_state::vram_map(address_map &map)
{
	map(0x0000, 0x1fff).ram(); // MB8464A-10L (second half unused?)
}


static INPUT_PORTS_START(ampex210p)
INPUT_PORTS_END


void ampex210_state::ampex210p(machine_config &config)
{
	Z80(config, m_maincpu, 3.6864_MHz_XTAL); // Z80ACPU; clock uncertain
	m_maincpu->set_addrmap(AS_PROGRAM, &ampex210_state::mem_map);
	m_maincpu->set_addrmap(AS_IO, &ampex210_state::io_map);

	NVRAM(config, "nvram", nvram_device::DEFAULT_ALL_0); // UM6116-2

	mos6551_device &acia(MOS6551(config, "acia", 3.6864_MHz_XTAL / 2)); // AMI S6551AP
	acia.irq_handler().set_inputline("maincpu", INPUT_LINE_IRQ0);

	scn2672_device &pvtc(SCN2672(config, "pvtc", 19.602_MHz_XTAL / 9)); // MC2672B4P
	pvtc.intr_callback().set_inputline("maincpu", INPUT_LINE_NMI);
	pvtc.set_character_width(9);
	pvtc.set_display_callback(FUNC(ampex210_state::draw_character));
	pvtc.set_addrmap(0, &ampex210_state::vram_map);
	pvtc.set_screen("screen");

	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_raw(19.602_MHz_XTAL, 900, 0, 720, 363, 0, 300);
	//screen.set_raw(32.147_MHz_XTAL, 1476, 0, 1188, 363, 0, 300);
	screen.set_screen_update("pvtc", FUNC(scn2672_device::screen_update));
}


ROM_START(ampex210p) // Z80 (+6551,MC2672,3515260-01, 3 xtals, speaker) // 8k ram // amber
	ROM_REGION(0x8000, "maincpu", 0)
	ROM_LOAD("35-5960-03.u30", 0x0000, 0x8000, CRC(d3f86117) SHA1(f8a9b66899117b362b195bfc94c75bc902fb1376))

	ROM_REGION(0x1000, "chargen", 0)
	ROM_LOAD("35-526-01.u3", 0x0000, 0x1000, CRC(4659bcd2) SHA1(554574f55ed875baba0a6133648c44df763cc5c4))
ROM_END

COMP(1989, ampex210p, 0, 0, ampex210p, ampex210p, ampex210_state, empty_init, "Ampex",              "210+", MACHINE_IS_SKELETON)
