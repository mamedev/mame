// license:BSD-3-Clause
/*
 Sartorius 3733 digital scale

 Visible ICs:
 * Intel D4040 CPU
 * Intel D4289 8-bit memory controller
 * 2 * Intel P4002-1 (64+16)*4 RAM (sockets for 2 * Intel P4002-2 unpopulated)
 * 2 * 2708 1k*8 ROM labelled "129/0" and "129/1"
 * National Semiconductor MM74C907N hex open-drain P-channel buffer
 * Intersil IM6561AIJN 256*4 CMOS RAM
 * RCA CD4007AE dual complementary pair plus inverter
 * National Semicondictor CD4049CN hex inverting buffer
 * 4 * Motorola MC14099B 8-bit addressable latch
 * 2 * RCA CD4052BE 2-channel 4:1 analog switch

 Front panel has a digital display, an "R" button for switching between
 300g and 3kg ranges, and a T button for zeroing.  Rear panel has a
 three-position rotary switch to select between standalone operation and
 data acquisition modes, and a 50-pin micro ribbon connector for data
 communication.

 No dates or other ASCII strings in program ROM.  IC date codes up to
 1978.
 */

#include "emu.h"
#include "cpu/mcs40/mcs40.h"

namespace {

class sart3733_state : public driver_device
{
public:
	sart3733_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag)
	{
	}

	void sart3733(machine_config &config);
};


ADDRESS_MAP_START(sart3733_rom, i4040_cpu_device::AS_ROM, 8, sart3733_state)
	AM_RANGE(0x0000, 0x07ff) AM_ROM AM_REGION("maincpu", 0x0000)
ADDRESS_MAP_END

ADDRESS_MAP_START(sart3733_memory, i4040_cpu_device::AS_RAM_MEMORY, 8, sart3733_state)
	AM_RANGE(0x0000, 0x007f) AM_RAM // 2 * 4002
ADDRESS_MAP_END

ADDRESS_MAP_START(sart3733_status, i4040_cpu_device::AS_RAM_STATUS, 8, sart3733_state)
	AM_RANGE(0x0000, 0x001f) AM_RAM // 2 * 4002
ADDRESS_MAP_END


MACHINE_CONFIG_START(sart3733_state::sart3733)
	MCFG_CPU_ADD("maincpu", I4040, 750'000) // clock speed is a complete guess - can't see crystal or multi-phase clock generator
	MCFG_I4040_ROM_MAP(sart3733_rom)
	MCFG_I4040_RAM_MEMORY_MAP(sart3733_memory)
	MCFG_I4040_RAM_STATUS_MAP(sart3733_status)
MACHINE_CONFIG_END


INPUT_PORTS_START(sart3733)
INPUT_PORTS_END


ROM_START (sart3733)
	ROM_REGION(0x0800, "maincpu", 0)
	ROM_LOAD("129-0", 0x0000, 0x0400, CRC(7ba30b9a) SHA1(40f828145542c2032d30b9921c829406ba9a3679))
	ROM_LOAD("129-1", 0x0400, 0x0400, CRC(a1e2ac6e) SHA1(aaa504274bc077a0de2ba78675d1eb3ad3157b6a))
ROM_END

} // anonymous namespace

SYST( 1978?, sart3733, 0, 0, sart3733, sart3733, sart3733_state, 0, "Sartorius-Werke GmbH", "3733 (digital scale)", MACHINE_NOT_WORKING | MACHINE_NO_SOUND_HW )
