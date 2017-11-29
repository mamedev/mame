// license:BSD-3-Clause
// copyright-holders:Robbbert
/***************************************************************************

        SWTPC 6800

        10/12/2009 Skeleton driver.

        http://www.swtpc.com/mholley/swtpc_6800.htm

    bios 0 (SWTBUG) is made for a PIA (parallel) interface.
    bios 1 (MIKBUG) is made for a ACIA (serial) interface at the same address.
    MIKBUG will actually read the bits as they arrive and assemble a byte.

    Note: All commands must be in uppercase. See the SWTBUG manual.

    ToDo:
        - Split into 2 systems each with different hardware.


Commands:
B Breakpoint
C Clear screen
D Disk boot
E End of tape
F Find a byte
G Goto
J Jump
L Ascii Load
M Memory change (enter to quit, - to display next byte)
O Optional Port
P Ascii Punch
R Register dump
Z Goto Prom (0xC000)

****************************************************************************/

#include "emu.h"
#include "cpu/m6800/m6800.h"
#include "machine/6850acia.h"
#include "machine/clock.h"
#include "bus/rs232/rs232.h"

class swtpc_state : public driver_device
{
public:
	swtpc_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
	{ }

private:
	required_device<cpu_device> m_maincpu;
};

static ADDRESS_MAP_START(mem_map, AS_PROGRAM, 8, swtpc_state)
	ADDRESS_MAP_UNMAP_HIGH
	AM_RANGE(0x8004, 0x8004) AM_MIRROR(2) AM_DEVREADWRITE("uart", acia6850_device, status_r, control_w)
	AM_RANGE(0x8005, 0x8005) AM_MIRROR(2) AM_DEVREADWRITE("uart", acia6850_device, data_r, data_w)
	AM_RANGE(0xa000, 0xa07f) AM_RAM
	AM_RANGE(0xe000, 0xe3ff) AM_MIRROR(0x1c00) AM_ROM
ADDRESS_MAP_END

/* Input ports */
static INPUT_PORTS_START( swtpc )
INPUT_PORTS_END


static MACHINE_CONFIG_START( swtpc )
	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", M6800, XTAL_1MHz)
	MCFG_CPU_PROGRAM_MAP(mem_map)

	/* video hardware */
	MCFG_DEVICE_ADD("uart_clock", CLOCK, 153600)
	MCFG_CLOCK_SIGNAL_HANDLER(DEVWRITELINE("uart", acia6850_device, write_txc))
	MCFG_DEVCB_CHAIN_OUTPUT(DEVWRITELINE("uart", acia6850_device, write_rxc))

	MCFG_DEVICE_ADD("uart", ACIA6850, XTAL_1MHz)
	MCFG_ACIA6850_TXD_HANDLER(DEVWRITELINE("rs232", rs232_port_device, write_txd))
	MCFG_ACIA6850_RTS_HANDLER(DEVWRITELINE("rs232", rs232_port_device, write_rts))

	MCFG_RS232_PORT_ADD("rs232", default_rs232_devices, "terminal")
	MCFG_RS232_RXD_HANDLER(DEVWRITELINE("uart", acia6850_device, write_rxd))
	MCFG_RS232_CTS_HANDLER(DEVWRITELINE("uart", acia6850_device, write_cts))
MACHINE_CONFIG_END

/* ROM definition */
ROM_START( swtpc )
	ROM_REGION( 0x10000, "maincpu", ROMREGION_ERASEFF )
	ROM_SYSTEM_BIOS( 0, "swt", "SWTBUG" )
	ROMX_LOAD( "swtbug.bin", 0xe000, 0x0400, CRC(f9130ef4) SHA1(089b2d2a56ce9526c3e78ce5d49ce368b9eabc0c), ROM_BIOS(1))
	ROM_SYSTEM_BIOS( 1, "mik", "MIKBUG" )
	ROMX_LOAD( "mikbug.bin", 0xe000, 0x0400, CRC(e7f4d9d0) SHA1(5ad585218f9c9c70f38b3c74e3ed5dfe0357621c), ROM_BIOS(2))
ROM_END

/* Driver */

//    YEAR  NAME    PARENT  COMPAT  MACHINE  INPUT  STATE        INIT  COMPANY                                     FULLNAME      FLAGS
COMP( 1975, swtpc,  0,      0,      swtpc,   swtpc, swtpc_state, 0,    "Southwest Technical Products Corporation", "SWTPC 6800", MACHINE_NO_SOUND_HW )
