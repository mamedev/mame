// license:BSD-3-Clause
// copyright-holders:David Haywood

// TV Word Processor, with printer

#include "emu.h"
#include "cpu/nec/v25.h"

namespace {

class tvdear_state : public driver_device
{
public:
	tvdear_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "systemcpu")
	{
	}

	void tvdear(machine_config &config);

private:
	void mem_map(address_map &map);

	required_device<v25_device> m_maincpu;
};


void tvdear_state::mem_map(address_map &map)
{
	map(0xe0000, 0xfffff).rom().region("maincpu", 0xe0000); // wrong
}

static INPUT_PORTS_START(tvdear)
INPUT_PORTS_END

void tvdear_state::tvdear(machine_config &config)
{
	V25(config, m_maincpu, 16000000);
	m_maincpu->set_addrmap(AS_PROGRAM, &tvdear_state::mem_map);
}

ROM_START(tvdear)
	ROM_REGION(0x200000, "maincpu", 0)
	ROM_LOAD("d23c160000.u5", 0x00000, 0x200000, CRC(9bb22f7d) SHA1(a9b623e04a9b75d2624201d91d3a57248ae8e268) )

	ROM_REGION(0x20000, "cart", 0) // TODO: move this to a software list
	ROM_LOAD("lc371100.u15", 0x00000, 0x20000, CRC(f70696d1) SHA1(21da45720a48e18fd6173f2482bd7db4988d1548) )
ROM_END

} // anonymous namespace

CONS( 1996, tvdear,  0,          0,  tvdear,  tvdear, tvdear_state, empty_init, "Takara", "TV Dear", MACHINE_IS_SKELETON )
