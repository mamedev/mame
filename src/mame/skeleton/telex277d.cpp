// license:BSD-3-Clause
// copyright-holders:AJR
/************************************************************************************************************

    Skeleton driver for Telex 277/D IBM 3277-compatible coaxial terminal.

************************************************************************************************************/

#include "emu.h"
#include "cpu/i8085/i8085.h"
#include "machine/i8155.h"


namespace {

class telex277d_state : public driver_device
{
public:
	telex277d_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_chargen(*this, "chargen")
	{
	}

	void telex277d(machine_config &config);

private:
	void mem_map(address_map &map) ATTR_COLD;
	void io_map(address_map &map) ATTR_COLD;

	required_device<cpu_device> m_maincpu;
	required_region_ptr<u8> m_chargen;
};


void telex277d_state::mem_map(address_map &map)
{
	map(0x0000, 0x1fff).rom().region("program", 0);
	map(0x4000, 0x47ff).ram();
	map(0x8000, 0x80ff).rw("ramio", FUNC(i8155_device::memory_r), FUNC(i8155_device::memory_w));
}

void telex277d_state::io_map(address_map &map)
{
	map(0x80, 0x87).rw("ramio", FUNC(i8155_device::io_r), FUNC(i8155_device::io_w));
}


static INPUT_PORTS_START(telex277d)
INPUT_PORTS_END


void telex277d_state::telex277d(machine_config &config)
{
	I8085A(config, m_maincpu, 6.144_MHz_XTAL);
	m_maincpu->set_addrmap(AS_PROGRAM, &telex277d_state::mem_map);
	m_maincpu->set_addrmap(AS_IO, &telex277d_state::io_map);

	I8155(config, "ramio", 6.144_MHz_XTAL / 2);
}


ROM_START(telex277d) // D8085A, P8155. bank of 8 dips between these 2 chips. Xtals on cpu board = 6.144,14.286MHz; Xtal on video board = 16.414
	ROM_REGION(0x2000, "program", 0)
	ROM_LOAD("15834.bin", 0x0000, 0x0800, CRC(364602ed) SHA1(574b1052ab000cfb9e7f194454de65f5255c250e))
	ROM_LOAD("15835.bin", 0x0800, 0x0800, CRC(b587d005) SHA1(de38b1dcbb871dc5f7dcbc177dfcbc25ecc743c4))
	ROM_LOAD("15836.bin", 0x1000, 0x0800, CRC(33a7179f) SHA1(c4b2a8f9d2b3e2f97c7d9b5458cad314fc8c79c1))
	ROM_LOAD("15837.bin", 0x1800, 0x0800, CRC(6d726662) SHA1(cff3ea2f06b802b94acfb780014d0e389cb61c42))

	ROM_REGION(0x0400, "chargen", 0)
	ROM_LOAD("15181_font.bin", 0x0000, 0x0400, CRC(2a7abd0b) SHA1(4456723c59307671dd0615723e6439f6532df531))
ROM_END

} // anonymous namespace


COMP(1979, telex277d, 0, 0, telex277d, telex277d, telex277d_state, empty_init, "Telex Computer Products", "Telex 277-D Display Terminal (Model 2)", MACHINE_IS_SKELETON)
