// license:BSD-3-Clause
// copyright-holders:AJR
/***************************************************************************

    Skeleton driver for Kawai K5 synthesizer.

***************************************************************************/

#include "emu.h"
#include "cpu/nec/v5x.h"
#include "video/t6963c.h"


namespace {

class kawai_k5_state : public driver_device
{
public:
	kawai_k5_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
	{
	}

	void k5(machine_config &config);

private:
	void mem_map(address_map &map) ATTR_COLD;
	void lcd_map(address_map &map) ATTR_COLD;

	required_device<v40_device> m_maincpu;
};


void kawai_k5_state::mem_map(address_map &map)
{
	map(0x00000, 0x0ffff).rom().region("coderom", 0);
	map(0x11000, 0x117ff).ram();
	map(0x14000, 0x142ff).ram();
	map(0x16800, 0x16801).rw("lcdc", FUNC(t6963c_device::read), FUNC(t6963c_device::write));
	map(0x17000, 0x17000).lr8([]() { return 0x04; }, "unknown_poll_r");
	map(0x17800, 0x17800).nopw();
	map(0x18000, 0x1ffff).ram();
	map(0x20000, 0x2ffff).ram();
	map(0xf0000, 0xfffff).rom().region("coderom", 0);
}

void kawai_k5_state::lcd_map(address_map &map)
{
	map(0x0000, 0x1fff).ram();
}


static INPUT_PORTS_START(k5)
INPUT_PORTS_END

void kawai_k5_state::k5(machine_config &config)
{
	V40(config, m_maincpu, 16'000'000); // XTAL unknown
	m_maincpu->set_addrmap(AS_PROGRAM, &kawai_k5_state::mem_map);

	T6963C(config, "lcdc", 0).set_addrmap(0, &kawai_k5_state::lcd_map);
}

ROM_START(k5)
	ROM_REGION(0x10000, "coderom", 0)
	//ROM_SYSTEM_BIOS(0, "v1.3", "Version 1.3")
	ROM_LOAD("k5_1.3.bin", 0x00000, 0x10000, CRC(cbefe520) SHA1(8c53867ebc403d24320a57a5ff2d37ab6c1fc994))

	ROM_REGION(0x400, "lcdc:cgrom", ROMREGION_ERASE00)
ROM_END

ROM_START(k5m)
	ROM_REGION(0x10000, "coderom", 0)
	//ROM_SYSTEM_BIOS(0, "v1.2", "Version 1.2")
	ROM_LOAD("k5m_1.2.bin", 0x00000, 0x10000, CRC(d2ab0fac) SHA1(34f0c2685b39e459a915ed5effa5b7a4cd2a1f8a))

	ROM_REGION(0x400, "lcdc:cgrom", ROMREGION_ERASE00)
ROM_END

} // anonymous namespace


SYST(1987, k5,  0,  0, k5, k5, kawai_k5_state, empty_init, "Kawai Musical Instrument Manufacturing", "K5 Digital Multi-Dimensional Synthesizer",         MACHINE_IS_SKELETON)
SYST(1987, k5m, k5, 0, k5, k5, kawai_k5_state, empty_init, "Kawai Musical Instrument Manufacturing", "K5m Digital Multi-Dimensional Synthesizer Module", MACHINE_IS_SKELETON)
