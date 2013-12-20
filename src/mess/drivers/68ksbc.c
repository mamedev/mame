// license:MAME
// copyright-holders:Robbbert
/***************************************************************************

    Skeleton driver for 68k Single Board Computer

    29/03/2011

    http://www.kmitl.ac.th/~kswichit/68k/68k.html

    TODO:
    - Add RTC (type DS12887)

    All of the address and i/o decoding is done by a pair of XC9536
    mask-programmed custom devices.

    There are some chips used for unclear purposes (GPI, GPO, LCD).

    This computer has no sound, and no facility for saving or loading programs.

    When started, press ? key for a list of commands.

    Has 1MB of flash (which is actually just 12k of program),
    and 1MB of RAM. Memory map should probably be corrected.


****************************************************************************/

#include "emu.h"
#include "cpu/m68000/m68000.h"
#include "machine/6850acia.h"
#include "machine/serial.h"


class c68ksbc_state : public driver_device
{
public:
	c68ksbc_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu")
	{ }

	required_device<cpu_device> m_maincpu;
	virtual void machine_reset();
};

static ADDRESS_MAP_START(c68ksbc_mem, AS_PROGRAM, 16, c68ksbc_state)
	ADDRESS_MAP_UNMAP_HIGH
	AM_RANGE(0x000000, 0x002fff) AM_ROM
	AM_RANGE(0x003000, 0x5fffff) AM_RAM
	AM_RANGE(0x600000, 0x600001) AM_DEVREADWRITE8("acia", acia6850_device, status_read, control_write, 0x00ff)
	AM_RANGE(0x600002, 0x600003) AM_DEVREADWRITE8("acia", acia6850_device, data_read, data_write, 0x00ff)
ADDRESS_MAP_END


/* Input ports */
static INPUT_PORTS_START( c68ksbc )
INPUT_PORTS_END


void c68ksbc_state::machine_reset()
{
}

static ACIA6850_INTERFACE( acia_intf )
{
	153600,
	153600,
	DEVCB_DEVICE_LINE_MEMBER("rs232", serial_port_device, rx),
	DEVCB_DEVICE_LINE_MEMBER("rs232", serial_port_device, tx),
	DEVCB_DEVICE_LINE_MEMBER("rs232", rs232_port_device, cts_r),
	DEVCB_DEVICE_LINE_MEMBER("rs232", rs232_port_device, rts_w),
	DEVCB_NULL,
	DEVCB_NULL
};

static MACHINE_CONFIG_START( c68ksbc, c68ksbc_state )
	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", M68000, 8000000) // text says 8MHz, schematic says 10MHz
	MCFG_CPU_PROGRAM_MAP(c68ksbc_mem)

	MCFG_ACIA6850_ADD("acia", acia_intf)
	MCFG_RS232_PORT_ADD("rs232", default_rs232_devices, "serial_terminal")
MACHINE_CONFIG_END

/* ROM definition */
ROM_START( 68ksbc )
	ROM_REGION(0x1000000, "maincpu", 0)
	ROM_LOAD( "t68k.bin", 0x0000, 0x2f78, CRC(20a8d0d0) SHA1(544fd8bd8ed017115388c8b0f7a7a59a32253e43) )
ROM_END

/* Driver */

/*    YEAR  NAME    PARENT  COMPAT   MACHINE    INPUT    CLASS         INIT    COMPANY                FULLNAME               FLAGS */
COMP( 2002, 68ksbc,   0,       0,    c68ksbc,   c68ksbc, driver_device,  0,  "Ichit Sirichote", "68k Single Board Computer", GAME_NO_SOUND_HW)
