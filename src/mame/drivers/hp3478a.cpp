// license:BSD-3-Clause
// copyright-holders:fenugrec
/******************************************************************************
* HP 3478A Digital Multimeter
*
* Emulating test equipment is not very meaningful except for developping ROM patches.
* This aims to be the minimal emulation sufficient to run the UI (keypad and display).
* Ideally, faking ADC readings could be useful too.
*
* Some of this will probably be applicable to HP 3468A units too.
*
* Current status : runs (with banking !), fails CAL checksum due to unimplemented
*
*
* TODO kindof important
* * keypad on port1
* * something something display
*
* TODO next level
* * DIP switches
* * proper CAL RAM (see NVRAM() macro ?)
* * ability to preload CAL RAM on startup ("media" or "software" ?)
*
* TODO level 9000
* * Connect this with the existing i8291.cpp driver
* * dump + add analog CPU (8049)


**** Hardware details (refer to service manual for schematics)
Main CPU : i8039 , no internal ROM
Analog (floating) CPU : i8049, internal ROM (never dumped ?)
ROM : 2764 (64kbit, org 8kB) . Stores calibration data
RAM : 5101 , 256 * 4bit
GPIB:  i8291


Main cpu I/O ports:
Port1
P14-P17 : keypad out (cols)
P10-P13 : keypad in (rows)

P20 : disp.clk1
P21 : !CS for GPIB, and disp.IWA
P22 : !CS for DIPswitch; disp.ISA (for instructions)
P23 = !OE for RAM ; disp.sync (enable instruction)
P24 = disp.PWO	(enable)
P25 = disp.clk2
P26 : address bit12 ! (0x1000) => hardware banking
P27 : data out thru isol, to analog CPU

T1 : data in thru isol, from analog CPU (opcodes jt1 / jnt1)
*/

#include "emu.h"
#include "includes/hp3478a.h"

#include "cpu/mcs48/mcs48.h"



#define CPU_CLOCK       XTAL(5'856'000)

#define A12_PIN	P26
#define CALRAM_CS P23
#define DIPSWITCH_CS P22
#define GPIB_CS P21


/**** optional debug outputs, must be before #include */
#define DEBUG_PORTS (LOG_GENERAL << 1)
#define DEBUG_BANKING (LOG_GENERAL << 2)
#define DEBUG_BUS (LOG_GENERAL << 3)

#define VERBOSE (DEBUG_BUS)

#include "logmacro.h"



/***** callbacks */

WRITE8_MEMBER( hp3478a_state::p1write )
{
	LOGMASKED(DEBUG_PORTS, "port1 write: %02X\n", data);
}

READ8_MEMBER( hp3478a_state::p1read )
{
	uint8_t data = 0;
	LOGMASKED(DEBUG_PORTS, "port1 read: 0x%02X\n", data);
	return data;
}

WRITE8_MEMBER( hp3478a_state::p2write )
{
	LOGMASKED(DEBUG_PORTS, "port2 write: %02X\n", data);

	/* inefficient ? calls set_entry on every P2 write. Should maybe do "if oldstate != newstate" ? */
	if (data & A12_PIN) {
		m_bank0->set_entry(1);
		LOGMASKED(DEBUG_BANKING, "changed to bank1\n");
	} else {
		m_bank0->set_entry(0);
		LOGMASKED(DEBUG_BANKING, "changed to bank0\n");
	}

}

/** external bus read callback
 * runs when main cpu accesses an external IC, e.g. GPIB or CAL RAM.
 */
READ8_MEMBER( hp3478a_state::busread )
{
	uint8_t p2_state;
	uint8_t data = 0;
	unsigned found = 0;

	p2_state = m_maincpu->p2_r();
	// check which CS line is active.

	if (!(p2_state & CALRAM_CS)) {
		//XXX read from calram
		found += 1;
		LOGMASKED(DEBUG_BUS, "read 0x%02X from CAL RAM[0x%02X]\n", data, offset);
	}
	if (!(p2_state & DIPSWITCH_CS)) {
		//XXX parse inputs
		found += 1;
		LOGMASKED(DEBUG_BUS, "read DIP state : 0x%02X\n", data);
	}
	if (!(p2_state & GPIB_CS)) {
		found += 1 ;
		LOGMASKED(DEBUG_BUS, "read GPIB register %X\n", offset & 0x07);
	}

	if (!found) {
		logerror("Bus read with no CS active !\n");
		return 0xFF;	//pulled up
	}

	if (found > 1) {
		logerror("Bus read with more than one CS active !\n");
	}
	return data;
}



WRITE8_MEMBER( hp3478a_state::buswrite )
{
	uint8_t p2_state;
	unsigned found = 0;

	p2_state = m_maincpu->p2_r();
	// check which CS line is active.

	if (!(p2_state & CALRAM_CS)) {
		//XXX write from calram
		found += 1;
		LOGMASKED(DEBUG_BUS, "write 0x%02X to CAL RAM[0x%02X]\n", data, offset);
	}
	if (!(p2_state & DIPSWITCH_CS)) {
		logerror("Illegal write to DIP switch !\n");
		found += 1;
	}
	if (!(p2_state & GPIB_CS)) {
		found += 1 ;
		LOGMASKED(DEBUG_BUS, "GPIB register %X write 0x%02X\n", offset & 0x07, data);
	}


	if (!found) {
		logerror("Bus write with no CS active !\n");
	}

	if (found > 1) {
		logerror("Bus write with more than one CS active !\n");
	}
}








void hp3478a_state::machine_start() {
	m_bank0->configure_entries(0, 2, memregion("maincpu")->base(), 0x1000);
}

/******************************************************************************
 Address Maps
******************************************************************************/

void hp3478a_state::i8039_map(address_map &map)
{
	//map(0x0000, 0x0fff).rom(); /* CPU address space : 4kB */
	map(0x0000, 0x0fff).bankr("bank0");	// CPU address space (4kB), banked according to P26 pin
}

void hp3478a_state::i8039_io(address_map &map)
{
	map.global_mask(0xff);
	map(0,0xff).rw(FUNC(hp3478a_state::busread), FUNC(hp3478a_state::buswrite));	//"external" access callbacks
}

/******************************************************************************
 Input Ports
******************************************************************************/
static INPUT_PORTS_START( hp3478a )
INPUT_PORTS_END

/******************************************************************************
 Machine Drivers
******************************************************************************/

void hp3478a_state::hp3478a(machine_config &config)
{
	auto &mcu(I8039(config, "maincpu", CPU_CLOCK));
	mcu.set_addrmap(AS_PROGRAM, &hp3478a_state::i8039_map);
	mcu.set_addrmap(AS_IO, &hp3478a_state::i8039_io);
	mcu.p1_in_cb().set(FUNC(hp3478a_state::p1read));
	mcu.p1_out_cb().set(FUNC(hp3478a_state::p1write));
	mcu.p2_out_cb().set(FUNC(hp3478a_state::p2write));
}

/******************************************************************************
 ROM Definitions
******************************************************************************/
ROM_START( hp3478a )
	ROM_REGION( 0x2000, "maincpu", 0 )
	ROM_LOAD_OPTIONAL("rom_dc118.bin", 0, 0x2000, CRC(10097ced) SHA1(bd665cf7e07e63f825b2353c8322ed8a4376b3bd))	//main CPU ROM, can match other datecodes too
ROM_END

/******************************************************************************
 Drivers
******************************************************************************/

//    YEAR  NAME  PARENT  COMPAT  MACHINE  INPUT  CLASS      INIT        COMPANY                        FULLNAME             FLAGS
COMP( 1983, hp3478a,  0,      0,  hp3478a, hp3478a,hp3478a_state, empty_init, "HP", "HP 3478A Multimeter", MACHINE_IS_INCOMPLETE | MACHINE_NO_SOUND_HW | MACHINE_TYPE_OTHER)
