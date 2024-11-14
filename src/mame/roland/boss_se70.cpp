// license:BSD-3-Clause
// copyright-holders:AJR
/****************************************************************************

    Skeleton driver for Boss SE-70 signal processor.

****************************************************************************/

#include "emu.h"
#include "cpu/h8500/h8510.h"


namespace {

class boss_se70_state : public driver_device
{
public:
	boss_se70_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
	{
	}

	void se70(machine_config &config);

private:
	void mem_map(address_map &map) ATTR_COLD;

	required_device<h8510_device> m_maincpu;
};


void boss_se70_state::mem_map(address_map &map)
{
	map(0x000000, 0x07ffff).mirror(0xf00000).rom().region("program", 0);
}


static INPUT_PORTS_START(se70)
INPUT_PORTS_END

void boss_se70_state::se70(machine_config &config)
{
	HD6415108(config, m_maincpu, 16_MHz_XTAL);
	// TODO: operates in mode 3 with 8-bit data bus
	m_maincpu->set_addrmap(AS_PROGRAM, &boss_se70_state::mem_map);

	//TC6088AF(config, "csp", 65.152_MHz_XTAL);
	//UPD65622GF040(config, "interface", 24.576_MHz_XTAL);
}

ROM_START(se70)
	ROM_REGION16_BE(0x80000, "program", 0)
	ROM_LOAD("boss_se-70_v1.01.ic29", 0x00000, 0x80000, CRC(f19151f3) SHA1(6c0de1e0debe72374802d54f8d37517b3ad0b131)) // 27C4001
ROM_END

} // anonymous namespace


SYST(1993, se70, 0, 0, se70, se70, boss_se70_state, empty_init, "Roland", "Boss SE-70 Super Effects Processor", MACHINE_IS_SKELETON)
