// license:BSD-3-Clause
// copyright-holders:
/****************************************************************************************
    Skeleton driver for Inor "bubble" electromechanical machines.
    Known games with this PCB:
      -Olimpic Hockey (latest generation, there's an older version on different hardware)
      -Futbol Champ

    Hardware:
      -8080
      -MEA-8000 Voice Synth
      -AY-3-8910
      -MOSEL MS6516L-10PC 2Kx8 Static RAM
****************************************************************************************/

#include "emu.h"
#include "cpu/i8085/i8085.h"
#include "sound/ay8910.h"
#include "sound/mea8000.h"
#include "speaker.h"

namespace {

class inorbubble_state : public driver_device
{

public:
	inorbubble_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
	{
	}

	void inorbubble(machine_config &config);

private:
	required_device<cpu_device> m_maincpu;
};

INPUT_PORTS_START(inorbubble)
INPUT_PORTS_END

void inorbubble_state::inorbubble(machine_config &config)
{
	// Basic machine hardware
	I8080(config, m_maincpu, 4'000'000); // Unknown clock // AMD 8080 CPU, exact model unknown

	// Sound hardware
	SPEAKER(config, "measnd").front_center();
	MEA8000(config, "mea8000", 4'000'000).add_route(ALL_OUTPUTS, "measnd", 1.0); // Unknown clock
	SPEAKER(config, "aysnd").front_center();
	AY8910(config, "ay", 4'000'000).add_route(ALL_OUTPUTS, "aysnd", 0.25); // Unknown clock
}

ROM_START(olihock)
	ROM_REGION(0x4000, "program", 0)
	ROM_LOAD("d27128d.bin", 0x0000, 0x4000, CRC(6ea22e0a) SHA1(6be260e7366c1f5be4f0773818da96d5aa93483b))
ROM_END

} // Anonymous namespace

GAME(1986?, olihock, 0, inorbubble, inorbubble, inorbubble_state, empty_init, ROT0, "Inor", "Olimpic Hockey (EM Bubble Hockey)", MACHINE_IS_SKELETON_MECHANICAL)
