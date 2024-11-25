// license:BSD-3-Clause
// copyright-holders:
/****************************************************************************

    Skeleton driver for Roland MT-80s MIDI player.

****************************************************************************/

#include "emu.h"

//#include "bus/midi/midi.h"
#include "cpu/f2mc16/f2mc16.h"
#include "imagedev/floppy.h"


namespace {

class roland_mt80s_state : public driver_device
{
public:
	roland_mt80s_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
	{
	}

	void mt80s(machine_config &config);

private:
	void mem_map(address_map &map) ATTR_COLD;

	required_device<cpu_device> m_maincpu;
	//required_device<hd63266_device> m_hdfdc;
};

void roland_mt80s_state::mem_map(address_map &map)
{
	map(0x000100, 0x0008ff).ram(); // TODO: should be mapped as internal memory by device
	map(0x100000, 0x13ffff).ram();
	map(0xf80000, 0xffffff).rom().region("maincpu", 0);
}

static INPUT_PORTS_START(mt80s)
INPUT_PORTS_END

void roland_mt80s_state::mt80s(machine_config &config)
{
	F2MC16(config, m_maincpu, 24_MHz_XTAL); // actually an MB90705H
	m_maincpu->set_addrmap(AS_PROGRAM, &roland_mt80s_state::mem_map);

	// HD63266(config, m_hdfdc, 16_MHz_XTAL); // HD63266F

	// TC6116(config, "pcm", 16_MHz_XTAL); // TC6116AF
}

ROM_START(mt80s)
	ROM_REGION(0x80000, "maincpu", 0)
	ROM_LOAD("mt80s_101.ic4", 0x00000, 0x80000, CRC(c23f5cc4) SHA1(8583aaa146ac51ab0b8d62cec0403a89130f1389))

	ROM_REGION(0x100000, "wave", 0)
	ROM_LOAD("mb838000.ic7", 0x000000, 0x100000, NO_DUMP)
ROM_END

} // anonymous namespace


SYST(1995, mt80s,    0, 0, mt80s,    mt80s, roland_mt80s_state, empty_init, "Roland", "MT 80s Music Player", MACHINE_IS_SKELETON)
