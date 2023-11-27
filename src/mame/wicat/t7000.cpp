// license:BSD-3-Clause
// copyright-holders:AJR
/****************************************************************************

    Skeleton driver for Wicat T7000 terminal.

****************************************************************************/

#include "emu.h"
//#include "bus/rs232/rs232.h"
#include "cpu/z80/z80.h"
#include "machine/74259.h"
#include "machine/scn_pci.h"
#include "machine/x2212.h"
#include "video/i8275.h"
#include "screen.h"

namespace {

class t7000_state : public driver_device
{
public:
	t7000_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_pci(*this, "pci%u", 0U)
		, m_crtc(*this, "crtc%u", 0U)
		, m_chargen(*this, "chargen")
	{
	}

	void t7000(machine_config &config);

private:
	I8275_DRAW_CHARACTER_MEMBER(display_character);

	void crtc_combined_w(offs_t offset, u8 data);

	void mem_map(address_map &map);
	void io_map(address_map &map);

	required_device<cpu_device> m_maincpu;
	required_device_array<scn_pci_device, 2> m_pci;
	required_device_array<i8275_device, 2> m_crtc;
	required_region_ptr<u8> m_chargen;
};

I8275_DRAW_CHARACTER_MEMBER(t7000_state::display_character)
{
}

void t7000_state::crtc_combined_w(offs_t offset, u8 data)
{
	for (auto &crtc : m_crtc)
		crtc->write(BIT(offset, 0), data);
}

void t7000_state::mem_map(address_map &map)
{
	map(0x0000, 0x3fff).rom().region("program", 0);
	map(0x4000, 0x7fff).ram();
	map(0x8000, 0x803f).rw("novram", FUNC(x2210_device::read), FUNC(x2210_device::write));
}

void t7000_state::io_map(address_map &map)
{
	map.global_mask(0xff);
	map(0xb0, 0xb1).w(FUNC(t7000_state::crtc_combined_w));
	map(0xb2, 0xb3).rw(m_crtc[0], FUNC(i8275_device::read), FUNC(i8275_device::write));
	map(0xb4, 0xb5).rw(m_crtc[1], FUNC(i8275_device::read), FUNC(i8275_device::write));
	map(0xc0, 0xc3).rw(m_pci[0], FUNC(scn_pci_device::read), FUNC(scn_pci_device::write));
	map(0xd0, 0xd3).rw(m_pci[1], FUNC(scn_pci_device::read), FUNC(scn_pci_device::write));
	map(0xe0, 0xe0).nopw(); // ?
	map(0xf0, 0xf7).w("outlatch", FUNC(ls259_device::write_d0));
}

static INPUT_PORTS_START(t7000)
INPUT_PORTS_END

void t7000_state::t7000(machine_config &config)
{
	Z80(config, m_maincpu, 4_MHz_XTAL);
	m_maincpu->set_addrmap(AS_PROGRAM, &t7000_state::mem_map);
	m_maincpu->set_addrmap(AS_IO, &t7000_state::io_map);

	X2210(config, "novram"); // U39

	SCN2651(config, m_pci[0], 5.0688_MHz_XTAL);

	SCN2651(config, m_pci[1], 5.0688_MHz_XTAL);

	LS259(config, "outlatch"); // U43

	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_raw(19.6608_MHz_XTAL, 1020, 0, 800, 324, 0, 300);
	screen.set_screen_update(m_crtc[0], FUNC(i8275_device::screen_update));

	I8276(config, m_crtc[0], 19.6608_MHz_XTAL / 10);
	m_crtc[0]->set_character_width(10);
	m_crtc[0]->set_display_callback(FUNC(t7000_state::display_character));
	m_crtc[0]->set_screen("screen");

	I8276(config, m_crtc[1], 19.6608_MHz_XTAL / 10);
	m_crtc[1]->set_character_width(10);
	m_crtc[1]->set_screen("screen");
}

ROM_START(t7000)
	ROM_REGION(0x4000, "program", 0)
	ROM_LOAD("t7000_0_8-17-82.u35", 0x0000, 0x1000, CRC(d1645232) SHA1(cdc203942af5b8b3e6bd189c4c7121e480ce1e17)) // all Intel D2732A-3 or MBM2732A-30
	ROM_LOAD("t7000_1_8-17-82.u36", 0x1000, 0x1000, CRC(3441e9cc) SHA1(323d97308170ec6a52a64a60bb8d4554e11e9c12))
	ROM_LOAD("t7000_2_8-17-82.u37", 0x2000, 0x1000, CRC(43a50f3e) SHA1(ae25d3d586ff7027d326e7ce061523b435c2d651))
	ROM_LOAD("t7000_3_8-17-82.u38", 0x3000, 0x1000, CRC(d4ef7293) SHA1(e8c331f629c29d9441723dad9e01f7447638202d))

	ROM_REGION(0x800, "chargen", 0)
	ROM_LOAD("blank.u45", 0x000, 0x800, CRC(0fadcbc8) SHA1(b939b204e76b5d390814a3e575f5473b0a4cbf9d)) // MM2716Q-1
ROM_END

} // anonymous namespace

SYST(1982, t7000, 0, 0, t7000, t7000, t7000_state, empty_init, "Wicat Systems", "T7000 Video Terminal", MACHINE_IS_SKELETON)
