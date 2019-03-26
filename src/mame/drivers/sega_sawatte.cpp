// license:BSD-3-Clause
// copyright-holders:

/* Sega Sawatte / S-Pico

a sound-only Pico type system (one of the boards even says S-PICO)

CPU is unknown (I can't see one?! MCU with internal ROM?) cartridge dumps have been tested as working using a flash cart.
driver does nothing except allow the softlist to be connected to the -romident commands etc.


images supplied by Team Europe

http://mamedev.emulab.it/haze/reference/sawatte/cartridge_pcb_front.jpg
http://mamedev.emulab.it/haze/reference/sawatte/cartridge_pcb_back.jpg

http://mamedev.emulab.it/haze/reference/sawatte/PCB_Front.jpg
http://mamedev.emulab.it/haze/reference/sawatte/PCB_Back.jpg

http://mamedev.emulab.it/haze/reference/sawatte/Console_Front.JPG
http://mamedev.emulab.it/haze/reference/sawatte/Console_Back.JPG

http://mamedev.emulab.it/haze/reference/sawatte/cartridge_example.jpg

*/

#include "emu.h"
#include "softlist_dev.h"


class sawatte_state : public driver_device
{
public:
	sawatte_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
	{ }

	void sawatte(machine_config &config);
protected:
};


static INPUT_PORTS_START( sawatte )
INPUT_PORTS_END


void sawatte_state::sawatte(machine_config &config)
{
	SOFTWARE_LIST(config, "cart_list").set_original("sawatte");
}

ROM_START( sawatte )
ROM_END


CONS( 1996?, sawatte, 0, 0, sawatte,  sawatte, sawatte_state, empty_init, "Sega", "Sawatte", MACHINE_IS_SKELETON )
