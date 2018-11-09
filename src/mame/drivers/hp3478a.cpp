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
* Current status : hahahaaa
*
* the TODO list is longer than the code here
*

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
T1 : data in thru isol (opcodes jt1 / jnt1)
*/

#include "emu.h"
#include "includes/hp3478a.h"

#include "cpu/mcs48/mcs48.h"

#define CPU_CLOCK       XTAL(5'856'000)

//XXX
#undef DEBUG_FIFO
#undef DEBUG_SERIAL_CB
#undef DEBUG_PORTS

WRITE8_MEMBER( hp3478a_state::port1_w )
{
#ifdef DEBUG_PORTS
	logerror("port1 write: %02X\n", data);
#endif
}

READ8_MEMBER( hp3478a_state::port1_r )
{
	uint8_t data = 0;
#ifdef DEBUG_PORTS
	logerror("port1 read: 0x%02X\n", data);
#endif
	return data;
}

void hp3478a_state::machine_start() {
}

/******************************************************************************
 Address Maps
******************************************************************************/

void hp3478a_state::i8039_map(address_map &map)
{
	map(0x0000, 0x0fff).rom(); /* 27C64 ROM */
	// AM_RANGE(0x2000, 0x3fff) AM_RAM /* 6164 8k SRAM, not populated */
}

void hp3478a_state::i8039_io(address_map &map)
{
	map.global_mask(0xff);
//XXX	map(0x1, 0x1).rw(FUNC(hp3478a_state::port1_r), FUNC(hp3478a_state::port1_w)); /* tms5220 reads and writes */
}

/******************************************************************************
 Input Ports
******************************************************************************/
static INPUT_PORTS_START( hp3478a )
INPUT_PORTS_END

/******************************************************************************
 Machine Drivers
******************************************************************************/
MACHINE_CONFIG_START(hp3478a_state::hp3478a)
	/* basic machine hardware */
	MCFG_DEVICE_ADD("maincpu", I8039, CPU_CLOCK)
	MCFG_DEVICE_PROGRAM_MAP(i8039_map)
	MCFG_DEVICE_IO_MAP(i8039_io)

MACHINE_CONFIG_END

/******************************************************************************
 ROM Definitions
******************************************************************************/
ROM_START( hp3478a )
	ROM_REGION( 0x2000, "maincpu", 0 )
	//ROM_LOAD("rom_dc118.bin", 0, 0x2000, CRC(7e65cdf6) SHA1(bd665cf7e07e63f825b2353c8322ed8a4376b3bd))	//main CPU ROM, can match other datecodes too
ROM_END

/******************************************************************************
 Drivers
******************************************************************************/

//    YEAR  NAME  PARENT  COMPAT  MACHINE  INPUT  CLASS      INIT        COMPANY                        FULLNAME             FLAGS
COMP( 1983, hp3478a,  0,      0,  hp3478a, hp3478a,hp3478a_state, empty_init, "HP", "HP 3478A Multimeter", MACHINE_IS_INCOMPLETE | MACHINE_NO_SOUND_HW | MACHINE_TYPE_OTHER)
