// license:BSD-3-Clause
// copyright-holders:

/*
Cirsa-Unidesa 910510-3 PCB, used by games like Mini Money, Mini Cherry,
Mini Nevada, Flamingo, etc.
Most of them use also the Cirsa-Unidesa 930902-3 for extra sound.

   _________________________________________________________
  |      __________                          ........     _|_
 _|_    |SN74HC14N|                                      |   |
|   |    __________        910510-3                      | P3|
| P1|   |__EMPTY__|                                      |   |
|   |    __________                          ____        |   |
|   |   |_SN7407N_|   ___________________  24C16CB1      |   |
|   |                | Intel P8155H-2   |    ____   Xtal |   |
|   |    __________  |__________________|  PCF8583P KDS3L|   |
|   |   |MCT1413P_|   ___________________    ....        |___|
|___|    __________  | Intel P8256AH    |                  |
  |     |_SN7406N_|  |__________________|                · |
  | ·    __________   ___________________               P· |
  | ·P  |SN74HC14N|  | Cirsa 37301      |    __________ 9· |
  | ·7   __________  |__________________|   |__EMPTY__|    |
  | ·   |SN74HC14N|   ___________________    __________  · |
  |                  | Winbond WF19054  |   |74HC393AP|  · |
  | ·                |__________________|    __________ P· |
  | ·P   __________   _________ _________   |MC14060BCP 2· |
  | ·1  |__EMPTY__|  |_8xDIPS_||_8xDIPS_|                · |
  | ·1   __________  _________ _________     __________    |
  | ·   |__EMPTY__|  SN74HC32N SN74LS08N    |SN74HC00N|    |
  |      ___________________          _________________    |
  |     | Intel P8088-2    |         | EPROM U12      |    |
  |     |__________________|         |________________|    |
  |      __________    __________     _________________    |
  |     |SN74LS373N   |SN74LS373N    | EPROM U12 EMPTY|    |
  |      __________    __________    |________________|    |
  |     |_PAT_053_|   |_PAT_054_|      ________________    |
  |      __________    __________     | UM62256B-10L  |    |
  |     |SN74LS373N   |__EMPTY__|     |_______________|    |
  |      ___________________        __________     ____    |
  |     | SAB 8155-2-P     |       |_D71084C_|   LM393N    |
  |     |__________________|       XTAL                    |
  |      __________    __________  18.432 MHz              |
  | ·   |TD62064AP|   |SN74HC32N|                          |
  | ·    __________    __________          BATT            |
  | ·P  |TD62064AP|   |SN74HC145N          3.6V            |
  | ·5   __________    __________                          |
  | ·   |TD62064AP|   |_KA2580A_|                          |
  | ·     __________    _________      _________________   |
  | ·    |_MDP1603_|   |MCT1413P|     | Intel P8256AH  |   |
  | ·    __________                   |________________|   |
  |     |__EMPTY__|                                        |
  |________________________________________________________|


             ____________________________________
            | _________  ________  ::::::::::    |
            ||________| |_______|                |
            | ___________________   930902-3     |
            || Intel/Oki 8155   |             __ |
    ________||__________________|            | | |
   | _________  _________________            |_| |
   ||________| | EPROM          |   _______      |
   |           |________________|  | OKI  |   __ |
   |            _________________  |M6376 |  | | |
   | _________ | EPROM          |  |______|  |_| |
   ||________| |________________|                |
   |_____________________________________________|
*/

#include "emu.h"
#include "cpu/i86/i86.h"
#include "machine/i2cmem.h"
#include "machine/i8155.h"
//#include "machine/i8256.h"
#include "machine/pcf8583.h"
#include "sound/ay8910.h"
#include "sound/okim6376.h"

#include "emupal.h"
#include "screen.h"
#include "speaker.h"


namespace {

class cirsa910510_state : public driver_device
{
public:
	cirsa910510_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
			m_maincpu(*this, "maincpu")
	{ }

	void cirsa910510(machine_config &config);

private:
	void io_map(address_map &map) ATTR_COLD;
	void main_map(address_map &map) ATTR_COLD;

	required_device<cpu_device> m_maincpu;
};


void cirsa910510_state::main_map(address_map &map)
{
	map(0x00000, 0x0ffff).ram();
	map(0x80000, 0xfffff).rom().region("maincpu", 0);
}

void cirsa910510_state::io_map(address_map &map)
{
}


static INPUT_PORTS_START( cirsa910510 )
INPUT_PORTS_END


void cirsa910510_state::cirsa910510(machine_config &config)
{
	// Basic machine hardware
	I8088(config, m_maincpu, 18.432_MHz_XTAL / 2); // divider not verified
	m_maincpu->set_addrmap(AS_PROGRAM, &cirsa910510_state::main_map);
	m_maincpu->set_addrmap(AS_IO, &cirsa910510_state::io_map);

	I8155(config, "main_8155_0", 0);

	I8155(config, "main_8155_1", 0);

	I8155(config, "sound_8155", 0); // on sound PCB

	//I8256(config, "muart1", 18.432_MHz_XTAL / 3);

	//I8256(config, "muart2", 18.432_MHz_XTAL / 3);

	PCF8583(config, "rtc", 32.768_kHz_XTAL);

	I2C_24C16(config, "eeprom");

	// Sound hardware
	SPEAKER(config, "mono").front_center();

	AY8910(config, "ay", 18.432_MHz_XTAL / 12).add_route(ALL_OUTPUTS, "mono", 0.5); // actually WF19054, divider not verified

	OKIM6376(config, "oki", 18.432_MHz_XTAL / 128).add_route(ALL_OUTPUTS, "mono", 0.5); // on sound PCB, divider not verified
}

/***************************************************************************

  Game driver(s)

***************************************************************************/

ROM_START( minimony )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD( "minimoney.u12", 0x00000, 0x80000, CRC(b09dad28) SHA1(dbcf855ea994a70dd3f82ef318d49b899444ac96) )

	ROM_REGION( 0x100000, "oki", 0 ) // missing, size guessed
	ROM_LOAD( "samples1.bin", 0x00000, 0x80000, NO_DUMP )
	ROM_LOAD( "samples2.bin", 0x80000, 0x80000, NO_DUMP )

	ROM_REGION( 0x800, "eeprom", 0 )
	ROM_LOAD( "24c16.u16", 0x000, 0x800, CRC(a6a6d866) SHA1(3d94da425372d94072e24b910b33de374c9d0f70) )

	ROM_REGION( 0x400, "plds", 0 ) // protected
	ROM_LOAD( "pat-053_f16l8-25.u2", 0x000, 0x104, NO_DUMP )
	ROM_LOAD( "pat-054_f16r4-25.u1", 0x200, 0x104, NO_DUMP )
ROM_END

ROM_START( minimonya )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD( "1.5.bin", 0x00000, 0x80000, CRC(ce29f243) SHA1(150b73993d3f840e75cc4567f30a9669b94ff2d5) )

	ROM_REGION( 0x100000, "oki", 0 ) // missing, size guessed
	ROM_LOAD( "samples1.bin", 0x00000, 0x80000, NO_DUMP )
	ROM_LOAD( "samples2.bin", 0x80000, 0x80000, NO_DUMP )

	ROM_REGION( 0x800, "eeprom", 0 )
	ROM_LOAD( "cat24c16_unidesa.bin", 0x000, 0x800, CRC(25c23555) SHA1(efd9b19633428bab8907fedd2f6211fba1f1a7bd) )

	ROM_REGION( 0x400, "plds", 0 ) // protected
	ROM_LOAD( "pat-053_f16l8-25.u2", 0x000, 0x104, NO_DUMP )
	ROM_LOAD( "pat-054_f16r4-25.u1", 0x200, 0x104, NO_DUMP )
ROM_END

ROM_START( minimonyb )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD( "1.4.bin", 0x00000, 0x80000, CRC(f19470ed) SHA1(68a57641aeeb056ebab8550abb248ebf0b0acb7f) )

	ROM_REGION( 0x100000, "oki", 0 ) // missing, size guessed
	ROM_LOAD( "samples1.bin", 0x00000, 0x80000, NO_DUMP )
	ROM_LOAD( "samples2.bin", 0x80000, 0x80000, NO_DUMP )

	ROM_REGION( 0x800, "eeprom", 0 )
	ROM_LOAD( "cat24c16_unidesa.bin", 0x000, 0x800, CRC(25c23555) SHA1(efd9b19633428bab8907fedd2f6211fba1f1a7bd) )

	ROM_REGION( 0x400, "plds", 0 ) // protected
	ROM_LOAD( "pat-053_f16l8-25.u2", 0x000, 0x104, NO_DUMP )
	ROM_LOAD( "pat-054_f16r4-25.u1", 0x200, 0x104, NO_DUMP )
ROM_END

} // Anonymous namespace

// This has mechanical reels and small LED displays, some other titles have an auxiliary video PCB
GAME( 199?, minimony,  0,        cirsa910510, cirsa910510, cirsa910510_state, empty_init, ROT0, "Cirsa", "Mini Money",                MACHINE_IS_SKELETON_MECHANICAL )
GAME( 199?, minimonya, minimony, cirsa910510, cirsa910510, cirsa910510_state, empty_init, ROT0, "Cirsa", "Mini Money (set 2, v1.5?)", MACHINE_IS_SKELETON_MECHANICAL )
GAME( 199?, minimonyb, minimony, cirsa910510, cirsa910510, cirsa910510_state, empty_init, ROT0, "Cirsa", "Mini Money (set 3, v1.4?)", MACHINE_IS_SKELETON_MECHANICAL )
