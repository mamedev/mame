// license:BSD-3-Clause
// copyright-holders:Robbbert
/***************************************************************************

Skeleton driver for numerous random terminals

2018-08-08

http://oldcomputer.info/terminal/


****************************************************************************/

#include "emu.h"

#include "cpu/m6800/m6800.h"
#include "cpu/mcs48/mcs48.h"
#include "cpu/mcs51/mcs51.h"

#include "machine/6821pia.h"


namespace {

class terminal_state : public driver_device
{
public:
	terminal_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
	{ }

	void terminal(machine_config &config);
	void blaucds32(machine_config &config);

private:
	void mem_map(address_map &map) ATTR_COLD;
	required_device<cpu_device> m_maincpu;
};


void terminal_state::mem_map(address_map &map)
{
	map(0x0000, 0xffff).rom();
}


// Input ports
static INPUT_PORTS_START( terminal )
INPUT_PORTS_END

// Input ports for Blaupunkt CDS 32-ID
static INPUT_PORTS_START( blaucds32 )
	// Printer interface config
	PORT_START("DSW1")
	PORT_DIPUNKNOWN_DIPLOC(0x01, 0x01, "SW1:1")
	PORT_DIPUNKNOWN_DIPLOC(0x02, 0x02, "SW1:2")
	PORT_DIPUNKNOWN_DIPLOC(0x04, 0x04, "SW1:3")
	PORT_DIPUNKNOWN_DIPLOC(0x08, 0x08, "SW1:4")
INPUT_PORTS_END


void terminal_state::terminal(machine_config &config)
{
	I8031(config, m_maincpu, 12'000'000);
	m_maincpu->set_addrmap(AS_PROGRAM, &terminal_state::mem_map);
}

void terminal_state::blaucds32(machine_config &config)
{
	terminal(config);
	m_maincpu->set_clock(11.0592_MHz_XTAL);

	// Printer interface
	M6802(config, "printercpu", 400_kHz_XTAL); // Motorola MC6802P
	PIA6821(config, "pia"); // Motorola MC6821P

	// Cherry 601-1415 keyboard (TODO: Convert to device)
	I8039(config, "kbdc", 9.216_MHz_XTAL); // Mitsubishi M5L8039P-11
}


// ROM definition

// for French Minitel network
ROM_START( alcat258 ) // MSM80C154 (+ TS9347// 8k RAM // B&W
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "polish.bin",   0x0000, 0x8000, CRC(ce90f550) SHA1(fca5311704ca9e4d57414cfed96bb2a8ff73a145) )

	ROM_REGION( 0x0100, "user1", 0 )
	ROM_LOAD( "serial.bin",   0x0000, 0x0100, CRC(f0b99b8f) SHA1(906c285fd327eba2ba9798695acc456535b84570) )
ROM_END


ROM_START( loewed ) // order unknown // i8031, i8051(xtal 11.000 next to it), ITT LOTTI // 64k RAM + battery-backed NVRAM // B&W
	ROM_REGION( 0x28000, "maincpu", 0 )
	ROM_LOAD( "mainboard_18764_100.bin", 0x00000, 0x020000, CRC(f9ec7591) SHA1(1df7bdf33b8086166f1addb686a911a0c52dde32) )
	ROM_LOAD( "module_19315_056.bin",    0x20000, 0x008000, CRC(b333c5ed) SHA1(93cfa95e595bea83fe1b34a1426b80ceb1755c50) )
ROM_END


ROM_START( loewe715 ) // i8051, ITT LOTTI // 64k RAM // colour
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "eprom.bin",    0x00000, 0x10000, CRC(2668b944) SHA1(b7773c4d7a1e0dde2a2b414ae76e5faa1fa5e324) )
ROM_END


ROM_START( 7951om ) // TTL (no CPU) // 1k x 6bits display RAM 64-characters uppercase only, screen 40x12 // green
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "prom1_rear.bin", 0x0000, 0x0100, CRC(ab231a4c) SHA1(1412d0e9163125f28a777717c4dd9d5fd54b5196) )
	ROM_LOAD( "prom2.bin",      0x0100, 0x0100, CRC(5d65b9b6) SHA1(2ea22beb6edbedb1d215b4c55233af897cdeb535) )

	ROM_REGION( 0x0800, "chargen", ROMREGION_INVERT )
	ROM_LOAD( "7951om.bin",     0x0000, 0x0800, CRC(36fc61c6) SHA1(6b5e8701b185b32a1a2630ddfc5402345628ecba) )
ROM_END


ROM_START( teleguide ) // order unknown // i8051, i8031 (layout very similar to loewed) // 64k RAM + battery-backed NVRAM // B&W
	ROM_REGION( 0x38000, "maincpu", 0 )
	ROM_LOAD( "cardreader_17044-068_349-1163.bin", 0x00000, 0x10000, CRC(3c980c0d) SHA1(9904ffd283a11defbe3daf2cb9029bcead8b02d0) )
	ROM_LOAD( "mainboard_18764-063_349-1173.bin",  0x10000, 0x20000, CRC(eb5c2d05) SHA1(dba2f72f928487e83741ad24d70b568e4510988e) )
	ROM_LOAD( "module_19315-051_349-01173.bin",    0x20000, 0x08000, CRC(29c4b49d) SHA1(9bf37616eb130cb6bf86954b4a4952ea99d43ce8) )
ROM_END

/*
   Blaupunkt CDS 32-ID terminal with colour screen.
   More hardware infor and photos:  https://www.oldcomputers.es/terminal-blaupunkt-cds-32-id/

   Hardware setup:

 Printer PCB (silkscreened as "BLAUPUNKT  BTX-DRUCKER-INTERFACE  TYP 8668 305 585")
    ____________________________________________________________________________
   |                    ___________   ___________   ___________   ___________  |
 __|__                 |CONN_MAIN_|  |SN74ALS174N  |SN74LS164N|  |SN74ALS161BN |
|     | ___________   ___________    ___________    ___________                |
|DB15 ||SN74LS367AN  |SN74LS02N_|   |SN74LS244N|   |SN74LS374N|                |
| FEM |               __________________________    ________________           |
|     | ___________  | Motorola MC6821P        |   | NEC D4016C-3  |           |
|     ||SN74LS367AN  |                         |   |               |           |
|_____|              |_________________________|   |_______________|           |
   |        Xtal      __________________________    ___________   ___________  |
   |_     400 kHz    | Motorola MC6802P        |   |SN74LS161AN  |SN74AS04N_|  |
   - |               |                         |    ___________   ___________  |
   -_|<- Dips x 4    |_________________________|   |SN74LS161AN  |SN74AS02N_|  |
   |    ___________   ________________              ___________   ___________  |
   |   |_MC14069U_|  | EPROM         |             |SN74LS161AN  |SN74LS00N_|  |
   |                 |_______________|                                         |
   |___________________________________________________________________________|

Main PCB
   _____________________________________________________________________________
  |  _______________   _______________                            ___________  |
  | | NEC D4016C-3 |  | NEC D4016C-3 |                           |PC74HCT367P  |
  | |______________|  |______________|                             __________  |
  |  ___________       ___________                                CONN PRINTER |
  | |CD74HCT365E      |CD74HCT373E         Xtal 6 MHz                          |
  |  ___________       ___________   ____________________                      |
  | |CD74HCT365E      |CD74HCT373E  | M4613D/A          |                      |
  |  ____________      ___________  | 4782 DUG8629      |                      |
  | |CD74HCT373E|     |CD74HCT365E  |___________________|       _____________  |
  |  ___________       ___________   ___   ______________      | ASTEC       | |
  | |CD74HCT86E|      |_N82S153N_|  |  |  | NMC9816AN-25|      | AD1D12A10   | |
  |  _____________________   Xtal   |  |  |_____________|      |_____________| |
  | | MAB 8031AH 12P     | 11.0592  |_<-CD74HCT245E______                      |
  | |                    |   MHz          | D4016C-3    |                      |
  | |____________________|                |_____________|                      |
  |                                        ______________                      |
  |                                       | EPROM       |                      |
  |                                       |_____________|                      |
  |                                                                            |
  |____                                                                        |
      |                   -----------------------------------------            |
      |                   -------------- V24 INTERFACE ------------            |
      |_______________________      ___      ___      ___      ________________|
                              |____|   |____|   |____|   |____|   
                             Keyboard   Tape      CVS     Modem

V24 Interface riser PCB
   ____________________________________________________________
  |  ___________   ___________    ___________    ___________  |
  | |_SN75189N_|  |_SN75188N_|   |PC74HCT00P|   |PC74HCT157P  |
  |_____________________                                      |
                        |_|_|_|_|_|_|_|_|_|_|_|_|_|_|_|_|_|_|_|

Keyboard PCB
  ___________________________________________________________________________________________
 |  _____________   ___________   ___________________   ____________                        |
 | |CD74HCT4514E|  |_74LS244N_|  | M5L8039P-11      |  | EPROM     |   ___________          |
 | |____________|    Xtal        |__________________|  |___________|  |PC74HCT373P          |
 |________________ 9.216 MHz _______________________________________________________________|

*/
ROM_START( blaucds32 )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "v4292_b0147-00.v4292", 0x00000, 0x10000, CRC(a25d8966) SHA1(72298dd4f3d8bb333c545fec2683bcae4bb18e26) )

	ROM_REGION( 0x2000, "printercpu", 0 )
	ROM_LOAD( "v5270_b0141-01.v270",  0x00000, 0x02000, CRC(95943b74) SHA1(4554b796b75c6790d07dc4bf5278df00f00b6804) )

	ROM_REGION( 0x800, "keyboard", 0 )
	ROM_LOAD( "426.bin",              0x00000, 0x00800, CRC(0465f6a7) SHA1(14d9d9ae58baad2f7ccbf1f35ef8599e32ec1ed1) )

	ROM_REGION( 0x0eb, "prom", 0 )
	ROM_LOAD( "n82s153n.v4245",       0x00000, 0x000eb, NO_DUMP ) // On main PCB
ROM_END

} // anonymous namespace


// Driver

//    YEAR  NAME       PARENT  COMPAT  MACHINE    INPUT      CLASS           INIT        COMPANY               FULLNAME          FLAGS
COMP( 1991, alcat258,  0,      0,      terminal,  terminal,  terminal_state, empty_init, "Alcatel",            "Terminatel 258", MACHINE_NO_SOUND | MACHINE_NOT_WORKING )
COMP( 1992, loewed,    0,      0,      terminal,  terminal,  terminal_state, empty_init, "Loewe",              "Multitel D",     MACHINE_NO_SOUND | MACHINE_NOT_WORKING )
COMP( 1988, loewe715,  0,      0,      terminal,  terminal,  terminal_state, empty_init, "Loewe",              "Multicom 715L",  MACHINE_NO_SOUND | MACHINE_NOT_WORKING )
COMP( 1987, 7951om,    0,      0,      terminal,  terminal,  terminal_state, empty_init, "Mera-Elzab",         "7951om",         MACHINE_NO_SOUND | MACHINE_NOT_WORKING )
COMP( 1992, teleguide, 0,      0,      terminal,  terminal,  terminal_state, empty_init, "Loewe / Televerket", "Teleguide",      MACHINE_NO_SOUND | MACHINE_NOT_WORKING )
COMP( 19??, blaucds32, 0,      0,      blaucds32, blaucds32, terminal_state, empty_init, "Blaupunkt",          "CDS 32-ID",      MACHINE_NO_SOUND | MACHINE_NOT_WORKING )
