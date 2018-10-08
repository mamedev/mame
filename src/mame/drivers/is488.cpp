// license:BSD-3-Clause
// copyright-holders:AJR
/***********************************************************************************************************************************

    Skeleton driver for Decision Data IS-488 IBM 3488-compatible workstation display.

***********************************************************************************************************************************/

#include "emu.h"
#include "cpu/i86/i186.h"
//#include "cpu/bcp/bcp.h"
//#include "machine/eeprompar.h"
//#include "video/mc6845.h"
//#include "screen.h"

class is488_state : public driver_device
{
public:
	is488_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
	{ }

	void is488a(machine_config &config);

private:
	void mem_map(address_map &map);
	void io_map(address_map &map);

	required_device<cpu_device> m_maincpu;
};

void is488_state::mem_map(address_map &map)
{
	map(0x00000, 0x07fff).ram();
	map(0x40000, 0x47fff).ram();
	map(0x50000, 0x51fff).ram();
	map(0x54000, 0x55fff).ram();
	map(0x60022, 0x60022).nopr();
	map(0x61ffa, 0x61ffa).nopr();
	map(0x80000, 0xfffff).rom().region("program", 0);
}

void is488_state::io_map(address_map &map)
{
	map(0x8005, 0x8005).nopw();
	//map(0x8080, 0x8080).w("crtc", FUNC(mc6845_device::address_w));
	//map(0x8081, 0x8081).rw("crtc", FUNC(mc6845_device::register_r), FUNC(mc6845_device::register_w));
	map(0x8101, 0x8101).nopr();
	map(0x8180, 0x8180).ram();
}

static INPUT_PORTS_START(is488a)
INPUT_PORTS_END

void is488_state::is488a(machine_config &config)
{
	I80188(config, m_maincpu, 16_MHz_XTAL);
	m_maincpu->set_addrmap(AS_PROGRAM, &is488_state::mem_map);
	m_maincpu->set_addrmap(AS_IO, &is488_state::io_map);

	//DP8344(config, "bcp", 18.867_MHz_XTAL);
}

ROM_START(is488a)
	ROM_REGION(0x80000, "program", 0)
	ROM_LOAD("is-482_u67_s008533243.bin", 0x00000, 0x80000, CRC(1e23ac17) SHA1(aadc73bc0454c5b1c33d440dc511009dc6b7f9e0))
ROM_END

COMP(199?, is488a, 0, 0, is488a, is488a, is488_state, empty_init, "Decision Data", "IS-488-A Workstation", MACHINE_IS_SKELETON)
