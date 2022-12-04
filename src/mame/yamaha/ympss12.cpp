// license:BSD-3-Clause
// copyright-holders:Devin Acker

/*

    Skeleton driver for Yamaha YMW270-F (GEW7) and YMW282-F (GEW7S) keyboards

*/

#include "emu.h"

#include "cpu/m6502/m65c02.h"
#include "speaker.h"

namespace {

class pss12_state : public driver_device
{
public:
	pss12_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
	{ }

	void pss12(machine_config &config);

private:
	void pss12_map(address_map &map);

	required_device<m65c02_device> m_maincpu;
};

void pss12_state::pss12_map(address_map &map)
{
	map(0x0000, 0x03ff).ram();
	map(0x4000, 0xbfff).rom().region("maincpu", 0x8000); // probably bankswitched (regs at $0408-0409?)
	map(0xc000, 0xffff).rom().region("maincpu", 0x0000);
}

INPUT_PORTS_START( pss12 )
INPUT_PORTS_END

void pss12_state::pss12(machine_config &config)
{
	M65C02(config, m_maincpu, 2'000'000); // TODO: YMW282-F, clock unknown
	m_maincpu->set_addrmap(AS_PROGRAM, &pss12_state::pss12_map);

	SPEAKER(config, "speaker", 0).front_center();
}

ROM_START( pss12 )
	ROM_REGION( 0x40000, "maincpu", 0 )
	ROM_LOAD( "yamaha_pss12.bin", 0x00000, 0x40000, CRC(7e05f1cb) SHA1(1a05996002bb7bfdde215349d235269795c88693))
ROM_END

} // anonymous namespace

SYST( 1994, pss12, 0, 0, pss12, pss12, pss12_state, empty_init, "Yamaha", "PSS-12", MACHINE_IS_SKELETON )
