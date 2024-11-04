// license:BSD-3-Clause
// copyright-holders:AJR
/****************************************************************************

    Skeleton driver for Waldorf MiniWorks 4-Pole analog filter module.

****************************************************************************/

#include "emu.h"
#include "cpu/mc68hc11/mc68hc11.h"

namespace {

class mw4pole_state : public driver_device
{
public:
	mw4pole_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
	{
	}

	void mw4pole(machine_config &config);

private:
	void mem_map(address_map &map) ATTR_COLD;

	required_device<mc68hc11_cpu_device> m_maincpu;
};


void mw4pole_state::mem_map(address_map &map)
{
	map(0x8000, 0xffff).rom().region("eprom", 0);
}

static INPUT_PORTS_START(mw4pole)
INPUT_PORTS_END

void mw4pole_state::mw4pole(machine_config &config)
{
	MC68HC11E1(config, m_maincpu, 8_MHz_XTAL);
	m_maincpu->set_addrmap(AS_PROGRAM, &mw4pole_state::mem_map);
}

ROM_START(mw4pole)
	ROM_REGION(0x8000, "eprom", 0)
	ROM_LOAD("4-pole v1.48 001eaf32.bin", 0x0000, 0x8000, CRC(51be6962) SHA1(20e793573a49002c854b012280aed13058ba0b63)) // TMS27C256
ROM_END

} // anonymous namespace

SYST(1995, mw4pole, 0, 0, mw4pole, mw4pole, mw4pole_state, empty_init, "Waldorf Electronics", "MiniWorks 4-Pole", MACHINE_IS_SKELETON)
