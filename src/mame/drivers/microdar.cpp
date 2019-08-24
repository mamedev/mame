// license:BSD-3-Clause
// copyright-holders:AJR
/*************************************************************************

    Skeleton driver for Compumatic dart board with LED display.

**************************************************************************
_______________________________
|__  ________________________  |
||C| |_CN____________________| |
||N|    __   __   __   __   __ |
|| |    |IC  |IC  |IC  |IC  |CN|
||_|    |19  |20  |18  |21  |_||
| __    |_|  |_|  |_|  |_|  __ |
| |CN   __   ______________ |_| CN
| |_|   |IC  |IC17 REF0032 || ||
| :..   |25  |___9817K0552_||_||
| __    |_| ______ ________ |_||
| |CN   __  |IC23_||IC15   || ||
| | |   |IC ______ |_______||_||
| | |   |24 |IC22_|         |_||
| | |   |_| ______   BATT   __ |
| | |   __  |IC12_|         | | CN
| | |   IC8 ______________  | ||
| |_|   __  |IC1  REF34VA | | ||
| __    IC7 |______9818h-_|XT1||
| |CN   ______  __________  | ||
| | |   |IC5__||IC4_______| | ||
| | |   ______ ___________  |_||
| | |   |IC6__||IC3       | __ |
| | |          |__________| |CN|
| | |          _______         |
| | |          |IC2___|        |
| |_|          _______         |
| __           |IC10__|        |
| |CN          _______         |
| | |          |IC9___|        |
| | |      __  _______       : |
| |_|     IC14 |IC13__|        |
|______________________________|

IC19, IC20, IC18, IC21 = Toshiba TD62703AP
IC25, IC24 = ST ULN2803A
IC17 = REF0032 9817K0552 (40-pin DIP: display controller?)
IC23, IC22, IC12 = Hitachi HD74HC273P
IC15 = Natsemi CD4514BCN
IC8 = Microchip 24LC16B
IC7 = TI TL7705ACP
IC1 = Philips REF34VA 9818h- (40-pin DIP: 80C51?)
XT1 = 20.000 MHz
IC5 = Hitachi HD74HC08P
IC6 = Hitachi HD74HC138P
IC4 = LGS GM76C88ALK-15
IC3 = Winbond W29EE011-15
IC2 = Hitachi HD74HC373P
IC2 = Hitachi HD74HC373P
IC10 = Hitachi HD74HC244P

**************************************************************************

The "Sagitario" darts machine from CIC Play S.L. uses the same PCB with a different program ROM (from CIC Play).
The chip at IC1 on the CIC Play PCB is silkscreened as "REF 0034 9115S", and the other at IC17 has
its surface scratched out.

*/

#include "emu.h"
#include "cpu/mcs51/mcs51.h"
#include "machine/i2cmem.h"
#include "machine/nvram.h"

class microdar_state : public driver_device
{
public:
	microdar_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_eeprom(*this, "eeprom")
	{
	}

	void microdar(machine_config &config);

private:
	void prog_map(address_map &map);
	void ext_map(address_map &map);

	required_device<mcs51_cpu_device> m_maincpu;
	required_device<i2cmem_device> m_eeprom;
};

void microdar_state::prog_map(address_map &map)
{
	map(0x1000, 0xffff).rom().region("program", 0x1000);
}

void microdar_state::ext_map(address_map &map)
{
	map(0x0000, 0x1fff).ram().share("nvram");
}

static INPUT_PORTS_START(microdar)
INPUT_PORTS_END

void microdar_state::microdar(machine_config &config)
{
	I80C51(config, m_maincpu, 20_MHz_XTAL);
	m_maincpu->set_addrmap(AS_PROGRAM, &microdar_state::prog_map);
	m_maincpu->set_addrmap(AS_IO, &microdar_state::ext_map);

	NVRAM(config, "nvram", nvram_device::DEFAULT_ALL_0); // GM76C88ALK-15 + battery

	I2CMEM(config, m_eeprom).set_page_size(16).set_data_size(0x800);

	// Code also references some sort of serial RTC?
}


ROM_START(microdar)
	ROM_REGION(0x1000, "maincpu", ROMREGION_ERASE00)
	ROM_LOAD("ref34va_k7v5534_9818h-.ic1", 0x0000, 0x1000, NO_DUMP)
	ROM_FILL(0x0000, 1, 0x02) // temporary LJMP to external init code
	ROM_FILL(0x0001, 1, 0x10)
	ROM_FILL(0x0002, 1, 0x1b)
	ROM_FILL(0x000b, 1, 0x02) // temporary LJMP to interrupt handler
	ROM_FILL(0x000c, 1, 0x10)
	ROM_FILL(0x000d, 1, 0x15)
	ROM_FILL(0x001b, 1, 0x02) // temporary LJMP to interrupt handler
	ROM_FILL(0x001c, 1, 0x10)
	ROM_FILL(0x001d, 1, 0x18)
	ROM_FILL(0x0058, 1, 0x32) // RETI stubs
	ROM_FILL(0x00af, 1, 0x32)
	ROM_FILL(0x00c9, 1, 0x02) // temporary LJMP to end of interrupt handler
	ROM_FILL(0x00ca, 1, 0x12)
	ROM_FILL(0x00cb, 1, 0xd1)
	ROM_FILL(0x0135, 1, 0x22) // RET stubs
	ROM_FILL(0x0163, 1, 0x22)
	ROM_FILL(0x0185, 1, 0x22)
	ROM_FILL(0x01cf, 1, 0x22)
	ROM_FILL(0x02de, 1, 0x22)
	ROM_FILL(0x02fa, 1, 0x22)
	ROM_FILL(0x0308, 1, 0x22)
	ROM_FILL(0x0313, 1, 0x22)
	ROM_FILL(0x037d, 1, 0x22)
	ROM_FILL(0x03be, 1, 0x22)
	ROM_FILL(0x0496, 1, 0x22)
	ROM_FILL(0x04ce, 1, 0x22)
	ROM_FILL(0x0514, 1, 0x22)
	ROM_FILL(0x0520, 1, 0x22)
	ROM_FILL(0x0550, 1, 0x22)
	ROM_FILL(0x0574, 1, 0x22)
	ROM_FILL(0x05b4, 1, 0x22)
	ROM_FILL(0x05bb, 1, 0x22)
	ROM_FILL(0x05ca, 1, 0x22)
	ROM_FILL(0x05d8, 1, 0x22)
	ROM_FILL(0x0605, 1, 0x22)
	ROM_FILL(0x0638, 1, 0x22)
	ROM_FILL(0x068a, 1, 0x22)
	ROM_FILL(0x06a2, 1, 0x22)
	ROM_FILL(0x06bd, 1, 0x22)
	ROM_FILL(0x06db, 1, 0x22)
	ROM_FILL(0x0708, 1, 0x22)
	ROM_FILL(0x0715, 1, 0x22)
	ROM_FILL(0x072d, 1, 0x22)
	ROM_FILL(0x0744, 1, 0x22)
	ROM_FILL(0x0751, 1, 0x22)
	ROM_FILL(0x0767, 1, 0x22)
	ROM_FILL(0x0772, 1, 0x22)
	ROM_FILL(0x077d, 1, 0x22)
	ROM_FILL(0x07a4, 1, 0x22)
	ROM_FILL(0x07c2, 1, 0x22)
	ROM_FILL(0x0802, 1, 0x22)
	ROM_FILL(0x0836, 1, 0x22)
	ROM_FILL(0x087d, 1, 0x22)
	ROM_FILL(0x0893, 1, 0x22)
	ROM_FILL(0x0930, 1, 0x22)
	ROM_FILL(0x094a, 1, 0x22)
	ROM_FILL(0x095a, 1, 0x22)
	ROM_FILL(0x096a, 1, 0x22)
	ROM_FILL(0x097b, 1, 0x22)
	ROM_FILL(0x098f, 1, 0x22)
	ROM_FILL(0x09a3, 1, 0x22)
	ROM_FILL(0x09c0, 1, 0x22)
	ROM_FILL(0x0a21, 1, 0x22)
	ROM_FILL(0x0a54, 1, 0x22)
	ROM_FILL(0x0a63, 1, 0x22)
	ROM_FILL(0x0a73, 1, 0x22)
	ROM_FILL(0x0a90, 1, 0x22)
	ROM_FILL(0x0ae8, 1, 0x22)
	ROM_FILL(0x0abf, 1, 0x22)
	ROM_FILL(0x0ac7, 1, 0x22)
	ROM_FILL(0x0b11, 1, 0x22)
	ROM_FILL(0x0b7f, 1, 0x22)
	ROM_FILL(0x0bf1, 1, 0x22)
	ROM_FILL(0x0bf6, 1, 0x22)
	ROM_FILL(0x0c4a, 1, 0x22)
	ROM_FILL(0x0c59, 1, 0x22)
	ROM_FILL(0x0c64, 1, 0x22)
	ROM_FILL(0x0c99, 1, 0x22)
	ROM_FILL(0x0ca8, 1, 0x22)
	ROM_FILL(0x0cbd, 1, 0x22)
	ROM_FILL(0x0dac, 1, 0x22)
	ROM_FILL(0x0dca, 1, 0x22)
	ROM_FILL(0x0e6a, 1, 0x22)

	ROM_REGION(0x20000, "program", 0)
	ROM_LOAD("compumatic_727.ic3", 0x00000, 0x20000, CRC(ccf973b6) SHA1(ab67e466849b3bbd8f24be041c979c3f833a32a8))

	ROM_REGION(0x800, "eeprom", 0)
	ROM_LOAD("24lc16b.ic8", 0x000, 0x800, CRC(1cae70db) SHA1(575d4c787fd65950417e85fdb34d2961fc327c74))
ROM_END

GAME(199?, microdar, 0, microdar, microdar, microdar_state, empty_init, ROT0, "Compumatic / Bifuca", "Microdar SPD", MACHINE_IS_SKELETON_MECHANICAL)
