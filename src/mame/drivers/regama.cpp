// license:BSD-3-Clause
// copyright-holders:
// PINBALL
/*
Skeleton driver for Regama's Trebol.
Hardware listing and ROM definitions from PinMAME.


Hardware:
CPU:   1 x I8085A
IO:    1 x I8155
Sound: 1 x AY8910
*/

#include "emu.h"
#include "machine/genpin.h"
#include "cpu/i8085/i8085.h"
#include "machine/i8155.h"
#include "sound/ay8910.h"

class regama_state : public genpin_class
{
public:
	regama_state(const machine_config &mconfig, device_type type, const char *tag)
		: genpin_class(mconfig, type, tag)
		, m_maincpu(*this, "maincpu") { }

	void regama(machine_config &config);

private:
	void maincpu_map(address_map &map);

	required_device<cpu_device> m_maincpu;
};

void regama_state::maincpu_map(address_map &map)
{
	map(0x0000, 0x1fff).ram();
}


static INPUT_PORTS_START( regama )
	PORT_START("DSW0")
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNKNOWN )
INPUT_PORTS_END


void regama_state::regama(machine_config &config)
{
	I8085A(config, m_maincpu, 6144000 / 2);
	m_maincpu->set_addrmap(AS_PROGRAM, &regama_state::maincpu_map);

	I8155(config, "i8155", 6144000 / 2);

	genpin_audio(config);

	AY8910(config, "ay", 6144000 / 2);
}


ROM_START(trebol)
	ROM_REGION(0x2000, "maincpu", 0)
	ROM_LOAD("m69.bin", 0x0000, 0x2000, CRC(8fb8cd39) SHA1(4ed505d06b489ce83316fdaa39f7ce128011fb4b))
ROM_END


GAME( 1985, trebol, 0, regama, regama, regama_state, empty_init, ROT0, "Regama", "Trebol", MACHINE_IS_SKELETON_MECHANICAL )
