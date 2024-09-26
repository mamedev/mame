// license:BSD-3-Clause
// copyright-holders:

/*
  Skeleton driver for MCS51-based Mars Electronics vending machines.

  Hardware for models 11x:
  ________________________________________________
 |        ___   ·········  ····    ::::::::      |
 |       |  |                                    |
 |   ___   ________   ________   ________   ___  |
 |· |2061 |74HC238N  |UDN2595A  |74HC03AN  555CN |
 |·  ________   ________         ______________  |
 |· |ULN2064B  |74HC32AN        | HY6116AP-10 |  |
 |·                             |_____________|  |
 |   ________   ________        _______________  |
 |· |ULN2064B  |74HC572N       | EPROM        |  |
 |·                            |______________|  |
 |·             ________                         |
 |·            |CA3081_|         ________       ·|
 |·                             |74HC573N       ·|
 |·                       _____________________  |
 |·                      | Intel P80C31BH     | ·|
 |·                      |____________________| ·|
 |             ________                 Xtal     |
 |            |74HC572N               11.0 MHz   |
 |                    ________   ________        |
 |  _____            74HC4052N  |74HC541N        |
 | LM317T                        ________        |
 |·                             |74HC573N        |
 |·                 ________     ____            |
 |·                |LM339N_|    DS1232   SWITCH  |
 |·         ____                                 |
 |·        555CN              ____    SPEAKER   ·|
 |·                         L9130H              ·|
 |                    ________      ________    ·|
 |·                  |74HC14N|     |UDN2595A    ·|
 |                     ____          ____       ·|
 |                    X24C04P       PCF8583P    ·|
 |  _____             ________     Xtal          |
 | LM2940CT          |DS14C88N                  ·|
 |                           3.6V BATTERY       :|
 |_______________________________________________|

 Plus a display PCB with 10 digits, 16 segments per digit (character with decimal
 point and comma tail), controlled with a Rockwell 10957P-40 or compatible (Micrel, etc.).

*/

#include "emu.h"
#include "cpu/mcs51/mcs51.h"
#include "machine/i2cmem.h"
#include "machine/pcf8583.h"
#include "machine/roc10937.h"
#include "speaker.h"

namespace {

class marsvending_state : public driver_device
{
public:
	marsvending_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu")
	{ }

	void marsvending(machine_config &config);
	void zunknecta(machine_config &config);

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;

	required_device<cpu_device> m_maincpu;
};

void marsvending_state::machine_start()
{
}

void marsvending_state::machine_reset()
{
}

static INPUT_PORTS_START( marsvending )
INPUT_PORTS_END

void marsvending_state::marsvending(machine_config &config)
{
	I80C31(config, m_maincpu, 11_MHz_XTAL); // Intel P80C31BH

	PCF8583(config, "clock", 32.768_kHz_XTAL); // PCF8583P

	I2C_24C04(config, "i2cmem", 0); // X24C04P

	ROC10957(config, "display", 0); // Rockwell 10957P-40 or compatible, 10 digits, 16 segments per digit (character with decimal point and comma tail)

	SPEAKER(config, "mono").front_center();
}

ROM_START( apvm110 )
	ROM_REGION( 0x8000, "maincpu", 0 )
	ROM_LOAD( "31058i.bin",     0x0000, 0x8000, CRC(b83a7b19) SHA1(8e7dffd2c1040017151a2da5bd72a16eda542fc5) )
ROM_END

ROM_START( apvm110a )
	ROM_REGION( 0x8000, "maincpu", 0 )
	ROM_LOAD( "mcb110-005.bin", 0x0000, 0x8000, CRC(94e0e2a2) SHA1(cfa5d41d5ff9dfdd0a042be7e1a321cf4b0253c2) )
ROM_END


} // anonymous namespace


SYST( 1990, apvm110,  0,       0, marsvending, marsvending, marsvending_state, empty_init, "Mars Electronics", "Automatic Products Vending Machine model 110 (set 1)", MACHINE_IS_SKELETON )
SYST( 1990, apvm110a, apvm110, 0, marsvending, marsvending, marsvending_state, empty_init, "Mars Electronics", "Automatic Products Vending Machine model 110 (set 2)", MACHINE_IS_SKELETON )
