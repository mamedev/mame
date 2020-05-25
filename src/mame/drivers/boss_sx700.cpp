// license:BSD-3-Clause
// copyright-holders:AJR
/****************************************************************************

    Skeleton driver for Boss SX-700 signal processor.

****************************************************************************/

#include "emu.h"
#include "cpu/h8/h83002.h"

class boss_sx700_state : public driver_device
{
public:
	boss_sx700_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
	{
	}

	void sx700(machine_config &config);

private:
	void mem_map(address_map &map);

	required_device<h83002_device> m_maincpu;
};


void boss_sx700_state::mem_map(address_map &map)
{
	map(0x000000, 0x07ffff).rom().region("program", 0);
}


static INPUT_PORTS_START(sx700)
INPUT_PORTS_END

void boss_sx700_state::sx700(machine_config &config)
{
	H83002(config, m_maincpu, 16_MHz_XTAL); // HD6413002F
	// TODO: operates in mode 3 with 8-bit data bus
	m_maincpu->set_addrmap(AS_PROGRAM, &boss_sx700_state::mem_map);

	//TC170C140AF_003(config, "dsp", 67.7376_MHz_XTAL);
}

ROM_START(sx700)
	ROM_REGION16_BE(0x80000, "program", 0)
	ROM_LOAD("sx-700_1_0_2.ic17", 0x00000, 0x80000, CRC(6739f525) SHA1(3370f43fd586baa0bcc71891a766b45e1d42253d)) // M27C4001-10F1
ROM_END

SYST(1996, sx700, 0, 0, sx700, sx700, boss_sx700_state, empty_init, "Roland", "Boss SX-700 Studio Effects Processor", MACHINE_IS_SKELETON)
