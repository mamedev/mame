// license:GPL-2.0+
// copyright-holders:Felipe Sanches
/*
    RAMBo (RepRap Arduino-compatible Mother Board) by UltiMachine
    for controlling desktop 3d printers
    http://reprap.org/wiki/Rambo

    driver by Felipe Correa da Silva Sanches <fsanches@metamaquina.com.br>

    This driver is based on the schematics of the version 1.1b:
    http://reprap.org/mediawiki/images/7/75/Rambo1-1-schematic.png

    3d printers currently supported by this driver:
    * Metam??quina 2

    3d printers known to use this board:
    * TODO: list them all here
*/

#include "emu.h"
#include "cpu/avr8/avr8.h"


namespace {

#define MASTER_CLOCK    16000000

/****************************************************\
* I/O devices                                        *
\****************************************************/

class rambo_state : public driver_device
{
public:
	rambo_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu")
	{
	}

	void rambo(machine_config &config);

private:
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;

	void rambo_prg_map(address_map &map) ATTR_COLD;
	void rambo_data_map(address_map &map) ATTR_COLD;

	uint8_t m_port_a = 0;
	required_device<atmega2560_device> m_maincpu;
};

/****************************************************\
* Address maps                                       *
\****************************************************/

void rambo_state::rambo_prg_map(address_map &map)
{
	map(0x0000, 0x1FFFF).rom();
}

void rambo_state::rambo_data_map(address_map &map)
{
	map(0x0200, 0x21FF).ram();  /* ATMEGA2560 Internal SRAM */
}

/****************************************************\
* Machine definition                                 *
\****************************************************/

void rambo_state::machine_start()
{
	save_item(NAME(m_port_a));
}

void rambo_state::machine_reset()
{
	m_port_a = 0;
}

void rambo_state::rambo(machine_config &config)
{
	ATMEGA2560(config, m_maincpu, MASTER_CLOCK);
	m_maincpu->set_addrmap(AS_PROGRAM, &rambo_state::rambo_prg_map);
	m_maincpu->set_addrmap(AS_DATA, &rambo_state::rambo_data_map);
	m_maincpu->set_eeprom_tag("eeprom");
	m_maincpu->set_low_fuses(0xff);
	m_maincpu->set_high_fuses(0xda);
	m_maincpu->set_extended_fuses(0xf4);
	m_maincpu->set_lock_bits(0x0f);
	m_maincpu->gpio_in<atmega2560_device::GPIOA>().set([this]() { return m_port_a; });
	m_maincpu->gpio_out<atmega2560_device::GPIOA>().set([this](uint8_t data) { m_port_a = data; });

	/*TODO: Add an ATMEGA32U2 for USB-Serial communications */
	/*TODO: Emulate the AD5206 digipot */
	/*TODO: Emulate the A4982 stepper motor drivers and instantiate 5 of these here
	        for controlling the X, Y, Z, E1 (and optionally E2) motors */
	/*TODO: Simulate the heating elements */
	/*TODO: Implement the thermistor measurements */
}

ROM_START( metamaq2 )
	ROM_REGION( 0x20000, "maincpu", 0 )
	ROM_DEFAULT_BIOS("20131015")

	ROM_SYSTEM_BIOS( 0, "20130619", "June 19th, 2013" )
	/* SOURCE(https://github.com/Metamaquina/Repetier-Firmware/tree/MM2_2013_06_19) */
	ROMX_LOAD("repetier-fw-metamaquina2-2013-06-19.bin", 0x0000, 0x1000e, CRC(4279b178) SHA1(e4d3c9d6421287c980639c2df32d07b754adc8fc), ROM_BIOS(0))

	ROM_SYSTEM_BIOS( 1, "20130624", "June 24th, 2013" )
	/* SOURCE(https://github.com/Metamaquina/Repetier-Firmware/tree/MM2_RC2_RAMBo_rev10e_2013_06_24) */
	ROMX_LOAD("repetier-fw-metamaquina2-2013-06-24_mm2rc2_rambo_rev10e.bin", 0x0000, 0xcebc, CRC(82400a3c) SHA1(0781ce29406ce69b63edb93d776b9c081bed841e), ROM_BIOS(1))

	ROM_SYSTEM_BIOS( 2, "20130625", "June 25th, 2013" )
	/* SOURCE(https://github.com/Metamaquina/Repetier-Firmware/tree/MM2_2013_06_25) */
	ROMX_LOAD("repetier-fw-metamaquina2-2013-06-25.bin", 0x0000, 0x10076, CRC(e7e4db38) SHA1(0c307bb0a0ee4e9d38253936e7030d0efb3c1845), ROM_BIOS(2))

	ROM_SYSTEM_BIOS( 3, "20130709", "July 9th, 2013" )
	/* SOURCE(https://github.com/Metamaquina/Repetier-Firmware/tree/MM2_2013_07_09) */
	ROMX_LOAD("repetier-fw-metamaquina2-2013-07-09.bin", 0x0000, 0x10078, CRC(9a45509f) SHA1(3a2e6516b45cc0ea1aef039335b02208847aaebf), ROM_BIOS(3))

	ROM_SYSTEM_BIOS( 4, "20130712", "July 12th, 2013" )
	/* SOURCE(https://github.com/Metamaquina/Repetier-Firmware/tree/MM2_2013_07_12) */
	ROMX_LOAD("repetier-fw-metamaquina2-2013-07-12.bin", 0x0000, 0x10184, CRC(9aeac87c) SHA1(c1441096553c214c12a34da87fa42cc3f0eaf74d), ROM_BIOS(4))

	ROM_SYSTEM_BIOS( 5, "20130717", "July 17th, 2013" )
	/* SOURCE(https://github.com/Metamaquina/Repetier-Firmware/tree/MM2_2013_07_17) */
	ROMX_LOAD("repetier-fw-metamaquina2-2013-07-17.bin", 0x0000, 0x10180, CRC(7c053ed0) SHA1(7abeabcbfdb411b6e681e2d0c9398c40b142f76b), ROM_BIOS(5))

	ROM_SYSTEM_BIOS( 6, "20130806", "August 6th, 2013" )
	/* SOURCE(https://github.com/Metamaquina/Repetier-Firmware/tree/MM2_2013_08_06) */
	ROMX_LOAD("repetier-fw-metamaquina2-2013-08-06.bin", 0x0000, 0x1017e, CRC(6aaf5a14) SHA1(93cebee8ab9eda9d81e70504b407268a198577f0), ROM_BIOS(6))

	ROM_SYSTEM_BIOS( 7, "20130809", "August 9th, 2013" )
	/* SOURCE(https://github.com/Metamaquina/Repetier-Firmware/tree/MM2_2013_08_09) */
	ROMX_LOAD("repetier-fw-metamaquina2-2013-08-09.bin", 0x0000, 0x1018a, CRC(ee53a011) SHA1(666d09fe69220a172528fe8d1c358e3ddaaa743a), ROM_BIOS(7))

	ROM_SYSTEM_BIOS( 8, "20130822", "August 22nd, 2013" )
	/* SOURCE(https://github.com/Metamaquina/Repetier-Firmware/tree/MM2_2013_08_22) */
	ROMX_LOAD("repetier-fw-metamaquina2-2013-08-22.bin", 0x0000, 0x1018a, CRC(70a5a3c9) SHA1(20e52ea7bf40e71020b815b9fb6385d880677927), ROM_BIOS(8))

	ROM_SYSTEM_BIOS( 9, "20130913", "September 13th, 2013" )
	/* source code for this one is unavailable as it was an unreleased internal development build */
	ROMX_LOAD("repetier-fw-metamaquina2-2013-09-13-devel.bin", 0x0000, 0x101bc, CRC(5e7c7933) SHA1(5b9bfe919daf705ad7a9a2de3cf4c51e3338ec47), ROM_BIOS(9))

	ROM_SYSTEM_BIOS( 10, "20130920", "September 20th, 2013" )
	/* SOURCE(https://github.com/Metamaquina/Repetier-Firmware/tree/MM2_2013_09_20) */
	ROMX_LOAD("repetier-fw-metamaquina2-2013-09-20.bin", 0x0000, 0x10384, CRC(48378e58) SHA1(513f0a0c65219875cc467420cc091e3489b58919), ROM_BIOS(10))

	ROM_SYSTEM_BIOS( 11, "20131015", "October 15th, 2013" )
	/* SOURCE(https://github.com/Metamaquina/Repetier-Firmware/tree/MM2_2013_10_15) */
	ROMX_LOAD("repetier-fw-metamaquina2-2013-10-15.bin", 0x0000, 0x102c8, CRC(520134bd) SHA1(dfe2251aad06972f237eb4920ce14ccb32da5af0), ROM_BIOS(11))

	/*Arduino MEGA bootloader */
	/* This is marked as a BAD_DUMP because we're not sure this is the bootloader we're actually using.
	   This is inherited from the Replicator 1 driver.
	   A proper dump would be good.
	   Also, it is not clear whether there's any difference in the bootloader
	   between the ATMEGA1280 and the ATMEGA2560 MCUs */
	ROM_LOAD( "atmegaboot_168_atmega1280.bin", 0x1f000, 0x0f16, BAD_DUMP CRC(c041f8db) SHA1(d995ebf360a264cccacec65f6dc0c2257a3a9224) )

	/* on-die 4kbyte eeprom */
	ROM_REGION( 0x1000, "eeprom", ROMREGION_ERASEFF )
ROM_END

} // anonymous namespace


//   YEAR  NAME      PARENT  COMPAT  MACHINE  INPUT  CLASS        INIT        COMPANY        FULLNAME                            FLAGS
COMP(2012, metamaq2, 0,      0,      rambo,   0,     rambo_state, empty_init, "Metamaquina", "Metamaquina 2 desktop 3d printer", MACHINE_NOT_WORKING | MACHINE_NO_SOUND)
