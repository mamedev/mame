// license:BSD-3-Clause
// copyright-holders:

/* Sega Sawatte / S-Pico

a sound-only Pico type system (one of the boards even says S-PICO)

CPU is unknown (I can't see one?!) cartridge dumps should be good, but not confirmed, might be data only for an MCU?
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



class sawatte_state : public driver_device
{
public:
	sawatte_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
	{ }

protected:
};


static INPUT_PORTS_START( sawatte )
INPUT_PORTS_END


static MACHINE_CONFIG_START( sawatte, sawatte_state )
	MCFG_SOFTWARE_LIST_ADD("cart_list", "sawatte")
MACHINE_CONFIG_END

ROM_START( sawatte )
ROM_END


GAME( 1996?, sawatte,    0,       sawatte,  sawatte, driver_device,  0,  ROT0,  "Sega",    "Sawatte",          MACHINE_IS_SKELETON )
