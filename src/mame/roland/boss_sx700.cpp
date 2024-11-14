// license:BSD-3-Clause
// copyright-holders:AJR
/****************************************************************************

    Skeleton driver for Boss SX-700 signal processor.

****************************************************************************/

#include "emu.h"
#include "cpu/h8/h83002.h"


namespace {

class boss_sx700_state : public driver_device
{
public:
	boss_sx700_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
	{
	}

	void sx700(machine_config &config);
	void gx700(machine_config &config);

private:
	void sx700_map(address_map &map) ATTR_COLD;
	void gx700_map(address_map &map) ATTR_COLD;

	required_device<h83002_device> m_maincpu;
};


void boss_sx700_state::sx700_map(address_map &map)
{
	map(0x000000, 0x01ffff).rom().region("program", 0);
	map(0x020000, 0x02ffff).ram();

}

void boss_sx700_state::gx700_map(address_map &map)
{
	map(0x000000, 0x01ffff).rom().region("program", 0);
	map(0x020000, 0x027fff).ram();
	map(0x028000, 0x03ffff).rom().region("program", 0x28000);
}


static INPUT_PORTS_START(sx700)
INPUT_PORTS_END

void boss_sx700_state::sx700(machine_config &config)
{
	H83002(config, m_maincpu, 16_MHz_XTAL); // HD6413002F
	// TODO: operates in mode 3 with 8-bit data bus
	m_maincpu->set_addrmap(AS_PROGRAM, &boss_sx700_state::sx700_map);

	//TC170C140AF_003(config, "dsp", 67.7376_MHz_XTAL);
}

void boss_sx700_state::gx700(machine_config &config)
{
	sx700(config);
	m_maincpu->set_addrmap(AS_PROGRAM, &boss_sx700_state::gx700_map);
}

ROM_START(sx700)
	ROM_REGION16_BE(0x80000, "program", 0)
	ROM_LOAD("sx-700_1_0_2.ic17", 0x00000, 0x80000, CRC(6739f525) SHA1(3370f43fd586baa0bcc71891a766b45e1d42253d)) // M27C4001-10F1
ROM_END

ROM_START(gx700)
	ROM_REGION16_BE(0x40000, "program", 0)
	ROM_SYSTEM_BIOS(0, "v110", "v1.10 WATERDRAGON")
	ROMX_LOAD("gx-700_1_1_0.ic20", 0x00000, 0x40000, CRC(feb8a186) SHA1(5f039ff1aa45ed4a33ecd8fd9ad39880646999cb), ROM_BIOS(0)) // M27C2001-10F1
	ROM_SYSTEM_BIOS(1, "v109", "v1.09 WATERDRAGON")
	ROMX_LOAD("gx-700_1_0_9.ic20", 0x00000, 0x40000, CRC(e38b3eeb) SHA1(8ce0563b70d37103acafe1578706ec1c32419e34), ROM_BIOS(1)) // M27C2001-10F1
ROM_END

} // anonymous namespace


SYST(1996, sx700, 0, 0, sx700, sx700, boss_sx700_state, empty_init, "Roland", "Boss SX-700 Studio Effects Processor", MACHINE_IS_SKELETON)
SYST(1996, gx700, 0, 0, gx700, sx700, boss_sx700_state, empty_init, "Roland", "Boss GX-700 Guitar Effects Processor", MACHINE_IS_SKELETON)
