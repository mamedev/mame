// license:BSD-3-Clause
// copyright-holders:
/********************************************************************************

 Skeleton driver for MCS51-based crane coinops from Compumatic.
 The same PCB was used on machines from different manufacturers, like OM Vending
 and Covielsa.

 Display is just a four digits 7-segments (on a small PCB called "Plumadig").

 Different hardware revisions from Compumatic, called "GANCHONEW" PCB.
 From V2 to V8 hardware with the following layout (minor changes, like the power
 supply connector, moving from PC AT to PC ATX):

 COMPUMATIC "GANCHONEW V8" CPU
                               COUNTERS
  ______________________________________
 |     ______  ______  ______  ········ |
 |     ST8251  ST8251  ST8251           |
 | __________    ____                   |
 | ULN2803APG    LM358N                 |
 |                                      |
 | __________    __________     __ __  _|_
 | SN74HC2       SN74HC244N    | || | |   |
 |                             | || | | C |
 | __________    __________    |FUSES | O |
 | SN74HC737N    SN74HC244N    | || | | N |
 |                             |_||_| | N |
 | __________    __________           |___|
 | SN74HC737N    |_GAL16V8_|            |
 |                                    oo|
 | ________________    ____     ____  oo|
 || EPROM         | TL7705ACP         oo|
 ||_______________|    ____           oo|<- ATX Power
 |                    24C16           oo|   Supply conn
 | ___________________   Xtal         oo|
 || 80C32            |   12MHz  TEST  oo|
 ||__________________|           SW   oo|
 | ....  ...... ..  ..... .......       |
 |______________________________________|
  DISPLAY SENSOR SPK SELECT JOYSTICK
          +V RET

 The MCU on the older PCBs can differ between 80C32 compatible models (found
 with a Winbond W78C32C-40 and with a TS80C32X2-MCA).

 "GANCHONEW" (V1) PCB has a different layout:
              __________
  ___________|         |__________________
 |           |_________| |||||||||||||  .|
 |:                ________   _______   :|
 |:   ____        TD62703AP  HD74HC244P  |
 |:   BUZ12        ________   _______   :|
 |:               TD62083AP  74HCT273N  :|
 |  ____________   ________   _______    |
 | | ROM       |  HD74HC373P HD74HC244P :|
 | |___________|   ________   _______    |
 |                PALCE16V8H HD74HC273P :|
 |  _______________    ____   _______   :|
 | | TSC80C31-12CA|  24LC16B TD62083AP   |
 | |______________|                     :|
 |                   Xtal 12MHz         :|
 |_______________________________________|

********************************************************************************/

#include "emu.h"
#include "cpu/mcs51/i80c51.h"
#include "cpu/mcs51/i80c52.h"
#include "speaker.h"

namespace
{

class compucranes_state : public driver_device
{
public:
	compucranes_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
	{
	}

	void ganchonew(machine_config &config);
	void ganchonew_v1(machine_config &config);
	void toyshop(machine_config &config);

protected:
	required_device<mcs51_cpu_device> m_maincpu;
};

INPUT_PORTS_START(ganchonew)
INPUT_PORTS_END

void compucranes_state::ganchonew(machine_config &config)
{
	I80C32(config, m_maincpu, 12_MHz_XTAL);

	SPEAKER(config, "mono").front_center();
}

void compucranes_state::ganchonew_v1(machine_config &config)
{
	I80C31(config, m_maincpu, 12_MHz_XTAL);

	SPEAKER(config, "mono").front_center();
}

void compucranes_state::toyshop(machine_config &config)
{
	AT89S52(config, m_maincpu, 12_MHz_XTAL);

	SPEAKER(config, "mono").front_center();
}

// "GANCHONEW" (V1) PCB. TSC80C31-12CA CPU.
ROM_START(crsauruss)
	ROM_REGION(0x80000, "maincpu", 0)
	ROM_LOAD("30.01.ic3",   0x00000, 0x20000, CRC(c735e024) SHA1(63dd3a71472bde7f9dead49a8dc889365fd024ef)) // 1xxxxxxxxxxxxxxxx = 0xFF

	ROM_REGION(0x00117, "pld", 0)
	ROM_LOAD("pal16v8.ic4", 0x00000, 0x00117, NO_DUMP) // AMD PALCE16V8H-25, may be different on V1
ROM_END

// "GANCHONEW V8" PCB with ATX PSU connector. TS80C32X2-MCA CPU.
ROM_START(mastcrane)
	ROM_REGION(0x80000, "maincpu", 0)
	ROM_LOAD("v8.ic3",      0x00000, 0x40000, CRC(733dfcbc) SHA1(d18d7945e9b8f189f2169d3d90c3cfea97d3b39c)) // 1ST AND 2ND HALF IDENTICAL

	ROM_REGION(0x00117, "pld", 0)
	ROM_LOAD("gal16v8.ic4", 0x00000, 0x00117, CRC(4d665a06) SHA1(504f0107482f636cd216579e982c6162c0b120a7)) // Verified to be the same on all known PCB revisions
ROM_END

// "GANCHONEW V7" PCB with AT PSU connector
ROM_START(mastcranea)
	ROM_REGION(0x80000, "maincpu", 0)
	ROM_LOAD("v7.ic3",      0x00000, 0x40000, CRC(299c9ad1) SHA1(b0ba2ab588151dba89307e118ba061cad2b8116b)) // 1ST AND 2ND HALF IDENTICAL (W29C020C)

	ROM_REGION(0x00117, "pld", 0)
	ROM_LOAD("atf16v8.ic4", 0x00000, 0x00117, CRC(4d665a06) SHA1(504f0107482f636cd216579e982c6162c0b120a7)) // Verified to be the same on all known PCB revisions
ROM_END

// "GANCHONEW V2" PCB with AT PSU connector. W78C32C-40 CPU.
ROM_START(mastcraneb)
	ROM_REGION(0x80000, "maincpu", 0)
	ROM_LOAD("505.ic3",     0x00000, 0x20000, CRC(3dbb83f1) SHA1(3536762937332add0ca942283cc22ff301884a4a))

	ROM_REGION(0x00117, "pld", 0)
	ROM_LOAD("atf168b.ic4", 0x00000, 0x00117, CRC(4d665a06) SHA1(504f0107482f636cd216579e982c6162c0b120a7)) // Verified to be the same on all known PCB revisions
ROM_END

// "GANCHONEW V2" PCB with AT PSU connector
ROM_START(octopussy)
	ROM_REGION(0x80000, "maincpu", 0)
	ROM_LOAD("w29c011.ic5", 0x00000, 0x20000, CRC(47da93e8) SHA1(aa821dd22c1912ec2942ca6afd989d61df4387d7))

	ROM_REGION(0x00117, "pld", 0)
	ROM_LOAD("atf16v8.ic4", 0x00000, 0x00117, CRC(4d665a06) SHA1(504f0107482f636cd216579e982c6162c0b120a7))
ROM_END

/* Direct clone of the GANCHONEW PCB by OM Vending, silkcreened as "CPU GRUA V2  O. M. VENDING".
   Atmel AT89S52 as CPU, probably the internal ROM is not used, but should be confirmed. 12 MHz xtal. 24C16 SEEPROM. */
ROM_START(toyshop)
	ROM_REGION(0x10000, "internal", 0)
	ROM_LOAD("89s52.ic1",   0x00000, 0x10000, NO_DUMP) // 8 KBytes internal ROM

	ROM_REGION(0x80000, "maincpu", 0)
	ROM_LOAD("39sf040.ic3", 0x00000, 0x80000, CRC(0d9d157d) SHA1(e70f095d3524e3a4c8d5d07857bb2692b6260cc1))

	ROM_REGION(0x00117, "pld", 0)
	ROM_LOAD("atf16v8.ic4", 0x00000, 0x00117, NO_DUMP)
ROM_END

} // anonymous namespace

//    YEAR  NAME        PARENT     MACHINE       INPUT      CLASS              INIT        ROT   COMPANY               FULLNAME                FLAGS
GAME( 199?, crsauruss,  0,         ganchonew_v1, ganchonew, compucranes_state, empty_init, ROT0, "Recreativos Presas", "Cranesaurus Single",   MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK )
GAME( 199?, mastcrane,  0,         ganchonew,    ganchonew, compucranes_state, empty_init, ROT0, "Compumatic",         "Master Crane (set 1)", MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK )
GAME( 199?, mastcranea, mastcrane, ganchonew,    ganchonew, compucranes_state, empty_init, ROT0, "Compumatic",         "Master Crane (set 2)", MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK )
GAME( 199?, mastcraneb, mastcrane, ganchonew,    ganchonew, compucranes_state, empty_init, ROT0, "Compumatic",         "Master Crane (set 3)", MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK )
GAME( 2000, octopussy,  mastcrane, ganchonew,    ganchonew, compucranes_state, empty_init, ROT0, "Covielsa",           "Octopussy",            MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK )
GAME( 2012, toyshop,    0,         toyshop,      ganchonew, compucranes_state, empty_init, ROT0, "OM Vending",         "Toy Shop",             MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK )
