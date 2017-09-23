// license:BSD-3-Clause
// copyright-holders:Robbbert
/***************************************************************************

Seattle Computer SCP-300F S100 card. It has sockets on the card for
one serial and 2 parallel connections.

2013-08-14 Skeleton driver.

When started you must press Enter twice before anything happens.

All commands must be in UPPER case.

Known Commands:
B : Boot from disk?
D : Dump memory
E : Edit memory
F : Find
G : Go?
I : Input port
M : Move
O : Output port
R : Display / Modify Registers
S : Search
T : Trace

Chips on the board: 8259 x2; AM9513; 8251; 2716 ROM (MON-86 V1.5TDD)
There is a 4MHz crystal connected to the 9513.

****************************************************************************/

#include "emu.h"
#include "cpu/i86/i86.h"
#include "machine/clock.h"
#include "machine/i8251.h"
#include "bus/rs232/rs232.h"


class seattle_comp_state : public driver_device
{
public:
	seattle_comp_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
	{ }

private:
	required_device<cpu_device> m_maincpu;
};


static ADDRESS_MAP_START(seattle_mem, AS_PROGRAM, 16, seattle_comp_state)
	ADDRESS_MAP_UNMAP_HIGH
	AM_RANGE(0x00000,0xff7ff) AM_RAM
	AM_RANGE(0xff800,0xfffff) AM_ROM AM_REGION("user1", 0)
ADDRESS_MAP_END

static ADDRESS_MAP_START(seattle_io, AS_IO, 16, seattle_comp_state)
	//ADDRESS_MAP_UNMAP_HIGH
	ADDRESS_MAP_GLOBAL_MASK(0xff)
	AM_RANGE(0xf6, 0xf7) AM_DEVREADWRITE8("uart", i8251_device, data_r, data_w, 0x00ff)
	AM_RANGE(0xf6, 0xf7) AM_DEVREADWRITE8("uart", i8251_device, status_r, control_w, 0xff00)
	//AM_RANGE(0xf0, 0xf1) 8259_0
	//AM_RANGE(0xf2, 0xf3) 8259_1
	//AM_RANGE(0xf4, 0xf5) AM9513
	//AM_RANGE(0xf6, 0xf7) 8251
	//AM_RANGE(0xfc, 0xfd) Parallel data, status, serial DCD
	//AM_RANGE(0xfe, 0xff) Eprom disable bit, read sense switches (bank of 8 dipswitches)
ADDRESS_MAP_END

/* Input ports */
static INPUT_PORTS_START( seattle )
INPUT_PORTS_END


// bit 7 needs to be stripped off, we do this by choosing 7 bits and even parity
static DEVICE_INPUT_DEFAULTS_START( terminal )
	DEVICE_INPUT_DEFAULTS( "RS232_RXBAUD", 0xff, RS232_BAUD_9600 )
	DEVICE_INPUT_DEFAULTS( "RS232_TXBAUD", 0xff, RS232_BAUD_9600 )
	DEVICE_INPUT_DEFAULTS( "RS232_STARTBITS", 0xff, RS232_STARTBITS_1 )
	DEVICE_INPUT_DEFAULTS( "RS232_DATABITS", 0xff, RS232_DATABITS_7 )
	DEVICE_INPUT_DEFAULTS( "RS232_PARITY", 0xff, RS232_PARITY_EVEN )
	DEVICE_INPUT_DEFAULTS( "RS232_STOPBITS", 0xff, RS232_STOPBITS_2 )
DEVICE_INPUT_DEFAULTS_END

static MACHINE_CONFIG_START( seattle )
	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", I8086, 4000000) // no idea
	MCFG_CPU_PROGRAM_MAP(seattle_mem)
	MCFG_CPU_IO_MAP(seattle_io)

	/* video hardware */
	MCFG_DEVICE_ADD("uart_clock", CLOCK, 153600)
	MCFG_CLOCK_SIGNAL_HANDLER(DEVWRITELINE("uart", i8251_device, write_txc))
	MCFG_DEVCB_CHAIN_OUTPUT(DEVWRITELINE("uart", i8251_device, write_rxc))

	MCFG_DEVICE_ADD("uart", I8251, 0)
	MCFG_I8251_TXD_HANDLER(DEVWRITELINE("rs232", rs232_port_device, write_txd))
	MCFG_I8251_DTR_HANDLER(DEVWRITELINE("rs232", rs232_port_device, write_dtr))
	MCFG_I8251_RTS_HANDLER(DEVWRITELINE("rs232", rs232_port_device, write_rts))

	MCFG_RS232_PORT_ADD("rs232", default_rs232_devices, "terminal")
	MCFG_RS232_RXD_HANDLER(DEVWRITELINE("uart", i8251_device, write_rxd))
	MCFG_RS232_DSR_HANDLER(DEVWRITELINE("uart", i8251_device, write_dsr))
	MCFG_RS232_CTS_HANDLER(DEVWRITELINE("uart", i8251_device, write_cts))
	MCFG_DEVICE_CARD_DEVICE_INPUT_DEFAULTS("terminal", terminal) // must be exactly here
MACHINE_CONFIG_END

/* ROM definition */
ROM_START( seattle )
	ROM_REGION( 0x800, "user1", 0 )
	ROM_LOAD( "mon86 v1.5tdd", 0x0000, 0x0800, CRC(7db23169) SHA1(c791b02ca33a4e1f8e95eb541624a59738f378c4))
ROM_END

/* Driver */

//    YEAR  NAME     PARENT  COMPAT   MACHINE   INPUT    CLASS               INIT    COMPANY            FULLNAME    FLAGS
COMP( 1986, seattle, 0,      0,       seattle,  seattle, seattle_comp_state, 0,    "Seattle Computer", "SCP-300F", MACHINE_NO_SOUND_HW )
