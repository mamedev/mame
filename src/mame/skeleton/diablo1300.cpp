// license:BSD-3-Clause
// copyright-holders:Joakim Larsson Edstrom
/*

 Diablo Printer Series 1300 HyType II driver

 - Microprocessor based control logic for increased capacity and flexibility, plus provision for implementation of additional features.
 - Advanced servo design for improved efficiency and performance.
 - Rigid one piece cast aluminium frame to better maintain print quality, and reduce maintenance requirements.
 - Rugged highly stable carriage assembly for enhanced print position accuracy and reduced maintenance.
 - Plug-in interchangeable printed pircuit boards (PCB's), readily accessible for ease and simplicity of service, and implementation
   of options and interfaces.
 - Operator control of print hammer energy (Impression Control Switch) to shift the printer's internal hammer energy scale up
   for multiple carbon forms or down for smaller lighter print font styles.
 - 1/120 inch (.212 mm) horizontal spacing on command.
 - 88/82 or 96 character metal wheel
 - Optional interface access to directly address print hammer energy levels character by character.
 - Optional interface access to command ribbon advance.
 - Optional Paper Out Switch installation for either normal top or an optional bottom paper feed.
 - Optional Cover Open Switch installation.
 - Optional End Of Ribbon sensor installation for use with mu1tistrike carbon ribbon cartridges which are not the recirculating type.
 - Carriage Return takes max 300 mS
 - Tabulation can be set as right or left
 - Column spacing 60 pt/inch by operator or 120 pt/inch by controller
 - Print Line: 13.1 inch (332.74mm)
 - Paper Feed: 4 inch/sec
 - Dimensions: 591x365x400mm
 - Weight: 12Kg

Model performance
-----------------
                     1345A    1355HS    1355WP
Print Speed char/sec  45       55        40
Character Set         96       96        88/92/96

Configurations
--------------
There are many options that comes with the Diablo 1300 series and while many are mechanical the electronics are built up with cards
interconnected by a backplane. The backplane has well defined slots for each type of cards and there are also many external cables
between the cards, sensors and motors of the printer. The backplane consists of up to 8 female connectors for 56 signals card edge
connectors numbered A-H ordered in two rows, D,C,B,A on top with the fans to the left and H,G,F,E bellow. The signals are routed as
needed and the slots are NOT generic, a specific card goes in at a specific slot but can be interchanged to accomodate improved
performance or replaced for repair. Slots E and F are used for feature expansions such as serial, network cards etc.

The slots are populated as follows:

A: Logic #1 Command buffering and host signalling over a 50 pin ribbon cable. Sends commands to Logic #2 as needed
B: Logic #2 TTL CPU that interpretes commands from Logic #1 and controls all motors in the system
C: Servo
D: Carriage Power Amp
E: Optional 8080/Z80interface board, connects to Logic #1 board acting as host over the bus or the 50 pin ribbon cable
F: Optional slot with all signals of slot F
G: Transducer
H: Print Wheel Power Amp

In case the serial/IEEE488/network interface card is missing in the printer the host computer is supposed to drive which
connects to the printer over the 50 pin ribbon cable instead of the printer hosted interface card.

Logic #1 Card - printer command management
------------------------------------------
The board is marked 40505 and has an option field at the top and a J7 connector for the 50 pin ribbon cable. It produces the
system clock of 5 MHz that is used by the TTL CPU at Logic #2 Card,

 Identified IC:s
 ---------------
 1 74LS221       Dual Monostable multivibrator
 7 74LS74   7907-7908 Dual D-type pos edg trg flip-flops w clr and preset
 3 74LS367  7849 Non inverted 3 state outputs, 2 and 4 line enabled inputs
 1 7451     7849 Dual AND+OR invert gates
 1 7486     7849 Quad XOR gates
 3 74LS170  7906 4 by 4 register file
 4 8837     7736
 2 7408     7906 Quad AND gales
 2 74LS42   7906 BCD to decimal decoder
 1 7426     7906 Quad NAND gates
 1 74LS174  7836 Hex D-type flip flops
 1 7432     7901 QUAD OR gates
 2 74LSI07  7906 Dual J-K M/S flip flops w clear
 1 7404     7901 Hex Inverters
 5 75452    7840-7901
 2 7400     7849 Quad NAND gates

Logic #2 Card - printer command execution (TTL CPU)
---------------------------------------------------
The board is marked 40510 and has no connectors except the 56 signal bus edge connector

 Identified IC:s
 ---------------
 4 7400     7848-7902 Quad NAND gates
 3 74LS04   7850 Hex Inverters
 1 7408     7901 Quad AND gales
 1 7410     7840 Tripple 3-input NAND gates
 2 7453     7903 Expandable 4 wide AND+OR invert gates
 1 74LS74   7908 Dual D-type pos edg trg flip-flops w clr and preset
 2 74LS83   7901 4 bit binary full addres with fast carry
 4 74S289        4x16 bit RAM
 1 74107         Dual J-K M/S flip flops w clear
 1 74LS155  7731 1/2/3 to 4/8 lines decoder nwih totem pole ouputs
 2 74161    7904 Synchronous binary 4 bit counter
 4 74LS259  7906 8 bit addressable latches
 4 74298    7849 Quad 2 input mux with storage
 1 74367    7840 Non inverted 3 state outputs, 2 and 4 line enabled inputs
 1 74LS174       Hex D-type flip flops

RS232 Serial Interface Card
----------------------------
The serial interface card is z80 based and marked DIABLO-1300-V24

 Identified ICs:
 ---------------
 1 Z80-CPU 7904 Zilog CPU
 1 TMS2716 7906 2KB EPROM
 1 AM9551  7850 8251 USART
 2 Z80-PIO 7852 Zilog Paralell IO interface
10 74367   7845 Non inverted 3 state outputs, 2 and 4 line enabled inputs
 2 UPB7400 7845 Quad NAND gates
 3 7432N   7832 QUAD OR gates
 1 1489    7841 Quad line receivers
 1 1488    7823 Quad line tranceivers
 1 74163   7827 Synchronous 4 bit counters
 2 7493    7822 4 bit binary counters
 2 7404    7849 Hex inverters
 1 7410    7849 Tripple 3-input NAND gates
 2 2114         1024 x 4 bit SRAM
 1 9602    7423 Dual retriggable resetable one shots


 Address decoding
 ----------------
 Z80 A0 30 -> 74367 -> Z80 PIO* Port A/B     6
 Z80 A1 31 -> 74367 -> Z80 PIO* Control/Data 5
 (Z80 A5 35 -> 74367) OR (Z80 IORQ 20) -> Z80 PIO1 CE* 4
 (Z80 A4 34 -> 74367) OR (Z80 IORQ 20) -> Z80 PIO2 CE* 4
*/

#include "emu.h"
#include "cpu/diablo/diablo1300.h"


namespace {

class diablo1300_state : public driver_device
{
public:
	diablo1300_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
  { }

	void diablo1300(machine_config &config);

private:
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;
	required_device<cpu_device> m_maincpu;

	void diablo1300_map(address_map &map) ATTR_COLD;
	void diablo1300_data_map(address_map &map) ATTR_COLD;
};

void diablo1300_state::diablo1300_map(address_map &map)
{
	map(0x0000, 0x01ff).rom();
}

void diablo1300_state::diablo1300_data_map(address_map &map)
{
	map(0x00, 0x1f).ram();
}

static INPUT_PORTS_START( diablo1300 )
INPUT_PORTS_END

void diablo1300_state::machine_start()
{
}

void diablo1300_state::machine_reset()
{
}

void diablo1300_state::diablo1300(machine_config &config)
{
	/* basic machine hardware */
	DIABLO1300(config, m_maincpu, XTAL(1'689'600));
	m_maincpu->set_addrmap(AS_PROGRAM, &diablo1300_state::diablo1300_map);
	m_maincpu->set_addrmap(AS_DATA, &diablo1300_state::diablo1300_data_map);
}

ROM_START( diablo )
	ROM_REGION( 0x10000, "maincpu", ROMREGION_16BIT )
	ROM_DEFAULT_BIOS("diablo1300")

	ROM_SYSTEM_BIOS(0, "diablo1300", "Diablo Printer Series 1300 14510-xx CPU microcode") // Recreated 82S115 binaries using Jeffs documentation
	ROMX_LOAD ("diablo1300.odd",  0x0001, 0x200, CRC (5e295350) SHA1 (6ea9a22b23b8bab93ae57671541d65dba698c722), ROM_SKIP(1) | ROM_BIOS(0))
	ROMX_LOAD ("diablo1300.even", 0x0000, 0x200, CRC (85562eb1) SHA1 (9335eeeabdd37255d6ffee153a027944a4519126), ROM_SKIP(1) | ROM_BIOS(0))

	ROM_SYSTEM_BIOS(1, "diablo1355", "Diablo Printer 1355WP 14510-xx CPU microcode")  // Dumped 82S115 using Arduino
	ROMX_LOAD ("13067-36.bin", 0x0001, 0x200, CRC (b8bf070f) SHA1 (c51f9f4009a771b1d0e9c948592c988b1f4e840f), ROM_SKIP(1) | ROM_BIOS(1))
	ROMX_LOAD ("13066-35.bin", 0x0000, 0x200, CRC (73698143) SHA1 (5d2b3e956ae0d2b606d081a242fc64928e68687d), ROM_SKIP(1) | ROM_BIOS(1))

	ROM_REGION(0x1000, "trom", 0)
	/* Table ROM, holds data for specific mechnical configuration of the printer engine, such as the number of characters and hammer energy for each character */
	ROM_LOAD( "55wp92ch.bin",  0x0000, 0x200,   CRC(58ba7913) SHA1(968e87318b49ad05a66a8148e9048bfbeba2a97f) ) // Dumped 82S115 using Arduino

ROM_END

} // anonymous namespace


//   YEAR  NAME    PARENT  COMPAT  MACHINE     INPUT       CLASS             INIT        COMPANY               FULLNAME
COMP(1976, diablo, 0,      0,      diablo1300, diablo1300, diablo1300_state, empty_init, "Diablo Systems Inc", "Diablo HyType II Series 1300 CPU", MACHINE_IS_SKELETON)
