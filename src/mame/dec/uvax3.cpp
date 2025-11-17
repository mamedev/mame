// license:BSD-3-Clause
// copyright-holders:AJR
/*******************************************************************************

    Skeleton driver for DEC third-generation MicroVAX models ("Mayfair").

*******************************************************************************/

#include "emu.h"

//#include "bus/dssi/dssi.h"
//#include "bus/qbus/qbus.h"
#include "cpu/vax/vax.h"
//#include "machine/am79c90.h"

namespace {

class uvax3_state : public driver_device
{
public:
	uvax3_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
	{
	}

	void mv3400(machine_config &config) ATTR_COLD;

private:
	void mem_map(address_map &map) ATTR_COLD;

	required_device<cpu_device> m_maincpu;
};

void uvax3_state::mem_map(address_map &map)
{
	map(0x20040000, 0x2005ffff).mirror(0x20000).rom().region("firmware", 0);
}

static INPUT_PORTS_START(mv3400)
INPUT_PORTS_END

void uvax3_state::mv3400(machine_config &config)
{
	DC341(config, m_maincpu, 40_MHz_XTAL / 2);
	m_maincpu->set_addrmap(AS_PROGRAM, &uvax3_state::mem_map);

	//AM7990(config, "lance", ?);
}

ROM_START(mv3400)
	ROM_REGION32_LE(0x20000, "firmware", 0)
	ROM_LOAD16_BYTE("m7624_dec88_lm8914_152e7_d27512.bin", 0x00000, 0x10000, CRC(eb61f8d0) SHA1(17d56f59120881df2b6522097998b4a9e4bac77e))
	ROM_LOAD16_BYTE("m7624_dec88_lm8914_153e7_d27512.bin", 0x00001, 0x10000, CRC(6727bff2) SHA1(4f566e82a8fa8490056d62b3e44c070ba5ab1754))
ROM_END

ROM_START(mv3500)
	ROM_REGION32_LE(0x20000, "firmware", 0)
	ROM_LOAD16_BYTE("ka650-a-v5.3-vmb2.7_192e7.bin", 0x00000, 0x10000, CRC(a3ec59a6) SHA1(6d0121d7e232c841484a328340c828a4f0fdf903))
	ROM_LOAD16_BYTE("ka650-a-v5.3-vmb2.7_193e7.bin", 0x00001, 0x10000, CRC(873ee8bd) SHA1(b0abfebda60e9394e1045644634a8845b5573d04))
ROM_END

} // anonymous namespace

COMP(1988, mv3400, 0, 0, mv3400, mv3400, uvax3_state, empty_init, "Digital Equipment Corporation", "MicroVAX 3400", MACHINE_NOT_WORKING | MACHINE_NO_SOUND_HW)
COMP(1988, mv3500, 0, 0, mv3400, mv3400, uvax3_state, empty_init, "Digital Equipment Corporation", "MicroVAX 3500", MACHINE_NOT_WORKING | MACHINE_NO_SOUND_HW)
