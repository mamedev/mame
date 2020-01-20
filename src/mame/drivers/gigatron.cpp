// license:BSD-3-Clause
// copyright-holders:Sterophonick
/***************************************************************************

   Skeleton driver for Gigatron TTL Microcomputer
   Driver by Sterophonick

***************************************************************************/

#include "emu.h"
#include "cpu/m6502/m6502.h"
//#include "cpu/gigatron/gigatron.h"
#include "machine/nvram.h"
#include "speaker.h"

#define MAIN_CLOCK 6250000

class gigatron_state : public driver_device
{
public:
	gigatron_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
	{
	}

	void gigatron(machine_config &config);

private:

	required_device<cpu_device> m_maincpu;
};

static INPUT_PORTS_START(gigatron)
INPUT_PORTS_END

void gigatron_state::gigatron(machine_config &config)
{
	M6502(config, m_maincpu, MAIN_CLOCK); // actually its own custom cpu but i cant get it to work
	//GTRON(config, m_maincpu, MAIN_CLOCK);
	SPEAKER(config, "mono").front_center();

}

ROM_START( gigatron )
	ROM_REGION( 0x00000, "maincpu", 0 )
	ROM_LOAD( "gigatron.rom",  0x0000, 0x20000, CRC(78995109) SHA1(2395fc48e64099836111f5aeca39ddbf4650ea4e) )
ROM_END

GAME(199?, gigatron,         0, gigatron, gigatron, gigatron_state, empty_init, ROT0, "Marcel van Kervinck", "Gigatron TTL Microcomputer", MACHINE_IS_SKELETON_MECHANICAL)