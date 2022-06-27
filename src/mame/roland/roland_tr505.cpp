// license:BSD-3-Clause
// copyright-holders:AJR
/**********************************************************************

    Skeleton driver for Roland TR-505 drum machine.

    At present only the sound ROM is dumped. The firmware is entirely
    an internal mask program.

**********************************************************************/

#include "emu.h"
#include "mb63h114.h"
//#include "bus/midi/midi.h"
#include "cpu/m6800/m6801.h"
//#include "machine/nvram.h"
//#include "video/upd7227.h"

class roland_tr505_state : public driver_device
{
public:
	roland_tr505_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_mac(*this, "mac")
	{
	}

	void tr505(machine_config &config);

private:
	void mem_map(address_map &map);

	required_device<hd6301y0_cpu_device> m_maincpu;
	required_device<mb63h114_device> m_mac;
};


void roland_tr505_state::mem_map(address_map &map)
{
	map(0x0000, 0x0027).m(m_maincpu, FUNC(hd6301y0_cpu_device::hd6301y_io));
	map(0x0040, 0x013f).ram(); // internal RAM
	map(0xc000, 0xffff).rom().region("maincpu", 0); // internal ROM
}


static INPUT_PORTS_START(tr505)
INPUT_PORTS_END

void roland_tr505_state::tr505(machine_config &config)
{
	HD6301Y0(config, m_maincpu, 4_MHz_XTAL); // HD6301Y0A in single chip mode (MP0 = MP1 = +5V)
	m_maincpu->set_addrmap(AS_PROGRAM, &roland_tr505_state::mem_map);
	m_maincpu->set_disable();

	//NVRAM(config, "nvram", nvram_device::DEFAULT_ALL_0); // TC5517APL + battery

	//UPD7225(config, "lcdd");

	MB63H114(config, m_mac, 1.6_MHz_XTAL);
}

ROM_START(tr505)
	ROM_REGION(0x4000, "maincpu", 0)
	ROM_LOAD("hd6301y0a99p.ic3", 0x0000, 0x4000, NO_DUMP) // Version 1.1 (690700 & up)
	//ROM_LOAD("hd6301y0a51p.ic3", 0x0000, 0x4000, NO_DUMP) // Version 1.0 (630100–690699)

	ROM_REGION(0x20000, "mac", 0)
	ROM_LOAD("tr-505_rawromdump.bin", 0x00000, 0x20000, CRC(2234c834) SHA1(6441d3e7b53aff4511b23021dc854b7a5cc57689)) // TC531000P
ROM_END

SYST(1986, tr505, 0, 0, tr505, tr505, roland_tr505_state, empty_init, "Roland", "TR-505 Rhythm Composer", MACHINE_IS_SKELETON)
