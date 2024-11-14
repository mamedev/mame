// license:BSD-3-Clause
// copyright-holders:Robbbert
/***************************************************************************

Skeleton driver for numerous random terminals

2018-08-08

http://oldcomputer.info/terminal/



****************************************************************************/

#include "emu.h"
#include "cpu/mcs51/mcs51.h"


namespace {

class terminal_state : public driver_device
{
public:
	terminal_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
	{ }

	void terminal(machine_config &config);

private:

	void mem_map(address_map &map) ATTR_COLD;
	required_device<cpu_device> m_maincpu;
};


void terminal_state::mem_map(address_map &map)
{
	map(0x0000, 0xffff).rom();
}



/* Input ports */
static INPUT_PORTS_START( terminal )
INPUT_PORTS_END


void terminal_state::terminal(machine_config &config)
{
	I8031(config, m_maincpu, 12'000'000);
	m_maincpu->set_addrmap(AS_PROGRAM, &terminal_state::mem_map);
}


/* ROM definition */

// for French Minitel network
ROM_START( alcat258 ) // MSM80C154 (+ TS9347// 8k ram // b&w
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "polish.bin",   0x0000, 0x8000, CRC(ce90f550) SHA1(fca5311704ca9e4d57414cfed96bb2a8ff73a145) )

	ROM_REGION( 0x0100, "user1", 0 )
	ROM_LOAD( "serial.bin",   0x0000, 0x0100, CRC(f0b99b8f) SHA1(906c285fd327eba2ba9798695acc456535b84570) )
ROM_END


ROM_START( loewed ) // order unknown // i8031, i8051(xtal 11.000 next to it), ITT LOTTI // 64k ram + battery-backed nvram // b&w
	ROM_REGION( 0x28000, "maincpu", 0 )
	ROM_LOAD( "mainboard_18764_100.bin", 0x00000, 0x020000, CRC(f9ec7591) SHA1(1df7bdf33b8086166f1addb686a911a0c52dde32) )
	ROM_LOAD( "module_19315_056.bin",    0x20000, 0x008000, CRC(b333c5ed) SHA1(93cfa95e595bea83fe1b34a1426b80ceb1755c50) )
ROM_END


ROM_START( loewe715 ) // i8051, ITT LOTTI // 64k ram // colour
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "eprom.bin",    0x00000, 0x10000, CRC(2668b944) SHA1(b7773c4d7a1e0dde2a2b414ae76e5faa1fa5e324) )
ROM_END


ROM_START( 7951om ) // TTL (no cpu) // 1k x 6bits display ram 64-characters uppercase only, screen 40x12 // green
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "prom1_rear.bin", 0x0000, 0x0100, CRC(ab231a4c) SHA1(1412d0e9163125f28a777717c4dd9d5fd54b5196) )
	ROM_LOAD( "prom2.bin",      0x0100, 0x0100, CRC(5d65b9b6) SHA1(2ea22beb6edbedb1d215b4c55233af897cdeb535) )

	ROM_REGION( 0x0800, "chargen", ROMREGION_INVERT )
	ROM_LOAD( "7951om.bin",     0x0000, 0x0800, CRC(36fc61c6) SHA1(6b5e8701b185b32a1a2630ddfc5402345628ecba) )
ROM_END


ROM_START( teleguide ) // order unknown // i8051, i8031 (layout very similar to loewed) // 64k ram + battery-backed nvram // b&w
	ROM_REGION( 0x38000, "maincpu", 0 )
	ROM_LOAD( "cardreader_17044-068_349-1163.bin", 0x00000, 0x10000, CRC(3c980c0d) SHA1(9904ffd283a11defbe3daf2cb9029bcead8b02d0) )
	ROM_LOAD( "mainboard_18764-063_349-1173.bin",  0x10000, 0x20000, CRC(eb5c2d05) SHA1(dba2f72f928487e83741ad24d70b568e4510988e) )
	ROM_LOAD( "module_19315-051_349-01173.bin",    0x20000, 0x08000, CRC(29c4b49d) SHA1(9bf37616eb130cb6bf86954b4a4952ea99d43ce8) )
ROM_END

} // anonymous namespace


/* Driver */

/*    YEAR  NAME       PARENT  COMPAT  MACHINE   INPUT     CLASS           INIT          COMPANY             FULLNAME                     FLAGS */
COMP( 1991, alcat258,  0,      0,      terminal, terminal, terminal_state, empty_init, "Alcatel",            "Terminatel 258",         MACHINE_IS_SKELETON )
COMP( 1992, loewed,    0,      0,      terminal, terminal, terminal_state, empty_init, "Loewe",              "Multitel D",             MACHINE_IS_SKELETON )
COMP( 1988, loewe715,  0,      0,      terminal, terminal, terminal_state, empty_init, "Loewe",              "Multicom 715L",          MACHINE_IS_SKELETON )
COMP( 1987, 7951om,    0,      0,      terminal, terminal, terminal_state, empty_init, "Mera-Elzab",         "7951om",                 MACHINE_IS_SKELETON )
COMP( 1992, teleguide, 0,      0,      terminal, terminal, terminal_state, empty_init, "Loewe / Televerket", "Teleguide",              MACHINE_IS_SKELETON )
