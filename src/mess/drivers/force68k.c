// license:BSD-3-Clause
// copyright-holders:Joamim Larsson Edström
/***************************************************************************

    Force SYS68K/CPU-1 VME SBC driver

    13/06/2015

 The info found on the link below is for a later revision of the board I have
 but I hope it is compatible, My board is has proms from 1983 and no rev markings 
 so probably the original.

 http://bitsavers.trailing-edge.com/pdf/forceComputers/1988_Force_VMEbus_Products.pdf

 Some info from that document:

Address Range
----------------------------------------------------------
000 000 - 000 007 Initialisation vectors from system EPROM
000 008 - 01F FFF Dynamic RAM on CPU-1 B
000 008 - 07F FFF Dynamic RAM on CPU-1 D
080 008 - 09F FFF SYSTEM EPROM Area
OAO 000 - OBF FFF USER EPROMArea
OEO 000 - OFF FFF I/O Interfaces
100 000 - FEF FFF VMEbus addresses (A24)
FFO 000 - FFF FFF VMEbus Short I/O (A16)
----------------------------------------------------------

Description                  Device  Lvl  IRQ Vector No.
----------------------------------------------------------
 ABORT                        Switch  7    31
 Real Time Clock              58167A  6    30
 Parallel Interface and Timer 68230   5    29
 Terminal ACIA                6850    4    28
 Remote ACIA                  6850    3    27
 Host ACIA                    6850    2    26
----------------------------------------------------------

10. The VMEbus
---------------
The implemented VMEbus Interface includes 24 address, 16 data, 
6 address modifier and the asynchronous control signals.
A single level bus arbiter is provided to build multi master 
systems. In addition to the bus arbiter, a separate slave bus 
arbitration allows selection of the arbitration level (0-3).

The address modifier range .,Short 110 Access« can be selected 
via a jumper for variable system generation. The 7 interrupt 
request levels of the VMEbus are fully supported from the 
SYS68K1CPU-1 B/D. For multi-processing, each IRQ signal can be 
enabled/disabled via a jumper field.

Additionally, the SYS68K1CPU-1 B/D supports the ACFAIL, SYSRESET, 
SYSFAIL and SYSCLK signal (16 MHz).


Based on the 68ksbc.c

    TODO:
    - Memory map
    - Dump ROM:s
    - Add 3 x ACIA6850 
    - Add 1 x 68230 Motorola, Parallel Interface / Timer
    - Add 1 x MM58167A RTC
    - Add 1 x Abort Switch  
    - Add serial connector between ACIA:s and real terminal emulator
    - VME bus driver


****************************************************************************/

#include "bus/rs232/rs232.h"
#include "cpu/m68000/m68000.h"
#include "machine/6850acia.h"
#include "machine/clock.h"

class force68k_state : public driver_device
{
public:
	force68k_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_acia1(*this, "acia1")
		m_acia2(*this, "acia2")
		m_acia3(*this, "acia3")
	{
	}

	DECLARE_WRITE_LINE_MEMBER(write_acia_clock);

private:
	required_device<cpu_device> m_maincpu;
	required_device<acia6850_device> m_acia1;
	required_device<acia6850_device> m_acia2;
	required_device<acia6850_device> m_acia3;
};

static ADDRESS_MAP_START(force68k_mem, AS_PROGRAM, 16, force68k_state)
	AM_RANGE(0x000000, 0x000007) AM_ROM /* Vectors mapped from System EPROM */
	AM_RANGE(0x000008, 0x01ffff) AM_RAM /* DRAM */
	AM_RANGE(0x080008, 0x09ffff) AM_ROM /* System EPROM Area */
//	AM_RANGE(0x0a0000, 0x0bffff) AM_ROM /* User EPROM Area   */
//      AM_RANGE(0x0e0000, 0x0fffff) AM_READWRITE_PORT /* IO interfaces */
//	AM_RANGE(0x0e0000, 0x0e0001) AM_DEVREADWRITE8("acia", acia6850_device, status_r, control_w, 0x00ff)
//	AM_RANGE(0x0e0002, 0x0e0003) AM_DEVREADWRITE8("acia", acia6850_device, data_r, data_w, 0x00ff)
//	AM_RANGE(0x0e0004, 0x0e0005) AM_DEVREADWRITE8("acia", acia6850_device, data_r, data_w, 0x00ff)
//      AM_RANGE(0x100000, 0xfeffff) /* VMEbus Rev B addresses (24 bits) */
//      AM_RANGE(0xff0000, 0xffffff) /* VMEbus Rev B addresses (16 bits) */
ADDRESS_MAP_END


/* Input ports */
static INPUT_PORTS_START( force68k )
INPUT_PORTS_END


WRITE_LINE_MEMBER(force68k_state::write_acia_clock)
{
	m_acia->write_txc(state);
	m_acia->write_rxc(state);
}

static MACHINE_CONFIG_START( force68k, force68k_state )
	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", M68000, 8000000) 
	MCFG_CPU_PROGRAM_MAP(force68k_mem)

/*
	MCFG_DEVICE_ADD("acia", ACIA6850, 0)
	MCFG_ACIA6850_TXD_HANDLER(DEVWRITELINE("rs232", rs232_port_device, write_txd))
	MCFG_ACIA6850_RTS_HANDLER(DEVWRITELINE("rs232", rs232_port_device, write_rts))

	MCFG_RS232_PORT_ADD("rs232", default_rs232_devices, "terminal")
	MCFG_RS232_RXD_HANDLER(DEVWRITELINE("acia", acia6850_device, write_rxd))
	MCFG_RS232_CTS_HANDLER(DEVWRITELINE("acia", acia6850_device, write_cts))

	MCFG_DEVICE_ADD("acia_clock", CLOCK, 153600)
	MCFG_CLOCK_SIGNAL_HANDLER(WRITELINE(force68k_state, write_acia_clock))
*/
MACHINE_CONFIG_END

/* ROM definition */
ROM_START( forcecpu1 )
	ROM_REGION(0x1000000, "maincpu", 0)
//	ROM_LOAD( "forcecpu1.bin", 0x0000, 0x2f78, CRC(20a8d0d0) SHA1(544fd8bd8ed017115388c8b0f7a7a59a32253e43) )
ROM_END

/* Driver */

/*    YEAR  NAME    PARENT  COMPAT   MACHINE    INPUT    CLASS         INIT    COMPANY                FULLNAME               FLAGS */
COMP( 1983, forcecpu1,  0,       0,    force68k,   force68k, driver_device,  0,  "Force Computers",  "SYS68K/CPU-1", GAME_NO_SOUND_HW)
