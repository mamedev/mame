// license:BSD-3-Clause
// copyright-holders:
/**************************************************************************************************

Skeleton driver for Clavia Nord Lead/Rack series of virtual analog synthesizer.

**************************************************************************************************/

#include "emu.h"

#include "cpu/m68000/tmp68301.h"

#include "emupal.h"
#include "speaker.h"

namespace {

class nordlead_state : public driver_device
{
public:
	nordlead_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
	{ }

	void nordle2x(machine_config &config);

private:
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;

	void host_map(address_map &map) ATTR_COLD;

	required_device<cpu_device> m_maincpu;
};


void nordlead_state::host_map(address_map &map)
{
	map(0x000000, 0x07ffff).rom();
	map(0x100000, 0x13ffff).ram();
//  map(0x200000, 0x200007) both DSPs
//  map(0x200008, 0x20000f) DSP A
//  map(0x200010, 0x200017) DSP B
//  map(0x202000, 0x2027ff) Front Panel CS6
//  map(0x202800, 0x202fff) Front Panel CS4
//  map(0x203000, 0x2037ff) keyboard
}


static INPUT_PORTS_START(nordle2x)
INPUT_PORTS_END

void nordlead_state::machine_start()
{
}

void nordlead_state::machine_reset()
{
}

void nordlead_state::nordle2x(machine_config &config)
{
	// TODO: actually TMP68331
	TMP68301(config, m_maincpu, XTAL(32'000'000) / 2);
	m_maincpu->set_addrmap(AS_PROGRAM, &nordlead_state::host_map);

	// TODO: 2 x DSP56362, original Nord Lead with stock DSP56002
}


ROM_START(nordle2x)
	ROM_REGION(0x80000, "maincpu", 0)
	ROM_LOAD("nord_lead_2x.bin", 0x00000, 0x80000, CRC(66188eb1) SHA1(549394349f7371e48ead57d8a0ec6b4010014d12))
ROM_END

} // anonymous namespace

//SYST(1995, nordlead, 0, 0, nordlead, nordlead, nordlead_state, empty_init, "Clavia", "Nord Lead", ...)
// ...
SYST(2003, nordle2x, 0, 0, nordle2x,  nordle2x, nordlead_state, empty_init, "Clavia", "Nord Lead 2X", MACHINE_IS_SKELETON )
